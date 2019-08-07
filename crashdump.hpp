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
#include <array>
#include <vector>
extern "C" {
#include "libpeci.h"
}
#include "utils.hpp"

namespace crashdump
{
constexpr char const* dbgStatusItemName = "status";
constexpr const char* dbgFailedStatus = "N/A";

// PECI Client Address List
constexpr const int minClientAddr = 0x30;
constexpr const int maxClientAddr = 0x37;
constexpr const int maxCPUs = maxClientAddr - minClientAddr + 1;

enum class CPUModel
{
    skx,
    icx,
    icx_b0,
};

struct CPUIDMap
{
    int cpuID;
    CPUModel model;
};

static constexpr const std::array cpuIDMap{
    CPUIDMap{0x00050654, CPUModel::skx},
    CPUIDMap{0x000606A0, CPUModel::icx},
    CPUIDMap{0x000606A1, CPUModel::icx_b0},
};

struct CPUInfo
{
    int clientAddr;
    CPUModel model;
    uint64_t coreMask;
    uint64_t crashedCoreMask;
    int chaCount;
};
} // namespace crashdump
