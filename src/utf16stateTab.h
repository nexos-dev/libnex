/*
    utf16stateTab.h - contains state table for UTF-16 FSM
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

/// @file utf16stateTab.h

// This table describes states transititons for the UTF-16 decoder. The index is the upper 6 bits
// All but 2 locations translate to state 0, which is the accept state. State 1 is low surrogate value,
// state 2 is high surrogate.

// clang-format off

unsigned char utf16stateTab[] = {
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
    0, 0, 0, 0, 1, 2, 0, 0, 0, 0, 
    0, 0, 0, 0
};

// clang-format on
