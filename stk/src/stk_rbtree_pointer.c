#include "stk_rbtree_pointer.h"
#include <stdlib.h>
#include <stdio.h>

// ========== 辅助函数 ==========

static RBNodePointer* RBNodePointer_Create(void* value, RBNodePointer* nil) {
    RBNodePointer* node = (RBNodePointer*)malloc(sizeof(RBNodePointer));
    node->data = value;
    node->color = RB_RED;
    node->left = nil;
    node->right = nil;
    node->parent = nil;
    return node;
}

static void RBTreePointer_LeftRotate(RBTreePointer* tree, RBNodePointer* x) {
    RBNodePointer* y = x->right;
    x->right = y->left;
    if (y->left != tree->nil) {
        y->left->parent = x;
    }
    y->parent = x->parent;
    if (x->parent == tree->nil) {
        tree->root = y;
    }
    else if (x == x->parent->left) {
        x->parent->left = y;
    }
    else {
        x->parent->right = y;
    }
    y->left = x;
    x->parent = y;
}

static void RBTreePointer_RightRotate(RBTreePointer* tree, RBNodePointer* y) {
    RBNodePointer* x = y->left;
    y->left = x->right;
    if (x->right != tree->nil) {
        x->right->parent = y;
    }
    x->parent = y->parent;
    if (y->parent == tree->nil) {
        tree->root = x;
    }
    else if (y == y->parent->left) {
        y->parent->left = x;
    }
    else {
        y->parent->right = x;
    }
    x->right = y;
    y->parent = x;
}

static void RBTreePointer_InsertFixup(RBTreePointer* tree, RBNodePointer* z) {
    while (z->parent->color == RB_RED) {
        if (z->parent == z->parent->parent->left) {
            RBNodePointer* y = z->parent->parent->right;
            if (y->color == RB_RED) {
                z->parent->color = RB_BLACK;
                y->color = RB_BLACK;
                z->parent->parent->color = RB_RED;
                z = z->parent->parent;
            }
            else {
                if (z == z->parent->right) {
                    z = z->parent;
                    RBTreePointer_LeftRotate(tree, z);
                }
                z->parent->color = RB_BLACK;
                z->parent->parent->color = RB_RED;
                RBTreePointer_RightRotate(tree, z->parent->parent);
            }
        }
        else {
            RBNodePointer* y = z->parent->parent->left;
            if (y->color == RB_RED) {
                z->parent->color = RB_BLACK;
                y->color = RB_BLACK;
                z->parent->parent->color = RB_RED;
                z = z->parent->parent;
            }
            else {
                if (z == z->parent->left) {
                    z = z->parent;
                    RBTreePointer_RightRotate(tree, z);
                }
                z->parent->color = RB_BLACK;
                z->parent->parent->color = RB_RED;
                RBTreePointer_LeftRotate(tree, z->parent->parent);
            }
        }
    }
    tree->root->color = RB_BLACK;
}

static RBNodePointer* RBTreePointer_Minimum(RBTreePointer* tree, RBNodePointer* node) {
    while (node->left != tree->nil) {
        node = node->left;
    }
    return node;
}

static RBNodePointer* RBTreePointer_Maximum(RBTreePointer* tree, RBNodePointer* node) {
    while (node->right != tree->nil) {
        node = node->right;
    }
    return node;
}

static RBNodePointer* RBTreePointer_Successor(RBTreePointer* tree, RBNodePointer* node) {
    if (node->right != tree->nil) {
        return RBTreePointer_Minimum(tree, node->right);
    }
    RBNodePointer* y = node->parent;
    while (y != tree->nil && node == y->right) {
        node = y;
        y = y->parent;
    }
    return y;
}

static void RBTreePointer_Transplant(RBTreePointer* tree, RBNodePointer* u, RBNodePointer* v) {
    if (u->parent == tree->nil) {
        tree->root = v;
    }
    else if (u == u->parent->left) {
        u->parent->left = v;
    }
    else {
        u->parent->right = v;
    }
    v->parent = u->parent;
}

