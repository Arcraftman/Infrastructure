#include "stk/def.h"
#include "stk/utils/status.h"
#include "stk/utils/logger.h"
#include "stk/core/preset.h"
#include "stk/core/arena.h"


#define ALIGN_UP(x, a) (((x) + (a) - 1) & ~((size_t)(a) - 1))

static stk_arena_block *block_create(stk_arena *a, size_t min_size) {
    size_t cap = (min_size > a->block_size) ? min_size : a->block_size;
    stk_arena_block *blk = (stk_arena_block *)a->malloc_fn(sizeof(stk_arena_block) + cap);
    if (!blk) {
        STK_LOG_ERROR("Failed to create arena block (size=%zu)", cap);
        return NULL;
    }
    blk->next     = NULL;
    blk->used     = 0;
    blk->capacity = cap;
    a->total_allocated += cap;
    
    STK_LOG_DEBUG("Arena block created: capacity=%zu, total=%zu", cap, a->total_allocated);
    return blk;
}

STK_STATUS stk_arena_init(stk_arena *a, size_t block_size) {
    STK_LOG_DEBUG("Arena init: block_size=%zu", block_size);
    return stk_arena_init_custom(a, block_size, malloc, free);
}

STK_STATUS stk_arena_init_custom(stk_arena *a, size_t block_size,
                                  void *(*malloc_fn)(size_t), void (*free_fn)(void *)) {
    STK_RETURN_IF(!a, STK_EINVAL, "Arena init: NULL arena pointer");
    STK_RETURN_IF(!malloc_fn || !free_fn, STK_EINVAL, "Arena init: NULL malloc/free function");
    
    a->head           = NULL;
    a->current        = NULL;
    a->block_size     = block_size > 0 ? block_size : 8192;
    a->total_allocated = 0;
    a->malloc_fn      = malloc_fn;
    a->free_fn        = free_fn;
    
    STK_LOG_DEBUG("Arena init_custom: block_size=%zu", a->block_size);
    return STK_OK;
}

void stk_arena_free(stk_arena *a) {
    if (!a) {
        STK_LOG_WARN("Arena free: NULL arena pointer");
        return;
    }
    
    STK_LOG_DEBUG("Arena free: total_allocated=%zu, blocks=%zu",
                  a->total_allocated, stk_arena_block_count(a));
    
    stk_arena_block *blk = a->head;
    size_t block_count = 0;
    while (blk) {
        stk_arena_block *next = blk->next;
        a->free_fn(blk);
        blk = next;
        block_count++;
    }
    a->head    = NULL;
    a->current = NULL;
    a->total_allocated = 0;
    
    STK_LOG_DEBUG("Arena freed: %zu blocks released", block_count);
}

void *stk_arena_alloc(stk_arena *a, size_t size) {
    if (!a || size == 0) {
        STK_LOG_WARN("Arena alloc: invalid args (a=%p, size=%zu)", (void*)a, size);
        return NULL;
    }
    
    size_t original_size = size;
    size = ALIGN_UP(size, sizeof(void *));
    
    STK_LOG_DEBUG("Arena alloc: request=%zu, aligned=%zu", original_size, size);
    
    if (!a->current || a->current->used + size > a->current->capacity) {
        STK_LOG_DEBUG("Arena alloc: need new block (current_used=%zu, capacity=%zu)",
                      a->current ? a->current->used : 0,
                      a->current ? a->current->capacity : 0);
        
        stk_arena_block *blk = block_create(a, size + sizeof(stk_arena_block));
        if (!blk) {
            STK_LOG_ERROR("Arena alloc: failed to create new block");
            return NULL;
        }
        if (a->current)
            a->current->next = blk;
        else
            a->head = blk;
        a->current = blk;
    }
    
    void *ptr = (char *)(a->current + 1) + a->current->used;
    a->current->used += size;
    
    STK_LOG_DEBUG("Arena alloc: ptr=%p, block_used=%zu/%zu", ptr,
                  a->current->used, a->current->capacity);
    
    return ptr;
}