#ifndef STK_LIST_TCC
#define STK_LIST_TCC

#include <cmath>
#include <random>
#include <stdexcept>
#include <algorithm>
#include <vector>
#include <utility>

namespace stk {


    template<typename T>
    void list<T>::create_sentinel() {
        sentinel_ = new Node();
        sentinel_->next = sentinel_;
        sentinel_->prev = sentinel_;
        size_ = 0;
    }

    template<typename T>
    typename list<T>::Node* list<T>::create_node(const T& _value) {
        return new Node(_value);
    }

    template<typename T>
    typename list<T>::Node* list<T>::create_node(T&& _value) {
        return new Node(std::move(_value));
    }

    template<typename T>
    void list<T>::destroy_node(Node* _node) noexcept {
        delete _node;
    }

    template<typename T>
    void list<T>::destroy_all() noexcept {
        Node* curr = sentinel_->next;
        while (curr != sentinel_) {
            Node* next = curr->next;
            destroy_node(curr);
            curr = next;
        }
    }

    template<typename T>
    void list<T>::transfer(Node* _pos, Node* _first, Node* _last) noexcept {
        if (_first == _last) return;

        Node* first_prev = _first->prev;
        Node* last_prev = _last->prev;

        first_prev->next = _last;
        _last->prev = first_prev;

        Node* pos_prev = _pos->prev;
        pos_prev->next = _first;
        _first->prev = pos_prev;

        last_prev->next = _pos;
        _pos->prev = last_prev;
    }

    // ========== 构造/析构 ==========

    template<typename T>
    list<T>::list() {
        create_sentinel();
    }

    template<typename T>
    list<T>::list(size_t _size) : list() {
        for (size_t i = 0; i < _size; ++i) {
            push_back(T());
        }
    }

    template<typename T>
    list<T>::list(const list& _other) : list() {
        for (const auto& val : _other) {
            push_back(val);
        }
    }

    template<typename T>
    list<T>::list(list&& _other) noexcept
        : sentinel_(_other.sentinel_)
        , size_(_other.size_) {
        _other.sentinel_ = nullptr;
        _other.size_ = 0;
    }

    template<typename T>
    list<T>::list(std::initializer_list<T> _other) : list() {
        for (const auto& val : _other) {
            push_back(val);
        }
    }

    template<typename T>
    list<T>::~list() {
        if (sentinel_) {
            clear();
            delete sentinel_;
            sentinel_ = nullptr;
        }
    }

    // ========== 赋值 ==========

    template<typename T>
    list<T>& list<T>::operator=(const list& _other) {
        if (this != &_other) {
            list tmp(_other);
            swap(tmp);
        }
        return *this;
    }

    template<typename T>
    list<T>& list<T>::operator=(list&& _other) {
        if (this != &_other) {
            clear();
            delete sentinel_;
            sentinel_ = _other.sentinel_;
            size_ = _other.size_;
            _other.sentinel_ = nullptr;
            _other.size_ = 0;
        }
        return *this;
    }

    template<typename T>
    list<T>& list<T>::operator=(std::initializer_list<T> _other) {
        list tmp(_other);
        swap(tmp);
        return *this;
    }

    // ========== 元素访问 ==========

    template<typename T>
    T& list<T>::front() {
        if (empty()) {
            throw std::out_of_range("list::front: empty list");
        }
        return sentinel_->next->data;
    }

    template<typename T>
    const T& list<T>::front() const {
        if (empty()) {
            throw std::out_of_range("list::front: empty list");
        }
        return sentinel_->next->data;
    }

    template<typename T>
    T& list<T>::back() {
        if (empty()) {
            throw std::out_of_range("list::back: empty list");
        }
        return sentinel_->prev->data;
    }

    template<typename T>
    const T& list<T>::back() const {
        if (empty()) {
            throw std::out_of_range("list::back: empty list");
        }
        return sentinel_->prev->data;
    }

    // ========== 迭代器 ==========

    template<typename T>
    typename list<T>::iterator list<T>::begin() noexcept {
        return iterator(sentinel_->next);
    }

