/*
    list.c - linked list test driver
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

#include <libnex.h>

#define NEXTEST_NAME "list"
#include <nextest.h>

int main()
{
    // Test list creation
    ListHead_t* head = ListCreate ("Test");
    TEST_BOOL_ANON (head->back == NULL && head->front == NULL);
    // Test adding to the front of a list
    ListEntry_t* entry1 = ListAddFront (head, NULL, 1);
    TEST_BOOL_ANON (entry1->next == NULL && entry1->prev == NULL);
    TEST_BOOL_ANON (head->front->key == entry1->key && head->back->key == entry1->key);
    // Test adding to the front of the list again
    ListEntry_t* entry2 = ListAddFront (head, NULL, 2);
    TEST_BOOL_ANON (entry2->next->key == entry1->key && entry1->prev->key == entry2->key &&
                    head->front->key == entry2->key && entry2->prev == NULL);
    // Test adding to the back of the list
    ListEntry_t* entry3 = ListAddBack (head, NULL, 3);
    TEST_BOOL_ANON (entry3->next == NULL && entry1->next->key == entry3->key && entry3->prev->key == entry1->key &&
                    head->back->key == entry3->key);
    // Test adding an item at the end of the list with the ListAddAfterKey function
    ListEntry_t* entry4 = ListAddAfterKey (head, NULL, 4, entry3->key);
    TEST_BOOL_ANON (entry4->next == NULL && entry4->prev->key == entry3->key && entry3->next->key == entry4->key);
    // Test adding an item in the middle
    ListEntry_t* entry5 = ListAddAfterKey (head, NULL, 5, entry1->key);
    TEST_BOOL_ANON (entry5->next->key == entry3->key && entry5->prev->key == entry1->key &&
                    entry3->prev->key == entry5->key && entry1->next->key == entry5->key);
    // Test ListAddBeforeKey in the same way
    // Test adding an item at the end of the list with the ListAddBeforeKey function
    ListEntry_t* entry6 = ListAddBeforeKey (head, NULL, 6, entry2->key);
    TEST_BOOL_ANON (entry6->prev == NULL && entry6->next->key == entry2->key && entry2->prev->key == entry6->key);
    // Test adding an item in the middle
    ListEntry_t* entry7 = ListAddBeforeKey (head, NULL, 7, entry1->key);
    TEST_BOOL_ANON (entry7->next->key == entry1->key && entry7->prev->key == entry2->key &&
                    entry1->prev->key == entry7->key && entry2->next->key == entry7->key);
    // Test that every item in the list is in the right order going forwards
    ListEntry_t* iter = head->front;
    TEST_BOOL_ANON (iter->key == entry6->key);
    iter = iter->next;
    TEST_BOOL_ANON (iter->key == entry2->key);
    iter = iter->next;
    TEST_BOOL_ANON (iter->key == entry7->key);
    iter = iter->next;
    TEST_BOOL_ANON (iter->key == entry1->key);
    iter = iter->next;
    TEST_BOOL_ANON (iter->key == entry5->key);
    iter = iter->next;
    TEST_BOOL_ANON (iter->key == entry3->key);
    iter = iter->next;
    TEST_BOOL_ANON (iter->key == entry4->key);
    TEST_BOOL_ANON (iter->key == head->back->key && iter->next == NULL);

    // .. and backwards
    TEST_BOOL_ANON (iter->key == entry4->key);
    iter = iter->prev;
    TEST_BOOL_ANON (iter->key == entry3->key);
    iter = iter->prev;
    TEST_BOOL_ANON (iter->key == entry5->key);
    iter = iter->prev;
    TEST_BOOL_ANON (iter->key == entry1->key);
    iter = iter->prev;
    TEST_BOOL_ANON (iter->key == entry7->key);
    iter = iter->prev;
    TEST_BOOL_ANON (iter->key == entry2->key);
    iter = iter->prev;
    TEST_BOOL_ANON (iter->key == entry6->key);
    TEST_BOOL_ANON (iter->key == head->front->key && iter->prev == NULL);

    // Test finding an item
    TEST_BOOL_ANON (ListFind (head, 1)->key == 1 && ListFind (head, 2)->key == 2 && ListFind (head, 3)->key == 3 &&
                    ListFind (head, 9) == NULL);

    // Test popping an item off the front
    TEST_BOOL_ANON (ListPopFront (head)->key == entry6->key);

    // Test removing an item on the front
    ListRemoveKey (head, entry2->key);
    TEST_BOOL_ANON (entry7->key == head->front->key && entry7->prev == NULL);
    // Test removing an item on the back
    ListRemoveKey (head, entry4->key);
    TEST_BOOL_ANON (entry3->key == head->back->key && entry3->next == NULL);
    // Test removing an item in the middle
    ListRemoveKey (head, entry1->key);
    TEST_BOOL_ANON (entry7->next->key == entry5->key && entry5->prev->key == entry7->key);
    // Test destroying
    ListDestroyEntry (head, entry4);

    // Test destroying the list
    ListDestroy (head);

    return 0;
}
