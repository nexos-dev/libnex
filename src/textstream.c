/*
    textstream.c - contains functions to work with text files
    Copyright 2022 - 2025 The NexNix Project

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

/// @file textstream.c

#include "codepages/win1252toUtf8.h"
#include "internal.h"
#include <assert.h>
#include <errno.h>
#include <libnex/bits.h>
#include <libnex/endian.h>
#include <libnex/safemalloc.h>
#include <libnex/textstream.h>
#include <libnex/unicode.h>
#include <limits.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

// Helper macros
#define TEXT_DEFAULT_BUFSZ 1024    // Staging buffer has a default size of 4 KiB

// Reads in a new frame if needed. Returns error code or TEXT_SUCCESS
static inline short _textReadFrameMaybe (TextStream_t* stream)
{
    assert (stream);
    assert (stream->mode == TEXT_MODE_READ);
    // Check if we need to read in a new frame
    if (stream->bufPos >= stream->bufSize)
    {
        // Read it in
        size_t bytesRead = fread (stream->buf, 1, stream->bufSize, stream->file);
        // Check if we are at the end of the file
        if (feof (stream->file) && !bytesRead)
        {
            // Report EOF. Note that EOF may come when we aren't finished parsing yet.
            // For this reason, onlt report EOF when bytesRead == 0, and we truly are finished
            stream->isEof = true;
            return TEXT_SUCCESS;
        }
        // Check for failure
        if (bytesRead == 0)
            return TEXT_SYS_ERROR;
        // Ensure the buffer size is limited to the number of bytes read
        stream->bufSize = bytesRead;
        stream->bufPos = 0;
    }
    return TEXT_SUCCESS;
}

// Writes out the current frame if needed
static inline short _textWriteFrameMaybe (TextStream_t* stream, bool force)
{
    assert (stream);
    assert (stream->mode != TEXT_MODE_READ);
    // Check if we need to write out the current frame
    if ((stream->bufPos == stream->bufSize) || force)
    {
        // Write it out
        if (fwrite (stream->buf, 1, stream->bufPos, stream->file) < stream->bufPos)
            return TEXT_SYS_ERROR;
        // Start a new buffer
        stream->bufPos = 0;
    }
    return TEXT_SUCCESS;
}

// Decodes count characters of text
static short _textDecode (TextStream_t* stream, char* buf, size_t count, size_t* charsRead, bool stopOnLine)
{
    assert (stream && buf);
    bool foundCr = false;
    size_t charsParsed = 0;
    short res = TEXT_SUCCESS;
    size_t charSz = 0;
    int i = 0;
    // Loop through buffer
    for (; i < (count - 1); i += charSz)
    {
        // Maybe read the buffer
        res = _textReadFrameMaybe (stream);
        if (res != TEXT_SUCCESS)
            return res;
        // If we hit EOF, end decoding
        if (stream->isEof)
            break;
        if (stream->encoding == TEXT_ENC_ASCII)
        {
            // Get character
            char c = stream->buf[stream->bufPos++];
            // Handle a CRLF combo
            if (stream->skipLf)
            {
                stream->skipLf = false;
                if (c == '\n')
                {
                    charSz = 0;    // Dont change i
                    continue;
                }
            }
            buf[i] = c;    // Copy out
            charSz = 1;    // ASCII is always one byte sized
        }
        else if (stream->encoding == TEXT_ENC_WIN1252)
        {
            // Here is the algorithm for decoding: If current char doesn't have bit 7 set,
            // or it is an ISO-8859-1 character, we copy it directly.
            // Else, we go into the Windows 1252 to Unicode decoding table
            // to get the right character
            if (BitGet (stream->buf[stream->bufPos], 7) == 0)
            {
                // This character lies in the ASCII or ISO-8859-1 realm
                char c = stream->buf[stream->bufPos++];
                // Check if we need to skip an LF
                if (c == '\n' && stream->skipLf)
                {
                    charSz = 0;
                    continue;
                }
                buf[i] = c;
                charSz = 1;
            }
            else if (stream->buf[stream->bufPos] >= 0xA0)
            {
                // Convert to UTF-8// This character lies in the ASCII or ISO-8859-1 realm
                char c = stream->buf[stream->bufPos++];
                charSz = UnicodeEncode8 (&buf[i], c, 4);
            }
            else
            {
                // This is a windows-1252 character, meaning we need to reference the table
                char* c = win1252ToUtf8[BitClearNew (stream->buf[stream->bufPos++], 7)];
                // Copy to buffer
                memcpy (&buf[i], c, strlen (c));
                charSz = strlen (c);
            }
            // Reset skipLf
            if (stream->skipLf)
                stream->skipLf = false;
        }
        else if (stream->encoding == TEXT_ENC_UTF32)
        {
            // Get a character
            char32_t* ptr = (char32_t*) (stream->buf + stream->bufPos);
            stream->bufPos += 4;    // To next character
            char32_t c = EndianRead32 (ptr, stream->order);
            // Handle newline
            if (stream->skipLf)
            {
                stream->skipLf = false;
                if (c == U'\n')
                {
                    charSz = 0;    // Dont change i
                    continue;
                }
            }
            // Add to buffer
            charSz = UnicodeEncode8 (&buf[i], *ptr, 4);
        }
        else if (stream->encoding == TEXT_ENC_UTF16)
        {
            // Get character in UTF-32
            char32_t c32 = 0;
            ssize_t u16sParsed = (ssize_t) UnicodeDecode16 (&c32,
                                                            ((uint16_t*) (stream->buf + stream->bufPos)),
                                                            stream->bufSize - stream->bufPos,
                                                            stream->order);
            if (u16sParsed == 0)
                return TEXT_INVALID_CHAR;
            // Update buffer
            stream->bufPos += (u16sParsed * 2);
            // Handle newline
            if (stream->skipLf)
            {
                stream->skipLf = false;
                if (c32 == U'\n')
                {
                    charSz = 0;    // Dont change i
                    continue;
                }
            }
            // Convert to UTF-8
            charSz = UnicodeEncode8 (&buf[i], c32, 4);
        }
        else if (stream->encoding == TEXT_ENC_UTF8)
        {
            uint8_t* c = stream->buf + stream->bufPos;
            // Get length of c
            size_t len = UnicodeGetCharLen8 (c);
            if (!len)
                return TEXT_INVALID_CHAR;
            stream->bufPos += len;
            charSz = len;
            // Handle newline
            if (stream->skipLf)
            {
                stream->skipLf = false;
                if (*c == '\n')
                {
                    charSz = 0;    // Dont change i
                    continue;
                }
            }
            // Copy it out
            memcpy (&buf[i], c, len);
        }
        else
            assert (false);
        // Handle CR
        if (buf[i] == '\r')
        {
            stream->skipLf = true;
            buf[i] = '\n';
        }
        ++charsParsed;
        if (buf[i] == '\n' && stopOnLine)
        {
            ++i;    // Move i passed the newline
            break;
        }
    }
    buf[i] = 0;
    if (charsRead)
        *charsRead = charsParsed;
    return res;
}

static short _textEncode (TextStream_t* stream, const char* buf, size_t count, size_t* charsWritten)
{
    assert (stream && buf);
    short res = TEXT_SUCCESS;
    size_t charsEncoded = 0;
    size_t charSz = 0;
    for (int i = 0; i < count; i += charSz)
    {
        if (stream->encoding == TEXT_ENC_ASCII)
        {
            stream->buf[stream->bufPos++] = buf[i];
            charSz = 1;
        }
        else if (stream->encoding == TEXT_ENC_WIN1252)
        {
            // Convert to UTF-32
            char32_t c = 0;
            charSz = UnicodeDecode8 (&c, &buf[i], 4);
            if (c <= 0x7F || (c >= 0x80 && c <= 0xFF))
            {
                stream->buf[stream->bufPos++] = (char) c;
            }
            else
            {
                // Reference table
                // This is kind of slow, but the best way overall.
                // We loop through the translation table until we find character that matches
                // buf[i]. We set bit 7 on the index, and that's the character
                const int tableSize = ARRAY_SIZE (win1252ToUtf8);
                int tableIdx = 0;
                bool charFound = 0;
                while (tableIdx < tableSize)
                {
                    if (!memcmp (&win1252ToUtf8[tableIdx], &buf[i], strlen (win1252ToUtf8[tableIdx])))
                    {
                        // Set bit 7 on tableIndex, and that is the character
                        stream->buf[stream->bufPos++] = BitSetNew (tableIdx, 7);
                        charFound = true;
                        break;
                    }
                    ++tableIdx;
                }
                if (!charFound)
                    return TEXT_INVALID_CHAR;
            }
        }
        else if (stream->encoding == TEXT_ENC_UTF32)
        {
            // COnvert to UTF-32
            char32_t c = 0;
            charSz = UnicodeDecode8 (&c, &buf[i], 4);
            // Write it out
            uint32_t* ptr = (uint32_t*) (stream->buf + stream->bufPos);
            *ptr = c;
            stream->bufPos += 4;
        }
        else if (stream->encoding == TEXT_ENC_UTF16)
        {
            // Convert to UTF-32
            char32_t c = 0;
            charSz = UnicodeDecode8 (&c, &buf[i], 4);
            // Encode as UTF-16
            size_t u16sEncoded = UnicodeEncode16 ((uint16_t*) (stream->buf + stream->bufPos), c, stream->order);
            stream->bufPos += (u16sEncoded * 2);
        }
        else if (stream->encoding == TEXT_ENC_UTF8)
        {
            // Get length of current character
            charSz = UnicodeGetCharLen8 (&buf[i]);
            // Copy it out
            memcpy (stream->buf + stream->bufPos, &buf[i], charSz);
            stream->bufPos += charSz;
        }
        else
            assert (false);
        ++charsEncoded;
        // Check if we need to write out to file
        res = _textWriteFrameMaybe (stream, false);
        if (res != TEXT_SUCCESS)
            return res;
    }
    return res;
}

LIBNEX_PUBLIC short TextRead (TextStream_t* stream, char* buf, size_t count, size_t* charsRead)
{
    if (!stream || !buf)
        return TEXT_INVALID_PARAMETER;
    TextLock (stream);
    // Decode file contents
    short res = _textDecode (stream, buf, count, charsRead, false);
    TextUnlock (stream);
    return res;
}

LIBNEX_PUBLIC short TextReadChar (TextStream_t* stream, char* c)
{
    if (!stream || !c)
        return TEXT_INVALID_PARAMETER;
    char buf[2];
    short res = TextRead (stream, buf, 2, NULL);
    if (res != TEXT_SUCCESS)
        return res;
    *c = buf[0];
    return TEXT_SUCCESS;
}

LIBNEX_PUBLIC short TextReadLine (TextStream_t* stream, char* buf, size_t count, size_t* charsRead)
{
    if (!stream || !buf)
        return TEXT_INVALID_PARAMETER;
    TextLock (stream);
    // Decode contents of frame
    short res = _textDecode (stream, buf, count, charsRead, true);
    TextUnlock (stream);
    return res;
}

LIBNEX_PUBLIC short TextWrite (TextStream_t* stream, const char* buf, size_t count, size_t* charsWritten)
{
    if (!stream || !buf)
        return TEXT_INVALID_PARAMETER;
    TextLock (stream);
    // Encode into buffer
    short res = _textEncode (stream, buf, count, charsWritten);
    TextUnlock (stream);
    return res;
}

LIBNEX_PUBLIC short TextOpen (const char* file,
                              TextStream_t** out,
                              char mode,
                              char encoding,
                              bool hasBom,
                              char order)
{
    // Allocate the new stream
    TextStream_t* stream = (TextStream_t*) malloc (sizeof (TextStream_t));
    if (!stream)
        return TEXT_SYS_ERROR;
    // Allocate the staging buffer
    stream->buf = (uint8_t*) malloc (TEXT_DEFAULT_BUFSZ);
    if (!stream->buf)
    {
        free (stream);
        errno = ENOMEM;
        return TEXT_SYS_ERROR;
    }
    stream->bufSize = TEXT_DEFAULT_BUFSZ;
    // Figure out the mode
    char* fopenMode = NULL;
    if (mode == TEXT_MODE_READ)
        fopenMode = "r";
    else if (mode == TEXT_MODE_WRITE)
        fopenMode = "w";
    else if (mode == TEXT_MODE_APPEND)
        fopenMode = "a";
    else
    {
        free (stream);
        return TEXT_INVALID_PARAMETER;
    }
    stream->mode = mode;
    // Open the file
    stream->file = fopen (file, fopenMode);
    if (!stream->file)
    {
        free (stream);
        return TEXT_SYS_ERROR;
    }
    // If encoding is 0, then chances are, file is in an unsupported format.
    // The reason for this is because if we use libchardet, and TextGetEncId sees that
    // libchardet found an encoding that we don't support, it will return 0. Then, when the user passes
    // that ID, we will see that here
    if (!encoding)
    {
        (void) fclose (stream->file);
        free (stream);
        return TEXT_INVALID_ENC;
    }
    // Set the encoding
    stream->encoding = encoding;
    // Check if there is a BOM, and if there is, set the byte order based on that
    if (hasBom && (mode == TEXT_MODE_READ || mode == TEXT_MODE_APPEND))
    {
        if (encoding == TEXT_ENC_UTF16)
        {
            // Read in the BOM
            uint8_t bom[2];
            if (fread (bom, 2, 1, stream->file) != 1)
            {
                (void) fclose (stream->file);
                free (stream);
                return TEXT_SYS_ERROR;
            }
            stream->order = UnicodeReadBom16 (bom);
            if (stream->order == TEXT_ORDER_NONE)
            {
                fclose (stream->file);
                free (stream);
                return TEXT_BAD_BOM;
            }
        }
        else if (encoding == TEXT_ENC_UTF8)
        {
            uint8_t bom[3];
            if (fread (bom, 3, 1, stream->file) != 1)
            {
                (void) fclose (stream->file);
                free (stream);
                return TEXT_SYS_ERROR;
            }
            if (!UnicodeReadBom8 (bom))
            {
                (void) fclose (stream->file);
                free (stream);
                return TEXT_BAD_BOM;
            }
        }
        else if (encoding == TEXT_ENC_UTF32)
        {
            // Read in the BOM
            uint8_t bom[4];
            if (fread (bom, 4, 1, stream->file) != 1)
            {
                (void) fclose (stream->file);
                free (stream);
                return TEXT_SYS_ERROR;
            }
            stream->order = UnicodeReadBom32 (bom);
            if (stream->order == TEXT_ORDER_NONE)
            {
                (void) fclose (stream->file);
                free (stream);
                return TEXT_BAD_BOM;
            }
        }
    }
    else
    {
        // It we are creating a new file, set the order based on the parameter
        if (mode == TEXT_MODE_WRITE)
            stream->order = order;
        else
        {
            if (stream->encoding == TEXT_ENC_UTF16 || stream->encoding == TEXT_ENC_UTF32)
            {
                // According to RFC 2781, UTF-16 text (so presuambly UTF-32 as well)
                // should be treated as big endian in the absence of a BOM
                stream->order = TEXT_ORDER_BE;
            }
            // This is a single byte character set
            else
                stream->order = TEXT_ORDER_NONE;
        }
    }
    // Set size of encoding
    if (!(encoding == TEXT_ENC_ASCII || encoding == TEXT_ENC_WIN1252 || encoding == TEXT_ENC_UTF32 ||
          encoding == TEXT_ENC_UTF16 || encoding == TEXT_ENC_UTF8))
    {
        (void) fclose (stream->file);
        free (stream);
        return TEXT_INVALID_PARAMETER;
    }

    // Finally, create the object
    ObjCreate ("TextStream", &stream->obj);
    // Check if we need to write out a BOM
    if (mode == TEXT_MODE_WRITE)
    {
        if (order != TEXT_ORDER_BE && order != TEXT_ORDER_LE && order != TEXT_ORDER_NONE)
            return TEXT_INVALID_PARAMETER;
        if (encoding == TEXT_ENC_UTF16)
        {
            if (order == TEXT_ORDER_NONE)
                order = EndianHost();
            uint16_t bom;
            UnicodeWriteBom16 (&bom, order);
            // Write it out
            if (fwrite (&bom, 1, 2, stream->file) != 2)
            {
                (void) fclose (stream->file);
                free (stream);
                return TEXT_SYS_ERROR;
            }
        }
        else if (encoding == TEXT_ENC_UTF32)
        {
            if (order == TEXT_ORDER_NONE)
                order = EndianHost();
            uint32_t bom;
            UnicodeWriteBom32 (&bom, order);
            // Write it out
            if (fwrite (&bom, 1, 4, stream->file) != 4)
            {
                (void) fclose (stream->file);
                free (stream);
                return TEXT_SYS_ERROR;
            }
        }
        stream->bufPos = 0;
    }
    else if (mode == TEXT_MODE_READ)
    {
        // Set up frame buffer. We set it equal to the max size so _textReadFrame knows
        // to read in a buffer
        stream->bufPos = stream->bufSize;
    }
    stream->isEof = false;
    if (!out)
    {
        (void) fclose (stream->file);
        free (stream);
        return TEXT_INVALID_PARAMETER;
    }
    *out = stream;
    return TEXT_SUCCESS;
}

LIBNEX_PUBLIC short TextFlush (TextStream_t* stream)
{
    TextLock (stream);
    short res = _textWriteFrameMaybe (stream, true);
    TextUnlock (stream);
    return res;
}

LIBNEX_PUBLIC short TextClose (TextStream_t* stream)
{
    short res = TEXT_SUCCESS;
    // Ensure we can do this
    if (!TextDeRef (stream))
    {
        TextLock (stream);
        // Flush stream if in a write mode
        if (stream->mode != TEXT_MODE_READ)
        {
            res = _textWriteFrameMaybe (stream, true);
            if (res != TEXT_SUCCESS)
            {
                TextUnlock (stream);
                return res;
            }
        }
        // Free the buffer
        free (stream->buf);
        // Close the file
        if (fclose (stream->file))
            res = TEXT_SYS_ERROR;
        ObjDestroy (&stream->obj);
    }
    free (stream);
    return res;
}

LIBNEX_PUBLIC void TextGetEncId (const char* encName, char* enc, char* order)
{
    if (!strcmp (encName, "ASCII") || !strcmp (encName, "UTF-8"))
    {
        *enc = TEXT_ENC_UTF8;
        *order = TEXT_ORDER_NONE;
    }
    else if (!strcmp (encName, "UTF-16LE"))
    {
        *enc = TEXT_ENC_UTF16;
        *order = TEXT_ORDER_LE;
    }
    else if (!strcmp (encName, "UTF-16BE"))
    {
        *enc = TEXT_ENC_UTF16;
        *order = TEXT_ORDER_BE;
    }
    else if (!strcmp (encName, "UTF-32LE"))
    {
        *enc = TEXT_ENC_UTF32;
        *order = TEXT_ORDER_LE;
    }
    else if (!strcmp (encName, "UTF-32BE"))
    {
        *enc = TEXT_ENC_UTF32;
        *order = TEXT_ORDER_BE;
    }
    // Note: proper ISO-8859-3 support coming soon
    else if (!strcmp (encName, "windows-1252") || !strcmp (encName, "ISO-8859-3"))
    {
        *enc = TEXT_ENC_WIN1252;
        *order = TEXT_ORDER_NONE;
    }
    else
    {
        *enc = 0;
        *order = 0;
    }
}

// Error condition strings
static const char* errorStrings[] = {
    "",                          // 0 doesn't represent anything
    N_ ("No error"),             // TEXT_SUCCESS
    NULL,                        // TEXT_SYS_ERROR. This is NULL so TextError knows to call strerror(3) instead
    N_ ("Invalid parameter"),    // TEXT_INVALID_PARAMETER
    N_ ("Invalid byte order mark"),                        // TEXT_BAD_BOM
    N_ ("Character can't be encoded by character set"),    // TEXT_INVALID_CHAR
    N_ ("Result buffer too small"),                        // TEXT_BUF_TOO_SMALL
    N_ ("Unsupported character encoding")                  // TEXT_INVALID_ENC
};

LIBNEX_PUBLIC const char* TextError (int code)
{
    // Initialize text domain if needed
    __Libnex_i18n_init();
    // Bounds check
    if (code > ARRAY_SIZE (errorStrings))
        return _ (errorStrings[code]);

    // Check if this corresponds to a system error
    if (!errorStrings[code])
        return _ (errorStrings[code]);
    return _ (errorStrings[code]);
}
