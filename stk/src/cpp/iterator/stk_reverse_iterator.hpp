#ifndef STK_REVERSE_ITERATOR_HPP
#define STK_REVERSE_ITERATOR_HPP

#include "stk_base_iterator.hpp"

namespace stk {

    template<typename Iterator>
    class reverse_iterator {
    private:
        Iterator current_;

    public:
        using iterator_type = Iterator;
        using iterator_category = typename iterator_traits<Iterator>::iterator_category;
        using value_type = typename iterator_traits<Iterator>::value_type;
        using difference_type = typename iterator_traits<Iterator>::difference_type;
        using pointer = typename iterator_traits<Iterator>::pointer;
        using reference = typename iterator_traits<Iterator>::reference;

        reverse_iterator() : current_() {}
        explicit reverse_iterator(Iterator it) : current_(it) {}

        template<typename U>
        reverse_iterator(const reverse_iterator<U>& other) : current_(other.base()) {}

        Iterator base() const { return current_; }

        reference operator*() const {
            Iterator tmp = current_;
            --tmp;
            return *tmp;
        }

        pointer operator->() const {
            return &(operator*());
        }

        reverse_iterator& operator++() {
            --current_;
            return *this;
        }

        reverse_iterator operator++(int) {
            reverse_iterator tmp = *this;
            --current_;
            return tmp;
        }

        reverse_iterator& operator--() {
            ++current_;
            return *this;
        }

        reverse_iterator operator--(int) {
            reverse_iterator tmp = *this;
            ++current_;
            return tmp;
        }

        reverse_iterator operator+(difference_type n) const {
            return reverse_iterator(current_ - n);
        }

        reverse_iterator& operator+=(difference_type n) {
            current_ -= n;
            return *this;
        }

        reverse_iterator operator-(difference_type n) const {
            return reverse_iterator(current_ + n);
        }

        reverse_iterator& operator-=(difference_type n) {
            current_ += n;
            return *this;
        }

        reference operator[](difference_type n) const {
            return *(*this + n);
        }

        bool operator==(const reverse_iterator& other) const {
            return current_ == other.current_;
        }

        bool operator!=(const reverse_iterator& other) const {
            return current_ != other.current_;
        }

        bool operator<(const reverse_iterator& other) const {
            return current_ > other.current_;
        }

        bool operator>(const reverse_iterator& other) const {
            return current_ < other.current_;
        }

        bool operator<=(const reverse_iterator& other) const {
            return current_ >= other.current_;
        }

        bool operator>=(const reverse_iterator& other) const {
            return current_ <= other.current_;
        }
    };

} // namespace stk

#endif