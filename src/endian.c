/*
    endian.c - contains declaration to handle values endian independently
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

/// @file endian.c

#include <libnex/endian.h>

/**
 * @brief Gets the endianess of the host
 *
 * EndianHost() gets the endianess of the host, returning ENDIAN_BIG or ENDIAN_LITTLE
 * @return ENDIAN_BIG if host is big endian, ENDIAN_LITTLE if little endian
 */
PUBLIC char EndianHost()
{
    // Declare a 16 bit value
    uint16_t val = 0x9867;
    // Cast to pointer so we can access the individual bytes
    uint8_t* val8 = (uint8_t*) &val;
    // Check what order the bytes are in
    if (val8[0] == 0x98)
        return ENDIAN_BIG;
    else if (val8[0] == 0x67)
        return ENDIAN_LITTLE;
    return 0;
}

/**
 * @brief Writes a 16 bit value with specified endianess
 * @param buf the buffer to write to
 * @param val 16 bit value, in host's order
 * @param endian ENDIAN_BIG if writing big endian, ENDIAN_LITTLE if little endian
 */
PUBLIC int EndianWrite16 (uint16_t* buf, uint16_t val, char endian)
{
    if (!buf)
        return 0;
    // Swapping must occur is target endian and host endian are different
    if (endian != EndianHost())
        *buf = EndianSwap16 (val);
    else
        *buf = val;
    return 1;
}

/**
 * @brief Writes a 32 bit value with specified endianess
 * @param buf the buffer to write to
 * @param val 32 bit value, in host's order
 * @param endian ENDIAN_BIG if writing big endian, ENDIAN_LITTLE if little endian
 */
PUBLIC int EndianWrite32 (uint32_t* buf, uint32_t val, char endian)
{
    if (!buf)
        return 0;
    // Swapping must occur is target endian and host endian are different
    if (endian != EndianHost())
        *buf = EndianSwap32 (val);
    else
        *buf = val;
    return 1;
}

/**
 * @brief Writes a 64 bit value with specified endianess
 * @param buf the buffer to write to
 * @param val 64 bit value, in host's order
 * @param endian ENDIAN_BIG if writing big endian, ENDIAN_LITTLE if little endian
 */
PUBLIC int EndianWrite64 (uint64_t* buf, uint64_t val, char endian)
{
    if (!buf)
        return 0;
    // Swapping must occur is target endian and host endian are different
    if (endian != EndianHost())
        *buf = EndianSwap64 (val);
    else
        *buf = val;
    return 1;
}

/**
 * @brief Reads a 16 bit value with specified endianess
 * @param buf the buffer to read from
 * @param endian ENDIAN_BIG if reading big endian, ENDIAN_LITTLE if little endian
 * @return the value in the host's order
 */
PUBLIC uint16_t EndianRead16 (uint16_t* buf, char endian)
{
    if (!buf)
        return 0;
    if (endian != EndianHost())
        return EndianSwap16 (*buf);
    else
        return *buf;
}

/**
 * @brief Reads a 32 bit value with specified endianess
 * @param buf the buffer to read from
 * @param endian ENDIAN_BIG if reading big endian, ENDIAN_LITTLE if little endian
 * @return the value in the host's order
 */
PUBLIC uint32_t EndianRead32 (uint32_t* buf, char endian)
{
    if (!buf)
        return 0;
    if (endian != EndianHost())
        return EndianSwap32 (*buf);
    else
        return *buf;
}

/**
 * @brief Reads a 64 bit value with specified endianess
 * @param buf the buffer to read from
 * @param endian ENDIAN_BIG if reading big endian, ENDIAN_LITTLE if little endian
 * @return the value in the host's order
 */
PUBLIC uint64_t EndianRead64 (uint64_t* buf, char endian)
{
    if (!buf)
        return 0;
    if (endian != EndianHost())
        return EndianSwap64 (*buf);
    else
        return *buf;
}
