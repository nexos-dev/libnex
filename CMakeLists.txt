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

cmake_minimum_required(VERSION 3.00)
cmake_policy(SET CMP0054 NEW)
project(libnex VERSION 0.0.1)
enable_language(C)
set(CMAKE_C_STANDARD 11)

# Various options
option(BUILD_SHARED_LIBS "Specifies if shared libraries should be built" OFF)
option(LIBNEX_ENABLE_TESTS "Specifies if the test suite should be built" OFF)
option(LIBNEX_BAREMETAL "Specifies if the build should be tailored to baremetal platforms" OFF)

if(${LIBNEX_ENABLE_TESTS} AND NOT ${CMAKE_CROSSCOMPILING})
    enable_testing()
endif()

# Check if we should use the SDK or not
if(CMAKE_SYSTEM_NAME STREQUAL "NexNix")
    find_package(NexNixSdk REQUIRED)
else()
    list(APPEND CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/nextest")
    include_directories(nextest)
endif()

# Include modules
include(GNUInstallDirs)
include(NexTest)
include(CheckSymbolExists)
include(CMakePackageConfigHelpers)
include(SdkCompilerTest)

# See if tests should be enabled
if(${LIBNEX_ENABLE_TESTS})
    nextest_enable_tests()
endif()

# Check for different functions
check_symbol_exists(strlcpy "string.h" HAVE_BSD_STRING)
check_symbol_exists(setprogname "stdlib.h" HAVE_PROGNAME)
check_symbol_exists(pthread_self "pthread.h" HAVE_PTHREAD)
if(NOT HAVE_PTHREAD)
    check_symbol_exists(EnterCriticalSection "windows.h" HAVE_WIN32_THREADS)
endif()
# Check for stat
check_symbol_exists(stat "sys/stat.h" HAVE_UNIX_STAT)
if(NOT HAVE_UNIX_STAT AND NOT LIBNEX_BAREMETAL)
    check_symbol_exists(_stat "sys/stat.h" HAVE_WIN32_STAT)
    if(NOT HAVE_WIN32_STAT)
        message(FATAL_ERROR "Stat is required")
    endif()
endif()

# Configure the header
configure_file(${CMAKE_SOURCE_DIR}/src/libnex_config.in.h ${CMAKE_BINARY_DIR}/libnex_config.h)
configure_file(${CMAKE_SOURCE_DIR}/data/nex.pc.in ${CMAKE_BINARY_DIR}/nex.pc @ONLY)

set_property(DIRECTORY APPEND PROPERTY CMAKE_CONFIGURE_DEPENDS
                 ${CMAKE_SOURCE_DIR}/data/nex.pc.in
                 ${CMAKE_SOURCE_DIR}/src/libnex_config.in.h
                 ${CMAKE_SOURCE_DIR}/data/LibNexConfig.cmake.in)

# Setup different source code types
list(APPEND LIBNEX_SOURCES_ALWAYS
     src/list.c
     src/lock.c
     src/endian.c
     src/object.c
     src/char32.c
     src/unicode.c)

list(APPEND LIBNEX_HOSTED_SOURCES
    src/textstream.c
    src/getopt.c
    src/error.c
    src/safemalloc.c)

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

# Create the include files list
list(APPEND LIBNEX_HEADERS_ALWAYS
     ${CMAKE_SOURCE_DIR}/include/libnex/container.h
     ${CMAKE_SOURCE_DIR}/include/libnex/decls.h
     ${CMAKE_SOURCE_DIR}/include/libnex/endian.h
     ${CMAKE_SOURCE_DIR}/include/libnex/list.h
     ${CMAKE_SOURCE_DIR}/include/libnex/safestring.h
     ${CMAKE_SOURCE_DIR}/include/libnex/bits.h
     ${CMAKE_SOURCE_DIR}/include/libnex/object.h
     ${CMAKE_SOURCE_DIR}/include/libnex/lock.h
     ${CMAKE_SOURCE_DIR}/include/libnex/unicode.h)

# Figure out which libnex.h to use
if(LIBNEX_BAREMETAL)
    configure_file(${CMAKE_SOURCE_DIR}/src/libnex_baremetal.h ${CMAKE_BINARY_DIR}/libnex.h)
    set_property(DIRECTORY APPEND PROPERTY CMAKE_CONFIGURE_DEPENDS 
                 ${CMAKE_SOURCE_DIR}/src/libnex_baremetal.h)
else()
    configure_file(${CMAKE_SOURCE_DIR}/src/libnex_hosted.h ${CMAKE_BINARY_DIR}/libnex.h)
    set_property(DIRECTORY APPEND PROPERTY CMAKE_CONFIGURE_DEPENDS 
                 ${CMAKE_SOURCE_DIR}/src/libnex_hosted.h)
endif()
set(LIBNEX_HEADER ${CMAKE_BINARY_DIR}/libnex.h)

# Add hosted headers
list(APPEND LIBNEX_HEADERS_HOSTED
     ${CMAKE_SOURCE_DIR}/include/libnex/getopt.h
     ${CMAKE_SOURCE_DIR}/include/libnex/textstream.h
     ${CMAKE_SOURCE_DIR}/include/libnex/progname.h
     ${CMAKE_SOURCE_DIR}/include/libnex/error.h
     ${CMAKE_SOURCE_DIR}/include/libnex/safemalloc.h)

# Set the headers
list(APPEND LIBNEX_HEADERS ${LIBNEX_HEADERS_ALWAYS})
if(NOT LIBNEX_BAREMETAL)
    list(APPEND LIBNEX_HEADERS ${LIBNEX_HEADERS_HOSTED})
endif()

# Setup include directories
list(APPEND LIBNEX_INCLUDE_DIRS ${CMAKE_SOURCE_DIR}/include ${CMAKE_BINARY_DIR} 
                            ${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_INCLUDEDIR}
                            ${NEXNIXSDK_INCLUDE_DIRS})

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

# Set SOName
set_target_properties(nex PROPERTIES VERSION ${PROJECT_VERSION} SOVERSION ${PROJECT_VERSION})
# Include the directories
target_include_directories(nex PRIVATE ${LIBNEX_INCLUDE_DIRS})

# libnex_config.h is in the binary directory for the libnex build, 
# but in ${CMAKE_INSTALL_INCLUDEDIR}/include/libnex for consumers. Ensure the headers differentiate
# between them. This also is needed for deciding between dllexport and dllimport
target_compile_definitions(nex PRIVATE "IN_LIBNEX")

# Ensure symbols are hidden by default
if(BUILD_SHARED_LIBS)
    set_target_properties(nex PROPERTIES C_VISIBILITY_PRESET hidden)
endif()

# Check for a visibility method
check_library_visibility(HAVE_DECLSPEC_EXPORT HAVE_VISIBILITY)

# Install pkgconfig script
install(FILES ${CMAKE_BINARY_DIR}/nex.pc DESTINATION ${CMAKE_INSTALL_DATADIR}/pkgconfig)

# Install the library
install(TARGETS nex)

# Figure out library type
if(BUILD_SHARED_LIBS)
    set(LIBNEX_LIBTYPE "SHARED")
    set(LIBNEX_LIBRARY_FILE 
        "${CMAKE_INSTALL_FULL_LIBDIR}/${CMAKE_SHARED_LIBRARY_PREFIX}nex.${CMAKE_SHARED_LIBRARY_SUFFIX}")
else()
    set(LIBNEX_LIBTYPE "STATIC")
    set(LIBNEX_LIBRARY_FILE 
        "${CMAKE_INSTALL_FULL_LIBDIR}/${CMAKE_STATIC_LIBRARY_PREFIX}nex.${CMAKE_STATIC_LIBRARY_SUFFIX}")
endif()

# Install header files
install(FILES ${LIBNEX_HEADERS} DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/libnex)
install(FILES ${LIBNEX_HEADER} DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})
install(FILES ${CMAKE_BINARY_DIR}/libnex_config.h DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/libnex)

