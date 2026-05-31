#ifndef WEB_FORM_H
#define WEB_FORM_H

#include "web/def.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @file form.h
 * @brief Form data parser for application/x-www-form-urlencoded
 *        and multipart/form-data.
 *
 * For multipart parsing, the Content-Type boundary must be extracted
 * from the request headers by the caller and passed to the parser.
 */

/* =========================================================================
 * Form Field
 * ========================================================================= */

typedef struct web_form_field {
    char  *name;      /**< Field name (decoded). */
    char  *value;     /**< Field value (decoded for urlencoded; raw buffer for multipart). */
    size_t value_len; /**< Length of value. */
    char  *filename;  /**< Filename (multipart file uploads only). May be NULL. */
    char  *mime_type; /**< Content-Type (multipart only). May be NULL. */
    struct web_form_field *next;
} web_form_field_t;

/* =========================================================================
 * Form
 * ========================================================================= */

typedef struct web_form {
    web_form_field_t *fields; /**< Linked list of parsed fields. */
    size_t            count;  /**< Number of fields. */
} web_form_t;

/**
 * Parse an application/x-www-form-urlencoded body.
 * @param data     The body data.
 * @param len      Length of data.
 * @return Parsed form (caller must free with web_form_destroy()), or NULL on error.
 */
WEB_API web_form_t *
web_form_parse_urlencoded(const void *data, size_t len);

/**
 * Create an empty form for building multipart responses.
 */
WEB_API web_form_t *
web_form_create(void);

/**
 * Add a field to a form (makes copies of name and value).
 * @return 0 on success, -1 on error.
 */
WEB_API int
web_form_add_field(web_form_t *form, const char *name,
                   const void *value, size_t value_len,
                   const char *filename, const char *mime_type);

/**
 * Look up a field value by name.
 * @return The first matching field, or NULL if not found.
 */
WEB_API const web_form_field_t *
web_form_get(const web_form_t *form, const char *name);

/**
 * Look up a field value by name as a C string.
 * @return The value pointer (may contain null bytes for multipart files),
 *         or NULL if the field is not found or has no value.
 */
WEB_API const char *
web_form_get_value(const web_form_t *form, const char *name);

/**
 * Free all form fields and the form itself.
 */
WEB_API void
web_form_destroy(web_form_t *form);

#ifdef __cplusplus
}
#endif

#endif /* WEB_FORM_H */
