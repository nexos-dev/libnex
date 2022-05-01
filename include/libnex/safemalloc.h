/*
    safeMalloc.h - contains malloc_s, calloc_s, and realloc_s
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

/// @file safemalloc.h

#ifndef _SAFE_MALLOC_H
#define _SAFE_MALLOC_H

#ifdef IN_LIBNEX
#include <libnex_config.h>
#else
#include <libnex/libnex_config.h>
#endif

#ifndef LIBNEX_BAREMETAL

#include <libnex/decls.h>
#include <stddef.h>

__DECL_START

/**
 * @brief Allocates memory, checking for failure of malloc.
 *
 * Allocates memory.
 * If malloc fails, it prints an error
 * @param[in] sz specifies the allocation size
 * @return The allocated block of memory
 */
LIBNEX_PUBLIC __attribute__ ((malloc)) void* malloc_s (size_t sz);

/**
 * @brief Reallocates memory, checking for failure of realloc.
 *
 * Resizes memory.
 * If realloc fails, it prints an error
 * @param[in] sz specifies the allocation size
 * @param[inout] ptr specifies the pointer to resize
 * @return The allocated block of memory
 */
LIBNEX_PUBLIC void* realloc_s (void* ptr, size_t sz);

/**
 * @brief Allocates and intializes memory, checking for failure of calloc.
 *
 * Initializes allocated memory.
 * If calloc fails, it prints an error
 * @param[in] sz specifies the allocation size
 * @return The allocated block of memory
 */
LIBNEX_PUBLIC void* calloc_s (size_t sz);

__DECL_END

#else
#define malloc_s  malloc
#define calloc_s  calloc
#define realloc_s realloc
#endif

#endif
