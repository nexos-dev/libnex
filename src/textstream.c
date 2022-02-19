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
#include <errno.h>
#include <libnex/base.h>
#include <libnex/bits.h>
#include <libnex/endian.h>
#include <libnex/safemalloc.h>
#include <libnex/textstream.h>
#include <limits.h>
#include <stdlib.h>
#include <sys/stat.h>

// Helper macros
#define TEXT_DEFAULT_BUFSZ               4096    ///< Staging buffer has a default size of 4 KiB
#define TEXT_DECODE_TERMINATE_ON_NEWLINE 1       ///< Terminate decoding on newline discovery
#define TEXT_DECODE_ALL                  0       ///< Decode whole buffer

PUBLIC short TextOpen (char* file, TextStream_t** out, char mode, char encoding, char hasBom, char order)
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
    // Open the file
    stream->file = fopen (file, fopenMode);
    if (!stream->file)
        return TEXT_SYS_ERROR;
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
                // We make an assumption here. This file is UTF-16 or UTF-32,
                // and doesn't have a BOM. In either case, we assume this is little endian
                // If that assumption is false, we are in a world of hurting
                stream->order = TEXT_ORDER_LE;
            }
            // This is a single byte character set
            else
                stream->order = TEXT_ORDER_NONE;
        }
    }
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
    }
    *out = stream;
    stream->fileName = file;
    return TEXT_SUCCESS;
}

PUBLIC void TextClose (TextStream_t* stream)
{
    // Ensure we can do this
    if (!TextDeRef (stream))
    {
        TextLock (stream);
        // Free the buffer
        free (stream->buf);
        // Close the file
        fclose (stream->file);
        TextUnlock (stream);
    }
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

PUBLIC void TextSetBufSz (TextStream_t* stream, size_t sz)
{
    TextLock (stream);
    stream->bufSize = sz;
    // Resize it
    stream->buf = realloc_s (stream->buf, stream->bufSize);
    TextUnlock (stream);
}

// Checks if we reached a newline
static const char _textCheckNewLine8 (const uint8_t* buf, const int i, const size_t bytesLeft)
{
    if (buf[i] == '\n' || buf[i] == '\r')
    {
        // If this is CRLF, run the loop one more time so the '\n' can be copied
        if ((i + 1) < bytesLeft && buf[i + 1] == '\n')
            return 0;
        else
            return 1;
    }
    return 2;
}

static const char _textCheckNewLine32 (const uint32_t* buf, const int i, const size_t charsLeft)
{
    if (buf[i] == '\n' || buf[i] == '\r')
    {
        // If this is CRLF, run the loop one more time so the '\n' can be copied
        if ((i + 1) < charsLeft && buf[i + 1] == '\n')
            return 0;
        else
            return 1;
    }
    return 2;
}

// Internal function to decode characters
// terminator = 0 means to parse everything, 1 means parse until LF, CR, or CRLF is encountered
static ssize_t _textDecode (TextStream_t* stream, wchar_t* buf, const size_t count, const int terminator)
{
    ssize_t charParsed = 0;
    // Check what encoding is being used
    if (stream->encoding == TEXT_ENC_ASCII)
    {
        // Loop through the whole staging buffer, casting to wchar_t
        for (int i = 0; i < count; ++i)
        {
            buf[i] = (wchar_t) stream->buf[i];
            // Check if a terminator was reached
            if (terminator == 1)
            {
                char doWhat = _textCheckNewLine8 (stream->buf, i, count);
                if (doWhat == 0)
                    continue;
                else if (doWhat == 1)
                    return i;
            }
            ++charParsed;
        }
    }
    else if (stream->encoding == TEXT_ENC_WIN1252)
    {
        // Loop through staging buffer
        for (int i = 0; i < count; ++i)
        {
            // Here is the algorithm for decoding: If stream->buf[i] doesn't have bit 7 set,
            // it is like the ASCII version, or it is an ISO-8859-1 character, we copy it directly.
            // Else, we go into the Windows 1252 to Unicode decoding table
            // to get the right character
            if (BitGet (stream->buf[i], 7) == 0 || stream->buf[i] >= 0xA0)
            {
                // This character lies in the ASCII or ISO-8859-1 realm
                buf[i] = (wchar_t) stream->buf[i];
            }
            else
            {
                // Translate from table
                buf[i] = win1252toUtf32[BitClearNew (stream->buf[i], 7)];
            }
            // Check if a terminator was reached
            if (terminator == 1)
            {
                const char doWhat = _textCheckNewLine8 (stream->buf, i, count);
                if (doWhat == 0)
                    continue;
                else if (doWhat == 1)
                    return i;
            }
            ++charParsed;
        }
    }
    else if (stream->encoding == TEXT_ENC_UTF32)
    {
        // Ensure wchar_t is 4 bytes wide. Else, we don't support UTF-32
        if (sizeof (wchar_t) < 4)
            return -TEXT_NARROW_WCHAR;
        const uint32_t* wideBuf = (uint32_t*) stream->buf;
        for (int i = 0; i < count; ++i)
        {
            // Make sure we decode to host's byte order
            buf[i] = (wchar_t) EndianRead32 (&wideBuf[i], stream->order);
            // Check for a terminator
            if (terminator == 1)
            {
                const char doWhat = _textCheckNewLine32 (wideBuf, i, count);
                if (doWhat == 0)
                    continue;
                else if (doWhat == 1)
                    return i;
            }
            ++charParsed;
        }
    }
    return charParsed;
}

// Internal function to encode characters into staging buffer
// Returns a negative error code on error
static ssize_t _textEncode (TextStream_t* stream, const wchar_t* buf, const size_t count)
{
    ssize_t charEncoded = 0;
    // Check how to encode
    if (stream->encoding == TEXT_ENC_ASCII)
    {
        // Loop and encode everything
        for (int i = 0; i < count; ++i)
        {
            // Ensure buf[i] isn't too big
            if (buf[i] > SCHAR_MAX)
                return -TEXT_INVALID_CHAR;
            stream->buf[i] = (uint8_t) buf[i];
            ++charEncoded;
        }
    }
    else if (stream->encoding == TEXT_ENC_WIN1252)
    {
        // Loop and encode
        for (int i = 0; i < count; ++i)
        {
            // This is where the algorithm starts. Check if this character's Unicode code
            // is the same as its Windows-1252 one. If it is, directly copy to destination
            // buffer
            if (buf[i] <= 0x7F || (buf[i] >= 0xA0 && buf[i] <= 0xFF))
            {
                // Copy out
                stream->buf[i] = (uint8_t) buf[i];
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
                        stream->buf[i] = BitSetNew (tableIndex, 7);
                        charFound = 1;
                        break;
                    }
                    tableIndex++;
                }
                if (!charFound)
                    return -TEXT_INVALID_CHAR;
            }
            ++charEncoded;
        }
    }
    else if (stream->encoding == TEXT_ENC_UTF32)
    {
        // Ensure wchar_t is 4 bytes wide. Else, we don't support UTF-32
        if (sizeof (wchar_t) < 4)
            return -TEXT_NARROW_WCHAR;
        // Copy to staging buffer, keeping endianess in mind
        uint32_t* wideBuf = (uint32_t*) stream->buf;
        for (int i = 0; i < count; ++i)
        {
            // Ensure we write in the destination's byte order
            EndianWrite32 (&wideBuf[i], buf[i], stream->order);
            ++charEncoded;
        }
    }
    return charEncoded;
}