    template<typename T>
    typename list<T>::const_iterator list<T>::begin() const noexcept {
        return const_iterator(sentinel_->next);
    }

    template<typename T>
    typename list<T>::const_iterator list<T>::cbegin() const noexcept {
        return const_iterator(sentinel_->next);
    }

    template<typename T>
    typename list<T>::iterator list<T>::end() noexcept {
        return iterator(sentinel_);
    }

    template<typename T>
    typename list<T>::const_iterator list<T>::end() const noexcept {
        return const_iterator(sentinel_);
    }

    template<typename T>
    typename list<T>::const_iterator list<T>::cend() const noexcept {
        return const_iterator(sentinel_);
    }

    template<typename T>
    typename list<T>::reverse_iterator list<T>::rbegin() noexcept {
        return reverse_iterator(end());
    }

    template<typename T>
    typename list<T>::const_reverse_iterator list<T>::rbegin() const noexcept {
        return const_reverse_iterator(end());
    }

    template<typename T>
    typename list<T>::const_reverse_iterator list<T>::crbegin() const noexcept {
        return const_reverse_iterator(cend());
    }

    template<typename T>
    typename list<T>::reverse_iterator list<T>::rend() noexcept {
        return reverse_iterator(begin());
    }

    template<typename T>
    typename list<T>::const_reverse_iterator list<T>::rend() const noexcept {
        return const_reverse_iterator(begin());
    }

    template<typename T>
    typename list<T>::const_reverse_iterator list<T>::crend() const noexcept {
        return const_reverse_iterator(cbegin());
    }

    // ========== 容量 ==========

    template<typename T>
    bool list<T>::empty() const noexcept {
        return size_ == 0;
    }

    template<typename T>
    size_t list<T>::size() const noexcept {
        return size_;
    }

    template<typename T>
    void list<T>::resize(size_t _size) {
        resize(_size, T());
    }

    template<typename T>
    void list<T>::resize(size_t _size, const T& _value) {
        while (size_ > _size) {
            pop_back();
        }
        while (size_ < _size) {
            push_back(_value);
        }
    }

    // ========== 修改器 ==========

    template<typename T>
    void list<T>::clear() noexcept {
        destroy_all();
        sentinel_->next = sentinel_;
        sentinel_->prev = sentinel_;
        size_ = 0;
    }

    template<typename T>
    typename list<T>::iterator list<T>::insert(const_iterator _pos, const T& _value) {
        Node* curr = const_cast<Node*>(_pos.get_ptr());
        Node* prev = curr->prev;
        Node* node = create_node(_value);

        prev->next = node;
        node->prev = prev;
        node->next = curr;
        curr->prev = node;

        ++size_;
        return iterator(node);
    }

    template<typename T>
    typename list<T>::iterator list<T>::insert(const_iterator _pos, T&& _value) {
        Node* curr = const_cast<Node*>(_pos.get_ptr());
        Node* prev = curr->prev;
        Node* node = create_node(std::move(_value));

        prev->next = node;
        node->prev = prev;
        node->next = curr;
        curr->prev = node;

        ++size_;
        return iterator(node);
    }

    template<typename T>
    typename list<T>::iterator list<T>::insert(const_iterator _pos, size_t _count, const T& _value) {
        for (size_t i = 0; i < _count; ++i) {
            _pos = insert(_pos, _value);
        }
        return iterator(const_cast<Node*>(_pos.get_ptr()));
    }

    template<typename T>
    typename list<T>::iterator list<T>::erase(const_iterator _pos) {
        if (_pos == cend()) {
            return end();
        }

        Node* curr = const_cast<Node*>(_pos.get_ptr());
        Node* next = curr->next;
        Node* prev = curr->prev;

        prev->next = next;
        next->prev = prev;

        destroy_node(curr);
        --size_;

        return iterator(next);
    }

    template<typename T>
    typename list<T>::iterator list<T>::erase(const_iterator _first, const_iterator _last) {
        while (_first != _last) {
            _first = erase(_first);
        }
        return iterator(const_cast<Node*>(_last.get_ptr()));
    }

    template<typename T>
    void list<T>::push_front(const T& _value) {
        insert(cbegin(), _value);
    }

