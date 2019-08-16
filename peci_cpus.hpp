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

namespace crashdump
{
// PECI Client Address List
constexpr const int minClientAddr = 0x30;
constexpr const int maxClientAddr = 0x37;
constexpr const int maxCPUs = maxClientAddr - minClientAddr + 1;

enum class CPUModel
{
    clx_b0,
    clx_b1,
    cpx_a0,
    skx_h0,
    icx_a0,
    icx_b0,
};

struct CPUIDMap
{
    int cpuID;
    CPUModel model;
};

static constexpr const std::array cpuIDMap{

    // This code supports the following CPU's

    CPUIDMap{0x0005065a, CPUModel::cpx_a0},
    CPUIDMap{0x000606A0, CPUModel::icx_a0},
    CPUIDMap{0x000606A1, CPUModel::icx_b0},

    // These CPU’s are Included for testing purposes when using an Interposer.
    // This code base is not intended for production use on the Purely Platform.
    // This is due to differences in the Decoding tools used for the final
    // output.

    CPUIDMap{0x00050654, CPUModel::skx_h0},
    CPUIDMap{0x00050656, CPUModel::clx_b0},
    CPUIDMap{0x00050657, CPUModel::clx_b1},
};
} // namespace crashdump
