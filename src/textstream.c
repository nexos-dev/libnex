/*
    textstream.c - contains functions to work with text files
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

/// @file textstream.c

#include "codepages/win1252toUtf32.h"
#include "internal.h"
#include <assert.h>
#include <errno.h>
#include <libnex/base.h>
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
#define TEXT_DEFAULT_BUFSZ 4096    // Staging buffer has a default size of 4 KiB

// Reads in a new frame if needed. Returns error code or TEXT_SUCCESS
static short _textReadFrameMaybe (TextStream_t* stream, bool* isEof)
{
    assert (stream && isEof);
    assert (stream->mode == TEXT_MODE_READ);
    // Check if we need to read in a new frame
    if (stream->bufPos == stream->bufSize)
    {
        // Read it in
        size_t bytesRead = fread (stream->buf, 1, stream->bufSize, stream->file);
        // Check if we are at the end of the file
        if (feof (stream->file) && !bytesRead)
        {
            // Report EOF. Note that EOF may come when we aren't finished parsing yet.
            // For this reason, onlt report EOF when bytesRead == 0, and we truly are finished
            *isEof = true;
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
static short _textWriteFrameMaybe (TextStream_t* stream, bool force)
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

// Macro to help checking for a newline
#define CHECK_NEWLINE            \
    if (stopOnLine)              \
    {                            \
        if (buf[i] == '\n')      \
            break;               \
        else if (buf[i] == '\r') \
        {                        \
            buf[i] = '\n';       \
            foundCr = true;      \
        }                        \
    }

// Macro to help reading in a buffer
#define READ_BUFFER                             \
    res = _textReadFrameMaybe (stream, &isEof); \
    if (res != TEXT_SUCCESS)                    \
        return res;                             \
    if (isEof)                                  \
        goto end;                               \
    assert (foundCr ? stopOnLine : true);

// Macro to help writing a buffer
#define WRITE_BUFFER                            \
    res = _textWriteFrameMaybe (stream, false); \
    if (res != TEXT_SUCCESS)                    \
        return res;

// Decodes count characters of text
static short _textDecode (TextStream_t* stream,
                          char32_t* buf,
                          const size_t count,
                          size_t* charsRead,
                          bool stopOnLine)
{
    assert (stream && buf);

    bool isEof = false;
    bool foundCr = false;
    size_t charsParsed = 0;
    short res = TEXT_SUCCESS;
    int i = 0;
    // Decide what encoding we are decoding
    if (stream->encoding == TEXT_ENC_ASCII)
    {
        // This is easy. Loop through stream->buf, casting.
        for (; i < (count - 1); ++i)
        {
            // Maybe read the buffer
            READ_BUFFER
            // If we are simply skipping an LF, don't copy out a character
            if (!foundCr)
                buf[i] = (char32_t) stream->buf[stream->bufPos];
            // If we are looking for a LF, only advance if an LF is found
            if (!foundCr || (foundCr && stream->buf[stream->bufPos] == '\n'))
            {
                ++stream->bufPos;
                ++charsParsed;
            }
            // Check if we should stop on newline
            CHECK_NEWLINE
        }
    }
    else if (stream->encoding == TEXT_ENC_WIN1252)
    {
        // Loop through the buffer
        for (; i < (count - 1); ++i)
        {
            READ_BUFFER
            // If we are simply skipping an LF, don't copy out a character
            if (!foundCr)
            {
                // Here is the algorithm for decoding: If stream->buf[stream->bufPos] doesn't have bit 7 set,
                // or it is an ISO-8859-1 character, we copy it directly.
                // Else, we go into the Windows 1252 to Unicode decoding table
                // to get the right character
                if (BitGet (stream->buf[stream->bufPos], 7) == 0 || stream->buf[stream->bufPos] >= 0xA0)
                {
                    // This character lies in the ASCII or ISO-8859-1 realm
                    buf[i] = (char32_t) stream->buf[stream->bufPos];
                }
                else
                {
                    // Translate from table
                    buf[i] = win1252toUtf32[BitClearNew (stream->buf[stream->bufPos], 7)];
                }
            }
            // If we are looking for a LF, only advance if an LF is found
            if (!foundCr || (foundCr && stream->buf[stream->bufPos] == '\n'))
            {
                ++stream->bufPos;
                ++charsParsed;
            }
            CHECK_NEWLINE
        }
    }
    else if (stream->encoding == TEXT_ENC_UTF32)
    {
        for (; i < (count - 1); ++i)
        {
            // Maybe read the buffer
            READ_BUFFER
            // If we are simply skipping an LF, don't copy out a character
            if (!foundCr)
                buf[i] = EndianRead32 ((char32_t*) (stream->buf + stream->bufPos), stream->order);
            // If we are looking for a LF, only advance if an LF is found
            if (!foundCr ||
                (foundCr && (EndianRead32 ((char32_t*) (stream->buf + stream->bufPos), stream->order) == '\n')))
            {
                stream->bufPos += 4;
                ++charsParsed;
            }
            // Check if we should stop on newline
            CHECK_NEWLINE
        }
    }
    else if (stream->encoding == TEXT_ENC_UTF16)
    {
        for (; i < (count - 1); ++i)
        {
            READ_BUFFER
            if (!foundCr)
            {
                // FIXME: If stream->buf's last 16 bit element is a surrogate pair,
                // then this will return TEXT_INVALID_CHAR (as it would be an incomplete pair)
                ssize_t u16sParsed = (ssize_t) UnicodeDecode16 (&buf[i],
                                                                ((uint16_t*) (stream->buf + stream->bufPos)),
                                                                stream->bufSize - stream->bufPos,
                                                                stream->order);
                if (u16sParsed < 0)
                    return TEXT_INVALID_CHAR;
                stream->bufPos += (u16sParsed * 2);
                ++charsParsed;
            }
            if (foundCr && (EndianRead16 ((uint16_t*) (stream->buf + stream->bufPos), stream->order) == '\n'))
            {
                stream->bufPos += 2;
                ++charsParsed;
            }
            CHECK_NEWLINE
        }
    }
    else if (stream->encoding == TEXT_ENC_UTF8)
    {
        for (; i < (count - 1); ++i)
        {
            if (!foundCr)
            {
                // Decode part by part
                Utf8State_t state;
                UnicodeStateInit (state);
                while (!UnicodeIsAccepted (state))
                {
                    // Read in a buffer if needed
                    READ_BUFFER
                    size_t u8sparsed = UnicodeDecodePart8 (&buf[i], stream->buf[stream->bufPos], &state);
                    if (u8sparsed == 0)
                        return TEXT_INVALID_CHAR;
                    stream->bufPos += u8sparsed;
                }
                ++charsParsed;
            }
            if (foundCr && stream->buf[stream->bufPos] == '\n')
            {
                ++stream->bufPos;
                ++charsParsed;
            }
            CHECK_NEWLINE
        }
    }
end:
    buf[i] = 0;
    if (charsRead)
        *charsRead = charsParsed;
    return res;
}

static short _textEncode (TextStream_t* stream, const char32_t* buf, const size_t count, size_t* charsWritten)
{
    assert (stream && buf);
    short res = TEXT_SUCCESS;
    size_t charsEncoded = 0;
    // Decide to encoding to encode buf in
    if (stream->encoding == TEXT_ENC_ASCII)
    {
        // Just copy it out in a loop, taking buffering into account
        for (int i = 0; i < count; ++i)
        {
            if (buf[i] > SCHAR_MAX)
                return TEXT_INVALID_CHAR;
            stream->buf[stream->bufPos] = (uint8_t) buf[i];
            ++charsEncoded;
            ++stream->bufPos;
            WRITE_BUFFER
        }
    }
    else if (stream->encoding == TEXT_ENC_WIN1252)
    {
        // Loop through buffer
        for (int i = 0; i < count; ++i)
        {
            res = _textWriteFrameMaybe (stream, false);
            if (res != TEXT_SUCCESS)
                return res;
            // Check if this character's Unicode code
            // is the same as its Windows-1252 one. If it is, directly copy to destination
            // buffer
            if (buf[i] <= 0x7F || (buf[i] >= 0xA0 && buf[i] <= 0xFF))
            {
                // Copy out
                stream->buf[stream->bufPos] = (uint8_t) buf[i];
            }
            // It's a Windows-1252 character
            else
            {
                // This is kind of slow, but the best way overall.
                // We loop through the translation table until we find character that matches
                // buf[i]. We set bit 7 on the index, and that's the character
                const int tableSize = ARRAY_SIZE (win1252toUtf32);
                int tableIndex = 0;
                int charFound = 0;
                while (tableIndex < tableSize)
                {
                    // Check for a match
                    if (win1252toUtf32[tableIndex] == buf[i])
                    {
                        // Set bit 7 on tableIndex, and that is the character
                        stream->buf[stream->bufPos] = BitSetNew (tableIndex, 7);
                        charFound = 1;
                        break;
                    }
                    ++tableIndex;
                }
                if (!charFound)
                    return TEXT_INVALID_CHAR;
            }
            ++charsEncoded;
            ++stream->bufPos;
            WRITE_BUFFER
        }
    }
    else if (stream->encoding == TEXT_ENC_UTF32)
    {
        // Copy out
        for (int i = 0; i < count; ++i)
        {
            EndianWrite32 ((char32_t*) (stream->buf + stream->bufPos), buf[i], stream->order);
            ++charsEncoded;
            stream->bufPos += 4;
            WRITE_BUFFER
        }
    }
    else if (stream->encoding == TEXT_ENC_UTF16)
    {
        // Copy out, encoding it
        for (int i = 0; i < count; ++i)
        {
            size_t u16sEncoded =
                UnicodeEncode16 ((uint16_t*) (stream->buf + stream->bufPos), buf[i], stream->order);
            ++charsEncoded;
            stream->bufPos += (u16sEncoded * 2);
            WRITE_BUFFER
        }
    }
    else if (stream->encoding == TEXT_ENC_UTF8)
    {
        // Copy out, encoding everything
        for (int i = 0; i < count; ++i)
        {
            size_t u8sEncoded =
                UnicodeEncode8 (stream->buf + stream->bufPos, buf[i], stream->bufSize - stream->bufPos);
            if (u8sEncoded == 0)
                return TEXT_INVALID_CHAR;
            stream->bufPos += u8sEncoded;
            ++charsEncoded;
            WRITE_BUFFER
        }
    }
    return res;
}

PUBLIC short TextRead (TextStream_t* stream, char32_t* buf, const size_t count, size_t* charsRead)
{
    if (!stream || !buf)
        return TEXT_INVALID_PARAMETER;
    TextLock (stream);
    // Decode file contents
    short res = _textDecode (stream, buf, count, charsRead, false);
    TextUnlock (stream);
    return res;
}

PUBLIC short TextReadLine (TextStream_t* stream, char32_t* buf, const size_t count, size_t* charsRead)
{
    if (!stream || !buf)
        return TEXT_INVALID_PARAMETER;
    TextLock (stream);
    // Decode contents of frame
    short res = _textDecode (stream, buf, count, charsRead, true);
    TextUnlock (stream);
    return res;
}

PUBLIC short TextWrite (TextStream_t* stream, const char32_t* buf, const size_t count, size_t* charsWritten)
{
    if (!stream || !buf)
        return TEXT_INVALID_PARAMETER;
    TextLock (stream);
    // Encode into buffer
    short res = _textEncode (stream, buf, count, charsWritten);
    TextUnlock (stream);
    return res;
}

PUBLIC short TextOpen (const char* file, TextStream_t** out, char mode, char encoding, bool hasBom, char order)
{
    // Allocate the new stream
    TextStream_t* stream = (TextStream_t*) malloc_s (sizeof (TextStream_t));
    // Allocate the staging buffer
    stream->buf = (uint8_t*) malloc_s (TEXT_DEFAULT_BUFSZ);
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
        return TEXT_INVALID_PARAMETER;
    stream->mode = mode;
    // Open the file
    stream->file = fopen (file, fopenMode);
    if (!stream->file)
        return TEXT_SYS_ERROR;
    // If encoding is 0, then chances are, file is in an unsupported format.
    // The reason for this is because if we use libchardet, and TextGetEncId sees that
    // libchardet found an encoding that we don't support, it will return 0. Then, when the user passes
    // that ID, we will see that here
    if (!encoding)
        return TEXT_INVALID_ENC;
    // Set the encoding
    stream->encoding = encoding;
    // Check if there is a BOM, and if there is, set the byte order based on that
    if (hasBom && (mode == TEXT_MODE_READ || mode == TEXT_MODE_APPEND))
    {
        if (encoding == TEXT_ENC_UTF16)
        {
            // Read in the BOM
            uint8_t bom[2];
            fread (bom, 2, 1, stream->file);
            // Check for a UTF-16 BOM in big endian order
            if (bom[0] == 0xFE && bom[1] == 0xFF)
                stream->order = TEXT_ORDER_BE;
            // Check in little endian order
            else if (bom[0] == 0xFF && bom[1] == 0xFE)
                stream->order = TEXT_ORDER_LE;
            else
                return TEXT_BAD_BOM;
        }
        else if (encoding == TEXT_ENC_UTF8)
        {
            uint8_t bom[3];
            fread (bom, 3, 1, stream->file);
            // Check for its validity
            if (bom[0] == 0xEF && bom[1] == 0xBB && bom[2] == 0xBF)
                ;
            else
                return TEXT_BAD_BOM;
        }
        else if (encoding == TEXT_ENC_UTF32)
        {
            // Read in the BOM
            uint8_t bom[4];
            fread (bom, 4, 1, stream->file);
            // Check if UTF-32 BOM in big endian order
            if (bom[0] == 0x00 && bom[1] == 0x00 && bom[2] == 0xFE && bom[3] == 0xFF)
                stream->order = TEXT_ORDER_BE;
            else if (bom[0] == 0xFF && bom[1] == 0xFE && bom[2] == 0x00 && bom[3] == 0x00)
                stream->order = TEXT_ORDER_LE;
            else
                return TEXT_BAD_BOM;
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
        return TEXT_INVALID_PARAMETER;

    // Finally, create the object
    ObjCreate (file, &stream->obj);
    // Check if we need to write out a BOM
    if (mode == TEXT_MODE_WRITE)
    {
        if (encoding == TEXT_ENC_UTF16)
        {
            uint8_t bom[2];
            // Write a little endian BOM
            if (order == TEXT_ORDER_LE)
            {
                bom[0] = 0xFF;
                bom[1] = 0xFE;
            }
            // Write a big endian BOM
            else if (order == TEXT_ORDER_BE)
            {
                bom[0] = 0xFE;
                bom[1] = 0xFF;
            }
            else
                return TEXT_INVALID_PARAMETER;
            // Write it out
            fwrite (bom, 1, 2, stream->file);
        }
        else if (encoding == TEXT_ENC_UTF32)
        {
            uint8_t bom[4];
            // Check what order the BOM should be
            if (order == TEXT_ORDER_LE)
            {
                bom[0] = 0xFF;
                bom[1] = 0xFE;
                bom[2] = 0x00;
                bom[3] = 0x00;
            }
            else if (order == TEXT_ORDER_BE)
            {
                bom[0] = 0x00;
                bom[1] = 0x00;
                bom[2] = 0xFE;
                bom[3] = 0xFF;
            }
            else
                return TEXT_INVALID_PARAMETER;
            // Write it out
            fwrite (bom, 1, 4, stream->file);
        }
        stream->bufPos = 0;
    }
    else if (mode == TEXT_MODE_READ)
    {
        // Set up frame buffer. We set it equal to the max size so _textReadFrame knows
        // to read in a buffer
        stream->bufPos = stream->bufSize;
    }
    if (!out)
        return TEXT_INVALID_PARAMETER;
    *out = stream;
    stream->fileName = file;
    return TEXT_SUCCESS;
}

PUBLIC short TextFlush (TextStream_t* stream)
{
    TextLock (stream);
    short res = _textWriteFrameMaybe (stream, true);
    TextUnlock (stream);
    return res;
}

PUBLIC short TextClose (TextStream_t* stream)
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
        TextUnlock (stream);
    }
    return res;
}

PUBLIC long TextSize (TextStream_t* stream)
{
    // Lock it
    TextLock (stream);
    stat_t st;
    if (stat (stream->fileName, &st) == -1)
        return -1;
    TextUnlock (stream);
    return st.st_size;
}

PUBLIC void TextGetEncId (const char* encName, char* enc, char* order)
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
    else if (!strcmp (encName, "windows-1252"))
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

// Error condition string
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

PUBLIC const char* TextError (int code)
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
