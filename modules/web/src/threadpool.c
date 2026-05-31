#include "web/threadpool.h"

#include <errno.h>
#include <pthread.h>
#include <stdlib.h>
#include <string.h>

/* =========================================================================
 * Internal task queue node
 * ========================================================================= */

typedef struct task_node {
    web_task_fn  fn;
    void        *arg;
    struct task_node *next;
} task_node_t;

struct web_threadpool {
    pthread_t      *threads;
    int             thread_count;
    task_node_t    *queue_head;
    task_node_t    *queue_tail;
    int             queue_size;
    pthread_mutex_t lock;
    pthread_cond_t  signal;
    volatile int    running;
};

/* =========================================================================
 * Worker routine — pull tasks from the queue and execute them
 * ========================================================================= */

static void *
worker_routine(void *arg)
{
    web_threadpool_t *pool = (web_threadpool_t *)arg;

    while (1) {
        pthread_mutex_lock(&pool->lock);
        while (pool->running && !pool->queue_head)
            pthread_cond_wait(&pool->signal, &pool->lock);

        if (!pool->running) {
            pthread_mutex_unlock(&pool->lock);
            return NULL;
        }

        /* Dequeue */
        task_node_t *task = pool->queue_head;
        pool->queue_head = task->next;
        if (!pool->queue_head)
            pool->queue_tail = NULL;
        pool->queue_size--;
        pthread_mutex_unlock(&pool->lock);

        task->fn(task->arg);
        free(task);
    }
}

/* =========================================================================
 * Public API
 * ========================================================================= */

WEB_API web_threadpool_t *
web_threadpool_create(int thread_count)
{
    if (thread_count < 1) {
        errno = EINVAL;
        return NULL;
    }

    web_threadpool_t *pool = (web_threadpool_t *)calloc(1, sizeof(*pool));
    if (!pool) return NULL;

    pool->thread_count = thread_count;
    pool->running      = 1;

    if (pthread_mutex_init(&pool->lock, NULL) != 0) {
        free(pool);
        return NULL;
    }
    if (pthread_cond_init(&pool->signal, NULL) != 0) {
        pthread_mutex_destroy(&pool->lock);
        free(pool);
        return NULL;
    }

    pool->threads = (pthread_t *)calloc((size_t)thread_count, sizeof(pthread_t));
    if (!pool->threads) {
        pthread_cond_destroy(&pool->signal);
        pthread_mutex_destroy(&pool->lock);
        free(pool);
        return NULL;
    }

    for (int i = 0; i < thread_count; i++) {
        if (pthread_create(&pool->threads[i], NULL, worker_routine, pool) != 0) {
            /* Failed to spawn thread i — tear down what we have */
            pool->running = 0;
            pthread_cond_broadcast(&pool->signal);
            for (int j = 0; j < i; j++)
                pthread_join(pool->threads[j], NULL);
            pthread_cond_destroy(&pool->signal);
            pthread_mutex_destroy(&pool->lock);
            free(pool->threads);
            free(pool);
            return NULL;
        }
    }

    return pool;
}

WEB_API int
web_threadpool_dispatch(web_threadpool_t *pool, web_task_fn fn, void *arg)
{
    if (!pool || !fn) return -1;

    task_node_t *task = (task_node_t *)malloc(sizeof(*task));
    if (!task) return -1;

    task->fn   = fn;
    task->arg  = arg;
    task->next = NULL;

    pthread_mutex_lock(&pool->lock);

    if (!pool->running) {
        pthread_mutex_unlock(&pool->lock);
        free(task);
        errno = ESHUTDOWN;
        return -1;
    }

    if (pool->queue_tail)
        pool->queue_tail->next = task;
    else
        pool->queue_head = task;
    pool->queue_tail = task;
    pool->queue_size++;

    pthread_cond_signal(&pool->signal);
    pthread_mutex_unlock(&pool->lock);
    return 0;
}

WEB_API int
web_threadpool_count(const web_threadpool_t *pool)
{
    return pool ? pool->thread_count : 0;
}

WEB_API int
web_threadpool_queued(const web_threadpool_t *pool)
{
    if (!pool) return 0;
    int sz;
    pthread_mutex_lock(&((web_threadpool_t *)pool)->lock);
    sz = pool->queue_size;
    pthread_mutex_unlock(&((web_threadpool_t *)pool)->lock);
    return sz;
}

WEB_API void
web_threadpool_destroy(web_threadpool_t *pool)
{
    if (!pool) return;

    pthread_mutex_lock(&pool->lock);
    pool->running = 0;
    pthread_cond_broadcast(&pool->signal);
    pthread_mutex_unlock(&pool->lock);

    for (int i = 0; i < pool->thread_count; i++)
        pthread_join(pool->threads[i], NULL);

    /* Drain any remaining tasks */
    task_node_t *t = pool->queue_head;
    while (t) {
        task_node_t *next = t->next;
        free(t);
        t = next;
    }

    pthread_cond_destroy(&pool->signal);
    pthread_mutex_destroy(&pool->lock);
    free(pool->threads);
    free(pool);
}
