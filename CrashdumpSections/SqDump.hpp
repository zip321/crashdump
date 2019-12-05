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
#define SQ_START_PARAM 0x3002
#define SQ_ADDR_ARRAY 0x00000000
#define SQ_CTRL_ARRAY 0x80000000

// Other
#define SQ_JSON_STRING_LEN 32
#define SQ_JSON_CORE_NAME "core%d"
#define SQ_JSON_ENTRY_NAME "entry%d"

/******************************************************************************
 *
 *   Structures
 *
 ******************************************************************************/
typedef struct
{
    uint32_t* pu32SqAddrArray;
    uint32_t u32SqAddrSize;
    uint32_t* pu32SqCtrlArray;
    uint32_t u32SqCtrlSize;
} SSqDump;

typedef struct
{
    crashdump::cpu::Model cpuModel;
    int (*logSqDumpVx)(crashdump::CPUInfo& cpuInfo, cJSON* pJsonChild);
} SSqDumpVx;

int logSqDump(crashdump::CPUInfo& cpuInfo, cJSON* pJsonChild);
