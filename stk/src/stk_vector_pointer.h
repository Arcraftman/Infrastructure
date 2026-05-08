#ifndef _STK_VECTOR_POINTER_H
#define _STK_VECTOR_POINTER_H

#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "stk_vector_config.h"

typedef struct {
    void** data;
    size_t size;
    size_t capacity;
} VecPointer;

STK_API void VecPointer_Init(VecPointer* v);
STK_API void VecPointer_Destroy(VecPointer* v);
STK_API void VecPointer_PushBack(VecPointer* v, void* value);
STK_API void VecPointer_PopBack(VecPointer* v);
STK_API void* VecPointer_At(VecPointer* v, size_t index);
STK_API size_t VecPointer_Size(VecPointer* v);
STK_API int VecPointer_Empty(VecPointer* v);
STK_API void VecPointer_Resize(VecPointer* v, size_t new_size);
STK_API void VecPointer_Fill(VecPointer* v, void* value);

#endif