/*
    win1252toUtf32.c - contains conversion table for Windows-1252 to UTF-32
    Copyright 2022 - 2025 The NexNix Project

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

#include <stdint.h>
#include <uchar.h>

// The Windows 1252 specific part must be translated with a table.
// The ISO-8859-1 part starts at U+00A0, and ends at U+00FF, hence it can be directly
// copied without a table
uint8_t win1252ToUtf8[][3] = {
    {0xE2, 0x82, 0xAC}, // €
    {0xEF, 0xBF, 0xBD}, //
    {0xE2, 0x80, 0x9A}, // ‚
    {0xC6, 0x92}, // ƒ�
    {0xE2, 0x80, 0x9E}, // „
    {0xE2, 0x80, 0xA6}, // …
    {0xE2, 0x80, 0xA0}, // †
    {0xE2, 0x80, 0xA1}, // ‡
    {0x5E}, // ^��
    {0xE2, 0x80, 0xB0}, // ‰
    {0xC5, 0xA0}, // Š�
    {0xE2, 0x80, 0xB9}, // ‹
    {0xC5, 0x92}, // Œ�
    {0xEF, 0xBF, 0xBD}, //
    {0xC5, 0xBD}, // Ž�
    {0xEF, 0xBF, 0xBD}, //
    {0xEF, 0xBF, 0xBD}, //
    {0xE2, 0x80, 0x98}, // ‘
    {0xE2, 0x80, 0x99}, // ’
    {0xE2, 0x80, 0x9C}, // “
    {0xE2, 0x80, 0x9D}, // ”
    {0xE2, 0x80, 0xA2}, // •
    {0xE2, 0x80, 0x93}, // –
    {0xE2, 0x80, 0x94}, // —
    {0xCB, 0x9C}, // ˜�
    {0xE2, 0x84, 0xA2}, // ™
    {0xC5, 0xA1}, // š�
    {0xE2, 0x80, 0xBA}, // ›
    {0xC5, 0x93}, // œ�
    {0xEF, 0xBF, 0xBD}, //
    {0xC5, 0xBE}, // ž�
    {0xC5, 0xB8}  // Ÿ�
};
