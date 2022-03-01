/*
    unicode.h - contains declarations to work with text
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

/// @file unicode.h

#ifndef _UNICODE_H
#define _UNICODE_H

#ifdef IN_LIBNEX
#include <libnex_config.h>
#else
#include <libnex/libnex_config.h>
#endif
#include <libnex/char16.h>
#include <libnex/decls.h>
#include <libnex/endian.h>
#include <uchar.h>
#include <wchar.h>

__DECL_START

/**
 * @brief Decodes a UTF-16 character to UTF-32
 * @param out the character to write out the decoded data to
 * @param in a pointer to UTF-16 data to decode
 * @param endian the endianess of the input buffer, either ENDIAN_LITTLE or ENDIAN_BIG
 * @return How many 16 bit values were decoded. If -1, then an invalid character was found
 */
PUBLIC ssize_t UnicodeDecode16 (char32_t* out, const uint16_t* in, char endian);

/**
 * @brief Encodes s UTF-32 character as UTF-16
 * @param out the buffer to write out the 16 bit values to
 * @param in UTF-32 character to encode
 * @param endian the endianess of out
 * @return the number of 16 bit values encoded
 */
PUBLIC size_t UnicodeEncode16 (uint16_t* out, char32_t in, char endian);

__DECL_END

#endif
