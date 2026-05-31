#ifndef WEB_DEF_H
#define WEB_DEF_H

#ifdef __cplusplus
extern "C" {
#endif

/* =========================================================================
 * Platform Detection
 * ========================================================================= */

#if defined(_WIN32) || defined(_WIN64)
#  define WEB_OS_WINDOWS 1
#elif defined(__linux__)
#  define WEB_OS_LINUX   1
#elif defined(__APPLE__) || defined(__MACH__)
#  define WEB_OS_MACOS   1
#elif defined(__FreeBSD__)
#  define WEB_OS_FREEBSD 1
#else
#  define WEB_OS_UNKNOWN 1
#endif

/* =========================================================================
 * Compiler Detection
 * ========================================================================= */

#if defined(__clang__)
#  define WEB_COMPILER_CLANG 1
#elif defined(__GNUC__) || defined(__GNUG__)
#  define WEB_COMPILER_GCC   1
#elif defined(_MSC_VER)
#  define WEB_COMPILER_MSVC  1
#endif

/* =========================================================================
 * DLL Export / Import
 *
 * Define WEB_DLL when building or using the shared library.
 * Define WEB_EXPORTING when building the library (not consuming it).
 * ========================================================================= */

#if defined(WEB_COMPILER_MSVC)
#  if defined(WEB_DLL)
#    if defined(WEB_EXPORTING)
#      define WEB_API __declspec(dllexport)
#    else
#      define WEB_API __declspec(dllimport)
#    endif
#  else
#    define WEB_API
#  endif
#elif defined(WEB_COMPILER_GCC) || defined(WEB_COMPILER_CLANG)
#  if defined(WEB_DLL) && defined(WEB_EXPORTING)
#    define WEB_API __attribute__((visibility("default")))
#  else
#    define WEB_API
#  endif
#else
#  define WEB_API
#endif

/* =========================================================================
 * Compiler Hints
 * ========================================================================= */

#if defined(WEB_COMPILER_GCC) || defined(WEB_COMPILER_CLANG)
#  define WEB_LIKELY(x)   __builtin_expect(!!(x), 1)
#  define WEB_UNLIKELY(x) __builtin_expect(!!(x), 0)
#  define WEB_UNUSED      __attribute__((unused))
#  define WEB_PRINTF(f,a) __attribute__((format(printf, f, a)))
#else
#  define WEB_LIKELY(x)   (x)
#  define WEB_UNLIKELY(x) (x)
#  define WEB_UNUSED
#  define WEB_PRINTF(f,a)
#endif

/* =========================================================================
 * Common Standard Headers
 * ========================================================================= */

#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>

/* =========================================================================
 * Module API
 * ========================================================================= */

WEB_API const char *
web_version(void);

#ifdef __cplusplus
}
#endif

#endif /* WEB_DEF_H */
