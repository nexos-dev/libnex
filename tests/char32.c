/*
    unicode.c - contains test suite for unicode handling
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

/// @file unicode.c

#include <libnex.h>
#include <stdlib.h>
#include <string.h>
#define NEXTEST_NAME "char32"
#include <nextest.h>

int main()
{
    // Test c32stombs
    const char32_t str[] = U"Test string";
    char* mbStr = malloc_s (500);
    mbstate_t state1;
    memset (&state1, 0, sizeof (mbstate_t));
    c32stombs (mbStr, str, 500, &state1);
    // Convert to wchar_t for easy comparison
    wchar_t* wcStr = malloc_s (500);
    mbsrtowcs (wcStr, (const char**) &mbStr, 500, &state1);
    TEST_BOOL (!wcscmp (wcStr, L"Test string"), "c32stombs");
    free (mbStr);
    // Test mbstoc32s
    const wchar_t* str2 = L"Test string";
    char* mbStr2 = malloc_s (500);
    mbstate_t state2;
    memset (&state2, 0, sizeof (mbstate_t));
    wcsrtombs (mbStr2, &str2, 500, &state2);
    char32_t* str3 = (char32_t*) malloc_s (500);
    mbstoc32s (str3, mbStr2, 500, 500, &state2);
    TEST_BOOL (str3[5] == 's', "mbstoc32s");

    // Test wcstoc32s
    char32_t* dest5 = (char32_t*) malloc_s (500);
    const wchar_t* str4 = L"Test string";
    wcstoc32s (dest5, str4, 500);
    TEST_BOOL (!c32cmp (dest5, U"Test string"), "wcstoc32s");

    // Test c32stowcs
    const char32_t* str5 = U"Test string";
    wchar_t* dest6 = (wchar_t*) malloc_s (500);
    c32stowcs (dest6, str5, 500);
    TEST_BOOL (!wcscmp (dest6, L"Test string"), "c32towcs");

    TEST (c32len (str3), 11, "c32len");

    TEST_BOOL (!c32cmp (str3, str), "c32cmp");

    const char32_t src1[] = U"a test string";
    char32_t dest1[10];
    char32_t dest2[16];
    char32_t dest3[14];
    // Test that truncation is detectable
    TEST (c32lcpy (dest1, src1, 10), 13, "c32lcpy truncate");
    // Check that ideal copying works
    memset (dest2, 1, 16 * sizeof (char32_t));
    TEST (c32lcpy (dest2, src1, 16), 13, "c32lcpy copy");
    TEST_BOOL (!c32cmp (src1, dest2), "c32lcpy copy");
    // Check that copying strings with equal lengths works
    memset (dest3, 1, 14 * sizeof (char32_t));
    TEST (c32lcpy (dest3, src1, 13), 13, "c32lcpy equal length copying");
    //  Test copying with size of 0
    TEST (c32lcpy (dest3, src1, 0), 13, "c32lcpy zero size copying");

    // Check truncation
    char32_t* src4 = U"string\n";
    char32_t* dest4 = malloc (14 * sizeof (char32_t));
    c32lcpy (dest4, U"a test ", 14);
    TEST (c32lcat (dest4, src4, 14), 14, "c32lcat truncation");
    free (dest4);
    // Check concatenation
    dest4 = malloc (17 * sizeof (char32_t));
    c32lcpy (dest4, U"a test ", 17);
    TEST (c32lcat (dest4, src4, 17), 14, "c32lcat concatenation");
    TEST_BOOL (!c32cmp (dest4, U"a test string\n"), "c32lcat concatenation");
    free (dest4);

    char32_t* str6 = c32chr (src1, U't');
    TEST_BOOL (!c32cmp (str6, U"test string"), "c32chr");
    TEST_BOOL (!c32chr (U"Test 1", U'i'), "c32chr");

    char32_t* str7 = c32rchr (src1, U't');
    TEST_BOOL (!c32cmp (str7, U"tring"), "c32rchr");

    char32_t* str8 = c32pbrk (src1, U"iytu");
    TEST_BOOL (!c32cmp (str8, U"test string"), "c32pbrk");

    return 0;
}
