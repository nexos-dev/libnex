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
 * So, just what is an object? An object is the abstraction which most classes
 * in libnex (and hopefully users) work with. Basically, an object is anything.
 * The idea is that a structure includes a type Object_t.
 * Users or the implementation then use the object functions on this struct to
 * allow for reference counting, comparison, and other useful things. See source
 * or header for more info
 */

#ifndef _OBJECT_H
#define _OBJECT_H

#include <libnex/decls.h>
#include <libnex/libnex_config.h>
#include <libnex/lock.h>

/**
 * @brief An object.
 *
 * Object_t is a type that defines an object. An object is designed to be used
 * for conviencince by allowing for comparison, type comparison, and other
 * useful things for divergent types. It also can be used to promote memory
 * safety by reference counting. To use Object_t in your own types, ensure it is
 * the first thing in the structure. That way, users can you types coequally as
 * Object_t and whatever its native form it as well.
 * This is an opaque data type, meaning you shouldn't access the internal members in your
 * code
 */
typedef struct _Object
{
    const char* type;                        ///< The type of this object
    int id;                                  ///< A unique ID for this object
    int refCount;                            ///< The number of consumers this object currently has
    lock_t lock;                             ///< Used to synchronize access to this object
    void (*destroyObj) (struct _Object*);    ///< Function to destroy object with
} Object_t;

__DECL_START

/**
 * @brief Creates a new object
 *
 * ObjCreate() intializes a new object, assigning an ID, and lock, and also
 * initializing a type name for it.
 * @param[in] type the type to use for this object
 * @param[out] obj the object to initialize
 */
LIBNEX_PUBLIC void ObjCreate (const char* type, Object_t* obj);

/**
 * @brief Sets function used to destroy object
 * @param obj object to set on
 * @param func function to destroy with
 */
LIBNEX_PUBLIC void ObjSetDestroy (Object_t* obj, void (*func) (Object_t*));

/**
 * @brief Destroys an object instance
 *
 * ObjDestroy() destroys an object. It decrements the reference count, and if it equals zero,
 * ObjDestroy() will delete the core object data as well. It returns the new reference count.
 * @param[in] obj the object to destroy
 */
LIBNEX_PUBLIC int ObjDestroy (const Object_t* obj);

/// Alternate name for ObjDestroy
#define ObjDeRef(obj) (ObjDestroy (obj))

/**
 * @brief References an object
 *
 * ObjRef() adds a reference to this object. Use this to ensure an object isn't destroyed while another
 * consumer is using it. It should be called EVERY time a reference to an object is handed out
 * @param[in] obj the object to reference
 * @return the object
 */
LIBNEX_PUBLIC Object_t* ObjRef (const Object_t* obj);

/**
 * @brief Compares two objects
 *
 * ObjCompare() compares to object by their IDs. If the IDs are the same, then is assumes that
 * the objects are the same
 * @param[in] obj1 the first object to compare
 * @param[in] obj2 the second object to compare
 * @return 1 if they are equal, 0 other wise
 */
LIBNEX_PUBLIC int ObjCompare (const Object_t* obj1, const Object_t* obj2);

/**
 * @brief Compares two object's types
 * @param[in] obj1 the first object to compare
 * @param[in] obj2 the second object to compare
 * @return 1 if equal, 0 otherwise
 */
LIBNEX_PUBLIC int ObjCompareType (const Object_t* obj1, const Object_t* obj2);

/**
 * @brief Locks this object
 * @param[in] obj the object to lock
 */
LIBNEX_PUBLIC void ObjLock (const Object_t* obj);

/**
 * @brief Unlocks this object
 * @param[in] obj the object to unlock
 */
LIBNEX_PUBLIC void ObjUnlock (const Object_t* obj);

/**
 * @brief Gets the type of an object
 * @param[in] obj the object to get the type of
 * @return The name of the type
 */
#define ObjGetType(obj) (((const Object_t*) (obj))->type)

/**
 * @brief Gets containing data structure of object
 * @param ptr pointer to Object_t structure
 * @param type type of container
 * @param member member field of structure containing object
 * @return Containing data structure of object
 */
// See this for my inspiration: https://nullprogram.com/blog/2015/02/17/
#define ObjGetContainer(ptr, type, member) ((type*) ((void*) ptr - offsetof (type, member)))

__DECL_END

#endif
