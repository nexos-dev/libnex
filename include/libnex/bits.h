/*
    bits.h - contains macros to perform common bitwise operations
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

/// @file bits.h

#ifndef _BITS_H
#define _BITS_H

/**
 * @brief Sets a given bit in a bitset
 *
 * BitSetNew() sets a given bit in a bitset. It takes the literal number of the bit, and uses an OR operation to set it
 * @param set the value to set the bit in
 * @param bit the bit to set
 * @return The bitset with the set value
 */
#define BitSetNew(set, bit) ((set) | (1 << (bit)))

/**
 * @brief Sets a given bit in a bitset in place
 *
 * BitSet() sets a given bit in a bitset. It takes the literal number of the bit, and uses an OR operation to set it
 * @param set the value to set the bit in
 * @param bit the bit to set
 */
#define BitSet(set, bit) ((set) |= (1 << (bit)))

/**
 * @brief Clears a bit in a bitset
 *
 * BitClearNew() clears a given bit in a bit set. It returns a new bit set with the bit cleared
 * @param set the value to clear the bit it
 * @param bin the bit to clear
 * @return The set with the bit cleared
 */
#define BitClearNew(set, bit) ((set) & ~(1 << (bit)))

/**
 * @brief Clears a bit in a bitset
 *
 * BitClear() clears a given bit in a bit set in place
 * @param set the value to clear the bit it
 * @param bit the bit to clear
 */
#define BitClear(set, bit) ((set) &= ~(1 << (bit)))

/**
 * @brief Gets a bit's state in a bit set
 * @param set the bit set to work with
 * @param bit the bit to check
 * @return the bits's state
 */
#define BitGet(set, bit) (((set) & (1 << (bit))) >> (bit))

/**
 * @brief Sets a given range of bits in a bit set
 *
 * BitSetRange() accepts a range of bit sets to in a value. It sets them in place
 * @param set the value to set the bits in
 * @param start the first bit to set
 * @param count the number of bits to set
 */
#define BitSetRange(set, start, count) ((set) |= (((1 << (count)) - 1) << (start)))

/**
 * @brief Set a given range of bits in a bit set
 *
 * BitSetRangeNew() sets a range of bits in a bit set, returning a new set
 * @param set the the value to set bits in
 * @param start the first bit to set
 * @param count the number of bits to set
 * @return The new set
 */
#define BitSetRangeNew(set, start, count) ((set) | (((1 << (count)) - 1) << (start)))

/**
 * @brief Clears a range of bits in a bit set
 *
 * BitClearRange() clears a range of bits in a bit set in place
 * @param set the value to clear in
 * @param start the first bit to clear
 * @param count the number of bits to clear
 */
#define BitClearRange(set, start, count) ((set) &= ~(((1 << (count)) - 1) << (start)))

/**
 * @brief Clears a range of bits in a bit set
 *
 * BitClearRangeNew() clears a range of bits in a bit set, returning a new set
 * @param set the value to clear in
 * @param start the first bit to clear
 * @param count the number of bits to clear
 * @return the new bit set
 */
#define BitClearRangeNew(set, start, count) ((set) & ~(((1 << (count)) - 1) << (start)))

/**
 * @brief Gets a range of bits from a bit set
 *
 * BitGetRange() gets a range of bits in a bit set, returning those bits
 * @param set the set to get bits from
 * @param start the first bit to get
 * @param count the number of bits to get
 * @return The new set
 */
#define BitGetRange(set, start, count) (((set) >> (start)) & ((1 << (count)) - 1))

#endif
