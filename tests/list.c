/*
    list.c - linked list test driver
    Copyright 2022 - 2026 The NexNix Project

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

/// @file list.c

#include <libnex.h>

#define NEXTEST_NAME "list"
#include <nextest.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Test node that embeds ListEntry_t at the start
typedef struct
{
    ListEntry_t entry;
    int value;
    char name[16];
} TestNode_t;

static int destroyCount = 0;

static void destroyEntry (const void* data)
{
    destroyCount++;
    free ((void*) data);
}

static bool findByValue (const ListEntry_t* entry, const void* data)
{
    const int* value = (const int*) data;
    TestNode_t* entryValue = (TestNode_t*) ListEntryData ((ListEntry_t*) entry);
    return entryValue && (entryValue->value == *value);
}

static TestNode_t* createNode (int value, const char* name)
{
    TestNode_t* node = (TestNode_t*) calloc (1, sizeof (TestNode_t));
    if (!node)
        return NULL;
    ListEntryInit (&node->entry);
    node->value = value;
    if (name)
        strncpy (node->name, name, sizeof (node->name) - 1);
    return node;
}

// Test basic creation and destruction
static int testCreateDestroy()
{
    ListHead_t* list = ListCreate ("test", NULL, NULL, 0);
    TEST_BOOL (list != NULL, "list_create");
    ListDestroy (list);
    return 0;
}

// Test add front/back and find
static int testAddFind()
{
    ListHead_t* list = ListCreate ("test", findByValue, destroyEntry, 0);
    TEST_BOOL (list != NULL, "list_create");

    TestNode_t* n1 = createNode (11, "one");
    TestNode_t* n2 = createNode (22, "two");
    TestNode_t* n3 = createNode (33, "three");

    TEST_BOOL (n1 && n2 && n3, "node_alloc");

    ListAddFront (list, &n1->entry, 1);
    ListAddBack (list, &n2->entry, 2);
    ListAddBack (list, &n3->entry, 3);

    TEST_BOOL (ListFront (list) == &n1->entry, "front_is_n1");
    TEST_BOOL (list->back == &n3->entry, "back_is_n3");

    ListEntry_t* found2 = ListFind (list, 2);
    TEST_BOOL (found2 == &n2->entry, "find_key_2");
    TestNode_t* val2 = ListEntryData (found2);
    TEST_BOOL (val2 && val2->value, "find_value_2");

    ListDestroy (list);
    return 0;
}

// Test add before/after variants
static int testAddBeforeAfter()
{
    ListHead_t* list = ListCreate ("test", findByValue, destroyEntry, 0);
    TEST_BOOL (list != NULL, "list_create");

    TestNode_t* n1 = createNode (1, "n1");
    TestNode_t* n3 = createNode (3, "n3");
    TestNode_t* n5 = createNode (5, "n5");
    TEST_BOOL (n1 && n3 && n5, "node_alloc");

    ListAddBack (list, &n1->entry, 1);
    ListAddBack (list, &n3->entry, 3);
    ListAddBack (list, &n5->entry, 5);

    TestNode_t* n2 = createNode (2, "n2");
    TestNode_t* n4 = createNode (4, "n4");
    TEST_BOOL (n2 && n4, "node_alloc_2");

    ListAddBefore (list, &n2->entry, 2, &n3->entry);
    ListAddAfterKey (list, &n4->entry, 4, 3);

    // Verify order: 1 -> 2 -> 3 -> 4 -> 5
    int expected[] = {1, 2, 3, 4, 5};
    int idx = 0;
    ListEntry_t* cur = ListFront (list);
    while (cur && idx < 5)
    {
        TestNode_t* v = (TestNode_t*) ListEntryData (cur);
        TEST_BOOL (v && v->value == expected[idx], "order_check");
        cur = ListIterate (cur);
        idx++;
    }
    TEST_BOOL (idx == 5 && cur == NULL, "order_len");

    // Invalid key variants
    TestNode_t* nx = createNode (99, "nx");
    TEST_BOOL (nx != NULL, "node_alloc_x");
    TEST_BOOL (ListAddBeforeKey (list, &nx->entry, 99, 999) == NULL, "add_before_key_invalid");
    free (nx);

    ListDestroy (list);
    return 0;
}

// Test pop front and empty edge cases
static int testPopFront()
{
    ListHead_t* list = ListCreate ("test", findByValue, destroyEntry, 0);
    TEST_BOOL (list != NULL, "list_create");

    TEST_BOOL (ListPopFront (list) == NULL, "pop_empty");

    TestNode_t* n1 = createNode (1, "n1");
    TEST_BOOL (n1 != NULL, "node_alloc");
    ListAddFront (list, &n1->entry, 1);

    ListEntry_t* popped = ListPopFront (list);
    TEST_BOOL (popped == &n1->entry, "pop_single");
    TEST_BOOL (list->front == NULL, "pop_single_front_null");
    TEST_BOOL (list->back == NULL, "pop_single_back_null");

    free (popped);

    TestNode_t* n2 = createNode (2, "n2");
    TestNode_t* n3 = createNode (3, "n3");
    TEST_BOOL (n2 && n3, "node_alloc_2");
    ListAddBack (list, &n2->entry, 2);
    ListAddBack (list, &n3->entry, 3);

    popped = ListPopFront (list);
    TEST_BOOL (popped == &n2->entry, "pop_head");
    TEST_BOOL (list->front == &n3->entry, "front_after_pop");
    TEST_BOOL (n3->entry.prev == NULL, "front_prev_null");
    TEST_BOOL (list->back == &n3->entry, "back_after_pop");
    free (popped);

    ListDestroy (list);
    return 0;
}

// Test remove by key and remove by entry
static int testRemove()
{
    ListHead_t* list = ListCreate ("test", findByValue, destroyEntry, 0);
    TEST_BOOL (list != NULL, "list_create");

    TestNode_t* n1 = createNode (1, "n1");
    TestNode_t* n2 = createNode (2, "n2");
    TestNode_t* n3 = createNode (3, "n3");
    TEST_BOOL (n1 && n2 && n3, "node_alloc");

    ListAddBack (list, &n1->entry, 1);
    ListAddBack (list, &n2->entry, 2);
    ListAddBack (list, &n3->entry, 3);

    ListEntry_t* e2 = ListFind (list, 2);
    TEST_BOOL (e2 == &n2->entry, "find_2");
    ListRemove (list, e2);
    TEST_BOOL (ListFind (list, 2) == NULL, "removed_2");
    free (e2);

    ListEntry_t* e1 = ListFind (list, 1);
    ListRemove (list, e1);
    TEST_BOOL (list->front == &n3->entry, "front_after_remove");
    free (e1);

    ListEntry_t* e3 = ListFind (list, 3);
    ListRemove (list, e3);
    TEST_BOOL (list->front == NULL && list->back == NULL, "empty_after_remove");
    free (e3);

    ListDestroy (list);
    return 0;
}

// Test find by data
static int testFindEntryBy()
{
    ListHead_t* list = ListCreate ("test", findByValue, destroyEntry, 0);
    TEST_BOOL (list != NULL, "list_create");

    TestNode_t* n1 = createNode (10, "n1");
    TestNode_t* n2 = createNode (20, "n2");
    TestNode_t* n3 = createNode (30, "n3");
    TEST_BOOL (n1 && n2 && n3, "node_alloc");

    ListAddBack (list, &n1->entry, 1);
    ListAddBack (list, &n2->entry, 2);
    ListAddBack (list, &n3->entry, 3);

    int target = 20;
    ListEntry_t* found = ListFindEntryBy (list, &target);
    TEST_BOOL (found == &n2->entry, "find_by_value");

    int missing = 99;
    TEST_BOOL (ListFindEntryBy (list, &missing) == NULL, "find_by_value_missing");

    ListDestroy (list);
    return 0;
}

// Test destroy callbacks and ListDestroyEntry
static int testDestroyCallbacks()
{
    destroyCount = 0;
    ListHead_t* list = ListCreate ("test", findByValue, destroyEntry, 0);
    TEST_BOOL (list != NULL, "list_create");

    TestNode_t* n1 = createNode (1, "n1");
    TestNode_t* n2 = createNode (2, "n2");
    TestNode_t* n3 = createNode (3, "n3");
    TEST_BOOL (n1 && n2 && n3, "node_alloc");

    ListAddBack (list, &n1->entry, 1);
    ListAddBack (list, &n2->entry, 2);
    ListAddBack (list, &n3->entry, 3);

    ListDestroyEntry (list, &n2->entry);
    TEST_BOOL (destroyCount == 1, "destroy_entry_count");
    TEST_BOOL (ListFind (list, 2) == NULL, "destroy_entry_removed");

    ListDestroy (list);
    TEST_BOOL (destroyCount == 3, "destroy_list_count");

    return 0;
}

// Test edge cases
static int testEdgeCases()
{
    ListHead_t* list = ListCreate ("test", findByValue, destroyEntry, 0);
    TEST_BOOL (list != NULL, "list_create");

    TEST_BOOL (ListFind (list, 1) == NULL, "find_empty");
    TEST_BOOL (ListRemoveKey (list, 1) == NULL, "remove_key_empty");

    TestNode_t* nx = createNode (9, "nx");
    TEST_BOOL (nx != NULL, "node_alloc");
    TEST_BOOL (ListAddAfterKey (list, &nx->entry, 9, 999) == NULL, "add_after_key_invalid");
    free (nx);

    ListDestroy (list);
    return 0;
}

// Stress test
static int testStress()
{
    const int N = 2000;

    ListHead_t* list = ListCreate ("test", findByValue, destroyEntry, 0);
    TEST_BOOL (list != NULL, "stress_create");

    TestNode_t** nodes = (TestNode_t**) calloc (N, sizeof (TestNode_t*));
    TEST_BOOL (nodes != NULL, "stress_alloc");

    for (int i = 0; i < N; i++)
    {
        nodes[i] = createNode (i, "node");
        TEST_BOOL (nodes[i] != NULL, "stress_node_alloc");
        ListAddBack (list, &nodes[i]->entry, i);
    }

    for (int i = 0; i < N; i++)
    {
        ListEntry_t* e = ListFind (list, i);
        TEST_BOOL (e != NULL, "stress_find");
    }

    // Remove even keys
    for (int i = 0; i < N; i += 2)
    {
        ListRemoveKey (list, i);
        free (nodes[i]);
        nodes[i] = NULL;
    }

    for (int i = 0; i < N; i++)
    {
        ListEntry_t* e = ListFind (list, i);
        if (i % 2 == 0)
        {
            TEST_BOOL (e == NULL, "stress_removed");
        }
        else
        {
            TEST_BOOL (e != NULL, "stress_remaining");
        }
    }

    // Pop remaining entries
    int count = 0;
    ListEntry_t* cur = NULL;
    while ((cur = ListPopFront (list)) != NULL)
    {
        count++;
        free (cur);
    }
    TEST_BOOL (count == (N / 2), "stress_pop_count");

    free (nodes);
    ListDestroy (list);
    return 0;
}

int main()
{
    if (testCreateDestroy() != 0)
        return 1;
    if (testAddFind() != 0)
        return 1;
    if (testAddBeforeAfter() != 0)
        return 1;
    if (testPopFront() != 0)
        return 1;
    if (testRemove() != 0)
        return 1;
    if (testFindEntryBy() != 0)
        return 1;
    if (testDestroyCallbacks() != 0)
        return 1;
    if (testEdgeCases() != 0)
        return 1;
    if (testStress() != 0)
        return 1;

    return 0;
}
