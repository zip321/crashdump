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
#include <string.h>

#define AM_REG_NAME_LEN 32

#define AM_JSON_STRING_LEN 32

#define AM_FAILED "N/A"

/******************************************************************************
 *
 *   Structures
 *
 ******************************************************************************/
typedef union
{
    uint64_t u64;
    uint32_t u32[2];
} UAddrMapRegValue;

typedef struct
{
    char regName[AM_REG_NAME_LEN];
    uint8_t u8Bus;
    uint8_t u8Dev;
    uint8_t u8Func;
    uint16_t u16Reg;
    uint8_t u8Size;
} SAddrMapEntry;

typedef struct
{
    crashdump::CPUModel cpuModel;
    int (*logAddrMapVx)(crashdump::CPUInfo &cpuInfo, cJSON *pJsonChild);
} SAddrMapVx;

int logAddressMap(crashdump::CPUInfo &cpuInfo, cJSON *pJsonChild);
