/*
    win1252toUtf32.c - contains conversion table for Windows-1252 to UTF-32
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

#include <wchar.h>

// The Windows 1252 specific part must be translated with a table.
// The ISO-8859-1 part starts at U+00A0, and ends at U+00FF, hence it can be directly
// copied without a table
wchar_t win1252toUtf32[] = {
    0x20AC,    // €
    0x0,
    0x201A,    // ‚
    0x0192,    // ƒ
    0x201E,    // „
    0x2026,    // …
    0x2020,    // †
    0x2021,    // ‡
    0x005E,    // ^
    0x2030,    // ‰
    0x0160,    // Š
    0x2039,    // ‹
    0x0152,    // Œ
    0x0,
    0x017D,    // Ž
    0x0,    0x0,
    0x2018,    // ‘
    0x2019,    // ’
    0x201C,    // “
    0x201D,    // ”
    0x2022,    // •
    0x2013,    // –
    0x2014,    // —
    0x02DC,    // ˜
    0x2122,    // ™
    0x0161,    // š
    0x203A,    // ›
    0x0153,    // œ
    0x0,
    0x017E,    // ž
    0x0178     // Ÿ
};
