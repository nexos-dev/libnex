/*
    endian.h - contains declaration to handle value endian independently
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

/// @file endian.h

#ifndef _LIBNEX_ENDIAN_H
#define _LIBNEX_ENDIAN_H

#ifdef IN_LIBNEX
#include <libnex_config.h>
#else
#include <libnex/libnex_config.h>
#endif
#include <libnex/bits.h>
#include <stdint.h>

// Endian macro declarations
#define ENDIAN_LITTLE 1    ///< Result is little endian
#define ENDIAN_BIG    2    ///< Result is big endian

/**
 * @brief Gets the endianess of the host
 *
 * EndianHost() gets the endianess of the host, returning ENDIAN_BIG or ENDIAN_LITTLE
 * @return ENDIAN_BIG if host is big endian, ENDIAN_LITTLE if little endian
 */
PUBLIC char EndianHost();

/**
 * @brief Writes a 16 bit value with specified endianess
 * @param buf the buffer to write to
 * @param val 16 bit value, in host's order
 * @param endian ENDIAN_BIG if writing big endian, ENDIAN_LITTLE if little endian
 */
PUBLIC int EndianWrite16 (uint16_t* buf, const uint16_t val, const char endian);

/**
 * @brief Writes a 32 bit value with specified endianess
 * @param buf the buffer to write to
 * @param val 32 bit value, in host's order
 * @param endian ENDIAN_BIG if writing big endian, ENDIAN_LITTLE if little endian
 */
PUBLIC int EndianWrite32 (uint32_t* buf, const uint32_t val, const char endian);

/**
 * @brief Writes a 64 bit value with specified endianess
 * @param buf the buffer to write to
 * @param val 64 bit value, in host's order
 * @param endian ENDIAN_BIG if writing big endian, ENDIAN_LITTLE if little endian
 */
PUBLIC int EndianWrite64 (uint64_t* buf, const uint64_t val, const char endian);

/**
 * @brief Reads a 16 bit value with specified endianess
 * @param buf the buffer to read from
 * @param endian ENDIAN_BIG if reading big endian, ENDIAN_LITTLE if little endian
 * @return the value in the host's order
 */
PUBLIC uint16_t EndianRead16 (const uint16_t* buf, const char endian);

/**
 * @brief Reads a 32 bit value with specified endianess
 * @param buf the buffer to read from
 * @param endian ENDIAN_BIG if reading big endian, ENDIAN_LITTLE if little endian
 * @return the value in the host's order
 */
PUBLIC uint32_t EndianRead32 (const uint32_t* buf, const char endian);

/**
 * @brief Reads a 64 bit value with specified endianess
 * @param buf the buffer to read from
 * @param endian ENDIAN_BIG if reading big endian, ENDIAN_LITTLE if little endian
 * @return the value in the host's order
 */
PUBLIC uint64_t EndianRead64 (const uint64_t* buf, const char endian);

/**
 * @brief Swaps the endianess of a 16 bit value
 * @param val the value to swap
 * @return The swapped value
 */
#define EndianSwap16(val) ((uint16_t) (((val) << 8) | ((val) >> 8)))

/**
 * @brief Swaps the endianess of a 32 bit value
 * @param val the value to swap
 * @return The swapped value
 */
#define EndianSwap32(val)                                                                                   \
    ((uint32_t) ((((val) >> 24) & 0x000000FF) | (((val) >> 8) & 0x0000FF00) | (((val) << 8) & 0x00FF0000) | \
                 (((val) << 24) & 0xFF000000)))

/**
 * @brief Swaps the endianess of a 64 bit value
 * @param val the value to swap
 * @return The swapped value
 */
#define EndianSwap64(val)                                                          \
    ((((val) << 56) & 0xFF00000000000000) | (((val) << 40) & 0x00FF000000000000) | \
     (((val) << 24) & 0x0000FF0000000000) | (((val) << 8) & 0x000000FF00000000) |  \
     (((val) >> 8) & 0x00000000FF000000) | (((val) >> 24) & 0x0000000000FF0000) |  \
     (((val) >> 40) & 0x000000000000FF00) | ((val) >> 56) & 0x00000000000000FF)

#endif
