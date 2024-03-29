/*
    object.c - contains functions to work with objects
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
 * @file object.c
 * So, just what is an object? An object is the abstraction which most classes in libnex (and hopefully users)
 * work with. Basically, an object is anything. The idea is that a structure includes a type Object_t
 * as its first member. Users or the implementation then use the object functions on this struct to allow for
 * reference counting, comparison, and other useful things. See source / header for more info
 */

#include <libnex/object.h>
#include <string.h>

// The next ID to be used
static unsigned long long nextId = 1;

/**
 * @brief Creates a new object
 *
 * ObjCreate() intializes a new object, assigning an ID, and lock, and also
 * initializing a type name for it.
 * @param[in] type the type to use for this object
 * @param[out] obj the object to initialize
 */
LIBNEX_PUBLIC void ObjCreate (const char* type, Object_t* obj)
{
    // Set the members
    obj->id = nextId++;
    obj->refCount = 1;
    obj->type = type;
    obj->destroyObj = NULL;
    __Libnex_lock_init (&obj->lock);
}

LIBNEX_PUBLIC void ObjSetDestroy (Object_t* obj, void (*func) (const Object_t*))
{
    obj->destroyObj = func;
}

/**
 * @brief Destroys an object instance
 *
 * ObjDestroy() destroys an object. It decrements the reference count, and if it equals zero,
 * ObjDestroy() will delete the core object data as well. It returns the new reference count.
 * @param[in] obj the object to destroy
 */
LIBNEX_PUBLIC int ObjDestroy (const Object_t* obj)
{
    ObjLock (obj);
    --((Object_t*) obj)->refCount;
    int refCount = 0;
    if (obj->refCount == 0)
    {
        // Destroy the lock and object, as the object is done
        ObjUnlock (obj);
        __Libnex_lock_destroy (&((Object_t*) obj)->lock);
        refCount = obj->refCount;
        if (obj->destroyObj)
            obj->destroyObj ((Object_t*) obj);
    }
    else
    {
        refCount = obj->refCount;
        ObjUnlock (obj);
    }
    return refCount;
}

/**
 * @brief References an object
 *
 * ObjRef() adds a reference to this object. Use this to ensure an object isn't destroyed while another
 * consumer is using it. It should be called EVERY time a reference to an object is handed out
 * @param[in] obj the object to reference
 * @return the object
 */
LIBNEX_PUBLIC Object_t* ObjRef (const Object_t* obj)
{
    ObjLock (obj);
    ++((Object_t*) obj)->refCount;
    ObjUnlock (obj);
    return (Object_t*) obj;
}

/**
 * @brief Compares two objects
 *
 * ObjCompare() compares to object by their IDs. If the IDs are the same, then is assumes that
 * the objects are the same
 * @param[in] obj1 the first object to compare
 * @param[in] obj2 the second object to compare
 * @return 1 if they are equal, 0 other wise
 */
LIBNEX_PUBLIC int ObjCompare (const Object_t* obj1, const Object_t* obj2)
{
    return obj1->id == obj2->id;
}

/**
 * @brief Compares two object's types
 * @param[in] obj1 the first object to compare
 * @param[in] obj2 the second object to compare
 * @return 1 if equal, 0 otherwise
 */
LIBNEX_PUBLIC int ObjCompareType (const Object_t* obj1, const Object_t* obj2)
{
    return !strcmp (obj1->type, obj2->type);
}

/**
 * @brief Locks this object
 * @param[in] obj the object to lock
 */
LIBNEX_PUBLIC void ObjLock (const Object_t* obj)
{
    __Libnex_lock_lock (&((Object_t*) obj)->lock);
}

/**
 * @brief Unlocks this object
 * @param[in] obj the object to unlock
 */
LIBNEX_PUBLIC void ObjUnlock (const Object_t* obj)
{
    __Libnex_lock_unlock (&((Object_t*) obj)->lock);
}
