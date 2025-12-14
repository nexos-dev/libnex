/*
    hash.c - contains hash table implementation
    Copyright 2022 - 2025 The NexNix Project

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

#include <assert.h>
#include <libnex/hash.h>
#include <stdlib.h>
#include <string.h>

// Hash table entry
typedef struct _htentry
{
    bool removed;       // Has this entry been removed?
    const char* key;    // Key of this entry
    HashBuf_t buf;      // Buffer of data
} HashEntry_t;

#define HASH_ARRAY_OBJ_MIN 8
#define HASH_ARRAY_OBJ_MAX 1024
#define HASH_ARRAY_MAX_SZ  (256 * 1024)

// Array of arrays expansion
#define HASH_ARRAY_ARRAY_MIN              1024
#define HASH_ARRAY_EXPAND_SIZE(numArrays) (((numArrays) * sizeof (void*)) / 2)

LIBNEX_PUBLIC HashTable_t* HashCreateTable (size_t elemSize,
                                            size_t maxElems,
                                            HashEntryDestroy destroyFunc,
                                            int flags)
{
    // Allocate the hash table
    HashTable_t* table = malloc (sizeof (HashTable_t));
    if (!table)
    {
        LibnexSetError (LIBNEX_ERR_OOM);
        return NULL;
    }
    memset (table, 0, sizeof (HashTable_t));
    // Initialize it
    ObjCreate ("HashTable_t", &table->obj);
    table->destroyFunc = destroyFunc;
    table->dataSize = elemSize;
    elemSize += sizeof (HashEntry_t);
    // Align to 64 for cache efficiency
    elemSize = AlignNumberUp (elemSize, 64);
    table->elemSize = elemSize;
    table->maxElems = (!maxElems) ? SIZE_MAX : maxElems;
    table->flags = flags;
    table->numElems = 0, table->usedElems = 0;
    // Figure out array size
    // We must fit at least HASH_ARRAY_OBJ_MIN objects in the array,
    // but also dont want it to be larger than 256KiB
    // First bound maxElems
    size_t arrayMax = (HASH_ARRAY_OBJ_MAX > maxElems) ? maxElems : HASH_ARRAY_OBJ_MAX;
    size_t arrayMin = elemSize * HASH_ARRAY_OBJ_MIN;
    size_t arraySize = arrayMax * elemSize;
    // Now ensure it is smaller then 256KiB
    while (arraySize > HASH_ARRAY_MAX_SZ)
    {
        size_t tmp = arraySize;
        tmp -= elemSize;
        // Ensure we didn't go to small
        if (tmp < arrayMin)
            break;    // we're done
    }
    table->arraySize = arraySize;
    // Allocate array
    table->array = malloc (arraySize);
    if (!table->array)
    {
        LibnexSetError (LIBNEX_ERR_OOM);
        free (table);
        return NULL;
    }
    memset (table->array, 0, arraySize);
    return table;
}

LIBNEX_PUBLIC void HashDestroyTable (HashTable_t* table)
{
    assert (table);
    if (!ObjDestroy (&table->obj))
    {
        // Free every entry
        HashEntry_t* entry = (HashEntry_t*) table->array;
        for (int i = 0; i < (table->arraySize / table->elemSize); ++i)
        {
            if (!entry->key)
                break;    // End of array
            if (entry->removed)
                continue;
            // Check if data needs to be destroyed
            HashBuf_t* buf = &entry->buf;
            if (table->destroyFunc && buf->type != HASH_TYPE_INT && buf->type != HASH_TYPE_CSTRING)
                table->destroyFunc (buf);
        }
        // Free memory
        free (table->array);
        free (table);
    }
}

// Allocates a hash table entry
static HashEntry_t* hashAllocateEntry (hash_t hash, int flags)
{
    return NULL;
}

LIBNEX_PUBLIC bool HashInsertEntry (HashTable_t* table, const char* key, HashBuf_t* buf, int flags)
{
    assert (key && table && buf);
    // Obtain hash number
    hash_t hash = HashCreateHash (key);
    // Get hash table entry
    HashEntry_t* entry = hashAllocateEntry (hash, flags);
    if (!entry)
        return false;
    entry->key = key;
    entry->removed = false;
    // Setup buffer
    entry->buf.type = buf->type;
    return true;
}

LIBNEX_PUBLIC void* HashGetEntry (HashTable_t* table, const char* key, int flags)
{
}

LIBNEX_PUBLIC bool HashInsertEntryIdx (HashTable_t* table, hash_t idx, HashBuf_t* data, int flags)
{
}

LIBNEX_PUBLIC void* HashGetEntryIdx (HashTable_t* table, hash_t idx, int flags)
{
}

LIBNEX_PUBLIC void HashRemoveEntry (HashTable_t* table, const char* key)
{
}

LIBNEX_PUBLIC void HashRemoveEntryIdx (HashTable_t* table, hash_t idx)
{
}

LIBNEX_PUBLIC void* HashFindEntry (HashTable_t* table, const char* key)
{
}

LIBNEX_PUBLIC void* HashFindEntryIdx (HashTable_t* table, hash_t idx)
{
}

LIBNEX_PUBLIC bool HashTableExpand (HashTable_t* table)
{
}

LIBNEX_PUBLIC void HashStartIterate (HashTable_t* table, HashIter_t* iter)
{
}

LIBNEX_PUBLIC HashIter_t* HashIterate (HashIter_t* iter)
{
}

// Hash function parameters
#define HASH_FNV1A_PRIME       16777619
#define HASH_FNV1A_OFFSET_BASE 2166136261

// Hashes a string
LIBNEX_PUBLIC hash_t HashCreateHash (const char* str)
{
    uint8_t* buf = (uint8_t*) str;
    // Setup hash
    hash_t hash = HASH_FNV1A_OFFSET_BASE;
    // Compute it
    while (*buf)
    {
        hash ^= (hash_t) *buf++;
        hash *= HASH_FNV1A_PRIME;
    }
    return hash;
}
