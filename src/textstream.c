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

#include <errno.h>
#include <libnex/safemalloc.h>
#include <libnex/textstream.h>
#include <stdlib.h>

// Helper macros
#define TEXT_DEFAULT_BUFSZ 4096    ///< Staging buffer has a default size of 4 KiB

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
    else if (mode == TEXT_MODE_READ_APPEND)
        fopenMode = "a+";
    else if (mode == TEXT_MODE_READ_WRITE)
        fopenMode = "w+";
    else if (mode == TEXT_MODE_READ_WRITE_NOTRUNC)
        fopenMode = "r+";
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
        // We make an assumption here. This file is either an 8 bit format,
        // in which case endianess doesn't matter. Or this file is UTF-16 or UTF-32,
        // and doesn't have a BOM. In either case, we assume this is little endian
        // If that assumption is false, we are in a world of hurting
        stream->encoding = TEXT_ORDER_LE;
    }
    // Finally, create the lock
    _libnex_lock_init (&stream->lock);
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
    // Lock the stream
    _libnex_lock_lock (&stream->lock);
    // Free the buffer
    free (stream->buf);
}
