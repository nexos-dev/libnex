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
#include <libnex/safemalloc.h>
#include <libnex/textstream.h>
#include <limits.h>
#include <stdlib.h>

// Helper macros
#define TEXT_DEFAULT_BUFSZ               4096    ///< Staging buffer has a default size of 4 KiB
#define TEXT_DECODE_TERMINATE_ON_NEWLINE 1       ///< Terminate decoding on newline discovery
#define TEXT_DECODE_ALL                  0       ///< Decode whole buffer

/**
 * @brief Opens a up a text stream
 *
 * TextOpen takes a file name and a mode, and creates a new buffer according to
 * the mode that was requested
 *
 * @param[in] name specifies the file name to open
 * @param[in] mode the opening mode
 * @return A new textStream_t object. NULL on error
 */
PUBLIC TextStream_t* TextOpen (char* file, char mode, char encoding, char hasBom)
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
    {
        errno = EINVAL;
        return NULL;
    }
    // Open the file
    stream->file = fopen (file, fopenMode);
    if (!stream->file)
        return NULL;
    // Set the encoding
    stream->encoding = encoding;
    // Check if there is a BOM, and if there is, set the byte order based on that
    if (hasBom)
    {
        // Read in the BOM
        uint8_t bom[8];
        fread (bom, 8, 1, stream->file);
        if (encoding == TEXT_ENC_UTF16)
        {
            // Check for a UTF-16 BOM in big endian order
            if (bom[0] == 0xFE && bom[1] == 0xFF)
                stream->order = TEXT_ORDER_BE;
            // Check in little endian order
            else if (bom[0] == 0xFF && bom[1] == 0xFE)
                stream->order = TEXT_ORDER_LE;
            else
            {
                // Bad BOM
                errno = EINVAL;
                return NULL;
            }
        }
        // We don't care about a BOM otherwise.
    }
    else
    {
        if (stream->encoding == TEXT_ENC_UTF16)
        {
            // We make an assumption here. This file is UTF-16,
            // and doesn't have a BOM. In either case, we assume this is little endian
            // If that assumption is false, we are in a world of hurting
            stream->order = TEXT_ORDER_LE;
        }
        // This is a single byte character set
        else
            stream->order = TEXT_ORDER_NONE;
    }
    // Finally, create the object
    ObjCreate (file, &stream->obj);
    return stream;
}

/**
 * @brief Closes a text stream
 *
 * TextClose takes a textStream and closes it, freeing all associated data about
 * it
 *
 * @param[in] stream the stream to close
 */
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

/**
 * @brief Gets size of text stream
 *
 * TextSize obtains the size of the text stream specified by stream
 *
 * @param[in] stream the stream to get the size from
 * @return < 0 on error, else, the size
 */
PUBLIC long TextSize (TextStream_t* stream)
{
    // Lock it
    TextLock (stream);
    // Get current position and seek to end
    long pos = ftell (stream->file);
    fseek (stream->file, 0, SEEK_END);
    // Get the size
    long size = ftell (stream->file);
    // Reset the old size
    fseek (stream->file, pos, SEEK_SET);
    TextUnlock (stream);
    return size;
}

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
PUBLIC void TextSetBufSz (TextStream_t* stream, size_t sz)
{
    TextLock (stream);
    stream->bufSize = sz;
    // Resize it
    stream->buf = realloc_s (stream->buf, stream->bufSize);
    TextUnlock (stream);
}

// Checks if we reached a newline
char _textCheckNewLine (uint8_t* buf, int i, size_t bytesLeft)
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

// Internal function to decode characters
// terminator = 0 means to parse everything, 1 means parse until LF, CR, or CRLF is encountered
ssize_t _textDecode (TextStream_t* stream, wchar_t* buf, size_t count, int terminator)
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
                char doWhat = _textCheckNewLine (stream->buf, i, count);
                if (doWhat == 0)
                    continue;
                else if (doWhat == 1)
                    return i;
            }
        }
        charParsed = (ssize_t) count;
    }
    else if (stream->encoding == TEXT_ENC_WIN1252)
    {
        // Loop through staging buffer
        for (int i = 0; i < count; ++i)
        {
            // Here is the algorithm for decoding: If stream->buf[i] doesn't have bit 7 set,
            // it is like the ASCII version, or it is an ISO-8859-1 character, we copy it directly.
            // Else, we go into the Windows 1252 to UTF-32 decoding table
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
                char doWhat = _textCheckNewLine (stream->buf, i, count);
                if (doWhat == 0)
                    continue;
                else if (doWhat == 1)
                    return i;
            }
        }
        charParsed = (ssize_t) count;
    }
    return charParsed;
}

