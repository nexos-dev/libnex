/*
    list.c - contains linked list functions
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

#include <assert.h>
#include <libnex/list.h>
#include <libnex/lock.h>
#include <libnex/safemalloc.h>
#include <stdlib.h>

// Prepares a list entry
static inline ListEntry_t* listPrepareEntry (const void* data, int flags, int key)
{
    ListEntry_t* entry = (ListEntry_t*) data;
    entry->flags = flags & LIST_ENTRY_FLAG_MASK;
    entry->key = key;
    return entry;
}

LIBNEX_PUBLIC ListHead_t* ListCreate (const char* type,
                                      ListEntryFindBy findBy,
                                      ListEntryDestroy destroy,
                                      int flags)
{
    ListHead_t* list = calloc (1, sizeof (ListHead_t));
    if (!list)
    {
        LibnexSetError (LIBNEX_ERR_OOM);
        return NULL;
    }
    // Initialize the object associated with this list
    ObjCreate (type, &list->obj);
    // Initialize the other stuff
    list->findByFunc = findBy, list->destroyFunc = destroy;
    list->flags = flags;
    return list;
}

LIBNEX_PUBLIC ListEntry_t* ListAddFront (ListHead_t* list, const void* data, int key)
{
    assert (list && data);
    ListEntry_t* entry = listPrepareEntry (data, list->flags, key);
    ListLock (list);
    // Set all the links
    if (list->front)
        list->front->prev = entry;
    entry->next = list->front;
    entry->prev = NULL;
    list->front = entry;
    // If this is the first entry
    if (!list->back)
        list->back = list->front;
    // Set everything else
    entry->data = data;
    entry->key = key;
    ObjCreate (ObjGetType (list), &entry->obj);
    ListUnlock (list);
    return entry;
}

LIBNEX_PUBLIC ListEntry_t* ListAddBack (ListHead_t* list, const void* data, int key)
{
    assert (list && data);
    ListEntry_t* entry = listPrepareEntry (data, list->flags, key);
    ObjCreate (ObjGetType (list), &entry->obj);
    ListLock (list);
    // Set all the links
    entry->prev = list->back;
    if (list->back)
        list->back->next = entry;
    entry->next = NULL;
    list->back = entry;
    // If this is the first entry
    if (!list->front)
        list->front = list->back;
    // Set everything else
    entry->data = data;
    entry->key = key;
    ListUnlock (list);
    return entry;
}

LIBNEX_PUBLIC ListEntry_t* ListFind (const ListHead_t* list, const int key)
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

LIBNEX_PUBLIC ListEntry_t* ListFindEntryBy (const ListHead_t* list, const void* data)
{
    ListLock (list);
    ListEntry_t* search = list->front;
    while (search)
    {
        ListLock (search);
        if (list->findByFunc (search, data))
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

LIBNEX_PUBLIC ListEntry_t* ListAddBefore (ListHead_t* list, const void* data, int key, ListEntry_t* entryAfter)
{
    assert (list && data);
    ListEntry_t* entry = listPrepareEntry (data, list->flags, key);
    ListLock (list);
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
    ListUnlock (list);
    return entry;
}

LIBNEX_PUBLIC ListEntry_t* ListAddBeforeKey (ListHead_t* list, const void* data, int key, int keyAfter)
{
    ListEntry_t* entryAfter = ListFind (list, keyAfter);
    if (!entryAfter)
        return NULL;
    return ListAddBefore (list, data, key, entryAfter);
}

LIBNEX_PUBLIC ListEntry_t* ListAddAfter (ListHead_t* list, const void* data, int key, ListEntry_t* entryBefore)
{
    assert (list && data);
    ListEntry_t* entry = listPrepareEntry (data, list->flags, key);
    ListLock (list);
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
    ListUnlock (list);
    return entry;
}

LIBNEX_PUBLIC ListEntry_t* ListAddAfterKey (ListHead_t* list, const void* data, int key, int keyBefore)
{
    ListEntry_t* entryBefore = ListFind (list, keyBefore);
    if (!entryBefore)
        return NULL;
    return ListAddAfter (list, data, key, entryBefore);
}

LIBNEX_PUBLIC ListEntry_t* ListPopFront (ListHead_t* list)
{
    assert (list);
    ListLock (list);
    if (!list->front)
    {
        ListUnlock (list);
        return NULL;
    }
    ListEntry_t* entry = list->front;
    ListLock (entry);
    if (entry->next)
        entry->next->prev = NULL;
    list->front = entry->next;
    ListUnlock (entry);
    ListUnlock (list);
    return entry;
}

LIBNEX_PUBLIC ListEntry_t* ListRemoveKey (ListHead_t* list, int key)
{
    ListEntry_t* entry = ListFind (list, key);
    if (!entry)
        return NULL;
    ListRemove (list, entry);
}

// Internal function to remove a list entry
static ListEntry_t* listRemove (ListHead_t* list, ListEntry_t* entry, bool doRef)
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

LIBNEX_PUBLIC ListEntry_t* ListRemove (ListHead_t* list, ListEntry_t* entry)
{
    assert (list && entry);
    if (!ListDeRef (entry))
        return listRemove (list, entry, false);
    else
        return entry;
}

LIBNEX_PUBLIC void ListDestroyEntry (ListHead_t* list, ListEntry_t* entry)
{
    assert (list && entry);
    // De-reference the entry and remove it
    if (!ListDeRef (entry))
    {
        listRemove (list, entry, true);
        // Destroy it if we have a callback
        if (list->destroyFunc)
            list->destroyFunc (entry->data);
    }
}

LIBNEX_PUBLIC void ListDestroy (ListHead_t* list)
{
    assert (list);
    if (!ListDeRef (list))
    {
        // Go through every entry, destroying it
        ListEntry_t* curEntry = list->front;
        while (curEntry)
        {
            ListEntry_t* next = curEntry->next;
            ListDestroyEntry (list, curEntry);
            curEntry = next;
        }
        free (list);
    }
}
