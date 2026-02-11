/*
    array.c - array test driver
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

/// @file array.c

#include <libnex.h>

#define NEXTEST_NAME "array"
#include <nextest.h>

typedef struct _tests
{
    uint32_t num;
} TestStruct_t;

bool findByNum (const void* data, const void* hint)
{
    uint32_t num = (uint32_t) (uintptr_t) hint;
    TestStruct_t* s = (TestStruct_t*) data;
    if (s->num == num)
        return true;
    return false;
}

void destroyTestStruct (void* elem)
{
    UNUSED (elem);
}

int main()
{
    // Test ArrayCreate
    Array_t* arr = ArrayCreate (4, 20, sizeof (TestStruct_t));
    TEST_BOOL_ANON (arr != NULL);

    // Test ArrayGetElement and adding elements
    TestStruct_t* elem1 = (TestStruct_t*) ArrayGetElement (arr, 0);
    TEST_BOOL_ANON (elem1 != NULL);
    TEST_BOOL_ANON (ArrayMarkElementUsed (arr, 0));
    elem1->num = 100;

    TestStruct_t* elem2 = (TestStruct_t*) ArrayGetElement (arr, 1);
    TEST_BOOL_ANON (elem2 != NULL);
    TEST_BOOL_ANON (ArrayMarkElementUsed (arr, 1));
    elem2->num = 200;

    TestStruct_t* elem3 = (TestStruct_t*) ArrayGetElement (arr, 2);
    TEST_BOOL_ANON (elem3 != NULL);
    TEST_BOOL_ANON (ArrayMarkElementUsed (arr, 2));
    elem3->num = 300;
    // Test getting an out of bounds element
    TEST_BOOL_ANON (ArrayGetElement (arr, 5005) == NULL);
    // Test marking an in use element
    TEST_BOOL_ANON (ArrayMarkElementUsed (arr, 1) == false);

    // Test ArrayFindFreeElement
    size_t freeIdx = ArrayFindFreeElement (arr);
    TEST_BOOL_ANON (freeIdx == 3);
    TestStruct_t* elem4 = (TestStruct_t*) ArrayGetElement (arr, freeIdx);
    TEST_BOOL_ANON (elem4 != NULL);
    elem4->num = 400;

    // Test array expasion
    size_t freeIdx2 = ArrayFindFreeElement (arr);
    TEST_BOOL_ANON (freeIdx2 == 4);
    TestStruct_t* elem5 = (TestStruct_t*) ArrayGetElement (arr, freeIdx);
    TEST_BOOL_ANON (elem4 != NULL);
    elem4->num = 500;

    // Test ArraySetFindBy
    ArraySetFindBy (arr, findByNum);

    // Test ArrayFindElement
    size_t foundIdx = ArrayFindElement (arr, (void*) (uintptr_t) 200);
    TEST_BOOL_ANON (foundIdx != ARRAY_ERROR);
    TestStruct_t* foundElem = (TestStruct_t*) ArrayGetElement (arr, foundIdx);
    TEST_BOOL_ANON (foundElem->num == 200);

    // Test finding another element
    foundIdx = ArrayFindElement (arr, (void*) (uintptr_t) 300);
    TEST_BOOL_ANON (foundIdx != ARRAY_ERROR);
    foundElem = (TestStruct_t*) ArrayGetElement (arr, foundIdx);
    TEST_BOOL_ANON (foundElem->num == 300);

    // Test finding non-existent element
    foundIdx = ArrayFindElement (arr, (void*) (uintptr_t) 999);
    TEST_BOOL_ANON (foundIdx == ARRAY_ERROR);

    // Test ArrayIterate
    ArrayIter_t iter = {0};
    ArrayIter_t* iterRes = ArrayIterate (arr, &iter);
    TEST_BOOL_ANON (iterRes != NULL && iterRes->ptr != NULL);
    TestStruct_t* iterElem = (TestStruct_t*) iterRes->ptr;
    TEST_BOOL_ANON (iterElem->num == 100);

    iterRes = ArrayIterate (arr, iterRes);
    TEST_BOOL_ANON (iterRes != NULL && iterRes->ptr != NULL);
    iterElem = (TestStruct_t*) iterRes->ptr;
    TEST_BOOL_ANON (iterElem->num == 200);

    // Test ArrayRemoveElement
    ArrayRemoveElement (arr, 1);

    // Check that element 2 is now removed
    foundIdx = ArrayFindElement (arr, (void*) (uintptr_t) 200);
    TEST_BOOL_ANON (foundIdx == ARRAY_ERROR);

    // Element 3 should still be findable
    foundIdx = ArrayFindElement (arr, (void*) (uintptr_t) 300);
    TEST_BOOL_ANON (foundIdx != ARRAY_ERROR);

    // Test ArraySetDestroy callback
    ArraySetDestroy (arr, destroyTestStruct);

    // Test ArrayDestroy
    ArrayDestroy (arr);

    return 0;
}
