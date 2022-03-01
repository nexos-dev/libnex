/*
    char16.c - contains test suite for char16 handling
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

/// @file char16.c

#include <libnex.h>
#include <stdlib.h>
#include <string.h>
#define NEXTEST_NAME "char16"
#include <nextest.h>

int main()
{
    // Test c16stombs
    const char16_t str[] = u"Test string";
    char* mbStr = malloc_s (500);
    mbstate_t state1;
    memset (&state1, 0, sizeof (mbstate_t));
    c16stombs (mbStr, str, 500, &state1);
    // Convert to wchar_t for easy comparison
    wchar_t* wcStr = malloc_s (500);
    mbsrtowcs (wcStr, (const char**) &mbStr, 500, &state1);
    TEST_BOOL (!wcscmp (wcStr, L"Test string"), "c16stombs");
    free (mbStr);
    // Test mbstoc16s
    const wchar_t* str2 = L"Test string";
    char* mbStr2 = malloc_s (500);
    mbstate_t state2;
    memset (&state2, 0, sizeof (mbstate_t));
    wcsrtombs (mbStr2, &str2, 500, &state2);
    char16_t* str3 = (char16_t*) malloc_s (500);
    mbstoc16s (str3, mbStr2, 500, 500, &state2);
    TEST_BOOL (str3[5] == 's', "mbstoc16s");

    // Test wcstoc16s
    char16_t* dest5 = (char16_t*) malloc_s (500);
    const wchar_t* str4 = L"Test string";
    wcstoc16s (dest5, str4, 500);
    TEST_BOOL (!c16cmp (dest5, u"Test string"), "wcstoc16s");

    // Test c16stowcs
    const char16_t* str5 = u"Test string";
    wchar_t* dest6 = (wchar_t*) malloc_s (500);
    c16stowcs (dest6, str5, 500);
    TEST_BOOL (!wcscmp (dest6, L"Test string"), "c16towcs");

    TEST (c16len (str3), 11, "c16len");

    TEST_BOOL (!c16cmp (str3, str), "c16cmp");

    const char16_t src1[] = u"a test string";
    char16_t dest1[10];
    char16_t dest2[16];
    char16_t dest3[14];
    // Test that truncation is detectable
    TEST (c16lcpy (dest1, src1, 10), 13, "c16lcpy truncate");
    // Check that ideal copying works
    memset (dest2, 1, 16 * sizeof (char16_t));
    TEST (c16lcpy (dest2, src1, 16), 13, "c16lcpy copy");
    TEST_BOOL (!c16cmp (src1, dest2), "c16lcpy copy");
    // Check that copying strings with equal lengths works
    memset (dest3, 1, 14 * sizeof (char16_t));
    TEST (c16lcpy (dest3, src1, 13), 13, "c16lcpy equal length copying");
    //  Test copying with size of 0
    TEST (c16lcpy (dest3, src1, 0), 13, "c16lcpy zero size copying");

    // Check truncation
    char16_t* src4 = u"string\n";
    char16_t* dest4 = malloc (14 * sizeof (char16_t));
    c16lcpy (dest4, u"a test ", 14);
    TEST (c16lcat (dest4, src4, 14), 14, "c16lcat truncation");
    free (dest4);
    // Check concatenation
    dest4 = malloc (17 * sizeof (char16_t));
    c16lcpy (dest4, u"a test ", 17);
    TEST (c16lcat (dest4, src4, 17), 14, "c16lcat concatenation");
    TEST_BOOL (!c16cmp (dest4, u"a test string\n"), "c16lcat concatenation");
    free (dest4);

    char16_t* str6 = c16chr (src1, u't');
    TEST_BOOL (!c16cmp (str6, u"test string"), "c16chr");
    TEST_BOOL (!c16chr (u"Test 1", u'i'), "c16chr");

    char16_t* str7 = c16rchr (src1, u't');
    TEST_BOOL (!c16cmp (str7, u"tring"), "c16rchr");

    char16_t* str8 = c16pbrk (src1, u"iytu");
    TEST_BOOL (!c16cmp (str8, u"test string"), "c16pbrk");

    return 0;
}
