// stk_rbtree.h

#ifndef STK_CORE_RBTREE_H
#define STK_CORE_RBTREE_H

#include "stk/core/preset.h"
#include "stk/core/internal/rbtree.h"

typedef struct stk_rbnode {
    void* data;
    int color;
    struct stk_rbnode* left;
    struct stk_rbnode* right;
    struct stk_rbnode* parent;
} stk_rbnode;

typedef struct {
    stk_rbnode* root;
    stk_rbnode* nil;
    size_t size;
    int (*compare)(const void* a, const void* b);  // Compare function
} stk_rbtree;

STK_API void stk_rbtree_init(stk_rbtree* t, int (*compare)(const void* a, const void* b));
STK_API void stk_rbtree_free(stk_rbtree* t);
STK_API void stk_rbtree_insert(stk_rbtree* t, void* val);
STK_API void stk_rbtree_remove(stk_rbtree* t, void* val);
STK_API void* stk_rbtree_find(stk_rbtree* t, void* val);
STK_API bool stk_rbtree_has(stk_rbtree* t, void* val);
STK_API bool stk_rbtree_empty(stk_rbtree* t);
STK_API size_t stk_rbtree_size(stk_rbtree* t);
STK_API void stk_rbtree_inorder(stk_rbtree* t, void (*visit)(void*));
STK_API void stk_rbtree_preorder(stk_rbtree* t, void (*visit)(void*));
STK_API void stk_rbtree_postorder(stk_rbtree* t, void (*visit)(void*));
STK_API void* stk_rbtree_min(stk_rbtree* t);
STK_API void* stk_rbtree_max(stk_rbtree* t);
STK_API void stk_rbtree_clear(stk_rbtree* t);

#endif