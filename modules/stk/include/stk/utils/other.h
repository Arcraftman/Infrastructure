#ifndef STK_UTILS_OTHER_H
#define STK_UTILS_OTHER_H

#include "stk/def.h"
#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/* =========================================================================
 * String Utilities
 * ========================================================================= */

/* Returns length of a null-terminated string (safe, caps at max_len). */
STK_API size_t stk_strnlen(const char *s, size_t max_len);

/* Copy at most dst_size-1 bytes; always null-terminates. Returns strlen(src). */
STK_API size_t stk_strlcpy(char *dst, const char *src, size_t dst_size);

/* Append at most dst_size-1 bytes; always null-terminates. Returns total length. */
STK_API size_t stk_strlcat(char *dst, const char *src, size_t dst_size);

/* Case-insensitive comparison. Returns 0 on match. */
STK_API int    stk_stricmp(const char *a, const char *b);

/* Trim whitespace (spaces, tabs, newlines) in-place. Returns s. */
STK_API char  *stk_strtrim(char *s);

/* Split a string by delimiter, filling an array (max tokens). Returns token count. */
STK_API size_t stk_strsplit(const char *s, int delim, char **tokens, size_t max_tokens);

/* Join an array of strings with separator. Returns allocated string (caller frees). */
STK_API char  *stk_strjoin(const char **parts, size_t count, const char *sep);

/* Convert to upper/lower case in-place. Returns s. */
STK_API char  *stk_strupper(char *s);
STK_API char  *stk_strlower(char *s);

/* =========================================================================
 * Path Utilities
 * ========================================================================= */

/* Extract directory part from path (allocated, caller frees). Returns "." if no slash. */
STK_API char  *stk_path_dirname(const char *path);

/* Extract filename part from path (allocated, caller frees). */
STK_API char  *stk_path_basename(const char *path);

/* Extract file extension (including dot) from path. Returns NULL if none. */
STK_API char  *stk_path_extension(const char *path);

/* Join two path components with the platform separator. Caller frees result. */
STK_API char  *stk_path_join(const char *a, const char *b);

/* =========================================================================
 * Environment Utilities
 * ========================================================================= */

/* Get environment variable, or fallback if unset. */
STK_API const char *stk_env_get(const char *name, const char *fallback);

/* Get the user's home directory. Returns "$HOME" on Unix, "%USERPROFILE%" on Windows. */
STK_API const char *stk_env_home(void);

/* =========================================================================
 * Math Utilities
 * ========================================================================= */

/* Round value up/down to the next multiple of alignment (must be power-of-2). */
STK_API size_t stk_math_round_up(size_t val, size_t alignment);
STK_API size_t stk_math_round_down(size_t val, size_t alignment);

/* Check if a value is a power of 2. */
STK_API int    stk_math_is_pow2(size_t val);

/* Align pointer to the given alignment (power-of-2). Returns aligned pointer. */
STK_API void  *stk_math_align_ptr(void *ptr, size_t alignment);

/* =========================================================================
 * Hash Utilities
 * ========================================================================= */

/* FNV-1a 64-bit hash over arbitrary data. */
STK_API uint64_t stk_hash_fnv1a(const void *data, size_t len);

/* FNV-1a 64-bit hash for null-terminated string. */
STK_API uint64_t stk_hash_str(const char *s);

/* MurmurHash3 32-bit (x86) — fast, well-distributed. */
STK_API uint32_t stk_hash_murmur32(const void *key, size_t len, uint32_t seed);

/* CRC-32 (IEEE polynomial). */
STK_API uint32_t stk_hash_crc32(const void *data, size_t len);

#ifdef __cplusplus
}
#endif

#endif /* STK_UTILS_OTHER_H */
