/*
    hash.c - contains hash table implementation
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

#include <libnex/hash.h>

// Hash function parameters
#define HASH_FNV1A_PRIME       16777619
#define HASH_FNV1A_OFFSET_BASE 2166136261

uint32_t HashCreateHash (void* data, size_t sz)
{
    // Get buffer bounds
    uint8_t* buf = data;
    uint8_t* bufEnd = buf + sz;
    // Setup hash
    uint32_t hash = HASH_FNV1A_OFFSET_BASE;
    // Compute it
    while (buf < bufEnd)
    {
        hash ^= (uint32_t) *buf++;
        hash *= HASH_FNV1A_PRIME;
    }
    return hash;
}

uint32_t HashCreateHashStr (char* str)
{
    uint8_t* buf = (uint8_t*) str;
    // Setup hash
    uint32_t hash = HASH_FNV1A_OFFSET_BASE;
    // Compute it
    while (*buf)
    {
        hash ^= (uint32_t) *buf++;
        hash *= HASH_FNV1A_PRIME;
    }
    return hash;
}
