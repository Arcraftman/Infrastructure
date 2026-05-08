#include "stk_rbtree_double.h"
#include <stdlib.h>
#include <stdio.h>
#include <math.h>

#define DOUBLE_EPSILON 1e-12

static int double_compare(double a, double b) {
    if (fabs(a - b) < DOUBLE_EPSILON) return 0;
    return (a < b) ? -1 : 1;
}

// ========== 辅助函数 ==========

static RBNodeDouble* RBNodeDouble_Create(double value, RBNodeDouble* nil) {
    RBNodeDouble* node = (RBNodeDouble*)malloc(sizeof(RBNodeDouble));
    node->data = value;
    node->color = RB_RED;
    node->left = nil;
    node->right = nil;
    node->parent = nil;
    return node;
}

static void RBTreeDouble_LeftRotate(RBTreeDouble* tree, RBNodeDouble* x) {
    RBNodeDouble* y = x->right;
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

static void RBTreeDouble_RightRotate(RBTreeDouble* tree, RBNodeDouble* y) {
    RBNodeDouble* x = y->left;
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

static void RBTreeDouble_InsertFixup(RBTreeDouble* tree, RBNodeDouble* z) {
    while (z->parent->color == RB_RED) {
        if (z->parent == z->parent->parent->left) {
            RBNodeDouble* y = z->parent->parent->right;
            if (y->color == RB_RED) {
                z->parent->color = RB_BLACK;
                y->color = RB_BLACK;
                z->parent->parent->color = RB_RED;
                z = z->parent->parent;
            }
            else {
                if (z == z->parent->right) {
                    z = z->parent;
                    RBTreeDouble_LeftRotate(tree, z);
                }
                z->parent->color = RB_BLACK;
                z->parent->parent->color = RB_RED;
                RBTreeDouble_RightRotate(tree, z->parent->parent);
            }
        }
        else {
            RBNodeDouble* y = z->parent->parent->left;
            if (y->color == RB_RED) {
                z->parent->color = RB_BLACK;
                y->color = RB_BLACK;
                z->parent->parent->color = RB_RED;
                z = z->parent->parent;
            }
            else {
                if (z == z->parent->left) {
                    z = z->parent;
                    RBTreeDouble_RightRotate(tree, z);
                }
                z->parent->color = RB_BLACK;
                z->parent->parent->color = RB_RED;
                RBTreeDouble_LeftRotate(tree, z->parent->parent);
            }
        }
    }
    tree->root->color = RB_BLACK;
}

static RBNodeDouble* RBTreeDouble_Minimum(RBTreeDouble* tree, RBNodeDouble* node) {
    while (node->left != tree->nil) {
        node = node->left;
    }
    return node;
}

static RBNodeDouble* RBTreeDouble_Maximum(RBTreeDouble* tree, RBNodeDouble* node) {
    while (node->right != tree->nil) {
        node = node->right;
    }
    return node;
}

static RBNodeDouble* RBTreeDouble_Successor(RBTreeDouble* tree, RBNodeDouble* node) {
    if (node->right != tree->nil) {
        return RBTreeDouble_Minimum(tree, node->right);
    }
    RBNodeDouble* y = node->parent;
    while (y != tree->nil && node == y->right) {
        node = y;
        y = y->parent;
    }
    return y;
}

static void RBTreeDouble_Transplant(RBTreeDouble* tree, RBNodeDouble* u, RBNodeDouble* v) {
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

static void RBTreeDouble_DeleteFixup(RBTreeDouble* tree, RBNodeDouble* x) {
    while (x != tree->root && x->color == RB_BLACK) {
        if (x == x->parent->left) {
            RBNodeDouble* w = x->parent->right;
            if (w->color == RB_RED) {
                w->color = RB_BLACK;
                x->parent->color = RB_RED;
                RBTreeDouble_LeftRotate(tree, x->parent);
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
                    RBTreeDouble_RightRotate(tree, w);
                    w = x->parent->right;
                }
                w->color = x->parent->color;
                x->parent->color = RB_BLACK;
                w->right->color = RB_BLACK;
                RBTreeDouble_LeftRotate(tree, x->parent);
                x = tree->root;
            }
        }
        else {
            RBNodeDouble* w = x->parent->left;
            if (w->color == RB_RED) {
                w->color = RB_BLACK;
                x->parent->color = RB_RED;
                RBTreeDouble_RightRotate(tree, x->parent);
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
                    RBTreeDouble_LeftRotate(tree, w);
                    w = x->parent->left;
                }
                w->color = x->parent->color;
                x->parent->color = RB_BLACK;
                w->left->color = RB_BLACK;
                RBTreeDouble_RightRotate(tree, x->parent);
                x = tree->root;
            }
        }
    }
    x->color = RB_BLACK;
}

static void RBTreeDouble_DestroyNodes(RBTreeDouble* tree, RBNodeDouble* node) {
    if (node == tree->nil) return;
    RBTreeDouble_DestroyNodes(tree, node->left);
    RBTreeDouble_DestroyNodes(tree, node->right);
    free(node);
}

static void RBTreeDouble_InorderRecursive(RBTreeDouble* tree, RBNodeDouble* node, void (*visit)(double)) {
    if (node == tree->nil) return;
    RBTreeDouble_InorderRecursive(tree, node->left, visit);
    visit(node->data);
    RBTreeDouble_InorderRecursive(tree, node->right, visit);
}

static void RBTreeDouble_PreorderRecursive(RBTreeDouble* tree, RBNodeDouble* node, void (*visit)(double)) {
    if (node == tree->nil) return;
    visit(node->data);
    RBTreeDouble_PreorderRecursive(tree, node->left, visit);
    RBTreeDouble_PreorderRecursive(tree, node->right, visit);
}

static void RBTreeDouble_PostorderRecursive(RBTreeDouble* tree, RBNodeDouble* node, void (*visit)(double)) {
    if (node == tree->nil) return;
    RBTreeDouble_PostorderRecursive(tree, node->left, visit);
    RBTreeDouble_PostorderRecursive(tree, node->right, visit);
    visit(node->data);
}

// ========== 公共接口 ==========

STK_API void RBTreeDouble_Init(RBTreeDouble* tree) {
    tree->nil = (RBNodeDouble*)malloc(sizeof(RBNodeDouble));
    tree->nil->color = RB_BLACK;
    tree->nil->left = tree->nil->right = tree->nil->parent = tree->nil;
    tree->root = tree->nil;
    tree->size = 0;
}

STK_API void RBTreeDouble_Destroy(RBTreeDouble* tree) {
    RBTreeDouble_DestroyNodes(tree, tree->root);
    free(tree->nil);
    tree->root = NULL;
    tree->nil = NULL;
    tree->size = 0;
}

STK_API void RBTreeDouble_Insert(RBTreeDouble* tree, double value) {
    RBNodeDouble* z = RBNodeDouble_Create(value, tree->nil);
    RBNodeDouble* y = tree->nil;
    RBNodeDouble* x = tree->root;

    while (x != tree->nil) {
        y = x;
        if (double_compare(z->data, x->data) < 0) {
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
    else if (double_compare(z->data, y->data) < 0) {
        y->left = z;
    }
    else {
        y->right = z;
    }

    RBTreeDouble_InsertFixup(tree, z);
    tree->size++;
}

STK_API void RBTreeDouble_Remove(RBTreeDouble* tree, double value) {
    RBNodeDouble* z = tree->root;
    while (z != tree->nil) {
        int cmp = double_compare(value, z->data);
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

    RBNodeDouble* y = z;
    RBNodeDouble* x = NULL;
    int y_original_color = y->color;

    if (z->left == tree->nil) {
        x = z->right;
        RBTreeDouble_Transplant(tree, z, z->right);
    }
    else if (z->right == tree->nil) {
        x = z->left;
        RBTreeDouble_Transplant(tree, z, z->left);
    }
    else {
        y = RBTreeDouble_Minimum(tree, z->right);
        y_original_color = y->color;
        x = y->right;
        if (y->parent == z) {
            x->parent = y;
        }
        else {
            RBTreeDouble_Transplant(tree, y, y->right);
            y->right = z->right;
            y->right->parent = y;
        }
        RBTreeDouble_Transplant(tree, z, y);
        y->left = z->left;
        y->left->parent = y;
        y->color = z->color;
    }

    if (y_original_color == RB_BLACK) {
        RBTreeDouble_DeleteFixup(tree, x);
    }

    free(z);
    tree->size--;
}

STK_API double RBTreeDouble_Find(RBTreeDouble* tree, double value) {
    RBNodeDouble* node = tree->root;
    while (node != tree->nil) {
        int cmp = double_compare(value, node->data);
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
    return 0.0;
}

STK_API bool RBTreeDouble_Contains(RBTreeDouble* tree, double value) {
    RBNodeDouble* node = tree->root;
    while (node != tree->nil) {
        int cmp = double_compare(value, node->data);
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

STK_API bool RBTreeDouble_Empty(RBTreeDouble* tree) {
    return tree->root == tree->nil;
}

STK_API size_t RBTreeDouble_Size(RBTreeDouble* tree) {
    return tree->size;
}

STK_API void RBTreeDouble_Inorder(RBTreeDouble* tree, void (*visit)(double)) {
    RBTreeDouble_InorderRecursive(tree, tree->root, visit);
}

STK_API void RBTreeDouble_Preorder(RBTreeDouble* tree, void (*visit)(double)) {
    RBTreeDouble_PreorderRecursive(tree, tree->root, visit);
}

STK_API void RBTreeDouble_Postorder(RBTreeDouble* tree, void (*visit)(double)) {
    RBTreeDouble_PostorderRecursive(tree, tree->root, visit);
}

STK_API double RBTreeDouble_Min(RBTreeDouble* tree) {
    if (tree->root == tree->nil) return 0.0;
    return RBTreeDouble_Minimum(tree, tree->root)->data;
}

STK_API double RBTreeDouble_Max(RBTreeDouble* tree) {
    if (tree->root == tree->nil) return 0.0;
    return RBTreeDouble_Maximum(tree, tree->root)->data;
}

STK_API void RBTreeDouble_Clear(RBTreeDouble* tree) {
    RBTreeDouble_DestroyNodes(tree, tree->root);
    tree->root = tree->nil;
    tree->size = 0;
}