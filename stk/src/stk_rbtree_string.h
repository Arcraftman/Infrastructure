#ifndef STK_RBTREE_STRING_H
#define STK_RBTREE_STRING_H

#include "stk_rbtree_config.h"
#include "stk_string.h"

typedef struct RBNodeString {
    String data;
    int color;
    struct RBNodeString* left;
    struct RBNodeString* right;
    struct RBNodeString* parent;
} RBNodeString;

typedef struct {
    RBNodeString* root;
    RBNodeString* nil;
    size_t size;
} RBTreeString;

STK_API void RBTreeString_Init(RBTreeString* tree);
STK_API void RBTreeString_Destroy(RBTreeString* tree);
STK_API void RBTreeString_Insert(RBTreeString* tree, String value);
STK_API void RBTreeString_Remove(RBTreeString* tree, String value);
STK_API String RBTreeString_Find(RBTreeString* tree, String value);
STK_API bool RBTreeString_Contains(RBTreeString* tree, String value);
STK_API bool RBTreeString_Empty(RBTreeString* tree);
STK_API size_t RBTreeString_Size(RBTreeString* tree);
STK_API void RBTreeString_Inorder(RBTreeString* tree, void (*visit)(String));
STK_API void RBTreeString_Preorder(RBTreeString* tree, void (*visit)(String));
STK_API void RBTreeString_Postorder(RBTreeString* tree, void (*visit)(String));
STK_API String RBTreeString_Min(RBTreeString* tree);
STK_API String RBTreeString_Max(RBTreeString* tree);
STK_API void RBTreeString_Clear(RBTreeString* tree);

#endif