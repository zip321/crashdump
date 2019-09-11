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

#include "crashdump.hpp"

#include <cjson/cJSON.h>
#include <stdint.h>

// PECI sequence
#define PM_CSTATE_PARAM 0x4660B4
#define PM_VID_PARAM 0x488004
#define PM_READ_PARAM 0x1019
#define PM_CORE_OFFSET 24

#define PM_JSON_STRING_LEN 32
#define PM_JSON_CORE_NAME "core%d"
#define PM_JSON_CSTATE_REG_NAME "c_state_reg"
#define PM_JSON_VID_REG_NAME "vid_ratio_reg"

/******************************************************************************
 *
 *   Structures
 *
 ******************************************************************************/
typedef struct
{
    uint32_t u32CState;
    uint32_t u32VidRatio;
} SCpuPowerState;

typedef struct
{
    CPUModel cpuModel;
    int (*logPowerManagementVx)(crashdump::CPUInfo& cpuInfo, cJSON* pJsonChild);
} SPowerManagementVx;

int logPowerManagement(crashdump::CPUInfo& cpuInfo, cJSON* pJsonChild);
