#include "web/form.h"

#include <ctype.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

/*
 * application/x-www-form-urlencoded parsing:
 *   key1=value1&key2=value2
 * Both keys and values are percent-decoded and '+' → ' '.
 */

static void
form_decode_inplace(char *s)
{
    if (!s) return;
    size_t r = 0, w = 0;
    while (s[r]) {
        if (s[r] == '+') {
            s[w++] = ' ';
            r++;
        } else if (s[r] == '%' && s[r+1] && s[r+2]) {
            unsigned char hi = (unsigned char)s[r+1];
            unsigned char lo = (unsigned char)s[r+2];
            int h = (hi >= '0' && hi <= '9') ? hi - '0' :
                    (hi >= 'a' && hi <= 'f') ? hi - 'a' + 10 :
                    (hi >= 'A' && hi <= 'F') ? hi - 'A' + 10 : -1;
            int l = (lo >= '0' && lo <= '9') ? lo - '0' :
                    (lo >= 'a' && lo <= 'f') ? lo - 'a' + 10 :
                    (lo >= 'A' && lo <= 'F') ? lo - 'A' + 10 : -1;
            if (h >= 0 && l >= 0) {
                s[w++] = (char)((h << 4) | l);
                r += 3;
            } else {
                s[w++] = s[r++];
            }
        } else {
            s[w++] = s[r++];
        }
    }
    s[w] = '\0';
}

/* =========================================================================
 * Parse urlencoded
 * ========================================================================= */

WEB_API web_form_t *
web_form_parse_urlencoded(const void *data, size_t len)
{
    if (!data || len == 0) return NULL;

    web_form_t *form = (web_form_t *)calloc(1, sizeof(*form));
    if (!form) return NULL;

    const char *p = (const char *)data;
    const char *end = p + len;

    while (p < end) {
        /* skip leading & */
        while (p < end && *p == '&') p++;
        if (p >= end) break;

        /* find & or end */
        const char *amp = (const char *)memchr(p, '&', (size_t)(end - p));
        const char *pair_end = amp ? amp : end;

        const char *eq = (const char *)memchr(p, '=', (size_t)(pair_end - p));

        web_form_field_t *f = (web_form_field_t *)calloc(1, sizeof(*f));
        if (!f) break;

        if (eq && eq < pair_end) {
            f->name  = strndup(p, (size_t)(eq - p));
            f->value = strndup(eq + 1, (size_t)(pair_end - eq - 1));
        } else {
            f->name  = strndup(p, (size_t)(pair_end - p));
            f->value = strdup("");
        }

        if (f->name) {
            form_decode_inplace(f->name);
            if (f->value) form_decode_inplace(f->value);
            f->value_len = f->value ? strlen(f->value) : 0;

            f->next = form->fields;
            form->fields = f;
            form->count++;
        } else {
            free(f->value);
            free(f);
        }

        p = pair_end;
    }

    return form;
}

/* =========================================================================
 * Create / Add / Get / Destroy
 * ========================================================================= */

WEB_API web_form_t *
web_form_create(void)
{
    return (web_form_t *)calloc(1, sizeof(web_form_t));
}

WEB_API int
web_form_add_field(web_form_t *form, const char *name,
                   const void *value, size_t value_len,
                   const char *filename, const char *mime_type)
{
    if (!form || !name) return -1;

    web_form_field_t *f = (web_form_field_t *)calloc(1, sizeof(*f));
    if (!f) return -1;

    f->name = strdup(name);
    if (!f->name) { free(f); return -1; }

    if (value && value_len > 0) {
        f->value = (char *)malloc(value_len + 1);
        if (!f->value) { free(f->name); free(f); return -1; }
        memcpy(f->value, value, value_len);
        f->value[value_len] = '\0';
        f->value_len = value_len;
    } else {
        f->value = strdup("");
        if (!f->value) { free(f->name); free(f); return -1; }
        f->value_len = 0;
    }

    if (filename)
        f->filename = strdup(filename);
    if (mime_type)
        f->mime_type = strdup(mime_type);

    f->next = form->fields;
    form->fields = f;
    form->count++;
    return 0;
}

WEB_API const web_form_field_t *
web_form_get(const web_form_t *form, const char *name)
{
    if (!form || !name) return NULL;
    for (const web_form_field_t *f = form->fields; f; f = f->next) {
        if (f->name && strcmp(f->name, name) == 0)
            return f;
    }
    return NULL;
}

WEB_API const char *
web_form_get_value(const web_form_t *form, const char *name)
{
    const web_form_field_t *f = web_form_get(form, name);
    return f ? f->value : NULL;
}

WEB_API void
web_form_destroy(web_form_t *form)
{
    if (!form) return;
    web_form_field_t *f = form->fields;
    while (f) {
        web_form_field_t *next = f->next;
        free(f->name);
        free(f->value);
        free(f->filename);
        free(f->mime_type);
        free(f);
        f = next;
    }
    free(form);
}
