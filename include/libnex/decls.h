/*
    decls.h - contains code for C++ declaration safety
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

/// @file decls.h

#ifndef _DECLS_H
#define _DECLS_H

#ifdef __cplusplus
#define __DECL_START \
    extern "C"       \
    {
#define __DECL_END }
#else
#define __DECL_START
#define __DECL_END
#endif

#endif
