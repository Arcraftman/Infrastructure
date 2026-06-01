// stk_rbtree.c

#include "stk/core/rbtree.h"
#include <stdlib.h>
#include <string.h>

#define STK_RB_RED   0
#define STK_RB_BLACK 1

#define STK_RB_DEFAULT_CAPACITY 16
#define STK_RB_GROW_FACTOR 2


static stk_rbnode* node_new(void* val, stk_rbnode* nil) {
    stk_rbnode* n = (stk_rbnode*)malloc(sizeof(stk_rbnode));
    n->data = val;
    n->color = STK_RB_RED;
    n->left = nil;
    n->right = nil;
    n->parent = nil;
    return n;
}

static void left_rotate(stk_rbtree* t, stk_rbnode* x) {
    stk_rbnode* y = x->right;
    x->right = y->left;
    if (y->left != t->nil) y->left->parent = x;
    y->parent = x->parent;
    if (x->parent == t->nil) t->root = y;
    else if (x == x->parent->left) x->parent->left = y;
    else x->parent->right = y;
    y->left = x;
    x->parent = y;
}

static void right_rotate(stk_rbtree* t, stk_rbnode* y) {
    stk_rbnode* x = y->left;
    y->left = x->right;
    if (x->right != t->nil) x->right->parent = y;
    x->parent = y->parent;
    if (y->parent == t->nil) t->root = x;
    else if (y == y->parent->left) y->parent->left = x;
    else y->parent->right = x;
    x->right = y;
    y->parent = x;
}

static void insert_fixup(stk_rbtree* t, stk_rbnode* z) {
    while (z->parent->color == STK_RB_RED) {
        if (z->parent == z->parent->parent->left) {
            stk_rbnode* y = z->parent->parent->right;
            if (y->color == STK_RB_RED) {
                z->parent->color = STK_RB_BLACK;
                y->color = STK_RB_BLACK;
                z->parent->parent->color = STK_RB_RED;
                z = z->parent->parent;
            } else {
                if (z == z->parent->right) {
                    z = z->parent;
                    left_rotate(t, z);
                }
                z->parent->color = STK_RB_BLACK;
                z->parent->parent->color = STK_RB_RED;
                right_rotate(t, z->parent->parent);
            }
        } else {
            stk_rbnode* y = z->parent->parent->left;
            if (y->color == STK_RB_RED) {
                z->parent->color = STK_RB_BLACK;
                y->color = STK_RB_BLACK;
                z->parent->parent->color = STK_RB_RED;
                z = z->parent->parent;
            } else {
                if (z == z->parent->left) {
                    z = z->parent;
                    right_rotate(t, z);
                }
                z->parent->color = STK_RB_BLACK;
                z->parent->parent->color = STK_RB_RED;
                left_rotate(t, z->parent->parent);
            }
        }
    }
    t->root->color = STK_RB_BLACK;
}

static stk_rbnode* minimum(stk_rbtree* t, stk_rbnode* n) {
    while (n->left != t->nil) n = n->left;
    return n;
}

static stk_rbnode* maximum(stk_rbtree* t, stk_rbnode* n) {
    while (n->right != t->nil) n = n->right;
    return n;
}

static void transplant(stk_rbtree* t, stk_rbnode* u, stk_rbnode* v) {
    if (u->parent == t->nil) t->root = v;
    else if (u == u->parent->left) u->parent->left = v;
    else u->parent->right = v;
    v->parent = u->parent;
}

static void delete_fixup(stk_rbtree* t, stk_rbnode* x) {
    while (x != t->root && x->color == STK_RB_BLACK) {
        if (x == x->parent->left) {
            stk_rbnode* w = x->parent->right;
            if (w->color == STK_RB_RED) {
                w->color = STK_RB_BLACK;
                x->parent->color = STK_RB_RED;
                left_rotate(t, x->parent);
                w = x->parent->right;
            }
            if (w->left->color == STK_RB_BLACK && w->right->color == STK_RB_BLACK) {
                w->color = STK_RB_RED;
                x = x->parent;
            } else {
                if (w->right->color == STK_RB_BLACK) {
                    w->left->color = STK_RB_BLACK;
                    w->color = STK_RB_RED;
                    right_rotate(t, w);
                    w = x->parent->right;
                }
                w->color = x->parent->color;
                x->parent->color = STK_RB_BLACK;
                w->right->color = STK_RB_BLACK;
                left_rotate(t, x->parent);
                x = t->root;
            }
        } else {
            stk_rbnode* w = x->parent->left;
            if (w->color == STK_RB_RED) {
                w->color = STK_RB_BLACK;
                x->parent->color = STK_RB_RED;
                right_rotate(t, x->parent);
                w = x->parent->left;
            }
            if (w->right->color == STK_RB_BLACK && w->left->color == STK_RB_BLACK) {
                w->color = STK_RB_RED;
                x = x->parent;
            } else {
                if (w->left->color == STK_RB_BLACK) {
                    w->right->color = STK_RB_BLACK;
                    w->color = STK_RB_RED;
                    left_rotate(t, w);
                    w = x->parent->left;
                }
                w->color = x->parent->color;
                x->parent->color = STK_RB_BLACK;
                w->left->color = STK_RB_BLACK;
                right_rotate(t, x->parent);
                x = t->root;
            }
        }
    }
    x->color = STK_RB_BLACK;
}

