#include "stk/def.h"
#include "stk/utils/status.h"
#include "stk/utils/logger.h"
#include "stk/core/preset.h"
#include "stk/core/pool.h"

#define POOL_ALIGN(n) (((n) + sizeof(void*) - 1) & ~(sizeof(void*) - 1))

STK_STATUS stk_pool_init(stk_pool *p, size_t element_size, size_t slab_capacity) {
    STK_RETURN_IF(!p, STK_EINVAL, "Pool init: NULL pool pointer");
    STK_RETURN_IF(element_size == 0, STK_EINVAL, "Pool init: element_size must be > 0");
    
    p->element_size  = POOL_ALIGN(element_size);
    if (p->element_size < sizeof(void *))
        p->element_size = sizeof(void *);
    p->slab_capacity = slab_capacity > 0 ? slab_capacity : 64;
    p->slabs         = NULL;
    p->freelist      = NULL;
    p->allocated     = 0;
    p->total         = 0;
    
    STK_LOG_DEBUG("Pool init: element_size=%zu, slab_capacity=%zu", 
                  p->element_size, p->slab_capacity);
    return STK_OK;
}

static int pool_add_slab(stk_pool *p) {
    size_t slab_bytes = sizeof(stk_pool_slab) + p->element_size * p->slab_capacity;
    stk_pool_slab *slab = (stk_pool_slab *)malloc(slab_bytes);
    if (!slab) {
        STK_LOG_ERROR("Pool add_slab: malloc failed (size=%zu)", slab_bytes);
        return -1;
    }
    
    slab->next = p->slabs;
    p->slabs = slab;
    
    char *start = (char *)(slab + 1);
    for (size_t i = 0; i < p->slab_capacity; i++) {
        stk_pool_free_node *node = (stk_pool_free_node *)(start + i * p->element_size);
        node->next = p->freelist;
        p->freelist = node;
    }
    p->total += p->slab_capacity;
    
    STK_LOG_DEBUG("Pool add_slab: added slab, total=%zu, available=%zu", 
                  p->total, p->total - p->allocated);
    return 0;
}

STK_STATUS stk_pool_destroy(stk_pool *p) {
    if (!p) {
        STK_LOG_WARN("Pool destroy: NULL pool pointer");
        return STK_EINVAL;
    }
    
    stk_pool_slab *slab = p->slabs;
    size_t count = 0;
    while (slab) {
        stk_pool_slab *next = slab->next;
        free(slab);
        slab = next;
        count++;
    }
    
    p->slabs     = NULL;
    p->freelist  = NULL;
    p->allocated = 0;
    p->total     = 0;
    
    STK_LOG_DEBUG("Pool destroyed: %zu slabs freed", count);
    return STK_OK;
}

void *stk_pool_alloc(stk_pool *p) {
    if (!p) {
        STK_LOG_WARN("Pool alloc: NULL pool pointer");
        return NULL;
    }
    
    if (!p->freelist) {
        if (pool_add_slab(p) != 0) {
            STK_LOG_ERROR("Pool alloc: failed to add new slab");
            return NULL;
        }
    }
    
    stk_pool_free_node *node = p->freelist;
    p->freelist = node->next;
    p->allocated++;
    
    STK_LOG_DEBUG("Pool alloc: allocated=%zu, available=%zu", p->allocated, p->total - p->allocated);
    return (void *)node;
}

STK_STATUS stk_pool_free(stk_pool *p, void *element) {
    if (!p) {
        STK_LOG_WARN("Pool free: NULL pool pointer");
        return STK_EINVAL;
    }
    
    if (!element) {
        STK_LOG_WARN("Pool free: NULL element pointer");
        return STK_EINVAL;
    }
    
    stk_pool_free_node *node = (stk_pool_free_node *)element;
    node->next = p->freelist;
    p->freelist = node;
    p->allocated--;
    
    STK_LOG_DEBUG("Pool free: allocated=%zu, available=%zu", p->allocated, p->total - p->allocated);
    return STK_OK;
}

size_t stk_pool_element_size(const stk_pool *p) { 
    return p ? p->element_size : 0; 
}
size_t stk_pool_allocated(const stk_pool *p) { 
    return p ? p->allocated : 0; 
}
size_t stk_pool_available(const stk_pool *p) { 
    return p ? p->total - p->allocated : 0; 
}