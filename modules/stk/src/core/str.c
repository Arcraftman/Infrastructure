#include "stk/core/str.h"
#include <string.h>
#include <stdlib.h>

#define STK_STRING_DEFAULT_CAPACITY 16
#define STK_STRING_GROW_FACTOR 2

#define STK_STRING_COMPARE_CASE_SENSITIVE 1
#define STK_STRING_COMPARE_CASE_INSENSITIVE 0

void stk_str_init(stk_str *s)
{
    if (!s)
        return;
    s->data = NULL;
    s->len = 0;
}

void stk_str_init_from(stk_str *s, const char *cstr)
{
    if (!s)
        return;

    if (!cstr || cstr[0] == '\0')
    {
        s->data = NULL;
        s->len = 0;
        return;
    }

    s->len = strlen(cstr);
    s->data = (char *)malloc(s->len + 1);
    if (s->data)
    {
        strcpy(s->data, cstr);
    }
    else
    {
        s->len = 0;
    }
}

void stk_str_free(stk_str *s)
{
    if (!s)
        return;
    if (s->data)
    {
        free(s->data);
        s->data = NULL;
    }
    s->len = 0;
}

const char *stk_str_cstr(const stk_str *s)
{
    return s && s->data ? s->data : "";
}

size_t stk_str_len(const stk_str *s)
{
    return s ? s->len : 0;
}

bool stk_str_empty(const stk_str *s)
{
    return s ? s->len == 0 : true;
}

void stk_str_clear(stk_str *s)
{
    if (!s)
        return;
    if (s->data)
    {
        s->data[0] = '\0';
    }
    s->len = 0;
}

void stk_str_push(stk_str *s, char ch)
{
    if (!s)
        return;

    char *new_data = (char *)realloc(s->data, s->len + 2);
    if (!new_data)
        return;

    s->data = new_data;
    s->data[s->len++] = ch;
    s->data[s->len] = '\0';
}

void stk_str_pop(stk_str *s)
{
    if (!s || s->len == 0)
        return;

    s->len--;
    s->data[s->len] = '\0';

    if (s->len == 0)
    {
        free(s->data);
        s->data = NULL;
    }
}

void stk_str_append(stk_str *s, const char *cstr)
{
    if (!s || !cstr)
        return;

    size_t add_len = strlen(cstr);
    if (add_len == 0)
        return;

    char *new_data = (char *)realloc(s->data, s->len + add_len + 1);
    if (!new_data)
        return;

    s->data = new_data;
    memcpy(s->data + s->len, cstr, add_len + 1);
    s->len += add_len;
}

void stk_str_append_str(stk_str *s, const stk_str *other)
{
    if (!s || !other)
        return;
    stk_str_append(s, other->data);
}

void stk_str_assign(stk_str *s, const char *cstr)
{
    if (!s)
        return;

    stk_str_clear(s);
    if (!cstr || cstr[0] == '\0')
        return;

    size_t new_len = strlen(cstr);
    char *new_data = (char *)realloc(s->data, new_len + 1);
    if (!new_data)
        return;

    s->data = new_data;
    strcpy(s->data, cstr);
    s->len = new_len;
}

void stk_str_assign_str(stk_str *s, const stk_str *other)
{
    if (!s || !other)
        return;
    stk_str_assign(s, other->data);
}

int stk_str_find_char(const stk_str *s, char ch, size_t start)
{
    if (!s || !s->data || start >= s->len)
        return -1;

    for (size_t i = start; i < s->len; i++)
    {
        if (s->data[i] == ch)
            return (int)i;
    }
    return -1;
}

int stk_str_find(const stk_str *s, const char *substr, size_t start)
{
    if (!s || !s->data || !substr || start >= s->len)
        return -1;

    char *pos = strstr(s->data + start, substr);
    if (!pos)
        return -1;

    return (int)(pos - s->data);
}
int stk_str_cmp(const stk_str *s1, const stk_str *s2)
{
    if (!s1 && !s2)
        return 0;
    if (!s1)
        return -1;
    if (!s2)
        return 1;

    const char *str1 = s1->data ? s1->data : "";
    const char *str2 = s2->data ? s2->data : "";
    return strcmp(str1, str2);
}

int stk_str_cmp_qsort(const void *a, const void *b)
{
    const stk_str *sa = *(const stk_str *const *)a;
    const stk_str *sb = *(const stk_str *const *)b;
    return stk_str_cmp(sa, sb);
}
int stk_str_cmp_cstr(const stk_str *s, const char *cstr)
{
    if (!s && !cstr)
        return 0;
    if (!s)
        return -1;
    if (!cstr)
        return 1;

    const char *data = s->data ? s->data : "";
    return strcmp(data, cstr);
}

void stk_str_to_upper(stk_str *s)
{
    if (!s || !s->data)
        return;

    for (size_t i = 0; i < s->len; i++)
    {
        s->data[i] = (char)toupper((unsigned char)s->data[i]);
    }
}

void stk_str_to_lower(stk_str *s)
{
    if (!s || !s->data)
        return;

    for (size_t i = 0; i < s->len; i++)
    {
        s->data[i] = (char)tolower((unsigned char)s->data[i]);
    }
}

stk_str *stk_str_sub(const stk_str *s, size_t start, size_t end)
{
    stk_str *result = (stk_str *)malloc(sizeof(stk_str));
    if (!result)
        return NULL;
    stk_str_init(result);

    if (!s || !s->data || start >= s->len || end <= start)
    {
        return result;
    }

    if (end > s->len)
        end = s->len;

    size_t sub_len = end - start;
    result->data = (char *)malloc(sub_len + 1);
    if (!result->data)
    {
        free(result);
        return NULL;
    }

    memcpy(result->data, s->data + start, sub_len);
    result->data[sub_len] = '\0';
    result->len = sub_len;

    return result;
}

void stk_str_print(const stk_str *s)
{
    if (s && s->data)
    {
        printf("%s", s->data);
    }
}

void stk_str_println(const stk_str *s)
{
    stk_str_print(s);
    printf("\n");
}

void stk_str_copy(stk_str *dest, const stk_str *src)
{
    if (!dest || !src)
        return;
    stk_str_init_from(dest, src->data);
}