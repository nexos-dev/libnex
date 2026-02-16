/*
    hash.c - contains hash table implementation
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

#include <assert.h>
#include <libnex/endian.h>
#include <libnex/hash.h>
#include <libnex/object.h>
#include <libnex/safemalloc.h>
#include <stdlib.h>
#include <string.h>

// Hash table data structure
typedef struct _hasht
{
    Object_t obj;              // Object for reference counting
    size_t elemSize;           // Size of each element in the hash table
    size_t elemUnalignedSz;    // Size used for copies
    size_t numBuckets;         // Current number of buckets in the hash table
    size_t usedElems;          // Number of elements currently in the hash table
                               // Load factor is usedElems / numBuckets, we'll resize when it reaches
                               // 0.7
    int flags;                 // Flags for the hash table
    HashEntryDestroy destroyFunc;    // Function to call when an entry is removed from the hash
                                     // table
    HashMatchKey matchKey;           // Function to match keys
    HashMakeHash makeHash;           // Function to make keys
    void* entries;                   // Open addressed array of entries, each entry is elemSize bytes
} HashTable_t;

// Hash table entry structure
typedef struct _hashe
{
    hash_t hash;    // Hash of this entry
    const char* key;
    int removed;    // Wheter this entry has been removed. An int for alignment's sake
} HashEntry_t;

static bool hashMatchHash (const char* key1, const char* key2)
{
    return !strcmp (key1, key2);
}

// Gets a pointer to a  element in the table
#define HASH_GET_PTR(table, idx) \
    ((HashEntry_t*) ((table)->entries + ((idx) * (sizeof (HashEntry_t) + table->elemSize))))
// Gets a pointer to where data goes in key-value elements
#define HASH_GET_VALUE_PTR(entry) ((const void**) ((void*) entry + sizeof (HashEntry_t)))

LIBNEX_PUBLIC HashTable_t* HashCreateTable (size_t elemSize,
                                            size_t numBuckets,
                                            HashEntryDestroy destroyFunc,
                                            int flags)
{
    if (elemSize == 0 || numBuckets == 0)
    {
        LibnexSetError (LIBNEX_ERR_BAD_PARAM);
        return NULL;
    }
    // Allocate the hash table
    HashTable_t* table = (HashTable_t*) malloc_s (sizeof (HashTable_t));
    if (!table)
        return NULL;
    ObjCreate ("HashTable", &table->obj);
    table->elemUnalignedSz = elemSize;
    // Round up elemSize to power of 2, for better cache performance as long as it is smaller
    // than a cache line (64 bytes Otherwise round up to 64 bytes to ensure good cache
    // performance
    if (elemSize < sizeof (void*))
        elemSize = sizeof (void*);
    else if (elemSize < 64)
    {
        size_t rounded = 1;
        while (rounded < elemSize)
            rounded <<= 1;
        elemSize = rounded;
    }
    else
        elemSize = AlignNumberUp (elemSize, 64);
    // Set up fields
    table->elemSize = elemSize;
    table->numBuckets = numBuckets;
    table->usedElems = 0;
    table->destroyFunc = destroyFunc;
    table->flags = flags;
    table->entries = calloc_s (numBuckets * (sizeof (HashEntry_t) + elemSize));
    table->makeHash = HashCreateHash;
    table->matchKey = hashMatchHash;
    if (!table->entries)
    {
        free (table);
        return NULL;
    }
    return table;
}

// Destroy a hash table
LIBNEX_PUBLIC void HashDestroyTable (HashTable_t* table)
{
    if (!table)
        return;
    if (!ObjDeRef (&table->obj))
    {
        // Go through all entries and call destroy callback on used entries
        if (table->destroyFunc)
        {
            for (int i = 0; i < table->numBuckets; ++i)
            {
                HashEntry_t* entry =
                    (HashEntry_t*) (table->entries + (i * (sizeof (HashEntry_t) + table->elemSize)));
                if (entry->hash && !entry->removed)
                {
                    void* elem = (void*) entry + sizeof (HashEntry_t);
                    table->destroyFunc (elem);
                }
            }
        }
        free (table->entries);
        free (table);
    }
}

LIBNEX_PUBLIC void HashSetFuncs (HashTable_t* table, HashMatchKey matchKey, HashMakeHash makeHash)
{
    if (!table || !matchKey || !makeHash)
        return;
    table->makeHash = makeHash;
    table->matchKey = matchKey;
}

// Sets up a hash table entry
static inline void hashSetEntry (HashTable_t* table, HashEntry_t* entry, const void* buf)
{
    // Figure out how to add entry
    if (table->flags & HASH_FLAG_BUF)
    {
        // Copy it out
        memcpy ((void*) entry + sizeof (HashEntry_t), buf, table->elemUnalignedSz);
    }
    else
    {
        assert (table->elemSize == sizeof (void*));
        const void** loc = HASH_GET_VALUE_PTR (entry);
        *loc = buf;
    }
}

// Checks if a resize is needed
static inline bool hashCheckResize (HashTable_t* table)
{
    // Check if a resize is needed
    if (((float) (table->usedElems + 1) / (float) table->numBuckets) >= 0.7)
    {
        // Figure out number of buckets we need (we double it)
        size_t numBuckets = table->numBuckets * 2;
        // Allocvate new array
        void* newEntries = calloc_s (numBuckets * (sizeof (HashEntry_t) + table->elemSize));
        if (!newEntries)
        {
            LibnexSetError (LIBNEX_ERR_OOM);
            return false;
        }
        // Rehash onto new array also removing removed entries
        for (int i = 0; i < table->numBuckets; ++i)
        {
            HashEntry_t* entry = HASH_GET_PTR (table, i);
            if (entry->hash && !entry->removed)
            {
                // Compute index of new element
                size_t newIdx = entry->hash % numBuckets;
                // Add to array
                while (1)
                {
                    HashEntry_t* newEntry =
                        (HashEntry_t*) (newEntries +
                                        (newIdx * (sizeof (HashEntry_t) + table->elemSize)));
                    // Check if hash is free
                    if (!newEntry->hash)
                    {
                        // Add to it
                        if (table->flags & HASH_FLAG_BUF)
                            hashSetEntry (table, newEntry, (void*) entry + sizeof (HashEntry_t));
                        else
                        {
                            const void** loc = HASH_GET_VALUE_PTR (entry);
                            hashSetEntry (table, newEntry, *loc);
                        }
                        newEntry->hash = entry->hash;
                        newEntry->key = entry->key;
                        break;
                    }
                    newIdx = (newIdx + 1) % numBuckets;
                }
            }
        }
        // Set new fields
        free (table->entries);
        table->entries = newEntries;
        table->numBuckets = numBuckets;
    }
    return true;
}

LIBNEX_PUBLIC bool HashInsertEntry (HashTable_t* table, const char* key, const void* value)
{
    if (!table || !key || !value)
    {
        LibnexSetError (LIBNEX_ERR_BAD_PARAM);
        return false;
    }
    if (!hashCheckResize (table))
        return false;
    // Get entry
    hash_t hash = table->makeHash (key);
    size_t idx = hash % table->numBuckets;
    size_t startIdx = idx;
    HashEntry_t* entry = HASH_GET_PTR (table, idx);
    while (entry->hash)
    {
        if (entry->hash == hash && table->matchKey (key, entry->key))
            return false;    // Entry already exists
        // Keep advancing until we find a free bucket
        idx = (idx + 1) % table->numBuckets;
        if (idx == startIdx)
            return false;
        entry = HASH_GET_PTR (table, idx);
    }
    assert (entry && !entry->hash);
    // Setup new entry
    entry->removed = false;
    entry->key = key;
    entry->hash = hash;
    hashSetEntry (table, entry, value);
    // Bookkeeping
    table->usedElems++;
    return true;
}

// Find a hash entry
static inline HashEntry_t* hashFindEntry (HashTable_t* table, const char* key)
{
    hash_t hash = table->makeHash (key);
    // Get entry associated with hash
    size_t idx = hash % table->numBuckets;
    size_t startIdx = idx;
    while (1)
    {
        HashEntry_t* cur = HASH_GET_PTR (table, idx);
        // Check if it matches
        if (cur->hash == hash && !cur->removed)
        {
            // Now compare keys
            // We don't just compare keys for performance reasons
            if (table->matchKey (key, cur->key))
                return cur;
        }
        idx = (idx + 1) % table->numBuckets;
        if (idx == startIdx)
            return NULL;
    }
    return NULL;
}

LIBNEX_PUBLIC void HashRemoveEntry (HashTable_t* table, const char* key)
{
    if (!table || !key)
        return;
    HashEntry_t* entry = hashFindEntry (table, key);
    if (!entry)
        return;
    entry->removed = true;
    --table->usedElems;
}
LIBNEX_PUBLIC bool HashGetEntryBuf (HashTable_t* table, const char* key, void* buf)
{
    if (!table || !key || !buf)
        return false;
    HashEntry_t* entry = hashFindEntry (table, key);
    if (!entry)
        return NULL;
    // Copy it
    memcpy (buf, entry + 1, table->elemUnalignedSz);
    return true;
}

LIBNEX_PUBLIC bool HashCheckEntry (HashTable_t* table, const char* key)
{
    if (!table || !key)
        return false;
    return !!hashFindEntry (table, key);
}

LIBNEX_PUBLIC void* HashFindEntry (HashTable_t* table, const char* key)
{
    if (!table || !key)
        return NULL;
    HashEntry_t* entry = hashFindEntry (table, key);
    if (!entry)
        return NULL;
    void** ptr = (void**) HASH_GET_VALUE_PTR (entry);
    return *ptr;
}

static inline void hashSetupIter (HashTable_t* table, HashIter_t* iter, HashEntry_t* entry)
{
    iter->entry = entry;
    // Setup key/value
    iter->key = entry->key;
    if (table->flags & HASH_FLAG_BUF)
        iter->value = (void*) entry + sizeof (HashEntry_t);
    else
        iter->value = *((void**) HASH_GET_VALUE_PTR (entry));
}

LIBNEX_PUBLIC void HashStartIter (HashTable_t* table, HashIter_t* iter, const char* key)
{
    if (!table || !iter)
        return;
    // Initialize iter
    iter->table = table;
    iter->entry = (HashEntry_t*) table->entries;
    iter->idx = 0;
    if (key)
    {
        HashEntry_t* entry = hashFindEntry (table, key);
        if (entry)
        {
            iter->idx = ((void*) entry - table->entries) / table->elemSize;
            hashSetupIter (table, iter, entry);
        }
    }
    else
    {
        // Go ahead and iterate to first entry if it isn't valid
        if (!iter->entry->hash)
            HashIterate (iter);
        else
            hashSetupIter (table, iter, iter->entry);
    }
}

LIBNEX_PUBLIC HashIter_t* HashIterate (HashIter_t* iter)
{
    if (!iter)
        return NULL;
    HashTable_t* table = iter->table;
    HashEntry_t* next = iter->entry;
    // Find next valid entry
    while (1)
    {
        ++iter->idx;
        // Check if we've reached end
        if (iter->idx >= table->numBuckets)
            break;
        next = (HashEntry_t*) ((void*) next + table->elemUnalignedSz + sizeof (HashEntry_t));
        // Check if we found a valid entry
        if (next->hash && !next->removed)
        {
            // Return this
            hashSetupIter (table, iter, next);
            return iter;
        }
    }
    return NULL;
}

// Hash function parameters
#define HASH_FNV1A_PRIME       16777619
#define HASH_FNV1A_OFFSET_BASE 2166136261

// Hashes a string
LIBNEX_PUBLIC hash_t HashCreateHash (const char* str)
{
    if (!str)
        return 0;
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
