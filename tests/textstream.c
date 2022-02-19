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
        TextStream_t* stream;
        if (TextOpen ("testAscii1.testxt", &stream, TEXT_MODE_READ, TEXT_ENC_ASCII, 0, 0) != TEXT_SUCCESS)
            return 1;
        // Read in some text
        wchar_t* buf = (wchar_t*) malloc_s (500 * sizeof (wchar_t));
        if (TextRead (stream, buf, 500, NULL) != TEXT_SUCCESS)
            return 1;
        // FIXME: If C compiler doesn't use unicode by default, these wcscmp's may not work
        // Luckily, CL uses UTF-16 and Clang and GCC use UTF-32, making this work most of the time.
        wchar_t buf2[] = L"Test string. This is an ASCII document.\r\n";
        TEST_BOOL (!wcscmp (buf, buf2), "reading ASCII");
        // Test TextSize
        TEST (TextSize (stream), wcslen (buf2), "TextSize");
        free (buf);
        TextClose (stream);
        // Test TextReadLine
        if (TextOpen ("testAscii1.testxt", &stream, TEXT_MODE_READ, TEXT_ENC_ASCII, 0, 0) != TEXT_SUCCESS)
            return 1;
        buf = (wchar_t*) malloc_s (500 * sizeof (wchar_t));
        if (TextReadLine (stream, buf, 500, NULL) != TEXT_SUCCESS)
            return 1;
        wchar_t buf5[] = L"Test string. This is an ASCII document.\r\n";
        TEST_BOOL (!wcscmp (buf, buf5), "reading a line of ASCII");
        TextClose (stream);
        free (buf);
        // Write out some text
        wchar_t buf3[] = L"This is a test document.\n";
        // Create a new file
        TextStream_t* stream2;
        if (TextOpen ("testAscii1.testout", &stream2, TEXT_MODE_WRITE, TEXT_ENC_ASCII, 0, 0) != TEXT_SUCCESS)
            return 1;
        // Write out buf
        if (TextWrite (stream2, buf3, wcslen (buf3), NULL) != TEXT_SUCCESS)
            return 1;
        TextClose (stream2);
        // Read it in now
        if (TextOpen ("testAscii1.testout", &stream2, TEXT_MODE_READ, TEXT_ENC_ASCII, 0, 0) != TEXT_SUCCESS)
            return 1;
        // Read and compare
        wchar_t* buf4 = (wchar_t*) malloc_s (500 * sizeof (wchar_t));
        if (TextRead (stream2, buf4, wcslen (buf3), NULL) != TEXT_SUCCESS)
            return 1;
        TEST_BOOL (!wcscmp (buf4, buf3), "writing ASCII");
        free (buf4);
        TextClose (stream2);
    }
    // Test Windows-1252 support
    {
        TextStream_t* stream1;
        if (TextOpen ("testWin1252.testxt", &stream1, TEXT_MODE_READ, TEXT_ENC_WIN1252, 0, 0) != TEXT_SUCCESS)
            return 1;
        wchar_t* buf = (wchar_t*) malloc_s (500 * sizeof (wchar_t));
        if (TextRead (stream1, buf, 500, NULL) != TEXT_SUCCESS)
            return 1;
        wchar_t buf2[] = L"Test windows 1252 document. Here is a non-ASCII character: ÿ Ž\n";
        TEST_BOOL (!wcscmp (buf, buf2), "reading Windows 1252");
        free (buf);
        TextClose (stream1);
        // Test writing it
        TextStream_t* stream2;
        if (TextOpen ("testWin1252.testout", &stream2, TEXT_MODE_WRITE, TEXT_ENC_WIN1252, 0, 0) != TEXT_SUCCESS)
            return 1;
        if (TextWrite (stream2, buf2, wcslen (buf2), NULL) != TEXT_SUCCESS)
            return 1;
        TextClose (stream2);
        // Read and compare
        buf = (wchar_t*) malloc_s (500 * sizeof (wchar_t));
        if (TextOpen ("testWin1252.testout", &stream2, TEXT_MODE_READ, TEXT_ENC_WIN1252, 0, 0) != TEXT_SUCCESS)
            return 1;
        if (TextRead (stream2, buf, wcslen (buf2), NULL) != TEXT_SUCCESS)
            return 1;
        TEST_BOOL (!wcscmp (buf, buf2), "writing Windows 1252");
        TextClose (stream2);
        free (buf);
    }
    // Test UTF-32 support
    {
        TextStream_t* stream1;
        if (TextOpen ("testUtf32.testxt", &stream1, TEXT_MODE_READ, TEXT_ENC_UTF32, 1, 0) != TEXT_SUCCESS)
            return 1;
        TEST (stream1->order, TEXT_ORDER_LE, "UTF-32 BOM support");
        TextClose (stream1);
    }
    return 0;
}
