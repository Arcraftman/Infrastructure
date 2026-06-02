#include "stk/def.h"
#include "stk/utils/status.h"
#include "stk/utils/logger.h"
#include "stk/core/preset.h"
#include "stk/core/str.h"

#define STK_STRING_DEFAULT_CAPACITY 16
#define STK_STRING_GROW_FACTOR 2

STK_STATUS stk_str_init(stk_str *s) {
    STK_RETURN_IF(!s, STK_EINVAL, "String init: NULL string pointer");
    
    s->data = NULL;
    s->len = 0;
    
    STK_LOG_DEBUG("String initialized");
    return STK_OK;
}

STK_STATUS stk_str_init_from(stk_str *s, const char *cstr) {
    STK_RETURN_IF(!s, STK_EINVAL, "String init_from: NULL string pointer");
    
    if (!cstr || cstr[0] == '\0') {
        s->data = NULL;
        s->len = 0;
        return STK_OK;
    }
    
    s->len = strlen(cstr);
    s->data = (char *)malloc(s->len + 1);
    if (!s->data) {
        STK_LOG_ERROR("String init_from: failed to allocate %zu bytes", s->len + 1);
        s->len = 0;
        return STK_ENOMEM;
    }
    strcpy(s->data, cstr);
    
    STK_LOG_DEBUG("String init_from: len=%zu", s->len);
    return STK_OK;
}

STK_STATUS stk_str_free(stk_str *s) {
    if (!s) {
        STK_LOG_WARN("String free: NULL string pointer");
        return STK_EINVAL;
    }
    
    if (s->data) {
        free(s->data);
        s->data = NULL;
    }
    s->len = 0;
    
    STK_LOG_DEBUG("String freed");
    return STK_OK;
}

const char *stk_str_cstr(const stk_str *s) {
    return (s && s->data) ? s->data : "";
}

size_t stk_str_len(const stk_str *s) {
    return s ? s->len : 0;
}

bool stk_str_empty(const stk_str *s) {
    return s ? s->len == 0 : true;
}

STK_STATUS stk_str_clear(stk_str *s) {
    STK_RETURN_IF(!s, STK_EINVAL, "String clear: NULL string pointer");
    
    if (s->data) {
        s->data[0] = '\0';
    }
    s->len = 0;
    
    STK_LOG_DEBUG("String cleared");
    return STK_OK;
}

STK_STATUS stk_str_push(stk_str *s, char ch) {
    STK_RETURN_IF(!s, STK_EINVAL, "String push: NULL string pointer");
    
    char *new_data = (char *)realloc(s->data, s->len + 2);
    if (!new_data) {
        STK_LOG_ERROR("String push: realloc failed (size=%zu)", s->len + 2);
        return STK_ENOMEM;
    }
    
    s->data = new_data;
    s->data[s->len++] = ch;
    s->data[s->len] = '\0';
    
    STK_LOG_DEBUG("String push: '%c', new len=%zu", ch, s->len);
    return STK_OK;
}

STK_STATUS stk_str_pop(stk_str *s) {
    STK_RETURN_IF(!s, STK_EINVAL, "String pop: NULL string pointer");
    STK_RETURN_IF(s->len == 0, STK_EMPTY, "String pop: string is empty");
    
    s->len--;
    s->data[s->len] = '\0';
    
    if (s->len == 0) {
        free(s->data);
        s->data = NULL;
    }
    
    STK_LOG_DEBUG("String pop: new len=%zu", s->len);
    return STK_OK;
}

STK_STATUS stk_str_append(stk_str *s, const char *cstr) {
    STK_RETURN_IF(!s, STK_EINVAL, "String append: NULL string pointer");
    STK_RETURN_IF(!cstr, STK_EINVAL, "String append: NULL string");
    
    size_t add_len = strlen(cstr);
    if (add_len == 0) return STK_OK;
    
    char *new_data = (char *)realloc(s->data, s->len + add_len + 1);
    if (!new_data) {
        STK_LOG_ERROR("String append: realloc failed (size=%zu)", s->len + add_len + 1);
        return STK_ENOMEM;
    }
    
    s->data = new_data;
    memcpy(s->data + s->len, cstr, add_len + 1);
    s->len += add_len;
    
    STK_LOG_DEBUG("String append: added %zu bytes, new len=%zu", add_len, s->len);
    return STK_OK;
}

STK_STATUS stk_str_append_str(stk_str *s, const stk_str *other) {
    STK_RETURN_IF(!s, STK_EINVAL, "String append_str: NULL string pointer");
    STK_RETURN_IF(!other, STK_EINVAL, "String append_str: NULL other string");
    
    return stk_str_append(s, other->data);
}

