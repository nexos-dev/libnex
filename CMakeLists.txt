#[[
    CMakeLists.txt - contains CMake build code for libnex
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
    limitations under the License
]]

cmake_minimum_required(VERSION 3.7)
cmake_policy(SET CMP0054 NEW)
project(libnex VERSION 0.0.1)
enable_language(C)
set(CMAKE_C_STANDARD 11)

# Various options
option(BUILD_SHARED_LIBS "Specifies if shared libraries should be built" OFF)
option(LIBNEX_ENABLE_TESTS "Specifies if the test suite should be built" OFF)
option(LIBNEX_BAREMETAL "Specifies if the build should be tailored to baremetal platforms" OFF)
option(LIBNEX_ENABLE_NLS "Specifies if NLS support should be compiled into libnex" ON)
option(LIBNEX_BUILDONLY "Specifies if libnex should be built without installing" OFF)

if(LIBNEX_BUILDONLY AND LIBNEX_ENABLE_NLS)
    # Turn NLS off
    message(STATUS "LIBNEX_BUILDONLY specified, turning off NLS")
    set(LIBNEX_ENABLE_NLS OFF)
endif()

if(LIBNEX_BUILDONLY AND BUILD_SHARED_LIBS)
    message(STATUS "LIBNEX_BUILDONLY specified, turning off BUILD_SHARED_LIBS")
    set(BUILD_SHARED_LIBS OFF)
endif()

if(${LIBNEX_ENABLE_TESTS} AND NOT ${CMAKE_CROSSCOMPILING})
    enable_testing()
endif()

# Check if we should use the SDK or not
if(CMAKE_SYSTEM_NAME STREQUAL "NexNix")
    find_package(NexNixSdk REQUIRED)
else()
    list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")
    include_directories(cmake)
endif()

# Include modules
include(GNUInstallDirs)
include(NexTest)
include(CheckSymbolExists)
include(CMakePackageConfigHelpers)
include(SdkCompilerTest)
include(GettextHelp)

# Find libintl
find_package(Intl)
if(NOT Intl_FOUND)
    message(WARNING "libintl not found, compiling without NLS (multilingual) support")
    set(LIBNEX_ENABLE_NLS OFF)
endif()

# Find GNU gettext
gettext_check(GETTEXT_FOUND)
if(NOT GETTEXT_FOUND)
    message(WARNING "GNU gettext not found, compiling without NLS")
    set(LIBNEX_ENABLE_NLS OFF)
else()
    message(STATUS "Found GNU gettext")
endif()

if(LIBNEX_ENABLE_NLS)
    # Find the locale directory base
    set(LIBNEX_LOCALE_BASE "${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_DATADIR}/locale/")

    # Check which locales should be enabled
    set(LIBNEX_SUPPORTED_LOCALES "en;es" CACHE STRING "Languages supported by libnex")
    if(NOT LIBNEX_ENABLED_LOCALES)
        set(LIBNEX_ENABLED_LOCALES "${LIBNEX_SUPPORTED_LOCALES}" CACHE STRING "Languages to use in libnex")
    endif()
    # Validate locales
    foreach(locale ${LIBNEX_ENABLED_LOCALES})
        # Strip en from list. en_US is the locale that the message IDs are in, meaning that we
        # don't need a l10n for that
        if("${locale}" STREQUAL "en")
            list(REMOVE_ITEM LIBNEX_ENABLED_LOCALES "en")
        endif()
        list(FIND LIBNEX_SUPPORTED_LOCALES ${locale} locale_supported)
        if("${locale_supported}" STREQUAL "-1")
            message(FATAL_ERROR "Locale ${locale} not supported")
        endif()
    endforeach()
endif()

# See if tests should be enabled
if(${LIBNEX_ENABLE_TESTS})
    nextest_enable_tests()
endif()

