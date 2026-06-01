#ifndef STK_DETAIL_RBTREE_H
#define STK_DETAIL_RBTREE_H


// ============================================================
// Basic type red-black tree declaration (int, double, float, etc.) - stores values
// ============================================================
#define RBTREE_DECLARE_BASIC(type, name)                                                   \
    typedef struct stk_rbnode_##name {                                                     \
        type data;                                                                         \
        int color;                                                                         \
        struct stk_rbnode_##name* left;                                                    \
        struct stk_rbnode_##name* right;                                                   \
        struct stk_rbnode_##name* parent;                                                  \
    } stk_rbnode_##name;                                                                   \
                                                                                           \
    typedef struct {                                                                       \
        stk_rbnode_##name* root;                                                           \
        stk_rbnode_##name* nil;                                                            \
        size_t size;                                                                       \
    } stk_rbtree_##name;                                                                   \
                                                                                           \
    STK_API void stk_rbtree_##name##_init(stk_rbtree_##name* t);                           \
    STK_API void stk_rbtree_##name##_free(stk_rbtree_##name* t);                           \
    STK_API void stk_rbtree_##name##_insert(stk_rbtree_##name* t, type val);               \
    STK_API void stk_rbtree_##name##_remove(stk_rbtree_##name* t, type val);               \
    STK_API type stk_rbtree_##name##_find(stk_rbtree_##name* t, type val);                 \
    STK_API bool stk_rbtree_##name##_has(stk_rbtree_##name* t, type val);                  \
    STK_API bool stk_rbtree_##name##_empty(stk_rbtree_##name* t);                          \
    STK_API size_t stk_rbtree_##name##_size(stk_rbtree_##name* t);                         \
    STK_API void stk_rbtree_##name##_inorder(stk_rbtree_##name* t, void (*visit)(type));   \
    STK_API void stk_rbtree_##name##_preorder(stk_rbtree_##name* t, void (*visit)(type));  \
    STK_API void stk_rbtree_##name##_postorder(stk_rbtree_##name* t, void (*visit)(type)); \
    STK_API type stk_rbtree_##name##_min(stk_rbtree_##name* t);                            \
    STK_API type stk_rbtree_##name##_max(stk_rbtree_##name* t);                            \
    STK_API void stk_rbtree_##name##_clear(stk_rbtree_##name* t);

// ============================================================
// Struct type red-black tree declaration (str, Person, etc.) - stores pointers
// ============================================================
#define RBTREE_DECLARE_STRUCT(type, name)                                                   \
    typedef struct stk_rbnode_##name {                                                      \
        type* data;                                                                         \
        int color;                                                                          \
        struct stk_rbnode_##name* left;                                                     \
        struct stk_rbnode_##name* right;                                                    \
        struct stk_rbnode_##name* parent;                                                   \
    } stk_rbnode_##name;                                                                    \
                                                                                            \
    typedef struct {                                                                        \
        stk_rbnode_##name* root;                                                            \
        stk_rbnode_##name* nil;                                                             \
        size_t size;                                                                        \
    } stk_rbtree_##name;                                                                    \
                                                                                            \
    STK_API void stk_rbtree_##name##_init(stk_rbtree_##name* t);                            \
    STK_API void stk_rbtree_##name##_free(stk_rbtree_##name* t);                            \
    STK_API void stk_rbtree_##name##_insert(stk_rbtree_##name* t, type* val);               \
    STK_API void stk_rbtree_##name##_remove(stk_rbtree_##name* t, type* val);               \
    STK_API type* stk_rbtree_##name##_find(stk_rbtree_##name* t, type* val);                \
    STK_API bool stk_rbtree_##name##_has(stk_rbtree_##name* t, type* val);                  \
    STK_API bool stk_rbtree_##name##_empty(stk_rbtree_##name* t);                           \
    STK_API size_t stk_rbtree_##name##_size(stk_rbtree_##name* t);                          \
    STK_API void stk_rbtree_##name##_inorder(stk_rbtree_##name* t, void (*visit)(type*));   \
    STK_API void stk_rbtree_##name##_preorder(stk_rbtree_##name* t, void (*visit)(type*));  \
    STK_API void stk_rbtree_##name##_postorder(stk_rbtree_##name* t, void (*visit)(type*)); \
    STK_API type* stk_rbtree_##name##_min(stk_rbtree_##name* t);                            \
    STK_API type* stk_rbtree_##name##_max(stk_rbtree_##name* t);                            \
    STK_API void stk_rbtree_##name##_clear(stk_rbtree_##name* t);

