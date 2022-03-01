/*
    unicode.c - contains unicode handling functions
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

#include "unicode/utf16stateTab.h"
#include <libnex/unicode.h>

// UTF-16 parser states
#define UTF16_START          0
#define UTF16_SCALAR         1
#define UTF16_HIGH_SURROGATE 2
#define UTF16_LOW_SURROGATE  3
#define UTF16_ACCEPT         4

PUBLIC ssize_t UnicodeDecode16 (char32_t* out, const uint16_t* in, char endian)
{
    // Setup the state
    unsigned char state = UTF16_START;
    unsigned char prevState = UTF16_START;
    char32_t codepoint = 0;
    const uint16_t* oin = in;
    // Keep parsing until accepted
    while (state != UTF16_ACCEPT)
    {
        if (state == UTF16_START)
            state = utf16stateTab[EndianRead16 (in, endian) >> 10];
        else
        {
            if (state != utf16stateTab[EndianRead16 (in, endian) >> 10])
                return -1;
        }
        uint16_t val = EndianRead16 (in, endian);
        codepoint |= ((val & utf16maskTab[state]) << utf16shiftTab[state]);
        if (state == UTF16_LOW_SURROGATE)
            codepoint += 0x10000;
        ++in;
        prevState = state;
        state = utf16nextTab[prevState];
    }
    *out = codepoint;
    return in - oin;
}

PUBLIC size_t UnicodeEncode16 (uint16_t* out, char32_t in, char endian)
{
    // Check if this should be a surrogate pair
    if (in >= 0x10000)
    {
        // Follows the algorithm in RFC 2781
        in -= 0x10000;
        // Encode first surrogate
        uint16_t sur1 = 0xD800;
        sur1 |= (in >> 10);
        EndianWrite16 (out, sur1, endian);
        ++out;
        uint16_t sur2 = 0xDC00;
        sur2 |= BitClearRange (in, 10, 10);
        EndianWrite16 (out, sur2, endian);
        return 2;
    }
    else
    {
        // Cast to UTF-16
        EndianWrite16 (out, (uint16_t) in, endian);
        return 1;
    }
}

PUBLIC ssize_t UnicodeDecode8 (char32_t* out, uint8_t* in, char endian)
{
}
