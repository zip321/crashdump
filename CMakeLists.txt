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

cmake_minimum_required (VERSION 3.6)
project (crashdump)

find_package (cJSON REQUIRED)
find_package (PkgConfig REQUIRED)
pkg_check_modules (SAFEC REQUIRED libsafec)
include_directories (${SAFEC_INCLUDE_DIRS})
link_directories (${SAFEC_LIBRARY_DIRS})

set (CMAKE_CXX_STANDARD 17)
set (CMAKE_CXX_STANDARD_REQUIRED ON)

set (SRC_FILES utils.cpp CrashdumpSections/AddressMap.cpp
     CrashdumpSections/TorDump.cpp CrashdumpSections/BigCore.cpp
     CrashdumpSections/MetaData.cpp CrashdumpSections/CoreMca.cpp
     CrashdumpSections/SqDump.cpp CrashdumpSections/Uncore.cpp
     CrashdumpSections/UncoreRegs.hpp CrashdumpSections/PowerManagement.cpp
     CrashdumpSections/UncoreMca.cpp CrashdumpSections/OemData.cpp)

add_executable (crashdump crashdump.cpp ${SRC_FILES})

option (
    CRASHDUMP_PRINT_UNFORMATTED
    "Use cJSON_PrintUnformatted() to print the crashdump contents without
         whitespace to reduce the file size."
    OFF
)

option (OEMDATA_SECTION
        "Adds oemdata Section to the crashdump contents, for OEM usage." OFF)

target_compile_definitions (crashdump PRIVATE
                            $<$<BOOL:${CRASHDUMP_PRINT_UNFORMATTED}>:
                            -DCRASHDUMP_PRINT_UNFORMATTED>
                            $<$<BOOL:${OEMDATA_SECTION}>: -DOEMDATA_SECTION>)

target_include_directories (crashdump PRIVATE ${CMAKE_SOURCE_DIR})
set_property (TARGET crashdump PROPERTY C_STANDARD 99)
target_link_libraries (crashdump cjson peci -lsystemd pthread -lstdc++fs
                       sdbusplus ${SAFEC_LIBRARIES})

install (TARGETS crashdump
         RUNTIME DESTINATION bin
         LIBRARY DESTINATION lib
         ARCHIVE DESTINATION lib/static)

find_package (Boost 1.66 REQUIRED)
include_directories (${BOOST_SRC_DIR})

add_definitions (-DBOOST_ERROR_CODE_HEADER_ONLY)
add_definitions (-DBOOST_SYSTEM_NO_DEPRECATED)
add_definitions (-DBOOST_ALL_NO_LIB)
add_definitions (-DBOOST_NO_RTTI)
add_definitions (-DBOOST_NO_TYPEID)

# Unit Tests
if (${CRASHDUMP_BUILD_UT})
    include (CTest)
    include_directories (${CMAKE_SOURCE_DIR})

    set (UT_FILES tests/CrashdumpSections/UncoreRegs_test.cpp
         tests/CrashdumpSections/SqDump_test.cpp
         tests/CrashdumpSections/BigCore_test.cpp tests/test_utils.cpp
         tests/test_main.cpp)

    enable_testing ()

    add_definitions (-DCOMPILE_UNIT_TESTS)
    add_definitions (-DMOCK)
    add_executable (crashdump_test ${SRC_FILES} ${UT_FILES})

    find_package (GTest REQUIRED)
    target_link_libraries (crashdump_test ${GTEST_LIBRARIES} gmock)
    target_link_libraries (crashdump_test cjson peci -lsystemd pthread)
    target_link_libraries (crashdump_test -lstdc++fs sdbusplus)
    target_link_libraries (crashdump_test ${SAFEC_LIBRARIES})

    add_test (crashdump_test crashdump_test
              "--gtest_output=xml:crashdump_test.xml")

    install (TARGETS crashdump_test
             RUNTIME DESTINATION bin
             LIBRARY DESTINATION lib
             ARCHIVE DESTINATION lib/static)

endif (${CRASHDUMP_BUILD_UT})
