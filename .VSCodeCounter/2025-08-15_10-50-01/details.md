# Details

Date : 2025-08-15 10:50:01

Directory /home/nexos/source/libnex

Total : 65 files,  4291 codes, 2514 comments, 774 blanks, all 7579 lines

[Summary](results.md) / Details / [Diff Summary](diff.md) / [Diff Details](diff-details.md)

## Files
| filename | language | code | comment | blank | total |
| :--- | :--- | ---: | ---: | ---: | ---: |
| [CMakeLists.txt](/CMakeLists.txt) | CMake | 304 | 0 | 50 | 354 |
| [README.md](/README.md) | Markdown | 2 | 0 | 1 | 3 |
| [cmake/GettextHelp.cmake](/cmake/GettextHelp.cmake) | CMake | 129 | 0 | 11 | 140 |
| [cmake/NexTest.cmake](/cmake/NexTest.cmake) | CMake | 47 | 0 | 9 | 56 |
| [cmake/SdkCompilerTest.cmake](/cmake/SdkCompilerTest.cmake) | CMake | 29 | 0 | 6 | 35 |
| [cmake/nextest.h](/cmake/nextest.h) | C++ | 21 | 21 | 8 | 50 |
| [include/libnex/array.h](/include/libnex/array.h) | C | 46 | 79 | 20 | 145 |
| [include/libnex/base.h](/include/libnex/base.h) | C++ | 30 | 25 | 12 | 67 |
| [include/libnex/bits.h](/include/libnex/bits.h) | C++ | 14 | 107 | 15 | 136 |
| [include/libnex/char32.h](/include/libnex/char32.h) | C++ | 24 | 134 | 22 | 180 |
| [include/libnex/container.h](/include/libnex/container.h) | C++ | 4 | 18 | 5 | 27 |
| [include/libnex/crc32.h](/include/libnex/crc32.h) | C++ | 10 | 11 | 7 | 28 |
| [include/libnex/decls.h](/include/libnex/decls.h) | C | 14 | 18 | 6 | 38 |
| [include/libnex/endian.h](/include/libnex/endian.h) | C++ | 28 | 94 | 19 | 141 |
| [include/libnex/error.h](/include/libnex/error.h) | C++ | 10 | 34 | 9 | 53 |
| [include/libnex/getopt.h](/include/libnex/getopt.h) | C++ | 41 | 14 | 15 | 70 |
| [include/libnex/hash.h](/include/libnex/hash.h) | C++ | 63 | 105 | 29 | 197 |
| [include/libnex/list.h](/include/libnex/list.h) | C++ | 61 | 159 | 27 | 247 |
| [include/libnex/lock.h](/include/libnex/lock.h) | C++ | 23 | 43 | 12 | 78 |
| [include/libnex/object.h](/include/libnex/object.h) | C++ | 28 | 103 | 19 | 150 |
| [include/libnex/progname.h](/include/libnex/progname.h) | C++ | 11 | 26 | 12 | 49 |
| [include/libnex/safemalloc.h](/include/libnex/safemalloc.h) | C++ | 21 | 43 | 13 | 77 |
| [include/libnex/safestring.h](/include/libnex/safestring.h) | C++ | 12 | 41 | 11 | 64 |
| [include/libnex/stringref.h](/include/libnex/stringref.h) | C++ | 24 | 43 | 13 | 80 |
| [include/libnex/textstream.h](/include/libnex/textstream.h) | C++ | 65 | 116 | 22 | 203 |
| [include/libnex/unicode.h](/include/libnex/unicode.h) | C++ | 35 | 98 | 21 | 154 |
| [include/uchar.h](/include/uchar.h) | C++ | 21 | 1 | 11 | 33 |
| [src/array.c](/src/array.c) | C | 256 | 55 | 15 | 326 |
| [src/char32.c](/src/char32.c) | C | 204 | 35 | 17 | 256 |
| [src/codepages/win1252toUtf8.h](/src/codepages/win1252toUtf8.h) | C | 36 | 20 | 3 | 59 |
| [src/crc32.c](/src/crc32.c) | C | 38 | 7 | 4 | 49 |
| [src/endian.c](/src/endian.c) | C | 89 | 24 | 13 | 126 |
| [src/error.c](/src/error.c) | C | 23 | 22 | 7 | 52 |
| [src/getopt.c](/src/getopt.c) | C | 274 | 91 | 23 | 388 |
| [src/hash.c](/src/hash.c) | C | 165 | 46 | 21 | 232 |
| [src/internal.c](/src/internal.c) | C | 28 | 20 | 7 | 55 |
| [src/internal.h](/src/internal.h) | C++ | 17 | 19 | 9 | 45 |
| [src/libnex\_baremetal.h](/src/libnex_baremetal.h) | C++ | 12 | 18 | 5 | 35 |
| [src/libnex\_config.in.h](/src/libnex_config.in.h) | C++ | 42 | 21 | 8 | 71 |
| [src/libnex\_hosted.h](/src/libnex_hosted.h) | C++ | 17 | 18 | 5 | 40 |
| [src/list.c](/src/list.c) | C | 246 | 31 | 19 | 296 |
| [src/lock.c](/src/lock.c) | C | 52 | 43 | 7 | 102 |
| [src/object.c](/src/object.c) | C | 58 | 73 | 12 | 143 |
| [src/progname.c](/src/progname.c) | C | 11 | 26 | 6 | 43 |
| [src/safemalloc.c](/src/safemalloc.c) | C | 25 | 46 | 6 | 77 |
| [src/stringref.c](/src/stringref.c) | C | 22 | 18 | 5 | 45 |
| [src/strlcat.c](/src/strlcat.c) | C | 27 | 33 | 4 | 64 |
| [src/strlcpy.c](/src/strlcpy.c) | C | 22 | 32 | 4 | 58 |
| [src/textstream.c](/src/textstream.c) | C | 537 | 106 | 20 | 663 |
| [src/uchar.c](/src/uchar.c) | C | 85 | 3 | 12 | 100 |
| [src/unicode.c](/src/unicode.c) | C | 215 | 47 | 23 | 285 |
| [src/unicode/utf16stateTab.h](/src/unicode/utf16stateTab.h) | C++ | 19 | 26 | 9 | 54 |
| [src/unicode/utf8stateTab.h](/src/unicode/utf8stateTab.h) | C++ | 18 | 25 | 9 | 52 |
| [tests/array.c](/tests/array.c) | C | 74 | 19 | 9 | 102 |
| [tests/bits.c](/tests/bits.c) | C | 26 | 18 | 5 | 49 |
| [tests/char32.c](/tests/char32.c) | C | 65 | 29 | 13 | 107 |
| [tests/endian.c](/tests/endian.c) | C | 79 | 20 | 16 | 115 |
| [tests/hash.c](/tests/hash.c) | C | 25 | 18 | 8 | 51 |
| [tests/list.c](/tests/list.c) | C | 73 | 36 | 13 | 122 |
| [tests/object.c](/tests/object.c) | C | 24 | 17 | 3 | 44 |
| [tests/stringref.c](/tests/stringref.c) | C | 26 | 18 | 4 | 48 |
| [tests/strlcat.c](/tests/strlcat.c) | C | 18 | 20 | 5 | 43 |
| [tests/strlcpy.c](/tests/strlcpy.c) | C | 19 | 22 | 6 | 47 |
| [tests/textstream.c](/tests/textstream.c) | C | 155 | 37 | 4 | 196 |
| [tests/unicode.c](/tests/unicode.c) | C | 52 | 22 | 10 | 84 |

[Summary](results.md) / Details / [Diff Summary](diff.md) / [Diff Details](diff-details.md)