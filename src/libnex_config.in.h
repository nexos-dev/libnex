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
#cmakedefine HAVE_C11_THREADS
#cmakedefine HAVE_VISIBILITY
#cmakedefine HAVE_DECLSPEC_EXPORT
#cmakedefine LIBNEX_ENABLE_NLS
#ifdef LIBNEX_ENABLE_NLS
#define LIBNEX_LOCALE_BASE "@LIBNEX_LOCALE_BASE@"
#endif

// Get visibility stuff right
#ifdef HAVE_VISIBILITY
#define LIBNEX_PUBLIC __attribute__ ((visibility ("default")))
#elif defined HAVE_DECLSPEC_EXPORT
#ifdef IN_LIBNEX
#define LIBNEX_PUBLIC __declspec(dllexport)
#else
#define LIBNEX_PUBLIC __declspec(dllimport)
#endif
#else
#define LIBNEX_PUBLIC
#endif

// Ensure that ssize_t is defined
#ifdef WIN32
#include <basetsd.h>
typedef SSIZE_T ssize_t;
#else
#include <sys/types.h>
#endif

// Make sure stat is defined right
#ifdef HAVE_WIN32_STAT
#define stat   _stat
#define stat_t struct _stat
#else
#define stat_t struct stat
#endif

// Make internal.h visible if we are in libnex
#ifdef IN_LIBNEX
#include "internal.h"
#endif

#endif