# Configure package configuration file
configure_package_config_file(${CMAKE_SOURCE_DIR}/data/LibNexConfig.cmake.in 
                              ${CMAKE_BINARY_DIR}/LibNexConfig.cmake
                              INSTALL_DESTINATION ${CMAKE_INSTALL_DATADIR}/${PROJECT_NAME}/cmake
                              PATH_VARS CMAKE_INSTALL_INCLUDEDIR)

write_basic_package_version_file(${CMAKE_BINARY_DIR}/LibNexConfigVersion.cmake
                                 COMPATIBILITY AnyNewerVersion)

# Install cmake package files
install(FILES ${CMAKE_BINARY_DIR}/LibNexConfig.cmake 
        DESTINATION ${CMAKE_INSTALL_DATADIR}/${PROJECT_NAME}/cmake)
install(FILES ${CMAKE_BINARY_DIR}/LibNexConfigVersion.cmake 
        DESTINATION ${CMAKE_INSTALL_DATADIR}/${PROJECT_NAME}/cmake)

# Figure out which tests to run
list(APPEND LIBNEX_TESTS
     endian list
     bits object
     char32 unicode
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
                             LIBS nex
                             INCLUDES ${LIBNEX_INCLUDE_DIRS}
                             DEFINES IN_LIBNEX
                             WORKDIR ${CMAKE_SOURCE_DIR}/tests)
endforeach()
