/*
    strlcpy.c - contains BSD strlcpy
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

/**
 * @brief A safer version of strcpy(3)
 *
 * strlcpy takes a buffer size, and copies at most size - 1 bytes, padding with null character.
 * It returns the length of the buffer they tried to create.
 *
 * @param[out] dest the string to write out to
 * @param[in] src the string copy from
 * @param[in] size the size of dest
 * @return the length of src
 */
LIBNEX_PUBLIC size_t strlcpy (char* dest, const char* src, size_t size)
{
    const char* osrc = src;
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
