/*
    error.h - contains error reporting functions
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

/// @file error.h

#ifndef _ERROR_H
#define _ERROR_H

#ifdef IN_LIBNEX
#include <libnex_config.h>
#else
#include <libnex/libnex_config.h>
#endif
#include <libnex/base.h>
#include <libnex/decls.h>

__DECL_START

/**
 * @brief Reports an error to stderr.
 *
 * This function prints a message to the stderr, with the program name and an arbitrary amount of arguments,
 * followed by a newline
 * @param[in] msg the message to report
 * @param[in] ... the list of variable arguments
 */
LIBNEX_PUBLIC void error (char* msg, ...);

/**
 * @brief Reports a warning to stderr.
 *
 * This function prints a message to the stderr, with the program name and an arbitrary amount of arguments,
 * followed by a newline
 * @param[in] msg the message to report
 * @param[in] ... the list of variable arguments
 */
LIBNEX_PUBLIC void warn (char* msg, ...);

__DECL_END

#endif