static void RBTreePointer_DeleteFixup(RBTreePointer* tree, RBNodePointer* x) {
    while (x != tree->root && x->color == RB_BLACK) {
        if (x == x->parent->left) {
            RBNodePointer* w = x->parent->right;
            if (w->color == RB_RED) {
                w->color = RB_BLACK;
                x->parent->color = RB_RED;
                RBTreePointer_LeftRotate(tree, x->parent);
                w = x->parent->right;
            }
            if (w->left->color == RB_BLACK && w->right->color == RB_BLACK) {
                w->color = RB_RED;
                x = x->parent;
            }
            else {
                if (w->right->color == RB_BLACK) {
                    w->left->color = RB_BLACK;
                    w->color = RB_RED;
                    RBTreePointer_RightRotate(tree, w);
                    w = x->parent->right;
                }
                w->color = x->parent->color;
                x->parent->color = RB_BLACK;
                w->right->color = RB_BLACK;
                RBTreePointer_LeftRotate(tree, x->parent);
                x = tree->root;
            }
        }
        else {
            RBNodePointer* w = x->parent->left;
            if (w->color == RB_RED) {
                w->color = RB_BLACK;
                x->parent->color = RB_RED;
                RBTreePointer_RightRotate(tree, x->parent);
                w = x->parent->left;
            }
            if (w->right->color == RB_BLACK && w->left->color == RB_BLACK) {
                w->color = RB_RED;
                x = x->parent;
            }
            else {
                if (w->left->color == RB_BLACK) {
                    w->right->color = RB_BLACK;
                    w->color = RB_RED;
                    RBTreePointer_LeftRotate(tree, w);
                    w = x->parent->left;
                }
                w->color = x->parent->color;
                x->parent->color = RB_BLACK;
                w->left->color = RB_BLACK;
                RBTreePointer_RightRotate(tree, x->parent);
                x = tree->root;
            }
        }
    }
    x->color = RB_BLACK;
}

static void RBTreePointer_DestroyNodes(RBTreePointer* tree, RBNodePointer* node) {
    if (node == tree->nil) return;
    RBTreePointer_DestroyNodes(tree, node->left);
    RBTreePointer_DestroyNodes(tree, node->right);
    if (tree->destroy && node->data) {
        tree->destroy(node->data);
    }
    free(node);
}

static void RBTreePointer_InorderRecursive(RBTreePointer* tree, RBNodePointer* node, void (*visit)(void*)) {
    if (node == tree->nil) return;
    RBTreePointer_InorderRecursive(tree, node->left, visit);
    visit(node->data);
    RBTreePointer_InorderRecursive(tree, node->right, visit);
}

static void RBTreePointer_PreorderRecursive(RBTreePointer* tree, RBNodePointer* node, void (*visit)(void*)) {
    if (node == tree->nil) return;
    visit(node->data);
    RBTreePointer_PreorderRecursive(tree, node->left, visit);
    RBTreePointer_PreorderRecursive(tree, node->right, visit);
}

static void RBTreePointer_PostorderRecursive(RBTreePointer* tree, RBNodePointer* node, void (*visit)(void*)) {
    if (node == tree->nil) return;
    RBTreePointer_PostorderRecursive(tree, node->left, visit);
    RBTreePointer_PostorderRecursive(tree, node->right, visit);
    visit(node->data);
}

// ========== 公共接口 ==========

STK_API void RBTreePointer_Init(RBTreePointer* tree, int (*compare)(const void*, const void*)) {
    RBTreePointer_InitWithDestroy(tree, compare, NULL);
}

STK_API void RBTreePointer_InitWithDestroy(RBTreePointer* tree, int (*compare)(const void*, const void*), void (*destroy)(void*)) {
    tree->nil = (RBNodePointer*)malloc(sizeof(RBNodePointer));
    tree->nil->color = RB_BLACK;
    tree->nil->left = tree->nil->right = tree->nil->parent = tree->nil;
    tree->root = tree->nil;
    tree->compare = compare;
    tree->destroy = destroy;
    tree->size = 0;
}

STK_API void RBTreePointer_Destroy(RBTreePointer* tree) {
    RBTreePointer_DestroyNodes(tree, tree->root);
    free(tree->nil);
    tree->root = NULL;
    tree->nil = NULL;
    tree->size = 0;
}

