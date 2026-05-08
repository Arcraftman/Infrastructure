#ifndef _STK_RBTREE_TCC
#define _STK_RBTREE_TCC

#include <iostream>

namespace stk {

    
    template<typename T>
    rbtree<T>::rbtree()
    {
        nil_ = new rbnode<T>(T());
        nil_->color_ = Color::BLACK;
        nil_->left_ = nil_->right_ = nil_->parent_ = nullptr;
        root_ = nil_;
    }

    template<typename T>
    rbtree<T>::~rbtree()
    {
        clearHelper(root_);
        delete nil_;
    }

    template<typename T>
    void rbtree<T>::clearHelper(rbnode<T>* node)
    {
        if (node != nullptr && node != nil_) {
            clearHelper(node->left_);
            clearHelper(node->right_);
            delete node;
        }
    }

    template<typename T>
    bool rbtree<T>::isEmpty() const
    {
        return root_ == nil_;
    }

    
    template<typename T>
    rbnode<T>* rbtree<T>::searchNode(const T& value) const
    {
        rbnode<T>* cur = root_;
        while (cur != nil_) {
            if (value < cur->data_) cur = cur->left_;
            else if (cur->data_ < value) cur = cur->right_;
            else return cur;
        }
        return nil_;
    }

    template<typename T>
    bool rbtree<T>::find(const T& value) const
    {
        return searchNode(value) != nil_;
    }

    
    template<typename T>
    void rbtree<T>::leftRotate(rbnode<T>* x)
    {
        rbnode<T>* y = x->right_;
        x->right_ = y->left_;
        if (y->left_ != nil_) y->left_->parent_ = x;
        y->parent_ = x->parent_;
        if (x->parent_ == nil_) root_ = y;
        else if (x == x->parent_->left_) x->parent_->left_ = y;
        else x->parent_->right_ = y;
        y->left_ = x;
        x->parent_ = y;
    }

    template<typename T>
    void rbtree<T>::rightRotate(rbnode<T>* y)
    {
        rbnode<T>* x = y->left_;
        y->left_ = x->right_;
        if (x->right_ != nil_) x->right_->parent_ = y;
        x->parent_ = y->parent_;
        if (y->parent_ == nil_) root_ = x;
        else if (y == y->parent_->left_) y->parent_->left_ = x;
        else y->parent_->right_ = x;
        x->right_ = y;
        y->parent_ = x;
    }

    
    template<typename T>
    void rbtree<T>::insert(const T& value)
    {
        rbnode<T>* z = new rbnode<T>(value);
        rbnode<T>* y = nil_;
        rbnode<T>* x = root_;
        
        while (x != nil_) {
            y = x;
            if (z->data_ < x->data_) x = x->left_;
            else x = x->right_;
        }
        
        z->parent_ = y;
        if (y == nil_) root_ = z;
        else if (z->data_ < y->data_) y->left_ = z;
        else y->right_ = z;
        
        z->left_ = z->right_ = nil_;
        z->color_ = Color::RED;
        fixInsert(z);
    }

    template<typename T>
    void rbtree<T>::fixInsert(rbnode<T>* z)
    {
        while (z->parent_->color_ == Color::RED) {
            if (z->parent_ == z->parent_->parent_->left_) {
                rbnode<T>* y = z->parent_->parent_->right_;
                if (y->color_ == Color::RED) {
                    z->parent_->color_ = Color::BLACK;
                    y->color_ = Color::BLACK;
                    z->parent_->parent_->color_ = Color::RED;
                    z = z->parent_->parent_;
                } else {
                    if (z == z->parent_->right_) {
                        z = z->parent_;
                        leftRotate(z);
                    }
                    z->parent_->color_ = Color::BLACK;
                    z->parent_->parent_->color_ = Color::RED;
                    rightRotate(z->parent_->parent_);
                }
            } else {
                rbnode<T>* y = z->parent_->parent_->left_;
                if (y->color_ == Color::RED) {
                    z->parent_->color_ = Color::BLACK;
                    y->color_ = Color::BLACK;
                    z->parent_->parent_->color_ = Color::RED;
                    z = z->parent_->parent_;
                } else {
                    if (z == z->parent_->left_) {
                        z = z->parent_;
                        rightRotate(z);
                    }
                    z->parent_->color_ = Color::BLACK;
                    z->parent_->parent_->color_ = Color::RED;
                    leftRotate(z->parent_->parent_);
                }
            }
        }
        root_->color_ = Color::BLACK;
    }

    
    template<typename T>
    rbnode<T>* rbtree<T>::minimum(rbnode<T>* x) const
    {
        while (x->left_ != nil_) x = x->left_;
        return x;
    }

