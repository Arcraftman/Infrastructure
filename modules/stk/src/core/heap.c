#include "stk/core/heap.h"
#include <stdlib.h>
#include <string.h>

#define HEAP_GROW_FACTOR 2

static void swap(stk_heap *h, size_t i, size_t j) {
    void *tmp = h->data[i];
    h->data[i] = h->data[j];
    h->data[j] = tmp;
}

static void sift_up(stk_heap *h, size_t idx) {
    while (idx > 0) {
        size_t parent = (idx - 1) / 2;
        if (h->cmp(h->data[parent], h->data[idx]) <= 0) break;
        swap(h, idx, parent);
        idx = parent;
    }
}

static void sift_down(stk_heap *h, size_t idx) {
    for (;;) {
        size_t smallest = idx;
        size_t left  = 2 * idx + 1;
        size_t right = 2 * idx + 2;
        if (left  < h->size && h->cmp(h->data[left],  h->data[smallest]) < 0)
            smallest = left;
        if (right < h->size && h->cmp(h->data[right], h->data[smallest]) < 0)
            smallest = right;
        if (smallest == idx) break;
        swap(h, idx, smallest);
        idx = smallest;
    }
}

static bool grow(stk_heap *h) {
    size_t new_cap = h->capacity ? h->capacity * HEAP_GROW_FACTOR : 16;
    void **p = (void **)realloc(h->data, new_cap * sizeof(void *));
    if (!p) return false;
    h->data     = p;
    h->capacity = new_cap;
    return true;
}

void stk_heap_init(stk_heap *h, stk_heap_compare_fn cmp) {
    h->data     = NULL;
    h->size     = 0;
    h->capacity = 0;
    h->cmp      = cmp;
}

void stk_heap_init_with_capacity(stk_heap *h, stk_heap_compare_fn cmp, size_t cap) {
    h->data     = (void **)malloc(cap * sizeof(void *));
    h->size     = 0;
    h->capacity = h->data ? cap : 0;
    h->cmp      = cmp;
}

void stk_heap_free(stk_heap *h) {
    free(h->data);
    h->data     = NULL;
    h->size     = 0;
    h->capacity = 0;
}

void stk_heap_push(stk_heap *h, void *val) {
    if (h->size >= h->capacity && !grow(h)) return;
    h->data[h->size++] = val;
    sift_up(h, h->size - 1);
}

void *stk_heap_pop(stk_heap *h) {
    if (h->size == 0) return NULL;
    void *val = h->data[0];
    h->data[0] = h->data[--h->size];
    sift_down(h, 0);
    return val;
}

void *stk_heap_top(const stk_heap *h) {
    return h->size > 0 ? h->data[0] : NULL;
}

void stk_heap_remove(stk_heap *h, size_t idx) {
    if (idx >= h->size) return;
    h->data[idx] = h->data[--h->size];
    sift_down(h, idx);
    if (idx > 0 && h->cmp(h->data[idx], h->data[(idx - 1) / 2]) < 0)
        sift_up(h, idx);
}

bool   stk_heap_empty(const stk_heap *h)     { return h->size == 0; }
size_t stk_heap_size(const stk_heap *h)      { return h->size; }
size_t stk_heap_capacity(const stk_heap *h)  { return h->capacity; }