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

#include <libnex/char32.h>
#include <libnex/decls.h>
#include <libnex/endian.h>
#include <libnex/libnex_config.h>
#include <stdbool.h>
#include <uchar.h>

__DECL_START

/**
 * @brief The state of parsing a given UTF-8 character
 * This data type is opaque
 */
typedef struct _utf8state
{
    uint8_t prevState;        ///< The previous state of the parser
    uint8_t state;            ///< The current state of the parser
    uint8_t bytesRequired;    ///< The number of bytes required for input
    uint8_t bytesLeft;        ///< The remaining number of bytes needed to finish processing
} Utf8State_t;

#define UnicodeIsAccepted(state)  ((state).state == 6)     ///< Checks if state has been accepted
#define UnicodeIsAcceptedP(state) ((state)->state == 6)    ///< Checks if state has been accepted
#define UnicodeStateInit(state)   ((state).state = 0)      ///< Initializes a state structure
#define UnicodeStateInitP(state)  ((state)->state = 0)     ///< Initiailzes a state structure by pointer

/**
 * @brief Decodes a UTF-16 character to UTF-32
 * @param out the character to write out the decoded data to
 * @param in a pointer to UTF-16 data to decode
 * @param sz the size of in
 * @param endian the endianess of the input buffer, either ENDIAN_LITTLE or ENDIAN_BIG
 * @return How many 16 bit values were decoded. If -1, then an invalid character was found
 */
LIBNEX_PUBLIC size_t UnicodeDecode16 (char32_t* out, const uint16_t* in, size_t sz, char endian);

/**
 * @brief Encodes a UTF-32 character as UTF-16
 * @param out the buffer to write out the 16 bit values to
 * @param in UTF-32 character to encode
 * @param endian the endianess of out
 * @return the number of 16 bit values encoded
 */
LIBNEX_PUBLIC size_t UnicodeEncode16 (uint16_t* out, char32_t in, char endian);

/**
 * @brief Decodes a one octet section of a larger UTF-8 sequence.
 * UnicodeDecodePart8 decodes one component of a sequence. It is useful
 * when you only have one byte at call time, such as when reading from the keyboard.
 * Before calling for the first time, ensure state is zeroed
 * Keep running until UnicodeIsAccepted is true for state.
 * @param out pointer to the resulting codepoint
 * @param in the current byte in the sequence
 * @param state pointer to structure maintaining the parser's state
 */
LIBNEX_PUBLIC size_t UnicodeDecodePart8 (char32_t* out, uint8_t in, Utf8State_t* state);

/**
 * @brief Decodes a UTF-8 character to UTF-32
 * @param out the buffer to write the character out to
 * @param in pointer to buffer containing sequence to convert
 * @param sz size of in
 */
LIBNEX_PUBLIC size_t UnicodeDecode8 (char32_t* out, const uint8_t* in, size_t sz);

/**
 * @brief Encodes a UTF-32 character as UTF-8
 * @param out the buffer to write the encoded UTF-8 out to
 * @param in character to encode
 * @param sz size of out
 */
LIBNEX_PUBLIC size_t UnicodeEncode8 (uint8_t* out, char32_t in, size_t sz);

/**
 * @brief Writes out a UTF-8 byte order mark (BOM)
 * NOTE: You typically shouldn't need to use a BOM on UTF-8. This function is provided only
 * for completeness sake only
 * @param buf the buffer to write the BOM to. It must be at least 3 bytes in size
 */
LIBNEX_PUBLIC void UnicodeWriteBom8 (uint8_t* buf);

/**
 * @brief Writes out a UTF-16 style BOM
 * @param out the buffer to write the BOM to. It must be at least 2 bytes in size
 * @param order the byte order you want the BOM in. If 0, it defaults to the host's order
 */
LIBNEX_PUBLIC void UnicodeWriteBom16 (uint16_t* out, char order);

/**
 * @brief Writes out a UTF-32 style BOM
 * @param out the buffer to write the BOM to. It must be at least 4 bytes in size
 * @param order the byte order you want the BOM in. If 0, it defaults to the host's order
 */
LIBNEX_PUBLIC void UnicodeWriteBom32 (uint32_t* out, char order);

/**
 * @brief Reads in a UTF-8 BOM, checking its validity
 * @param bom the BOM to check
 * @return true if one is found, false otherwise
 */
LIBNEX_PUBLIC bool UnicodeReadBom8 (const uint8_t* bom);

/**
 * @brief Reads in a UTF-16 BOM, checking its validity and order
 * @param bom the BOM to check
 * @return ENDIAN_LITTLE if BOM indicates little endian, ENDIAN_BIG if it indicates big endian, ENDIAN_NONE
 * otherwise
 */
LIBNEX_PUBLIC char UnicodeReadBom16 (const uint8_t* bom);

/**
 * @brief Reads in a UTF-32 BOM, checking its validity and order
 * @param bom the BOM to check
 * @return ENDIAN_LITTLE if BOM indicates little endian, ENDIAN_BIG if it indicates big endian, ENDIAN_NONE
 * otherwise
 */
LIBNEX_PUBLIC char UnicodeReadBom32 (const uint8_t* bom);

/**
 * @brief Converts UTF-32 string to host's multibyte format
 * NOTE: DO NOT pass returned string to free!
 * @param s string to convert
 * @return multibyte string. DO NOT pass to free. Returns NULL on error
 */
LIBNEX_PUBLIC char* UnicodeToHost (const char32_t* s);

__DECL_END

#endif
