#ifndef _STK_VECTOR_STRING_H
#define _STK_VECTOR_STRING_H

#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "stk_vector_config.h"
#include "stk_string.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    String* data;
    size_t size;
    size_t capacity;
} VecString;

STK_API void VecString_Init(VecString* v);
STK_API void VecString_Destroy(VecString* v);
STK_API void VecString_PushBack(VecString* v, String value);
STK_API void VecString_PopBack(VecString* v);
STK_API String VecString_At(VecString* v, size_t index);
STK_API size_t VecString_Size(VecString* v);
STK_API int VecString_Empty(VecString* v);
STK_API void VecString_Resize(VecString* v, size_t new_size);
STK_API void VecString_Fill(VecString* v, String value);

#ifdef __cplusplus
}
#endif

#endif