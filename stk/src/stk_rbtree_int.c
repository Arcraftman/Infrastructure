#include "stk_rbtree_int.h"
#include <stdlib.h>
#include <stdio.h>


static RBNodeInt* RBNodeInt_Create(int value, RBNodeInt* nil) {
    RBNodeInt* node = (RBNodeInt*)malloc(sizeof(RBNodeInt));
    node->data = value;
    node->color = RB_RED;
    node->left = nil;
    node->right = nil;
    node->parent = nil;
    return node;
}

static void RBTreeInt_LeftRotate(RBTreeInt* tree, RBNodeInt* x) {
    RBNodeInt* y = x->right;
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

static void RBTreeInt_RightRotate(RBTreeInt* tree, RBNodeInt* y) {
    RBNodeInt* x = y->left;
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

static void RBTreeInt_InsertFixup(RBTreeInt* tree, RBNodeInt* z) {
    while (z->parent->color == RB_RED) {
        if (z->parent == z->parent->parent->left) {
            RBNodeInt* y = z->parent->parent->right;
            if (y->color == RB_RED) {
                z->parent->color = RB_BLACK;
                y->color = RB_BLACK;
                z->parent->parent->color = RB_RED;
                z = z->parent->parent;
            }
            else {
                if (z == z->parent->right) {
                    z = z->parent;
                    RBTreeInt_LeftRotate(tree, z);
                }
                z->parent->color = RB_BLACK;
                z->parent->parent->color = RB_RED;
                RBTreeInt_RightRotate(tree, z->parent->parent);
            }
        }
        else {
            RBNodeInt* y = z->parent->parent->left;
            if (y->color == RB_RED) {
                z->parent->color = RB_BLACK;
                y->color = RB_BLACK;
                z->parent->parent->color = RB_RED;
                z = z->parent->parent;
            }
            else {
                if (z == z->parent->left) {
                    z = z->parent;
                    RBTreeInt_RightRotate(tree, z);
                }
                z->parent->color = RB_BLACK;
                z->parent->parent->color = RB_RED;
                RBTreeInt_LeftRotate(tree, z->parent->parent);
            }
        }
    }
    tree->root->color = RB_BLACK;
}

static RBNodeInt* RBTreeInt_Minimum(RBTreeInt* tree, RBNodeInt* node) {
    while (node->left != tree->nil) {
        node = node->left;
    }
    return node;
}

static RBNodeInt* RBTreeInt_Maximum(RBTreeInt* tree, RBNodeInt* node) {
    while (node->right != tree->nil) {
        node = node->right;
    }
    return node;
}

static RBNodeInt* RBTreeInt_Successor(RBTreeInt* tree, RBNodeInt* node) {
    if (node->right != tree->nil) {
        return RBTreeInt_Minimum(tree, node->right);
    }
    RBNodeInt* y = node->parent;
    while (y != tree->nil && node == y->right) {
        node = y;
        y = y->parent;
    }
    return y;
}

static void RBTreeInt_Transplant(RBTreeInt* tree, RBNodeInt* u, RBNodeInt* v) {
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

static void RBTreeInt_DeleteFixup(RBTreeInt* tree, RBNodeInt* x) {
    while (x != tree->root && x->color == RB_BLACK) {
        if (x == x->parent->left) {
            RBNodeInt* w = x->parent->right;
            if (w->color == RB_RED) {
                w->color = RB_BLACK;
                x->parent->color = RB_RED;
                RBTreeInt_LeftRotate(tree, x->parent);
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
                    RBTreeInt_RightRotate(tree, w);
                    w = x->parent->right;
                }
                w->color = x->parent->color;
                x->parent->color = RB_BLACK;
                w->right->color = RB_BLACK;
                RBTreeInt_LeftRotate(tree, x->parent);
                x = tree->root;
            }
        }
        else {
            RBNodeInt* w = x->parent->left;
            if (w->color == RB_RED) {
                w->color = RB_BLACK;
                x->parent->color = RB_RED;
                RBTreeInt_RightRotate(tree, x->parent);
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
                    RBTreeInt_LeftRotate(tree, w);
                    w = x->parent->left;
                }
                w->color = x->parent->color;
                x->parent->color = RB_BLACK;
                w->left->color = RB_BLACK;
                RBTreeInt_RightRotate(tree, x->parent);
                x = tree->root;
            }
        }
    }
    x->color = RB_BLACK;
}

static void RBTreeInt_DestroyNodes(RBTreeInt* tree, RBNodeInt* node) {
    if (node == tree->nil) return;
    RBTreeInt_DestroyNodes(tree, node->left);
    RBTreeInt_DestroyNodes(tree, node->right);
    free(node);
}

static void RBTreeInt_InorderRecursive(RBTreeInt* tree, RBNodeInt* node, void (*visit)(int)) {
    if (node == tree->nil) return;
    RBTreeInt_InorderRecursive(tree, node->left, visit);
    visit(node->data);
    RBTreeInt_InorderRecursive(tree, node->right, visit);
}

static void RBTreeInt_PreorderRecursive(RBTreeInt* tree, RBNodeInt* node, void (*visit)(int)) {
    if (node == tree->nil) return;
    visit(node->data);
    RBTreeInt_PreorderRecursive(tree, node->left, visit);
    RBTreeInt_PreorderRecursive(tree, node->right, visit);
}

static void RBTreeInt_PostorderRecursive(RBTreeInt* tree, RBNodeInt* node, void (*visit)(int)) {
    if (node == tree->nil) return;
    RBTreeInt_PostorderRecursive(tree, node->left, visit);
    RBTreeInt_PostorderRecursive(tree, node->right, visit);
    visit(node->data);
}

// ========== 公共接口 ==========

STK_API void RBTreeInt_Init(RBTreeInt* tree) {
    tree->nil = (RBNodeInt*)malloc(sizeof(RBNodeInt));
    tree->nil->color = RB_BLACK;
    tree->nil->left = tree->nil->right = tree->nil->parent = tree->nil;
    tree->root = tree->nil;
    tree->size = 0;
}

STK_API void RBTreeInt_Destroy(RBTreeInt* tree) {
    RBTreeInt_DestroyNodes(tree, tree->root);
    free(tree->nil);
    tree->root = NULL;
    tree->nil = NULL;
    tree->size = 0;
}

STK_API void RBTreeInt_Insert(RBTreeInt* tree, int value) {
    RBNodeInt* z = RBNodeInt_Create(value, tree->nil);
    RBNodeInt* y = tree->nil;
    RBNodeInt* x = tree->root;

    while (x != tree->nil) {
        y = x;
        if (z->data < x->data) {
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
    else if (z->data < y->data) {
        y->left = z;
    }
    else {
        y->right = z;
    }

    RBTreeInt_InsertFixup(tree, z);
    tree->size++;
}

STK_API void RBTreeInt_Remove(RBTreeInt* tree, int value) {
    RBNodeInt* z = tree->root;
    while (z != tree->nil) {
        if (value < z->data) {
            z = z->left;
        }
        else if (value > z->data) {
            z = z->right;
        }
        else {
            break;
        }
    }

    if (z == tree->nil) return;

    RBNodeInt* y = z;
    RBNodeInt* x = NULL;
    int y_original_color = y->color;

    if (z->left == tree->nil) {
        x = z->right;
        RBTreeInt_Transplant(tree, z, z->right);
    }
    else if (z->right == tree->nil) {
        x = z->left;
        RBTreeInt_Transplant(tree, z, z->left);
    }
    else {
        y = RBTreeInt_Minimum(tree, z->right);
        y_original_color = y->color;
        x = y->right;
        if (y->parent == z) {
            x->parent = y;
        }
        else {
            RBTreeInt_Transplant(tree, y, y->right);
            y->right = z->right;
            y->right->parent = y;
        }
        RBTreeInt_Transplant(tree, z, y);
        y->left = z->left;
        y->left->parent = y;
        y->color = z->color;
    }

    if (y_original_color == RB_BLACK) {
        RBTreeInt_DeleteFixup(tree, x);
    }

    free(z);
    tree->size--;
}

STK_API int RBTreeInt_Find(RBTreeInt* tree, int value) {
    RBNodeInt* node = tree->root;
    while (node != tree->nil) {
        if (value < node->data) {
            node = node->left;
        }
        else if (value > node->data) {
            node = node->right;
        }
        else {
            return node->data;
        }
    }
    return 0;
}

STK_API bool RBTreeInt_Contains(RBTreeInt* tree, int value) {
    RBNodeInt* node = tree->root;
    while (node != tree->nil) {
        if (value < node->data) {
            node = node->left;
        }
        else if (value > node->data) {
            node = node->right;
        }
        else {
            return true;
        }
    }
    return false;
}

STK_API bool RBTreeInt_Empty(RBTreeInt* tree) {
    return tree->root == tree->nil;
}

STK_API size_t RBTreeInt_Size(RBTreeInt* tree) {
    return tree->size;
}

STK_API void RBTreeInt_Inorder(RBTreeInt* tree, void (*visit)(int)) {
    RBTreeInt_InorderRecursive(tree, tree->root, visit);
}

STK_API void RBTreeInt_Preorder(RBTreeInt* tree, void (*visit)(int)) {
    RBTreeInt_PreorderRecursive(tree, tree->root, visit);
}

STK_API void RBTreeInt_Postorder(RBTreeInt* tree, void (*visit)(int)) {
    RBTreeInt_PostorderRecursive(tree, tree->root, visit);
}

STK_API int RBTreeInt_Min(RBTreeInt* tree) {
    if (tree->root == tree->nil) return 0;
    return RBTreeInt_Minimum(tree, tree->root)->data;
}

STK_API int RBTreeInt_Max(RBTreeInt* tree) {
    if (tree->root == tree->nil) return 0;
    return RBTreeInt_Maximum(tree, tree->root)->data;
}

STK_API void RBTreeInt_Clear(RBTreeInt* tree) {
    RBTreeInt_DestroyNodes(tree, tree->root);
    tree->root = tree->nil;
    tree->size = 0;
}