    template<typename T>
    rbnode<T>* rbtree<T>::maximum(rbnode<T>* x) const
    {
        while (x->right_ != nil_) x = x->right_;
        return x;
    }

    
    template<typename T>
    void rbtree<T>::transplant(rbnode<T>* u, rbnode<T>* v)
    {
        if (u->parent_ == nil_) root_ = v;
        else if (u == u->parent_->left_) u->parent_->left_ = v;
        else u->parent_->right_ = v;
        v->parent_ = u->parent_;
    }

    
    template<typename T>
    void rbtree<T>::remove(const T& value)
    {
        rbnode<T>* z = searchNode(value);
        if (z == nil_) return;
        
        rbnode<T>* y = z;
        rbnode<T>* x = nullptr;
        Color yOriginalColor = y->color_;
        
        if (z->left_ == nil_) {
            x = z->right_;
            transplant(z, z->right_);
        } else if (z->right_ == nil_) {
            x = z->left_;
            transplant(z, z->left_);
        } else {
            y = minimum(z->right_);
            yOriginalColor = y->color_;
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
        
        if (yOriginalColor == Color::BLACK) {
            fixDelete(x);
        }
    }

    template<typename T>
    void rbtree<T>::fixDelete(rbnode<T>* x)
    {
        while (x != root_ && x->color_ == Color::BLACK) {
            if (x == x->parent_->left_) {
                rbnode<T>* w = x->parent_->right_;
                if (w->color_ == Color::RED) {
                    w->color_ = Color::BLACK;
                    x->parent_->color_ = Color::RED;
                    leftRotate(x->parent_);
                    w = x->parent_->right_;
                }
                if (w->left_->color_ == Color::BLACK && w->right_->color_ == Color::BLACK) {
                    w->color_ = Color::RED;
                    x = x->parent_;
                } else {
                    if (w->right_->color_ == Color::BLACK) {
                        w->left_->color_ = Color::BLACK;
                        w->color_ = Color::RED;
                        rightRotate(w);
                        w = x->parent_->right_;
                    }
                    w->color_ = x->parent_->color_;
                    x->parent_->color_ = Color::BLACK;
                    w->right_->color_ = Color::BLACK;
                    leftRotate(x->parent_);
                    x = root_;
                }
            } else {
                rbnode<T>* w = x->parent_->left_;
                if (w->color_ == Color::RED) {
                    w->color_ = Color::BLACK;
                    x->parent_->color_ = Color::RED;
                    rightRotate(x->parent_);
                    w = x->parent_->left_;
                }
                if (w->right_->color_ == Color::BLACK && w->left_->color_ == Color::BLACK) {
                    w->color_ = Color::RED;
                    x = x->parent_;
                } else {
                    if (w->left_->color_ == Color::BLACK) {
                        w->right_->color_ = Color::BLACK;
                        w->color_ = Color::RED;
                        leftRotate(w);
                        w = x->parent_->left_;
                    }
                    w->color_ = x->parent_->color_;
                    x->parent_->color_ = Color::BLACK;
                    w->left_->color_ = Color::BLACK;
                    rightRotate(x->parent_);
                    x = root_;
                }
            }
        }
        x->color_ = Color::BLACK;
    }

    
    template<typename T>
    void rbtree<T>::inorderHelper(rbnode<T>* node) const
    {
        if (node != nil_) {
            inorderHelper(node->left_);
            std::cout << node->data_ << " ";
            inorderHelper(node->right_);
        }
    }

    template<typename T>
    void rbtree<T>::inorderPrint() const
    {
        inorderHelper(root_);
        std::cout << std::endl;
    }

} 

#endif 
