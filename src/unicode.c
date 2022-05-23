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
#include "unicode/utf8stateTab.h"
#include <libnex/unicode.h>
#include <string.h>

// UTF-16 parser states
#define UTF16_START         0
#define UTF16_LOW_SURROGATE 3
#define UTF16_ACCEPT        4

LIBNEX_PUBLIC size_t UnicodeDecode16 (char32_t* out, const uint16_t* in, size_t sz, char endian)
{
    if (!endian)
        endian = EndianHost();
    // Setup the state
    unsigned char state = UTF16_START;
    unsigned char prevState = UTF16_START;
    char32_t codepoint = 0;
    const uint16_t* oin = in;
    // Keep parsing until accepted
    while (state != UTF16_ACCEPT)
    {
        if (in > (oin + sz))
            return 0;
        if (state == UTF16_START)
            state = utf16stateTab[EndianRead16 (in, endian) >> 10];
        else
        {
            if (state != utf16stateTab[EndianRead16 (in, endian) >> 10])
            {
                *out = 0xFFFD;
                return in - oin;
            }
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

LIBNEX_PUBLIC size_t UnicodeEncode16 (uint16_t* out, char32_t in, char endian)
{
    if (!endian)
        endian = EndianHost();
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
        // Encode second surrogate
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

// States
#define UTF8_CONT   2
#define UTF8_START  0
#define UTF8_ACCEPT 6

// Checks if this is a bad octet
#define IsBadOctect(oct) ((oct) == 0xC0 || (oct) == 0xC1 || ((oct) >= 0xF5))

LIBNEX_PUBLIC size_t UnicodeDecodePart8 (char32_t* out, uint8_t in, Utf8State_t* state)
{
    // If this sequence has been accepted, return
    if (state->state == UTF8_ACCEPT)
        return 0;

    // Check if we need to initialize the state
    if (state->state == UTF8_START)
    {
        // Get the intial state
        state->state = utf8stateTab[in >> 3];
        state->prevState = 0;
        // Ensure this isn't a continuation
        if (state->state == UTF8_CONT)
            return 0;
        // Get the size of this character
        state->bytesLeft = utf8sizeTab[state->state];
        state->bytesRequired = state->bytesLeft;
        *out = 0;
    }
    else
    {
        // Ensure this is the right state
        if (state->state != utf8stateTab[in >> 3])
            return 0;
    }

    // Ensure in isn't a bad octet
    if (IsBadOctect (in))
        return 0;

    // Update the codepoint
    *out = (in & utf8maskTab[state->state]) | ((*out) << utf8shiftTab[state->state]);

    // Set the state
    state->prevState = state->state;
    state->state = utf8nextTab[state->prevState];

    // Should we accept this sequence?
    if ((--state->bytesLeft) == 0)
        state->state = UTF8_ACCEPT;
    return 1;
}

LIBNEX_PUBLIC size_t UnicodeDecode8 (char32_t* out, const uint8_t* in, size_t sz)
{
    Utf8State_t state;
    const uint8_t* oin = in;
    memset (&state, 0, sizeof (Utf8State_t));
    do
    {
        // Bounds check
        if (in > (oin + sz))
            return 0;
        // Decode current octet
        if (!UnicodeDecodePart8 (out, *in, &state))
            *out = 0xFFFD;
        ++in;
    } while (!UnicodeIsAccepted (state));
    return in - oin;
}

LIBNEX_PUBLIC size_t UnicodeEncode8 (uint8_t* out, char32_t in, size_t sz)
{
    // Figure out the size needed and create the leading byte
    if (in <= 0x7F)
    {
        *out = (uint8_t) BitMask (in, 0x7F);
        return 1;
    }
    else if (in <= 0x7FF)
    {
        if ((out + 2) > (out + sz))
            return 0;
        // Set up leading byte
        *out = 0xC0 | (in >> 6);
        ++out;
        BitClearRange (in, 6, 5);
        // Encode continuation
        *out = (uint8_t) in | 0x80;
        return 2;
    }
    else if (in <= 0xFFFF)
    {
        if ((out + 3) > (out + sz))
            return 0;
        *out = 0xE0 | (in >> 12);
        ++out;
        BitClearRange (in, 12, 4);
        *out = (in >> 6) | 0x80;
        ++out;
        *out = (BitClearRangeNew (in, 6, 5)) | 0x80;
        return 3;
    }
    else if (in <= 0x10FFFF)
    {
        if ((out + 4) > (out + sz))
            return 0;
        *out = 0xF0 | (in >> 0x12);
        ++out;
        BitClearRange (in, 18, 3);
        *out = (in >> 12) | 0x80;
        ++out;
        BitClearRange (in, 12, 6);
        *out = (in >> 6) | 0x80;
        ++out;
        BitClearRange (in, 6, 6);
        *out = in | 0x80;
        return 4;
    }
    else
        return 0;
}

LIBNEX_PUBLIC void UnicodeWriteBom8 (uint8_t* buf)
{
    buf[0] = 0xEF;
    buf[1] = 0xBB;
    buf[2] = 0xBF;
}

LIBNEX_PUBLIC void UnicodeWriteBom16 (uint16_t* out, char order)
{
    if (!order)
        order = EndianHost();
    EndianWrite16 (out, 0xFEFF, order);
}

LIBNEX_PUBLIC void UnicodeWriteBom32 (uint32_t* out, char order)
{
    if (!order)
        order = EndianHost();
    EndianWrite32 (out, 0xFEFF, order);
}

LIBNEX_PUBLIC bool UnicodeReadBom8 (const uint8_t* bom)
{
    // Check its validity
    if (!(bom[0] == 0xEF && bom[1] == 0xBB && bom[2] == 0xBF))
        return false;
    else
        return true;
}

LIBNEX_PUBLIC char UnicodeReadBom16 (const uint8_t* bom)
{
    // Check the BOM's order
    if (bom[0] == 0xFF && bom[1] == 0xFE)
        return ENDIAN_LITTLE;
    else if (bom[0] == 0xFE && bom[1] == 0xFF)
        return ENDIAN_BIG;
    else
        return 0;
}

LIBNEX_PUBLIC char UnicodeReadBom32 (const uint8_t* bom)
{
    // Check the BOM's order
    if (bom[0] == 0xFF && bom[1] == 0xFE && bom[2] == 0x00 && bom[3] == 0x00)
        return ENDIAN_LITTLE;
    else if (bom[0] == 0x00 && bom[1] == 0x00 && bom[2] == 0xFE && bom[3] == 0xFF)
        return ENDIAN_BIG;
    else
        return 0;
}
