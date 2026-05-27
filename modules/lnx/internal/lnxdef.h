#ifndef LNX_DEF_H
#define LNX_DEF_H

#ifdef _MSC_VER
#ifdef LNX_DLL
#ifdef LNX_EXPORTING
#define LNX_API __declspec(dllexport)
#else
#define LNX_API __declspec(dllimport)
#endif
#else
#define LNX_API   
#endif
#else
#ifdef LNX_DLL
#ifdef LNX_EXPORTING
#define LNX_API __attribute__((visibility("default")))
#else
#define LNX_API
#endif
#else
#define LNX_API
#endif
#endif

#endif