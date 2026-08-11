#ifndef STK_CORE_CXX_RBTREE_H
#define STK_CORE_CXX_RBTREE_H

#include <cstddef>
#include <utility>

namespace stk {

// ============================================================================
// 红黑树
// ============================================================================

template <typename T>
class rbtree {
public:
    rbtree();
    ~rbtree();

    bool empty() const noexcept;
    std::size_t size() const noexcept;

    void insert(const T& value);
    bool erase(const T& value);
    void clear();

    bool contains(const T& value) const;
    const T* find(const T& value) const;

    const T* min() const;
    const T* max() const;

    // ========================================================================
    // 内部节点类型（声明在 iterator 之前，使其成员 node* 可见）
    // ========================================================================

    enum class color : bool { red = false, black = true };

    struct node {
        T data_;
        color color_ = color::red;
        node* left_ = nullptr;
        node* right_ = nullptr;
        node* parent_ = nullptr;

        explicit node(const T& data) : data_(data)
        {
        }
        explicit node(T&& data) : data_(std::move(data))
        {
        }
        node() : color_(color::black)
        {
        }
    };

    // ========================================================================
    // 迭代器
    // ========================================================================

    class iterator {
    public:
        iterator() = default;

        T& operator*() const
        {
            return node_->data_;
        }
        T* operator->() const
        {
            return &node_->data_;
        }

        iterator& operator++();
        iterator operator++(int);

        bool operator==(const iterator& other) const
        {
            return node_ == other.node_;
        }
        bool operator!=(const iterator& other) const
        {
            return node_ != other.node_;
        }

    private:
        friend class rbtree;

        node* node_ = nullptr;
        const rbtree* tree_ = nullptr;

        iterator(node* n, const rbtree* t) : node_(n), tree_(t)
        {
        }
    };

    iterator begin() noexcept;
    iterator end() noexcept;

private:
    // ========================================================================
    // 内部实现
    // ========================================================================

    node* root_ = nullptr;
    node* nil_ = nullptr;
    std::size_t size_ = 0;

    void left_rotate(node* x);
    void right_rotate(node* y);
    void insert_fixup(node* z);
    void erase_fixup(node* x);
    void transplant(node* u, node* v);
    node* minimum(node* x) const;
    node* search(const T& value) const;
    void clear_subtree(node* n);
};

} // namespace stk

#include "rbtree.tcc"

#endif