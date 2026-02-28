/*
    array.c - contains dynamic array implementation
    Copyright 2023 - 2026 The NexNix Project

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

// Array structure definition
typedef struct _lnarray
{
    Object_t obj;                    ///< Underlying object for reference counting
    void* data;                      ///< Pointer to array data
    size_t elemSize;                 ///< Size of each element
    size_t numElements;              ///< Current number of allocated elements
    size_t maxElements;              ///< Maximum allowed elements
    size_t growSize;                 ///< Size to grow by when expanding
    ArrayFindBy findFunc;            ///< Custom find callback
    ArrayDestroyElem destroyFunc;    ///< Custom destroy callback
    uint8_t* usedMap;                ///< Maps of bitmaps used to track used elements
    size_t mapSize;
    size_t freeHint;    ///< Used to track where a free element might be at. If -1 there is no hint
} Array_t;

#define ARRAY_MAP_SIZE 4096

LIBNEX_PUBLIC Array_t* ArrayCreate (size_t elements,
                                    size_t maxElems,
                                    size_t elemSize,
                                    ArrayDestroyElem destroyFunc)
{
    if (!elements || !elemSize || elements > (ARRAY_MAP_SIZE / 8) ||
        ((maxElems) ? maxElems < elements : 0))
    {
        LibnexSetError (LIBNEX_ERR_BAD_PARAM);
        return NULL;
    }
    // Allocate array structure
    Array_t* arr = malloc_s (sizeof (Array_t));
    if (!arr)
        return NULL;
    ObjCreate ("Array", &arr->obj);
    // Allocate data buffer
    arr->data = malloc_s (elements * elemSize);
    if (!arr->data)
    {
        free (arr);
        return NULL;
    }
    // Create bitmap to keep track of used elements
    arr->usedMap = calloc_s (ARRAY_MAP_SIZE);
    if (!arr->usedMap)
    {
        free (arr->data);
        free (arr);
        return NULL;
    }
    arr->mapSize = ARRAY_MAP_SIZE;
    // Initialize array members
    arr->elemSize = elemSize;
    arr->numElements = elements;
    if (maxElems)
        arr->maxElements = maxElems;
    else
        arr->maxElements = SIZE_MAX;
    arr->growSize = elements;
    arr->findFunc = NULL;
    arr->destroyFunc = destroyFunc;
    arr->freeHint = 0;
    return arr;
}

LIBNEX_PUBLIC void ArrayDestroy (Array_t* array)
{
    assert (array);
    if (!ObjDeRef (&array->obj))
    {
        // If there's a destroy callback, call it on all used elements
        if (array->destroyFunc)
        {
            for (size_t i = 0; i < array->numElements; i++)
            {
                // Get bitmap for this element
                size_t byteIdx = i / 8;
                size_t bitIdx = i % 8;
                if (array->usedMap[byteIdx] & (1 << bitIdx))
                {
                    void* elem = (char*) array->data + (i * array->elemSize);
                    array->destroyFunc (elem);
                }
            }
        }

        // Free everything
        free (array->data);
        free (array->usedMap);
        free (array);
    }
}

LIBNEX_PUBLIC void* ArrayGetElement (Array_t* array, size_t pos)
{
    if (!array || pos >= array->numElements)
        return NULL;
    return (char*) array->data + (pos * array->elemSize);
}

LIBNEX_PUBLIC bool ArrayCheckElement (Array_t* array, size_t pos)
{
    if (!array || pos >= array->numElements)
        return NULL;
    // Get in bitmap
    size_t byteIdx = pos / 8;
    size_t bitIdx = pos % 8;
    if (array->usedMap[byteIdx] & (1 << bitIdx))
        return true;
    return false;
}

LIBNEX_PUBLIC void ArrayRemoveElement (Array_t* array, size_t pos)
{
    if (!array || pos >= array->numElements)
        return;
    // Get location in bitmap
    size_t byteIdx = pos / 8;
    size_t bitIdx = pos % 8;
    // Check if element is actually in use
    if (!(array->usedMap[byteIdx] & (1 << bitIdx)))
        return;
    // If there's a destroy callback, call it
    if (array->destroyFunc)
    {
        void* elem = (char*) array->data + (pos * array->elemSize);
        array->destroyFunc (elem);
    }
    // Mark as unused
    array->usedMap[byteIdx] &= ~(1 << bitIdx);
    // Set the free hint to this
    // FIXME: this may be a bad idea if the array isn't very fragmented
    // Needs testing
    array->freeHint = pos;
}

LIBNEX_PUBLIC bool ArrayMarkElementUsed (Array_t* array, size_t pos)
{
    if (!array || pos >= array->numElements)
        return false;
    // Get location in map
    size_t byteIdx = pos / 8;
    size_t bitIdx = pos % 8;
    // If already used, return false
    if (array->usedMap[byteIdx] & (1 << bitIdx))
        return false;
    // Mark as in use
    array->usedMap[byteIdx] |= (1 << bitIdx);
    return true;
}

// Helper function to check a range of an array
static inline size_t arrayCheckRange (Array_t* array, size_t start, size_t end)
{
    for (size_t i = start; i < end; i++)
    {
        // Get location in bitmap
        size_t byteIdx = i / 8;
        size_t bitIdx = i % 8;
        if (!(array->usedMap[byteIdx] & (1 << bitIdx)))
        {
            // Mark as used, clearing old data
            void* elem = (char*) array->data + (i * array->elemSize);
            memset (elem, 0, array->elemSize);
            array->usedMap[byteIdx] |= (1 << bitIdx);
            return i;
        }
    }
    return -1;
}

LIBNEX_PUBLIC size_t ArrayFindFreeElement (Array_t* array)
{
    if (!array)
    {
        LibnexSetError (LIBNEX_ERR_BAD_PARAM);
        return ARRAY_ERROR;
    }
    // Search for an unused element
    // Start with freeHint
    size_t idx = arrayCheckRange (array, array->freeHint, array->numElements);
    if (idx != -1)
    {
        array->freeHint = idx + 1;
        return idx;
    }
    // Check under free hint
    if (array->freeHint)
    {
        idx = arrayCheckRange (array, 0, array->freeHint);
        if (idx != -1)
        {
            array->freeHint = idx + 1;
            return idx;
        }
    }
    // No free elements found, try to expand if allowed
    if (array->numElements >= array->maxElements)
    {
        LibnexSetError (LIBNEX_ERR_BOUNDS);
        return ARRAY_ERROR;
    }
    // Calculate new size
    size_t newSize = array->numElements + array->growSize;
    if (newSize > array->maxElements)
        newSize = array->maxElements;
    // Check if we need to re-allocate used map
    // mapSize is in bytes, each byte tracks 8 elements
    if (newSize > (array->mapSize * 8))
    {
        size_t oldMapSize = array->mapSize;
        uint8_t* newUsedMap = realloc_s (array->usedMap, array->mapSize * 2);
        if (!newUsedMap)
        {
            LibnexSetError (LIBNEX_ERR_OOM);
            return ARRAY_ERROR;
        }
        array->usedMap = newUsedMap;
        array->mapSize *= 2;
        // Clear it
        memset (array->usedMap + oldMapSize, 0, array->mapSize - oldMapSize);
    }
    // Expand data buffer
    void* newData = realloc_s (array->data, newSize * array->elemSize);
    if (!newData)
    {
        LibnexSetError (LIBNEX_ERR_OOM);
        return ARRAY_ERROR;
    }
    // Zero out the newly allocated elements
    memset ((char*) newData + (array->numElements * array->elemSize),
            0,
            (newSize - array->numElements) * array->elemSize);
    // Mark the first new element as used
    size_t freeIdx = array->numElements;
    size_t byteIdx = freeIdx / 8;
    size_t bitIdx = freeIdx % 8;
    array->usedMap[byteIdx] |= (1 << bitIdx);
    // Update size and increase grow size
    array->numElements = newSize;
    array->growSize *= 2;
    array->data = newData;
    array->freeHint = freeIdx + 1;
    return freeIdx;
}

LIBNEX_PUBLIC size_t ArrayFindElement (Array_t* array, const void* hint)
{
    if (!array || !array->findFunc)
    {
        LibnexSetError (LIBNEX_ERR_BAD_PARAM);
        return ARRAY_ERROR;
    }
    // Search through all elements
    for (size_t i = 0; i < array->numElements; i++)
    {
        // Get bitmap
        size_t byteIdx = i / 8;
        size_t bitIdx = i % 8;
        // Check if element is in use
        if (!(array->usedMap[byteIdx] & (1 << bitIdx)))
            continue;
        void* elem = (char*) array->data + (i * array->elemSize);
        if (array->findFunc (elem, hint))
            return i;
    }
    return ARRAY_ERROR;
}

LIBNEX_PUBLIC ArrayIter_t* ArrayIterate (Array_t* array, ArrayIter_t* iter)
{
    if (!array || !iter)
        return NULL;
    // Find the next used element
    while (iter->idx < array->numElements)
    {
        int i = iter->idx;
        iter->idx++;
        size_t byteIdx = i / 8;
        size_t bitIdx = i % 8;
        if (array->usedMap[byteIdx] & (1 << bitIdx))
        {
            iter->ptr = (char*) array->data + (i * array->elemSize);
            return iter;
        }
    }
    // No more elements
    iter->ptr = NULL;
    return NULL;
}

LIBNEX_PUBLIC void ArraySetFindBy (Array_t* array, ArrayFindBy func)
{
    if (array)
        array->findFunc = func;
}

LIBNEX_PUBLIC size_t ArrayGetSize(Array_t* array)
{
    return array->numElements;
}
