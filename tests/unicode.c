/*
    unicode.c - contains test suite for unicode stuff
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

/// @file unicode.c

#include <libnex.h>
#define NEXTEST_NAME "unicode"
#include <nextest.h>

int main()
{
    // Test decoding UTF-16
    char16_t val[] = u"t";
    char32_t out;
    UnicodeDecode16 (&out, val, 4, EndianHost());
    TEST (out, U't', "UnicodeDecode16");
    char16_t val2[] = u"𠀀";
    char32_t refVal = U'𠀀';
    UnicodeDecode16 (&out, val2, 6, EndianHost());
    TEST (out, refVal, "UnicodeDecode16 surrogates");

    // Test encoding UTF-16
    char32_t val3 = U'𠀀';
    char16_t buf[3];
    UnicodeEncode16 (buf, val3, EndianHost());
    char32_t out2;
    UnicodeDecode16 (&out2, buf, 3, EndianHost());
    TEST (out2, refVal, "UnicodeEncode16 surrogates");

    // Test decoding UTF-8
    uint8_t val4[] = "𠀀";
    UnicodeDecode8 (&out, val4, 5);
    TEST (out, refVal, "UnicodeDecode8 with 4 bytes");

    uint8_t val5[] = "╤";
    UnicodeDecode8 (&out, val5, 4);
    refVal = U'╤';
    TEST (out, refVal, "UnicodeDecode8 with 3 bytes");

    uint8_t val6[] = "Þ";
    refVal = U'Þ';
    UnicodeDecode8 (&out, val6, 3);
    TEST (out, refVal, "UnicodeDecode8 with 2 bytes");

    uint8_t val7 = 'a';
    UnicodeDecode8 (&out, &val7, 1);
    TEST (out, U'a', "UnicodeDecode8 with 1 byte");

    // Test encoding
    char32_t val8 = U'𡿿';
    uint8_t val9[4];
    UnicodeEncode8 (val9, val8, 4);
    UnicodeDecode8 (&out, val9, 4);
    TEST (out, U'𡿿', "UnicodeEncode8");
    val8 = U'ሴ';
    UnicodeEncode8 (val9, val8, 4);
    UnicodeDecode8 (&out, val9, 4);
    TEST (out, U'ሴ', "UnicodeEncode8");
    val8 = U'¬';
    UnicodeEncode8 (val9, val8, 4);
    UnicodeDecode8 (&out, val9, 4);
    TEST (out, U'¬', "UnicodeEncode8");
    val8 = U'a';
    UnicodeEncode8 (val9, val8, 4);
    UnicodeDecode8 (&out, val9, 4);
    TEST (out, U'a', "UnicodeEncode8");
    return 0;
}
