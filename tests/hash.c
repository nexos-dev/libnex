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
#include <stdlib.h>

typedef struct elem
{
    int val1;
    int val2;
} elem_t;

void destroy1 (HashBuf_t* data)
{
    free (data);
}

void destroy2 (HashBuf_t* data)
{
}

int main()
{
    HashTable_t* table1 = HashCreateTable (sizeof (elem_t), 128, destroy2, HASH_FLAG_ENTRY_IS_DATA);
    HashTable_t* table2 = HashCreateTable (sizeof (elem_t*), 64, destroy1, 0);
    TEST_BOOL (table1 && table2, "HashCreateTable");
    HashDestroyTable (table1);
    HashDestroyTable (table2);
    return 0;
}
