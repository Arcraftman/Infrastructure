#ifndef STK_CORE_CXX_STRING_H
#define STK_CORE_CXX_STRING_H

#include <cstddef>
#include <cstring>

namespace stk {

class string {
public:
    // 常量
    static const std::size_t npos = static_cast<std::size_t>(-1);

    // 构造 / 析构
    string();
    explicit string(const char* str);
    string(std::size_t count, char ch);
    string(const string& other);
    string(string&& other) noexcept;
    ~string();

    // 赋值
    string& operator=(const string& other);
    string& operator=(string&& other) noexcept;
    string& operator=(const char* str);

    // 元素访问
    char& operator[](std::size_t index);
    const char& operator[](std::size_t index) const;
    char& at(std::size_t index);
    const char& at(std::size_t index) const;
    char& front();
    const char& front() const;
    char& back();
    const char& back() const;

    // 容量
    std::size_t size() const;
    std::size_t length() const;
    bool empty() const;
    void clear();
    void resize(std::size_t new_size);
    void resize(std::size_t new_size, char ch);
    void reserve(std::size_t new_cap);
    std::size_t capacity() const;
    void shrink_to_fit();

    // 修改器
    void push_back(char ch);
    void pop_back();
    string& append(const string& str);
    string& append(const char* str);
    string& append(std::size_t count, char ch);
    string& append(const char* str, std::size_t len);

    string& operator+=(const string& str);
    string& operator+=(const char* str);
    string& operator+=(char ch);

    string& insert(std::size_t pos, const string& str);
    string& insert(std::size_t pos, const char* str);
    string& insert(std::size_t pos, std::size_t count, char ch);
    string& insert(std::size_t pos, const char* str, std::size_t len);

    string& erase(std::size_t pos = 0, std::size_t len = npos);
    string& replace(std::size_t pos, std::size_t len, const string& str);
    string& replace(std::size_t pos, std::size_t len, const char* str);
    string& replace(std::size_t pos, std::size_t len, const char* str, std::size_t str_len);

    void swap(string& other);

    // 查找
    std::size_t find(const string& str, std::size_t pos = 0) const;
    std::size_t find(const char* str, std::size_t pos = 0) const;
    std::size_t find(char ch, std::size_t pos = 0) const;
    std::size_t rfind(const string& str, std::size_t pos = npos) const;
    std::size_t rfind(const char* str, std::size_t pos = npos) const;
    std::size_t rfind(char ch, std::size_t pos = npos) const;

    // 子串
    string substr(std::size_t pos = 0, std::size_t len = npos) const;

    // 比较
    int compare(const string& str) const;
    int compare(const char* str) const;

    // 转换
    void to_upper();
    void to_lower();
    const char* c_str() const;

private:
    char* data_ = nullptr;
    std::size_t size_ = 0;
    std::size_t capacity_ = 0;

    void grow(std::size_t min_capacity);
    void copy_data(const char* src, std::size_t len);
};

// 关系运算符
bool operator==(const string& lhs, const string& rhs);
bool operator==(const string& lhs, const char* rhs);
bool operator==(const char* lhs, const string& rhs);
bool operator!=(const string& lhs, const string& rhs);
bool operator<(const string& lhs, const string& rhs);
bool operator<=(const string& lhs, const string& rhs);
bool operator>(const string& lhs, const string& rhs);
bool operator>=(const string& lhs, const string& rhs);

// 字符串连接
string operator+(const string& lhs, const string& rhs);
string operator+(const string& lhs, const char* rhs);
string operator+(const char* lhs, const string& rhs);
string operator+(const string& lhs, char rhs);
string operator+(char lhs, const string& rhs);

} // namespace stk

#include "string.tcc"

#endif // STK_CORE_CXX_STRING_H