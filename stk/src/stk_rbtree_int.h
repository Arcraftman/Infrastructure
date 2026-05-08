#ifndef STK_RBTREE_INT_H
#define STK_RBTREE_INT_H

#include <stddef.h>
#include <stdbool.h>
#include "stk_export.h"
#include "stk_rbtree_config.h"

typedef struct RBNodeInt {
    int data;
    int color;
    struct RBNodeInt* left;
    struct RBNodeInt* right;
    struct RBNodeInt* parent;
} RBNodeInt;

typedef struct {
    RBNodeInt* root;
    RBNodeInt* nil;
    size_t size;
} RBTreeInt;

STK_API void RBTreeInt_Init(RBTreeInt* tree);
STK_API void RBTreeInt_Destroy(RBTreeInt* tree);
STK_API void RBTreeInt_Insert(RBTreeInt* tree, int value);
STK_API void RBTreeInt_Remove(RBTreeInt* tree, int value);
STK_API int RBTreeInt_Find(RBTreeInt* tree, int value);
STK_API bool RBTreeInt_Contains(RBTreeInt* tree, int value);
STK_API bool RBTreeInt_Empty(RBTreeInt* tree);
STK_API size_t RBTreeInt_Size(RBTreeInt* tree);
STK_API void RBTreeInt_Inorder(RBTreeInt* tree, void (*visit)(int));
STK_API void RBTreeInt_Preorder(RBTreeInt* tree, void (*visit)(int));
STK_API void RBTreeInt_Postorder(RBTreeInt* tree, void (*visit)(int));
STK_API int RBTreeInt_Min(RBTreeInt* tree);
STK_API int RBTreeInt_Max(RBTreeInt* tree);
STK_API void RBTreeInt_Clear(RBTreeInt* tree);

#endif