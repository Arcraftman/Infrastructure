#ifndef _STK_VECTOR_DOUBLE_H
#define _STK_VECTOR_DOUBLE_H

#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "stk_vector_config.h"

typedef struct  {
    double* data;
    size_t size;
    size_t capacity;
} VecDouble;

STK_API void VecDouble_Init(VecDouble* v);
STK_API void VecDouble_Destroy(VecDouble* v);
STK_API void VecDouble_PushBack(VecDouble* v, double value);
STK_API void VecDouble_PopBack(VecDouble* v);
STK_API double VecDouble_At(VecDouble* v, size_t index);
STK_API size_t VecDouble_Size(VecDouble* v);
STK_API int VecDouble_Empty(VecDouble* v);
STK_API void VecDouble_Resize(VecDouble* v, size_t new_size);
STK_API void VecDouble_Fill(VecDouble* v, double value);

#endif