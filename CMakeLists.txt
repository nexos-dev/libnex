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
project(libnex VERSION 0.0.1)

# Various options
option(BUILD_SHARED_LIBS "Specifies if shared libraries should be built" OFF)
option(LIBNEX_ENABLE_TESTS "Specifies if the test suite should be built" OFF)
option(LIBNEX_BAREMETAL "Specifies if the build should be tailored to baremetal platforms" OFF)

if(${LIBNEX_ENABLE_TESTS} AND NOT ${CMAKE_CROSSCOMPILING})
    enable_testing()
endif()

# Find the NexNix SDK
find_package(NexNixSdk REQUIRED)

# Include modules
include(GNUInstallDirs)
include(NexTest)
include(CheckSymbolExists)
include(CheckCSourceCompiles)
include(CMakePackageConfigHelpers)

# See if tests should be enabled
if(${LIBNEX_ENABLE_TESTS})
    nextest_enable_tests()
endif()

# Check for different functions
check_symbol_exists(strlcpy "string.h" HAVE_BSD_STRING)
check_symbol_exists(setprogname "stdlib.h" HAVE_PROGNAME)
check_symbol_exists(pthread_self "pthread.h" HAVE_PTHREAD)

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
     src/object.c)

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
     ${CMAKE_SOURCE_DIR}/include/object.h)

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

# Create the library
add_library(nex ${LIBNEX_SOURCES})

# Set compiler flags

# Set SOName
set_target_properties(nex PROPERTIES VERSION ${PROJECT_VERSION} SOVERSION ${PROJECT_VERSION})
# Include the directories
target_include_directories(nex PRIVATE ${LIBNEX_INCLUDE_DIRS})

# libnex_config.h is in the binary directory for the libnex build, 
# but in ${CMAKE_INSTALL_INCLUDEDIR}/include/libnex for consumers. Ensure the headers differentiate
# between them
target_compile_definitions(nex PRIVATE "IN_LIBNEX")

# Ensure symbols are hidden by default
if(BUILD_SHARED_LIBS)
    set_target_properties(nex PROPERTIES C_VISIBILITY_PRESET hidden)
endif()

# Check for __declspec(dllexport)
check_c_source_compiles([=[
    __declspec(dllexport) int f() {}
    int main() {}
]=] HAVE_DECLSPEC_EXPORT)

if(NOT HAVE_DECLSPEC_EXPORT)
    # Check for __attribute__((visibility("default")))
    check_c_source_compiles([=[
        __attribute__((visibility("default"))) int f() {}
        int main(){}
    ]=] HAVE_VISIBILITY)
endif()

# Install pkgconfig script
install(FILES ${CMAKE_BINARY_DIR}/nex.pc DESTINATION ${CMAKE_INSTALL_DATADIR}/pkgconfig)

# Install the library
install(TARGETS nex)

# Figure out library type
if(BUILD_SHARED_LIBS)
    set(LIBNEX_LIBTYPE "SHARED")
    set(LIBNEX_LIBRARY_FILE "${CMAKE_INSTALL_FULL_LIBDIR}/libnex.so")
else()
    set(LIBNEX_LIBTYPE "STATIC")
    set(LIBNEX_LIBRARY_FILE "${CMAKE_INSTALL_FULL_LIBDIR}/libnex.a")
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
     endian
     list
     bits)

if(NOT HAVE_BSD_STRING)
    list(APPEND LIBNEX_TESTS strlcat strlcpy)
endif()

foreach(test ${LIBNEX_TESTS})
    nextest_add_library_test(NAME ${test}
                             SOURCE tests/${test}.c
                             LIBS nex
                             INCLUDES ${LIBNEX_INCLUDE_DIRS}
                             DEFINES IN_LIBNEX)
endforeach()
