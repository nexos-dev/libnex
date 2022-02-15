/*
    libnex_config.in.h - contains system configuration values
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

#ifndef _LIBNEX_CONFIG_H
#define _LIBNEX_CONFIG_H

#cmakedefine LIBNEX_BAREMETAL
#cmakedefine HAVE_BSD_STRING
#cmakedefine HAVE_PROGNAME
#cmakedefine HAVE_PTHREAD
#cmakedefine HAVE_VISIBILITY
#cmakedefine HAVE_DECLSPEC_EXPORT

// Decide the size of wchar_t
#include <wchar.h>
#ifndef WCHAR_MAX
#error Platform with wchar_t is required
#endif

#if WCHAR_MAX > 0x1000000
#define HAVE_WIDE_WCHAR
#else
#define HAVE_NARROW_WCHAR
#endif

// Get visibility stuff right
#ifdef HAVE_VISIBILITY
#define PUBLIC __attribute__ ((visibility ("default")))
#elif defined HAVE_DECLSPEC_EXPORT
#ifdef IN_LIBNEX
#define PUBLIC __declspec(dllexport)
#else
#define PUBLIC __declspec(dllimport)
#endif
#else
#define PUBLIC
#endif

#endif
