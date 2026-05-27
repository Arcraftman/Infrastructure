
#ifndef STK_SRC_CORE_INTERNAL_RBTREE_H
#define STK_SRC_CORE_INTERNAL_RBTREE_H

#include "config.h"

// ============================================================
// 基础类型红黑树声明（int, double, float 等）- 存储值
// ============================================================
#define RBTREE_DECLARE_BASIC(type, name)                                            \
    typedef struct rbnode_##name {                                                  \
        type data;                                                                  \
        int color;                                                                  \
        struct rbnode_##name* left;                                                 \
        struct rbnode_##name* right;                                                \
        struct rbnode_##name* parent;                                               \
    } rbnode_##name;                                                                \
                                                                                    \
    typedef struct {                                                                \
        rbnode_##name* root;                                                        \
        rbnode_##name* nil;                                                         \
        size_t size;                                                                \
    } rbtree_##name;                                                                \
                                                                                    \
    STK_API void rbtree_##name##_init(rbtree_##name* t);                            \
    STK_API void rbtree_##name##_free(rbtree_##name* t);                            \
    STK_API void rbtree_##name##_insert(rbtree_##name* t, type val);                \
    STK_API void rbtree_##name##_remove(rbtree_##name* t, type val);                \
    STK_API type rbtree_##name##_find(rbtree_##name* t, type val);                  \
    STK_API bool rbtree_##name##_has(rbtree_##name* t, type val);                   \
    STK_API bool rbtree_##name##_empty(rbtree_##name* t);                           \
    STK_API size_t rbtree_##name##_size(rbtree_##name* t);                          \
    STK_API void rbtree_##name##_inorder(rbtree_##name* t, void (*visit)(type));    \
    STK_API void rbtree_##name##_preorder(rbtree_##name* t, void (*visit)(type));   \
    STK_API void rbtree_##name##_postorder(rbtree_##name* t, void (*visit)(type));  \
    STK_API type rbtree_##name##_min(rbtree_##name* t);                             \
    STK_API type rbtree_##name##_max(rbtree_##name* t);                             \
    STK_API void rbtree_##name##_clear(rbtree_##name* t);

// ============================================================
// 结构体类型红黑树声明（str, Person 等）- 存储指针
// ============================================================
#define RBTREE_DECLARE_STRUCT(type, name)                                            \
    typedef struct rbnode_##name {                                                  \
        type* data;                                                                 \
        int color;                                                                  \
        struct rbnode_##name* left;                                                 \
        struct rbnode_##name* right;                                                \
        struct rbnode_##name* parent;                                               \
    } rbnode_##name;                                                                \
                                                                                    \
    typedef struct {                                                                \
        rbnode_##name* root;                                                        \
        rbnode_##name* nil;                                                         \
        size_t size;                                                                \
    } rbtree_##name;                                                                \
                                                                                    \
    STK_API void rbtree_##name##_init(rbtree_##name* t);                            \
    STK_API void rbtree_##name##_free(rbtree_##name* t);                            \
    STK_API void rbtree_##name##_insert(rbtree_##name* t, type* val);               \
    STK_API void rbtree_##name##_remove(rbtree_##name* t, type* val);               \
    STK_API type* rbtree_##name##_find(rbtree_##name* t, type* val);                \
    STK_API bool rbtree_##name##_has(rbtree_##name* t, type* val);                  \
    STK_API bool rbtree_##name##_empty(rbtree_##name* t);                           \
    STK_API size_t rbtree_##name##_size(rbtree_##name* t);                          \
    STK_API void rbtree_##name##_inorder(rbtree_##name* t, void (*visit)(type*));   \
    STK_API void rbtree_##name##_preorder(rbtree_##name* t, void (*visit)(type*));  \
    STK_API void rbtree_##name##_postorder(rbtree_##name* t, void (*visit)(type*)); \
    STK_API type* rbtree_##name##_min(rbtree_##name* t);                            \
    STK_API type* rbtree_##name##_max(rbtree_##name* t);                            \
    STK_API void rbtree_##name##_clear(rbtree_##name* t);

