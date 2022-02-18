/*
    textstream.c - contains test suite for textstream class
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

#include <libnex.h>
#define NEXTEST_NAME "textstream"
#include <locale.h>
#include <nextest.h>
#include <stdlib.h>

int main()
{
    {
        // Test opening a text stream
        TextStream_t* stream = TextOpen ("testAscii1.testxt", TEXT_MODE_READ, TEXT_ENC_ASCII, 0);
        if (!stream)
            return 1;
        // Read in some text
        wchar_t* buf = (wchar_t*) malloc_s (500 * sizeof (wchar_t));
        if (TextRead (stream, buf, 500) == -1)
            return 1;
        // FIXME: If C compiler doesn't use unicode by default, these wcscmp's may not work
        // Luckily, CL uses UTF-16 and Clang and GCC use UTF-32, making this work most of the time.
        wchar_t buf2[] = L"Test string. This is an ASCII document.\n";
        TEST_BOOL (!wcscmp (buf, buf2), "reading ASCII");
        // Test TextSize
        TEST (TextSize (stream), wcslen (buf2), "TextSize");
        free (buf);
        TextClose (stream);
        // Write out some text
        wchar_t buf3[] = L"This is a test document.\n";
        // Create a new file
        TextStream_t* stream2 = TextOpen ("testAscii1.testout", TEXT_MODE_WRITE, TEXT_ENC_ASCII, 0);
        if (!stream2)
            return 1;
        // Write out buf
        if (TextWrite (stream2, buf3, wcslen (buf3)) == -1)
            return 1;
        TextClose (stream2);
        // Read it in now
        stream2 = TextOpen ("testAscii1.testout", TEXT_MODE_READ, TEXT_ENC_ASCII, 0);
        // Read and compare
        wchar_t* buf4 = (wchar_t*) malloc_s (500 * sizeof (wchar_t));
        if (TextRead (stream2, buf4, wcslen (buf3)) == -1)
            return 1;
        TEST_BOOL (!wcscmp (buf4, buf3), "writing ASCII");
        free (buf4);
        TextClose (stream2);
    }
    // Test Windows-1252 support
    {
        TextStream_t* stream1 = TextOpen ("testWin1252.testxt", TEXT_MODE_READ, TEXT_ENC_WIN1252, 0);
        if (!stream1)
            return 1;
        wchar_t* buf = (wchar_t*) malloc_s (500 * sizeof (wchar_t));
        if (TextRead (stream1, buf, 500) == -1)
            return -1;
        wchar_t buf2[] = L"Test windows 1252 document. Here is a non-ASCII character: ÿ Ž\n";
        TEST_BOOL (!wcscmp (buf, buf2), "reading Windows 1252");
        free (buf);
        TextClose (stream1);
    }
    return 0;
}
