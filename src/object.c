/*
    object.c - contains functions to work with objects
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

/**
 * @file object.c
 * So, just what is an object? An object is the abstraction which most classes in libnex (and hopefully users)
 * work with. Basically, an object is anything. The idea is that a structure includes a type Object_t
 * as its first member. Users or the implementation then use the object functions on this struct to allow for
 * reference counting, comparison, and other useful things. See source / header for more info
 */
