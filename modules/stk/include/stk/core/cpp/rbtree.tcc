#ifndef STK_CORE_CXX_RBTREE_TCC
#define STK_CORE_CXX_RBTREE_TCC

namespace stk {

// ============================================================================
// 构造 / 析构
// ============================================================================

template <typename T>
rbtree<T>::rbtree()
{
    nil_ = new node();
    nil_->color_ = color::black;
    nil_->left_ = nullptr;
    nil_->right_ = nullptr;
    nil_->parent_ = nullptr;
    root_ = nil_;
}

template <typename T>
rbtree<T>::~rbtree()
{
    clear();
    delete nil_;
}

// ============================================================================
// 容量
// ============================================================================

template <typename T>
bool rbtree<T>::empty() const noexcept
{
    return size_ == 0;
}

template <typename T>
std::size_t rbtree<T>::size() const noexcept
{
    return size_;
}

// ============================================================================
// 旋转
// ============================================================================

template <typename T>
void rbtree<T>::left_rotate(node* x)
{
    node* y = x->right_;
    x->right_ = y->left_;
    if (y->left_ != nil_) {
        y->left_->parent_ = x;
    }
    y->parent_ = x->parent_;
    if (x->parent_ == nil_) {
        root_ = y;
    } else if (x == x->parent_->left_) {
        x->parent_->left_ = y;
    } else {
        x->parent_->right_ = y;
    }
    y->left_ = x;
    x->parent_ = y;
}

template <typename T>
void rbtree<T>::right_rotate(node* y)
{
    node* x = y->left_;
    y->left_ = x->right_;
    if (x->right_ != nil_) {
        x->right_->parent_ = y;
    }
    x->parent_ = y->parent_;
    if (y->parent_ == nil_) {
        root_ = x;
    } else if (y == y->parent_->left_) {
        y->parent_->left_ = x;
    } else {
        y->parent_->right_ = x;
    }
    x->right_ = y;
    y->parent_ = x;
}

// ============================================================================
// 插入
// ============================================================================

template <typename T>
void rbtree<T>::insert(const T& value)
{
    node* z = new node(value);
    node* y = nil_;
    node* x = root_;

    while (x != nil_) {
        y = x;
        if (z->data_ < x->data_) {
            x = x->left_;
        } else if (x->data_ < z->data_) {
            x = x->right_;
        } else {
            delete z;
            return;
        }
    }

    z->parent_ = y;
    if (y == nil_) {
        root_ = z;
    } else if (z->data_ < y->data_) {
        y->left_ = z;
    } else {
        y->right_ = z;
    }

    z->left_ = nil_;
    z->right_ = nil_;
    z->color_ = color::red;
    ++size_;
    insert_fixup(z);
}

template <typename T>
void rbtree<T>::insert_fixup(node* z)
{
    while (z->parent_->color_ == color::red) {
        if (z->parent_ == z->parent_->parent_->left_) {
            node* y = z->parent_->parent_->right_;
            if (y->color_ == color::red) {
                z->parent_->color_ = color::black;
                y->color_ = color::black;
                z->parent_->parent_->color_ = color::red;
                z = z->parent_->parent_;
            } else {
                if (z == z->parent_->right_) {
                    z = z->parent_;
                    left_rotate(z);
                }
                z->parent_->color_ = color::black;
                z->parent_->parent_->color_ = color::red;
                right_rotate(z->parent_->parent_);
            }
        } else {
            node* y = z->parent_->parent_->left_;
            if (y->color_ == color::red) {
                z->parent_->color_ = color::black;
                y->color_ = color::black;
                z->parent_->parent_->color_ = color::red;
                z = z->parent_->parent_;
            } else {
                if (z == z->parent_->left_) {
                    z = z->parent_;
                    right_rotate(z);
                }
                z->parent_->color_ = color::black;
                z->parent_->parent_->color_ = color::red;
                left_rotate(z->parent_->parent_);
            }
        }
    }
    root_->color_ = color::black;
}

// ============================================================================
// 删除
// ============================================================================

template <typename T>
bool rbtree<T>::erase(const T& value)
{
    node* z = search(value);
    if (z == nil_) {
        return false;
    }

    node* y = z;
    node* x = nullptr;
    color y_orig_col = y->color_;

    if (z->left_ == nil_) {
        x = z->right_;
        transplant(z, z->right_);
    } else if (z->right_ == nil_) {
        x = z->left_;
        transplant(z, z->left_);
    } else {
        y = minimum(z->right_);
        y_orig_col = y->color_;
        x = y->right_;
        if (y->parent_ == z) {
            x->parent_ = y;
        } else {
            transplant(y, y->right_);
            y->right_ = z->right_;
            y->right_->parent_ = y;
        }
        transplant(z, y);
        y->left_ = z->left_;
        y->left_->parent_ = y;
        y->color_ = z->color_;
    }

    delete z;
    --size_;

    if (y_orig_col == color::black) {
        erase_fixup(x);
    }

    if (size_ == 0) {
        root_ = nil_;
    }

    return true;
}

template <typename T>
void rbtree<T>::erase_fixup(node* x)
{
    while (x != root_ && x->color_ == color::black) {
        if (x == x->parent_->left_) {
            node* w = x->parent_->right_;
            if (w->color_ == color::red) {
                w->color_ = color::black;
                x->parent_->color_ = color::red;
                left_rotate(x->parent_);
                w = x->parent_->right_;
            }
            if (w->left_->color_ == color::black && w->right_->color_ == color::black) {
                w->color_ = color::red;
                x = x->parent_;
            } else {
                if (w->right_->color_ == color::black) {
                    w->left_->color_ = color::black;
                    w->color_ = color::red;
                    right_rotate(w);
                    w = x->parent_->right_;
                }
                w->color_ = x->parent_->color_;
                x->parent_->color_ = color::black;
                w->right_->color_ = color::black;
                left_rotate(x->parent_);
                x = root_;
            }
        } else {
            node* w = x->parent_->left_;
            if (w->color_ == color::red) {
                w->color_ = color::black;
                x->parent_->color_ = color::red;
                right_rotate(x->parent_);
                w = x->parent_->left_;
            }
            if (w->right_->color_ == color::black && w->left_->color_ == color::black) {
                w->color_ = color::red;
                x = x->parent_;
            } else {
                if (w->left_->color_ == color::black) {
                    w->right_->color_ = color::black;
                    w->color_ = color::red;
                    left_rotate(w);
                    w = x->parent_->left_;
                }
                w->color_ = x->parent_->color_;
                x->parent_->color_ = color::black;
                w->left_->color_ = color::black;
                right_rotate(x->parent_);
                x = root_;
            }
        }
    }
    x->color_ = color::black;
}

// ============================================================================
// 辅助
// ============================================================================

template <typename T>
void rbtree<T>::transplant(node* u, node* v)
{
    if (u->parent_ == nil_) {
        root_ = v;
    } else if (u == u->parent_->left_) {
        u->parent_->left_ = v;
    } else {
        u->parent_->right_ = v;
    }
    v->parent_ = u->parent_;
}

template <typename T>
typename rbtree<T>::node* rbtree<T>::minimum(node* x) const
{
    while (x->left_ != nil_) {
        x = x->left_;
    }
    return x;
}

template <typename T>
typename rbtree<T>::node* rbtree<T>::search(const T& value) const
{
    node* cur = root_;
    while (cur != nil_) {
        if (value < cur->data_) {
            cur = cur->left_;
        } else if (cur->data_ < value) {
            cur = cur->right_;
        } else {
            return cur;
        }
    }
    return nil_;
}

// ============================================================================
// 查找
// ============================================================================

template <typename T>
bool rbtree<T>::contains(const T& value) const
{
    return search(value) != nil_;
}

template <typename T>
const T* rbtree<T>::find(const T& value) const
{
    node* n = search(value);
    return n != nil_ ? &n->data_ : nullptr;
}

// ============================================================================
// 最小 / 最大
// ============================================================================

template <typename T>
const T* rbtree<T>::min() const
{
    return root_ != nil_ ? &minimum(root_)->data_ : nullptr;
}

template <typename T>
const T* rbtree<T>::max() const
{
    node* cur = root_;
    if (cur == nil_) {
        return nullptr;
    }
    while (cur->right_ != nil_) {
        cur = cur->right_;
    }
    return &cur->data_;
}

// ============================================================================
// 清空
// ============================================================================

template <typename T>
void rbtree<T>::clear_subtree(node* n)
{
    if (n != nil_ && n != nullptr) {
        clear_subtree(n->left_);
        clear_subtree(n->right_);
        delete n;
    }
}

template <typename T>
void rbtree<T>::clear()
{
    clear_subtree(root_);
    root_ = nil_;
    size_ = 0;
}

// ============================================================================
// 迭代器
// ============================================================================

template <typename T>
typename rbtree<T>::iterator& rbtree<T>::iterator::operator++()
{
    if (node_ == nullptr) {
        return *this;
    }

    if (node_->right_ != nullptr) {
        node_ = node_->right_;
        while (node_->left_ != nullptr) {
            node_ = node_->left_;
        }
        return *this;
    }

    node* y = node_->parent_;
    while (y != nullptr && node_ == y->right_) {
        node_ = y;
        y = y->parent_;
    }
    node_ = y;
    return *this;
}

template <typename T>
typename rbtree<T>::iterator rbtree<T>::iterator::operator++(int)
{
    iterator tmp = *this;
    ++(*this);
    return tmp;
}

// ============================================================================
// 迭代器获取
// ============================================================================

template <typename T>
typename rbtree<T>::iterator rbtree<T>::begin() noexcept
{
    node* first = root_;
    if (first == nil_) {
        return iterator(nullptr, this);
    }
    while (first->left_ != nil_) {
        first = first->left_;
    }
    return iterator(first, this);
}

template <typename T>
typename rbtree<T>::iterator rbtree<T>::end() noexcept
{
    return iterator(nullptr, this);
}

} // namespace stk

#endif