# Check for different functions
check_symbol_exists(strlcpy "string.h" HAVE_BSD_STRING)
check_symbol_exists(setprogname "stdlib.h" HAVE_PROGNAME)
check_symbol_exists(mtx_init "threads.h" HAVE_C11_THREADS)
check_symbol_exists(pthread_mutex_lock "pthread.h" HAVE_PTHREADS)
check_symbol_exists(c32rtomb "uchar.h" HAVE_UCHAR)
# Check for stat
check_symbol_exists(stat "sys/stat.h" HAVE_UNIX_STAT)
if(NOT HAVE_UNIX_STAT AND NOT LIBNEX_BAREMETAL)
    check_symbol_exists(_stat "sys/stat.h" HAVE_WIN32_STAT)
    if(NOT HAVE_WIN32_STAT)
        message(FATAL_ERROR "Stat is required")
    endif()
endif()

# Configure the header
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/src/libnex_config.in.h ${CMAKE_BINARY_DIR}/libnex/libnex_config.h)
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/data/nex.pc.in ${CMAKE_BINARY_DIR}/nex.pc @ONLY)

set_property(DIRECTORY APPEND PROPERTY CMAKE_CONFIGURE_DEPENDS
                 ${CMAKE_CURRENT_SOURCE_DIR}/data/nex.pc.in
                 ${CMAKE_CURRENT_SOURCE_DIR}/src/libnex_config.in.h
                 ${CMAKE_CURRENT_SOURCE_DIR}/data/LibNexConfig.cmake.in)

# Setup different source code types
list(APPEND LIBNEX_SOURCES_ALWAYS
     src/list.c
     src/array.c
     src/lock.c
     src/endian.c
     src/object.c
     src/unicode.c
     src/crc32.c
     src/hash.c
     src/stringref.c)

list(APPEND LIBNEX_HOSTED_SOURCES
    src/textstream.c
    src/getopt.c
    src/error.c
    src/char32.c
    src/safemalloc.c
    src/internal.c)

if(NOT HAVE_UCHAR)
    list(APPEND LIBNEX_HOSTED_SOURCES src/uchar.c)
endif()

# If the host lacks some functions, add them
if(NOT HAVE_PROGNAME)
    list(APPEND LIBNEX_HOSTED_SOURCES
         src/progname.c)
endif()

if(NOT HAVE_BSD_STRING)
    list(APPEND LIBNEX_SOURCES_ALWAYS
         src/strlcpy.c
         src/strlcat.c)
endif()

# Create the main source code list
list(APPEND LIBNEX_SOURCES ${LIBNEX_SOURCES_ALWAYS})
if(NOT LIBNEX_BAREMETAL)
    list(APPEND LIBNEX_SOURCES ${LIBNEX_HOSTED_SOURCES})
endif()

# Create the PO file lists
if(LIBNEX_ENABLE_NLS)
    gettext_files(DOMAIN libnex 
                  OUTPUT "${CMAKE_CURRENT_SOURCE_DIR}/src/po" 
                  LANGS "${LIBNEX_ENABLED_LOCALES}"
                  SOURCES "${LIBNEX_SOURCES}")
endif()

# Create the include files list
list(APPEND LIBNEX_HEADERS_ALWAYS
     ${CMAKE_CURRENT_SOURCE_DIR}/include/libnex/container.h
     ${CMAKE_CURRENT_SOURCE_DIR}/include/libnex/base.h
     ${CMAKE_CURRENT_SOURCE_DIR}/include/libnex/decls.h
     ${CMAKE_CURRENT_SOURCE_DIR}/include/libnex/endian.h
     ${CMAKE_CURRENT_SOURCE_DIR}/include/libnex/list.h
     ${CMAKE_CURRENT_SOURCE_DIR}/include/libnex/array.h
     ${CMAKE_CURRENT_SOURCE_DIR}/include/libnex/safestring.h
     ${CMAKE_CURRENT_SOURCE_DIR}/include/libnex/bits.h
     ${CMAKE_CURRENT_SOURCE_DIR}/include/libnex/object.h
     ${CMAKE_CURRENT_SOURCE_DIR}/include/libnex/lock.h
     ${CMAKE_CURRENT_SOURCE_DIR}/include/libnex/unicode.h
     ${CMAKE_CURRENT_SOURCE_DIR}/include/libnex/crc32.h
     ${CMAKE_CURRENT_SOURCE_DIR}/include/libnex/hash.h
     ${CMAKE_CURRENT_SOURCE_DIR}/include/libnex/stringref.h
     ${CMAKE_CURRENT_SOURCE_DIR}/include/libnex/safemalloc.h)

