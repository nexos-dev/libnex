/*
    stringref.c - contains test suite for string referencing
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

#include <stdio.h>
#define NEXTEST_NAME "stringref"
#include <libnex/safemalloc.h>
#include <libnex/stringref.h>
#include <nextest.h>
#include <string.h>

int main()
{
    char* s = malloc_s (128);
    if (!s)
        return 1;
    strcpy (s, "test string");
    StringRef_t* ref = StrRefCreate (s);
    TEST_BOOL (ref, "StrRefCreate() success");
    TEST (ref->obj.refCount, 1, "StrRefCreate() result validity 1");
    TEST_BOOL (!strcmp (ref->str, "test string"), "StrRefCreate() result validity 2");
    StringRef_t* ref2 = StrRefNew (ref);
    TEST (ref2->obj.refCount, 2, "StrRefNew() result validity 1");
    TEST (ref->obj.refCount, 2, "StrRefNew() result validity 2");
    TEST_BOOL (!strcmp (StrRefGet (ref2), "test string"), "StrRefNew()) result validity 3");
    StrRefDestroy (ref);
    TEST_BOOL (ref2->str, "StrRefDestroy() result validity 1");
    TEST (ref2->obj.refCount, 1, "StrRefDestroy() result validity 2");
    StrRefDestroy (ref2);
    return 0;
}
