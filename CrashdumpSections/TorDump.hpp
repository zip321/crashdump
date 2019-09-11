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

#include <cstdint>
extern "C" {
#include <cjson/cJSON.h>
}

/******************************************************************************
 *
 *   Common Defines
 *
 ******************************************************************************/
#define TD_JSON_STRING_LEN 64
#define TD_JSON_CHA_NAME "cha%d"
#define TD_JSON_TOR_NAME "index%d"
#define TD_JSON_SUBINDEX_NAME "subindex%d"

/******************************************************************************
 *
 *   CPX1 Defines
 *
 ******************************************************************************/
// PECI sequence
#define TD_START_PARAM 0x3001
#define TD_PARAM_ZERO 0x80000000

// Other
#define TD_TORS_PER_CHA_CPX1 24
#define TD_SUBINDEX_PER_TOR_CPX1 3

/******************************************************************************
 *
 *   ICX1 Defines
 *
 ******************************************************************************/
#define TD_TORS_PER_CHA_ICX1 32
#define TD_SUBINDEX_PER_TOR_ICX1 8

typedef struct
{
    CPUModel cpuModel;
    int (*logTorDumpVx)(crashdump::CPUInfo& cpuInfo, cJSON* pJsonChild);
} STorDumpVx;

int logTorDump(crashdump::CPUInfo& cpuInfo, cJSON* pJsonChild);
