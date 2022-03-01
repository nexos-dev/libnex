/*
    char16.h - contains declarations to work char16_t* types
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

/// @file .h

#ifndef _CHAR32_H
#define _CHAR32_H

#include <libnex/decls.h>
#ifdef IN_LIBNEX
#include <libnex_config.h>
#else
#include <libnex/libnex_config.h>
#endif
#include <uchar.h>
#include <wchar.h>

__DECL_START

/**
 * @brief Converts a null terminated UTF-32 string to the host's multibyte format.
 *
 * c16stombs converts a null terminated char16_t based string to the host's multibyte format.
 * It does so internally with the function c16tomb
 *
 * @param mbStr the multibyte string to write out to
 * @param u32Str the UTF-32 string to read from
 * @param sz the size of the buffer pointed to by mbStr
 * @param state the multibyte state
 * @return The number of bytes used in mbStr
 */
PUBLIC ssize_t c16stombs (char* mbStr, const char16_t* u32str, size_t sz, mbstate_t* state);

/**
 * @brief Converts a null terminated multibyte string to a UTF-32 string
 *
 * mbstoc16s converts a null terminated multibyte based string to UTF-32.
 * It does so internally with the function mbtoc16
 *
 * @param u32Str the UTF-32 string to write to
 * @param mbStr the multibyte string to read from
 * @param sz the size of the buffer pointed to by u32Str
 * @param mbSz the maximum size of mbStr
 * @param state the multibyte state
 * @return The number of characters copied
 */
PUBLIC ssize_t mbstoc16s (char16_t* u32str, const char* mbStr, size_t sz, size_t mbSz, mbstate_t* state);

/**
 * @brief Converts a wide character string to a char16_t
 *
 * wcstoc16s converts a string from the host's wide format to a UTF-32 string
 * It does a wcsrtomb, follwed by a mbstoc16
 *
 * @param u32Str the UTF-32 string to write to
 * @param wcStr the wide string to read from
 * @param sz the size of the buffer pointed to by u32str
 * @return The number of characters copied
 */
PUBLIC ssize_t wcstoc16s (char16_t* u32str, const wchar_t* wcStr, size_t sz);

/**
 * @brief Converts a null terminated UTF-32 string to the host's wide format.
 *
 * c16stowcs converts a null terminated char16_t based string to the host's wide format.
 * It does so internally with the function with c16stombs, followed by a mbsrtowcs
 *
 * @param wcStr the wide string to write out to
 * @param u32Str the UTF-32 string to read from
 * @param sz the size of the buffer pointed to by wcStr
 * @return The number of bytes used in wcStr
 */
PUBLIC ssize_t c16stowcs (wchar_t* wcStr, const char16_t* u32str, size_t sz);

/**
 * @brief Gets the length of a
 * @param s the string to get the length of
 * @return The length of s. If c16len ran off the end of the string, SIZE_MAX
 */
PUBLIC size_t c16len (const char16_t* s);

/**
 * @brief Compares s1 and s2
 * @param s1 the first string
 * @param s2 the second string
 * @return If < 0 s1 has a lower value then s2; 0 means equal; If > 0 s1 has a greater then s2
 */
PUBLIC int c16cmp (const char16_t* s1, const char16_t* s2);

/**
 * @brief Compares n characters of s1 and s2
 * @param s1 the first string
 * @param s2 the second string
 * @param n the number of characters to compare
 * @return If < 0 s1 has a lower value then s2; 0 means equal; If > 0 s1 has a greater then s2
 */
PUBLIC int c16ncmp (const char16_t* s1, const char16_t* s2, size_t n);

/**
 * @brief Concatenates strings
 *
 * c16lcat takes the size of the valid buffer dest points to. It copies
 * src - c16len(src) - 1 bytes, null terminating the result
 *
 * @param[out] dest the string to write to
 * @param[in] src the string to read from
 * @param[in] size the size of the buffer dest points to
 * @return the lenght of dest plus the length of source
 */
PUBLIC size_t c16lcat (char16_t* dest, const char16_t* src, size_t size);

/**
 * @brief Copies strings
 *
 * c16lcpy takes a buffer size, and copies at most size - 1 bytes, padding with null character.
 * It returns the length of the buffer they tried to create.
 *
 * @param[out] dest the string to write out to
 * @param[in] src the string copy from
 * @param[in] size the size of dest
 * @return the length of src
 */
PUBLIC size_t c16lcpy (char16_t* dest, const char16_t* src, size_t size);

/**
 * @brief Finds the first occurence of c in str, returning a pointer to it
 * @param str the string to look in
 * @param c the character to look for
 * @return Pointer to first occurence
 */
PUBLIC char16_t* c16chr (const char16_t* str, char16_t c);

/**
 * @brief Finds the last occurence of c in str, returning a pointer to it
 * @param str the string to look in
 * @param c the character to look for
 * @return Pointer to last occurence
 */
PUBLIC char16_t* c16rchr (const char16_t* str, char16_t c);

/**
 * @brief Finds the first occurence of any character in s2 in s1
 * @param s1 the string to look in
 * @param s2 the characters to look for
 * @return Pointer to first occurence
 */
PUBLIC char16_t* c16pbrk (const char16_t* s1, const char16_t* s2);

/**
 * @brief Duplicates a string
 * @param str the string to duplicate
 * @return the duplicated string
 */
PUBLIC char16_t* c16dup (char16_t* str);

__DECL_END

#endif
