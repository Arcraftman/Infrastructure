#ifndef STK_LIST_ITERATOR_HPP
#define STK_LIST_ITERATOR_HPP

#include "stk_base_iterator.hpp"

namespace stk {

    // 前向声明
    template<typename T> class list;

    // ========== List 迭代器（非 const）==========
    template<typename T>
    class list_iterator {
    private:
        // 使用 list 内部的 Node 类型（通过友元访问）
        typename list<T>::Node* ptr_;

    public:
        using iterator_category = bidirectional_iterator_tag;
        using value_type = T;
        using difference_type = std::ptrdiff_t;
        using pointer = T*;
        using reference = T&;

        list_iterator(typename list<T>::Node* ptr = nullptr) : ptr_(ptr) {}

        // 解引用
        reference operator*() const {
            return ptr_->data;
        }

        pointer operator->() const {
            return &ptr_->data;
        }

        // 前置 ++
        list_iterator& operator++() {
            ptr_ = ptr_->next;
            return *this;
        }

        // 后置 ++
        list_iterator operator++(int) {
            list_iterator tmp = *this;
            ptr_ = ptr_->next;
            return tmp;
        }

        // 前置 --
        list_iterator& operator--() {
            ptr_ = ptr_->prev;
            return *this;
        }

        // 后置 --
        list_iterator operator--(int) {
            list_iterator tmp = *this;
            ptr_ = ptr_->prev;
            return tmp;
        }

        // 比较
        bool operator==(const list_iterator& other) const {
            return ptr_ == other.ptr_;
        }

        bool operator!=(const list_iterator& other) const {
            return ptr_ != other.ptr_;
        }

        // 获取原始指针（供 list 使用）
        typename list<T>::Node* get_ptr() const { return ptr_; }
    };

    // ========== List 常量迭代器 ==========
    template<typename T>
    class list_const_iterator {
    private:
        const typename list<T>::Node* ptr_;

    public:
        using iterator_category = bidirectional_iterator_tag;
        using value_type = T;
        using difference_type = std::ptrdiff_t;
        using pointer = const T*;
        using reference = const T&;

        list_const_iterator(const typename list<T>::Node* ptr = nullptr) : ptr_(ptr) {}
        list_const_iterator(const list_iterator<T>& it) : ptr_(it.get_ptr()) {}

        reference operator*() const {
            return ptr_->data;
        }

        pointer operator->() const {
            return &ptr_->data;
        }

        list_const_iterator& operator++() {
            ptr_ = ptr_->next;
            return *this;
        }

        list_const_iterator operator++(int) {
            list_const_iterator tmp = *this;
            ptr_ = ptr_->next;
            return tmp;
        }

        list_const_iterator& operator--() {
            ptr_ = ptr_->prev;
            return *this;
        }

        list_const_iterator operator--(int) {
            list_const_iterator tmp = *this;
            ptr_ = ptr_->prev;
            return tmp;
        }

        bool operator==(const list_const_iterator& other) const {
            return ptr_ == other.ptr_;
        }

        bool operator!=(const list_const_iterator& other) const {
            return ptr_ != other.ptr_;
        }

        const typename list<T>::Node* get_ptr() const { return ptr_; }
    };

} // namespace stk

#endif