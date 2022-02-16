/*
    list.c - contains linked list functions
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

/// @file list.c

#include <libnex/list.h>
#include <libnex/lock.h>
#include <libnex/safemalloc.h>
#include <stdlib.h>

/**
 * @brief Creates a new linked list
 * @return The allocated list entry
 */
PUBLIC ListHead_t* ListCreate (char* type)
{
    ListHead_t* head = malloc_s (sizeof (ListHead_t));
    // Initialize the object associated with this list
    ObjCreate (type, &head->obj);
    // Initialize the other stuff
    head->front = NULL;
    head->back = NULL;
    return head;
}

/**
 * @brief Adds an item to the front of the list
 *
 * This function add an item before the front member of the ListHead.
 * It accepts a data pointer to initializes the member with, and a key
 * to identify the item
 * @param[in] list the list head to add to
 * @param[in] data a pointer to some data
 * @param[in] key the key for the data so it can identified
 * @return The list entry that was allocated
 */
PUBLIC ListEntry_t* ListAddFront (ListHead_t* head, void* data, int key)
{
    ListEntry_t* entry = malloc_s (sizeof (ListEntry_t));
    ListLock (head);
    // Set all the links
    if (head->front)
        head->front->prev = entry;
    entry->next = head->front;
    entry->prev = NULL;
    head->front = entry;
    // If this is the first entry
    if (!head->back)
        head->back = head->front;
    // Set everything else
    entry->data = data;
    entry->key = key;
    ObjCreate (ObjGetType (head), &entry->obj);
    ListUnlock (head);
    return entry;
}

/**
 * @brief Adds an item to the back of the list
 *
 * This function does the same ListAddFront(), just the other way
 * @param[in] list the list head to add to
 * @param[in] data a pointer to some data
 * @param[in] key the key for the data so it can identified
 * @return The list entry that was allocated
 */
PUBLIC ListEntry_t* ListAddBack (ListHead_t* head, void* data, int key)
{
    ListEntry_t* entry = malloc_s (sizeof (ListEntry_t));
    ObjCreate (ObjGetType (head), &entry->obj);
    ListLock (head);
    // Set all the links
    entry->prev = head->back;
    if (head->back)
        head->back->next = entry;
    entry->next = NULL;
    head->back = entry;
    // If this is the first entry
    if (!head->front)
        head->front = head->back;
    // Set everything else
    entry->data = data;
    entry->key = key;
    ListUnlock (head);
    return entry;
}

/**
 * @brief Finds an item in a list
 *
 * Accepts a key value, and finds the item associated with that key
 *
 * @param[in] list the list to search in
 * @param[in] key the key to search for
 * @return The list entry associated with the key
 */
PUBLIC ListEntry_t* ListFind (ListHead_t* list, int key)
{
    ListLock (list);
    ListEntry_t* search = list->front;
    while (search)
    {
        ListLock (search);
        if (search->key == key)
        {
            ListUnlock (search);
            ListUnlock (list);
            return search;
        }
        ListEntry_t* old = search;
        search = search->next;
        ListUnlock (old);
    }
    ListUnlock (list);
    return NULL;
}

/**
 * @brief Adds an item to a list somewhere in the middle
 *
 * This functions accepts the pointer to the item before what we want to add
 * The other form accepts a key value
 * @param[in] list the list head to add to
 * @param[in] data the data to initialize it with
 * @param[in] key the key of the new item
 * @param[in] entry the entry before the new item
 * @return The new ListEntry_t*
 */
PUBLIC ListEntry_t* ListAddBefore (ListHead_t* list, void* data, int key, ListEntry_t* entryAfter)
{
    ListEntry_t* entry = (ListEntry_t*) malloc_s (sizeof (ListEntry_t));
    ListLock (list);
    ListRef (entryAfter);
    ListLock (entryAfter);
    ObjCreate (ObjGetType (list), &entry->obj);
    entry->key = key;
    entry->data = data;
    entry->next = entryAfter;
    entry->prev = entryAfter->prev;
    if (entryAfter->prev)
        entryAfter->prev->next = entry;
    entryAfter->prev = entry;
    if (list->front == entryAfter)
        list->front = entry;
    ListUnlock (entryAfter);
    ListDeRef (entryAfter);
    ListUnlock (list);
    return entry;
}

/**
 * @brief Adds an item to a list somewhere in the middle
 *
 * This functions accepts the key of the item before what we want to add
 * The other form accepts a listEntry_t pointer
 * @param[in] list the list head to add to
 * @param[in] data the data to initialize it with
 * @param[in] key the key of the new item
 * @param[in] keyBefore the key before the new item
 * @return The new ListEntry_t*. NULL if entry specified by keyBefore doesn't
 * exist
 */
PUBLIC ListEntry_t* ListAddBeforeKey (ListHead_t* list, void* data, int key, int keyAfter)
{
    ListEntry_t* entryAfter = ListFind (list, keyAfter);
    if (!entryAfter)
        return NULL;
    return ListAddBefore (list, data, key, entryAfter);
}

/**
 * @brief Adds an item to a list somewhere in the middle
 *
 * This functions accepts the pointer to the item before what we want to add
 * The other form accepts a key value
 * @param[in] list the list head to add to
 * @param[in] data the data to initialize it with
 * @param[in] key the key of the new item
 * @param[in] entryBefore the entry before the new item
 * @return The new ListEntry_t*
 */
