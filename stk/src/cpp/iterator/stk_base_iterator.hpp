#ifndef STK_BASE_ITERATOR_HPP
#define STK_BASE_ITERATOR_HPP

#include <cstddef>
#include <type_traits>

namespace stk {

    // ========== 迭代器类型标签 ==========
    struct input_iterator_tag {};
    struct output_iterator_tag {};
    struct forward_iterator_tag : input_iterator_tag {};
    struct bidirectional_iterator_tag : forward_iterator_tag {};
    struct random_access_iterator_tag : bidirectional_iterator_tag {};

    // ========== 迭代器基础模板 ==========
    template<typename Category, typename T,
        typename Distance = std::ptrdiff_t,
        typename Pointer = T*,
        typename Reference = T&>
        struct iterator_base {
        using iterator_category = Category;
        using value_type = T;
        using difference_type = Distance;
        using pointer = Pointer;
        using reference = Reference;
    };

    // ========== 迭代器特性 ==========
    template<typename Iterator>
    struct iterator_traits {
        using iterator_category = typename Iterator::iterator_category;
        using value_type = typename Iterator::value_type;
        using difference_type = typename Iterator::difference_type;
        using pointer = typename Iterator::pointer;
        using reference = typename Iterator::reference;
    };

    // 原生指针特化
    template<typename T>
    struct iterator_traits<T*> {
        using iterator_category = random_access_iterator_tag;
        using value_type = T;
        using difference_type = std::ptrdiff_t;
        using pointer = T*;
        using reference = T&;
    };

    template<typename T>
    struct iterator_traits<const T*> {
        using iterator_category = random_access_iterator_tag;
        using value_type = T;
        using difference_type = std::ptrdiff_t;
        using pointer = const T*;
        using reference = const T&;
    };

    // ========== 辅助函数 ==========
    template<typename Iterator>
    typename iterator_traits<Iterator>::difference_type
        distance(Iterator first, Iterator last) {
        typename iterator_traits<Iterator>::difference_type n = 0;
        while (first != last) {
            ++first;
            ++n;
        }
        return n;
    }

    template<typename Iterator>
    void advance(Iterator& it,
        typename iterator_traits<Iterator>::difference_type n) {
        using category = typename iterator_traits<Iterator>::iterator_category;
        if constexpr (std::is_base_of_v<random_access_iterator_tag, category>) {
            it += n;
        }
        else if constexpr (std::is_base_of_v<bidirectional_iterator_tag, category>) {
            if (n > 0) while (n--) ++it;
            else while (n++) --it;
        }
        else {
            while (n--) ++it;
        }
    }

} // namespace stk

#endif