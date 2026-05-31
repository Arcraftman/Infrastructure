#ifndef STK_DEF_H
#define STK_DEF_H

#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>

/* =========================================================================
 * Platform Detection
 * ========================================================================= */
#if defined(_WIN32) || defined(_WIN64)
#    define STK_OS_WIN   1
#elif defined(__linux__)
#    define STK_OS_LINUX 1
#elif defined(__APPLE__)
#    define STK_OS_MACOS 1
#else
#    define STK_OS_UNKNOWN 1
#endif

/* =========================================================================
 * Compiler Detection
 * ========================================================================= */
#if defined(__clang__)
#    define STK_COMPILER_CLANG 1
#elif defined(__GNUC__) || defined(__GNUG__)
#    define STK_COMPILER_GCC   1
#elif defined(_MSC_VER)
#    define STK_COMPILER_MSVC  1
#else
#    define STK_COMPILER_UNKNOWN 1
#endif

/* =========================================================================
 * API Export / Import
 *
 * Define STK_DLL when building or using the shared library.
 * Define STK_EXPORTING when building the library (not consuming it).
 * ========================================================================= */
#if defined(STK_COMPILER_MSVC)
#    if defined(STK_DLL)
#        if defined(STK_EXPORTING)
#            define STK_API __declspec(dllexport)
#        else
#            define STK_API __declspec(dllimport)
#        endif
#    else
#        define STK_API
#    endif
#elif defined(STK_COMPILER_GCC) || defined(STK_COMPILER_CLANG)
#    if defined(STK_DLL) && defined(STK_EXPORTING)
#        define STK_API __attribute__((visibility("default")))
#    else
#        define STK_API
#    endif
#else
#    define STK_API
#endif

/* =========================================================================
 * Compiler Hints & Attributes
 * ========================================================================= */

/* Inline hint — use 'inline' on C99, '__inline' on MSVC pre-C99. */
#if defined(STK_COMPILER_MSVC)
#    define STK_INLINE __inline
#else
#    define STK_INLINE inline
#endif

/* Mark a function as never returning. */
#if defined(STK_COMPILER_MSVC)
#    define STK_NORETURN __declspec(noreturn)
#elif defined(STK_COMPILER_GCC) || defined(STK_COMPILER_CLANG)
#    define STK_NORETURN __attribute__((noreturn))
#else
#    define STK_NORETURN
#endif

/* Suppress "unused" warnings (e.g. for parameters kept for API consistency). */
#if defined(STK_COMPILER_GCC) || defined(STK_COMPILER_CLANG)
#    define STK_UNUSED __attribute__((unused))
#else
#    define STK_UNUSED
#endif

/* Branch prediction hints. */
#if defined(STK_COMPILER_GCC) || defined(STK_COMPILER_CLANG)
#    define STK_LIKELY(x)   __builtin_expect(!!(x), 1)
#    define STK_UNLIKELY(x) __builtin_expect(!!(x), 0)
#else
#    define STK_LIKELY(x)   (x)
#    define STK_UNLIKELY(x) (x)
#endif

/* Mark a declaration as deprecated. */
#if defined(STK_COMPILER_MSVC)
#    define STK_DEPRECATED(msg) __declspec(deprecated(msg))
#elif defined(STK_COMPILER_GCC) || defined(STK_COMPILER_CLANG)
#    define STK_DEPRECATED(msg) __attribute__((deprecated(msg)))
#else
#    define STK_DEPRECATED(msg)
#endif

/* Alignment. */
#if defined(STK_COMPILER_MSVC)
#    define STK_ALIGN(n) __declspec(align(n))
#elif defined(STK_COMPILER_GCC) || defined(STK_COMPILER_CLANG)
#    define STK_ALIGN(n) __attribute__((aligned(n)))
#else
#    define STK_ALIGN(n)
#endif

#endif /* STK_DEF_H */