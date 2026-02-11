/*
    array.h - contains dynamic array implementation
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

/// @file array.h

#ifndef _ARRAY_H
#define _ARRAY_H

#include <libnex/decls.h>
#include <libnex/list.h>
#include <libnex/object.h>
#include <limits.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

/// Function pointer types
typedef bool (*ArrayFindBy) (const void* elem, const void* data);
typedef void (*ArrayDestroyElem) (void* elem);

/**
 * @brief Dynamic array structure
 *
 * Contains information to manage array structure, such as data buffer,
 * max size, current size, element size, and so on
 */
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
    uint8_t** usedMaps;              ///< Maps of bitmaps used to track used elements
    size_t numMaps;                  ///< CUrrent number of maps we have
    size_t maxMaps;                  ////< Max number of maps we can have, based on maxElements
    uint8_t* hotMap;                 ///< Bitmap for quick access to first bitmap
} Array_t;

/**
 * @brief Dynamic array iterator
 */
typedef struct _arrayiter
{
    int idx;      // Index of element
    void* ptr;    // Pointer to item
} ArrayIter_t;

__DECL_START

/**
 * @brief Creates a dynamic array
 * @param elements Number of elements for array to contains initially
 * This is also the grow size
 * @param maxElems Maximum number of elements to allow in array
 * @param elemSize Size of an element
 * @return The initialized array
 */
LIBNEX_PUBLIC Array_t* ArrayCreate (size_t elements, size_t maxElems, size_t elemSize);

/**
 * @brief Destroys a dynamic array
 * @param array array to destroy
 */
LIBNEX_PUBLIC void ArrayDestroy (Array_t* array);

/**
 * @brief Gets element pointer
 * @param array Array to add element to
 * @param pos Position of element.
 * @return Element pointer
 */
LIBNEX_PUBLIC void* ArrayGetElement (Array_t* array, size_t pos);

/**
 * @brief Sets element at position as used
 * @param array Array to work on
 * @param pos Position to set
 * @return true on success, false on failure
 */
LIBNEX_PUBLIC bool ArrayMarkElementUsed (Array_t* array, size_t pos);

/**
 * @brief Remove element from array
 * @param array Array to remove from
 * @param pos Position of element to remove
 */
LIBNEX_PUBLIC void ArrayRemoveElement (Array_t* array, size_t pos);

/**
 * @brief Find first free element
 * @param array Array to work on
 * @param expand Wheter we are allowed to expand the array if needed
 * @return Position of free element
 */
LIBNEX_PUBLIC size_t ArrayFindFreeElement (Array_t* array);

/**
 * @brief Finds a specified element in array
 * @param array Array to work in
 * @param hint Hint to pass to find by function
 * @return Position of found element
 */
LIBNEX_PUBLIC size_t ArrayFindElement (Array_t* array, const void* hint);

/**
 * @brief Sets find by function
 * @param array Array to work in
 * @param func Function
 */
LIBNEX_PUBLIC void ArraySetFindBy (Array_t* array, ArrayFindBy func);

/**
 * @brief Sets destroy function
 * @param array Array to work in
 * @param func Function
 */
LIBNEX_PUBLIC void ArraySetDestroy (Array_t* array, ArrayDestroyElem func);

/**
 * @brief Iterates through array
 * @param array Array to work in
 * @param iter Iterator to work with
 * @return Iterator containing new item
 */
LIBNEX_PUBLIC ArrayIter_t* ArrayIterate (Array_t* array, ArrayIter_t* iter);

__DECL_END

#define ARRAY_ERROR         0xFFFFFFFF                    ///< Signifies an array occured in a function
#define ArrayRef(item)      (ObjRef (&(item)->obj))       ///< References the underlying object
#define ArrayLock(item)     (ObjLock (&(item)->obj))      ///< Locks this array
#define ArrayUnlock(item)   (ObjUnlock (&(item)->obj))    ///< Unlocks the array
#define ArrayIterData(iter) ((iter)->ptr)

#endif
