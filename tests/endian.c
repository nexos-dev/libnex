/*
    endian.c - contains test suite for endian values
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

/// @file endian.c

#include <libnex.h>

#define NEXTEST_NAME "endian"
#include <nextest.h>

int main()
{
    // Test the swapping macros
    uint16_t test16 = 0x3454;
    TEST (EndianSwap16 (test16), 0x5434, "16 bit endian swap");
    uint32_t test32 = 0x34541267;
    TEST (EndianSwap32 (test32), 0x67125434, "32 bit endian swap");
    uint64_t test64 = 0x0123456789ABCDEF;
    TEST (EndianSwap64 (test64), 0xEFCDAB8967452301, "64 bit endian swap");

    // Test the reading and writing functions
    if (EndianHost() == ENDIAN_LITTLE)
    {
        uint16_t val16 = 0x3454;
        TEST (EndianRead16 (&val16, ENDIAN_LITTLE), 0x3454, "reading value with like endianess as host (16 bit)");
        TEST (EndianRead16 (&val16, ENDIAN_BIG), 0x5434, "reading value of differing endianess then host (16 bit)");

        EndianWrite16 (&val16, 0x5689, ENDIAN_LITTLE);
        TEST (val16, 0x5689, "writing value with like endianess as host (16 bit)");
        EndianWrite16 (&val16, 0x5689, ENDIAN_BIG);
        TEST (val16, 0x8956, "writing value with like endianess as host (16 bit)");

        uint32_t val32 = 0x14243454;
        TEST (EndianRead32 (&val32, ENDIAN_LITTLE), 0x14243454, "reading value with like endianess as host (32 bit)");
        TEST (EndianRead32 (&val32, ENDIAN_BIG), 0x54342414, "reading value of differing endianess then host (32 bit)");

        EndianWrite32 (&val32, 0x568965, ENDIAN_LITTLE);
        TEST (val32, 0x568965, "writing value with like endianess as host (32 bit)");
        EndianWrite32 (&val32, 0x568965, ENDIAN_BIG);
        TEST (val32, 0x65895600, "writing value with differing endianess then host (32 bit)");

        uint64_t val64 = 0x0123456789ABCDEF;
        TEST (EndianRead64 (&val64, ENDIAN_LITTLE),
              0x0123456789ABCDEF,
              "reading value with like endianess as host (64 bit)");
        TEST (EndianRead64 (&val64, ENDIAN_BIG),
              0xEFCDAB8967452301,
              "reading value of differing endianess then host (64 bit)");

        EndianWrite64 (&val64, 0x0123456789ABCD23, ENDIAN_LITTLE);
        TEST (val64, 0x0123456789ABCD23, "writing value with like endianess as host (64 bit)");
        EndianWrite64 (&val64, 0x0123456789ABCD23, ENDIAN_BIG);
        TEST (val64, 0x23CDAB8967452301, "writing value with differing endianess then host (64 bit)");
    }
    else if (EndianHost() == ENDIAN_BIG)
    {
        uint16_t val16 = 0x3454;
        TEST (EndianRead16 (&val16, ENDIAN_BIG), 0x3454, "reading value with like endianess as host (16 bit)");
        TEST (EndianRead16 (&val16, ENDIAN_LITTLE), 0x5434, "reading value of differing endianess then host (16 bit)");

        EndianWrite16 (&val16, 0x5689, ENDIAN_BIG);
        TEST (val16, 0x5689, "writing value with like endianess as host (16 bit)");
        EndianWrite16 (&val16, 0x5689, ENDIAN_LITTLE);
        TEST (val16, 0x8956, "writing value with like endianess as host (16 bit)");

        uint32_t val32 = 0x14243454;
        TEST (EndianRead32 (&val32, ENDIAN_BIG), 0x14243454, "reading value with like endianess as host (32 bit)");
        TEST (EndianRead32 (&val32, ENDIAN_LITTLE),
              0x54342414,
              "reading value of differing endianess then host (32 bit)");

        EndianWrite32 (&val32, 0x568965, ENDIAN_BIG);
        TEST (val32, 0x568965, "writing value with like endianess as host (32 bit)");
        EndianWrite32 (&val32, 0x568965, ENDIAN_LITTLE);
        TEST (val32, 0x65895600, "writing value with differing endianess then host (32 bit)");

        uint64_t val64 = 0x0123456789ABCDEF;
        TEST (EndianRead64 (&val64, ENDIAN_BIG),
              0x0123456789ABCDEF,
              "reading value with like endianess as host (64 bit)");
        TEST (EndianRead64 (&val64, ENDIAN_LITTLE),
              0xEFCDAB8967452301,
              "reading value of differing endianess then host (64 bit)");

        EndianWrite64 (&val64, 0x0123456789ABCD23, ENDIAN_BIG);
        TEST (val64, 0x0123456789ABCD23, "writing value with like endianess as host (64 bit)");
        EndianWrite64 (&val64, 0x0123456789ABCD23, ENDIAN_LITTLE);
        TEST (val64, 0x23CDAB8967452301, "writing value with differing endianess then host (64 bit)");
    }
    return 0;
}
