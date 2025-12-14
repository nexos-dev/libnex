/*
    array.c - contains dynamic array implementation
    Copyright 2023 - 2025 The NexNix Project

    Licensed under the Apache License, Version 2.0 (the "License");
    you may not use this file except in compliance with the License.
    There should be a copy of the License distributed in a file named
    LICENSE, if not, you may obtain a copy of the License at

        http://www.apache.org/licenses/LICENSE-2.0

    Unless required by applicable law or agreed to in writing, software
    distributed under the License is distributed on an "AS IS" BASIS,
    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
    See the License for the specific language governing permissions and
    limitations under the License.
*/

/// @file array.c

#include <assert.h>
#include <libnex/array.h>
#include <libnex/lock.h>
#include <libnex/safemalloc.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

LIBNEX_PUBLIC Array_t* ArrayCreate (size_t elements, size_t maxElems, size_t elemSize)
{
    return NULL;
}

LIBNEX_PUBLIC void ArrayDestroy (Array_t* array)
{
}

LIBNEX_PUBLIC void* ArrayGetElement (Array_t* array, size_t pos)
{
}

LIBNEX_PUBLIC void ArrayRemoveElement (Array_t* array, size_t pos)
{
}

LIBNEX_PUBLIC size_t ArrayFindFreeElement (Array_t* array)
{
    return 0;
}

LIBNEX_PUBLIC size_t ArrayFindElement (Array_t* array, const void* hint)
{
    return ARRAY_ERROR;
}

LIBNEX_PUBLIC ArrayIter_t* ArrayIterate (Array_t* array, ArrayIter_t* iter)
{
    return NULL;
}

LIBNEX_PUBLIC void ArraySetFindBy (Array_t* array, ArrayFindBy func)
{
}

LIBNEX_PUBLIC void ArraySetDestroy (Array_t* array, ArrayDestroyElem func)
{
}