    template<typename T>
    void list<T>::push_front(T&& _value) {
        insert(cbegin(), std::move(_value));
    }

    template<typename T>
    void list<T>::pop_front() {
        if (!empty()) {
            erase(cbegin());
        }
    }

    template<typename T>
    void list<T>::push_back(const T& _value) {
        insert(cend(), _value);
    }

    template<typename T>
    void list<T>::push_back(T&& _value) {
        insert(cend(), std::move(_value));
    }

    template<typename T>
    void list<T>::pop_back() {
        if (!empty()) {
            erase(--cend());
        }
    }


    template<typename T>
    void list<T>::remove(const T& _value) {
        for (auto it = begin(); it != end(); ) {
            if (*it == _value) {
                it = erase(it);
            }
            else {
                ++it;
            }
        }
    }

    template<typename T>
    void list<T>::unique() {
        if (size_ < 2) return;

        auto it = begin();
        auto next = it;
        ++next;

        while (next != end()) {
            if (*it == *next) {
                next = erase(next);
            }
            else {
                it = next;
                ++next;
            }
        }
    }

    template<typename T>
    void list<T>::reverse() noexcept {
        if (size_ < 2) return;

        Node* curr = sentinel_;
        do {
            Node* next = curr->next;
            curr->next = curr->prev;
            curr->prev = next;
            curr = next;
        } while (curr != sentinel_);
    }

    template<typename T>
    void list<T>::merge(list& _other) {
        if (this == &_other) return;

        auto it1 = begin();
        auto it2 = _other.begin();

        while (it1 != end() && it2 != _other.end()) {
            if (*it2 < *it1) {
                auto next = it2;
                ++next;
                splice(it1, _other, it2);
                it2 = next;
            }
            else {
                ++it1;
            }
        }

        if (it2 != _other.end()) {
            splice(end(), _other, it2, _other.end());
        }
    }

    template<typename T>
    void list<T>::sort() {
        if (size_ < 2) return;

        list carry;
        list tmp[64];
        list* fill = nullptr;

        while (!empty()) {
            carry.splice(carry.begin(), *this, begin());
            int i = 0;
            while (i < 64 && !tmp[i].empty()) {
                tmp[i].merge(carry);
                carry.swap(tmp[i]);
                ++i;
            }
            carry.swap(tmp[i]);
            if (i == 64) {
                tmp[i - 1].merge(carry);
            }
        }

        for (int i = 0; i < 64; ++i) {
            if (!tmp[i].empty()) {
                if (fill == nullptr) {
                    fill = &tmp[i];
                }
                else {
                    fill->merge(tmp[i]);
                }
            }
        }

        if (fill) {
            swap(*fill);
        }
    }

    template<typename T>
    void list<T>::splice(const_iterator _pos, list& _other) {
        if (_other.empty()) return;
        transfer(const_cast<Node*>(_pos.get_ptr()),
            _other.sentinel_->next,
            _other.sentinel_);
        size_ += _other.size_;
        _other.size_ = 0;
        _other.sentinel_->next = _other.sentinel_;
        _other.sentinel_->prev = _other.sentinel_;
    }

    template<typename T>
    void list<T>::splice(const_iterator _pos, list& _other, const_iterator _it) {
        Node* curr = const_cast<Node*>(_it.get_ptr());
        transfer(const_cast<Node*>(_pos.get_ptr()), curr, curr->next);
        ++size_;
        --_other.size_;
    }

    template<typename T>
    void list<T>::splice(const_iterator _pos, list& _other,
        const_iterator _first, const_iterator _last) {
        if (_first == _last) return;

        Node* f = const_cast<Node*>(_first.get_ptr());
        Node* l = const_cast<Node*>(_last.get_ptr());

        size_t n = 0;
        for (auto it = _first; it != _last; ++it) ++n;

        transfer(const_cast<Node*>(_pos.get_ptr()), f, l);
        size_ += n;
        _other.size_ -= n;
    }


    template<typename T>
    list<T> list<T>::operator+(const list& _other) const {
        list result(*this);
        result += _other;
        return result;
    }

