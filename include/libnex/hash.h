/*
    hash.h - contains hash table interface
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

#ifndef _HASH_H
#define _HASH_H

#include <libnex/decls.h>
#include <libnex/libnex_config.h>
#include <stddef.h>
#include <stdint.h>

__DECL_START

/**
 * @brief Produces an FNV-1a hash for a buffer
 * HashCreateHash computes the FNV-1a hash for sz bytes of buf
 * @param buf buffer to compute hash of
 * @param sz number of bytes to compute hash of
 * @return The 32-bit FNV-1a hash
 */
uint32_t HashCreateHash (const void* buf, size_t sz);

/**
 * @brief Produces an FNV-1a hash for a string
 * @param buf bstring to compute hash of
 * @return The 32-bit FNV-1a hash
 */
uint32_t HashCreateHashStr (const char* str);

__DECL_END

#endif
