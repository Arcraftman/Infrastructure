#ifndef _STK_VECTOR_INT_H
#define _STK_VECTOR_INT_H

#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "stk_vector_config.h"

typedef struct {
    int* data;
    size_t size;
    size_t capacity;
} VecInt;

STK_API void VecInt_Init(VecInt* v);
STK_API void VecInt_Destroy(VecInt* v);
STK_API void VecInt_PushBack(VecInt* v, int value);
STK_API void VecInt_PopBack(VecInt* v);
STK_API int VecInt_At(VecInt* v, size_t index);
STK_API size_t VecInt_Size(VecInt* v);
STK_API int VecInt_Empty(VecInt* v);
STK_API void VecInt_Resize(VecInt* v, size_t new_size);
STK_API void VecInt_Fill(VecInt* v, int value);

#endif