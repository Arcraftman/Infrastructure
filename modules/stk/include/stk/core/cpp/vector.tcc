#ifndef STK_CORE_CXX_VECTOR_TCC
#define STK_CORE_CXX_VECTOR_TCC

#include <algorithm>
#include <cmath>
#include <random>
#include <stdexcept>

namespace stk {

// ============================================================================
// 构造 / 析构
// ============================================================================

template <typename T>
vector<T>::vector() = default;

template <typename T>
vector<T>::vector(std::size_t size) : size_(size)
{
    data_ = new T[size_]();
}

template <typename T>
vector<T>::vector(std::initializer_list<T> list) : size_(list.size())
{
    data_ = new T[size_];
    std::copy(list.begin(), list.end(), data_);
}

template <typename T>
vector<T>::vector(const vector& other) : size_(other.size_)
{
    data_ = new T[size_];
    std::copy(other.data_, other.data_ + size_, data_);
}

template <typename T>
vector<T>::vector(vector&& other) noexcept : data_(other.data_), size_(other.size_)
{
    other.data_ = nullptr;
    other.size_ = 0;
}

template <typename T>
vector<T>::~vector()
{
    delete[] data_;
}

// ============================================================================
// 赋值
// ============================================================================

template <typename T>
vector<T>& vector<T>::operator=(const vector& other)
{
    if (this != &other) {
        delete[] data_;
        size_ = other.size_;
        data_ = new T[size_];
        std::copy(other.data_, other.data_ + size_, data_);
    }
    return *this;
}

template <typename T>
vector<T>& vector<T>::operator=(vector&& other) noexcept
{
    if (this != &other) {
        delete[] data_;
        data_ = other.data_;
        size_ = other.size_;
        other.data_ = nullptr;
        other.size_ = 0;
    }
    return *this;
}

// ============================================================================
// 元素访问
// ============================================================================

template <typename T>
T vector<T>::operator[](std::size_t index) const
{
    if (index >= size_) {
        throw std::out_of_range("vector::operator[]: index out of range");
    }
    return data_[index];
}

template <typename T>
T& vector<T>::operator[](std::size_t index)
{
    if (index >= size_) {
        throw std::out_of_range("vector::operator[]: index out of range");
    }
    return data_[index];
}

// ============================================================================
// 算术运算
// ============================================================================

template <typename T>
vector<T> vector<T>::operator+(const vector& other) const
{
    if (size_ != other.size_) {
        throw std::invalid_argument("vector::operator+: sizes must match");
    }

    vector result(size_);
    for (std::size_t i = 0; i < size_; ++i) {
        result.data_[i] = data_[i] + other.data_[i];
    }
    return result;
}

template <typename T>
vector<T> vector<T>::operator-(const vector& other) const
{
    if (size_ != other.size_) {
        throw std::invalid_argument("vector::operator-: sizes must match");
    }

    vector result(size_);
    for (std::size_t i = 0; i < size_; ++i) {
        result.data_[i] = data_[i] - other.data_[i];
    }
    return result;
}

template <typename T>
vector<T> vector<T>::operator*(T scalar) const
{
    vector result(size_);
    for (std::size_t i = 0; i < size_; ++i) {
        result.data_[i] = data_[i] * scalar;
    }
    return result;
}

template <typename T>
vector<T>& vector<T>::operator+=(const vector& other)
{
    if (size_ != other.size_) {
        throw std::invalid_argument("vector::operator+=: sizes must match");
    }

    for (std::size_t i = 0; i < size_; ++i) {
        data_[i] += other.data_[i];
    }
    return *this;
}

template <typename T>
vector<T>& vector<T>::operator-=(const vector& other)
{
    if (size_ != other.size_) {
        throw std::invalid_argument("vector::operator-=: sizes must match");
    }

    for (std::size_t i = 0; i < size_; ++i) {
        data_[i] -= other.data_[i];
    }
    return *this;
}

template <typename T>
vector<T>& vector<T>::operator*=(T scalar)
{
    for (std::size_t i = 0; i < size_; ++i) {
        data_[i] *= scalar;
    }
    return *this;
}

// ============================================================================
// 容量
// ============================================================================

template <typename T>
std::size_t vector<T>::size() const
{
    return size_;
}

template <typename T>
void vector<T>::resize(std::size_t new_size)
{
    if (size_ == new_size) {
        return;
    }

    T* new_data = new T[new_size]();
    std::size_t copy_size = std::min(size_, new_size);
    std::copy(data_, data_ + copy_size, new_data);

    delete[] data_;
    data_ = new_data;
    size_ = new_size;
}

// ============================================================================
// 运算
// ============================================================================

template <typename T>
void vector<T>::normalize()
{
    T n = norm();
    if (n > T(1e-10)) {
        for (std::size_t i = 0; i < size_; ++i) {
            data_[i] /= n;
        }
    }
}

template <typename T>
T vector<T>::norm() const
{
    T sum = T(0);
    for (std::size_t i = 0; i < size_; ++i) {
        sum += data_[i] * data_[i];
    }
    return std::sqrt(sum);
}

template <typename T>
T vector<T>::dot(const vector& other) const
{
    if (size_ != other.size_) {
        throw std::invalid_argument("vector::dot: sizes must match");
    }

    T result = T(0);
    for (std::size_t i = 0; i < size_; ++i) {
        result += data_[i] * other.data_[i];
    }
    return result;
}

// ============================================================================
// 工具
// ============================================================================

template <typename T>
void vector<T>::fill(T value)
{
    for (std::size_t i = 0; i < size_; ++i) {
        data_[i] = value;
    }
}

template <typename T>
const T* vector<T>::data() const
{
    return data_;
}

// ============================================================================
// 静态工厂
// ============================================================================

template <typename T>
vector<T> vector<T>::zeros(std::size_t size)
{
    return vector<T>(size);
}

template <typename T>
vector<T> vector<T>::ones(std::size_t size)
{
    vector<T> result(size);
    for (std::size_t i = 0; i < size; ++i) {
        result.data_[i] = T(1);
    }
    return result;
}

template <typename T>
vector<T> vector<T>::random(std::size_t size, T min, T max)
{
    static std::random_device rd;
    static std::mt19937 gen(rd());
    std::uniform_real_distribution<> dis(min, max);

    vector<T> result(size);
    for (std::size_t i = 0; i < size; ++i) {
        result.data_[i] = static_cast<T>(dis(gen));
    }
    return result;
}

} // namespace stk

#endif // STK_CORE_CXX_VECTOR_TCC