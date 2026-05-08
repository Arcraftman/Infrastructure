#ifndef STK_RBTREE_FLOAT_H
#define STK_RBTREE_FLOAT_H

#include "stk_rbtree_config.h"

typedef struct RBNodeFloat {
    float data;
    int color;
    struct RBNodeFloat* left;
    struct RBNodeFloat* right;
    struct RBNodeFloat* parent;
} RBNodeFloat;

typedef struct {
    RBNodeFloat* root;
    RBNodeFloat* nil;
    size_t size;
} RBTreeFloat;

STK_API void RBTreeFloat_Init(RBTreeFloat* tree);
STK_API void RBTreeFloat_Destroy(RBTreeFloat* tree);
STK_API void RBTreeFloat_Insert(RBTreeFloat* tree, float value);
STK_API void RBTreeFloat_Remove(RBTreeFloat* tree, float value);
STK_API float RBTreeFloat_Find(RBTreeFloat* tree, float value);
STK_API bool RBTreeFloat_Contains(RBTreeFloat* tree, float value);
STK_API bool RBTreeFloat_Empty(RBTreeFloat* tree);
STK_API size_t RBTreeFloat_Size(RBTreeFloat* tree);
STK_API void RBTreeFloat_Inorder(RBTreeFloat* tree, void (*visit)(float));
STK_API void RBTreeFloat_Preorder(RBTreeFloat* tree, void (*visit)(float));
STK_API void RBTreeFloat_Postorder(RBTreeFloat* tree, void (*visit)(float));
STK_API float RBTreeFloat_Min(RBTreeFloat* tree);
STK_API float RBTreeFloat_Max(RBTreeFloat* tree);
STK_API void RBTreeFloat_Clear(RBTreeFloat* tree);

#endif