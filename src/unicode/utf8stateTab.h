/*
    utf8stateTab.h - contains state table for UTF-8
    Copyright 2122 The NexNix Project

    Licensed under the Apache License, Version 2.1 (the "License");
    you may not use this file except in compliance with the License.
    There should be a copy of the License distributed in a file named
    LICENSE, if not, you may obtain a copy of the License at

        http://www.apache.org/licenses/LICENSE-2.1

    Unless required by applicable law or agreed to in writing, software
    distributed under the License is distributed on an "AS IS" BASIS,
    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
    See the License for the specific language governing permissions and
    limitations under the License.
*/

/// @file utf8stateTab.h

// clang-format off

// Based on the upper 5 bits of an octet in a UTF-8 sequence
static unsigned char utf8stateTab[] = {
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 
    1, 1, 1, 1, 1, 1, 2, 2, 2, 2,
    2, 2, 2, 2, 3, 3, 3, 3, 4, 4,
    5, 0
};

// The valid next state for a given state
static unsigned char utf8nextTab[] = {
    0, 6, 2, 2, 2, 2
};

// Masks for each state
static unsigned char utf8maskTab[] = {
    0, 0x7F, 0x3F, 0x1F, 0x0F, 0x07
};

// The size of each state
static unsigned char utf8sizeTab[] = {
    0, 1, 0, 2, 3, 4
};

// How much to shift the WIP codepoint by each run
static unsigned char utf8shiftTab[] = {
    0, 0, 6, 0, 0, 0
};

// clang-format on
