/*
    list.h - contains linked list functions
    Copyright 2022 The NexNix Project

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

/// @file list.h

#ifndef _LIST_H
#define _LIST_H

#include <libnex/decls.h>
#include <libnex/object.h>

#include <stddef.h>

__DECL_START

/**
 * @brief Describes an entry in a linked list
 *
 * It contains two links to make a doubly linked list, for fast insertion and
 * removal It also contains a pointer to some arbitrary piece of data
 */
typedef struct _ListEntry
{
    Object_t obj;               ///< The underlying object
    void* data;                 ///< The underlying data in thist list entry
    int key;                    ///< Used to uniquely identify this entry
    struct _ListEntry* next;    ///< Pointer to next entry in list. NULL means end
    struct _ListEntry* prev;    ///< Pointer to previous entry in linked list. NULL means beginning
} ListEntry_t;

/**
 * @brief Describes the head of a list
 *
 * It contains the front and back of the list
 */
typedef struct _ListHead
{
    Object_t obj;                ///< The underlying object
    struct _ListEntry* front;    ///< The first item on the list
    struct _ListEntry* back;     ///< The last item on the list
} ListHead_t;

/**
 * @brief Creates a new linked list
 * @param type the data type of this entry. Used in the underlying object
 * @return The allocated list entry
 */
PUBLIC ListHead_t* ListCreate (const char* type);

/**
 * @brief Adds an item to the front of the list
 *
 * This function add an item before the front member of the listHead.
 * It accepts a data pointer to initializes the member with, and a key
 * to identify the item
 * @param[in] list the list head to add to
 * @param[in] data a pointer to some data
 * @param[in] key the key for the data so it can identified
 * @return The list entry that was allocated
 */
PUBLIC ListEntry_t* ListAddFront (ListHead_t* head, void* data, const int key);

/**
 * @brief Adds an item to the back of the list
 *
 * This function does the same listAddFront(), just the other way
 * @param[in] list the list head to add to
 * @param[in] data a pointer to some data
 * @param[in] key the key for the data so it can identified
 * @return The list entry that was allocated
 */
PUBLIC ListEntry_t* ListAddBack (ListHead_t* head, void* data, const int key);

/**
 * @brief Finds an item in a list
 *
 * Accepts a key value, and finds the item associated with that key
 *
 * @param[in] list the list to search in
 * @param[in] key the key to search for
 * @return The list entry associated with the key
 */
PUBLIC ListEntry_t* ListFind (ListHead_t* list, const int key);

/**
 * @brief Adds an item to a list somewhere in the middle
 *
 * This function accepts the pointer to the item before what we want to add
 * The other form accepts a key value
 * @param[in] list the list head to add to
 * @param[in] data the data to initialize it with
 * @param[in] key the key of the new item
 * @param[in] entry the entry before the new item
 * @return The new ListEntry_t*
 */
PUBLIC ListEntry_t* ListAddBefore (ListHead_t* list, void* data, const int key, ListEntry_t* entryAfter);

/**
 * @brief Adds an item to a list somewhere in the middle
 *
 * This function accepts the key of the item before what we want to add
 * The other form accepts a listEntry_t pointer
 * @param[in] list the list head to add to
 * @param[in] data the data to initialize it with
 * @param[in] key the key of the new item
 * @param[in] keyAfter the key after the new item
 * @return The new ListEntry_t*. NULL if entry specified by keyAfter doesn't
 * exist
 */
PUBLIC ListEntry_t* ListAddBeforeKey (ListHead_t* list, void* data, const int key, const int keyAfter);

/**
 * @brief Adds an item to a list somewhere in the middle
 *
 * This function accepts the pointer to the item before what we want to add
 * The other form accepts a key value
 * @param[in] list the list head to add to
 * @param[in] data the data to initialize it with
 * @param[in] key the key of the new item
 * @param[in] entryBefore the entry before the new item
 * @return The new ListEntry_t*
 */
PUBLIC ListEntry_t* ListAddAfter (ListHead_t* list, void* data, const int key, ListEntry_t* entryBefore);

/**
 * @brief Adds an item to a list somewhere in the middle
 *
 * This function accepts the key of the item before what we want to add
 * The other form accepts a listEntry_t pointer
 * @param[in] list the list head to add to
 * @param[in] data the data to initialize it with
 * @param[in] key the key of the new item
 * @param[in] keyBefore the key before the new item
 * @return The new ListEntry_t*. NULL if entry specified by keyBefore doesn't
 * exist
 */
PUBLIC ListEntry_t* ListAddAfterKey (ListHead_t* list, void* data, const int key, int keyBefore);

/**
 * @brief Removes the head of list, returning the item
 *
 * This function could be useful when treating a linked list as a queue
 * @param list the list head to remove the entry from
 * @return The ListEntry_t*
 */
PUBLIC ListEntry_t* ListPopFront (ListHead_t* list);

/**
 * @brief Removes entry by key
 *
 * This function accepts a key, and returns the item identified by that key
 * @param list the list to remove it from
 * @param key the key to remove
 * @return The removed entry
 */
PUBLIC ListEntry_t* ListRemoveKey (ListHead_t* list, const int key);

/**
 * @brief Removes an entry
 *
 * This function accepts a key, and returns the item identified by that key.
 * Note that if other consumers are referencing this entry still, it is not removed
 * @param list the list to remove from
 * @param entry the entry to remove
 * @return The removed entry
 */
PUBLIC ListEntry_t* ListRemove (ListHead_t* list, ListEntry_t* entry);

/**
 * @brief Destroys an entry
 *
 * Destroys a list entry, returning the data associated with it.
 * Note that if other consumers are referencing this entry still, it is not destroyed
 * @param list the list to destroy from
 * @param entry the entry to destroy
 * @return The data associated with this entry
 */
PUBLIC void* ListDestroyEntry (ListHead_t* list, ListEntry_t* entry);

/**
 * @brief Destroys an entry and its data
 *
 * Destroys a list entry and the data associated with it
 * Note that if other consumers are referencing this entry still, it is not destroyed
 * @param list the list to destroy from
 * @param entry the entry to destroy
 * @return The data associated with this entry
 */
PUBLIC void ListDestroyEntryAll (ListHead_t* list, ListEntry_t* entry);

/**
 * @brief Destroys a list
 * Note that if other consumers are referencing this list still, it is not destroyed
 * @param list the list to destroy from
 * @param entry the entry to destroy
 * @return The data associated with this entry
 */
PUBLIC void ListDestroy (ListHead_t* list);

__DECL_END

// Some helper macros to work with list entries
#define ListEntryData(entry) ((entry)->data)                           ///< Helper to access list entry data
#define ListPushFront        ListAddFront                              ///< Useful when using as a queue
#define ListRef(item)        ((ListEntry_t*) ObjRef (&(item)->obj))    ///< References the underlying the object
#define ListLock(item)       (ObjLock (&(item)->obj))                  ///< Locks this entry (or list)
#define ListUnlock(item)     (ObjUnlock (&(item)->obj))                ///< Unlocks the entry
#define ListDeRef(item)      (ObjDestroy (&(item)->obj))               ///< Dereferences this entry

#endif