STK_API void RBTreePointer_Insert(RBTreePointer* tree, void* value) {
    RBNodePointer* z = RBNodePointer_Create(value, tree->nil);
    RBNodePointer* y = tree->nil;
    RBNodePointer* x = tree->root;

    while (x != tree->nil) {
        y = x;
        if (tree->compare(value, x->data) < 0) {
            x = x->left;
        }
        else {
            x = x->right;
        }
    }

    z->parent = y;
    if (y == tree->nil) {
        tree->root = z;
    }
    else if (tree->compare(z->data, y->data) < 0) {
        y->left = z;
    }
    else {
        y->right = z;
    }

    RBTreePointer_InsertFixup(tree, z);
    tree->size++;
}

STK_API void RBTreePointer_Remove(RBTreePointer* tree, void* value) {
    RBNodePointer* z = tree->root;
    while (z != tree->nil) {
        int cmp = tree->compare(value, z->data);
        if (cmp < 0) {
            z = z->left;
        }
        else if (cmp > 0) {
            z = z->right;
        }
        else {
            break;
        }
    }

    if (z == tree->nil) return;

    RBNodePointer* y = z;
    RBNodePointer* x = NULL;
    int y_original_color = y->color;

    if (z->left == tree->nil) {
        x = z->right;
        RBTreePointer_Transplant(tree, z, z->right);
    }
    else if (z->right == tree->nil) {
        x = z->left;
        RBTreePointer_Transplant(tree, z, z->left);
    }
    else {
        y = RBTreePointer_Minimum(tree, z->right);
        y_original_color = y->color;
        x = y->right;
        if (y->parent == z) {
            x->parent = y;
        }
        else {
            RBTreePointer_Transplant(tree, y, y->right);
            y->right = z->right;
            y->right->parent = y;
        }
        RBTreePointer_Transplant(tree, z, y);
        y->left = z->left;
        y->left->parent = y;
        y->color = z->color;
    }

    if (y_original_color == RB_BLACK) {
        RBTreePointer_DeleteFixup(tree, x);
    }

    if (tree->destroy && z->data) {
        tree->destroy(z->data);
    }
    free(z);
    tree->size--;
}

STK_API void* RBTreePointer_Find(RBTreePointer* tree, void* value) {
    RBNodePointer* node = tree->root;
    while (node != tree->nil) {
        int cmp = tree->compare(value, node->data);
        if (cmp < 0) {
            node = node->left;
        }
        else if (cmp > 0) {
            node = node->right;
        }
        else {
            return node->data;
        }
    }
    return NULL;
}

STK_API bool RBTreePointer_Contains(RBTreePointer* tree, void* value) {
    return RBTreePointer_Find(tree, value) != NULL;
}

STK_API bool RBTreePointer_Empty(RBTreePointer* tree) {
    return tree->root == tree->nil;
}

STK_API size_t RBTreePointer_Size(RBTreePointer* tree) {
    return tree->size;
}

STK_API void RBTreePointer_Inorder(RBTreePointer* tree, void (*visit)(void*)) {
    RBTreePointer_InorderRecursive(tree, tree->root, visit);
}

STK_API void RBTreePointer_Preorder(RBTreePointer* tree, void (*visit)(void*)) {
    RBTreePointer_PreorderRecursive(tree, tree->root, visit);
}

STK_API void RBTreePointer_Postorder(RBTreePointer* tree, void (*visit)(void*)) {
    RBTreePointer_PostorderRecursive(tree, tree->root, visit);
}

STK_API void* RBTreePointer_Min(RBTreePointer* tree) {
    if (tree->root == tree->nil) return NULL;
    return RBTreePointer_Minimum(tree, tree->root)->data;
}

STK_API void* RBTreePointer_Max(RBTreePointer* tree) {
    if (tree->root == tree->nil) return NULL;
    return RBTreePointer_Maximum(tree, tree->root)->data;
}

STK_API void RBTreePointer_Clear(RBTreePointer* tree) {
    RBTreePointer_DestroyNodes(tree, tree->root);
    tree->root = tree->nil;
    tree->size = 0;
}