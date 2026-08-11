#ifndef STK_CORE_CXX_VECTOR_H
#define STK_CORE_CXX_VECTOR_H

#include <cstddef>
#include <initializer_list>

namespace stk {

template <typename T>
class vector {
public:
    // 构造 / 析构
    vector();
    explicit vector(std::size_t size);
    vector(std::initializer_list<T> list);

    vector(const vector& other);
    vector(vector&& other) noexcept;

    ~vector();

    // 赋值
    vector& operator=(const vector& other);
    vector& operator=(vector&& other) noexcept;

    // 元素访问
    T operator[](std::size_t index) const;
    T& operator[](std::size_t index);

    // 算术运算
    vector operator+(const vector& other) const;
    vector operator-(const vector& other) const;
    vector operator*(T scalar) const;

    template <typename U>
    friend vector<U> operator*(U scalar, const vector<U>& vec);

    vector& operator+=(const vector& other);
    vector& operator-=(const vector& other);
    vector& operator*=(T scalar);

    // 容量
    std::size_t size() const;
    void resize(std::size_t new_size);

    // 运算
    void normalize();
    T norm() const;
    T dot(const vector& other) const;

    // 工具
    void fill(T value);
    const T* data() const;

    // 静态工厂
    static vector zeros(std::size_t size);
    static vector ones(std::size_t size);
    static vector random(std::size_t size, T min = 0.0, T max = 1.0);

private:
    T* data_ = nullptr;
    std::size_t size_ = 0;
};

template <typename T>
inline vector<T> operator*(T scalar, const vector<T>& vec)
{
    return vec * scalar;
}

} // namespace stk

#include "vector.tcc"

#endif // STK_CORE_CXX_VECTOR_H