static void destroy_nodes(stk_rbtree* t, stk_rbnode* n) {
    if (n == t->nil) return;
    destroy_nodes(t, n->left);
    destroy_nodes(t, n->right);
    free(n);
}

static void inorder_rec(stk_rbtree* t, stk_rbnode* n, void (*visit)(void*)) {
    if (n == t->nil) return;
    inorder_rec(t, n->left, visit);
    visit(n->data);
    inorder_rec(t, n->right, visit);
}

static void preorder_rec(stk_rbtree* t, stk_rbnode* n, void (*visit)(void*)) {
    if (n == t->nil) return;
    visit(n->data);
    preorder_rec(t, n->left, visit);
    preorder_rec(t, n->right, visit);
}

static void postorder_rec(stk_rbtree* t, stk_rbnode* n, void (*visit)(void*)) {
    if (n == t->nil) return;
    postorder_rec(t, n->left, visit);
    postorder_rec(t, n->right, visit);
    visit(n->data);
}

void stk_rbtree_init(stk_rbtree* t, int (*compare)(const void* a, const void* b)) {
    t->nil = (stk_rbnode*)malloc(sizeof(stk_rbnode));
    t->nil->color = STK_RB_BLACK;
    t->nil->left = t->nil->right = t->nil->parent = t->nil;
    t->root = t->nil;
    t->size = 0;
    t->compare = compare;
}

void stk_rbtree_free(stk_rbtree* t) {
    destroy_nodes(t, t->root);
    free(t->nil);
    t->root = NULL;
    t->nil = NULL;
    t->size = 0;
    t->compare = NULL;
}

void stk_rbtree_insert(stk_rbtree* t, void* val) {
    stk_rbnode* z = node_new(val, t->nil);
    stk_rbnode* y = t->nil;
    stk_rbnode* x = t->root;

    while (x != t->nil) {
        y = x;
        if (t->compare(z->data, x->data) < 0) x = x->left;
        else x = x->right;
    }

    z->parent = y;
    if (y == t->nil) t->root = z;
    else if (t->compare(z->data, y->data) < 0) y->left = z;
    else y->right = z;

    insert_fixup(t, z);
    t->size++;
}

void stk_rbtree_remove(stk_rbtree* t, void* val) {
    stk_rbnode* z = t->root;
    while (z != t->nil) {
        int cmp = t->compare(val, z->data);
        if (cmp < 0) z = z->left;
        else if (cmp > 0) z = z->right;
        else break;
    }
    if (z == t->nil) return;

    stk_rbnode* y = z;
    stk_rbnode* x = NULL;
    int y_orig_color = y->color;

    if (z->left == t->nil) {
        x = z->right;
        transplant(t, z, z->right);
    } else if (z->right == t->nil) {
        x = z->left;
        transplant(t, z, z->left);
    } else {
        y = minimum(t, z->right);
        y_orig_color = y->color;
        x = y->right;
        if (y->parent == z) {
            x->parent = y;
        } else {
            transplant(t, y, y->right);
            y->right = z->right;
            y->right->parent = y;
        }
        transplant(t, z, y);
        y->left = z->left;
        y->left->parent = y;
        y->color = z->color;
    }

    if (y_orig_color == STK_RB_BLACK) delete_fixup(t, x);
    free(z);
    t->size--;
}

void* stk_rbtree_find(stk_rbtree* t, void* val) {
    stk_rbnode* n = t->root;
    while (n != t->nil) {
        int cmp = t->compare(val, n->data);
        if (cmp < 0) n = n->left;
        else if (cmp > 0) n = n->right;
        else return n->data;
    }
    return NULL;
}

bool stk_rbtree_has(stk_rbtree* t, void* val) {
    stk_rbnode* n = t->root;
    while (n != t->nil) {
        int cmp = t->compare(val, n->data);
        if (cmp < 0) n = n->left;
        else if (cmp > 0) n = n->right;
        else return true;
    }
    return false;
}

bool stk_rbtree_empty(stk_rbtree* t) {
    return t->root == t->nil;
}

size_t stk_rbtree_size(stk_rbtree* t) {
    return t->size;
}

void stk_rbtree_inorder(stk_rbtree* t, void (*visit)(void*)) {
    inorder_rec(t, t->root, visit);
}

void stk_rbtree_preorder(stk_rbtree* t, void (*visit)(void*)) {
    preorder_rec(t, t->root, visit);
}

void stk_rbtree_postorder(stk_rbtree* t, void (*visit)(void*)) {
    postorder_rec(t, t->root, visit);
}

void* stk_rbtree_min(stk_rbtree* t) {
    if (t->root == t->nil) return NULL;
    return minimum(t, t->root)->data;
}

void* stk_rbtree_max(stk_rbtree* t) {
    if (t->root == t->nil) return NULL;
    return maximum(t, t->root)->data;
}

void stk_rbtree_clear(stk_rbtree* t) {
    destroy_nodes(t, t->root);
    t->root = t->nil;
    t->size = 0;
}