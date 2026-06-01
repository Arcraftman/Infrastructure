#ifndef STK_UTILS_STATUS_H
#define STK_UTILS_STATUS_H


#ifdef __cplusplus
extern "C" {
#endif

/* Status codes for STK library functions */
typedef int STK_STATUS;

/* Success */
#define STK_OK          0

/* General errors (negative values) */
#define STK_ERROR       -1      /* Generic / unspecified error */
#define STK_ENOMEM      -2      /* Out of memory */
#define STK_EINVAL      -3      /* Invalid argument */
#define STK_ENOTFOUND   -4      /* Element not found */
#define STK_EEXISTS     -5      /* Element already exists */
#define STK_ERANGE      -6      /* Index out of range */
#define STK_EBUSY       -7      /* Resource busy / locked */
#define STK_EPERM       -8      /* Operation not permitted */
#define STK_EMPTY       -9      /* Container is empty */
#define STK_EFILE       -10     /* File error */

/* Convert STK_STATUS to human-readable string */
STK_API const char* stk_status_str(STK_STATUS status);

#ifdef __cplusplus
}
#endif

#endif /* STK_UTILS_STATUS_H */