/*
    object.h - contains functions to work with objects
    Copyright 2022 The NexNix Project

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

/**
 * @file object.h
 * So, just what is an object? An object is the abstraction which most classes in libnex (and hopefully users)
 * work with. Basically, an object is anything. The idea is that a structure includes a type Object_t
 * as its first member. Users or the implementation then use the object functions on this struct to allow for
 * reference counting, comparison, and other useful things. See source or header for more info
 */

#ifndef _OBJECT_H
#define _OBJECT_H

#include <libnex/decls.h>

#ifdef IN_LIBNEX
#include <libnex_config.h>
#else
#include <libnex/libnex_config.h>
#endif

#ifdef HAVE_PTHREAD
#include <pthread.h>
typedef pthread_mutex_t lock_t;
#else
#ifndef LIBNEX_BAREMETAL
#error Target platform has no threading library
#endif
#endif

/**
 * @brief An object.
 *
 * Object_t is a type that defines an object. An object is designed to be used for conviencince by allowing
 * for comparison, type comparison, and other useful things for divergent types. It also can be used
 * to promote memory safety by reference counting. To use Object_t in your own types, ensure it is
 * the first thing in the structure. That way, users can you types coequally as Object_t and whatever its
 * native form it as well.
 */
typedef struct _Object
{
    char* type;      ///< The type of this object
    int id;          ///< A unique ID for this object
    int refCount;    ///< The number of consumers this object currently has
    lock_t lock;     ///< Used to synchronize access to this object
} Object_t;

/**
 * @brief Creates a new object
 *
 * ObjCreate() intializes a new object, assigning an ID, and lock, and also initializing a type name
 * for it.
 * @param[in] type the type to use for this object
 * @param[out] obj the object to initialize
 * @return 1 on success, 0 on error
 */
PUBLIC int ObjCreate (char* type, Object_t* obj);

/**
 * @brief Destroys an object instance
 * 
 * ObjDestroy() destroys an object. It decrements the reference count, and if it equals zero,
 * ObjDestroy() will delete the lock as well. It returns the new reference count.

#endif
