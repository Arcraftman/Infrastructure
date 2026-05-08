#ifndef _STK_STRING_H
#define _STK_STRING_H

#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <ctype.h>
#include "stk_export.h"

typedef struct {
    char* data;
    size_t length;
} String;

STK_API void StrInit(String* s);
STK_API void StrInitFrom(String* s, const char* cstr);
STK_API void StrDestroy(String* s);
STK_API const char* StrCStr(const String* s);
STK_API size_t StrLength(const String* s);
STK_API int StrEmpty(const String* s);
STK_API void StrClear(String* s);
STK_API void StrPushBack(String* s, char ch);
STK_API void StrPopBack(String* s);
STK_API void StrAppend(String* s, const char* cstr);
STK_API void StrAppendString(String* s, const String* other);
STK_API void StrAssign(String* s, const char* cstr);
STK_API void StrAssignString(String* s, const String* other);
STK_API int StrFindChar(const String* s, char ch, size_t start);
STK_API int StrFind(const String* s, const char* substr, size_t start);
STK_API int StrCompare(const String* s1, const String* s2);
STK_API int StrCompareCStr(const String* s, const char* cstr);
STK_API void StrToUpper(String* s);
STK_API void StrToLower(String* s);
STK_API String* StrSubstring(const String* s, size_t start, size_t end);
STK_API void StrPrint(const String* s);
STK_API void StrPrintln(const String* s);
STK_API void StrDeepCopy(String* dest, const String* src);

#endif