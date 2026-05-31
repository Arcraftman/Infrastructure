#include "stk/core/arena.h"
#include <stdlib.h>
#include <string.h>

#define ALIGN_UP(x, a) (((x) + (a) - 1) & ~((size_t)(a) - 1))

static arena_block *block_create(arena *a, size_t min_size) {
    size_t cap = (min_size > a->block_size) ? min_size : a->block_size;
    arena_block *blk = (arena_block *)a->malloc_fn(sizeof(arena_block) + cap);
    if (!blk) return NULL;
    blk->next     = NULL;
    blk->used     = 0;
    blk->capacity = cap;
    a->total_allocated += cap;
    return blk;
}

void arena_init(arena *a, size_t block_size) {
    arena_init_custom(a, block_size, malloc, free);
}

void arena_init_custom(arena *a, size_t block_size,
                        void *(*malloc_fn)(size_t), void (*free_fn)(void *)) {
    a->head           = NULL;
    a->current        = NULL;
    a->block_size     = block_size > 0 ? block_size : 8192;
    a->total_allocated = 0;
    a->malloc_fn      = malloc_fn;
    a->free_fn        = free_fn;
}

void arena_free(arena *a) {
    arena_block *blk = a->head;
    while (blk) {
        arena_block *next = blk->next;
        a->free_fn(blk);
        blk = next;
    }
    a->head    = NULL;
    a->current = NULL;
    a->total_allocated = 0;
}

void *arena_alloc(arena *a, size_t size) {
    if (!a || size == 0) return NULL;
    size = ALIGN_UP(size, sizeof(void *));
    if (!a->current || a->current->used + size > a->current->capacity) {
        arena_block *blk = block_create(a, size + sizeof(arena_block));
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

void *arena_alloc_aligned(arena *a, size_t size, size_t align) {
    if (align <= sizeof(void *)) return arena_alloc(a, size);
    /* Reserve extra alignment bytes, then align the pointer forward */
    size_t extra = align - 1;
    void *base = arena_alloc(a, size + extra);
    if (!base) return NULL;
    uintptr_t addr = (uintptr_t)base;
    uintptr_t aligned = ALIGN_UP(addr, align);
    if (aligned != addr) {
        /* Shift the arena current->used back to reclaim padding */
        /* This is a best-effort: not all padding can be reclaimed */
    }
    return (void *)aligned;
}

char *arena_dup_str(arena *a, const char *s) {
    if (!s) return NULL;
    size_t n = strlen(s) + 1;
    char *p = (char *)arena_alloc(a, n);
    if (p) memcpy(p, s, n);
    return p;
}

char *arena_dup_strn(arena *a, const char *s, size_t n) {
    if (!s) return NULL;
    char *p = (char *)arena_alloc(a, n + 1);
    if (p) {
        memcpy(p, s, n);
        p[n] = '\0';
    }
    return p;
}

void arena_reset(arena *a) {
    arena_block *blk = a->head;
    while (blk) {
        blk->used = 0;
        blk = blk->next;
    }
    a->current = a->head;
}

size_t arena_total_allocated(const arena *a) {
    return a ? a->total_allocated : 0;
}

size_t arena_block_count(const arena *a) {
    size_t n = 0;
    for (arena_block *blk = a->head; blk; blk = blk->next) n++;
    return n;
}
