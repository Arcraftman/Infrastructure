#ifndef WEB_THREADPOOL_H
#define WEB_THREADPOOL_H

#include "web/def.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @file threadpool.h
 * @brief Simple pthread-based thread pool for parallel request handling.
 *
 * Tasks are submitted via web_threadpool_dispatch() and executed
 * by the next available worker thread.
 */

typedef struct web_threadpool web_threadpool_t;

/** Task function type. */
typedef void (*web_task_fn)(void *arg);

/**
 * Create a thread pool with the given number of worker threads.
 * @param thread_count Number of worker threads (must be >= 1).
 * @return New thread pool, or NULL on error.
 */
WEB_API web_threadpool_t *
web_threadpool_create(int thread_count);

/**
 * Dispatch a task to the thread pool.
 * The task will be executed by the next available worker thread.
 * @param pool Thread pool.
 * @param fn   Task function.
 * @param arg  Argument passed to the task function (may be NULL).
 * @return 0 on success, -1 if the queue is full or shutting down.
 */
WEB_API int
web_threadpool_dispatch(web_threadpool_t *pool, web_task_fn fn, void *arg);

/**
 * Get the number of worker threads in the pool.
 */
WEB_API int
web_threadpool_count(const web_threadpool_t *pool);

/**
 * Get the number of tasks currently queued.
 */
WEB_API int
web_threadpool_queued(const web_threadpool_t *pool);

/**
 * Shut down the thread pool.
 * Waits for all queued tasks to complete, then joins worker threads.
 * @param pool Thread pool (freed on return).
 */
WEB_API void
web_threadpool_destroy(web_threadpool_t *pool);

#ifdef __cplusplus
}
#endif

#endif /* WEB_THREADPOOL_H */
