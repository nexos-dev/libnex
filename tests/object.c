/*
    object.c - contains object tests
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

#include <libnex/object.h>
#define NEXTEST_NAME "object"
#include <nextest.h>

int main()
{
    Object_t obj;
    ObjCreate ("Test", &obj);
    TEST_ANON (obj.id, 1);
    TEST_ANON (obj.refCount, 1);
    Object_t* obj2 = ObjRef (&obj);
    TEST_ANON (obj2->refCount, 2);
    TEST_ANON (ObjDestroy (obj2), 1);
    TEST_ANON (ObjDestroy (&obj), 0);
    TEST_BOOL_ANON (ObjCompare (&obj, obj2));
    Object_t obj3;
    ObjCreate ("Test", &obj3);
    TEST_BOOL_ANON (!ObjCompare (&obj, &obj3));
    TEST_BOOL_ANON (ObjCompareType (obj2, &obj3));
    Object_t obj4;
    ObjCreate ("Test 2", &obj4);
    TEST_BOOL_ANON (!ObjCompareType (&obj4, &obj3));
    TEST_BOOL_ANON (ObjCompareType (&obj, obj2));
    return 0;
}