// ============================================================
// 基础类型红黑树实现
// ============================================================
#define RBTREE_IMPLEMENT_BASIC(type, name, cmp)                                     \
    static rbnode_##name* rbtree_##name##_node_new(type val, rbnode_##name* nil) {  \
        rbnode_##name* n = (rbnode_##name*)malloc(sizeof(rbnode_##name));          \
        n->data = val;                                                              \
        n->color = RB_RED;                                                          \
        n->left = nil;                                                              \
        n->right = nil;                                                             \
        n->parent = nil;                                                            \
        return n;                                                                   \
    }                                                                               \
                                                                                    \
    static void rbtree_##name##_left_rotate(rbtree_##name* t, rbnode_##name* x) {   \
        rbnode_##name* y = x->right;                                                \
        x->right = y->left;                                                         \
        if (y->left != t->nil) y->left->parent = x;                                 \
        y->parent = x->parent;                                                      \
        if (x->parent == t->nil) t->root = y;                                       \
        else if (x == x->parent->left) x->parent->left = y;                         \
        else x->parent->right = y;                                                  \
        y->left = x;                                                                \
        x->parent = y;                                                              \
    }                                                                               \
                                                                                    \
    static void rbtree_##name##_right_rotate(rbtree_##name* t, rbnode_##name* y) {  \
        rbnode_##name* x = y->left;                                                 \
        y->left = x->right;                                                         \
        if (x->right != t->nil) x->right->parent = y;                               \
        x->parent = y->parent;                                                      \
        if (y->parent == t->nil) t->root = x;                                       \
        else if (y == y->parent->left) y->parent->left = x;                         \
        else y->parent->right = x;                                                  \
        x->right = y;                                                               \
        y->parent = x;                                                              \
    }                                                                               \
                                                                                    \
    static void rbtree_##name##_insert_fixup(rbtree_##name* t, rbnode_##name* z) {  \
        while (z->parent->color == RB_RED) {                                        \
            if (z->parent == z->parent->parent->left) {                             \
                rbnode_##name* y = z->parent->parent->right;                        \
                if (y->color == RB_RED) {                                           \
                    z->parent->color = RB_BLACK;                                    \
                    y->color = RB_BLACK;                                            \
                    z->parent->parent->color = RB_RED;                              \
                    z = z->parent->parent;                                          \
                } else {                                                            \
                    if (z == z->parent->right) {                                    \
                        z = z->parent;                                              \
                        rbtree_##name##_left_rotate(t, z);                          \
                    }                                                               \
                    z->parent->color = RB_BLACK;                                    \
                    z->parent->parent->color = RB_RED;                              \
                    rbtree_##name##_right_rotate(t, z->parent->parent);             \
                }                                                                   \
            } else {                                                                \
                rbnode_##name* y = z->parent->parent->left;                         \
                if (y->color == RB_RED) {                                           \
                    z->parent->color = RB_BLACK;                                    \
                    y->color = RB_BLACK;                                            \
                    z->parent->parent->color = RB_RED;                              \
                    z = z->parent->parent;                                          \
                } else {                                                            \
                    if (z == z->parent->left) {                                     \
                        z = z->parent;                                              \
                        rbtree_##name##_right_rotate(t, z);                         \
                    }                                                               \
                    z->parent->color = RB_BLACK;                                    \
                    z->parent->parent->color = RB_RED;                              \
                    rbtree_##name##_left_rotate(t, z->parent->parent);              \
                }                                                                   \
            }                                                                       \
        }                                                                           \
        t->root->color = RB_BLACK;                                                  \
    }                                                                               \
                                                                                    \
    static rbnode_##name* rbtree_##name##_minimum(rbtree_##name* t, rbnode_##name* n) { \
        while (n->left != t->nil) n = n->left;                                      \
        return n;                                                                   \
    }                                                                               \
                                                                                    \
    static rbnode_##name* rbtree_##name##_maximum(rbtree_##name* t, rbnode_##name* n) { \
        while (n->right != t->nil) n = n->right;                                    \
        return n;                                                                   \
    }                                                                               \
                                                                                    \
    static void rbtree_##name##_transplant(rbtree_##name* t, rbnode_##name* u, rbnode_##name* v) { \
        if (u->parent == t->nil) t->root = v;                                       \
        else if (u == u->parent->left) u->parent->left = v;                         \
        else u->parent->right = v;                                                  \
        v->parent = u->parent;                                                      \
    }                                                                               \
                                                                                    \
    static void rbtree_##name##_delete_fixup(rbtree_##name* t, rbnode_##name* x) {  \
        while (x != t->root && x->color == RB_BLACK) {                              \
            if (x == x->parent->left) {                                             \
                rbnode_##name* w = x->parent->right;                                \
                if (w->color == RB_RED) {                                           \
                    w->color = RB_BLACK;                                            \
                    x->parent->color = RB_RED;                                      \
                    rbtree_##name##_left_rotate(t, x->parent);                      \
                    w = x->parent->right;                                           \
                }                                                                   \
                if (w->left->color == RB_BLACK && w->right->color == RB_BLACK) {    \
                    w->color = RB_RED;                                              \
                    x = x->parent;                                                  \
                } else {                                                            \
                    if (w->right->color == RB_BLACK) {                              \
                        w->left->color = RB_BLACK;                                  \
                        w->color = RB_RED;                                          \
                        rbtree_##name##_right_rotate(t, w);                         \
                        w = x->parent->right;                                       \
                    }                                                               \
                    w->color = x->parent->color;                                    \
                    x->parent->color = RB_BLACK;                                    \
                    w->right->color = RB_BLACK;                                     \
                    rbtree_##name##_left_rotate(t, x->parent);                      \
                    x = t->root;                                                    \
                }                                                                   \
            } else {                                                                \
                rbnode_##name* w = x->parent->left;                                 \
                if (w->color == RB_RED) {                                           \
                    w->color = RB_BLACK;                                            \
                    x->parent->color = RB_RED;                                      \
                    rbtree_##name##_right_rotate(t, x->parent);                     \
                    w = x->parent->left;                                            \
                }                                                                   \
                if (w->right->color == RB_BLACK && w->left->color == RB_BLACK) {    \
                    w->color = RB_RED;                                              \
                    x = x->parent;                                                  \
                } else {                                                            \
                    if (w->left->color == RB_BLACK) {                               \
                        w->right->color = RB_BLACK;                                 \
                        w->color = RB_RED;                                          \
                        rbtree_##name##_left_rotate(t, w);                          \
                        w = x->parent->left;                                        \
                    }                                                               \
                    w->color = x->parent->color;                                    \
                    x->parent->color = RB_BLACK;                                    \
                    w->left->color = RB_BLACK;                                      \
                    rbtree_##name##_right_rotate(t, x->parent);                     \
                    x = t->root;                                                    \
                }                                                                   \
            }                                                                       \
        }                                                                           \
        x->color = RB_BLACK;                                                        \
    }                                                                               \
                                                                                    \
    static void rbtree_##name##_destroy_nodes(rbtree_##name* t, rbnode_##name* n) { \
        if (n == t->nil) return;                                                    \
        rbtree_##name##_destroy_nodes(t, n->left);                                  \
        rbtree_##name##_destroy_nodes(t, n->right);                                 \
        free(n);                                                                    \
    }                                                                               \
                                                                                    \
    static void rbtree_##name##_inorder_rec(rbtree_##name* t, rbnode_##name* n, void (*visit)(type)) { \
        if (n == t->nil) return;                                                    \
        rbtree_##name##_inorder_rec(t, n->left, visit);                             \
        visit(n->data);                                                             \
        rbtree_##name##_inorder_rec(t, n->right, visit);                            \
    }                                                                               \
                                                                                    \
    static void rbtree_##name##_preorder_rec(rbtree_##name* t, rbnode_##name* n, void (*visit)(type)) { \
        if (n == t->nil) return;                                                    \
        visit(n->data);                                                             \
        rbtree_##name##_preorder_rec(t, n->left, visit);                            \
        rbtree_##name##_preorder_rec(t, n->right, visit);                           \
    }                                                                               \
                                                                                    \
    static void rbtree_##name##_postorder_rec(rbtree_##name* t, rbnode_##name* n, void (*visit)(type)) { \
        if (n == t->nil) return;                                                    \
        rbtree_##name##_postorder_rec(t, n->left, visit);                           \
        rbtree_##name##_postorder_rec(t, n->right, visit);                          \
        visit(n->data);                                                             \
    }                                                                               \
                                                                                    \
    void rbtree_##name##_init(rbtree_##name* t) {                                   \
        t->nil = (rbnode_##name*)malloc(sizeof(rbnode_##name));                     \
        t->nil->color = RB_BLACK;                                                   \
        t->nil->left = t->nil->right = t->nil->parent = t->nil;                     \
        t->root = t->nil;                                                           \
        t->size = 0;                                                                \
    }                                                                               \
                                                                                    \
    void rbtree_##name##_free(rbtree_##name* t) {                                   \
        rbtree_##name##_destroy_nodes(t, t->root);                                  \
        free(t->nil);                                                               \
        t->root = NULL;                                                             \
        t->nil = NULL;                                                              \
        t->size = 0;                                                                \
    }                                                                               \
                                                                                    \
    void rbtree_##name##_insert(rbtree_##name* t, type val) {                       \
        rbnode_##name* z = rbtree_##name##_node_new(val, t->nil);                   \
        rbnode_##name* y = t->nil;                                                  \
        rbnode_##name* x = t->root;                                                 \
                                                                                    \
        while (x != t->nil) {                                                       \
            y = x;                                                                  \
            if (cmp(z->data, x->data) < 0) x = x->left;                             \
            else x = x->right;                                                      \
        }                                                                           \
                                                                                    \
        z->parent = y;                                                              \
        if (y == t->nil) t->root = z;                                               \
        else if (cmp(z->data, y->data) < 0) y->left = z;                            \
        else y->right = z;                                                          \
                                                                                    \
        rbtree_##name##_insert_fixup(t, z);                                         \
        t->size++;                                                                  \
    }                                                                               \
                                                                                    \
    void rbtree_##name##_remove(rbtree_##name* t, type val) {                       \
        rbnode_##name* z = t->root;                                                 \
        while (z != t->nil) {                                                       \
            if (cmp(val, z->data) < 0) z = z->left;                                 \
            else if (cmp(val, z->data) > 0) z = z->right;                           \
            else break;                                                             \
        }                                                                           \
        if (z == t->nil) return;                                                    \
                                                                                    \
        rbnode_##name* y = z;                                                       \
        rbnode_##name* x = NULL;                                                    \
        int y_orig_color = y->color;                                                \
                                                                                    \
        if (z->left == t->nil) {                                                    \
            x = z->right;                                                           \
            rbtree_##name##_transplant(t, z, z->right);                             \
        } else if (z->right == t->nil) {                                            \
            x = z->left;                                                            \
            rbtree_##name##_transplant(t, z, z->left);                              \
        } else {                                                                    \
            y = rbtree_##name##_minimum(t, z->right);                               \
            y_orig_color = y->color;                                                \
            x = y->right;                                                           \
            if (y->parent == z) {                                                   \
                x->parent = y;                                                      \
            } else {                                                                \
                rbtree_##name##_transplant(t, y, y->right);                         \
                y->right = z->right;                                                \
                y->right->parent = y;                                               \
            }                                                                       \
            rbtree_##name##_transplant(t, z, y);                                    \
            y->left = z->left;                                                      \
            y->left->parent = y;                                                    \
            y->color = z->color;                                                    \
        }                                                                           \
                                                                                    \
        if (y_orig_color == RB_BLACK) rbtree_##name##_delete_fixup(t, x);           \
        free(z);                                                                    \
        t->size--;                                                                  \
    }                                                                               \
                                                                                    \
    type rbtree_##name##_find(rbtree_##name* t, type val) {                         \
        rbnode_##name* n = t->root;                                                 \
        while (n != t->nil) {                                                       \
            if (cmp(val, n->data) < 0) n = n->left;                                 \
            else if (cmp(val, n->data) > 0) n = n->right;                           \
            else return n->data;                                                    \
        }                                                                           \
        return (type)0;                                                             \
    }                                                                               \
                                                                                    \
    bool rbtree_##name##_has(rbtree_##name* t, type val) {                          \
        rbnode_##name* n = t->root;                                                 \
        while (n != t->nil) {                                                       \
            if (cmp(val, n->data) < 0) n = n->left;                                 \
            else if (cmp(val, n->data) > 0) n = n->right;                           \
            else return true;                                                       \
        }                                                                           \
        return false;                                                               \
    }                                                                               \
                                                                                    \
    bool rbtree_##name##_empty(rbtree_##name* t) {                                  \
        return t->root == t->nil;                                                   \
    }                                                                               \
                                                                                    \
    size_t rbtree_##name##_size(rbtree_##name* t) {                                 \
        return t->size;                                                             \
    }                                                                               \
                                                                                    \
    void rbtree_##name##_inorder(rbtree_##name* t, void (*visit)(type)) {           \
        rbtree_##name##_inorder_rec(t, t->root, visit);                             \
    }                                                                               \
                                                                                    \
    void rbtree_##name##_preorder(rbtree_##name* t, void (*visit)(type)) {          \
        rbtree_##name##_preorder_rec(t, t->root, visit);                            \
    }                                                                               \
                                                                                    \
    void rbtree_##name##_postorder(rbtree_##name* t, void (*visit)(type)) {         \
        rbtree_##name##_postorder_rec(t, t->root, visit);                           \
    }                                                                               \
                                                                                    \
    type rbtree_##name##_min(rbtree_##name* t) {                                    \
        if (t->root == t->nil) return (type)0;                                      \
        return rbtree_##name##_minimum(t, t->root)->data;                           \
    }                                                                               \
                                                                                    \
    type rbtree_##name##_max(rbtree_##name* t) {                                    \
        if (t->root == t->nil) return (type)0;                                      \
        return rbtree_##name##_maximum(t, t->root)->data;                           \
    }                                                                               \
                                                                                    \
    void rbtree_##name##_clear(rbtree_##name* t) {                                  \
        rbtree_##name##_destroy_nodes(t, t->root);                                  \
        t->root = t->nil;                                                           \
        t->size = 0;                                                                \
    }

