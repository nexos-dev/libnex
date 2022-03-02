/*
    textstream.h - contains declarations to work with text files
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

/// @file textstream.h

#ifndef _TEXTSTREAM_H
#define _TEXTSTREAM_H

#include <libnex/char32.h>
#include <libnex/decls.h>
#include <libnex/object.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>

// Valid text encodings
#define TEXT_ENC_ASCII   1    ///< File is encoded in tradit
#define TEXT_ENC_WIN1252 2    ///< File is encoded Windows codepage 1252 character set
#define TEXT_ENC_UTF8    3    ///< File is encoded in UTF-8
#define TEXT_ENC_UTF16   4    ///< File is encoded in UTF-16
#define TEXT_ENC_UTF32   5    ///< File is encoded in UTF-32

// Valid orderings
#define TEXT_ORDER_NONE 0    ///< No ordering. This is used for single byte sets (like ASCII)
#define TEXT_ORDER_LE   1    ///< File is little endian
#define TEXT_ORDER_BE   2    ///< File is big endian

// Modes for textOpen
#define TEXT_MODE_READ   0    ///< File will be opened solely for reading
#define TEXT_MODE_WRITE  1    ///< File will be created (or truncating), and writing solely allowed
#define TEXT_MODE_APPEND 2    ///< File will be appended to

// Valid errors that can occur
#define TEXT_SUCCESS           1    ///< No error
#define TEXT_SYS_ERROR         2    ///< errno contains the error
#define TEXT_INVALID_PARAMETER 3    ///< User passed an invalid parameter
#define TEXT_BAD_BOM           4    ///< A bad BOM was encountered
#define TEXT_INVALID_CHAR      6    ///< Character doesn't fit in destination character set
#define TEXT_BUF_TOO_SMALL     7    ///< Character won't fit in buffer
#define TEXT_INVALID_ENC       8    ///< Encoding not supported

__DECL_START

/**
 * @brief Describes a file stream
 *
 * TextStream_t is an opaque data type used by the program to represent a text
 * file It contains things such as the staging buffer, C FILE object, and
 * various other metadata items
 */
typedef struct _TextStream
{
    Object_t obj;       ///< The object for this stream
    FILE* file;         ///< Pointer to underlying file object
    char* fileName;     ///< The original file name
    uint8_t* buf;       ///< Buffer to use for staging
    size_t bufSize;     ///< Size of above buffer (defaults to 512 bytes)
    char encoding;      ///< Underlying encoding of the stream
    char order;         ///< Order of bytes for multi byte character sets
    char maxEncSize;    ///< Max size of one char in the encoding
    char minEncSize;    ///< Minimum size of one char in the encoding
} TextStream_t;

/**
 * @brief Opens a up a text stream
 *
 * TextOpen takes a file name and a mode, and creates a new buffer according to
 * the mode that was requested
 *
 * @param[in] name specifies the file name to open
 * @param[in] mode the opening mode
 * @param[in] hasBom if the specified stream has a BOM. Only used if mode is append or read
 * @param[in] order the byte order of the stream, either TEXT_ORDER_LE of TEXT_ORDER_BE. Only used if
 * mode is write
 * @param[out] stream result variable to put the stream in
 * @return TEXT_SUCCESS, otherwise, an error code
 */
PUBLIC short TextOpen (char* file, TextStream_t** stream, char mode, char encoding, char hasBom, char order);

/**
 * @brief Closes a text stream
 *
 * TextClose takes a textStream and closes it, freeing all associated data about
 * it
 *
 * @param[in] stream the stream to close
 */
PUBLIC void TextClose (TextStream_t* stream);

/**
 * @brief Reads data from a text stream
 *
 * TextRead takes a buffer, character count, and stream object,
 * and reads / decodes count codepoints into buf from stream.
 * Data is intially read into a staging buffer, and then the staging buffer is
 * decoded into the main buffer specified by buf.
 *
 * @param[in] stream the stream to read from
 * @param[out] buf a buffer of char32_t's to decode into
 * @param[in] count the number of char32_t's to decode
 * @param[out] charsRead the number or characters read
 * @return a result code
 */
PUBLIC short TextRead (TextStream_t* stream, char32_t* buf, const size_t count, size_t* charsRead);

/**
 * @brief Reads data from a text stream
 *
 * TextReadLine takes a buffer, buffer size, and stream object,
 * and reads / decodes one line of codepoints into buf from stream.
 * It is garunteed that no more then count codepoints will be read, however
 * Data is intially read into a staging buffer, and then the staging buffer is
 * decoded into the main buffer specified by buf
 *
 * @param[in] stream the stream to read from
 * @param[out] buf a buffer of char32_t's to decode into
 * @param[in] count the max number of char32_t's to decode
 * @param[out] charsRead the number or characters read
 * @return a status code
 */
PUBLIC short TextReadLine (TextStream_t* stream, char32_t* buf, const size_t count, size_t* charsRead);

/**
 * @brief Writes data into a text stream
 *
 * TextWrite writes out a buffer of char32_t's to a file, encoding them first.
 * Data is encoded into a staging buffer, and then written to stream
 *
 * @param[in] stream the stream to write to
 * @param[in] buf the buffer to write from
 * @param[in] count the number of char32_t's to write
 * @param[out] charsWritten the number or characters written
 * @return a status code
 */
PUBLIC short TextWrite (TextStream_t* stream, const char32_t* buf, const size_t count, size_t* charsWritten);

/**
 * @brief Gets size of text stream
 *
 * TextSize obtains the size of the text stream specified by stream
 *
 * @param[in] stream the stream to get the size from
 * @return The size. -1 on error
 */
PUBLIC long TextSize (TextStream_t* stream);

/**
 * @brief Sets the size of the staging buffer
 *
 * TextSetBufSz resizes the staging buffer to the size specified by sz
 * This could used to help optimize performance by selecting a large buffer
 * size, or optimizing memory usage by selecting a small buffer size This
 * function has a potential exit point. It may call exit(3) in an error
 * condition
 *
 * @param[in] stream the stream to operate on
 * @param[in] sz the new buffer size
 */
PUBLIC void TextSetBufSz (TextStream_t* stream, size_t sz);

/**
 * @brief Returns a textual representation of a textstream error code
 * @param code the error code turn into a string
 * @return the string message
 */
PUBLIC const char* TextError (int code);

/**
 * @brief Takes a libchardet charset name, and returns a textsream encoding and byte order
 * @param encName the name of the encoding
 * @param enc the numeric encoding ID to write to
 * @param order the byte order to write to
 */
PUBLIC void TextGetEncId (const char* encName, char* enc, char* order);

__DECL_END

// Helper macros
#define TextGetEncoding(stream) ((stream)->encoding)    ///< Grabs the character encoding of stream
#define TextGetOrder(stream)    ((stream)->order)       ///< Grabs the byte order of stream
#define TextGetBufSz(stream)    ((stream)->bufSize)     ///< Grabs the size of the staging buffer in stream
#define TextRef(item)           ((TextStream_t*) ObjRef (&(item)->obj))    ///< References the underlying the object
#define TextLock(item)          (ObjLock (&(item)->obj))                   ///< Locks this stream
#define TextUnlock(item)        (ObjUnlock (&(item)->obj))                 ///< Unlocks the stream
#define TextDeRef(item)         (ObjDestroy (&(item)->obj))                ///< Dereferences this stream
#define TextGetFile(stream)     ((stream)->file)                           ///< Obtains the file handle of this stream

#endif
