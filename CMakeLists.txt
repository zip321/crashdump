#
# INTEL CONFIDENTIAL
#
# Copyright 2019 Intel Corporation.
#
# This software and the related documents are Intel copyrighted materials, and
# your use of them is governed by the express license under which they were
# provided to you ("License"). Unless the License provides otherwise, you may
# not use, modify, copy, publish, distribute, disclose or transmit this software
# or the related documents without Intel's prior written permission.
#
# This software and the related documents are provided as is, with no express or
# implied warranties, other than those that are expressly stated in the License.
#

cmake_minimum_required(VERSION 3.6)
project(crashdump)

find_package(cJSON REQUIRED)
find_package(PkgConfig REQUIRED)
pkg_check_modules(SAFEC REQUIRED libsafec)
include_directories(${SAFEC_INCLUDE_DIRS})
link_directories(${SAFEC_LIBRARY_DIRS})

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_library(peci SHARED libpeci/libpeci.c)
target_link_libraries(peci ${SAFEC_LIBRARIES})

set_property(TARGET peci PROPERTY C_STANDARD 99)
target_include_directories(peci PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/libpeci)
set_target_properties(peci PROPERTIES VERSION "1.0" SOVERSION "1")

install(TARGETS peci DESTINATION lib)
install(FILES libpeci/libpeci.h DESTINATION include/peci)

add_executable(crashdump
               crashdump.cpp
               utils.cpp
               CrashdumpSections/AddressMap.cpp
               CrashdumpSections/TorDump.cpp
               CrashdumpSections/BigCore.cpp
               CrashdumpSections/MetaData.cpp
               CrashdumpSections/CoreMca.cpp
               CrashdumpSections/SqDump.cpp
               CrashdumpSections/Uncore.cpp
               CrashdumpSections/UncoreRegs.hpp
               CrashdumpSections/PowerManagement.cpp
               CrashdumpSections/UncoreMca.cpp)

target_include_directories(crashdump PRIVATE ${CMAKE_SOURCE_DIR})
set_property(TARGET crashdump PROPERTY C_STANDARD 99)
target_link_libraries(crashdump
                      cjson
                      peci
                      -lsystemd
                      pthread
                      -lstdc++fs
                      sdbusplus
                      ${SAFEC_LIBRARIES})

install(TARGETS crashdump
        RUNTIME DESTINATION bin
        LIBRARY DESTINATION lib
        ARCHIVE DESTINATION lib/static)

find_package(Boost 1.66 REQUIRED)
include_directories(${BOOST_SRC_DIR})

add_definitions(-DBOOST_ERROR_CODE_HEADER_ONLY)
add_definitions(-DBOOST_SYSTEM_NO_DEPRECATED)
add_definitions(-DBOOST_ALL_NO_LIB)
add_definitions(-DBOOST_NO_RTTI)
add_definitions(-DBOOST_NO_TYPEID)

add_executable(peci_cmds libpeci/peci_cmds.c)
add_dependencies(peci_cmds peci)
target_link_libraries(peci_cmds peci)

install(TARGETS peci_cmds
        RUNTIME DESTINATION bin
        LIBRARY DESTINATION lib
        ARCHIVE DESTINATION lib/static)