// ============================================================
// 结构体类型红黑树实现（存储指针）
// ============================================================
#define RBTREE_IMPLEMENT_STRUCT(type, name, cmp_func)                               \
    static rbnode_##name* rbtree_##name##_node_new(type* val, rbnode_##name* nil) { \
        rbnode_##name* n = (rbnode_##name*)malloc(sizeof(rbnode_##name));          \
        n->data = val;                                                              \
        n->color = RB_RED;                                                          \
        n->left = nil;                                                              \
        n->right = nil;                                                             \
        n->parent = nil;                                                            \
        return n;                                                                   \
    }                                                                               \
                                                                                    \
    static void rbtree_##name##_left_rotate(rbtree_##name* t, rbnode_##name* x) {   \
        rbnode_##name* y = x->right;                                                \
        x->right = y->left;                                                         \
        if (y->left != t->nil) y->left->parent = x;                                 \
        y->parent = x->parent;                                                      \
        if (x->parent == t->nil) t->root = y;                                       \
        else if (x == x->parent->left) x->parent->left = y;                         \
        else x->parent->right = y;                                                  \
        y->left = x;                                                                \
        x->parent = y;                                                              \
    }                                                                               \
                                                                                    \
    static void rbtree_##name##_right_rotate(rbtree_##name* t, rbnode_##name* y) {  \
        rbnode_##name* x = y->left;                                                 \
        y->left = x->right;                                                         \
        if (x->right != t->nil) x->right->parent = y;                               \
        x->parent = y->parent;                                                      \
        if (y->parent == t->nil) t->root = x;                                       \
        else if (y == y->parent->left) y->parent->left = x;                         \
        else y->parent->right = x;                                                  \
        x->right = y;                                                               \
        y->parent = x;                                                              \
    }                                                                               \
                                                                                    \
    static void rbtree_##name##_insert_fixup(rbtree_##name* t, rbnode_##name* z) {  \
        while (z->parent->color == RB_RED) {                                        \
            if (z->parent == z->parent->parent->left) {                             \
                rbnode_##name* y = z->parent->parent->right;                        \
                if (y->color == RB_RED) {                                           \
                    z->parent->color = RB_BLACK;                                    \
                    y->color = RB_BLACK;                                            \
                    z->parent->parent->color = RB_RED;                              \
                    z = z->parent->parent;                                          \
                } else {                                                            \
                    if (z == z->parent->right) {                                    \
                        z = z->parent;                                              \
                        rbtree_##name##_left_rotate(t, z);                          \
                    }                                                               \
                    z->parent->color = RB_BLACK;                                    \
                    z->parent->parent->color = RB_RED;                              \
                    rbtree_##name##_right_rotate(t, z->parent->parent);             \
                }                                                                   \
            } else {                                                                \
                rbnode_##name* y = z->parent->parent->left;                         \
                if (y->color == RB_RED) {                                           \
                    z->parent->color = RB_BLACK;                                    \
                    y->color = RB_BLACK;                                            \
                    z->parent->parent->color = RB_RED;                              \
                    z = z->parent->parent;                                          \
                } else {                                                            \
                    if (z == z->parent->left) {                                     \
                        z = z->parent;                                              \
                        rbtree_##name##_right_rotate(t, z);                         \
                    }                                                               \
                    z->parent->color = RB_BLACK;                                    \
                    z->parent->parent->color = RB_RED;                              \
                    rbtree_##name##_left_rotate(t, z->parent->parent);              \
                }                                                                   \
            }                                                                       \
        }                                                                           \
        t->root->color = RB_BLACK;                                                  \
    }                                                                               \
                                                                                    \
    static rbnode_##name* rbtree_##name##_minimum(rbtree_##name* t, rbnode_##name* n) { \
        while (n->left != t->nil) n = n->left;                                      \
        return n;                                                                   \
    }                                                                               \
                                                                                    \
    static rbnode_##name* rbtree_##name##_maximum(rbtree_##name* t, rbnode_##name* n) { \
        while (n->right != t->nil) n = n->right;                                    \
        return n;                                                                   \
    }                                                                               \
                                                                                    \
    static void rbtree_##name##_transplant(rbtree_##name* t, rbnode_##name* u, rbnode_##name* v) { \
        if (u->parent == t->nil) t->root = v;                                       \
        else if (u == u->parent->left) u->parent->left = v;                         \
        else u->parent->right = v;                                                  \
        v->parent = u->parent;                                                      \
    }                                                                               \
                                                                                    \
    static void rbtree_##name##_delete_fixup(rbtree_##name* t, rbnode_##name* x) {  \
        while (x != t->root && x->color == RB_BLACK) {                              \
            if (x == x->parent->left) {                                             \
                rbnode_##name* w = x->parent->right;                                \
                if (w->color == RB_RED) {                                           \
                    w->color = RB_BLACK;                                            \
                    x->parent->color = RB_RED;                                      \
                    rbtree_##name##_left_rotate(t, x->parent);                      \
                    w = x->parent->right;                                           \
                }                                                                   \
                if (w->left->color == RB_BLACK && w->right->color == RB_BLACK) {    \
                    w->color = RB_RED;                                              \
                    x = x->parent;                                                  \
                } else {                                                            \
                    if (w->right->color == RB_BLACK) {                              \
                        w->left->color = RB_BLACK;                                  \
                        w->color = RB_RED;                                          \
                        rbtree_##name##_right_rotate(t, w);                         \
                        w = x->parent->right;                                       \
                    }                                                               \
                    w->color = x->parent->color;                                    \
                    x->parent->color = RB_BLACK;                                    \
                    w->right->color = RB_BLACK;                                     \
                    rbtree_##name##_left_rotate(t, x->parent);                      \
                    x = t->root;                                                    \
                }                                                                   \
            } else {                                                                \
                rbnode_##name* w = x->parent->left;                                 \
                if (w->color == RB_RED) {                                           \
                    w->color = RB_BLACK;                                            \
                    x->parent->color = RB_RED;                                      \
                    rbtree_##name##_right_rotate(t, x->parent);                     \
                    w = x->parent->left;                                            \
                }                                                                   \
                if (w->right->color == RB_BLACK && w->left->color == RB_BLACK) {    \
                    w->color = RB_RED;                                              \
                    x = x->parent;                                                  \
                } else {                                                            \
                    if (w->left->color == RB_BLACK) {                               \
                        w->right->color = RB_BLACK;                                 \
                        w->color = RB_RED;                                          \
                        rbtree_##name##_left_rotate(t, w);                          \
                        w = x->parent->left;                                        \
                    }                                                               \
                    w->color = x->parent->color;                                    \
                    x->parent->color = RB_BLACK;                                    \
                    w->left->color = RB_BLACK;                                      \
                    rbtree_##name##_right_rotate(t, x->parent);                     \
                    x = t->root;                                                    \
                }                                                                   \
            }                                                                       \
        }                                                                           \
        x->color = RB_BLACK;                                                        \
    }                                                                               \
                                                                                    \
    static void rbtree_##name##_destroy_nodes(rbtree_##name* t, rbnode_##name* n) { \
        if (n == t->nil) return;                                                    \
        rbtree_##name##_destroy_nodes(t, n->left);                                  \
        rbtree_##name##_destroy_nodes(t, n->right);                                 \
        free(n);                                                                    \
    }                                                                               \
                                                                                    \
    static void rbtree_##name##_inorder_rec(rbtree_##name* t, rbnode_##name* n, void (*visit)(type*)) { \
        if (n == t->nil) return;                                                    \
        rbtree_##name##_inorder_rec(t, n->left, visit);                             \
        visit(n->data);                                                             \
        rbtree_##name##_inorder_rec(t, n->right, visit);                            \
    }                                                                               \
                                                                                    \
    static void rbtree_##name##_preorder_rec(rbtree_##name* t, rbnode_##name* n, void (*visit)(type*)) { \
        if (n == t->nil) return;                                                    \
        visit(n->data);                                                             \
        rbtree_##name##_preorder_rec(t, n->left, visit);                            \
        rbtree_##name##_preorder_rec(t, n->right, visit);                           \
    }                                                                               \
                                                                                    \
    static void rbtree_##name##_postorder_rec(rbtree_##name* t, rbnode_##name* n, void (*visit)(type*)) { \
        if (n == t->nil) return;                                                    \
        rbtree_##name##_postorder_rec(t, n->left, visit);                           \
        rbtree_##name##_postorder_rec(t, n->right, visit);                          \
        visit(n->data);                                                             \
    }                                                                               \
                                                                                    \
    void rbtree_##name##_init(rbtree_##name* t) {                                   \
        t->nil = (rbnode_##name*)malloc(sizeof(rbnode_##name));                     \
        t->nil->color = RB_BLACK;                                                   \
        t->nil->left = t->nil->right = t->nil->parent = t->nil;                     \
        t->root = t->nil;                                                           \
        t->size = 0;                                                                \
    }                                                                               \
                                                                                    \
    void rbtree_##name##_free(rbtree_##name* t) {                                   \
        rbtree_##name##_destroy_nodes(t, t->root);                                  \
        free(t->nil);                                                               \
        t->root = NULL;                                                             \
        t->nil = NULL;                                                              \
        t->size = 0;                                                                \
    }                                                                               \
                                                                                    \
    void rbtree_##name##_insert(rbtree_##name* t, type* val) {                      \
        rbnode_##name* z = rbtree_##name##_node_new(val, t->nil);                   \
        rbnode_##name* y = t->nil;                                                  \
        rbnode_##name* x = t->root;                                                 \
                                                                                    \
        while (x != t->nil) {                                                       \
            y = x;                                                                  \
            if (cmp_func(z->data, x->data) < 0) x = x->left;                        \
            else x = x->right;                                                      \
        }                                                                           \
                                                                                    \
        z->parent = y;                                                              \
        if (y == t->nil) t->root = z;                                               \
        else if (cmp_func(z->data, y->data) < 0) y->left = z;                       \
        else y->right = z;                                                          \
                                                                                    \
        rbtree_##name##_insert_fixup(t, z);                                         \
        t->size++;                                                                  \
    }                                                                               \
                                                                                    \
    void rbtree_##name##_remove(rbtree_##name* t, type* val) {                      \
        rbnode_##name* z = t->root;                                                 \
        while (z != t->nil) {                                                       \
            if (cmp_func(val, z->data) < 0) z = z->left;                            \
            else if (cmp_func(val, z->data) > 0) z = z->right;                      \
            else break;                                                             \
        }                                                                           \
        if (z == t->nil) return;                                                    \
                                                                                    \
        rbnode_##name* y = z;                                                       \
        rbnode_##name* x = NULL;                                                    \
        int y_orig_color = y->color;                                                \
                                                                                    \
        if (z->left == t->nil) {                                                    \
            x = z->right;                                                           \
            rbtree_##name##_transplant(t, z, z->right);                             \
        } else if (z->right == t->nil) {                                            \
            x = z->left;                                                            \
            rbtree_##name##_transplant(t, z, z->left);                              \
        } else {                                                                    \
            y = rbtree_##name##_minimum(t, z->right);                               \
            y_orig_color = y->color;                                                \
            x = y->right;                                                           \
            if (y->parent == z) {                                                   \
                x->parent = y;                                                      \
            } else {                                                                \
                rbtree_##name##_transplant(t, y, y->right);                         \
                y->right = z->right;                                                \
                y->right->parent = y;                                               \
            }                                                                       \
            rbtree_##name##_transplant(t, z, y);                                    \
            y->left = z->left;                                                      \
            y->left->parent = y;                                                    \
            y->color = z->color;                                                    \
        }                                                                           \
                                                                                    \
        if (y_orig_color == RB_BLACK) rbtree_##name##_delete_fixup(t, x);           \
        free(z);                                                                    \
        t->size--;                                                                  \
    }                                                                               \
                                                                                    \
    type* rbtree_##name##_find(rbtree_##name* t, type* val) {                       \
        rbnode_##name* n = t->root;                                                 \
        while (n != t->nil) {                                                       \
            if (cmp_func(val, n->data) < 0) n = n->left;                            \
            else if (cmp_func(val, n->data) > 0) n = n->right;                      \
            else return n->data;                                                    \
        }                                                                           \
        return NULL;                                                                \
    }                                                                               \
                                                                                    \
    bool rbtree_##name##_has(rbtree_##name* t, type* val) {                         \
        rbnode_##name* n = t->root;                                                 \
        while (n != t->nil) {                                                       \
            if (cmp_func(val, n->data) < 0) n = n->left;                            \
            else if (cmp_func(val, n->data) > 0) n = n->right;                      \
            else return true;                                                       \
        }                                                                           \
        return false;                                                               \
    }                                                                               \
                                                                                    \
    bool rbtree_##name##_empty(rbtree_##name* t) {                                  \
        return t->root == t->nil;                                                   \
    }                                                                               \
                                                                                    \
    size_t rbtree_##name##_size(rbtree_##name* t) {                                 \
        return t->size;                                                             \
    }                                                                               \
                                                                                    \
    void rbtree_##name##_inorder(rbtree_##name* t, void (*visit)(type*)) {          \
        rbtree_##name##_inorder_rec(t, t->root, visit);                             \
    }                                                                               \
                                                                                    \
    void rbtree_##name##_preorder(rbtree_##name* t, void (*visit)(type*)) {         \
        rbtree_##name##_preorder_rec(t, t->root, visit);                            \
    }                                                                               \
                                                                                    \
    void rbtree_##name##_postorder(rbtree_##name* t, void (*visit)(type*)) {        \
        rbtree_##name##_postorder_rec(t, t->root, visit);                           \
    }                                                                               \
                                                                                    \
    type* rbtree_##name##_min(rbtree_##name* t) {                                   \
        if (t->root == t->nil) return NULL;                                         \
        return rbtree_##name##_minimum(t, t->root)->data;                           \
    }                                                                               \
                                                                                    \
    type* rbtree_##name##_max(rbtree_##name* t) {                                   \
        if (t->root == t->nil) return NULL;                                         \
        return rbtree_##name##_maximum(t, t->root)->data;                           \
    }                                                                               \
                                                                                    \
    void rbtree_##name##_clear(rbtree_##name* t) {                                  \
        rbtree_##name##_destroy_nodes(t, t->root);                                  \
        t->root = t->nil;                                                           \
        t->size = 0;                                                                \
    }

#endif // STK_SRC_CORE_RBTREE_FACTORY_H