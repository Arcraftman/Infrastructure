// stk_rbtree.h

#ifndef STK_SRC_CORE_RBTREE_H
#define STK_SRC_CORE_RBTREE_H

#include "config.h"

typedef struct rbnode {
    void* data;
    int color;
    struct rbnode* left;
    struct rbnode* right;
    struct rbnode* parent;
} rbnode;

typedef struct {
    rbnode* root;
    rbnode* nil;
    size_t size;
    int (*compare)(const void* a, const void* b);  // Compare function
} rbtree;

STK_API void rbtree_init(rbtree* t, int (*compare)(const void* a, const void* b));
STK_API void rbtree_free(rbtree* t);
STK_API void rbtree_insert(rbtree* t, void* val);
STK_API void rbtree_remove(rbtree* t, void* val);
STK_API void* rbtree_find(rbtree* t, void* val);
STK_API bool rbtree_has(rbtree* t, void* val);
STK_API bool rbtree_empty(rbtree* t);
STK_API size_t rbtree_size(rbtree* t);
STK_API void rbtree_inorder(rbtree* t, void (*visit)(void*));
STK_API void rbtree_preorder(rbtree* t, void (*visit)(void*));
STK_API void rbtree_postorder(rbtree* t, void (*visit)(void*));
STK_API void* rbtree_min(rbtree* t);
STK_API void* rbtree_max(rbtree* t);
STK_API void rbtree_clear(rbtree* t);

#endif