// Internal function to encode characters into staging buffer
// Returns -1 on error
ssize_t _textEncode (TextStream_t* stream, wchar_t* buf, size_t count)
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
            {
                errno = EOVERFLOW;
                return -1;
            }
            stream->buf[i] = (uint8_t) buf[i];
        }
        charEncoded = (ssize_t) count;
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
                int tableSize = ARRAY_SIZE (win1252toUtf32);
                int tableIndex = 0;
                while (tableIndex < tableSize)
                {
                    // Check for a match
                    if (win1252toUtf32[tableIndex] == buf[i])
                    {
                        // Set bit 7 on tableIndex, and that is the character
                        stream->buf[i] = BitSetNew (tableIndex, 7);
                    }
                    tableIndex++;
                }
            }
        }
        charEncoded = (ssize_t) count;
    }
    return charEncoded;
}

/**
 * @brief Reads data from a text stream
 *
 * TextRead takes a buffer, character count, and stream object,
 * and reads / decodes count codepoints into buf from stream.
 * Data is intially read into a staging buffer, and then the staging buffer is
 * decoded into the main buffer specified by buf
 *
 * @param[in] stream the stream to read from
 * @param[out] buf a buffer of wchar_t's to decode into
 * @param[in] count the number of wchar_t's to decode
 * @return the number of characters read on success, -1 on failure
 */
PUBLIC ssize_t TextRead (TextStream_t* stream, wchar_t* buf, size_t count)
{
    // Check that buf is valid
    if (!buf)
    {
        errno = EINVAL;
        return 0;
    }
    TextLock (stream);
    // Sanity check. Make sure count isn't greater then the size of the buffer.
    // Else, buffer overflow here we come...
    if ((count * sizeof (wchar_t)) > stream->bufSize)
    {
        TextUnlock (stream);
        errno = EOVERFLOW;
        return 0;
    }
    // Read the data into the staging buffer
    ssize_t charRead = (ssize_t) fread (stream->buf, 1, count * sizeof (wchar_t), stream->file);
    // Decode the string
    ssize_t charParsed = _textDecode (stream, buf, charRead, TEXT_DECODE_ALL);
    if (charParsed == -1)
        return -1;
    // That's it
    TextUnlock (stream);
    return charRead;
}

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
 * @param[out] buf a buffer of wchar_t's to decode into
 * @param[in] count the max number of wchar_t's to decode
 * @return the number of characters read on success, -1 on failure
 */
PUBLIC ssize_t TextReadLine (TextStream_t* stream, wchar_t* buf, size_t count)
{
    // Check that buf is valid
    if (!buf)
    {
        errno = EINVAL;
        return 0;
    }
    TextLock (stream);
    // Sanity check. Make sure count isn't greater then the size of the buffer.
    // Else, buffer overflow here we come...
    if ((count * sizeof (wchar_t)) > stream->bufSize)
    {
        TextUnlock (stream);
        errno = EOVERFLOW;
        return 0;
    }
    // Read the data into the staging buffer
    ssize_t charRead = (ssize_t) fread (stream->buf, 1, count * sizeof (wchar_t), stream->file);
    // Decode the string
    ssize_t charParsed = _textDecode (stream, buf, charRead, TEXT_DECODE_TERMINATE_ON_NEWLINE);
    // That's it
    TextUnlock (stream);
    return charParsed;
}

/**
 * @brief Writes data into a text stream
 *
 * TextWrite writes out a buffer of wchar_t's to a file, encoding them first.
 * Data is encoded into a staging buffer, and then written to stream
 *
 * @param[in] stream the stream to write to
 * @param[in] buf the buffer to write from
 * @param[in] count the number of wchar_t's to write
 * @return count of wchar_t's written on success, 0 on failure
 */
PUBLIC ssize_t TextWrite (TextStream_t* stream, wchar_t* buf, size_t count)
{
    if (!buf)
    {
        errno = EINVAL;
        return 0;
    }
    TextLock (stream);
    // Ensure count isn't greater then the size of the staging buffer
    if ((count * sizeof (wchar_t)) > stream->bufSize)
    {
        TextUnlock (stream);
        errno = EOVERFLOW;
        return 0;
    }
    // Encode into staging buffer
    ssize_t charsEncoded = _textEncode (stream, buf, count);
    if (charsEncoded == -1)
        return -1;
    // Write it out
    ssize_t charsWritten = (ssize_t) fwrite (stream->buf, 1, charsEncoded, stream->file);
    TextUnlock (stream);
    return charsWritten;
}
