#include "stk/core/arena.h"
#include <stdlib.h>
#include <string.h>

#define ALIGN_UP(x, a) (((x) + (a) - 1) & ~((size_t)(a) - 1))

static stk_arena_block *block_create(stk_arena *a, size_t min_size) {
    size_t cap = (min_size > a->block_size) ? min_size : a->block_size;
    stk_arena_block *blk = (stk_arena_block *)a->malloc_fn(sizeof(stk_arena_block) + cap);
    if (!blk) return NULL;
    blk->next     = NULL;
    blk->used     = 0;
    blk->capacity = cap;
    a->total_allocated += cap;
    return blk;
}

void stk_arena_init(stk_arena *a, size_t block_size) {
    stk_arena_init_custom(a, block_size, malloc, free);
}

void stk_arena_init_custom(stk_arena *a, size_t block_size,
                            void *(*malloc_fn)(size_t), void (*free_fn)(void *)) {
    a->head           = NULL;
    a->current        = NULL;
    a->block_size     = block_size > 0 ? block_size : 8192;
    a->total_allocated = 0;
    a->malloc_fn      = malloc_fn;
    a->free_fn        = free_fn;
}

void stk_arena_free(stk_arena *a) {
    stk_arena_block *blk = a->head;
    while (blk) {
        stk_arena_block *next = blk->next;
        a->free_fn(blk);
        blk = next;
    }
    a->head    = NULL;
    a->current = NULL;
    a->total_allocated = 0;
}

void *stk_arena_alloc(stk_arena *a, size_t size) {
    if (!a || size == 0) return NULL;
    size = ALIGN_UP(size, sizeof(void *));
    if (!a->current || a->current->used + size > a->current->capacity) {
        stk_arena_block *blk = block_create(a, size + sizeof(stk_arena_block));
        if (!blk) return NULL;
        if (a->current)
            a->current->next = blk;
        else
            a->head = blk;
        a->current = blk;
    }
    void *ptr = (char *)(a->current + 1) + a->current->used;
    a->current->used += size;
    return ptr;
}

void *stk_arena_alloc_aligned(stk_arena *a, size_t size, size_t align) {
    if (align <= sizeof(void *)) return stk_arena_alloc(a, size);
    /* Reserve extra alignment bytes, then align the pointer forward */
    size_t extra = align - 1;
    void *base = stk_arena_alloc(a, size + extra);
    if (!base) return NULL;
    uintptr_t addr = (uintptr_t)base;
    uintptr_t aligned = ALIGN_UP(addr, align);
    if (aligned != addr) {
        /* Shift the arena current->used back to reclaim padding */
        /* This is a best-effort: not all padding can be reclaimed */
    }
    return (void *)aligned;
}

char *stk_arena_dup_str(stk_arena *a, const char *s) {
    if (!s) return NULL;
    size_t n = strlen(s) + 1;
    char *p = (char *)stk_arena_alloc(a, n);
    if (p) memcpy(p, s, n);
    return p;
}

char *stk_arena_dup_strn(stk_arena *a, const char *s, size_t n) {
    if (!s) return NULL;
    char *p = (char *)stk_arena_alloc(a, n + 1);
    if (p) {
        memcpy(p, s, n);
        p[n] = '\0';
    }
    return p;
}

void stk_arena_reset(stk_arena *a) {
    stk_arena_block *blk = a->head;
    while (blk) {
        blk->used = 0;
        blk = blk->next;
    }
    a->current = a->head;
}

size_t stk_arena_total_allocated(const stk_arena *a) {
    return a ? a->total_allocated : 0;
}

size_t stk_arena_block_count(const stk_arena *a) {
    size_t n = 0;
    for (stk_arena_block *blk = a->head; blk; blk = blk->next) n++;
    return n;
}