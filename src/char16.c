/*
    char16.c - contains char16_t* manipulation functions
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

PUBLIC ssize_t c16stombs (char* mbStr, const char16_t* u32str, size_t sz, mbstate_t* state)
{
    char* ombStr = mbStr;
    ssize_t res = 0;
    while (*u32str && mbStr < (mbStr + sz))
    {
        res = (ssize_t) c16rtomb (mbStr, *u32str, state);
        if (res < 0)
            return res;
        mbStr += res;
        ++u32str;
    }
    return mbStr - ombStr;
}

PUBLIC ssize_t mbstoc16s (char16_t* u32str, const char* mbStr, size_t sz, size_t mbSz, mbstate_t* state)
{
    char16_t* ou32str = u32str;
    ssize_t res = 0;
    do
    {
        res = (ssize_t) mbrtoc16 (u32str, mbStr, mbSz, state);
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

PUBLIC ssize_t wcstoc16s (char16_t* u32str, const wchar_t* wcStr, size_t sz)
{
    // Get lengths of buffers
    size_t wcLen = wcslen (wcStr);
    // FIXME: Uses too much memory
    size_t mbLen = wcLen * MB_CUR_MAX;
    // Do the conversion to multibyte form
    char* mbStr = malloc_s (mbLen);
    mbstate_t state;
    memset (&state, 0, sizeof (mbstate_t));
    if (wcsrtombs (mbStr, &wcStr, mbLen, &state) == -1)
        return -1;
    // Convert to char16_t form
    ssize_t res = mbstoc16s (u32str, mbStr, sz, mbLen, &state);
    free (mbStr);
    return res;
}

PUBLIC ssize_t c16stowcs (wchar_t* wcStr, const char16_t* u32str, size_t sz)
{
    // Get lengths of buffer
    size_t wideLen = c16len (u32str);
    // FIXME: Uses too much memory
    size_t mbLen = wideLen * MB_CUR_MAX;
    // Convert to multibyte
    char* mbStr = malloc_s (mbLen);
    mbstate_t state;
    memset (&state, 0, sizeof (mbstate_t));
    if (c16stombs (mbStr, u32str, mbLen, &state) < 0)
        return -1;
    // Convert to wide
    ssize_t res = (ssize_t) mbsrtowcs (wcStr, (const char**) &mbStr, sz, &state);
    free (mbStr);
    return res;
}

PUBLIC size_t c16len (const char16_t* s)
{
    const char16_t* os = s;
    while (*s)
        ++s;
    return s - os;
}

PUBLIC int c16cmp (const char16_t* s1, const char16_t* s2)
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

PUBLIC int c16ncmp (const char16_t* s1, const char16_t* s2, size_t n)
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

PUBLIC size_t c16lcat (char16_t* dest, const char16_t* src, size_t size)
{
    const char16_t* osrc = src;
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

PUBLIC size_t c16lcpy (char16_t* dest, const char16_t* src, size_t size)
{
    const char16_t* osrc = src;
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

PUBLIC char16_t* c16chr (const char16_t* str, char16_t c)
{
    while (*str)
    {
        if (*str == c)
            return (char16_t*) str;
        ++str;
    }
    return NULL;
}

PUBLIC char16_t* c16rchr (const char16_t* str, char16_t c)
{
    // There are possibly more effecient ways of doing this...
    const char16_t* rs = str + c16len (str);
    while (rs >= str)
    {
        if (*rs == c)
            return (char16_t*) rs;
        --rs;
    }
    return NULL;
}

PUBLIC char16_t* c16pbrk (const char16_t* s1, const char16_t* s2)
{
    const char16_t* p1 = s1;
    const char16_t* p2 = NULL;
    while (*p1)
    {
        p2 = s2;
        while (*p2)
        {
            if (*p1 == *p2)
                return (char16_t*) p1;
            ++p2;
        }
        ++p1;
    }
    return NULL;
}

PUBLIC char16_t* c16dup (char16_t* str)
{
    size_t len = c16len (str);
    char16_t* s = (char16_t*) malloc_s (len * sizeof (char16_t));
    c16lcpy (s, str, len);
    return s;
}