STK_STATUS stk_str_assign(stk_str *s, const char *cstr) {
    STK_RETURN_IF(!s, STK_EINVAL, "String assign: NULL string pointer");
    
    STK_STATUS rc = stk_str_clear(s);
    if (rc != STK_OK) return rc;
    
    if (!cstr || cstr[0] == '\0') return STK_OK;
    
    size_t new_len = strlen(cstr);
    char *new_data = (char *)realloc(s->data, new_len + 1);
    if (!new_data) {
        STK_LOG_ERROR("String assign: realloc failed (size=%zu)", new_len + 1);
        return STK_ENOMEM;
    }
    
    s->data = new_data;
    strcpy(s->data, cstr);
    s->len = new_len;
    
    STK_LOG_DEBUG("String assign: new len=%zu", s->len);
    return STK_OK;
}

STK_STATUS stk_str_assign_str(stk_str *s, const stk_str *other) {
    STK_RETURN_IF(!s, STK_EINVAL, "String assign_str: NULL string pointer");
    STK_RETURN_IF(!other, STK_EINVAL, "String assign_str: NULL other string");
    
    return stk_str_assign(s, other->data);
}

int stk_str_find_char(const stk_str *s, char ch, size_t start) {
    if (!s || !s->data || start >= s->len) {
        if (s) STK_LOG_WARN("String find_char: invalid params");
        return -1;
    }
    
    for (size_t i = start; i < s->len; i++) {
        if (s->data[i] == ch)
            return (int)i;
    }
    return -1;
}

int stk_str_find(const stk_str *s, const char *substr, size_t start) {
    if (!s || !s->data || !substr || start >= s->len) {
        if (s) STK_LOG_WARN("String find: invalid params");
        return -1;
    }
    
    char *pos = strstr(s->data + start, substr);
    if (!pos)
        return -1;
    
    return (int)(pos - s->data);
}

int stk_str_cmp(const stk_str *s1, const stk_str *s2) {
    if (!s1 && !s2) return 0;
    if (!s1) return -1;
    if (!s2) return 1;
    
    const char *str1 = s1->data ? s1->data : "";
    const char *str2 = s2->data ? s2->data : "";
    return strcmp(str1, str2);
}

int stk_str_cmp_qsort(const void *a, const void *b) {
    const stk_str *sa = *(const stk_str *const *)a;
    const stk_str *sb = *(const stk_str *const *)b;
    return stk_str_cmp(sa, sb);
}

int stk_str_cmp_cstr(const stk_str *s, const char *cstr) {
    if (!s && !cstr) return 0;
    if (!s) return -1;
    if (!cstr) return 1;
    
    const char *data = s->data ? s->data : "";
    return strcmp(data, cstr);
}

STK_STATUS stk_str_to_upper(stk_str *s) {
    STK_RETURN_IF(!s, STK_EINVAL, "String to_upper: NULL string pointer");
    
    if (!s->data) return STK_OK;
    
    for (size_t i = 0; i < s->len; i++) {
        s->data[i] = (char)toupper((unsigned char)s->data[i]);
    }
    
    STK_LOG_DEBUG("String converted to upper");
    return STK_OK;
}

STK_STATUS stk_str_to_lower(stk_str *s) {
    STK_RETURN_IF(!s, STK_EINVAL, "String to_lower: NULL string pointer");
    
    if (!s->data) return STK_OK;
    
    for (size_t i = 0; i < s->len; i++) {
        s->data[i] = (char)tolower((unsigned char)s->data[i]);
    }
    
    STK_LOG_DEBUG("String converted to lower");
    return STK_OK;
}

stk_str *stk_str_sub(const stk_str *s, size_t start, size_t end) {
    if (!s) {
        STK_LOG_WARN("String sub: NULL string pointer");
        return NULL;
    }
    
    stk_str *result = (stk_str *)malloc(sizeof(stk_str));
    if (!result) {
        STK_LOG_ERROR("String sub: malloc failed");
        return NULL;
    }
    
    STK_STATUS rc = stk_str_init(result);
    if (rc != STK_OK) {
        free(result);
        return NULL;
    }
    
    if (!s->data || start >= s->len || end <= start) {
        return result;
    }
    
    if (end > s->len) end = s->len;
    
    size_t sub_len = end - start;
    result->data = (char *)malloc(sub_len + 1);
    if (!result->data) {
        STK_LOG_ERROR("String sub: malloc failed (size=%zu)", sub_len + 1);
        free(result);
        return NULL;
    }
    
    memcpy(result->data, s->data + start, sub_len);
    result->data[sub_len] = '\0';
    result->len = sub_len;
    
    STK_LOG_DEBUG("String sub: extracted %zu bytes", sub_len);
    return result;
}

void stk_str_print(const stk_str *s) {
    if (s && s->data) {
        printf("%s", s->data);
    }
}

void stk_str_println(const stk_str *s) {
    stk_str_print(s);
    printf("\n");
}

STK_STATUS stk_str_copy(stk_str *dest, const stk_str *src) {
    STK_RETURN_IF(!dest, STK_EINVAL, "String copy: NULL dest pointer");
    STK_RETURN_IF(!src, STK_EINVAL, "String copy: NULL src pointer");
    
    return stk_str_init_from(dest, src->data);
}