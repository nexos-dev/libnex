/*
    bits.c - contains test suite bitwise operations
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

/// @file bits.c

#include <libnex.h>

#define NEXTEST_NAME "bits"
#include <nextest.h>

int main()
{
    uint32_t bitSet = 0x0;
    TEST (BitSetNew (bitSet, 4), 0x10, "BitSetNew()");
    BitSet (bitSet, 7);
    TEST (bitSet, 0x80, "BitSet()");
    TEST (BitClearNew (bitSet, 7), 0x0, "BitClearNew()");
    BitSet (bitSet, 10);
    BitSet (bitSet, 3);
    TEST_ANON (bitSet, 0x488);
    BitClear (bitSet, 3);
    TEST (bitSet, 0x480, "BitClear()");
    TEST (BitGet (bitSet, 10), 1, "BitGet()");
    TEST (BitGet (bitSet, 2), 0, "BitGet()");
    bitSet = 1;
    BitSetRange (bitSet, 2, 3);
    TEST (bitSet, 0x1D, "BitSetRange()");
    TEST (BitSetRangeNew (bitSet, 7, 10), 0x1FF9D, "BitSetRange()");
    BitSetRange (bitSet, 7, 10);
    TEST (BitClearRangeNew (bitSet, 7, 10), 0x1D, "BitClearRangeNew()");
    TEST (BitGetRange (bitSet, 1, 3), 6, "BitGetRange()");
    return 0;
}
