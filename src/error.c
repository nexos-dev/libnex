/*
    error.c - contains error reporting functions
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

/// @file error.c

#include <libnex/error.h>
#include <libnex/progname.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>

LIBNEX_PUBLIC void error (char* msg, ...)
{
    // Declare the argument list
    va_list ap;
    va_start (ap, msg);

    // Print it out
    fprintf (stderr, "%s: ", getprogname());
    vfprintf (stderr, msg, ap);
    fprintf (stderr, "\n");
    va_end (ap);
}

LIBNEX_PUBLIC void warn (char* msg, ...)
{
    // Declare the argument list
    va_list ap;
    va_start (ap, msg);

    // Print it out
    printf ("%s: ", getprogname());
    vfprintf (stderr, msg, ap);
    printf ("\n");
    va_end (ap);
}
