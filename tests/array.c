/*
    array.c - array test driver
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

#include <libnex.h>

#define NEXTEST_NAME "array"
#include <nextest.h>

// Internal array entry header
typedef struct _arrhdr
{
    Array_t* array;      // Array this belongs to
    bool initialized;    // If this entry has been initialized
    bool isUsed;         // If this entry is in use or not
} ArrayHdr_t;

#define ARRAY_DATA_OFFSET 16

typedef struct _tests
{
    uint32_t num;
} TestStruct_t;

bool findBy (const void* data, const void* hint)
{
    uint32_t num = (uint32_t) hint;
    TestStruct_t* s = data;
    if (s->num == num)
        return true;
    return false;
}

int main()
{
    Array_t* array = ArrayCreate (4, 24, sizeof (TestStruct_t));
    ArraySetFindBy (array, findBy);
    TEST_BOOL_ANON (array);
    size_t pos = ArrayFindFreeElement (array);
    TEST_BOOL_ANON (pos == 0);
    TestStruct_t* s = ArrayGetElement (array, pos);
    ArrayHdr_t* hdr = ((void*) s - ARRAY_DATA_OFFSET);
    TEST_BOOL_ANON (hdr->isUsed && (hdr->array == array));
    s->num = 0xDEADBEEF;
    TEST_BOOL_ANON (ArrayFindElement (array, (void*) 0xDEADBEEF) == 0);
    TEST_BOOL_ANON (!ArrayGetElement (array, 2));
    TEST_BOOL_ANON (!ArrayGetElement (array, 13));
    pos = ArrayFindFreeElement (array);
    TEST_BOOL_ANON (pos == 1);
    s = ArrayGetElement (array, pos);
    TEST_BOOL_ANON (s);
    s->num = 0xCAFEBABE;
    TEST_BOOL_ANON (ArrayFindElement (array, (void*) 0xCAFEBABE) == 1);
    TEST_BOOL_ANON (ArrayFindFreeElement (array) == 2);
    TEST_BOOL_ANON (ArrayFindFreeElement (array) == 3);
    pos = ArrayFindFreeElement (array);
    TEST_BOOL_ANON (pos == 4);
    s = ArrayGetElement (array, pos);
    TEST_BOOL_ANON (s);
    s->num = 0x12345678;
    TEST_BOOL_ANON (ArrayFindElement (array, (void*) 0x12345678));
    ArrayDestroy (array);
    return 0;
}
