/*
    base.h - contains macros that handle base stuff
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

/// @file base.h

#ifndef _BASE_H
#define _BASE_H

// Used to specify that a parameter is unused
#define UNUSED(param) (void) (param);

// To find the size of an array
#define ARRAY_SIZE(a) (sizeof (a) / sizeof ((a)[0]))

#endif
