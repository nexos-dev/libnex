/*
    base.h - contains macros that handle base stuff
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

/// @file base.h

#ifndef _BASE_H
#define _BASE_H

#include <libnex/libnex_config.h>
#include <stddef.h>

/// Used to specify that a parameter is unused
#define UNUSED(param) (void) (param);

/// To find the size of an array
#define ARRAY_SIZE(a) (sizeof (a) / sizeof ((a)[0]))

/// Specifies that a function will not return
#ifdef __GNUC__
#define NORETURN __attribute__ ((noreturn))
#else
#define NORETURN
#endif

/// Aligns a number to the specified power of two
static inline size_t AlignNumberUp (size_t num, size_t align)
{
    return (num + (align - 1)) & ~(align - 1);
}

/// Aligns a number down to the power of 2
static inline size_t AlignNumberDown (size_t num, size_t align)
{
    return (num + (align - 1)) & ~(align - 1);
}

// Libnex error codes
#define LIBNEX_ERR_NONE         0
#define LIBNEX_ERR_BOUNDS       1
#define LIBNEX_ERR_SYS          2
#define LIBNEX_ERR_BAD_PARAM    3
#define LIBNEX_ERR_BUF_SMALL    4
#define LIBNEX_ERR_NO_EXIST     5
#define LIBNEX_ERR_OUT_OF_SPACE 6
#define LIBNEX_ERR_OOM          7
#define LIBNEX_ERR_MAX          7

/// Gets error code
LIBNEX_PUBLIC int LibnexGetError();

#endif
