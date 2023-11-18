/*
    array.h - contains dynamic array implementation
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

/// @file array.h

#ifndef _ARRAY_H
#define _ARRAY_H

#include <libnex/decls.h>
#include <libnex/list.h>
#include <libnex/object.h>
#include <limits.h>
#include <stdbool.h>
#include <stddef.h>

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
    Object_t obj;                   ///< Underlying object
    ListHead_t* arrays;             ///< Allocated arrays
    int numArrays;                  ///< Number of arrays currently
    size_t numElems;                ///< Current number of initalized elements
    size_t totalElems;              ///< Total number of elements, including uninitialized ones
    size_t growSize;                ///< Number of elements to grow by
    size_t maxElems;                ///< Max number of elements
    size_t elemSize;                ///< Size of an element
    bool usesObj;                   ///< Wheter data elements use Object_t struct
    ArrayFindBy findByFun;          ///< Find by function
    ArrayDestroyElem destroyFun;    ///< Destroy function
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
Array_t* ArrayCreate (size_t elements, size_t maxElems, size_t elemSize);

/**
 * @brief Destroys a dynamic array
 * @param array array to destroy
 */
void ArrayDestroy (Array_t* array);

/**
 * @brief Gets element pointer
 * @param array Array to add element to
 * @param pos Position of element.
 * @return Element pointer
 */
void* ArrayGetElement (Array_t* array, size_t pos);

/**
 * @brief Remove element from array
 * @param array Array to remove from
 * @param pos Position of element to remove
 */
void ArrayRemoveElement (Array_t* array, size_t pos);

/**
 * @brief Find first free element
 * @param array Array to work on
 * @param expand Wheter we are allowed to expand the array if needed
 * @return Position of free element
 */
size_t ArrayFindFreeElement (Array_t* array);

/**
 * @brief Finds a specified element in array
 * @param array Array to work in
 * @param hint Hint to pass to find by function
 * @return Position of found element
 */
size_t ArrayFindElement (Array_t* array, void* hint);

/**
 * @brief Sets find by function
 * @param array Array to work in
 * @param func Function
 */
void ArraySetFindBy (Array_t* array, ArrayFindBy func);

/**
 * @brief Sets destroy function
 * @param array Array to work in
 * @param func Function
 */
void ArraySetDestroy (Array_t* array, ArrayDestroyElem func);

/**
 * @brief Sets if array elements use objects
 * @param array Array to work in
 * @param usesObj Flag to set
 */
void ArraySetUseObj (Array_t* array, bool usesObj);

/**
 * @brief Iterates through array
 * @param array Array to work in
 * @param iter Iterator to work with
 * @return Iterator containing new item
 */
ArrayIter_t* ArrayIterate (Array_t* array, ArrayIter_t* iter);

__DECL_END

#define ARRAY_ERROR       0xFFFFFFFF                    ///< Signifies an array occured in a function
#define ArrayRef(item)    (ObjRef (&(item)->obj))       ///< References the underlying object
#define ArrayLock(item)   (ObjLock (&(item)->obj))      ///< Locks this array
#define ArrayUnlock(item) (ObjUnlock (&(item)->obj))    ///< Unlocks the array

#endif
