/*
    stringref.c - manages references to strings
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

/// @file stringref.c

#include <libnex/safemalloc.h>
#include <libnex/stringref.h>

void destroyRef (const Object_t* obj)
{
    stringRef_t* ref = ObjGetContainer (obj, stringRef_t, obj);
    free ((void*) ref->str);
    ref->str = NULL;
    free (ref);
}

LIBNEX_PUBLIC stringRef_t* StrRefCreate (const void* s)
{
    stringRef_t* newRef = malloc_s (sizeof (stringRef_t));
    newRef->str = s;
    ObjCreate ("stringRef_t", &newRef->obj);
    ObjSetDestroy (&newRef->obj, destroyRef);
    return newRef;
}
