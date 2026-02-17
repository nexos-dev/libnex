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

// Global counter for destroy callback testing
static int destroyCallCount = 0;

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
    destroyCallCount++;
}

void destroyTestStructNoCount (void* elem)
{
    UNUSED (elem);
}

int main()
{
    // ===== BASIC FUNCTIONALITY TESTS =====

    // Test ArrayCreate
    Array_t* arr = ArrayCreate (4, 20, sizeof (TestStruct_t), destroyTestStructNoCount);
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

    // Test array expansion
    size_t freeIdx2 = ArrayFindFreeElement (arr);
    TEST_BOOL_ANON (freeIdx2 == 4);
    TestStruct_t* elem5 = (TestStruct_t*) ArrayGetElement (arr, freeIdx2);
    TEST_BOOL_ANON (elem5 != NULL);
    elem5->num = 500;

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

    // Test ArrayCheckElement for existing elements
    TEST_BOOL_ANON (ArrayCheckElement (arr, 0) == true);           // Element 1 exists
    TEST_BOOL_ANON (ArrayCheckElement (arr, 2) == true);           // Element 3 exists
    TEST_BOOL_ANON (ArrayCheckElement (arr, freeIdx) == true);     // Element 4 exists
    TEST_BOOL_ANON (ArrayCheckElement (arr, freeIdx2) == true);    // Element 5 exists

    // Test ArrayCheckElement for removed element
    TEST_BOOL_ANON (ArrayCheckElement (arr, 1) == false);    // Element 2 was removed

    // Test ArrayCheckElement for never-used element
    TEST_BOOL_ANON (ArrayCheckElement (arr, 10) == false);

    // Test ArrayCheckElement for out-of-bounds element
    TEST_BOOL_ANON (ArrayCheckElement (arr, 5005) == false);

    // Test ArrayDestroy
    ArrayDestroy (arr);

    // ===== EDGE CASE TESTS =====

    // Test NULL pointer handling for ArrayGetElement
    TEST_BOOL_ANON (ArrayGetElement (NULL, 0) == NULL);

    // Test ArrayCheckElement with NULL array
    TEST_BOOL_ANON (ArrayCheckElement (NULL, 0) == false);

    // Test ArrayFindFreeElement with NULL array
    TEST_BOOL_ANON (ArrayFindFreeElement (NULL) == ARRAY_ERROR);

    // Test ArrayFindElement without find function set
    Array_t* arr2 = ArrayCreate (4, 10, sizeof (TestStruct_t), NULL);
    TEST_BOOL_ANON (arr2 != NULL);
    TEST_BOOL_ANON (ArrayFindElement (arr2, (void*) 123) == ARRAY_ERROR);

    // Test marking out of bounds element
    TEST_BOOL_ANON (ArrayMarkElementUsed (arr2, 999) == false);

    // Test removing non-existent element (should not crash)
    ArrayRemoveElement (arr2, 99);

    // Test removing already-removed element
    size_t idx = ArrayFindFreeElement (arr2);
    TEST_BOOL_ANON (idx != ARRAY_ERROR);
    ArrayRemoveElement (arr2, idx);
    ArrayRemoveElement (arr2, idx);    // Try removing again

    ArrayDestroy (arr2);

    // Test iterator on empty array
    Array_t* arr3 = ArrayCreate (4, 10, sizeof (TestStruct_t), NULL);
    ArrayIter_t emptyIter = {0};
    TEST_BOOL_ANON (ArrayIterate (arr3, &emptyIter) == NULL);

    // Test iterator with NULL parameters
    TEST_BOOL_ANON (ArrayIterate (NULL, &emptyIter) == NULL);
    TEST_BOOL_ANON (ArrayIterate (arr3, NULL) == NULL);

    // Test single element iterator
    size_t singleIdx = ArrayFindFreeElement (arr3);
    TestStruct_t* singleElem = (TestStruct_t*) ArrayGetElement (arr3, singleIdx);
    singleElem->num = 777;
    ArrayIter_t singleIter = {0};
    ArrayIter_t* singleRes = ArrayIterate (arr3, &singleIter);
    TEST_BOOL_ANON (singleRes != NULL);
    TEST_BOOL_ANON (((TestStruct_t*) singleRes->ptr)->num == 777);
    singleRes = ArrayIterate (arr3, singleRes);
    TEST_BOOL_ANON (singleRes == NULL);

    ArrayDestroy (arr3);

    // Test element zeroing after ArrayFindFreeElement
    Array_t* arr4 = ArrayCreate (2, 10, sizeof (TestStruct_t), NULL);
    size_t zeroIdx = ArrayFindFreeElement (arr4);
    TestStruct_t* zeroElem = (TestStruct_t*) ArrayGetElement (arr4, zeroIdx);
    TEST_BOOL_ANON (zeroElem->num == 0);    // Should be zeroed
    ArrayDestroy (arr4);

    // Test ArraySetDestroy
    Array_t* arr5 = ArrayCreate (2, 10, sizeof (TestStruct_t), destroyTestStruct);
    destroyCallCount = 0;
    size_t dIdx = ArrayFindFreeElement (arr5);
    ArrayRemoveElement (arr5, dIdx);
    TEST_BOOL_ANON (destroyCallCount == 1);
    ArrayDestroy (arr5);

    // ===== STRESS TESTS =====

    // Test large-scale expansion
    Array_t* bigArr = ArrayCreate (8, 1000, sizeof (TestStruct_t), NULL);
    TEST_BOOL_ANON (bigArr != NULL);
    ArraySetFindBy (bigArr, findByNum);

    // Fill array to capacity
    for (int i = 0; i < 1000; i++)
    {
        size_t idx = ArrayFindFreeElement (bigArr);
        if (idx == ARRAY_ERROR)
            break;
        TestStruct_t* elem = (TestStruct_t*) ArrayGetElement (bigArr, idx);
        TEST_BOOL_ANON (elem != NULL);
        elem->num = i + 1000;
    }

    // Try to exceed capacity
    size_t overIdx = ArrayFindFreeElement (bigArr);
    TEST_BOOL_ANON (overIdx == ARRAY_ERROR);

    // Verify all elements are findable
    for (int i = 0; i < 100; i++)    // Sample check
    {
        size_t found = ArrayFindElement (bigArr, (void*) (uintptr_t) (i + 1000));
        TEST_BOOL_ANON (found != ARRAY_ERROR);
    }

    // Test iterator over large array
    ArrayIter_t bigIter = {0};
    int iterCount = 0;
    ArrayIter_t* bigIterRes = ArrayIterate (bigArr, &bigIter);
    while (bigIterRes != NULL)
    {
        iterCount++;
        bigIterRes = ArrayIterate (bigArr, bigIterRes);
    }
    TEST_BOOL_ANON (iterCount == 1000);

    ArrayDestroy (bigArr);

    // Test repeated add/remove cycles
    Array_t* cycleArr = ArrayCreate (4, 2000, sizeof (TestStruct_t), NULL);
    ArraySetFindBy (cycleArr, findByNum);

    for (int cycle = 0; cycle < 10; cycle++)
    {
        // Add 20 elements
        for (int i = 0; i < 20; i++)
        {
            size_t idx = ArrayFindFreeElement (cycleArr);
            TEST_BOOL_ANON (idx != ARRAY_ERROR);
            TestStruct_t* elem = (TestStruct_t*) ArrayGetElement (cycleArr, idx);
            elem->num = cycle * 1000 + i;
        }

        // Remove 10 elements
        for (int i = 0; i < 10; i++)
        {
            size_t found = ArrayFindElement (cycleArr, (void*) (uintptr_t) (cycle * 1000 + i));
            if (found != ARRAY_ERROR)
            {
                ArrayRemoveElement (cycleArr, found);
            }
        }
    }

    ArrayDestroy (cycleArr);

    // Test fragmentation (remove every other element)
    Array_t* fragArr = ArrayCreate (4, 100, sizeof (TestStruct_t), NULL);
    TEST_BOOL_ANON (fragArr != NULL);

    // Add 50 elements
    size_t indices[50];
    for (int i = 0; i < 50; i++)
    {
        indices[i] = ArrayFindFreeElement (fragArr);
        TEST_BOOL_ANON (indices[i] != ARRAY_ERROR);
        TestStruct_t* elem = (TestStruct_t*) ArrayGetElement (fragArr, indices[i]);
        elem->num = i + 2000;
    }

    // Remove every other element
    for (int i = 0; i < 50; i += 2)
    {
        ArrayRemoveElement (fragArr, indices[i]);
    }

    // Verify remaining elements
    for (int i = 1; i < 50; i += 2)
    {
        TestStruct_t* elem = (TestStruct_t*) ArrayGetElement (fragArr, indices[i]);
        TEST_BOOL_ANON (elem != NULL);
        TEST_BOOL_ANON (elem->num == i + 2000);
    }

    // Verify holes can be filled
    for (int i = 0; i < 25; i++)
    {
        size_t newIdx = ArrayFindFreeElement (fragArr);
        TEST_BOOL_ANON (newIdx != ARRAY_ERROR);
    }

    ArrayDestroy (fragArr);

    // Test destroy callback is called for all elements
    destroyCallCount = 0;
    Array_t* destroyArr = ArrayCreate (4, 50, sizeof (TestStruct_t), destroyTestStruct);

    // Add 30 elements
    for (int i = 0; i < 30; i++)
    {
        size_t idx = ArrayFindFreeElement (destroyArr);
        TEST_BOOL_ANON (idx != ARRAY_ERROR);
    }

    // Destroy should call callback for all 30 elements
    ArrayDestroy (destroyArr);
    TEST_BOOL_ANON (destroyCallCount == 30);

    // Test grow size doubling
    Array_t* growArr = ArrayCreate (2, 200, sizeof (TestStruct_t), NULL);
    TEST_BOOL_ANON (growArr != NULL);

    // Force multiple expansions by filling incrementally
    // Initial: 2, after 1st expand: 4, after 2nd: 8, after 3rd: 16, etc.
    for (int i = 0; i < 100; i++)
    {
        size_t idx = ArrayFindFreeElement (growArr);
        TEST_BOOL_ANON (idx != ARRAY_ERROR);
        TestStruct_t* elem = (TestStruct_t*) ArrayGetElement (growArr, idx);
        TEST_BOOL_ANON (elem != NULL);
        elem->num = i + 3000;
    }

    // Verify all elements are accessible
    for (int i = 0; i < 100; i++)
    {
        TestStruct_t* elem = (TestStruct_t*) ArrayGetElement (growArr, i);
        TEST_BOOL_ANON (elem != NULL);
        TEST_BOOL_ANON (elem->num == i + 3000);
    }

    ArrayDestroy (growArr);

    // Test bitmap expansion beyond initial 4096 bytes (32768 bits)
    // This tests if bitmap reallocation works correctly
    Array_t* bitmapArr = ArrayCreate (100, 40000, sizeof (TestStruct_t), NULL);
    TEST_BOOL_ANON (bitmapArr != NULL);

    // Add elements beyond bitmap initial capacity
    for (int i = 0; i < 35000; i++)
    {
        size_t idx = ArrayFindFreeElement (bitmapArr);
        TEST_BOOL_ANON (idx != ARRAY_ERROR);
        if (i % 1000 == 0)    // Sample verification
        {
            TestStruct_t* elem = (TestStruct_t*) ArrayGetElement (bitmapArr, idx);
            TEST_BOOL_ANON (elem != NULL);
            elem->num = i + 4000;
        }
    }

    // Verify elements across bitmap boundary
    for (int i = 0; i < 35000; i += 1000)
    {
        TestStruct_t* elem = (TestStruct_t*) ArrayGetElement (bitmapArr, i);
        TEST_BOOL_ANON (elem != NULL);
    }

    ArrayDestroy (bitmapArr);

    // Test iterator after removing all elements
    Array_t* emptyAfterRemove = ArrayCreate (4, 20, sizeof (TestStruct_t), NULL);
    size_t idx1 = ArrayFindFreeElement (emptyAfterRemove);
    size_t idx2 = ArrayFindFreeElement (emptyAfterRemove);
    ArrayRemoveElement (emptyAfterRemove, idx1);
    ArrayRemoveElement (emptyAfterRemove, idx2);

    ArrayIter_t emptyAfterIter = {0};
    TEST_BOOL_ANON (ArrayIterate (emptyAfterRemove, &emptyAfterIter) == NULL);

    ArrayDestroy (emptyAfterRemove);

    return 0;
}
