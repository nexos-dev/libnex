/*
    safestring.h - contains BSD style string functions
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

/// @file safestring.h

#ifndef _SAFESTRING_H
#define _SAFESTRING_H

#ifdef IN_LIBNEX
#include <libnex_config.h>
#else
#include <libnex/libnex_config.h>
#endif
#include <libnex/decls.h>
#include <stddef.h>

// Only declare these functions if the host didn't
#ifndef HAVE_BSD_STRING

__DECL_START

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
PUBLIC size_t strlcpy (char* dest, char* src, size_t size);

/**
 * @brief A safer version of strcat(3)
 *
 * strlcat takes the size of the valid buffer dest points to. It copies
 * src - strlen(src) - 1 bytes, null terminating the result
 *
 * @param[out] dest the string to write to
 * @param[in] src the string to read from
 * @param[in] size the size of the buffer dest points to
 * @return the lenght of dest plus the length of source
 */
PUBLIC size_t strlcat (char* dest, char* src, size_t size);

__DECL_END

#endif

#endif
