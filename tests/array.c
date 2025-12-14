/*
    array.c - array test driver
    Copyright 2023 - 2025 The NexNix Project

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

/// @file array.c

#include <libnex.h>

#define NEXTEST_NAME "array"
#include <nextest.h>

typedef struct _tests
{
    uint32_t num;
} TestStruct_t;

bool findBy (const void* data, const void* hint)
{
    uint32_t num = (uint32_t) hint;
    TestStruct_t* s = data;
    if (s->num == num)
        return true;
    return false;
}

int main()
{

    return 0;
}
