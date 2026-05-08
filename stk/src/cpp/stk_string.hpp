#ifndef _STK_STRING_CXX_H
#define _STK_STRING_CXX_H

#pragma once

#include <cstddef>
#include <cstring>
#include <cstdio>
#include <cctype>

namespace stk {

    class string {
    public:
        // 静态常量
        static const size_t npos = static_cast<size_t>(-1);

        // ========== 构造函数 ==========
        string();
        explicit string(const char* str);
        string(size_t count, char ch);
        string(const string& other);
        string(string&& other) noexcept;
        ~string();

        // ========== 赋值运算符 ==========
        string& operator=(const string& other);
        string& operator=(string&& other) noexcept;
        string& operator=(const char* str);

        // ========== 元素访问 ==========
        char& operator[](size_t index);
        const char& operator[](size_t index) const;
        char& at(size_t index);
        const char& at(size_t index) const;
        char& front();
        const char& front() const;
        char& back();
        const char& back() const;

        // ========== 容量 ==========
        size_t size() const;
        size_t length() const;
        bool empty() const;
        void clear();
        void resize(size_t new_size);
        void resize(size_t new_size, char ch);
        void reserve(size_t new_cap);
        size_t capacity() const;
        void shrink_to_fit();

        // ========== 修改器 ==========
        void push_back(char ch);
        void pop_back();
        string& append(const string& str);
        string& append(const char* str);
        string& append(size_t count, char ch);
        string& append(const char* str, size_t len);

        string& operator+=(const string& str);
        string& operator+=(const char* str);
        string& operator+=(char ch);

        string& insert(size_t pos, const string& str);
        string& insert(size_t pos, const char* str);
        string& insert(size_t pos, size_t count, char ch);

        string& erase(size_t pos = 0, size_t len = npos);

        string& replace(size_t pos, size_t len, const string& str);
        string& replace(size_t pos, size_t len, const char* str);

        void swap(string& other);

        // ========== 查找 ==========
        size_t find(const string& str, size_t pos = 0) const;
        size_t find(const char* str, size_t pos = 0) const;
        size_t find(char ch, size_t pos = 0) const;
        size_t rfind(const string& str, size_t pos = npos) const;
        size_t rfind(const char* str, size_t pos = npos) const;
        size_t rfind(char ch, size_t pos = npos) const;

        size_t find_first_of(const string& str, size_t pos = 0) const;
        size_t find_first_of(const char* str, size_t pos = 0) const;
        size_t find_first_of(char ch, size_t pos = 0) const;
        size_t find_first_not_of(const string& str, size_t pos = 0) const;
        size_t find_first_not_of(const char* str, size_t pos = 0) const;
        size_t find_first_not_of(char ch, size_t pos = 0) const;
        size_t find_last_of(const string& str, size_t pos = npos) const;
        size_t find_last_of(const char* str, size_t pos = npos) const;
        size_t find_last_of(char ch, size_t pos = npos) const;
        size_t find_last_not_of(const string& str, size_t pos = npos) const;
        size_t find_last_not_of(const char* str, size_t pos = npos) const;
        size_t find_last_not_of(char ch, size_t pos = npos) const;

        bool starts_with(const string& str) const;
        bool starts_with(const char* str) const;
        bool starts_with(char ch) const;
        bool ends_with(const string& str) const;
        bool ends_with(const char* str) const;
        bool ends_with(char ch) const;

        // ========== 子串 ==========
        string substr(size_t pos = 0, size_t len = npos) const;

        // ========== 比较 ==========
        int compare(const string& str) const;
        int compare(const char* str) const;

        // ========== 转换 ==========
        void to_upper();
        void to_lower();
        const char* c_str() const;

        // ========== 输出 ==========
        void print() const;
        void println() const;

        // ========== 静态工厂 ==========
        static string from_cstr(const char* str);
        static string repeat(char ch, size_t count);
        static string join(const string* strings, size_t count, const string& separator);
        static string join(const char** strings, size_t count, const char* separator);

    private:
        char* data_;
        size_t size_;
        size_t capacity_;

        void grow(size_t min_capacity);
        void copy_data(const char* src, size_t len);
    };

    // ========== 关系运算符 ==========
    bool operator==(const string& lhs, const string& rhs);
    bool operator==(const string& lhs, const char* rhs);
    bool operator==(const char* lhs, const string& rhs);
    bool operator!=(const string& lhs, const string& rhs);
    bool operator<(const string& lhs, const string& rhs);
    bool operator<=(const string& lhs, const string& rhs);
    bool operator>(const string& lhs, const string& rhs);
    bool operator>=(const string& lhs, const string& rhs);

    // ========== 字符串连接 ==========
    string operator+(const string& lhs, const string& rhs);
    string operator+(const string& lhs, const char* rhs);
    string operator+(const char* lhs, const string& rhs);
    string operator+(const string& lhs, char rhs);
    string operator+(char lhs, const string& rhs);

}

#include "stk_string.tcc"

#endif