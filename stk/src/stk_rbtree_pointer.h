#ifndef STK_RBTREE_POINTER_H
#define STK_RBTREE_POINTER_H

#include "stk_rbtree_config.h"

typedef struct RBNodePointer {
    void* data;
    int color;
    struct RBNodePointer* left;
    struct RBNodePointer* right;
    struct RBNodePointer* parent;
} RBNodePointer;

typedef struct {
    RBNodePointer* root;
    RBNodePointer* nil;
    int (*compare)(const void* a, const void* b);
    void (*destroy)(void* data);
    size_t size;
} RBTreePointer;

STK_API void RBTreePointer_Init(RBTreePointer* tree, int (*compare)(const void*, const void*));
STK_API void RBTreePointer_InitWithDestroy(RBTreePointer* tree, int (*compare)(const void*, const void*), void (*destroy)(void*));
STK_API void RBTreePointer_Destroy(RBTreePointer* tree);
STK_API void RBTreePointer_Insert(RBTreePointer* tree, void* value);
STK_API void RBTreePointer_Remove(RBTreePointer* tree, void* value);
STK_API void* RBTreePointer_Find(RBTreePointer* tree, void* value);
STK_API bool RBTreePointer_Contains(RBTreePointer* tree, void* value);
STK_API bool RBTreePointer_Empty(RBTreePointer* tree);
STK_API size_t RBTreePointer_Size(RBTreePointer* tree);
STK_API void RBTreePointer_Inorder(RBTreePointer* tree, void (*visit)(void*));
STK_API void RBTreePointer_Preorder(RBTreePointer* tree, void (*visit)(void*));
STK_API void RBTreePointer_Postorder(RBTreePointer* tree, void (*visit)(void*));
STK_API void* RBTreePointer_Min(RBTreePointer* tree);
STK_API void* RBTreePointer_Max(RBTreePointer* tree);
STK_API void RBTreePointer_Clear(RBTreePointer* tree);

#endif