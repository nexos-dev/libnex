/*
    strlcat.c - contains BSD strlcat test driver
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

/// @file strlcat.c

#include <libnex/safestring.h>
#include <stdio.h>
#include <stdlib.h>

#define NEXTEST_NAME "strlcat"
#include <nextest.h>

int main()
{
    // Check truncation
    char* src = "string\n";
    char* dest = malloc (14);
    strlcpy (dest, "a test ", 14);
    TEST (strlcat (dest, src, 14), 14, "truncation");
    free (dest);
    // Check concatenation
    dest = malloc (17);
    strlcpy (dest, "a test ", 17);
    TEST (strlcat (dest, src, 17), 14, "concatenation");
    free (dest);
    return 0;
}
