#ifndef _STK_VECTOR_TCC
#define _STK_VECTOR_TCC

#include <cmath>
#include <algorithm>
#include <stdexcept>
#include <random>
#include <numeric>

namespace stk {

    template<typename T>
    vector<T>::vector() : data_(nullptr), size_(0) {}

    template<typename T>
    vector<T>::vector(size_t _size) : size_(_size)
    {
        data_ = new T[size_]();
    }

    template<typename T>
    vector<T>::vector(std::initializer_list<T> _list) : size_(_list.size())
    {
        data_ = new T[size_];
        std::copy(_list.begin(), _list.end(), data_);
    }

    template<typename T>
    vector<T>::vector(const vector& _other) : size_(_other.size_)
    {
        data_ = new T[size_];
        std::copy(_other.data_, _other.data_ + size_, data_);
    }

    template<typename T>
    vector<T>::vector(vector&& _other) noexcept : data_(_other.data_), size_(_other.size_)
    {
        _other.data_ = nullptr;
        _other.size_ = 0;
    }

    template<typename T>
    vector<T>::~vector()
    {
        delete[] data_;
        data_ = nullptr;
        size_ = 0;
    }

    template<typename T>
    vector<T>& vector<T>::operator=(const vector& _other)
    {
        if (this != &_other) {
            delete[] data_;
            size_ = _other.size_;
            data_ = new T[size_];
            std::copy(_other.data_, _other.data_ + size_, data_);
        }
        return *this;
    }

    template<typename T>
    vector<T>& vector<T>::operator=(vector&& _other) noexcept
    {
        if (this != &_other) {
            delete[] data_;
            data_ = _other.data_;
            size_ = _other.size_;
            _other.data_ = nullptr;
            _other.size_ = 0;
        }
        return *this;
    }

    template<typename T>
    T vector<T>::operator[](size_t _index) const
    {
        if (_index >= size_) {
            throw std::out_of_range("vector index out of range");
        }
        return data_[_index];
    }

    template<typename T>
    T& vector<T>::operator[](size_t _index)
    {
        if (_index >= size_) {
            throw std::out_of_range("vector index out of range");
        }
        return data_[_index];
    }

    template<typename T>
    vector<T> vector<T>::operator+(const vector& _other) const
    {
        if (size_ != _other.size_) {
            throw std::invalid_argument("vector sizes must match for addition");
        }
        vector result(size_);
        for (size_t i = 0; i < size_; ++i) {
            result.data_[i] = data_[i] + _other.data_[i];
        }
        return result;
    }

    template<typename T>
    vector<T> vector<T>::operator-(const vector& _other) const
    {
        if (size_ != _other.size_) {
            throw std::invalid_argument("vector sizes must match for subtraction");
        }
        vector result(size_);
        for (size_t i = 0; i < size_; ++i) {
            result.data_[i] = data_[i] - _other.data_[i];
        }
        return result;
    }

    template<typename T>
    vector<T> vector<T>::operator*(T _scalar) const
    {
        vector result(size_);
        for (size_t i = 0; i < size_; ++i) {
            result.data_[i] = data_[i] * _scalar;
        }
        return result;
    }

    template<typename T>
    vector<T>& vector<T>::operator+=(const vector& _other)
    {
        if (size_ != _other.size_) {
            throw std::invalid_argument("vector sizes must match");
        }
        for (size_t i = 0; i < size_; ++i) {
            data_[i] += _other.data_[i];
        }
        return *this;
    }

    template<typename T>
    vector<T>& vector<T>::operator-=(const vector& _other)
    {
        if (size_ != _other.size_) {
            throw std::invalid_argument("vector sizes must match");
        }
        for (size_t i = 0; i < size_; ++i) {
            data_[i] -= _other.data_[i];
        }
        return *this;
    }

    template<typename T>
    vector<T>& vector<T>::operator*=(T _scalar)
    {
        for (size_t i = 0; i < size_; ++i) {
            data_[i] *= _scalar;
        }
        return *this;
    }

    template<typename T>
    size_t vector<T>::size() const
    {
        return size_;
    }

    template<typename T>
    void vector<T>::resize(size_t _size)
    {
        if (size_ == _size) return;

        T* newData = new T[_size]();
        size_t copySize = std::min(size_, _size);
        std::copy(data_, data_ + copySize, newData);

        delete[] data_;
        data_ = newData;
        size_ = _size;
    }

    template<typename T>
    void vector<T>::normalize()
    {
        T n = norm();
        if (n > 1e-10) {
            for (size_t i = 0; i < size_; ++i) {
                data_[i] /= n;
            }
        }
    }

    template<typename T>
    T vector<T>::norm() const
    {
        T sum = 0.0;
        for (size_t i = 0; i < size_; ++i) {
            sum += data_[i] * data_[i];
        }
        return std::sqrt(sum);
    }

    template<typename T>
    T vector<T>::dot(const vector& _other) const
    {
        if (size_ != _other.size_) {
            throw std::invalid_argument("vector sizes must match for dot product");
        }
        T result = 0.0;
        for (size_t i = 0; i < size_; ++i) {
            result += data_[i] * _other.data_[i];
        }
        return result;
    }

    template<typename T>
    void vector<T>::fill(T _value)
    {
        for (size_t i = 0; i < size_; ++i) {
            data_[i] = _value;
        }
    }

    template<typename T>
    const T* vector<T>::data() const
    {
        return data_;
    }

    template<typename T>
    vector<T> vector<T>::zeros(size_t _size)
    {
        return vector(_size);
    }

    template<typename T>
    vector<T> vector<T>::ones(size_t _size)
    {
        vector result(_size);
        for (size_t i = 0; i < _size; ++i) {
            result.data_[i] = 1.0;
        }
        return result;
    }

    template<typename T>
    vector<T> vector<T>::random(size_t _size, T _min, T _max)
    {
        static std::random_device rd;
        static std::mt19937 gen(rd());
        std::uniform_real_distribution<> dis(_min, _max);

        vector result(_size);
        for (size_t i = 0; i < _size; ++i) {
            result.data_[i] = static_cast<T>(dis(gen));
        }
        return result;
    }

}

#endif
