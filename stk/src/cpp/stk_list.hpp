#ifndef STK_LIST_CXX_H
#define STK_LIST_CXX_H

#pragma once

#include <initializer_list>
#include <cstddef>
#include <utility>
#include <vector>
#include "iterator/stk_list_iterator.hpp"

namespace stk {

    template<typename T> class list;
    template<typename T> class list_iterator;
    template<typename T> class list_const_iterator;

    template<typename T>
    class list {
    private:
        struct Node {
            T data;
            Node* prev;
            Node* next;

            Node() : prev(nullptr), next(nullptr) {}
            explicit Node(const T& val) : data(val), prev(nullptr), next(nullptr) {}
            explicit Node(T&& val) : data(std::move(val)), prev(nullptr), next(nullptr) {}
        };

        Node* sentinel_; 
        size_t size_; 

        void create_sentinel();
        Node* create_node(const T& _value);
        Node* create_node(T&& _value);
        void destroy_node(Node* _node) noexcept;
        void destroy_all() noexcept;
        void transfer(Node* _pos, Node* _first, Node* _last) noexcept;

    public:
        using iterator = list_iterator<T>;
        using const_iterator = list_const_iterator<T>;
        using reverse_iterator = stk::reverse_iterator<iterator>;
        using const_reverse_iterator = stk::reverse_iterator<const_iterator>;

        using value_type = T;
        using size_type = size_t;
        using reference = T&;
        using const_reference = const T&;

        list();
        explicit list(size_t _size);
        list(const list& _other);
        list(list&& _other) noexcept;
        list(std::initializer_list<T> _other);
        ~list();

        list& operator=(const list& _other);
        list& operator=(list&& _other);
        list& operator=(std::initializer_list<T> _other);

        T& front();
        const T& front() const;
        T& back();
        const T& back() const;

        iterator begin() noexcept;
        const_iterator begin() const noexcept;
        const_iterator cbegin() const noexcept;

        iterator end() noexcept;
        const_iterator end() const noexcept;
        const_iterator cend() const noexcept;

        reverse_iterator rbegin() noexcept;
        const_reverse_iterator rbegin() const noexcept;
        const_reverse_iterator crbegin() const noexcept;

        reverse_iterator rend() noexcept;
        const_reverse_iterator rend() const noexcept;
        const_reverse_iterator crend() const noexcept;

        bool empty() const noexcept;
        size_t size() const noexcept;
        void resize(size_t _size);
        void resize(size_t _size, const T& _value);

        void clear() noexcept;

        iterator insert(const_iterator _pos, const T& _value);
        iterator insert(const_iterator _pos, T&& _value);
        iterator insert(const_iterator _pos, size_t _count, const T& _value);

        iterator erase(const_iterator _pos);
        iterator erase(const_iterator _first, const_iterator _last);

        void push_front(const T& _value);
        void push_front(T&& _value);
        void pop_front();

        void push_back(const T& _value);
        void push_back(T&& _value);
        void pop_back();

        void remove(const T& _value);
        void unique();
        void reverse() noexcept;
        void merge(list& _other);
        void sort();

        void splice(const_iterator _pos, list& _other);
        void splice(const_iterator _pos, list& _other, const_iterator _it);
        void splice(const_iterator _pos, list& _other, const_iterator _first, const_iterator _last);

        list operator+(const list& _other) const;
        list operator-(const list& _other) const;
        list operator*(T _scalar) const;

        list& operator+=(const list& _other);
        list& operator-=(const list& _other);
        list& operator*=(T _scalar);

        void fill(const T& _value);
        T norm() const;
        void normalize();
        T dot(const list& _other) const;

        static list zeros(size_t _size);
        static list ones(size_t _size);
        static list random(size_t _size, T _min = 0, T _max = 1);

        bool operator==(const list& _other) const;
        bool operator!=(const list& _other) const;

        const T* data() const;

        void swap(list& _other) noexcept;

        friend class list_iterator<T>;
        friend class list_const_iterator<T>;
    };

    template<typename T>
    list<T> operator*(T _scalar, const list<T>& _vec);

    template<typename T>
    void swap(list<T>& _a, list<T>& _b) noexcept;

} // namespace stk

#include "stk_list.tcc"

#endif