# Figure out which libnex.h to use
if(LIBNEX_BAREMETAL)
    configure_file(${CMAKE_CURRENT_SOURCE_DIR}/src/libnex_baremetal.h ${CMAKE_BINARY_DIR}/libnex.h)
    set_property(DIRECTORY APPEND PROPERTY CMAKE_CONFIGURE_DEPENDS 
                 ${CMAKE_CURRENT_SOURCE_DIR}/src/libnex_baremetal.h)
else()
    configure_file(${CMAKE_CURRENT_SOURCE_DIR}/src/libnex_hosted.h ${CMAKE_BINARY_DIR}/libnex.h)
    set_property(DIRECTORY APPEND PROPERTY CMAKE_CONFIGURE_DEPENDS 
                 ${CMAKE_CURRENT_SOURCE_DIR}/src/libnex_hosted.h)
endif()
set(LIBNEX_HEADER ${CMAKE_BINARY_DIR}/libnex.h)

# Add hosted headers
list(APPEND LIBNEX_HEADERS_HOSTED
     ${CMAKE_CURRENT_SOURCE_DIR}/include/libnex/getopt.h
     ${CMAKE_CURRENT_SOURCE_DIR}/include/libnex/textstream.h
     ${CMAKE_CURRENT_SOURCE_DIR}/include/libnex/progname.h
     ${CMAKE_CURRENT_SOURCE_DIR}/include/libnex/error.h
     ${CMAKE_CURRENT_SOURCE_DIR}/include/libnex/char32.h)

# Set the headers
list(APPEND LIBNEX_HEADERS ${LIBNEX_HEADERS_ALWAYS})
if(NOT LIBNEX_BAREMETAL)
    list(APPEND LIBNEX_HEADERS ${LIBNEX_HEADERS_HOSTED})
endif()

# Setup include directories
list(APPEND LIBNEX_PRIVATE_INCLUDE_DIRS ${CMAKE_BINARY_DIR} 
                            ${CMAKE_CURRENT_SOURCE_DIR}/src ${Intl_INCLUDE_DIRS}
                            ${NEXNIXSDK_INCLUDE_DIRS})
list(APPEND LIBNEX_INTERFACE_INCLUDE_DIRS ${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_INCLUDEDIR})
list(APPEND LIBNEX_PUBLIC_INCLUDE_DIRS ${CMAKE_CURRENT_SOURCE_DIR}/include)
list(APPEND LIBNEX_INSTALL_INCLUDE_DIRS ${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_INCLUDEDIR})

# Setup libraries to link with
if(Intl_LIBRARIES)
    list(APPEND LIBNEX_LIBRARIES ${Intl_LIBRARIES})
endif()

# MSVC specific options
if(MSVC)
    # Ensure CL treats files as UTF-8. We need this because some source files have Unicode text in them
    # CL will erroneously treat them as Windows-1252 without passing /utf-8
    add_compile_options(/utf-8)
    # CL lets out annoying warning about fopen, getenv, strcpy, etc, etc, etc...
    # Tell it to be quiet
    add_compile_definitions(_CRT_SECURE_NO_WARNINGS)
endif()

# Create the library
add_library(nex ${LIBNEX_SOURCES})

# Ensure libnex is linked against required libraries
target_link_libraries(nex PUBLIC ${LIBNEX_LIBRARIES})

# Set SOName
set_target_properties(nex PROPERTIES VERSION ${PROJECT_VERSION} SOVERSION ${PROJECT_VERSION_MAJOR})
# Include the directories
target_include_directories(nex PRIVATE ${LIBNEX_PRIVATE_INCLUDE_DIRS})
target_include_directories(nex PUBLIC ${LIBNEX_PUBLIC_INCLUDE_DIRS})
target_include_directories(nex INTERFACE ${LIBNEX_INTERFACE_INCLUDE_DIRS})

# Ensure the headers differentiate between dllexport and dllimport
target_compile_definitions(nex PRIVATE "IN_LIBNEX")

# If baremetal, compile as PIE
if(LIBNEX_BAREMETAL AND NOT MSVC)
    target_compile_options(nex PRIVATE -fpie)
endif()

