/*
    hash.c - contains test suite for hash table functions
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

/// @file hash.c

#include <libnex.h>

#define NEXTEST_NAME "hash"
#include <nextest.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Test data structure for buffer mode
typedef struct
{
    int id;
    char name[32];
    double value;
} TestData_t;

// Destroy callback counter for testing
static int destroyCount = 0;

// Destroy callback for pointer mode
static void destroyPointer (void* ptr)
{
    void** p = (void**) ptr;
    if (*p)
    {
        free (*p);
        destroyCount++;
    }
}

// Destroy callback for buffer mode
static void destroyBuffer (void* buf)
{
    destroyCount++;
}

// Custom hash function for testing
static hash_t customHashFunc (const char* key)
{
    // Simple hash for testing - just sum of ASCII values
    hash_t hash = 0;
    while (*key)
    {
        hash += (hash_t) *key++;
    }
    return hash;
}

// Custom match function for testing
static bool customMatchFunc (const char* key1, const char* key2)
{
    return strcmp (key1, key2) == 0;
}

// Test basic hash table creation and destruction
static int testCreateDestroy()
{
    // Test pointer mode
    HashTable_t* table1 = HashCreateTable (sizeof (void*), 8, NULL, 0);
    TEST_BOOL (table1 != NULL, "create_pointer_table");
    HashDestroyTable (table1);

    // Test buffer mode
    HashTable_t* table2 = HashCreateTable (sizeof (TestData_t), 16, NULL, HASH_FLAG_BUF);
    TEST_BOOL (table2 != NULL, "create_buffer_table");
    HashDestroyTable (table2);

    // Test with destroy callback
    HashTable_t* table3 = HashCreateTable (sizeof (void*), 8, destroyPointer, 0);
    TEST_BOOL (table3 != NULL, "create_with_callback");
    HashDestroyTable (table3);

    return 0;
}

// Test FNV-1a hash function
static int testHashFunction()
{
    hash_t hash1 = HashCreateHash ("test");
    hash_t hash2 = HashCreateHash ("test");
    hash_t hash3 = HashCreateHash ("Test");
    hash_t hash4 = HashCreateHash ("");

    TEST_BOOL (hash1 == hash2, "hash_consistency");
    TEST_BOOL (hash1 != hash3, "hash_case_sensitive");
    TEST_BOOL (hash4 != 0, "hash_empty_string");

    // Test different strings produce different hashes (statistically)
    hash_t hashA = HashCreateHash ("apple");
    hash_t hashB = HashCreateHash ("banana");
    hash_t hashC = HashCreateHash ("cherry");

    TEST_BOOL (hashA != hashB, "hash_different_1");
    TEST_BOOL (hashB != hashC, "hash_different_2");
    TEST_BOOL (hashA != hashC, "hash_different_3");

    return 0;
}

// Test insertion in pointer mode
static int testInsertPointer()
{
    HashTable_t* table = HashCreateTable (sizeof (void*), 8, NULL, 0);
    TEST_BOOL (table != NULL, "create_table");

    // Insert some entries
    char* val1 = strdup ("value1");
    char* val2 = strdup ("value2");
    char* val3 = strdup ("value3");

    TEST_BOOL (HashInsertEntry (table, "key1", val1), "insert_1");
    TEST_BOOL (HashInsertEntry (table, "key2", val2), "insert_2");
    TEST_BOOL (HashInsertEntry (table, "key3", val3), "insert_3");

    // Test finding entries
    char* found1 = (char*) HashFindEntry (table, "key1");
    char* found2 = (char*) HashFindEntry (table, "key2");
    char* found3 = (char*) HashFindEntry (table, "key3");

    TEST_BOOL (found1 == val1, "find_1");
    TEST_BOOL (found2 == val2, "find_2");
    TEST_BOOL (found3 == val3, "find_3");
    TEST_BOOL (strcmp (found1, "value1") == 0, "find_value_1");
    TEST_BOOL (strcmp (found2, "value2") == 0, "find_value_2");
    TEST_BOOL (strcmp (found3, "value3") == 0, "find_value_3");

    // Clean up
    free (val1);
    free (val2);
    free (val3);
    HashDestroyTable (table);

    return 0;
}

// Test insertion in buffer mode
static int testInsertBuffer()
{
    HashTable_t* table = HashCreateTable (sizeof (TestData_t), 8, NULL, HASH_FLAG_BUF);
    TEST_BOOL (table != NULL, "create_table");

    // Insert some entries
    TestData_t data1 = {1, "Alice", 3.14};
    TestData_t data2 = {2, "Bob", 2.71};
    TestData_t data3 = {3, "Charlie", 1.41};

    TEST_BOOL (HashInsertEntry (table, "user1", &data1), "insert_1");
    TEST_BOOL (HashInsertEntry (table, "user2", &data2), "insert_2");
    TEST_BOOL (HashInsertEntry (table, "user3", &data3), "insert_3");

    // Test finding entries
    TestData_t found1 = {0};
    TestData_t found2 = {0};
    TestData_t found3 = {0};
    HashGetEntryBuf (table, "user1", &found1);
    HashGetEntryBuf (table, "user2", &found2);
    HashGetEntryBuf (table, "user3", &found3);

    TEST (found1.id, 1, "find_id_1");
    TEST (found2.id, 2, "find_id_2");
    TEST (found3.id, 3, "find_id_3");
    TEST_BOOL (strcmp (found1.name, "Alice") == 0, "find_name_1");
    TEST_BOOL (strcmp (found2.name, "Bob") == 0, "find_name_2");
    TEST_BOOL (strcmp (found3.name, "Charlie") == 0, "find_name_3");

    HashDestroyTable (table);

    return 0;
}

// Test HashCheckEntry
static int testCheckEntry()
{
    HashTable_t* table = HashCreateTable (sizeof (void*), 8, NULL, 0);
    TEST_BOOL (table != NULL, "create_table");

    char* val = strdup ("test_value");
    HashInsertEntry (table, "exists", val);

    TEST_BOOL (HashCheckEntry (table, "exists"), "check_exists");
    TEST_BOOL (!HashCheckEntry (table, "notexists"), "check_not_exists");
    TEST_BOOL (!HashCheckEntry (table, ""), "check_empty_key");

    free (val);
    HashDestroyTable (table);

    return 0;
}

// Test removal
static int testRemoveEntry()
{
    HashTable_t* table = HashCreateTable (sizeof (void*), 8, NULL, 0);
    TEST_BOOL (table != NULL, "create_table");

    // Insert entries
    char* val1 = strdup ("value1");
    char* val2 = strdup ("value2");
    char* val3 = strdup ("value3");

    HashInsertEntry (table, "key1", val1);
    HashInsertEntry (table, "key2", val2);
    HashInsertEntry (table, "key3", val3);

    // Verify they exist
    TEST_BOOL (HashCheckEntry (table, "key1"), "check_before_remove_1");
    TEST_BOOL (HashCheckEntry (table, "key2"), "check_before_remove_2");

    // Remove one entry
    HashRemoveEntry (table, "key2");

    // Verify removal
    TEST_BOOL (HashCheckEntry (table, "key1"), "check_after_remove_1");
    TEST_BOOL (!HashCheckEntry (table, "key2"), "check_after_remove_2");
    TEST_BOOL (HashCheckEntry (table, "key3"), "check_after_remove_3");

    // Clean up
    free (val1);
    free (val2);
    free (val3);
    HashDestroyTable (table);

    return 0;
}

// Test hash table expansion
static int testExpansion()
{
    // Start with small table (4 buckets)
    // Load factor threshold is 0.7, so should expand after 3 elements
    HashTable_t* table = HashCreateTable (sizeof (void*), 4, NULL, 0);
    TEST_BOOL (table != NULL, "create_table");

    // Insert enough elements to trigger expansion
    // With 4 buckets, inserting 3+ should trigger expansion
    char keys[20][16];
    char* values[20];

    for (int i = 0; i < 10; i++)
    {
        sprintf (keys[i], "key%d", i);
        values[i] = strdup (keys[i]);
        TEST_BOOL (HashInsertEntry (table, keys[i], values[i]), "insert_expansion");
    }

    // Verify all entries are still findable after expansion
    for (int i = 0; i < 10; i++)
    {
        TEST_BOOL (HashCheckEntry (table, keys[i]), "check_after_expansion");
        char* found = (char*) HashFindEntry (table, keys[i]);
        TEST_BOOL (found == values[i], "find_after_expansion");
    }

    // Clean up
    for (int i = 0; i < 10; i++)
    {
        free (values[i]);
    }
    HashDestroyTable (table);

    return 0;
}

// Test custom hash and match functions
static int testCustomFunctions()
{
    HashTable_t* table = HashCreateTable (sizeof (void*), 8, NULL, 0);
    TEST_BOOL (table != NULL, "create_table");

    // Set custom functions
    HashSetFuncs (table, customMatchFunc, customHashFunc);

    // Insert entries using custom hash
    char* val1 = strdup ("custom1");
    char* val2 = strdup ("custom2");

    TEST_BOOL (HashInsertEntry (table, "abc", val1), "insert_custom_1");
    TEST_BOOL (HashInsertEntry (table, "xyz", val2), "insert_custom_2");

    // Find entries
    char* found1 = (char*) HashFindEntry (table, "abc");
    char* found2 = (char*) HashFindEntry (table, "xyz");

    TEST_BOOL (found1 == val1, "find_custom_1");
    TEST_BOOL (found2 == val2, "find_custom_2");

    // Clean up
    free (val1);
    free (val2);
    HashDestroyTable (table);

    return 0;
}

// Test iterator functionality
static int testIterator()
{
    HashTable_t* table = HashCreateTable (sizeof (void*), 16, NULL, 0);
    TEST_BOOL (table != NULL, "create_table");

    // Insert several entries
    char* val1 = strdup ("iter1");
    char* val2 = strdup ("iter2");
    char* val3 = strdup ("iter3");
    char* val4 = strdup ("iter4");

    HashInsertEntry (table, "a", val1);
    HashInsertEntry (table, "b", val2);
    HashInsertEntry (table, "c", val3);
    HashInsertEntry (table, "d", val4);

    // Test iterating from beginning
    HashIter_t iter;
    HashStartIter (table, &iter, NULL);

    int count = 0;
    bool seenA = false, seenB = false, seenC = false, seenD = false;

    HashIter_t* iterPtr = &iter;
    while (iterPtr != NULL)
    {
        TEST_BOOL (iterPtr->key != NULL, "iter_key_not_null");
        TEST_BOOL (iterPtr->value != NULL, "iter_value_not_null");

        if (strcmp (iterPtr->key, "a") == 0)
        {
            TEST_BOOL (iterPtr->value == val1, "iter_value_a");
            seenA = true;
        }
        else if (strcmp (iterPtr->key, "b") == 0)
        {
            TEST_BOOL (iterPtr->value == val2, "iter_value_b");
            seenB = true;
        }
        else if (strcmp (iterPtr->key, "c") == 0)
        {
            TEST_BOOL (iterPtr->value == val3, "iter_value_c");
            seenC = true;
        }
        else if (strcmp (iterPtr->key, "d") == 0)
        {
            TEST_BOOL (iterPtr->value == val4, "iter_value_d");
            seenD = true;
        }

        count++;
        iterPtr = HashIterate (iterPtr);
    }

    // We should have visited some entries (exact count depends on implementation)
    TEST_BOOL (count >= 4, "iter_count");
    TEST_BOOL (seenA && seenB && seenC && seenD, "iter_seen_all");

    // Test iterating from specific key
    HashStartIter (table, &iter, "b");
    TEST_BOOL (iter.entry != NULL, "iter_start_from_key");
    TEST_BOOL (iter.key != NULL, "iter_start_key_not_null");
    TEST_BOOL (iter.value != NULL, "iter_start_value_not_null");

    if (iter.key && strcmp (iter.key, "b") == 0)
    {
        TEST_BOOL (iter.value == val2, "iter_start_value_b");
    }

    // Clean up
    free (val1);
    free (val2);
    free (val3);
    free (val4);
    HashDestroyTable (table);

    return 0;
}

// Test destroy callbacks
static int testDestroyCallbacks()
{
    destroyCount = 0;

    HashTable_t* table = HashCreateTable (sizeof (void*), 8, destroyPointer, 0);
    TEST_BOOL (table != NULL, "create_table");

    // Insert entries
    char* val1 = malloc (32);
    char* val2 = malloc (32);
    char* val3 = malloc (32);
    strcpy (val1, "destroy1");
    strcpy (val2, "destroy2");
    strcpy (val3, "destroy3");

    HashInsertEntry (table, "d1", val1);
    HashInsertEntry (table, "d2", val2);
    HashInsertEntry (table, "d3", val3);

    // Destroy table - should call destroy callback for each entry
    HashDestroyTable (table);

    TEST (destroyCount, 3, "destroy_callback_count");

    return 0;
}

// Test edge cases
static int testEdgeCases()
{
    HashTable_t* table = HashCreateTable (sizeof (void*), 8, NULL, 0);
    TEST_BOOL (table != NULL, "create_table");

    // Test finding non-existent entry
    void* notFound = HashFindEntry (table, "nonexistent");
    TEST_BOOL (notFound == NULL, "find_nonexistent");

    // Test removing non-existent entry (should not crash)
    HashRemoveEntry (table, "nonexistent");

    // Test empty string key
    char* val = strdup ("empty_key_value");
    TEST_BOOL (HashInsertEntry (table, "", val), "insert_empty_key");
    TEST_BOOL (HashCheckEntry (table, ""), "check_empty_key");

    // Test duplicate insertion (should fail)
    char* val2 = strdup ("duplicate");
    HashInsertEntry (table, "dup", val2);
    char* val3 = strdup ("duplicate2");
    bool dupResult = HashInsertEntry (table, "dup", val3);
    TEST_BOOL (!dupResult, "duplicate_insertion_fails");

    // Clean up
    free (val);
    free (val2);
    free (val3);
    HashDestroyTable (table);

    return 0;
}

// Test collision handling
static int testCollisions()
{
    // Use small table to force collisions
    HashTable_t* table = HashCreateTable (sizeof (void*), 4, NULL, 0);
    TEST_BOOL (table != NULL, "create_table");

    // Insert many entries into small table
    char keys[8][16];
    char* values[8];

    for (int i = 0; i < 8; i++)
    {
        sprintf (keys[i], "collision_%d", i);
        values[i] = strdup (keys[i]);
        TEST_BOOL (HashInsertEntry (table, keys[i], values[i]), "insert_collision");
    }

    // Verify all can be found despite collisions
    for (int i = 0; i < 8; i++)
    {
        char* found = (char*) HashFindEntry (table, keys[i]);
        TEST_BOOL (found == values[i], "find_after_collision");
    }

    // Clean up
    for (int i = 0; i < 8; i++)
    {
        free (values[i]);
    }
    HashDestroyTable (table);

    return 0;
}

// Stress test for large number of inserts/removals
static int testStress()
{
    const int N = 5000;

    HashTable_t* table = HashCreateTable (sizeof (void*), 64, NULL, 0);
    TEST_BOOL (table != NULL, "stress_create_table");

    char** values = (char**) calloc (N, sizeof (char*));
    TEST_BOOL (values != NULL, "stress_alloc_values");

    char key[32];

    // Insert
    for (int i = 0; i < N; i++)
    {
        snprintf (key, sizeof (key), "stress_key_%d", i);
        values[i] = strdup (key);
        TEST_BOOL (values[i] != NULL, "stress_strdup");
        TEST_BOOL (HashInsertEntry (table, key, values[i]), "stress_insert");
    }

    // Verify
    for (int i = 0; i < N; i++)
    {
        snprintf (key, sizeof (key), "stress_key_%d", i);
        char* found = (char*) HashFindEntry (table, key);
        TEST_BOOL (found == values[i], "stress_find");
    }

    // Remove half
    for (int i = 0; i < N; i += 2)
    {
        snprintf (key, sizeof (key), "stress_key_%d", i);
        HashRemoveEntry (table, key);
    }

    // Verify removals and remaining entries
    for (int i = 0; i < N; i++)
    {
        snprintf (key, sizeof (key), "stress_key_%d", i);
        bool exists = HashCheckEntry (table, key);
        if (i % 2 == 0)
        {
            TEST_BOOL (!exists, "stress_removed");
        }
        else
        {
            TEST_BOOL (exists, "stress_remaining");
        }
    }

    // Clean up
    for (int i = 0; i < N; i++)
    {
        free (values[i]);
    }
    free (values);
    HashDestroyTable (table);

    return 0;
}

int main()
{
    if (testCreateDestroy() != 0)
        return 1;
    if (testHashFunction() != 0)
        return 1;
    if (testInsertPointer() != 0)
        return 1;
    if (testInsertBuffer() != 0)
        return 1;
    if (testCheckEntry() != 0)
        return 1;
    if (testRemoveEntry() != 0)
        return 1;
    if (testExpansion() != 0)
        return 1;
    if (testCustomFunctions() != 0)
        return 1;
    if (testIterator() != 0)
        return 1;
    if (testDestroyCallbacks() != 0)
        return 1;
    if (testEdgeCases() != 0)
        return 1;
    if (testCollisions() != 0)
        return 1;
    if (testStress() != 0)
        return 1;

    return 0;
}
