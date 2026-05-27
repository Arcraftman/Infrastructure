#ifndef STK_SRC_CORE_STR_H
#define STK_SRC_CORE_STR_H


#include "config.h"

typedef struct str{
    char* data;
    size_t len;
} str;

STK_API void str_init(str* s);
STK_API void str_init_from(str* s, const char* cstr);
STK_API void str_free(str* s);
STK_API const char* str_cstr(const str* s);
STK_API size_t str_len(const str* s);
STK_API bool str_empty(const str* s);
STK_API void str_clear(str* s);
STK_API void str_push(str* s, char ch);
STK_API void str_pop(str* s);
STK_API void str_append(str* s, const char* cstr);
STK_API void str_append_str(str* s, const str* other);
STK_API void str_assign(str* s, const char* cstr);
STK_API void str_assign_str(str* s, const str* other);
STK_API int str_find_char(const str* s, char ch, size_t start);
STK_API int str_find(const str* s, const char* substr, size_t start);
STK_API int str_cmp(const void* s1, const void* s2);
STK_API int str_cmp_cstr(const str* s, const char* cstr);
STK_API void str_to_upper(str* s);
STK_API void str_to_lower(str* s);
STK_API str* str_sub(const str* s, size_t start, size_t end);
STK_API void str_print(const str* s);
STK_API void str_println(const str* s);
STK_API void str_copy(str* dest, const str* src);

#endif