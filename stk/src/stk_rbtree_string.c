#include "stk_rbtree_string.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

static int string_compare(const String* a, const String* b) {
    if (!a->data && !b->data) return 0;
    if (!a->data) return -1;
    if (!b->data) return 1;
    return strcmp(a->data, b->data);
}


static RBNodeString* RBNodeString_Create(String value, RBNodeString* nil) {
    RBNodeString* node = (RBNodeString*)malloc(sizeof(RBNodeString));
    StrInitFrom(&node->data, value.data);
    node->color = RB_RED;
    node->left = nil;
    node->right = nil;
    node->parent = nil;
    return node;
}

static void RBTreeString_LeftRotate(RBTreeString* tree, RBNodeString* x) {
    RBNodeString* y = x->right;
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

static void RBTreeString_RightRotate(RBTreeString* tree, RBNodeString* y) {
    RBNodeString* x = y->left;
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

static void RBTreeString_InsertFixup(RBTreeString* tree, RBNodeString* z) {
    while (z->parent->color == RB_RED) {
        if (z->parent == z->parent->parent->left) {
            RBNodeString* y = z->parent->parent->right;
            if (y->color == RB_RED) {
                z->parent->color = RB_BLACK;
                y->color = RB_BLACK;
                z->parent->parent->color = RB_RED;
                z = z->parent->parent;
            }
            else {
                if (z == z->parent->right) {
                    z = z->parent;
                    RBTreeString_LeftRotate(tree, z);
                }
                z->parent->color = RB_BLACK;
                z->parent->parent->color = RB_RED;
                RBTreeString_RightRotate(tree, z->parent->parent);
            }
        }
        else {
            RBNodeString* y = z->parent->parent->left;
            if (y->color == RB_RED) {
                z->parent->color = RB_BLACK;
                y->color = RB_BLACK;
                z->parent->parent->color = RB_RED;
                z = z->parent->parent;
            }
            else {
                if (z == z->parent->left) {
                    z = z->parent;
                    RBTreeString_RightRotate(tree, z);
                }
                z->parent->color = RB_BLACK;
                z->parent->parent->color = RB_RED;
                RBTreeString_LeftRotate(tree, z->parent->parent);
            }
        }
    }
    tree->root->color = RB_BLACK;
}

static RBNodeString* RBTreeString_Minimum(RBTreeString* tree, RBNodeString* node) {
    while (node->left != tree->nil) {
        node = node->left;
    }
    return node;
}

static RBNodeString* RBTreeString_Maximum(RBTreeString* tree, RBNodeString* node) {
    while (node->right != tree->nil) {
        node = node->right;
    }
    return node;
}

static RBNodeString* RBTreeString_Successor(RBTreeString* tree, RBNodeString* node) {
    if (node->right != tree->nil) {
        return RBTreeString_Minimum(tree, node->right);
    }
    RBNodeString* y = node->parent;
    while (y != tree->nil && node == y->right) {
        node = y;
        y = y->parent;
    }
    return y;
}

static void RBTreeString_Transplant(RBTreeString* tree, RBNodeString* u, RBNodeString* v) {
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

static void RBTreeString_DeleteFixup(RBTreeString* tree, RBNodeString* x) {
    while (x != tree->root && x->color == RB_BLACK) {
        if (x == x->parent->left) {
            RBNodeString* w = x->parent->right;
            if (w->color == RB_RED) {
                w->color = RB_BLACK;
                x->parent->color = RB_RED;
                RBTreeString_LeftRotate(tree, x->parent);
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
                    RBTreeString_RightRotate(tree, w);
                    w = x->parent->right;
                }
                w->color = x->parent->color;
                x->parent->color = RB_BLACK;
                w->right->color = RB_BLACK;
                RBTreeString_LeftRotate(tree, x->parent);
                x = tree->root;
            }
        }
        else {
            RBNodeString* w = x->parent->left;
            if (w->color == RB_RED) {
                w->color = RB_BLACK;
                x->parent->color = RB_RED;
                RBTreeString_RightRotate(tree, x->parent);
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
                    RBTreeString_LeftRotate(tree, w);
                    w = x->parent->left;
                }
                w->color = x->parent->color;
                x->parent->color = RB_BLACK;
                w->left->color = RB_BLACK;
                RBTreeString_RightRotate(tree, x->parent);
                x = tree->root;
            }
        }
    }
    x->color = RB_BLACK;
}

static void RBTreeString_DestroyNodes(RBTreeString* tree, RBNodeString* node) {
    if (node == tree->nil) return;
    RBTreeString_DestroyNodes(tree, node->left);
    RBTreeString_DestroyNodes(tree, node->right);
    StrDestroy(&node->data);
    free(node);
}

static void RBTreeString_InorderRecursive(RBTreeString* tree, RBNodeString* node, void (*visit)(String)) {
    if (node == tree->nil) return;
    RBTreeString_InorderRecursive(tree, node->left, visit);
    visit(node->data);
    RBTreeString_InorderRecursive(tree, node->right, visit);
}

static void RBTreeString_PreorderRecursive(RBTreeString* tree, RBNodeString* node, void (*visit)(String)) {
    if (node == tree->nil) return;
    visit(node->data);
    RBTreeString_PreorderRecursive(tree, node->left, visit);
    RBTreeString_PreorderRecursive(tree, node->right, visit);
}

static void RBTreeString_PostorderRecursive(RBTreeString* tree, RBNodeString* node, void (*visit)(String)) {
    if (node == tree->nil) return;
    RBTreeString_PostorderRecursive(tree, node->left, visit);
    RBTreeString_PostorderRecursive(tree, node->right, visit);
    visit(node->data);
}

// ========== 公共接口 ==========

STK_API void RBTreeString_Init(RBTreeString* tree) {
    tree->nil = (RBNodeString*)malloc(sizeof(RBNodeString));
    tree->nil->color = RB_BLACK;
    tree->nil->left = tree->nil->right = tree->nil->parent = tree->nil;
    StrInit(&tree->nil->data);
    tree->root = tree->nil;
    tree->size = 0;
}

STK_API void RBTreeString_Destroy(RBTreeString* tree) {
    RBTreeString_DestroyNodes(tree, tree->root);
    StrDestroy(&tree->nil->data);
    free(tree->nil);
    tree->root = NULL;
    tree->nil = NULL;
    tree->size = 0;
}

STK_API void RBTreeString_Insert(RBTreeString* tree, String value) {
    RBNodeString* z = RBNodeString_Create(value, tree->nil);
    RBNodeString* y = tree->nil;
    RBNodeString* x = tree->root;

    while (x != tree->nil) {
        y = x;
        if (string_compare(&z->data, &x->data) < 0) {
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
    else if (string_compare(&z->data, &y->data) < 0) {
        y->left = z;
    }
    else {
        y->right = z;
    }

    RBTreeString_InsertFixup(tree, z);
    tree->size++;
}

STK_API void RBTreeString_Remove(RBTreeString* tree, String value) {
    RBNodeString* z = tree->root;
    while (z != tree->nil) {
        int cmp = string_compare(&value, &z->data);
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

    RBNodeString* y = z;
    RBNodeString* x = NULL;
    int y_original_color = y->color;

    if (z->left == tree->nil) {
        x = z->right;
        RBTreeString_Transplant(tree, z, z->right);
    }
    else if (z->right == tree->nil) {
        x = z->left;
        RBTreeString_Transplant(tree, z, z->left);
    }
    else {
        y = RBTreeString_Minimum(tree, z->right);
        y_original_color = y->color;
        x = y->right;
        if (y->parent == z) {
            x->parent = y;
        }
        else {
            RBTreeString_Transplant(tree, y, y->right);
            y->right = z->right;
            y->right->parent = y;
        }
        RBTreeString_Transplant(tree, z, y);
        y->left = z->left;
        y->left->parent = y;
        y->color = z->color;
    }

    if (y_original_color == RB_BLACK) {
        RBTreeString_DeleteFixup(tree, x);
    }

    StrDestroy(&z->data);
    free(z);
    tree->size--;
}

STK_API String RBTreeString_Find(RBTreeString* tree, String value) {
    RBNodeString* node = tree->root;
    while (node != tree->nil) {
        int cmp = string_compare(&value, &node->data);
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
    return (String) { NULL, 0 };
}

STK_API bool RBTreeString_Contains(RBTreeString* tree, String value) {
    RBNodeString* node = tree->root;
    while (node != tree->nil) {
        int cmp = string_compare(&value, &node->data);
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

STK_API bool RBTreeString_Empty(RBTreeString* tree) {
    return tree->root == tree->nil;
}

STK_API size_t RBTreeString_Size(RBTreeString* tree) {
    return tree->size;
}

STK_API void RBTreeString_Inorder(RBTreeString* tree, void (*visit)(String)) {
    RBTreeString_InorderRecursive(tree, tree->root, visit);
}

STK_API void RBTreeString_Preorder(RBTreeString* tree, void (*visit)(String)) {
    RBTreeString_PreorderRecursive(tree, tree->root, visit);
}

STK_API void RBTreeString_Postorder(RBTreeString* tree, void (*visit)(String)) {
    RBTreeString_PostorderRecursive(tree, tree->root, visit);
}

STK_API String RBTreeString_Min(RBTreeString* tree) {
    if (tree->root == tree->nil) return (String) { NULL, 0 };
    return RBTreeString_Minimum(tree, tree->root)->data;
}

STK_API String RBTreeString_Max(RBTreeString* tree) {
    if (tree->root == tree->nil) return (String) { NULL, 0 };
    return RBTreeString_Maximum(tree, tree->root)->data;
}

STK_API void RBTreeString_Clear(RBTreeString* tree) {
    RBTreeString_DestroyNodes(tree, tree->root);
    tree->root = tree->nil;
    tree->size = 0;
}