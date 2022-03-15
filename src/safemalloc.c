/*
    safemalloc.c - contains malloc_s and friends
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

/// @file safemalloc.c

#include <libnex/error.h>
#include <libnex/progname.h>
#include <libnex/safemalloc.h>
#include <stdlib.h>

/**
 * @brief Allocates memory, checking for failure of malloc.
 *
 * Allocates memory.
 * If malloc fails, it exits. To free allocated memory,
 * create an atexit() handler.
 * @param[in] sz specifies the allocation size
 * @return The allocated block of memory
 */
LIBNEX_PUBLIC void* malloc_s (size_t sz)
{
    // Allocate the memory
    void* data = malloc (sz);
    if (!data)
        error ("memory allocation failed\n");
    return data;
}

/**
 * @brief Reallocates memory, checking for failure of realloc.
 *
 * Resizes memory.
 * If realloc fails, it exits. To free allocated memory,
 * create an atexit() handler.
 * @param[in] sz specifies the allocation size
 * @param[inout] ptr specifies the pointer to resize
 * @return The allocated block of memory
 */
LIBNEX_PUBLIC void* realloc_s (void* ptr, size_t sz)
{
    // Allocate the memory
    void* data = realloc (ptr, sz);
    if (!data)
        error ("memory allocation failed\n");
    return data;
}

/**
 * @brief Allocates and intializes memory, checking for failure of calloc.
 *
 * Initializes allocated memory.
 * If calloc fails, it exits. To free allocated memory,
 * create an atexit() handler.
 * @param[in] sz specifies the allocation size
 * @return The allocated block of memory
 */
LIBNEX_PUBLIC void* calloc_s (size_t sz)
{
    // Allocate the memory
    void* data = calloc (1, sz);
    if (!data)
        error ("memory allocation failed\n");
    return data;
}
