#include "stk/core/pool.h"
#include <stdlib.h>
#include <string.h>

/* Align element_size up to sizeof(void*) */
#define POOL_ALIGN(n) (((n) + sizeof(void*) - 1) & ~(sizeof(void*) - 1))

void pool_init(pool *p, size_t element_size, size_t slab_capacity) {
    p->element_size  = POOL_ALIGN(element_size);
    if (p->element_size < sizeof(void *))
        p->element_size = sizeof(void *);
    p->slab_capacity = slab_capacity > 0 ? slab_capacity : 64;
    p->slabs         = NULL;
    p->freelist      = NULL;
    p->allocated     = 0;
    p->total         = 0;
}

static int pool_add_slab(pool *p) {
    size_t slab_bytes = sizeof(pool_slab) + p->element_size * p->slab_capacity;
    pool_slab *slab = (pool_slab *)malloc(slab_bytes);
    if (!slab) return -1;
    slab->next = p->slabs;
    p->slabs = slab;
    /* Thread each element onto the freelist */
    char *start = (char *)(slab + 1);
    for (size_t i = 0; i < p->slab_capacity; i++) {
        pool_free_node *node = (pool_free_node *)(start + i * p->element_size);
        node->next = p->freelist;
        p->freelist = node;
    }
    p->total += p->slab_capacity;
    return 0;
}

void pool_destroy(pool *p) {
    pool_slab *slab = p->slabs;
    while (slab) {
        pool_slab *next = slab->next;
        free(slab);
        slab = next;
    }
    p->slabs     = NULL;
    p->freelist  = NULL;
    p->allocated = 0;
    p->total     = 0;
}

void *pool_alloc(pool *p) {
    if (!p->freelist) {
        if (pool_add_slab(p) != 0) return NULL;
    }
    pool_free_node *node = p->freelist;
    p->freelist = node->next;
    p->allocated++;
    return (void *)node;
}

void pool_free(pool *p, void *element) {
    if (!element) return;
    pool_free_node *node = (pool_free_node *)element;
    node->next = p->freelist;
    p->freelist = node;
    p->allocated--;
}

size_t pool_element_size(const pool *p) { return p->element_size; }
size_t pool_allocated(const pool *p)     { return p->allocated; }
size_t pool_available(const pool *p)     { return p->total - p->allocated; }
