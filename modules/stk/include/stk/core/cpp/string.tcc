#ifndef STK_CORE_CXX_STRING_TCC
#define STK_CORE_CXX_STRING_TCC

#include <algorithm>
#include <cctype>
#include <cstdio>
#include <cstdlib>
#include <stdexcept>

namespace stk {

const std::size_t string::npos;

// ============================================================================
// 内部辅助
// ============================================================================

void string::grow(std::size_t min_capacity)
{
    if (min_capacity <= capacity_) {
        return;
    }

    std::size_t new_cap = (capacity_ == 0) ? 16 : capacity_ * 2;
    while (new_cap < min_capacity) {
        new_cap *= 2;
    }

    char* new_data = new char[new_cap + 1];
    if (data_ != nullptr) {
        std::copy(data_, data_ + size_, new_data);
        delete[] data_;
    }
    data_ = new_data;
    capacity_ = new_cap;
    data_[size_] = '\0';
}

void string::copy_data(const char* src, std::size_t len)
{
    if (len == 0) {
        return;
    }
    std::copy(src, src + len, data_);
    data_[len] = '\0';
}

// ============================================================================
// 构造 / 析构
// ============================================================================

string::string() = default;

string::string(const char* str)
{
    if (str != nullptr) {
        size_ = std::strlen(str);
        grow(size_);
        copy_data(str, size_);
    }
}

string::string(std::size_t count, char ch)
{
    if (count > 0) {
        size_ = count;
        grow(size_);
        for (std::size_t i = 0; i < size_; ++i) {
            data_[i] = ch;
        }
        data_[size_] = '\0';
    }
}

string::string(const string& other)
{
    if (other.size_ > 0) {
        size_ = other.size_;
        grow(size_);
        copy_data(other.data_, size_);
    }
}

string::string(string&& other) noexcept
    : data_(other.data_), size_(other.size_), capacity_(other.capacity_)
{
    other.data_ = nullptr;
    other.size_ = 0;
    other.capacity_ = 0;
}

string::~string()
{
    if (data_ != nullptr) {
        delete[] data_;
        data_ = nullptr;
    }
}

// ============================================================================
// 赋值
// ============================================================================

string& string::operator=(const string& other)
{
    if (this != &other) {
        if (other.size_ > capacity_) {
            grow(other.size_);
        }
        size_ = other.size_;
        copy_data(other.data_, size_);
    }
    return *this;
}

string& string::operator=(string&& other) noexcept
{
    if (this != &other) {
        delete[] data_;
        data_ = other.data_;
        size_ = other.size_;
        capacity_ = other.capacity_;
        other.data_ = nullptr;
        other.size_ = 0;
        other.capacity_ = 0;
    }
    return *this;
}

string& string::operator=(const char* str)
{
    if (str != nullptr) {
        std::size_t len = std::strlen(str);
        if (len > capacity_) {
            grow(len);
        }
        size_ = len;
        copy_data(str, size_);
    } else {
        clear();
    }
    return *this;
}

// ============================================================================
// 元素访问
// ============================================================================

char& string::operator[](std::size_t index)
{
    return data_[index];
}

const char& string::operator[](std::size_t index) const
{
    return data_[index];
}

char& string::at(std::size_t index)
{
    if (index >= size_) {
        throw std::out_of_range("string::at: index out of range");
    }
    return data_[index];
}

const char& string::at(std::size_t index) const
{
    if (index >= size_) {
        throw std::out_of_range("string::at: index out of range");
    }
    return data_[index];
}

char& string::front()
{
    return data_[0];
}

const char& string::front() const
{
    return data_[0];
}

char& string::back()
{
    return data_[size_ - 1];
}

const char& string::back() const
{
    return data_[size_ - 1];
}

// ============================================================================
// 容量
// ============================================================================

std::size_t string::size() const
{
    return size_;
}

std::size_t string::length() const
{
    return size_;
}

bool string::empty() const
{
    return size_ == 0;
}

void string::clear()
{
    if (data_ != nullptr) {
        data_[0] = '\0';
    }
    size_ = 0;
}

void string::resize(std::size_t new_size)
{
    resize(new_size, '\0');
}

void string::resize(std::size_t new_size, char ch)
{
    if (new_size > capacity_) {
        grow(new_size);
    }
    if (new_size > size_) {
        for (std::size_t i = size_; i < new_size; ++i) {
            data_[i] = ch;
        }
    }
    size_ = new_size;
    data_[size_] = '\0';
}

void string::reserve(std::size_t new_cap)
{
    if (new_cap > capacity_) {
        grow(new_cap);
    }
}

std::size_t string::capacity() const
{
    return capacity_;
}

void string::shrink_to_fit()
{
    if (size_ < capacity_) {
        char* new_data = new char[size_ + 1];
        std::copy(data_, data_ + size_, new_data);
        new_data[size_] = '\0';
        delete[] data_;
        data_ = new_data;
        capacity_ = size_;
    }
}

// ============================================================================
// 修改器
// ============================================================================

void string::push_back(char ch)
{
    if (size_ + 1 > capacity_) {
        grow(size_ + 1);
    }
    data_[size_++] = ch;
    data_[size_] = '\0';
}

void string::pop_back()
{
    if (size_ > 0) {
        --size_;
        data_[size_] = '\0';
    }
}

string& string::append(const string& str)
{
    return append(str.data_, str.size_);
}

string& string::append(const char* str)
{
    if (str != nullptr) {
        return append(str, std::strlen(str));
    }
    return *this;
}

string& string::append(std::size_t count, char ch)
{
    if (count > 0) {
        if (size_ + count > capacity_) {
            grow(size_ + count);
        }
        for (std::size_t i = 0; i < count; ++i) {
            data_[size_ + i] = ch;
        }
        size_ += count;
        data_[size_] = '\0';
    }
    return *this;
}

string& string::append(const char* str, std::size_t len)
{
    if (len > 0 && str != nullptr) {
        if (size_ + len > capacity_) {
            grow(size_ + len);
        }
        std::copy(str, str + len, data_ + size_);
        size_ += len;
        data_[size_] = '\0';
    }
    return *this;
}

string& string::operator+=(const string& str)
{
    return append(str);
}

string& string::operator+=(const char* str)
{
    return append(str);
}

string& string::operator+=(char ch)
{
    push_back(ch);
    return *this;
}

string& string::insert(std::size_t pos, const string& str)
{
    return insert(pos, str.data_, str.size_);
}

string& string::insert(std::size_t pos, const char* str)
{
    if (str != nullptr) {
        return insert(pos, str, std::strlen(str));
    }
    return *this;
}

string& string::insert(std::size_t pos, std::size_t count, char ch)
{
    if (pos > size_) {
        throw std::out_of_range("string::insert: pos out of range");
    }
    if (count == 0) {
        return *this;
    }

    if (size_ + count > capacity_) {
        grow(size_ + count);
    }

    for (std::size_t i = size_; i > pos; --i) {
        data_[i + count - 1] = data_[i - 1];
    }

    for (std::size_t i = 0; i < count; ++i) {
        data_[pos + i] = ch;
    }

    size_ += count;
    data_[size_] = '\0';
    return *this;
}

string& string::insert(std::size_t pos, const char* str, std::size_t len)
{
    if (pos > size_) {
        throw std::out_of_range("string::insert: pos out of range");
    }
    if (len == 0 || str == nullptr) {
        return *this;
    }

    if (size_ + len > capacity_) {
        grow(size_ + len);
    }

    for (std::size_t i = size_; i > pos; --i) {
        data_[i + len - 1] = data_[i - 1];
    }

    std::copy(str, str + len, data_ + pos);

    size_ += len;
    data_[size_] = '\0';
    return *this;
}

string& string::erase(std::size_t pos, std::size_t len)
{
    if (pos >= size_) {
        return *this;
    }

    if (len == npos || pos + len > size_) {
        len = size_ - pos;
    }

    for (std::size_t i = pos; i + len < size_; ++i) {
        data_[i] = data_[i + len];
    }

    size_ -= len;
    data_[size_] = '\0';
    return *this;
}

string& string::replace(std::size_t pos, std::size_t len, const string& str)
{
    return replace(pos, len, str.data_, str.size_);
}

string& string::replace(std::size_t pos, std::size_t len, const char* str)
{
    if (str != nullptr) {
        return replace(pos, len, str, std::strlen(str));
    }
    return *this;
}

string& string::replace(std::size_t pos, std::size_t len, const char* str, std::size_t str_len)
{
    if (pos >= size_) {
        throw std::out_of_range("string::replace: pos out of range");
    }

    if (len == npos || pos + len > size_) {
        len = size_ - pos;
    }

    string temp;
    temp.reserve(size_ - len + str_len);
    temp.append(data_, pos);
    temp.append(str, str_len);
    temp.append(data_ + pos + len, size_ - pos - len);
    swap(temp);

    return *this;
}

void string::swap(string& other)
{
    std::swap(data_, other.data_);
    std::swap(size_, other.size_);
    std::swap(capacity_, other.capacity_);
}

// ============================================================================
// 查找
// ============================================================================

std::size_t string::find(const string& str, std::size_t pos) const
{
    return find(str.data_, pos);
}

std::size_t string::find(const char* str, std::size_t pos) const
{
    if (str == nullptr || pos > size_) {
        return npos;
    }

    std::size_t len = std::strlen(str);
    if (len == 0) {
        return pos;
    }
    if (pos + len > size_) {
        return npos;
    }

    for (std::size_t i = pos; i <= size_ - len; ++i) {
        if (std::memcmp(data_ + i, str, len) == 0) {
            return i;
        }
    }
    return npos;
}

std::size_t string::find(char ch, std::size_t pos) const
{
    if (pos >= size_) {
        return npos;
    }

    for (std::size_t i = pos; i < size_; ++i) {
        if (data_[i] == ch) {
            return i;
        }
    }
    return npos;
}

std::size_t string::rfind(const string& str, std::size_t pos) const
{
    return rfind(str.data_, pos);
}

std::size_t string::rfind(const char* str, std::size_t pos) const
{
    if (str == nullptr) {
        return npos;
    }

    std::size_t len = std::strlen(str);
    if (len == 0) {
        return pos < size_ ? pos : size_;
    }

    std::size_t start = (pos < size_ - len) ? pos : size_ - len;
    for (std::size_t i = start; i > 0; --i) {
        if (std::memcmp(data_ + i, str, len) == 0) {
            return i;
        }
    }
    if (std::memcmp(data_, str, len) == 0) {
        return 0;
    }
    return npos;
}

std::size_t string::rfind(char ch, std::size_t pos) const
{
    if (size_ == 0) {
        return npos;
    }

    std::size_t start = (pos < size_) ? pos : size_ - 1;
    for (std::size_t i = start; i > 0; --i) {
        if (data_[i] == ch) {
            return i;
        }
    }
    if (data_[0] == ch) {
        return 0;
    }
    return npos;
}

// ============================================================================
// 子串
// ============================================================================

string string::substr(std::size_t pos, std::size_t len) const
{
    if (pos > size_) {
        pos = size_;
    }
    if (len == npos || pos + len > size_) {
        len = size_ - pos;
    }

    string result;
    if (len > 0) {
        result.resize(len);
        std::copy(data_ + pos, data_ + pos + len, result.data_);
    }
    return result;
}

// ============================================================================
// 比较
// ============================================================================

int string::compare(const string& str) const
{
    return compare(str.data_);
}

int string::compare(const char* str) const
{
    if (str == nullptr) {
        return 1;
    }
    return std::strcmp(data_ != nullptr ? data_ : "", str);
}

// ============================================================================
// 转换
// ============================================================================

void string::to_upper()
{
    if (data_ == nullptr) {
        return;
    }
    for (std::size_t i = 0; i < size_; ++i) {
        data_[i] = static_cast<char>(std::toupper(static_cast<unsigned char>(data_[i])));
    }
}

void string::to_lower()
{
    if (data_ == nullptr) {
        return;
    }
    for (std::size_t i = 0; i < size_; ++i) {
        data_[i] = static_cast<char>(std::tolower(static_cast<unsigned char>(data_[i])));
    }
}

const char* string::c_str() const
{
    return data_ != nullptr ? data_ : "";
}

// ============================================================================
// 关系运算符
// ============================================================================

bool operator==(const string& lhs, const string& rhs)
{
    return lhs.compare(rhs) == 0;
}

bool operator==(const string& lhs, const char* rhs)
{
    return lhs.compare(rhs) == 0;
}

bool operator==(const char* lhs, const string& rhs)
{
    return rhs.compare(lhs) == 0;
}

bool operator!=(const string& lhs, const string& rhs)
{
    return !(lhs == rhs);
}

bool operator<(const string& lhs, const string& rhs)
{
    return lhs.compare(rhs) < 0;
}

bool operator<=(const string& lhs, const string& rhs)
{
    return lhs.compare(rhs) <= 0;
}

bool operator>(const string& lhs, const string& rhs)
{
    return lhs.compare(rhs) > 0;
}

bool operator>=(const string& lhs, const string& rhs)
{
    return lhs.compare(rhs) >= 0;
}

// ============================================================================
// 字符串连接
// ============================================================================

string operator+(const string& lhs, const string& rhs)
{
    string result(lhs);
    result += rhs;
    return result;
}

string operator+(const string& lhs, const char* rhs)
{
    string result(lhs);
    result += rhs;
    return result;
}

string operator+(const char* lhs, const string& rhs)
{
    string result(lhs);
    result += rhs;
    return result;
}

string operator+(const string& lhs, char rhs)
{
    string result(lhs);
    result.push_back(rhs);
    return result;
}

string operator+(char lhs, const string& rhs)
{
    string result(1, lhs);
    result += rhs;
    return result;
}

} // namespace stk

#endif // STK_CORE_CXX_STRING_TCC