/*
    hash.h - contains hash table interface
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

#ifndef _HASH_H
#define _HASH_H

#include <libnex/decls.h>
#include <libnex/libnex_config.h>
#include <libnex/list.h>
#include <stddef.h>
#include <stdint.h>

__DECL_START

typedef unsigned long hash_t;

typedef struct _hasht HashTable_t;
typedef struct _hashe HashEntry_t;

/// Callback type that destroys a hash entry
typedef void (*HashEntryDestroy) (void* buf);
/// Callback for check if keys are equals
typedef bool (*HashMatchKey) (const char* key1, const char* key2);
/// Callback for hashing
typedef hash_t (*HashMakeHash) (const char* key);

/**
 * @brief Creates a hash table
 * @param elemSize Size of each element in the hash table
 * @param numBuckets Number of buckets to use in the hash table to start out with
 * @param destroyFunc Function to call when an entry is removed from the hash table
 * @param flags Flags for the hash table.
 * @return Pointer to created hash table, or NULL on failure
 */
LIBNEX_PUBLIC HashTable_t* HashCreateTable (size_t elemSize,
                                            size_t numBuckets,
                                            HashEntryDestroy destroyFunc,
                                            int flags);

#define HASH_FLAG_STATIC (1 << 0)    ///< Indicates that size of hash table is invariant
#define HASH_FLAG_BUF \
    (1 << 1)    ///< Indicates that the entries contain buffers and there we must copy there data into
                ///< them

#define HASH_ELEM_VAL \
    8    // passed to elemSize to indicate that table contains values and not buffers

/**
 * @brief Destroys a hash table
 * @param table Hash table to destroy
 */
LIBNEX_PUBLIC void HashDestroyTable (HashTable_t* table);

/**
 * @brief Sets hashing functions
 * @param table Table to set on
 * @param hashFunc Function to hash a key
 * @param matchFunc Function to match a key
 */
LIBNEX_PUBLIC void HashSetFuncs (HashTable_t* table, HashMatchKey matchKey, HashMakeHash makeKey);

/**
 * @brief Inserts an element into a hash table by the key
 * @param table Hash table to insert into
 * @param key key of element to instert
 * @param value value to insert at key.
 * If HASH_FLAG_BUF is set, this should be a pointer to a buffer containing the data to insert
 * @return true on success, false on failure
 */
LIBNEX_PUBLIC bool HashInsertEntry (HashTable_t* table, const char* key, const void* value);

/**
 * @brief Removes an element with specified key from hash table
 * @param table Hash table to remove from
 * @param key Key of element to remove
 */
LIBNEX_PUBLIC void HashRemoveEntry (HashTable_t* table, const char* key);

/**
 * @brief Finds an element in the table with specified key
 * @param table Table to look in
 * @param key Key to find
 * @return Data associated with key, NULL if key is non-existant
 */
LIBNEX_PUBLIC void* HashFindEntry (HashTable_t* table, const char* key);

/**
 * @brief Gets element with hash but also copies its data into provided buffer
 * Only for hash tables with HASH_FLAG_BUF set
 * @param table Table to get from
 * @param key Key of value to obtain
 * @param buf Buffer to copy data into. Must be at least elemSize bytes long
 * @return false if hash isn't found, true otherwise
 */
LIBNEX_PUBLIC bool HashGetEntryBuf (HashTable_t* table, const char* key, void* buf);

/**
 * @brief Checks if key exists
 * @param table Table to check in
 * @param key Key to check
 * @return true if found, false otherwise
 */
LIBNEX_PUBLIC bool HashCheckEntry (HashTable_t* table, const char* key);

/// Iterator structure
typedef struct _hashi
{
    HashTable_t* table;
    HashEntry_t* entry;    // Current entry we are at
    size_t idx;            // Current index we are at
    const char* key;
    void* value;
} HashIter_t;
/**
 * @brief Initializes an iterator for a hash table
 * @param table Table to iterate over
 * @param iter Iterator to initialize
 * @param key Optional key to start iteration from. If NULL, iteration starts from the beginning
 */
LIBNEX_PUBLIC void HashStartIter (HashTable_t* table, HashIter_t* iter, const char* key);

/**
 * @brief Iterates through table
 * @param iter Iterator to iterate through
 * @return Same iterator, just with updated data. NULL if we reached end
 */
LIBNEX_PUBLIC HashIter_t* HashIterate (HashIter_t* iter);

/**
 * @brief Produces an FNV-1a hash for a string
 * @param buf bstring to compute hash of
 * @return The FNV-1a hash
 */
LIBNEX_PUBLIC hash_t HashCreateHash (const char* str);

// Helper macros
#define HashRef(item)    (ObjRef (&(item)->obj))       ///< References the underlying object
#define HashDeRef(item)  (ObjDeRef (&(item)->obj))     ///< Dereferences the hash table
#define HashLock(item)   (ObjLock (&(item)->obj))      ///< Locks this hash table
#define HashUnlock(item) (ObjUnlock (&(item)->obj))    ///< Unlocks this hash table

__DECL_END

#endif
