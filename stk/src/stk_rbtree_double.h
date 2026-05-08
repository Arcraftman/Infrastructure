#ifndef STK_RBTREE_DOUBLE_H
#define STK_RBTREE_DOUBLE_H

#include "stk_rbtree_config.h"

typedef struct RBNodeDouble {
    double data;
    int color;
    struct RBNodeDouble* left;
    struct RBNodeDouble* right;
    struct RBNodeDouble* parent;
} RBNodeDouble;

typedef struct {
    RBNodeDouble* root;
    RBNodeDouble* nil;
    size_t size;
} RBTreeDouble;

STK_API void RBTreeDouble_Init(RBTreeDouble* tree);
STK_API void RBTreeDouble_Destroy(RBTreeDouble* tree);
STK_API void RBTreeDouble_Insert(RBTreeDouble* tree, double value);
STK_API void RBTreeDouble_Remove(RBTreeDouble* tree, double value);
STK_API double RBTreeDouble_Find(RBTreeDouble* tree, double value);
STK_API bool RBTreeDouble_Contains(RBTreeDouble* tree, double value);
STK_API bool RBTreeDouble_Empty(RBTreeDouble* tree);
STK_API size_t RBTreeDouble_Size(RBTreeDouble* tree);
STK_API void RBTreeDouble_Inorder(RBTreeDouble* tree, void (*visit)(double));
STK_API void RBTreeDouble_Preorder(RBTreeDouble* tree, void (*visit)(double));
STK_API void RBTreeDouble_Postorder(RBTreeDouble* tree, void (*visit)(double));
STK_API double RBTreeDouble_Min(RBTreeDouble* tree);
STK_API double RBTreeDouble_Max(RBTreeDouble* tree);
STK_API void RBTreeDouble_Clear(RBTreeDouble* tree);

#endif