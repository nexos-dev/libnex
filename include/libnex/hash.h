/*
    hash.h - contains hash table interface
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

#ifndef _HASH_H
#define _HASH_H

#include <libnex/decls.h>
#include <libnex/libnex_config.h>
#include <libnex/list.h>
#include <stddef.h>
#include <stdint.h>

__DECL_START

// Hash table data
typedef struct _hashbuf
{
    int type;     // Type of data
    size_t sz;    // Size of data
    union
    {
        void* data;    // Data
        uint8_t buf[0];
    };
} HashBuf_t;

/// Callback type that destroys a hash entry
typedef void (*HashEntryDestroy) (HashBuf_t* buf);

typedef unsigned long hash_t;

// Hash table structure
typedef struct _hashtable
{
    Object_t obj;                    // For reference counting
    HashEntryDestroy destroyFunc;    // Destroy function
    size_t numElems;                 // Number of elements we can use
    size_t usedElems;                // Number of used elements
    size_t maxElems;                 // Max number of elements
    size_t elemSize;                 // Size of a single element
    size_t dataSize;                 // Size of a single data buffer
    size_t arraySize;                // Current size of array
    int flags;
    void* array;    // Data array
} HashTable_t;

#define HASH_FLAG_ENTRY_IS_DATA \
    (1 << 0)    /// Used if we actually want to set the entry data, and
                /// not just return a pointer to it on allocations

#define HASH_TYPE_CSTRING 0
#define HASH_TYPE_PTR     1
#define HASH_TYPE_INT     2
#define HASH_TYPE_BUF     3

/**
 * @brief Produces an FNV-1a hash for a string
 * @param buf bstring to compute hash of
 * @return The FNV-1a hash
 */
LIBNEX_PUBLIC hash_t HashCreateHash (const char* str);

/**
 * @brief Creates a hash table
 * @param elemSize size of each element
 * @param maxElems max number elements allowed. 0 for infinite sized hash table
 * @param destroyFunc function to destroy a hash table entry
 * @return the hash table
 */
LIBNEX_PUBLIC HashTable_t* HashCreateTable (size_t elemSize,
                                            size_t maxElems,
                                            HashEntryDestroy destroyFunc,
                                            int flags);

/**
 * @brief Destroys hash table
 * @param table hash table
 */
LIBNEX_PUBLIC void HashDestroyTable (HashTable_t* table);

/**
 * @brief Inserts entry into hash table, potentially expanding it
 * @param table table to operate on
 * @param key key used to hash data
 * @param data data to hash
 * @param flags flags used for insertion
 * @return wheter it was successful
 */
LIBNEX_PUBLIC bool HashInsertEntry (HashTable_t* table, const char* key, HashBuf_t* data, int flags);

#define HASH_FLAG_NO_EXPAND (1 << 0)    /// Don't expand it automatically

/**
 * @brief Gets free entry into hash table and allows us to manipulate data directly
 * @param table table to operate on
 * @param key key to get
 * @param flags flags used for insertion
 * @return free entry
 */
LIBNEX_PUBLIC void* HashGetEntry (HashTable_t* table, const char* key, int flags);

/**
 * @brief Inserts entry by index instead of key
 * @param table table to work on
 * @param idx index into table
 * @param data data to hash
 * @param flags flags used for insertion
 * @return sucess status
 */
LIBNEX_PUBLIC bool HashInsertEntryIdx (HashTable_t* table, hash_t idx, HashBuf_t* data, int flags);

/**
 * @brief Gets free entry into hash table by index
 * @param table table to operate on
 * @param idx index into table
 * @param flags flags used for insertion
 * @return free entry
 */
LIBNEX_PUBLIC void* HashGetEntryIdx (HashTable_t* table, hash_t idx, int flags);

/**
 * @brief Remove and entry in table
 * @param table entry to remove
 * @param key key to remove
 */
LIBNEX_PUBLIC void HashRemoveEntry (HashTable_t* table, const char* key);

/**
 * @brief Remove entry by index
 * @param table table to work on
 * @param idx index into table
 */
LIBNEX_PUBLIC void HashRemoveEntryIdx (HashTable_t* table, hash_t idx);

/**
 * @brief Finds an entry by key
 * @param table table to find in
 * @param key key to look for
 * @return the data. Cast it to what you need it to be
 */
LIBNEX_PUBLIC void* HashFindEntry (HashTable_t* table, const char* key);

/**
 * @brief Gets an entry by index
 * @param table table to get from
 * @param idx idx to retrieve
 * @return the data
 */
LIBNEX_PUBLIC void* HashFindEntryIdx (HashTable_t* table, hash_t idx);

/**
 * @brief Expand the table. Will only expand if load factor is sufficient
 * @param table table to expand
 * @return Wheter allocation suceeded or not
 */
LIBNEX_PUBLIC bool HashTableExpand (HashTable_t* table);

// Hash table iterator
typedef struct _htieter
{
    HashTable_t* table;    // table we are iterating through
    hash_t idx;            // Current interator index
} HashIter_t;

/**
 * @brief Initializes a hash table iterator
 * @param table table to iterate on
 * @param iter iterator to use
 */
LIBNEX_PUBLIC void HashStartIterate (HashTable_t* table, HashIter_t* iter);

/**
 * @brief Iterates to next hash table entry
 * @param iter iterator to use
 * @return iterator or NULL if we reached end
 */
LIBNEX_PUBLIC HashIter_t* HashIterate (HashIter_t* iter);

// Helper macros
#define HashRef(item)    (ObjRef (&(item)->obj))       ///< References the underlying object
#define HashDeRef(item)  (ObjDeRef (&(item)->obj))     ///< Dereferences the hash table
#define HashLock(item)   (ObjLock (&(item)->obj))      ///< Locks this hash table
#define HashUnlock(item) (ObjUnlock (&(item)->obj))    ///< Unlocks this hash table

__DECL_END

#endif
