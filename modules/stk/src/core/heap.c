#include "stk/def.h"
#include "stk/utils/status.h"
#include "stk/utils/logger.h"
#include "stk/core/preset.h"
#include "stk/core/heap.h"

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
    if (!p) {
        STK_LOG_ERROR("Heap grow: realloc failed (size=%zu)", new_cap);
        return false;
    }
    h->data     = p;
    h->capacity = new_cap;
    STK_LOG_DEBUG("Heap grew: capacity=%zu", new_cap);
    return true;
}

STK_STATUS stk_heap_init(stk_heap *h, stk_heap_compare_fn cmp) {
    STK_RETURN_IF(!h, STK_EINVAL, "Heap init: NULL heap pointer");
    STK_RETURN_IF(!cmp, STK_EINVAL, "Heap init: NULL compare function");
    
    h->data     = NULL;
    h->size     = 0;
    h->capacity = 0;
    h->cmp      = cmp;
    
    STK_LOG_DEBUG("Heap initialized");
    return STK_OK;
}

STK_STATUS stk_heap_init_with_capacity(stk_heap *h, stk_heap_compare_fn cmp, size_t cap) {
    STK_RETURN_IF(!h, STK_EINVAL, "Heap init: NULL heap pointer");
    STK_RETURN_IF(!cmp, STK_EINVAL, "Heap init: NULL compare function");
    
    h->data = (void **)malloc(cap * sizeof(void *));
    if (!h->data && cap > 0) {
        STK_LOG_ERROR("Heap init: failed to allocate %zu entries", cap);
        return STK_ENOMEM;
    }
    h->size     = 0;
    h->capacity = h->data ? cap : 0;
    h->cmp      = cmp;
    
    STK_LOG_DEBUG("Heap init: capacity=%zu", cap);
    return STK_OK;
}

STK_STATUS stk_heap_free(stk_heap *h) {
    if (!h) {
        STK_LOG_WARN("Heap free: NULL heap pointer");
        return STK_EINVAL;
    }
    free(h->data);
    h->data     = NULL;
    h->size     = 0;
    h->capacity = 0;
    STK_LOG_DEBUG("Heap freed");
    return STK_OK;
}

STK_STATUS stk_heap_push(stk_heap *h, void *val) {
    STK_RETURN_IF(!h, STK_EINVAL, "Heap push: NULL heap pointer");
    STK_RETURN_IF(!val, STK_EINVAL, "Heap push: NULL value");
    
    if (h->size >= h->capacity && !grow(h)) {
        STK_LOG_ERROR("Heap push: failed to grow heap");
        return STK_ENOMEM;
    }
    h->data[h->size++] = val;
    sift_up(h, h->size - 1);
    
    STK_LOG_DEBUG("Heap push: size=%zu", h->size);
    return STK_OK;
}

void *stk_heap_pop(stk_heap *h) {
    if (!h) {
        STK_LOG_WARN("Heap pop: NULL heap pointer");
        return NULL;
    }
    if (h->size == 0) {
        STK_LOG_WARN("Heap pop: heap is empty");
        return NULL;
    }
    
    void *val = h->data[0];
    h->data[0] = h->data[--h->size];
    sift_down(h, 0);
    
    STK_LOG_DEBUG("Heap pop: size=%zu", h->size);
    return val;
}

void *stk_heap_top(const stk_heap *h) {
    if (!h) {
        STK_LOG_WARN("Heap top: NULL heap pointer");
        return NULL;
    }
    return h->size > 0 ? h->data[0] : NULL;
}

STK_STATUS stk_heap_remove(stk_heap *h, size_t idx) {
    STK_RETURN_IF(!h, STK_EINVAL, "Heap remove: NULL heap pointer");
    
    if (idx >= h->size) {
        STK_LOG_ERROR("Heap remove: index %zu out of range (size=%zu)", idx, h->size);
        return STK_ERANGE;
    }
    
    h->data[idx] = h->data[--h->size];
    sift_down(h, idx);
    if (idx > 0 && h->cmp(h->data[idx], h->data[(idx - 1) / 2]) < 0)
        sift_up(h, idx);
    
    STK_LOG_DEBUG("Heap remove: removed index %zu, new size=%zu", idx, h->size);
    return STK_OK;
}

bool stk_heap_empty(const stk_heap *h)     { return h ? h->size == 0 : true; }
size_t stk_heap_size(const stk_heap *h)    { return h ? h->size : 0; }
size_t stk_heap_capacity(const stk_heap *h) { return h ? h->capacity : 0; }