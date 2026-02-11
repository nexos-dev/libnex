/*
    textstream.c - contains test suite for textstream class
    Copyright 2022 - 2026 The NexNix Project

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
#include <locale.h>
#define NEXTEST_NAME "textstream"
#include <nextest.h>
#include <stdlib.h>
#include <string.h>

int main()
{
    setlocale (LC_ALL, "");
    // Test TextError
    TEST_BOOL (!strcmp (TextError (TEXT_INVALID_CHAR), "Character can't be encoded by character set"),
               "TextError");
    {
        // Test opening a text stream
        TextStream_t* stream;
        if (TextOpen ("testAscii1.testxt", &stream, TEXT_MODE_READ, TEXT_ENC_ASCII, 0, 0) != TEXT_SUCCESS)
            return 1;
        // Read in some text
        char* buf = (char*) malloc_s (500 * UTF8_MAX);
        if (TextRead (stream, buf, 500, NULL) != TEXT_SUCCESS)
            return 1;
        char buf2[] = "Test string. This is an ASCII document.\n";
        TEST_BOOL (!strcmp (buf, buf2), "reading ASCII");
        // Test TextReadLine
        if (TextOpen ("testAscii1.testxt", &stream, TEXT_MODE_READ, TEXT_ENC_ASCII, 0, 0) != TEXT_SUCCESS)
            return 1;
        buf = (char*) malloc_s (500 * UTF8_MAX);
        if (TextReadLine (stream, buf, 500, NULL) != TEXT_SUCCESS)
            return 1;
        char buf5[] = "Test string. This is an ASCII document.\n";
        TEST_BOOL (!strcmp (buf, buf5), "reading a line of ASCII");
        TextClose (stream);
        free (buf);
        // Write out some text
        char buf3[] = "This is a test document.\n";
        // Create a new file
        TextStream_t* stream2;
        if (TextOpen ("testAscii1.testout", &stream2, TEXT_MODE_WRITE, TEXT_ENC_ASCII, 0, 0) != TEXT_SUCCESS)
            return 1;
        // Write out buf
        if (TextWrite (stream2, buf3, strlen (buf3), NULL) != TEXT_SUCCESS)
            return 1;
        TextClose (stream2);
        // Read it in now
        if (TextOpen ("testAscii1.testout", &stream2, TEXT_MODE_READ, TEXT_ENC_ASCII, 0, 0) != TEXT_SUCCESS)
            return 1;
        // Read and compare
        char* buf4 = (char*) malloc_s (500 * UTF8_MAX);
        if (TextRead (stream2, buf4, strlen (buf3) + 1, NULL) != TEXT_SUCCESS)
            return 1;
        TEST_BOOL (!strcmp (buf4, buf3), "writing ASCII");
        free (buf4);
        TextClose (stream2);
    }
    // Test Windows-1252 support
    {
        TextStream_t* stream1;
        if (TextOpen ("testWin1252.testxt", &stream1, TEXT_MODE_READ, TEXT_ENC_WIN1252, 0, 0) != TEXT_SUCCESS)
            return 1;
        char* buf = (char*) malloc_s (500 * UTF8_MAX);
        if (TextRead (stream1, buf, 500, NULL) != TEXT_SUCCESS)
            return 1;
        char buf2[] = "Test windows 1252 document. Here is a non-ASCII character: ÿ Ž\n";
        TEST_BOOL (!strcmp (buf, buf2), "reading Windows 1252");
        free (buf);
        TextClose (stream1);
        // Test writing it
        TextStream_t* stream2;
        if (TextOpen ("testWin1252.testout", &stream2, TEXT_MODE_WRITE, TEXT_ENC_WIN1252, 0, 0) != TEXT_SUCCESS)
            return 1;
        if (TextWrite (stream2, buf2, strlen (buf2), NULL) != TEXT_SUCCESS)
            return 1;
        TextClose (stream2);
        // Read and compare
        buf = (char*) malloc_s (500 * UTF8_MAX);
        if (TextOpen ("testWin1252.testout", &stream2, TEXT_MODE_READ, TEXT_ENC_WIN1252, 0, 0) != TEXT_SUCCESS)
            return 1;
        if (TextRead (stream2, buf, strlen (buf2) + 1, NULL) != TEXT_SUCCESS)
            return 1;
        TEST_BOOL (!strcmp (buf, buf2), "writing Windows 1252");
        TextClose (stream2);
        free (buf);
    }
    // Test UTF-32 support
    {
        TextStream_t* stream1;
        if (TextOpen ("testUtf32.testxt", &stream1, TEXT_MODE_READ, TEXT_ENC_UTF32, 1, 0) != TEXT_SUCCESS)
            return 1;
        TEST (stream1->order, TEXT_ORDER_LE, "UTF-32 BOM support");
        char buf[] = "Test document €\n";
        char* buf2 = (char*) malloc_s (500 * UTF8_MAX);
        if (TextRead (stream1, buf2, 500, NULL) != TEXT_SUCCESS)
            return 1;
        TEST_BOOL (!strcmp (buf, buf2), "reading UTF-32");
        TextClose (stream1);
        // Write UTF-32
        if (TextOpen ("testUtf32.testout", &stream1, TEXT_MODE_WRITE, TEXT_ENC_UTF32, 1, TEXT_ORDER_BE) !=
            TEXT_SUCCESS)
            return 1;
        char buf3[] = "Test document € 𮀀\n";
        if (TextWrite (stream1, buf3, strlen (buf3), NULL) != TEXT_SUCCESS)
            return 1;
        TextClose (stream1);
        // Test that it is correct
        if (TextOpen ("testUtf32.testout", &stream1, TEXT_MODE_READ, TEXT_ENC_UTF32, 1, 0) != TEXT_SUCCESS)
            return 1;
        char* buf4 = (char*) malloc_s (500 * UTF8_MAX);
        if (TextRead (stream1, buf4, strlen (buf3) + 1, NULL) != TEXT_SUCCESS)
            return 1;
        TEST_BOOL (!strcmp (buf4, buf3), "writing UTF-32");
        TextClose (stream1);
        free (buf4);
        free (buf2);
    }
    // Test UTF-16
    {
        TextStream_t* stream1 = NULL;
        if (TextOpen ("testUtf16.testxt", &stream1, TEXT_MODE_READ, TEXT_ENC_UTF16, 1, 0) != TEXT_SUCCESS)
            return 1;
        TEST (stream1->order, TEXT_ORDER_BE, "UTF-16 BOM support");
        char buf1[] = "Test document € 𠀀 test2\n";
        char* buf2 = (char*) malloc_s (500);
        if (TextRead (stream1, buf2, strlen (buf1) + 1, NULL) != TEXT_SUCCESS)
            return 1;
        TEST_BOOL (!strcmp (buf1, buf2), "reading UTF-16");
        TextClose (stream1);
        if (TextOpen ("testUtf16.testout", &stream1, TEXT_MODE_WRITE, TEXT_ENC_UTF16, 1, TEXT_ORDER_BE) !=
            TEXT_SUCCESS)
            return 1;
        char buf3[] = "Test document € 𠀀\n";
        if (TextWrite (stream1, buf3, strlen (buf3), NULL) != TEXT_SUCCESS)
            return 1;
        TextClose (stream1);
        // Test that it is correct
        if (TextOpen ("testUtf16.testout", &stream1, TEXT_MODE_READ, TEXT_ENC_UTF16, 1, 0) != TEXT_SUCCESS)
            return 1;
        char* buf4 = (char*) malloc_s (500 * UTF8_MAX);
        if (TextRead (stream1, buf4, strlen (buf3) + 1, NULL) != TEXT_SUCCESS)
            return 1;
        TEST_BOOL (!strcmp (buf4, buf3), "writing UTF-16");
        TextClose (stream1);
        free (buf4);
        free (buf2);
    }
    // Test UTF-8
    {
        TextStream_t* stream1 = NULL;
        if (TextOpen ("testUtf8.testxt", &stream1, TEXT_MODE_READ, TEXT_ENC_UTF8, 0, 0) != TEXT_SUCCESS)
            return 1;
        char buf1[] = "Test string € 𠀀\n";
        char* buf2 = (char*) malloc_s (500);
        if (TextRead (stream1, buf2, strlen (buf1) + 1, NULL) != TEXT_SUCCESS)
            return 1;
        TEST_BOOL (!strcmp (buf1, buf2), "reading UTF-8");
        TextClose (stream1);
        free (buf2);
        if (TextOpen ("testUtf8.testout", &stream1, TEXT_MODE_WRITE, TEXT_ENC_UTF8, 0, 0) != TEXT_SUCCESS)
            return 1;
        char buf3[] = "Test document € 𠀀\n";
        if (TextWrite (stream1, buf3, strlen (buf3), NULL) != TEXT_SUCCESS)
            return 1;
        TextClose (stream1);
        // Test that it is correct
        if (TextOpen ("testUtf8.testout", &stream1, TEXT_MODE_READ, TEXT_ENC_UTF8, 0, 0) != TEXT_SUCCESS)
            return 1;
        char* buf4 = (char*) malloc_s (500 * UTF8_MAX);
        if (TextRead (stream1, buf4, strlen (buf3) + 1, NULL) != TEXT_SUCCESS)
            return 1;
        TEST_BOOL (!strcmp (buf4, buf3), "writing UTF-8");
        TextClose (stream1);
        free (buf4);
    }
    return 0;
}
