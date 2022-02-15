/*
    strlcpy.c - contains BSD strlcpy test driver
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

/// @file strlcpy.c

#include <libnex/safestring.h>
#include <stdio.h>
#include <string.h>

#define NEXTEST_NAME "strlcpy"

#include <nextest.h>

int main()
{
    char src1[] = "a test string";
    char dest1[10];
    char dest2[16];
    char dest3[14];
    // Test that truncation is detectable
    TEST (strlcpy (dest1, src1, 10), 13, "truncate");
    // Check that ideal copying works
    memset (dest2, 1, 16);
    TEST (strlcpy (dest2, src1, 16), 13, "copy");
    // Check that copying strings with equal lengths works
    memset (dest3, 1, 14);
    TEST (strlcpy (dest3, src1, 13), 13, "equal length copying");
    // Test copying with size of 0
    TEST (strlcpy (dest3, src1, 0), 13, "zero size copying");
    return 0;
}