// ============================================================
// Basic type red-black tree implementation
// ============================================================
#define RBTREE_IMPLEMENT_BASIC(type, name, cmp)                                               \
    static stk_rbnode_##name* stk_rbtree_##name##_node_new(type val, stk_rbnode_##name* nil)  \
    {                                                                                         \
        stk_rbnode_##name* n = (stk_rbnode_##name*)malloc(sizeof(stk_rbnode_##name));         \
        n->data = val;                                                                        \
        n->color = STK_RB_RED;                                                                    \
        n->left = nil;                                                                        \
        n->right = nil;                                                                       \
        n->parent = nil;                                                                      \
        return n;                                                                             \
    }                                                                                         \
                                                                                              \
    static void stk_rbtree_##name##_left_rotate(stk_rbtree_##name* t, stk_rbnode_##name* x)   \
    {                                                                                         \
        stk_rbnode_##name* y = x->right;                                                      \
        x->right = y->left;                                                                   \
        if (y->left != t->nil)                                                                \
            y->left->parent = x;                                                              \
        y->parent = x->parent;                                                                \
        if (x->parent == t->nil)                                                              \
            t->root = y;                                                                      \
        else if (x == x->parent->left)                                                        \
            x->parent->left = y;                                                              \
        else                                                                                  \
            x->parent->right = y;                                                             \
        y->left = x;                                                                          \
        x->parent = y;                                                                        \
    }                                                                                         \
                                                                                              \
    static void stk_rbtree_##name##_right_rotate(stk_rbtree_##name* t, stk_rbnode_##name* y)  \
    {                                                                                         \
        stk_rbnode_##name* x = y->left;                                                       \
        y->left = x->right;                                                                   \
        if (x->right != t->nil)                                                               \
            x->right->parent = y;                                                             \
        x->parent = y->parent;                                                                \
        if (y->parent == t->nil)                                                              \
            t->root = x;                                                                      \
        else if (y == y->parent->left)                                                        \
            y->parent->left = x;                                                              \
        else                                                                                  \
            y->parent->right = x;                                                             \
        x->right = y;                                                                         \
        y->parent = x;                                                                        \
    }                                                                                         \
                                                                                              \
    static void stk_rbtree_##name##_insert_fixup(stk_rbtree_##name* t, stk_rbnode_##name* z)  \
    {                                                                                         \
        while (z->parent->color == STK_RB_RED) {                                                  \
            if (z->parent == z->parent->parent->left) {                                       \
                stk_rbnode_##name* y = z->parent->parent->right;                              \
                if (y->color == STK_RB_RED) {                                                     \
                    z->parent->color = STK_RB_BLACK;                                              \
                    y->color = STK_RB_BLACK;                                                      \
                    z->parent->parent->color = STK_RB_RED;                                        \
                    z = z->parent->parent;                                                    \
                } else {                                                                      \
                    if (z == z->parent->right) {                                              \
                        z = z->parent;                                                        \
                        stk_rbtree_##name##_left_rotate(t, z);                                \
                    }                                                                         \
                    z->parent->color = STK_RB_BLACK;                                              \
                    z->parent->parent->color = STK_RB_RED;                                        \
                    stk_rbtree_##name##_right_rotate(t, z->parent->parent);                   \
                }                                                                             \
            } else {                                                                          \
                stk_rbnode_##name* y = z->parent->parent->left;                               \
                if (y->color == STK_RB_RED) {                                                     \
                    z->parent->color = STK_RB_BLACK;                                              \
                    y->color = STK_RB_BLACK;                                                      \
                    z->parent->parent->color = STK_RB_RED;                                        \
                    z = z->parent->parent;                                                    \
                } else {                                                                      \
                    if (z == z->parent->left) {                                               \
                        z = z->parent;                                                        \
                        stk_rbtree_##name##_right_rotate(t, z);                               \
                    }                                                                         \
                    z->parent->color = STK_RB_BLACK;                                              \
                    z->parent->parent->color = STK_RB_RED;                                        \
                    stk_rbtree_##name##_left_rotate(t, z->parent->parent);                    \
                }                                                                             \
            }                                                                                 \
        }                                                                                     \
        t->root->color = STK_RB_BLACK;                                                            \
    }                                                                                         \
                                                                                              \
    static stk_rbnode_##name* stk_rbtree_##name##_minimum(stk_rbtree_##name* t,               \
                                                          stk_rbnode_##name* n)               \
    {                                                                                         \
        while (n->left != t->nil)                                                             \
            n = n->left;                                                                      \
        return n;                                                                             \
    }                                                                                         \
                                                                                              \
    static stk_rbnode_##name* stk_rbtree_##name##_maximum(stk_rbtree_##name* t,               \
                                                          stk_rbnode_##name* n)               \
    {                                                                                         \
        while (n->right != t->nil)                                                            \
            n = n->right;                                                                     \
        return n;                                                                             \
    }                                                                                         \
                                                                                              \
    static void stk_rbtree_##name##_transplant(stk_rbtree_##name* t,                          \
                                               stk_rbnode_##name* u,                          \
                                               stk_rbnode_##name* v)                          \
    {                                                                                         \
        if (u->parent == t->nil)                                                              \
            t->root = v;                                                                      \
        else if (u == u->parent->left)                                                        \
            u->parent->left = v;                                                              \
        else                                                                                  \
            u->parent->right = v;                                                             \
        v->parent = u->parent;                                                                \
    }                                                                                         \
                                                                                              \
    static void stk_rbtree_##name##_delete_fixup(stk_rbtree_##name* t, stk_rbnode_##name* x)  \
    {                                                                                         \
        while (x != t->root && x->color == STK_RB_BLACK) {                                        \
            if (x == x->parent->left) {                                                       \
                stk_rbnode_##name* w = x->parent->right;                                      \
                if (w->color == STK_RB_RED) {                                                     \
                    w->color = STK_RB_BLACK;                                                      \
                    x->parent->color = STK_RB_RED;                                                \
                    stk_rbtree_##name##_left_rotate(t, x->parent);                            \
                    w = x->parent->right;                                                     \
                }                                                                             \
                if (w->left->color == STK_RB_BLACK && w->right->color == STK_RB_BLACK) {              \
                    w->color = STK_RB_RED;                                                        \
                    x = x->parent;                                                            \
                } else {                                                                      \
                    if (w->right->color == STK_RB_BLACK) {                                        \
                        w->left->color = STK_RB_BLACK;                                            \
                        w->color = STK_RB_RED;                                                    \
                        stk_rbtree_##name##_right_rotate(t, w);                               \
                        w = x->parent->right;                                                 \
                    }                                                                         \
                    w->color = x->parent->color;                                              \
                    x->parent->color = STK_RB_BLACK;                                              \
                    w->right->color = STK_RB_BLACK;                                               \
                    stk_rbtree_##name##_left_rotate(t, x->parent);                            \
                    x = t->root;                                                              \
                }                                                                             \
            } else {                                                                          \
                stk_rbnode_##name* w = x->parent->left;                                       \
                if (w->color == STK_RB_RED) {                                                     \
                    w->color = STK_RB_BLACK;                                                      \
                    x->parent->color = STK_RB_RED;                                                \
                    stk_rbtree_##name##_right_rotate(t, x->parent);                           \
                    w = x->parent->left;                                                      \
                }                                                                             \
                if (w->right->color == STK_RB_BLACK && w->left->color == STK_RB_BLACK) {              \
                    w->color = STK_RB_RED;                                                        \
                    x = x->parent;                                                            \
                } else {                                                                      \
                    if (w->left->color == STK_RB_BLACK) {                                         \
                        w->right->color = STK_RB_BLACK;                                           \
                        w->color = STK_RB_RED;                                                    \
                        stk_rbtree_##name##_left_rotate(t, w);                                \
                        w = x->parent->left;                                                  \
                    }                                                                         \
                    w->color = x->parent->color;                                              \
                    x->parent->color = STK_RB_BLACK;                                              \
                    w->left->color = STK_RB_BLACK;                                                \
                    stk_rbtree_##name##_right_rotate(t, x->parent);                           \
                    x = t->root;                                                              \
                }                                                                             \
            }                                                                                 \
        }                                                                                     \
        x->color = STK_RB_BLACK;                                                                  \
    }                                                                                         \
                                                                                              \
    static void stk_rbtree_##name##_destroy_nodes(stk_rbtree_##name* t, stk_rbnode_##name* n) \
    {                                                                                         \
        if (n == t->nil)                                                                      \
            return;                                                                           \
        stk_rbtree_##name##_destroy_nodes(t, n->left);                                        \
        stk_rbtree_##name##_destroy_nodes(t, n->right);                                       \
        free(n);                                                                              \
    }                                                                                         \
                                                                                              \
    static void stk_rbtree_##name##_inorder_rec(stk_rbtree_##name* t,                         \
                                                stk_rbnode_##name* n,                         \
                                                void (*visit)(type))                          \
    {                                                                                         \
        if (n == t->nil)                                                                      \
            return;                                                                           \
        stk_rbtree_##name##_inorder_rec(t, n->left, visit);                                   \
        visit(n->data);                                                                       \
        stk_rbtree_##name##_inorder_rec(t, n->right, visit);                                  \
    }                                                                                         \
                                                                                              \
    static void stk_rbtree_##name##_preorder_rec(stk_rbtree_##name* t,                        \
                                                 stk_rbnode_##name* n,                        \
                                                 void (*visit)(type))                         \
    {                                                                                         \
        if (n == t->nil)                                                                      \
            return;                                                                           \
        visit(n->data);                                                                       \
        stk_rbtree_##name##_preorder_rec(t, n->left, visit);                                  \
        stk_rbtree_##name##_preorder_rec(t, n->right, visit);                                 \
    }                                                                                         \
                                                                                              \
    static void stk_rbtree_##name##_postorder_rec(stk_rbtree_##name* t,                       \
                                                  stk_rbnode_##name* n,                       \
                                                  void (*visit)(type))                        \
    {                                                                                         \
        if (n == t->nil)                                                                      \
            return;                                                                           \
        stk_rbtree_##name##_postorder_rec(t, n->left, visit);                                 \
        stk_rbtree_##name##_postorder_rec(t, n->right, visit);                                \
        visit(n->data);                                                                       \
    }                                                                                         \
                                                                                              \
    void stk_rbtree_##name##_init(stk_rbtree_##name* t)                                       \
    {                                                                                         \
        t->nil = (stk_rbnode_##name*)malloc(sizeof(stk_rbnode_##name));                       \
        t->nil->color = STK_RB_BLACK;                                                             \
        t->nil->left = t->nil->right = t->nil->parent = t->nil;                               \
        t->root = t->nil;                                                                     \
        t->size = 0;                                                                          \
    }                                                                                         \
                                                                                              \
    void stk_rbtree_##name##_free(stk_rbtree_##name* t)                                       \
    {                                                                                         \
        stk_rbtree_##name##_destroy_nodes(t, t->root);                                        \
        free(t->nil);                                                                         \
        t->root = NULL;                                                                       \
        t->nil = NULL;                                                                        \
        t->size = 0;                                                                          \
    }                                                                                         \
                                                                                              \
    void stk_rbtree_##name##_insert(stk_rbtree_##name* t, type val)                           \
    {                                                                                         \
        stk_rbnode_##name* z = stk_rbtree_##name##_node_new(val, t->nil);                     \
        stk_rbnode_##name* y = t->nil;                                                        \
        stk_rbnode_##name* x = t->root;                                                       \
                                                                                              \
        while (x != t->nil) {                                                                 \
            y = x;                                                                            \
            if (cmp(z->data, x->data) < 0)                                                    \
                x = x->left;                                                                  \
            else                                                                              \
                x = x->right;                                                                 \
        }                                                                                     \
                                                                                              \
        z->parent = y;                                                                        \
        if (y == t->nil)                                                                      \
            t->root = z;                                                                      \
        else if (cmp(z->data, y->data) < 0)                                                   \
            y->left = z;                                                                      \
        else                                                                                  \
            y->right = z;                                                                     \
                                                                                              \
        stk_rbtree_##name##_insert_fixup(t, z);                                               \
        t->size++;                                                                            \
    }                                                                                         \
                                                                                              \
    void stk_rbtree_##name##_remove(stk_rbtree_##name* t, type val)                           \
    {                                                                                         \
        stk_rbnode_##name* z = t->root;                                                       \
        while (z != t->nil) {                                                                 \
            if (cmp(val, z->data) < 0)                                                        \
                z = z->left;                                                                  \
            else if (cmp(val, z->data) > 0)                                                   \
                z = z->right;                                                                 \
            else                                                                              \
                break;                                                                        \
        }                                                                                     \
        if (z == t->nil)                                                                      \
            return;                                                                           \
                                                                                              \
        stk_rbnode_##name* y = z;                                                             \
        stk_rbnode_##name* x = NULL;                                                          \
        int y_orig_color = y->color;                                                          \
                                                                                              \
        if (z->left == t->nil) {                                                              \
            x = z->right;                                                                     \
            stk_rbtree_##name##_transplant(t, z, z->right);                                   \
        } else if (z->right == t->nil) {                                                      \
            x = z->left;                                                                      \
            stk_rbtree_##name##_transplant(t, z, z->left);                                    \
        } else {                                                                              \
            y = stk_rbtree_##name##_minimum(t, z->right);                                     \
            y_orig_color = y->color;                                                          \
            x = y->right;                                                                     \
            if (y->parent == z) {                                                             \
                x->parent = y;                                                                \
            } else {                                                                          \
                stk_rbtree_##name##_transplant(t, y, y->right);                               \
                y->right = z->right;                                                          \
                y->right->parent = y;                                                         \
            }                                                                                 \
            stk_rbtree_##name##_transplant(t, z, y);                                          \
            y->left = z->left;                                                                \
            y->left->parent = y;                                                              \
            y->color = z->color;                                                              \
        }                                                                                     \
                                                                                              \
        if (y_orig_color == STK_RB_BLACK)                                                         \
            stk_rbtree_##name##_delete_fixup(t, x);                                           \
        free(z);                                                                              \
        t->size--;                                                                            \
    }                                                                                         \
                                                                                              \
    type stk_rbtree_##name##_find(stk_rbtree_##name* t, type val)                             \
    {                                                                                         \
        stk_rbnode_##name* n = t->root;                                                       \
        while (n != t->nil) {                                                                 \
            if (cmp(val, n->data) < 0)                                                        \
                n = n->left;                                                                  \
            else if (cmp(val, n->data) > 0)                                                   \
                n = n->right;                                                                 \
            else                                                                              \
                return n->data;                                                               \
        }                                                                                     \
        return (type)0;                                                                       \
    }                                                                                         \
                                                                                              \
    bool stk_rbtree_##name##_has(stk_rbtree_##name* t, type val)                              \
    {                                                                                         \
        stk_rbnode_##name* n = t->root;                                                       \
        while (n != t->nil) {                                                                 \
            if (cmp(val, n->data) < 0)                                                        \
                n = n->left;                                                                  \
            else if (cmp(val, n->data) > 0)                                                   \
                n = n->right;                                                                 \
            else                                                                              \
                return true;                                                                  \
        }                                                                                     \
        return false;                                                                         \
    }                                                                                         \
                                                                                              \
    bool stk_rbtree_##name##_empty(stk_rbtree_##name* t)                                      \
    {                                                                                         \
        return t->root == t->nil;                                                             \
    }                                                                                         \
                                                                                              \
    size_t stk_rbtree_##name##_size(stk_rbtree_##name* t)                                     \
    {                                                                                         \
        return t->size;                                                                       \
    }                                                                                         \
                                                                                              \
    void stk_rbtree_##name##_inorder(stk_rbtree_##name* t, void (*visit)(type))               \
    {                                                                                         \
        stk_rbtree_##name##_inorder_rec(t, t->root, visit);                                   \
    }                                                                                         \
                                                                                              \
    void stk_rbtree_##name##_preorder(stk_rbtree_##name* t, void (*visit)(type))              \
    {                                                                                         \
        stk_rbtree_##name##_preorder_rec(t, t->root, visit);                                  \
    }                                                                                         \
                                                                                              \
    void stk_rbtree_##name##_postorder(stk_rbtree_##name* t, void (*visit)(type))             \
    {                                                                                         \
        stk_rbtree_##name##_postorder_rec(t, t->root, visit);                                 \
    }                                                                                         \
                                                                                              \
    type stk_rbtree_##name##_min(stk_rbtree_##name* t)                                        \
    {                                                                                         \
        if (t->root == t->nil)                                                                \
            return (type)0;                                                                   \
        return stk_rbtree_##name##_minimum(t, t->root)->data;                                 \
    }                                                                                         \
                                                                                              \
    type stk_rbtree_##name##_max(stk_rbtree_##name* t)                                        \
    {                                                                                         \
        if (t->root == t->nil)                                                                \
            return (type)0;                                                                   \
        return stk_rbtree_##name##_maximum(t, t->root)->data;                                 \
    }                                                                                         \
                                                                                              \
    void stk_rbtree_##name##_clear(stk_rbtree_##name* t)                                      \
    {                                                                                         \
        stk_rbtree_##name##_destroy_nodes(t, t->root);                                        \
        t->root = t->nil;                                                                     \
        t->size = 0;                                                                          \
    }

// ============================================================
// Struct type red-black tree implementation (stores pointers)
// ============================================================
#define RBTREE_IMPLEMENT_STRUCT(type, name, cmp_func)                                         \
    static stk_rbnode_##name* stk_rbtree_##name##_node_new(type* val, stk_rbnode_##name* nil) \
    {                                                                                         \
        stk_rbnode_##name* n = (stk_rbnode_##name*)malloc(sizeof(stk_rbnode_##name));         \
        n->data = val;                                                                        \
        n->color = STK_RB_RED;                                                                    \
        n->left = nil;                                                                        \
        n->right = nil;                                                                       \
        n->parent = nil;                                                                      \
        return n;                                                                             \
    }                                                                                         \
                                                                                              \
    static void stk_rbtree_##name##_left_rotate(stk_rbtree_##name* t, stk_rbnode_##name* x)   \
    {                                                                                         \
        stk_rbnode_##name* y = x->right;                                                      \
        x->right = y->left;                                                                   \
        if (y->left != t->nil)                                                                \
            y->left->parent = x;                                                              \
        y->parent = x->parent;                                                                \
        if (x->parent == t->nil)                                                              \
            t->root = y;                                                                      \
        else if (x == x->parent->left)                                                        \
            x->parent->left = y;                                                              \
        else                                                                                  \
            x->parent->right = y;                                                             \
        y->left = x;                                                                          \
        x->parent = y;                                                                        \
    }                                                                                         \
                                                                                              \
    static void stk_rbtree_##name##_right_rotate(stk_rbtree_##name* t, stk_rbnode_##name* y)  \
    {                                                                                         \
        stk_rbnode_##name* x = y->left;                                                       \
        y->left = x->right;                                                                   \
        if (x->right != t->nil)                                                               \
            x->right->parent = y;                                                             \
        x->parent = y->parent;                                                                \
        if (y->parent == t->nil)                                                              \
            t->root = x;                                                                      \
        else if (y == y->parent->left)                                                        \
            y->parent->left = x;                                                              \
        else                                                                                  \
            y->parent->right = x;                                                             \
        x->right = y;                                                                         \
        y->parent = x;                                                                        \
    }                                                                                         \
                                                                                              \
    static void stk_rbtree_##name##_insert_fixup(stk_rbtree_##name* t, stk_rbnode_##name* z)  \
    {                                                                                         \
        while (z->parent->color == STK_RB_RED) {                                                  \
            if (z->parent == z->parent->parent->left) {                                       \
                stk_rbnode_##name* y = z->parent->parent->right;                              \
                if (y->color == STK_RB_RED) {                                                     \
                    z->parent->color = STK_RB_BLACK;                                              \
                    y->color = STK_RB_BLACK;                                                      \
                    z->parent->parent->color = STK_RB_RED;                                        \
                    z = z->parent->parent;                                                    \
                } else {                                                                      \
                    if (z == z->parent->right) {                                              \
                        z = z->parent;                                                        \
                        stk_rbtree_##name##_left_rotate(t, z);                                \
                    }                                                                         \
                    z->parent->color = STK_RB_BLACK;                                              \
                    z->parent->parent->color = STK_RB_RED;                                        \
                    stk_rbtree_##name##_right_rotate(t, z->parent->parent);                   \
                }                                                                             \
            } else {                                                                          \
                stk_rbnode_##name* y = z->parent->parent->left;                               \
                if (y->color == STK_RB_RED) {                                                     \
                    z->parent->color = STK_RB_BLACK;                                              \
                    y->color = STK_RB_BLACK;                                                      \
                    z->parent->parent->color = STK_RB_RED;                                        \
                    z = z->parent->parent;                                                    \
                } else {                                                                      \
                    if (z == z->parent->left) {                                               \
                        z = z->parent;                                                        \
                        stk_rbtree_##name##_right_rotate(t, z);                               \
                    }                                                                         \
                    z->parent->color = STK_RB_BLACK;                                              \
                    z->parent->parent->color = STK_RB_RED;                                        \
                    stk_rbtree_##name##_left_rotate(t, z->parent->parent);                    \
                }                                                                             \
            }                                                                                 \
        }                                                                                     \
        t->root->color = STK_RB_BLACK;                                                            \
    }                                                                                         \
                                                                                              \
    static stk_rbnode_##name* stk_rbtree_##name##_minimum(stk_rbtree_##name* t,               \
                                                          stk_rbnode_##name* n)               \
    {                                                                                         \
        while (n->left != t->nil)                                                             \
            n = n->left;                                                                      \
        return n;                                                                             \
    }                                                                                         \
                                                                                              \
    static stk_rbnode_##name* stk_rbtree_##name##_maximum(stk_rbtree_##name* t,               \
                                                          stk_rbnode_##name* n)               \
    {                                                                                         \
        while (n->right != t->nil)                                                            \
            n = n->right;                                                                     \
        return n;                                                                             \
    }                                                                                         \
                                                                                              \
    static void stk_rbtree_##name##_transplant(stk_rbtree_##name* t,                          \
                                               stk_rbnode_##name* u,                          \
                                               stk_rbnode_##name* v)                          \
    {                                                                                         \
        if (u->parent == t->nil)                                                              \
            t->root = v;                                                                      \
        else if (u == u->parent->left)                                                        \
            u->parent->left = v;                                                              \
        else                                                                                  \
            u->parent->right = v;                                                             \
        v->parent = u->parent;                                                                \
    }                                                                                         \
                                                                                              \
    static void stk_rbtree_##name##_delete_fixup(stk_rbtree_##name* t, stk_rbnode_##name* x)  \
    {                                                                                         \
        while (x != t->root && x->color == STK_RB_BLACK) {                                        \
            if (x == x->parent->left) {                                                       \
                stk_rbnode_##name* w = x->parent->right;                                      \
                if (w->color == STK_RB_RED) {                                                     \
                    w->color = STK_RB_BLACK;                                                      \
                    x->parent->color = STK_RB_RED;                                                \
                    stk_rbtree_##name##_left_rotate(t, x->parent);                            \
                    w = x->parent->right;                                                     \
                }                                                                             \
                if (w->left->color == STK_RB_BLACK && w->right->color == STK_RB_BLACK) {              \
                    w->color = STK_RB_RED;                                                        \
                    x = x->parent;                                                            \
                } else {                                                                      \
                    if (w->right->color == STK_RB_BLACK) {                                        \
                        w->left->color = STK_RB_BLACK;                                            \
                        w->color = STK_RB_RED;                                                    \
                        stk_rbtree_##name##_right_rotate(t, w);                               \
                        w = x->parent->right;                                                 \
                    }                                                                         \
                    w->color = x->parent->color;                                              \
                    x->parent->color = STK_RB_BLACK;                                              \
                    w->right->color = STK_RB_BLACK;                                               \
                    stk_rbtree_##name##_left_rotate(t, x->parent);                            \
                    x = t->root;                                                              \
                }                                                                             \
            } else {                                                                          \
                stk_rbnode_##name* w = x->parent->left;                                       \
                if (w->color == STK_RB_RED) {                                                     \
                    w->color = STK_RB_BLACK;                                                      \
                    x->parent->color = STK_RB_RED;                                                \
                    stk_rbtree_##name##_right_rotate(t, x->parent);                           \
                    w = x->parent->left;                                                      \
                }                                                                             \
                if (w->right->color == STK_RB_BLACK && w->left->color == STK_RB_BLACK) {              \
                    w->color = STK_RB_RED;                                                        \
                    x = x->parent;                                                            \
                } else {                                                                      \
                    if (w->left->color == STK_RB_BLACK) {                                         \
                        w->right->color = STK_RB_BLACK;                                           \
                        w->color = STK_RB_RED;                                                    \
                        stk_rbtree_##name##_left_rotate(t, w);                                \
                        w = x->parent->left;                                                  \
                    }                                                                         \
                    w->color = x->parent->color;                                              \
                    x->parent->color = STK_RB_BLACK;                                              \
                    w->left->color = STK_RB_BLACK;                                                \
                    stk_rbtree_##name##_right_rotate(t, x->parent);                           \
                    x = t->root;                                                              \
                }                                                                             \
            }                                                                                 \
        }                                                                                     \
        x->color = STK_RB_BLACK;                                                                  \
    }                                                                                         \
                                                                                              \
    static void stk_rbtree_##name##_destroy_nodes(stk_rbtree_##name* t, stk_rbnode_##name* n) \
    {                                                                                         \
        if (n == t->nil)                                                                      \
            return;                                                                           \
        stk_rbtree_##name##_destroy_nodes(t, n->left);                                        \
        stk_rbtree_##name##_destroy_nodes(t, n->right);                                       \
        free(n);                                                                              \
    }                                                                                         \
                                                                                              \
    static void stk_rbtree_##name##_inorder_rec(stk_rbtree_##name* t,                         \
                                                stk_rbnode_##name* n,                         \
                                                void (*visit)(type*))                         \
    {                                                                                         \
        if (n == t->nil)                                                                      \
            return;                                                                           \
        stk_rbtree_##name##_inorder_rec(t, n->left, visit);                                   \
        visit(n->data);                                                                       \
        stk_rbtree_##name##_inorder_rec(t, n->right, visit);                                  \
    }                                                                                         \
                                                                                              \
    static void stk_rbtree_##name##_preorder_rec(stk_rbtree_##name* t,                        \
                                                 stk_rbnode_##name* n,                        \
                                                 void (*visit)(type*))                        \
    {                                                                                         \
        if (n == t->nil)                                                                      \
            return;                                                                           \
        visit(n->data);                                                                       \
        stk_rbtree_##name##_preorder_rec(t, n->left, visit);                                  \
        stk_rbtree_##name##_preorder_rec(t, n->right, visit);                                 \
    }                                                                                         \
                                                                                              \
    static void stk_rbtree_##name##_postorder_rec(stk_rbtree_##name* t,                       \
                                                  stk_rbnode_##name* n,                       \
                                                  void (*visit)(type*))                       \
    {                                                                                         \
        if (n == t->nil)                                                                      \
            return;                                                                           \
        stk_rbtree_##name##_postorder_rec(t, n->left, visit);                                 \
        stk_rbtree_##name##_postorder_rec(t, n->right, visit);                                \
        visit(n->data);                                                                       \
    }                                                                                         \
                                                                                              \
    void stk_rbtree_##name##_init(stk_rbtree_##name* t)                                       \
    {                                                                                         \
        t->nil = (stk_rbnode_##name*)malloc(sizeof(stk_rbnode_##name));                       \
        t->nil->color = STK_RB_BLACK;                                                             \
        t->nil->left = t->nil->right = t->nil->parent = t->nil;                               \
        t->root = t->nil;                                                                     \
        t->size = 0;                                                                          \
    }                                                                                         \
                                                                                              \
    void stk_rbtree_##name##_free(stk_rbtree_##name* t)                                       \
    {                                                                                         \
        stk_rbtree_##name##_destroy_nodes(t, t->root);                                        \
        free(t->nil);                                                                         \
        t->root = NULL;                                                                       \
        t->nil = NULL;                                                                        \
        t->size = 0;                                                                          \
    }                                                                                         \
                                                                                              \
    void stk_rbtree_##name##_insert(stk_rbtree_##name* t, type* val)                          \
    {                                                                                         \
        stk_rbnode_##name* z = stk_rbtree_##name##_node_new(val, t->nil);                     \
        stk_rbnode_##name* y = t->nil;                                                        \
        stk_rbnode_##name* x = t->root;                                                       \
                                                                                              \
        while (x != t->nil) {                                                                 \
            y = x;                                                                            \
            if (cmp_func(z->data, x->data) < 0)                                               \
                x = x->left;                                                                  \
            else                                                                              \
                x = x->right;                                                                 \
        }                                                                                     \
                                                                                              \
        z->parent = y;                                                                        \
        if (y == t->nil)                                                                      \
            t->root = z;                                                                      \
        else if (cmp_func(z->data, y->data) < 0)                                              \
            y->left = z;                                                                      \
        else                                                                                  \
            y->right = z;                                                                     \
                                                                                              \
        stk_rbtree_##name##_insert_fixup(t, z);                                               \
        t->size++;                                                                            \
    }                                                                                         \
                                                                                              \
    void stk_rbtree_##name##_remove(stk_rbtree_##name* t, type* val)                          \
    {                                                                                         \
        stk_rbnode_##name* z = t->root;                                                       \
        while (z != t->nil) {                                                                 \
            if (cmp_func(val, z->data) < 0)                                                   \
                z = z->left;                                                                  \
            else if (cmp_func(val, z->data) > 0)                                              \
                z = z->right;                                                                 \
            else                                                                              \
                break;                                                                        \
        }                                                                                     \
        if (z == t->nil)                                                                      \
            return;                                                                           \
                                                                                              \
        stk_rbnode_##name* y = z;                                                             \
        stk_rbnode_##name* x = NULL;                                                          \
        int y_orig_color = y->color;                                                          \
                                                                                              \
        if (z->left == t->nil) {                                                              \
            x = z->right;                                                                     \
            stk_rbtree_##name##_transplant(t, z, z->right);                                   \
        } else if (z->right == t->nil) {                                                      \
            x = z->left;                                                                      \
            stk_rbtree_##name##_transplant(t, z, z->left);                                    \
        } else {                                                                              \
            y = stk_rbtree_##name##_minimum(t, z->right);                                     \
            y_orig_color = y->color;                                                          \
            x = y->right;                                                                     \
            if (y->parent == z) {                                                             \
                x->parent = y;                                                                \
            } else {                                                                          \
                stk_rbtree_##name##_transplant(t, y, y->right);                               \
                y->right = z->right;                                                          \
                y->right->parent = y;                                                         \
            }                                                                                 \
            stk_rbtree_##name##_transplant(t, z, y);                                          \
            y->left = z->left;                                                                \
            y->left->parent = y;                                                              \
            y->color = z->color;                                                              \
        }                                                                                     \
                                                                                              \
        if (y_orig_color == STK_RB_BLACK)                                                         \
            stk_rbtree_##name##_delete_fixup(t, x);                                           \
        free(z);                                                                              \
        t->size--;                                                                            \
    }                                                                                         \
                                                                                              \
    type* stk_rbtree_##name##_find(stk_rbtree_##name* t, type* val)                           \
    {                                                                                         \
        stk_rbnode_##name* n = t->root;                                                       \
        while (n != t->nil) {                                                                 \
            if (cmp_func(val, n->data) < 0)                                                   \
                n = n->left;                                                                  \
            else if (cmp_func(val, n->data) > 0)                                              \
                n = n->right;                                                                 \
            else                                                                              \
                return n->data;                                                               \
        }                                                                                     \
        return NULL;                                                                          \
    }                                                                                         \
                                                                                              \
    bool stk_rbtree_##name##_has(stk_rbtree_##name* t, type* val)                             \
    {                                                                                         \
        stk_rbnode_##name* n = t->root;                                                       \
        while (n != t->nil) {                                                                 \
            if (cmp_func(val, n->data) < 0)                                                   \
                n = n->left;                                                                  \
            else if (cmp_func(val, n->data) > 0)                                              \
                n = n->right;                                                                 \
            else                                                                              \
                return true;                                                                  \
        }                                                                                     \
        return false;                                                                         \
    }                                                                                         \
                                                                                              \
    bool stk_rbtree_##name##_empty(stk_rbtree_##name* t)                                      \
    {                                                                                         \
        return t->root == t->nil;                                                             \
    }                                                                                         \
                                                                                              \
    size_t stk_rbtree_##name##_size(stk_rbtree_##name* t)                                     \
    {                                                                                         \
        return t->size;                                                                       \
    }                                                                                         \
                                                                                              \
    void stk_rbtree_##name##_inorder(stk_rbtree_##name* t, void (*visit)(type*))              \
    {                                                                                         \
        stk_rbtree_##name##_inorder_rec(t, t->root, visit);                                   \
    }                                                                                         \
                                                                                              \
    void stk_rbtree_##name##_preorder(stk_rbtree_##name* t, void (*visit)(type*))             \
    {                                                                                         \
        stk_rbtree_##name##_preorder_rec(t, t->root, visit);                                  \
    }                                                                                         \
                                                                                              \
    void stk_rbtree_##name##_postorder(stk_rbtree_##name* t, void (*visit)(type*))            \
    {                                                                                         \
        stk_rbtree_##name##_postorder_rec(t, t->root, visit);                                 \
    }                                                                                         \
                                                                                              \
    type* stk_rbtree_##name##_min(stk_rbtree_##name* t)                                       \
    {                                                                                         \
        if (t->root == t->nil)                                                                \
            return NULL;                                                                      \
        return stk_rbtree_##name##_minimum(t, t->root)->data;                                 \
    }                                                                                         \
                                                                                              \
    type* stk_rbtree_##name##_max(stk_rbtree_##name* t)                                       \
    {                                                                                         \
        if (t->root == t->nil)                                                                \
            return NULL;                                                                      \
        return stk_rbtree_##name##_maximum(t, t->root)->data;                                 \
    }                                                                                         \
                                                                                              \
    void stk_rbtree_##name##_clear(stk_rbtree_##name* t)                                      \
    {                                                                                         \
        stk_rbtree_##name##_destroy_nodes(t, t->root);                                        \
        t->root = t->nil;                                                                     \
        t->size = 0;                                                                          \
    }

#endif // STK_DETAIL_RBTREE_H