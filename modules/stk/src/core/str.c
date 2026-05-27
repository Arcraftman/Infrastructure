#include "str.h"

void str_init(str *s)
{
    if (!s)
        return;
    s->data = NULL;
    s->len = 0;
}

void str_init_from(str *s, const char *cstr)
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

void str_free(str *s)
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

const char *str_cstr(const str *s)
{
    return s && s->data ? s->data : "";
}

size_t str_len(const str *s)
{
    return s ? s->len : 0;
}

bool str_empty(const str *s)
{
    return s ? s->len == 0 : true;
}

void str_clear(str *s)
{
    if (!s)
        return;
    if (s->data)
    {
        s->data[0] = '\0';
    }
    s->len = 0;
}

void str_push(str *s, char ch)
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

void str_pop(str *s)
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

void str_append(str *s, const char *cstr)
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

void str_append_str(str *s, const str *other)
{
    if (!s || !other)
        return;
    str_append(s, other->data);
}

void str_assign(str *s, const char *cstr)
{
    if (!s)
        return;

    str_clear(s);
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

void str_assign_str(str *s, const str *other)
{
    if (!s || !other)
        return;
    str_assign(s, other->data);
}

int str_find_char(const str *s, char ch, size_t start)
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

int str_find(const str *s, const char *substr, size_t start)
{
    if (!s || !s->data || !substr || start >= s->len)
        return -1;

    char *pos = strstr(s->data + start, substr);
    if (!pos)
        return -1;

    return (int)(pos - s->data);
}
int str_cmp(const void *a, const void *b)
{
    if (!a && !b)
        return 0;
    if (!a)
        return -1;
    if (!b)
        return 1;

    const str *s1 = (const str *)a;
    const str *s2 = (const str *)b;

    const char *str1 = s1->data ? s1->data : "";
    const char *str2 = s2->data ? s2->data : "";
    return strcmp(str1, str2);
}

int str_cmp_cstr(const str *s, const char *cstr)
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

void str_to_upper(str *s)
{
    if (!s || !s->data)
        return;

    for (size_t i = 0; i < s->len; i++)
    {
        s->data[i] = (char)toupper((unsigned char)s->data[i]);
    }
}

void str_to_lower(str *s)
{
    if (!s || !s->data)
        return;

    for (size_t i = 0; i < s->len; i++)
    {
        s->data[i] = (char)tolower((unsigned char)s->data[i]);
    }
}

str *str_sub(const str *s, size_t start, size_t end)
{
    str *result = (str *)malloc(sizeof(str));
    if (!result)
        return NULL;
    str_init(result);

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

void str_print(const str *s)
{
    if (s && s->data)
    {
        printf("%s", s->data);
    }
}

void str_println(const str *s)
{
    str_print(s);
    printf("\n");
}

void str_copy(str *dest, const str *src)
{
    if (!dest || !src)
        return;
    str_init_from(dest, src->data);
}