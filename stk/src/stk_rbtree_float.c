#include "stk_rbtree_float.h"
#include <stdlib.h>
#include <stdio.h>
#include <math.h>

#define FLOAT_EPSILON 1e-6f

static int float_compare(float a, float b) {
    if (fabs(a - b) < FLOAT_EPSILON) return 0;
    return (a < b) ? -1 : 1;
}

// ========== 辅助函数 ==========

static RBNodeFloat* RBNodeFloat_Create(float value, RBNodeFloat* nil) {
    RBNodeFloat* node = (RBNodeFloat*)malloc(sizeof(RBNodeFloat));
    node->data = value;
    node->color = RB_RED;
    node->left = nil;
    node->right = nil;
    node->parent = nil;
    return node;
}

static void RBTreeFloat_LeftRotate(RBTreeFloat* tree, RBNodeFloat* x) {
    RBNodeFloat* y = x->right;
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

static void RBTreeFloat_RightRotate(RBTreeFloat* tree, RBNodeFloat* y) {
    RBNodeFloat* x = y->left;
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

static void RBTreeFloat_InsertFixup(RBTreeFloat* tree, RBNodeFloat* z) {
    while (z->parent->color == RB_RED) {
        if (z->parent == z->parent->parent->left) {
            RBNodeFloat* y = z->parent->parent->right;
            if (y->color == RB_RED) {
                z->parent->color = RB_BLACK;
                y->color = RB_BLACK;
                z->parent->parent->color = RB_RED;
                z = z->parent->parent;
            }
            else {
                if (z == z->parent->right) {
                    z = z->parent;
                    RBTreeFloat_LeftRotate(tree, z);
                }
                z->parent->color = RB_BLACK;
                z->parent->parent->color = RB_RED;
                RBTreeFloat_RightRotate(tree, z->parent->parent);
            }
        }
        else {
            RBNodeFloat* y = z->parent->parent->left;
            if (y->color == RB_RED) {
                z->parent->color = RB_BLACK;
                y->color = RB_BLACK;
                z->parent->parent->color = RB_RED;
                z = z->parent->parent;
            }
            else {
                if (z == z->parent->left) {
                    z = z->parent;
                    RBTreeFloat_RightRotate(tree, z);
                }
                z->parent->color = RB_BLACK;
                z->parent->parent->color = RB_RED;
                RBTreeFloat_LeftRotate(tree, z->parent->parent);
            }
        }
    }
    tree->root->color = RB_BLACK;
}

static RBNodeFloat* RBTreeFloat_Minimum(RBTreeFloat* tree, RBNodeFloat* node) {
    while (node->left != tree->nil) {
        node = node->left;
    }
    return node;
}

static RBNodeFloat* RBTreeFloat_Maximum(RBTreeFloat* tree, RBNodeFloat* node) {
    while (node->right != tree->nil) {
        node = node->right;
    }
    return node;
}

static RBNodeFloat* RBTreeFloat_Successor(RBTreeFloat* tree, RBNodeFloat* node) {
    if (node->right != tree->nil) {
        return RBTreeFloat_Minimum(tree, node->right);
    }
    RBNodeFloat* y = node->parent;
    while (y != tree->nil && node == y->right) {
        node = y;
        y = y->parent;
    }
    return y;
}

static void RBTreeFloat_Transplant(RBTreeFloat* tree, RBNodeFloat* u, RBNodeFloat* v) {
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

static void RBTreeFloat_DeleteFixup(RBTreeFloat* tree, RBNodeFloat* x) {
    while (x != tree->root && x->color == RB_BLACK) {
        if (x == x->parent->left) {
            RBNodeFloat* w = x->parent->right;
            if (w->color == RB_RED) {
                w->color = RB_BLACK;
                x->parent->color = RB_RED;
                RBTreeFloat_LeftRotate(tree, x->parent);
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
                    RBTreeFloat_RightRotate(tree, w);
                    w = x->parent->right;
                }
                w->color = x->parent->color;
                x->parent->color = RB_BLACK;
                w->right->color = RB_BLACK;
                RBTreeFloat_LeftRotate(tree, x->parent);
                x = tree->root;
            }
        }
        else {
            RBNodeFloat* w = x->parent->left;
            if (w->color == RB_RED) {
                w->color = RB_BLACK;
                x->parent->color = RB_RED;
                RBTreeFloat_RightRotate(tree, x->parent);
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
                    RBTreeFloat_LeftRotate(tree, w);
                    w = x->parent->left;
                }
                w->color = x->parent->color;
                x->parent->color = RB_BLACK;
                w->left->color = RB_BLACK;
                RBTreeFloat_RightRotate(tree, x->parent);
                x = tree->root;
            }
        }
    }
    x->color = RB_BLACK;
}

static void RBTreeFloat_DestroyNodes(RBTreeFloat* tree, RBNodeFloat* node) {
    if (node == tree->nil) return;
    RBTreeFloat_DestroyNodes(tree, node->left);
    RBTreeFloat_DestroyNodes(tree, node->right);
    free(node);
}

static void RBTreeFloat_InorderRecursive(RBTreeFloat* tree, RBNodeFloat* node, void (*visit)(float)) {
    if (node == tree->nil) return;
    RBTreeFloat_InorderRecursive(tree, node->left, visit);
    visit(node->data);
    RBTreeFloat_InorderRecursive(tree, node->right, visit);
}

static void RBTreeFloat_PreorderRecursive(RBTreeFloat* tree, RBNodeFloat* node, void (*visit)(float)) {
    if (node == tree->nil) return;
    visit(node->data);
    RBTreeFloat_PreorderRecursive(tree, node->left, visit);
    RBTreeFloat_PreorderRecursive(tree, node->right, visit);
}

static void RBTreeFloat_PostorderRecursive(RBTreeFloat* tree, RBNodeFloat* node, void (*visit)(float)) {
    if (node == tree->nil) return;
    RBTreeFloat_PostorderRecursive(tree, node->left, visit);
    RBTreeFloat_PostorderRecursive(tree, node->right, visit);
    visit(node->data);
}

// ========== 公共接口 ==========

STK_API void RBTreeFloat_Init(RBTreeFloat* tree) {
    tree->nil = (RBNodeFloat*)malloc(sizeof(RBNodeFloat));
    tree->nil->color = RB_BLACK;
    tree->nil->left = tree->nil->right = tree->nil->parent = tree->nil;
    tree->root = tree->nil;
    tree->size = 0;
}

STK_API void RBTreeFloat_Destroy(RBTreeFloat* tree) {
    RBTreeFloat_DestroyNodes(tree, tree->root);
    free(tree->nil);
    tree->root = NULL;
    tree->nil = NULL;
    tree->size = 0;
}

STK_API void RBTreeFloat_Insert(RBTreeFloat* tree, float value) {
    RBNodeFloat* z = RBNodeFloat_Create(value, tree->nil);
    RBNodeFloat* y = tree->nil;
    RBNodeFloat* x = tree->root;

    while (x != tree->nil) {
        y = x;
        if (float_compare(z->data, x->data) < 0) {
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
    else if (float_compare(z->data, y->data) < 0) {
        y->left = z;
    }
    else {
        y->right = z;
    }

    RBTreeFloat_InsertFixup(tree, z);
    tree->size++;
}

STK_API void RBTreeFloat_Remove(RBTreeFloat* tree, float value) {
    RBNodeFloat* z = tree->root;
    while (z != tree->nil) {
        int cmp = float_compare(value, z->data);
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

    RBNodeFloat* y = z;
    RBNodeFloat* x = NULL;
    int y_original_color = y->color;

    if (z->left == tree->nil) {
        x = z->right;
        RBTreeFloat_Transplant(tree, z, z->right);
    }
    else if (z->right == tree->nil) {
        x = z->left;
        RBTreeFloat_Transplant(tree, z, z->left);
    }
    else {
        y = RBTreeFloat_Minimum(tree, z->right);
        y_original_color = y->color;
        x = y->right;
        if (y->parent == z) {
            x->parent = y;
        }
        else {
            RBTreeFloat_Transplant(tree, y, y->right);
            y->right = z->right;
            y->right->parent = y;
        }
        RBTreeFloat_Transplant(tree, z, y);
        y->left = z->left;
        y->left->parent = y;
        y->color = z->color;
    }

    if (y_original_color == RB_BLACK) {
        RBTreeFloat_DeleteFixup(tree, x);
    }

    free(z);
    tree->size--;
}

STK_API float RBTreeFloat_Find(RBTreeFloat* tree, float value) {
    RBNodeFloat* node = tree->root;
    while (node != tree->nil) {
        int cmp = float_compare(value, node->data);
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
    return 0.0f;
}

STK_API bool RBTreeFloat_Contains(RBTreeFloat* tree, float value) {
    RBNodeFloat* node = tree->root;
    while (node != tree->nil) {
        int cmp = float_compare(value, node->data);
        if (cmp < 0) {
            node = node->left;
        }
        else if (cmp > 0) {
            node = node->right;
        }
        else {
            return true;
        }
    }
    return false;
}

STK_API bool RBTreeFloat_Empty(RBTreeFloat* tree) {
    return tree->root == tree->nil;
}

STK_API size_t RBTreeFloat_Size(RBTreeFloat* tree) {
    return tree->size;
}

STK_API void RBTreeFloat_Inorder(RBTreeFloat* tree, void (*visit)(float)) {
    RBTreeFloat_InorderRecursive(tree, tree->root, visit);
}

STK_API void RBTreeFloat_Preorder(RBTreeFloat* tree, void (*visit)(float)) {
    RBTreeFloat_PreorderRecursive(tree, tree->root, visit);
}

STK_API void RBTreeFloat_Postorder(RBTreeFloat* tree, void (*visit)(float)) {
    RBTreeFloat_PostorderRecursive(tree, tree->root, visit);
}

STK_API float RBTreeFloat_Min(RBTreeFloat* tree) {
    if (tree->root == tree->nil) return 0.0f;
    return RBTreeFloat_Minimum(tree, tree->root)->data;
}

STK_API float RBTreeFloat_Max(RBTreeFloat* tree) {
    if (tree->root == tree->nil) return 0.0f;
    return RBTreeFloat_Maximum(tree, tree->root)->data;
}

STK_API void RBTreeFloat_Clear(RBTreeFloat* tree) {
    RBTreeFloat_DestroyNodes(tree, tree->root);
    tree->root = tree->nil;
    tree->size = 0;
}