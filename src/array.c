/*
    array.c - contains dynamic array implementation
    Copyright 2023 The NexNix Project

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

// Internal array entry header
typedef struct _arrhdr
{
    Array_t* array;      // Array this belongs to
    bool initialized;    // If this entry has been initialized
    bool isUsed;         // If this entry is in use or not
} ArrayHdr_t;

#define ARRAY_DATA_OFFSET 16

// Destroy each array
static void destroyArray (const void* data)
{
    // Destroy elements first
    ArrayHdr_t* hdr = (ArrayHdr_t*) data;
    assert (hdr->array);
    Array_t* array = hdr->array;
    for (int i = 0; i < hdr->array->growSize; ++i)
    {
        hdr = (ArrayHdr_t*) ((void*) data + (i * array->elemSize));
        if (!hdr->initialized)
            break;    // We are done
        // Ensure creator has a chance to destroy element
        if (array->usesObj)
        {
            Object_t* obj = (Object_t*) ((void*) data + (i * array->elemSize) + sizeof (ArrayHdr_t));
            ObjDestroy (obj);
        }
        else
        {
            if (array->destroyFun)
                array->destroyFun ((void*) data + (i * array->elemSize) + sizeof (ArrayHdr_t));
        }
    }
    // Free it
    free ((void*) data);
}

Array_t* ArrayCreate (size_t elements, size_t maxElems, size_t elemSize)
{
    Array_t* array = malloc_s (sizeof (Array_t));
    if (!array)
        return NULL;
    memset (array, 0, sizeof (Array_t));
    ObjCreate ("Array_t", &array->obj);
    // Ensure maxElems is a multiple of elements
    if (maxElems % elements)
    {
        free (array);
        return NULL;
    }
    // Initalize information
    array->elemSize = elemSize + ARRAY_DATA_OFFSET;
    array->maxElems = maxElems;
    array->numElems = 1;
    array->growSize = elements;
    array->totalElems = elements;
    array->numArrays = 1;
    // Initialize list of arrays
    array->arrays = ListCreate ("void*", false, 0);
    ListSetDestroy (array->arrays, destroyArray);
    if (!array->arrays)
    {
        free (array);
        return NULL;
    }
    // Allocate first array
    void* firstArray = calloc_s (array->elemSize * elements);
    if (!firstArray)
    {
        ListDestroy (array->arrays);
        free (array);
    }
    // Initialize first entry
    ArrayHdr_t* hdr = firstArray;
    hdr->array = array;
    hdr->initialized = true;
    // Add it
    ListAddBack (array->arrays, firstArray, 0);
    return array;
}

void ArrayDestroy (Array_t* array)
{
    assert (array);
    ArrayLock (array);
    if (!ObjDestroy (&array->obj))
    {
        ArrayUnlock (array);
        // Destroy array list
        ListDestroy (array->arrays);
        free (array);
    }
    ArrayUnlock (array);
}

void* ArrayGetElement (Array_t* array, size_t pos)
{
    // Determine which array this is in
    size_t arrayIn = pos / array->growSize;
    size_t arrayPos = pos % array->growSize;
    // Get array
    ListEntry_t* ent = ListFind (array->arrays, arrayIn);
    if (!ent)
        return NULL;    // Entry doesn't exist
    void* arrayPtr = ListEntryData (ent);
    // Grab element pointer
    ArrayHdr_t* hdr = (ArrayHdr_t*) (arrayPtr + (arrayPos * array->elemSize));
    if (!hdr->initialized || !hdr->isUsed)
        return NULL;    // Entry doesn't exist
    return arrayPtr + (arrayPos * array->elemSize) + ARRAY_DATA_OFFSET;
}

void ArrayRemoveElement (Array_t* array, size_t pos)
{
    ArrayLock (array);
    // Determine which array this is in
    size_t arrayIn = pos / array->growSize;
    size_t arrayPos = pos % array->growSize;
    // Get array
    ListEntry_t* ent = ListFind (array->arrays, arrayIn);
    if (!ent)
    {
        ArrayUnlock (array);
        return;    // Entry doesn't exist
    }
    void* arrayPtr = ListEntryData (ent);
    // Grab element pointer
    ArrayHdr_t* hdr = (ArrayHdr_t*) (arrayPtr + (arrayPos * array->elemSize));
    hdr->isUsed = false;    // Deallocate it
    ++array->allocatedElems;
    ArrayUnlock (array);
}

size_t ArrayFindFreeElement (Array_t* array)
{
    ArrayLock (array);
    ListEntry_t* iter = ListFront (array->arrays);
    int curArray = 0;
    while (iter)
    {
        void* arrayPtr = ListEntryData (iter);
        // Search through array
        for (int i = 0; i < array->growSize; ++i)
        {
            ArrayHdr_t* hdr = arrayPtr;
            // Check if entry is in use
            if (!hdr->isUsed)
            {
                if (!hdr->initialized)
                {
                    // Initialize this entry
                    ++array->numElems;
                    hdr->initialized = true;
                    hdr->array = array;
                }
                hdr->isUsed = true;
                ++array->allocatedElems;
                ArrayUnlock (array);
                // Return index of entry
                return (curArray * array->growSize) + i;
            }
            // Move to next entry
            arrayPtr += array->elemSize;
        }
        ++curArray;
        iter = ListIterate (iter);
    }
    // Array is full, grow it
    // First check if we are out of space to grow
    if (array->totalElems == array->maxElems)
    {
        ArrayUnlock (array);
        return ARRAY_ERROR;
    }
    void* newArray = calloc_s (array->elemSize * array->growSize);
    if (!newArray)
    {
        ArrayUnlock (array);
        return ARRAY_ERROR;
    }
    // Initialize first element
    ArrayHdr_t* hdr = newArray;
    hdr->array = array;
    hdr->initialized = true;
    hdr->isUsed = true;
    // Update accounting info
    ++array->numElems;
    array->totalElems += array->growSize;
    // Add array to list
    ListAddBack (array->arrays, newArray, array->numArrays);
    ++array->numArrays;
    // Return entry
    ArrayUnlock (array);
    return (array->numArrays - 1) * array->growSize;
}

size_t ArrayFindElement (Array_t* array, const void* hint)
{
    if (!array->findByFun)
        return ARRAY_ERROR;
    ArrayLock (array);
    ListEntry_t* iter = ListFront (array->arrays);
    int curArray = 0;
    while (iter)
    {
        void* arrayPtr = ListEntryData (iter);
        for (int i = 0; i < array->growSize; ++i)
        {
            ArrayHdr_t* hdr = arrayPtr;
            if (!hdr->initialized)
                break;    // End of this array
            if (hdr->isUsed)
            {
                // Attempt to find it
                if (array->findByFun (arrayPtr + ARRAY_DATA_OFFSET, hint))
                {
                    ArrayUnlock (array);
                    return (curArray * array->growSize) + i;
                }
            }
            arrayPtr += array->elemSize;
        }
        ++curArray;
        iter = iter->next;
    }
    ArrayUnlock (array);
    return ARRAY_ERROR;
}

ArrayIter_t* ArrayIterate (Array_t* array, ArrayIter_t* iter)
{
    ArrayLock (array);
    // Treat first iteration special
    if (!iter->ptr && !iter->idx)
    {
        // Get to first allocated element
        iter->ptr = ArrayGetElement (array, iter->idx);
        ArrayHdr_t* hdr = iter->ptr - ARRAY_DATA_OFFSET;
        while (!hdr->isUsed)
        {
            if (!hdr->initialized)
                return NULL;    // Array is empty
            iter->ptr += array->elemSize;
            hdr = iter->ptr - ARRAY_DATA_OFFSET;
        }
        ArrayUnlock (array);
        return iter;
    }
    // Increment iter index, ensure we are in bounds, and then retrieve pointer
    iter->idx++;
    if (iter->idx >= array->numElems)
    {
        ArrayUnlock (array);
        return NULL;
    }
    iter->ptr = ArrayGetElement (array, iter->idx);
    // Get to next allocated element
    ArrayHdr_t* hdr = iter->ptr - ARRAY_DATA_OFFSET;
    while (!hdr->isUsed)
    {
        if (!hdr->initialized)
            return NULL;    // End of array
        iter->ptr += array->elemSize;
        hdr = iter->ptr - ARRAY_DATA_OFFSET;
    }
    ArrayUnlock (array);
    if (!iter->ptr)
        return NULL;
    return iter;
}

void ArraySetFindBy (Array_t* array, ArrayFindBy func)
{
    assert (array);
    array->findByFun = func;
}

void ArraySetDestroy (Array_t* array, ArrayDestroyElem func)
{
    assert (array);
    array->destroyFun = func;
}

void ArraySetUseObj (Array_t* array, bool usesObj)
{
    assert (array);
    array->usesObj = usesObj;
}
