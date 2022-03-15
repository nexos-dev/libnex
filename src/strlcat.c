/*
    strlcat.c - contains BSD strlcat
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
#include <string.h>

/**
 * @brief A safer version of strcat(3)
 *
 * strlcat takes the size of the valid buffer dest points to. It copies
 * size - strlen(src) - 1 bytes, null terminating the result
 *
 * @param[out] dest the string to write to
 * @param[in] src the string to read from
 * @param[in] size the size of the buffer dest points to
 * @return the length of dest plus the length of source
 */
LIBNEX_PUBLIC size_t strlcat (char* dest, const char* src, size_t size)
{
    const char* osrc = src;
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
