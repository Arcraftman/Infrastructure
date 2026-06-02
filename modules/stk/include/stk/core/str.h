#ifndef STK_CORE_STR_H
#define STK_CORE_STR_H



#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    char* data;
    size_t len;
} stk_str;

STK_API STK_STATUS stk_str_init(stk_str* s);
STK_API STK_STATUS stk_str_init_from(stk_str* s, const char* cstr);
STK_API STK_STATUS stk_str_free(stk_str* s);
STK_API const char* stk_str_cstr(const stk_str* s);
STK_API size_t      stk_str_len(const stk_str* s);
STK_API bool        stk_str_empty(const stk_str* s);
STK_API STK_STATUS  stk_str_clear(stk_str* s);
STK_API STK_STATUS  stk_str_push(stk_str* s, char ch);
STK_API STK_STATUS  stk_str_pop(stk_str* s);
STK_API STK_STATUS  stk_str_append(stk_str* s, const char* cstr);
STK_API STK_STATUS  stk_str_append_str(stk_str* s, const stk_str* other);
STK_API STK_STATUS  stk_str_assign(stk_str* s, const char* cstr);
STK_API STK_STATUS  stk_str_assign_str(stk_str* s, const stk_str* other);
STK_API int         stk_str_find_char(const stk_str* s, char ch, size_t start);
STK_API int         stk_str_find(const stk_str* s, const char* substr, size_t start);
STK_API int         stk_str_cmp(const stk_str* s1, const stk_str* s2);
STK_API int         stk_str_cmp_qsort(const void* a, const void* b);
STK_API int         stk_str_cmp_cstr(const stk_str* s, const char* cstr);
STK_API STK_STATUS  stk_str_to_upper(stk_str* s);
STK_API STK_STATUS  stk_str_to_lower(stk_str* s);
STK_API stk_str*    stk_str_sub(const stk_str* s, size_t start, size_t end);
STK_API void        stk_str_print(const stk_str* s);
STK_API void        stk_str_println(const stk_str* s);
STK_API STK_STATUS  stk_str_copy(stk_str* dest, const stk_str* src);

#ifdef __cplusplus
}
#endif

#endif /* STK_CORE_STR_H */