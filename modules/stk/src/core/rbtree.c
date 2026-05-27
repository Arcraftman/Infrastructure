// stk_rbtree.c

#include "rbtree.h"

static rbnode* node_new(void* val, rbnode* nil) {
    rbnode* n = (rbnode*)malloc(sizeof(rbnode));
    n->data = val;
    n->color = RB_RED;
    n->left = nil;
    n->right = nil;
    n->parent = nil;
    return n;
}

static void left_rotate(rbtree* t, rbnode* x) {
    rbnode* y = x->right;
    x->right = y->left;
    if (y->left != t->nil) y->left->parent = x;
    y->parent = x->parent;
    if (x->parent == t->nil) t->root = y;
    else if (x == x->parent->left) x->parent->left = y;
    else x->parent->right = y;
    y->left = x;
    x->parent = y;
}

static void right_rotate(rbtree* t, rbnode* y) {
    rbnode* x = y->left;
    y->left = x->right;
    if (x->right != t->nil) x->right->parent = y;
    x->parent = y->parent;
    if (y->parent == t->nil) t->root = x;
    else if (y == y->parent->left) y->parent->left = x;
    else y->parent->right = x;
    x->right = y;
    y->parent = x;
}

static void insert_fixup(rbtree* t, rbnode* z) {
    while (z->parent->color == RB_RED) {
        if (z->parent == z->parent->parent->left) {
            rbnode* y = z->parent->parent->right;
            if (y->color == RB_RED) {
                z->parent->color = RB_BLACK;
                y->color = RB_BLACK;
                z->parent->parent->color = RB_RED;
                z = z->parent->parent;
            } else {
                if (z == z->parent->right) {
                    z = z->parent;
                    left_rotate(t, z);
                }
                z->parent->color = RB_BLACK;
                z->parent->parent->color = RB_RED;
                right_rotate(t, z->parent->parent);
            }
        } else {
            rbnode* y = z->parent->parent->left;
            if (y->color == RB_RED) {
                z->parent->color = RB_BLACK;
                y->color = RB_BLACK;
                z->parent->parent->color = RB_RED;
                z = z->parent->parent;
            } else {
                if (z == z->parent->left) {
                    z = z->parent;
                    right_rotate(t, z);
                }
                z->parent->color = RB_BLACK;
                z->parent->parent->color = RB_RED;
                left_rotate(t, z->parent->parent);
            }
        }
    }
    t->root->color = RB_BLACK;
}

static rbnode* minimum(rbtree* t, rbnode* n) {
    while (n->left != t->nil) n = n->left;
    return n;
}

static rbnode* maximum(rbtree* t, rbnode* n) {
    while (n->right != t->nil) n = n->right;
    return n;
}

static void transplant(rbtree* t, rbnode* u, rbnode* v) {
    if (u->parent == t->nil) t->root = v;
    else if (u == u->parent->left) u->parent->left = v;
    else u->parent->right = v;
    v->parent = u->parent;
}

static void delete_fixup(rbtree* t, rbnode* x) {
    while (x != t->root && x->color == RB_BLACK) {
        if (x == x->parent->left) {
            rbnode* w = x->parent->right;
            if (w->color == RB_RED) {
                w->color = RB_BLACK;
                x->parent->color = RB_RED;
                left_rotate(t, x->parent);
                w = x->parent->right;
            }
            if (w->left->color == RB_BLACK && w->right->color == RB_BLACK) {
                w->color = RB_RED;
                x = x->parent;
            } else {
                if (w->right->color == RB_BLACK) {
                    w->left->color = RB_BLACK;
                    w->color = RB_RED;
                    right_rotate(t, w);
                    w = x->parent->right;
                }
                w->color = x->parent->color;
                x->parent->color = RB_BLACK;
                w->right->color = RB_BLACK;
                left_rotate(t, x->parent);
                x = t->root;
            }
        } else {
            rbnode* w = x->parent->left;
            if (w->color == RB_RED) {
                w->color = RB_BLACK;
                x->parent->color = RB_RED;
                right_rotate(t, x->parent);
                w = x->parent->left;
            }
            if (w->right->color == RB_BLACK && w->left->color == RB_BLACK) {
                w->color = RB_RED;
                x = x->parent;
            } else {
                if (w->left->color == RB_BLACK) {
                    w->right->color = RB_BLACK;
                    w->color = RB_RED;
                    left_rotate(t, w);
                    w = x->parent->left;
                }
                w->color = x->parent->color;
                x->parent->color = RB_BLACK;
                w->left->color = RB_BLACK;
                right_rotate(t, x->parent);
                x = t->root;
            }
        }
    }
    x->color = RB_BLACK;
}