# Ensure symbols are hidden by default
if(BUILD_SHARED_LIBS)
    set_target_properties(nex PROPERTIES C_VISIBILITY_PRESET hidden)
endif()

# Check for a visibility method
check_library_visibility(HAVE_DECLSPEC_EXPORT HAVE_VISIBILITY)

if(NOT LIBNEX_BUILDONLY)
    # Install pkgconfig script
    install(FILES ${CMAKE_BINARY_DIR}/nex.pc DESTINATION ${CMAKE_INSTALL_DATADIR}/pkgconfig)
    # Install the library
    install(TARGETS nex)
endif()

# Figure out library type
if(BUILD_SHARED_LIBS)
    set(LIBNEX_LIBTYPE "SHARED")
    set(LIBNEX_LIBRARY_FILE 
        "${CMAKE_INSTALL_FULL_LIBDIR}/${CMAKE_SHARED_LIBRARY_PREFIX}nex${CMAKE_SHARED_LIBRARY_SUFFIX}")
    set(LIBNEX_HAVE_SONAME TRUE)
    set(LIBNEX_SONAME "${CMAKE_SHARED_LIBRARY_PREFIX}nex${CMAKE_SHARED_LIBRARY_SUFFIX}.${PROJECT_VERSION_MAJOR}")
else()
    set(LIBNEX_LIBTYPE "STATIC")
    set(LIBNEX_HAVE_SONAME FALSE)
    set(LIBNEX_LIBRARY_FILE 
        "${CMAKE_INSTALL_FULL_LIBDIR}/${CMAKE_STATIC_LIBRARY_PREFIX}nex${CMAKE_STATIC_LIBRARY_SUFFIX}")
endif()

if(NOT LIBNEX_BUILDONLY)
    # Install header files
    install(FILES ${LIBNEX_HEADERS} DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/libnex)
    install(FILES ${LIBNEX_HEADER} DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})
    if(NOT HAVE_UCHAR AND NOT LIBNEX_BAREMETAL)
        install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/include/uchar.h DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})
    endif()
    install(FILES ${CMAKE_BINARY_DIR}/libnex/libnex_config.h DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/libnex)
endif()

# Configure package configuration file
configure_package_config_file(${CMAKE_CURRENT_SOURCE_DIR}/data/LibNexConfig.cmake.in 
                              ${CMAKE_BINARY_DIR}/LibNexConfig.cmake
                              INSTALL_DESTINATION ${CMAKE_INSTALL_DATADIR}/${PROJECT_NAME}/cmake
                              PATH_VARS CMAKE_INSTALL_INCLUDEDIR)

write_basic_package_version_file(${CMAKE_BINARY_DIR}/LibNexConfigVersion.cmake
                                 COMPATIBILITY AnyNewerVersion)

# Install cmake package files
if(NOT LIBNEX_BUILDONLY)
    install(FILES ${CMAKE_BINARY_DIR}/LibNexConfig.cmake 
            DESTINATION ${CMAKE_INSTALL_DATADIR}/${PROJECT_NAME}/cmake)
    install(FILES ${CMAKE_BINARY_DIR}/LibNexConfigVersion.cmake 
            DESTINATION ${CMAKE_INSTALL_DATADIR}/${PROJECT_NAME}/cmake)
    # Install man pages
    install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/man DESTINATION ${CMAKE_INSTALL_DATADIR})
endif()

# Figure out which tests to run
list(APPEND LIBNEX_TESTS
     endian list
     bits object
     char32 unicode
     hash stringref
     array
     )

if(NOT HAVE_BSD_STRING)
    list(APPEND LIBNEX_TESTS strlcat strlcpy)
endif()

if(NOT LIBNEX_BAREMETAL)
    list(APPEND LIBNEX_TESTS textstream)
endif()

foreach(test ${LIBNEX_TESTS})
    nextest_add_library_test(NAME ${test}
                             SOURCE tests/${test}.c
                             LIBS nex pthread
                             INCLUDES ${LIBNEX_PRIVATE_INCLUDE_DIRS} ${LIBNEX_PUBLIC_INCLUDE_DIRS}
                             DEFINES IN_LIBNEX
                             WORKDIR ${CMAKE_CURRENT_SOURCE_DIR}/tests)
endforeach()
