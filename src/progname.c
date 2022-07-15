/*
    progname.c - contains functions to work with program name
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

/// @file progname.c

#include <libnex/error.h>
#include <libnex/libnex_config.h>

static char* progName;    ///< The name of this program. It is used when printing out error primarily

/**
 * @brief Sets the name of this program
 * @param[in] name the name of the program
 */
LIBNEX_PUBLIC void setprogname (char* name)
{
    progName = name;
}

/**
 * @brief Gets the name of this program
 * @return The name of the program
 */
LIBNEX_PUBLIC const char* getprogname()
{
    return progName;
}