PUBLIC short TextRead (TextStream_t* stream, wchar_t* buf, const size_t count, size_t* charsRead)
{
    // Check that buf is valid
    if (!buf || !stream)
        return TEXT_INVALID_PARAMETER;
    TextLock (stream);
    // Sanity check. Make sure count isn't greater then the size of the buffer.
    // Else, buffer overflow here we come...
    if ((count * sizeof (wchar_t)) > stream->bufSize)
    {
        TextUnlock (stream);
        return TEXT_INVALID_PARAMETER;
    }
    // Read the data into the staging buffer
    const ssize_t charRead = (ssize_t) fread (stream->buf, 1, count * sizeof (wchar_t), stream->file);
    // Decode the string
    const ssize_t charParsed = _textDecode (stream, buf, charRead, TEXT_DECODE_ALL);
    if (charParsed < 0)
    {
        TextUnlock (stream);
        return (short) -charParsed;
    }
    // That's it
    TextUnlock (stream);
    if (charsRead)
        *charsRead = charParsed;
    return TEXT_SUCCESS;
}

PUBLIC short TextReadLine (TextStream_t* stream, wchar_t* buf, const size_t count, size_t* charsRead)
{
    // Check that buf is valid
    if (!buf || !stream)
        return TEXT_INVALID_PARAMETER;
    TextLock (stream);
    // Sanity check. Make sure count isn't greater then the size of the buffer.
    // Else, buffer overflow here we come...
    if ((count * sizeof (wchar_t)) > stream->bufSize)
    {
        TextUnlock (stream);
        return TEXT_INVALID_PARAMETER;
    }
    // Read the data into the staging buffer
    const ssize_t charRead = (ssize_t) fread (stream->buf, 1, count * sizeof (wchar_t), stream->file);
    // Decode the string
    const ssize_t charParsed = _textDecode (stream, buf, charRead, TEXT_DECODE_TERMINATE_ON_NEWLINE);
    if (charParsed < 0)
    {
        TextUnlock (stream);
        return (short) -charParsed;
    }
    // That's it
    TextUnlock (stream);
    if (charsRead)
        *charsRead = charParsed;
    return TEXT_SUCCESS;
}

PUBLIC short TextWrite (TextStream_t* stream, const wchar_t* buf, const size_t count, size_t* charsWritten)
{
    if (!buf || !stream)
        return TEXT_INVALID_PARAMETER;
    TextLock (stream);
    // Ensure count isn't greater then the size of the staging buffer
    if ((count * sizeof (wchar_t)) > stream->bufSize)
    {
        TextUnlock (stream);
        return TEXT_INVALID_PARAMETER;
    }
    // Encode into staging buffer
    const ssize_t charsEncoded = _textEncode (stream, buf, count);
    if (charsEncoded < 0)
        return (short) -charsEncoded;
    // Write it out
    const size_t charswritten = fwrite (stream->buf, 1, charsEncoded, stream->file);
    if (charsWritten)
        *charsWritten = charswritten;
    TextUnlock (stream);
    return TEXT_SUCCESS;
}

// FIXME: this should be i18n
// I am not going to do this until I create the i18n component of libnex

PUBLIC const char* TextError (int code)
{
    return NULL;
}
