#ifndef LNX_DEF_H
#define LNX_DEF_H

#ifdef __cplusplus
extern "C" {
#endif

/* =========================================================================
 * Platform Detection
 * ========================================================================= */

#if defined(_WIN32) || defined(_WIN64)
#  define LNX_OS_WINDOWS 1
#elif defined(__linux__)
#  define LNX_OS_LINUX   1
#elif defined(__APPLE__) || defined(__MACH__)
#  define LNX_OS_MACOS   1
#elif defined(__FreeBSD__)
#  define LNX_OS_FREEBSD 1
#else
#  define LNX_OS_UNKNOWN 1
#endif

/* =========================================================================
 * Compiler Detection
 * ========================================================================= */

#if defined(__clang__)
#  define LNX_COMPILER_CLANG 1
#elif defined(__GNUC__) || defined(__GNUG__)
#  define LNX_COMPILER_GCC   1
#elif defined(_MSC_VER)
#  define LNX_COMPILER_MSVC  1
#endif

/* =========================================================================
 * DLL Export / Import
 *
 * Define LNX_DLL when building or using the shared library.
 * Define LNX_EXPORTING when building the library (not consuming it).
 * ========================================================================= */

#if defined(LNX_COMPILER_MSVC)
#  if defined(LNX_DLL)
#    if defined(LNX_EXPORTING)
#      define LNX_API __declspec(dllexport)
#    else
#      define LNX_API __declspec(dllimport)
#    endif
#  else
#    define LNX_API
#  endif
#elif defined(LNX_COMPILER_GCC) || defined(LNX_COMPILER_CLANG)
#  if defined(LNX_DLL) && defined(LNX_EXPORTING)
#    define LNX_API __attribute__((visibility("default")))
#  else
#    define LNX_API
#  endif
#else
#  define LNX_API
#endif

/* =========================================================================
 * Compiler Hints
 * ========================================================================= */

#if defined(LNX_COMPILER_GCC) || defined(LNX_COMPILER_CLANG)
#  define LNX_LIKELY(x)   __builtin_expect(!!(x), 1)
#  define LNX_UNLIKELY(x) __builtin_expect(!!(x), 0)
#  define LNX_UNUSED      __attribute__((unused))
#  define LNX_PRINTF(f,a) __attribute__((format(printf, f, a)))
#else
#  define LNX_LIKELY(x)   (x)
#  define LNX_UNLIKELY(x) (x)
#  define LNX_UNUSED
#  define LNX_PRINTF(f,a)
#endif

/* =========================================================================
 * Common Standard Headers
 * ========================================================================= */

#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
}
#endif

#endif /* LNX_DEF_H */