static void destroy_nodes(rbtree* t, rbnode* n) {
    if (n == t->nil) return;
    destroy_nodes(t, n->left);
    destroy_nodes(t, n->right);
    free(n);
}

static void inorder_rec(rbtree* t, rbnode* n, void (*visit)(void*)) {
    if (n == t->nil) return;
    inorder_rec(t, n->left, visit);
    visit(n->data);
    inorder_rec(t, n->right, visit);
}

static void preorder_rec(rbtree* t, rbnode* n, void (*visit)(void*)) {
    if (n == t->nil) return;
    visit(n->data);
    preorder_rec(t, n->left, visit);
    preorder_rec(t, n->right, visit);
}

static void postorder_rec(rbtree* t, rbnode* n, void (*visit)(void*)) {
    if (n == t->nil) return;
    postorder_rec(t, n->left, visit);
    postorder_rec(t, n->right, visit);
    visit(n->data);
}

void rbtree_init(rbtree* t, int (*compare)(const void* a, const void* b)) {
    t->nil = (rbnode*)malloc(sizeof(rbnode));
    t->nil->color = RB_BLACK;
    t->nil->left = t->nil->right = t->nil->parent = t->nil;
    t->root = t->nil;
    t->size = 0;
    t->compare = compare;
}

void rbtree_free(rbtree* t) {
    destroy_nodes(t, t->root);
    free(t->nil);
    t->root = NULL;
    t->nil = NULL;
    t->size = 0;
    t->compare = NULL;
}

void rbtree_insert(rbtree* t, void* val) {
    rbnode* z = node_new(val, t->nil);
    rbnode* y = t->nil;
    rbnode* x = t->root;

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

void rbtree_remove(rbtree* t, void* val) {
    rbnode* z = t->root;
    while (z != t->nil) {
        int cmp = t->compare(val, z->data);
        if (cmp < 0) z = z->left;
        else if (cmp > 0) z = z->right;
        else break;
    }
    if (z == t->nil) return;

    rbnode* y = z;
    rbnode* x = NULL;
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

    if (y_orig_color == RB_BLACK) delete_fixup(t, x);
    free(z);
    t->size--;
}

void* rbtree_find(rbtree* t, void* val) {
    rbnode* n = t->root;
    while (n != t->nil) {
        int cmp = t->compare(val, n->data);
        if (cmp < 0) n = n->left;
        else if (cmp > 0) n = n->right;
        else return n->data;
    }
    return NULL;
}

bool rbtree_has(rbtree* t, void* val) {
    rbnode* n = t->root;
    while (n != t->nil) {
        int cmp = t->compare(val, n->data);
        if (cmp < 0) n = n->left;
        else if (cmp > 0) n = n->right;
        else return true;
    }
    return false;
}

bool rbtree_empty(rbtree* t) {
    return t->root == t->nil;
}

size_t rbtree_size(rbtree* t) {
    return t->size;
}

void rbtree_inorder(rbtree* t, void (*visit)(void*)) {
    inorder_rec(t, t->root, visit);
}

void rbtree_preorder(rbtree* t, void (*visit)(void*)) {
    preorder_rec(t, t->root, visit);
}

void rbtree_postorder(rbtree* t, void (*visit)(void*)) {
    postorder_rec(t, t->root, visit);
}

void* rbtree_min(rbtree* t) {
    if (t->root == t->nil) return NULL;
    return minimum(t, t->root)->data;
}

void* rbtree_max(rbtree* t) {
    if (t->root == t->nil) return NULL;
    return maximum(t, t->root)->data;
}

void rbtree_clear(rbtree* t) {
    destroy_nodes(t, t->root);
    t->root = t->nil;
    t->size = 0;
}