PUBLIC ListEntry_t* ListAddAfter (ListHead_t* list, void* data, int key, ListEntry_t* entryBefore)
{
    ListEntry_t* entry = (ListEntry_t*) malloc_s (sizeof (ListEntry_t));
    ListLock (list);
    ListRef (entryBefore);
    ListLock (entryBefore);
    ObjCreate (ObjGetType (list), &entry->obj);
    entry->key = key;
    entry->data = data;
    entry->prev = entryBefore;
    entry->next = entryBefore->next;
    if (entryBefore->next)
        entryBefore->next->prev = entry;
    entryBefore->next = entry;
    if (list->back == entryBefore)
        list->back = entry;
    ListUnlock (entryBefore);
    ListDeRef (entryBefore);
    ListUnlock (list);
    return entry;
}

/**
 * @brief Adds an item to a list somewhere in the middle
 *
 * This functions accepts the key of the item before what we want to add
 * The other form accepts a ListEntry_t pointer
 * @param[in] list the list head to add to
 * @param[in] data the data to initialize it with
 * @param[in] key the key of the new item
 * @param[in] keyBefore the key before the new item
 * @return The new ListEntry_t*. NULL if entry specified by keyBefore doesn't
 * exist
 */
PUBLIC ListEntry_t* ListAddAfterKey (ListHead_t* list, void* data, int key, int keyBefore)
{
    ListEntry_t* entryBefore = ListFind (list, keyBefore);
    if (!entryBefore)
        return NULL;
    return ListAddAfter (list, data, key, entryBefore);
}

/**
 * @brief Removes the head of list, returning the item
 *
 * This function could be useful when treating a linked list as a queue
 * @param list the list head to remove the entry from
 * @return The ListEntry_t*
 */
PUBLIC ListEntry_t* ListPopFront (ListHead_t* list)
{
    ListLock (list);
    ListEntry_t* entry = ListRef (list->front);
    ListLock (entry);
    if (entry->next)
        entry->next->prev = NULL;
    list->front = entry->next;
    ListUnlock (entry);
    ListDeRef (entry);
    ListUnlock (list);
    return entry;
}

/**
 * @brief Removes entry by key
 *
 * This function accepts a key, and returns the item identified by that key
 * @param list the list to remove it from
 * @param key the key to remove
 * @return The removed entry
 */
PUBLIC ListEntry_t* ListRemoveKey (ListHead_t* list, int key)
{
    ListEntry_t* entry = ListFind (list, key);
    if (!entry)
        return NULL;
    return ListRemove (list, entry);
}

// Internal function to remove a list entry
ListEntry_t* _listRemove (ListHead_t* list, ListEntry_t* entry, int doRef)
{
    ListLock (list);
    if (doRef)
        ListRef (entry);
    ListLock (entry);
    if (entry->prev)
        entry->prev->next = entry->next;
    if (entry->next)
        entry->next->prev = entry->prev;
    if (list->front == entry)
        list->front = entry->next;
    if (list->back == entry)
        list->back = entry->prev;
    ListUnlock (entry);
    if (doRef)
        ListDeRef (entry);
    ListUnlock (list);
    return entry;
}

/**
 * @brief Removes an entry
 *
 * This function accepts a key, and returns the item identified by that key.
 * Note that if other consumers are referencing this entry still, it is not removed
 * @param list the list to remove from
 * @param entry the entry to remove
 * @return The removed entry
 */
PUBLIC ListEntry_t* ListRemove (ListHead_t* list, ListEntry_t* entry)
{
    if (!ListDeRef (entry))
        return _listRemove (list, entry, 0);
    else
        return entry;
}

/**
 * @brief Destroys an entry
 *
 * Destroys a list entry, returning the data associated with it.
 * Note that if other consumers are referencing this entry still, it is not destroyed
 * @param list the list to destroy from
 * @param entry the entry to destroy
 * @return The data associated with this entry
 */
PUBLIC void* ListDestroyEntry (ListHead_t* list, ListEntry_t* entry)
{
    void* data = entry->data;
    // De-reference the entry and remove it
    if (!ListDeRef (entry))
        _listRemove (list, entry, 0);
    // Destroy it
    free (entry);
    return data;
}

/**
 * @brief Destroys an entry and its data
 *
 * Destroys a list entry and the data associated with it
 * Note that if other consumers are referencing this entry still, it is not destroyed
 * @param list the list to destroy from
 * @param entry the entry to destroy
 * @return The data associated with this entry
 */
PUBLIC void ListDestroyEntryAll (ListHead_t* list, ListEntry_t* entry)
{
    // De-reference the entry and remove it
    if (!ListDeRef (entry))
        _listRemove (list, entry, 0);
    // Destroy it
    free (entry->data);
    free (entry);
}

/**
 * @brief Destroys a list
 * Note that if other consumers are referencing this list still, it is not destroyed
 * @param list the list to destroy
 */
PUBLIC void ListDestroy (ListHead_t* list)
{
    ListLock (list);
    if (!ListDeRef (list))
    {
        // Go through every entry, destroying it
        ListEntry_t* curEntry = list->front;
        while (curEntry)
        {
            ListEntry_t* next = curEntry->next;
            // We want to free the memory as well
            ListDestroyEntryAll (list, curEntry);
            curEntry = next;
        }
    }
    ListUnlock (list);
}
