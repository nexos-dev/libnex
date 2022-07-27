/*
    stringref.h - manages references to strings
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

/// @file stringref.h

#ifndef _STRINGREF_H
#define _STRINGREF_H

#include <libnex/decls.h>
#include <libnex/libnex_config.h>
#include <libnex/object.h>

// String reference type
typedef struct _strref
{
    Object_t obj;       // Underlying object of string
    const void* str;    // Underlying string
} stringRef_t;

/// Macro to help avoid confusion for using stringRef on char32_t strings
#define stringRef32_t stringRef_t

__DECL_START

/**
 * @brief Creates a new string reference object
 * @param s string to reference
 * @return String referencing object
 */
LIBNEX_PUBLIC stringRef_t* StrRefCreate (const void* s);

/**
 * @brief Creates a new reference to a string
 * @param ref string reference object
 * @return our reference
 */
static inline stringRef_t* StrRefNew (const stringRef_t* ref)
{
    // Increase refcount on object
    ObjRef (&ref->obj);
    return (stringRef_t*) ref;
}

/**
 * @brief Deletes a reference to a string
 * @param ref reference to delete
 */
static inline void StrRefDestroy (const stringRef_t* ref)
{
    ObjDeRef (&ref->obj);
}

__DECL_END

#endif
