#ifndef _STK_VECTOR_FLOAT_H
#define _STK_VECTOR_FLOAT_H

#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "stk_vector_config.h"

typedef struct  {
    float* data;
    size_t size;
    size_t capacity;
} VecFloat;

STK_API void VecFloat_Init(VecFloat* v);
STK_API void VecFloat_Destroy(VecFloat* v);
STK_API void VecFloat_PushBack(VecFloat* v, float value);
STK_API void VecFloat_PopBack(VecFloat* v);
STK_API float VecFloat_At(VecFloat* v, size_t index);
STK_API size_t VecFloat_Size(VecFloat* v);
STK_API int VecFloat_Empty(VecFloat* v);
STK_API void VecFloat_Resize(VecFloat* v, size_t new_size);
STK_API void VecFloat_Fill(VecFloat* v, float value);

#endif