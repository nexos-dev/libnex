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

#define ARRAY_MAP_SIZE 4096

LIBNEX_PUBLIC Array_t* ArrayCreate (size_t elements, size_t maxElems, size_t elemSize)
{
    assert (elements > 0);
    assert (maxElems > 0);
    assert (elemSize > 0);
    assert (elements <= maxElems);
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
    // Create map of bitmaps. Based on maxElems. We want there to be a bitmap for every ARRAY_MAP_SIZE elements.
    // That sould be reasonable to cover most needs. And then if someone needs more than ARRAY_MAP_SIZE elements,
    // we will just add another bitmap as needed.
    // If maxElems is changed, we will need to reallocate the map of maps
    // First get number of bitmaps
    size_t numMaps = (maxElems + (ARRAY_MAP_SIZE - 1)) / ARRAY_MAP_SIZE;
    arr->maxMaps = numMaps;
    arr->usedMaps = calloc_s (numMaps * sizeof (uint8_t**));
    if (!arr->usedMaps)
    {
        free (arr->data);
        free (arr);
        return NULL;
    }
    // Create first bitmap
    arr->usedMaps[0] = calloc_s (ARRAY_MAP_SIZE / 8);
    if (!arr->usedMaps[0])
    {
        free (arr->usedMaps);
        free (arr->data);
        free (arr);
        return NULL;
    }
    // Initialize array members
    arr->hotMap = arr->usedMaps[0];
    arr->elemSize = elemSize;
    arr->numElements = elements;
    arr->maxElements = maxElems;
    arr->growSize = elements;
    arr->findFunc = NULL;
    arr->destroyFunc = NULL;
    arr->numMaps = 1;
    return arr;
}

LIBNEX_PUBLIC void ArrayDestroy (Array_t* array)
{
    if (!array)
        return;
    // If there's a destroy callback, call it on all used elements
    if (array->destroyFunc)
    {
        for (size_t i = 0; i < array->numElements; i++)
        {
            // Get bitmap for this element
            uint8_t* map = array->usedMaps[i / ARRAY_MAP_SIZE];
            size_t byteIdx = (i % ARRAY_MAP_SIZE) / 8;
            size_t bitIdx = (i % ARRAY_MAP_SIZE) % 8;
            if (map[byteIdx] & (1 << bitIdx))
            {
                void* elem = (char*) array->data + (i * array->elemSize);
                array->destroyFunc (elem);
            }
        }
    }

    // Free everything
    free (array->data);
    for (int i = 0; i < array->numMaps; ++i)
        free (array->usedMaps[i]);
    free (array->usedMaps);
    ObjDestroy (&array->obj);
    free (array);
}

LIBNEX_PUBLIC void* ArrayGetElement (Array_t* array, size_t pos)
{
    if (!array || pos >= array->numElements)
        return NULL;
    return (char*) array->data + (pos * array->elemSize);
}

LIBNEX_PUBLIC void ArrayRemoveElement (Array_t* array, size_t pos)
{
    if (!array || pos >= array->numElements)
        return;
    // Get bitmap for this element. If pos is less than 4096, we will use the hot map
    uint8_t* map = (pos < ARRAY_MAP_SIZE) ? array->hotMap : array->usedMaps[pos / ARRAY_MAP_SIZE];
    // Get location in bitmap
    size_t byteIdx = (pos % ARRAY_MAP_SIZE) / 8;
    size_t bitIdx = (pos % ARRAY_MAP_SIZE) % 8;
    // Check if element is actually in use
    if (!(map[byteIdx] & (1 << bitIdx)))
        return;
    // If there's a destroy callback, call it
    if (array->destroyFunc)
    {
        void* elem = (char*) array->data + (pos * array->elemSize);
        array->destroyFunc (elem);
    }
    // Mark as unused
    map[byteIdx] &= ~(1 << bitIdx);
}