    template<typename T>
    list<T> list<T>::operator-(const list& _other) const {
        if (size_ != _other.size_) {
            throw std::invalid_argument("list sizes must match for subtraction");
        }
        list result;
        auto it1 = begin();
        auto it2 = _other.begin();
        while (it1 != end()) {
            result.push_back(*it1 - *it2);
            ++it1; ++it2;
        }
        return result;
    }

    template<typename T>
    list<T> list<T>::operator*(T _scalar) const {
        list result;
        for (const auto& val : *this) {
            result.push_back(val * _scalar);
        }
        return result;
    }

    template<typename T>
    list<T>& list<T>::operator+=(const list& _other) {
        for (const auto& val : _other) {
            push_back(val);
        }
        return *this;
    }

    template<typename T>
    list<T>& list<T>::operator-=(const list& _other) {
        if (size_ != _other.size_) {
            throw std::invalid_argument("list sizes must match for subtraction");
        }
        auto it1 = begin();
        auto it2 = _other.begin();
        while (it1 != end()) {
            *it1 -= *it2;
            ++it1; ++it2;
        }
        return *this;
    }

    template<typename T>
    list<T>& list<T>::operator*=(T _scalar) {
        for (auto& val : *this) {
            val *= _scalar;
        }
        return *this;
    }

    template<typename T>
    list<T> operator*(T _scalar, const list<T>& _vec) {
        return _vec * _scalar;
    }


    template<typename T>
    void list<T>::fill(const T& _value) {
        for (auto& val : *this) {
            val = _value;
        }
    }

    template<typename T>
    T list<T>::norm() const {
        T sum = 0;
        for (const auto& val : *this) {
            sum += val * val;
        }
        return std::sqrt(sum);
    }

    template<typename T>
    void list<T>::normalize() {
        T n = norm();
        if (n > T(1e-10)) {
            for (auto& val : *this) {
                val /= n;
            }
        }
    }

    template<typename T>
    T list<T>::dot(const list& _other) const {
        if (size_ != _other.size_) {
            throw std::invalid_argument("list sizes must match for dot product");
        }
        T result = 0;
        auto it1 = begin();
        auto it2 = _other.begin();
        while (it1 != end()) {
            result += *it1 * *it2;
            ++it1; ++it2;
        }
        return result;
    }


    template<typename T>
    list<T> list<T>::zeros(size_t _size) {
        return list(_size);
    }

    template<typename T>
    list<T> list<T>::ones(size_t _size) {
        list result(_size);
        for (auto& val : result) {
            val = T(1);
        }
        return result;
    }

    template<typename T>
    list<T> list<T>::random(size_t _size, T _min, T _max) {
        static std::random_device rd;
        static std::mt19937 gen(rd());

        list result;
        if constexpr (std::is_floating_point<T>::value) {
            std::uniform_real_distribution<T> dis(_min, _max);
            for (size_t i = 0; i < _size; ++i) {
                result.push_back(dis(gen));
            }
        }
        else {
            std::uniform_int_distribution<T> dis(_min, _max);
            for (size_t i = 0; i < _size; ++i) {
                result.push_back(dis(gen));
            }
        }
        return result;
    }


    template<typename T>
    bool list<T>::operator==(const list& _other) const {
        if (size_ != _other.size_) return false;

        auto it1 = begin();
        auto it2 = _other.begin();

        while (it1 != end()) {
            if (!(*it1 == *it2)) return false;
            ++it1; ++it2;
        }
        return true;
    }

    template<typename T>
    bool list<T>::operator!=(const list& _other) const {
        return !(*this == _other);
    }


    template<typename T>
    const T* list<T>::data() const {
        static std::vector<T> cache;
        cache.clear();
        cache.reserve(size_);
        for (const auto& val : *this) {
            cache.push_back(val);
        }
        return cache.data();
    }


    template<typename T>
    void list<T>::swap(list& _other) noexcept {
        std::swap(sentinel_, _other.sentinel_);
        std::swap(size_, _other.size_);
    }

    template<typename T>
    void swap(list<T>& _a, list<T>& _b) noexcept {
        _a.swap(_b);
    }

} // namespace stk

#endif // STK_LIST_TCC