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

LIBNEX_PUBLIC ListHead_t* ListCreate (const char* type, bool usesObj, size_t offToObj)
{
    ListHead_t* head = calloc (sizeof (ListHead_t), 1);
    if (!head)
        return NULL;
    // Initialize the object associated with this list
    ObjCreate (type, &head->obj);
    // Initialize the other stuff
    head->usesObj = usesObj;
    head->objOffset = offToObj;
    return head;
}

LIBNEX_PUBLIC void ListSetCmp (ListHead_t* list, ListEntryCmp func)
{
    ListLock (list);
    list->cmpFunc = func;
    ListUnlock (list);
}

LIBNEX_PUBLIC void ListSetFindBy (ListHead_t* list, ListEntryFindBy func)
{
    ListLock (list);
    list->findByFunc = func;
    ListUnlock (list);
}

LIBNEX_PUBLIC void ListSetDestroy (ListHead_t* list, ListEntryDestroy func)
{
    ListLock (list);
    list->destroyFunc = func;
    ListUnlock (list);
}

LIBNEX_PUBLIC ListEntry_t* ListAddFront (ListHead_t* head, void* data, const int key)
{
    ListEntry_t* entry = malloc (sizeof (ListEntry_t));
    if (!entry)
        return NULL;
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

LIBNEX_PUBLIC ListEntry_t* ListAddBack (ListHead_t* head, void* data, const int key)
{
    ListEntry_t* entry = malloc (sizeof (ListEntry_t));
    if (!entry)
        return NULL;
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

LIBNEX_PUBLIC ListEntry_t* ListFind (ListHead_t* list, const int key)
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

LIBNEX_PUBLIC ListEntry_t* ListFindEntryBy (ListHead_t* list, void* data)
{
    ListLock (list);
    ListUnlock (list);
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

LIBNEX_PUBLIC ListEntry_t* ListAddBefore (ListHead_t* list, void* data, const int key, ListEntry_t* entryAfter)
{
    ListEntry_t* entry = (ListEntry_t*) malloc (sizeof (ListEntry_t));
    if (!entry)
        return NULL;
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

LIBNEX_PUBLIC ListEntry_t* ListAddBeforeKey (ListHead_t* list, void* data, const int key, const int keyAfter)
{
    ListEntry_t* entryAfter = ListFind (list, keyAfter);
    if (!entryAfter)
        return NULL;
    return ListAddBefore (list, data, key, entryAfter);
}

LIBNEX_PUBLIC ListEntry_t* ListAddAfter (ListHead_t* list, void* data, const int key, ListEntry_t* entryBefore)
{
    ListEntry_t* entry = (ListEntry_t*) malloc (sizeof (ListEntry_t));
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

LIBNEX_PUBLIC ListEntry_t* ListAddAfterKey (ListHead_t* list, void* data, const int key, const int keyBefore)
{
    ListEntry_t* entryBefore = ListFind (list, keyBefore);
    if (!entryBefore)
        return NULL;
    return ListAddAfter (list, data, key, entryBefore);
}

LIBNEX_PUBLIC ListEntry_t* ListPopFront (ListHead_t* list)
{
    ListLock (list);
    if (!list->front)
    {
        ListUnlock (list);
        return NULL;
    }
    ListRef (list->front);
    ListEntry_t* entry = list->front;
    ListLock (entry);
    if (entry->next)
        entry->next->prev = NULL;
    list->front = entry->next;
    ListUnlock (entry);
    ListDeRef (entry);
    ListUnlock (list);
    return entry;
}

LIBNEX_PUBLIC ListEntry_t* ListRemoveKey (ListHead_t* list, const int key)
{
    ListEntry_t* entry = ListFind (list, key);
    if (!entry)
        return NULL;
    return ListRemove (list, entry);
}

// Internal function to remove a list entry
ListEntry_t* _listRemove (ListHead_t* list, ListEntry_t* entry, const int doRef)
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
    if (!ListDeRef (entry))
        return _listRemove (list, entry, 0);
    else
        return entry;
}

LIBNEX_PUBLIC void ListDestroyEntry (ListHead_t* list, ListEntry_t* entry)
{
    // De-reference the entry and remove it
    if (!ListDeRef (entry))
    {
        _listRemove (list, entry, 0);
        // Destroy it
        if (!list->destroyFunc)
        {
            // Destroy it
            if (!list->usesObj)
                free (entry->data);
            else
            {
                if (!ObjDestroy ((Object_t*) (entry->data + list->objOffset)))
                    free (entry->data);
            }
        }
        else
        {
            list->destroyFunc (entry->data);
        }
        free (entry);
    }
}

LIBNEX_PUBLIC void ListDestroy (ListHead_t* list)
{
    ListLock (list);
    // Go through every entry, destroying it
    ListEntry_t* curEntry = list->front;
    while (curEntry)
    {
        ListEntry_t* next = curEntry->next;
        ListDestroyEntry (list, curEntry);
        curEntry = next;
    }
    if (!ListDeRef (list))
    {
        ListUnlock (list);
        free (list);
    }
    else
        ListUnlock (list);
}
