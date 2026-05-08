#ifndef STK_EXPORT_H
#define STK_EXPORT_H

#ifdef _MSC_VER
#ifdef STK_DLL
#ifdef STK_EXPORTING
#define STK_API __declspec(dllexport)
#else
#define STK_API __declspec(dllimport)
#endif
#else
#define STK_API   
#endif
#else
#ifdef STK_DLL
#ifdef STK_EXPORTING
#define STK_API __attribute__((visibility("default")))
#else
#define STK_API
#endif
#else
#define STK_API
#endif
#endif

#endif