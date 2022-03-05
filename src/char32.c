/*
    char32.c - contains char32_t* manipulation functions
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

#include <libnex/safemalloc.h>
#include <libnex/unicode.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

PUBLIC ssize_t c32stombs (char* mbStr, const char32_t* u32str, size_t sz, mbstate_t* state)
{
    char* ombStr = mbStr;
    ssize_t res = 0;
    do
    {
        res = (ssize_t) c32rtomb (mbStr, *u32str, state);
        // A little hack. On Windows, c32rtomb doesn't return 0 when it enounters a null character
        if (*u32str == 0)
            return mbStr - ombStr;
        if (res < 0)
            return res;
        mbStr += res;
        ++u32str;
    } while (res && mbStr < (mbStr + sz));
    return mbStr - ombStr;
}

PUBLIC ssize_t mbstoc32s (char32_t* u32str, const char* mbStr, size_t sz, size_t mbSz, mbstate_t* state)
{
    char32_t* ou32str = u32str;
    ssize_t res = 0;
    do
    {
        res = (ssize_t) mbrtoc32 (u32str, mbStr, mbSz, state);
        if (res < 0)
            return res;
        else if (!res)
            return u32str - ou32str;
        mbSz -= res;
        mbStr += res;
        ++u32str;
    } while (res && u32str < (u32str + sz));
    return u32str - ou32str;
}

PUBLIC ssize_t wcstoc32s (char32_t* u32str, const wchar_t* wcStr, size_t sz)
{
    // Get lengths of buffers
    size_t wcLen = wcslen (wcStr);
    // FIXME: Uses too much memory
    size_t mbLen = wcLen * MB_CUR_MAX;
    // Do the conversion to multibyte form
    char* mbStr = malloc_s (mbLen + 1);
    mbstate_t state;
    memset (&state, 0, sizeof (mbstate_t));
    if (wcsrtombs (mbStr, &wcStr, wcLen + 1, &state) == -1)
        return -1;
    // Convert to char32_t form
    ssize_t res = mbstoc32s (u32str, mbStr, sz, mbLen + 1, &state);
    free (mbStr);
    return res;
}

PUBLIC ssize_t c32stowcs (wchar_t* wcStr, const char32_t* u32str, size_t sz)
{
    // Get lengths of buffer
    size_t wideLen = c32len (u32str);
    // FIXME: Uses too much memory
    size_t mbLen = wideLen * MB_CUR_MAX;
    // Convert to multibyte
    char* mbStr = malloc_s (mbLen + 1);
    mbstate_t state;
    memset (&state, 0, sizeof (mbstate_t));
    if (c32stombs (mbStr, u32str, wideLen + 1, &state) < 0)
        return -1;
    // Convert to wide
    ssize_t res = (ssize_t) mbsrtowcs (wcStr, (const char**) &mbStr, sz, &state);
    free (mbStr);
    return res;
}

PUBLIC size_t c32len (const char32_t* s)
{
    const char32_t* os = s;
    while (*s)
        ++s;
    return s - os;
}

PUBLIC int c32cmp (const char32_t* s1, const char32_t* s2)
{
    while (*s1 && *s2)
    {
        if (*s1 != *s2)
            break;
        s1++;
        s2++;
    }
    return (int) *s1 - (int) *s2;
}

PUBLIC int c32ncmp (const char32_t* s1, const char32_t* s2, size_t n)
{
    while (n-- > 0 && *s1 && *s2)
    {
        if (*s1 != *s2)
            break;
        s1++;
        s2++;
    }
    return (int) *s1 - (int) *s2;
}

PUBLIC size_t c32lcat (char32_t* dest, const char32_t* src, size_t size)
{
    const char32_t* osrc = src;
    // Advance dest to null terminator
    size_t destSz = 0;
    while (*++dest)
    {
        // Check that we don't run off the string
        if (destSz == size)
            return size;
        destSz++;
    }
    // Main append loop
    size_t appendSz = size - destSz - 1;
    while (--appendSz)
    {
        if ((*dest++ = *src++) == '\0')
            break;
    }
    // Null terminate
    if (!appendSz)
    {
        if (size)
            *dest = '\0';
        while (*src++)
            ;
    }
    return destSz + (src - osrc);
}

PUBLIC size_t c32lcpy (char32_t* dest, const char32_t* src, size_t size)
{
    const char32_t* osrc = src;
    size_t sz = size;
    // Main copy loop
    if (size)
    {
        while (--sz)
        {
            if ((*dest++ = *src++) == '\0')
                break;
        }
    }
    // Null terminate dest if truncation occured
    if (!sz)
    {
        if (size)
            *dest = '\0';
        // Get size of input buffer
        while (*src++)
            ;
    }
    return src - osrc - 1;
}

PUBLIC char32_t* c32chr (const char32_t* str, char32_t c)
{
    while (*str)
    {
        if (*str == c)
            return (char32_t*) str;
        ++str;
    }
    return NULL;
}

PUBLIC char32_t* c32rchr (const char32_t* str, char32_t c)
{
    // There are possibly more effecient ways of doing this...
    const char32_t* rs = str + c32len (str);
    while (rs >= str)
    {
        if (*rs == c)
            return (char32_t*) rs;
        --rs;
    }
    return NULL;
}

PUBLIC char32_t* c32pbrk (const char32_t* s1, const char32_t* s2)
{
    const char32_t* p1 = s1;
    const char32_t* p2 = NULL;
    while (*p1)
    {
        p2 = s2;
        while (*p2)
        {
            if (*p1 == *p2)
                return (char32_t*) p1;
            ++p2;
        }
        ++p1;
    }
    return NULL;
}

PUBLIC char32_t* c32dup (char32_t* str)
{
    size_t len = c32len (str);
    char32_t* s = (char32_t*) malloc_s (len * sizeof (char32_t));
    c32lcpy (s, str, len);
    return s;
}
