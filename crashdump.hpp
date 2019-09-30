/******************************************************************************
 *
 * INTEL CONFIDENTIAL
 *
 * Copyright 2019 Intel Corporation.
 *
 * This software and the related documents are Intel copyrighted materials,
 * and your use of them is governed by the express license under which they
 * were provided to you ("License"). Unless the License provides otherwise,
 * you may not use, modify, copy, publish, distribute, disclose or transmit
 * this software or the related documents without Intel's prior written
 * permission.
 *
 * This software and the related documents are provided as is, with no express
 * or implied warranties, other than those that are expressly stated in the
 * License.
 *
 ******************************************************************************/

#pragma once
#include "utils.hpp"

#include <peci.h>

#include <array>
#include <vector>

extern "C" {
#include <cjson/cJSON.h>

#include "safe_str_lib.h"
}

namespace crashdump
{
constexpr char const* dbgStatusItemName = "status";
constexpr const char* dbgFailedStatus = "N/A";

struct CPUInfo
{
    int clientAddr;
    CPUModel model;
    uint64_t coreMask;
    uint64_t crashedCoreMask;
    int chaCount;
};
} // namespace crashdump

namespace revision
{
constexpr const int offset = 0;
constexpr const int revision1 = 0x01;
static int revision_uncore = 0;
} // namespace revision

namespace product_type
{
constexpr const int offset = 12;
constexpr const int clxSP = 0x2C;
constexpr const int cpx = 0x34;
constexpr const int skxSP = 0x2A;
constexpr const int icxSP = 0x1A;
constexpr const int bmcAutonomous = 0x23;
} // namespace product_type

namespace record_type
{
constexpr const int offset = 24;
constexpr const int coreCrashLog = 0x04;
constexpr const int uncoreStatusLog = 0x08;
constexpr const int torDump = 0x09;
constexpr const int metadata = 0x0b;
constexpr const int pmInfo = 0x0c;
constexpr const int addressMap = 0x0d;
constexpr const int bmcAutonomous = 0x23;
constexpr const int mcaLog = 0x3e;
} // namespace record_type

inline static void logCrashdumpVersion(cJSON* parent,
                                       crashdump::CPUInfo& cpuInfo,
                                       int recordType)
{
    struct VersionInfo
    {
        CPUModel cpuModel;
        int data;
    };

    static constexpr const std::array product{
        VersionInfo{clx, product_type::clxSP},
        VersionInfo{clx2, product_type::clxSP},
        VersionInfo{cpx, product_type::cpx},
        VersionInfo{skx, product_type::skxSP},
        VersionInfo{icx, product_type::icxSP},
    };

    static constexpr const std::array revision{
        VersionInfo{clx, revision::revision1},
        VersionInfo{clx2, revision::revision1},
        VersionInfo{cpx, revision::revision1},
        VersionInfo{skx, revision::revision1},
        VersionInfo{icx, revision::revision1},
    };

    int productType = 0;
    for (const VersionInfo& cpuProduct : product)
    {
        if (cpuInfo.model == cpuProduct.cpuModel)
        {
            productType = cpuProduct.data;
        }
    }

    int revisionNum = 0;
    for (const VersionInfo& cpuRevision : revision)
    {
        if (cpuInfo.model == cpuRevision.cpuModel)
        {
            revisionNum = cpuRevision.data;
        }
    }

    if (recordType == record_type::uncoreStatusLog)
    {
        revisionNum = revision::revision_uncore;
    }

    // Build the version number:
    //  [31:30] Reserved
    //  [29:24] Crash Record Type
    //  [23:12] Product
    //  [11:8] Reserved
    //  [7:0] Revision
    int version = recordType << record_type::offset |
                  productType << product_type::offset |
                  revisionNum << revision::offset;
    char versionString[64];
    cd_snprintf_s(versionString, sizeof(versionString), "0x%x", version);
    cJSON_AddStringToObject(parent, "_version", versionString);
}