LIBNEX_PUBLIC bool ArrayMarkElementUsed (Array_t* array, size_t pos)
{
    if (!array || pos >= array->numElements)
        return false;
    // Get bitmap for this element
    uint8_t* map = (pos < ARRAY_MAP_SIZE) ? array->hotMap : array->usedMaps[pos / ARRAY_MAP_SIZE];
    // Get location in map
    size_t byteIdx = (pos % ARRAY_MAP_SIZE) / 8;
    size_t bitIdx = (pos % ARRAY_MAP_SIZE) % 8;
    // If already used, return false
    if (map[byteIdx] & (1 << bitIdx))
        return false;
    // Mark as in use
    map[byteIdx] |= (1 << bitIdx);
    return true;
}

LIBNEX_PUBLIC size_t ArrayFindFreeElement (Array_t* array)
{
    if (!array)
    {
        LibnexSetError (LIBNEX_ERR_BAD_PARAM);
        return ARRAY_ERROR;
    }
    // Search for an unused element
    for (size_t i = 0; i < array->numElements; i++)
    {
        // Get bitmap for this element
        uint8_t* map = (i < ARRAY_MAP_SIZE) ? array->hotMap : array->usedMaps[i / ARRAY_MAP_SIZE];
        // Get location in bitmap
        size_t byteIdx = (i % ARRAY_MAP_SIZE) / 8;
        size_t bitIdx = (i % ARRAY_MAP_SIZE) % 8;
        if (!(map[byteIdx] & (1 << bitIdx)))
        {
            // Mark as used, clearing old data
            void* elem = (char*) array->data + (i * array->elemSize);
            memset (elem, 0, array->elemSize);
            map[byteIdx] |= (1 << bitIdx);
            return i;
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
    // Expand data buffer
    void* newData = realloc_s (array->data, newSize * array->elemSize);
    if (!newData)
    {
        LibnexSetError (LIBNEX_ERR_OOM);
        return ARRAY_ERROR;
    }
    array->data = newData;
    // Determine if we need to add a new bitmap
    if (newSize > array->numMaps * ARRAY_MAP_SIZE)
    {
        // Create a new bitmap
        uint8_t* newMap = calloc_s (ARRAY_MAP_SIZE / 8);
        if (!newMap)
        {
            LibnexSetError (LIBNEX_ERR_OOM);
            return ARRAY_ERROR;
        }
        // Add to map of maps
        array->usedMaps[array->numMaps + 1] = newMap;
        array->numMaps++;
    }
    // Mark the first new element as used
    size_t freeIdx = array->numElements;
    size_t byteIdx = (freeIdx % ARRAY_MAP_SIZE) / 8;
    size_t bitIdx = (freeIdx % ARRAY_MAP_SIZE) % 8;
    uint8_t* map = array->usedMaps[freeIdx / ARRAY_MAP_SIZE];
    map[byteIdx] |= (1 << bitIdx);
    // Update size
    array->numElements = newSize;
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
        // Get bitmap for this element
        uint8_t* map = (i < ARRAY_MAP_SIZE) ? array->hotMap : array->usedMaps[i / ARRAY_MAP_SIZE];
        size_t byteIdx = (i % ARRAY_MAP_SIZE) / 8;
        size_t bitIdx = (i % ARRAY_MAP_SIZE) % 8;
        // Check if element is in use
        if (!(map[byteIdx] & (1 << bitIdx)))
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
        uint8_t* map = (i < ARRAY_MAP_SIZE) ? array->hotMap : array->usedMaps[i / ARRAY_MAP_SIZE];
        size_t byteIdx = (i % ARRAY_MAP_SIZE) / 8;
        size_t bitIdx = (i % ARRAY_MAP_SIZE) % 8;
        if (map[byteIdx] & (1 << bitIdx))
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

LIBNEX_PUBLIC void ArraySetDestroy (Array_t* array, ArrayDestroyElem func)
{
    if (array)
        array->destroyFunc = func;
}
