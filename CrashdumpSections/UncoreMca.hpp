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
#define UNCORE_MCA_NAME_LEN 16

#define UNCORE_MCA_JSON_STRING_LEN 32
#define UNCORE_MCA_JSON_SECTION_NAME "uncore"

#define UNCORE_MCA_JSON_CBO_NAME "CBO%d"
#define UNCORE_MCA_CBO_REG_NAME "cbo%ld_%s"

#define UNCORE_MCA_FAILED "N/A"
#define UNCORE_MCA_UA "UA:0x%x"

/******************************************************************************
 *
 *   CPX1 Defines
 *
 ******************************************************************************/
// PECI sequence
#define UCM_BANK_PARAM 0x1000

#define FIRST_UNCORE_MCA 4
#define LAST_UNCORE_MCA 22

#define UCM_NUM_MCA_DWORDS 10
#define UCM_NUM_MCA_QWORDS (UCM_NUM_MCA_DWORDS / 2)

#define UNCORE_MCA_REG_NAME "mc%d_%s"
#define UNCORE_MCA_JSON_MCA_NAME "MC%d"

#define US_MCA_UNMERGE (1 << 22)
#define US_BASE_CBO_BANK 9
#define US_NUM_CBO_BANKS 3

/******************************************************************************
 *
 *   ICX1 Defines
 *
 ******************************************************************************/
enum
{
    UCM_CBO_MCA_PARAM_CTL = 0x8000,
    UCM_CBO_MCA_PARAM_STATUS = 0x8001,
    UCM_CBO_MCA_PARAM_ADDR = 0x8002,
    UCM_CBO_MCA_PARAM_MISC = 0x8003,
    UCM_CBO_MCA_PARAM_MISC2 = 0x8004,
};

/******************************************************************************
 *
 *   CPX1 Structures
 *
 ******************************************************************************/
typedef enum
{
    UNCORE_CTL,
    UNCORE_CTL2,
    UNCORE_STATUS,
    UNCORE_ADDR,
    UNCORE_MISC
} EUncoreRegNames;

/******************************************************************************
 *
 *   CPX1/ICX1 Structures
 *
 ******************************************************************************/
typedef union
{
    uint64_t u64Raw[UCM_NUM_MCA_QWORDS];
    uint32_t u32Raw[UCM_NUM_MCA_DWORDS];
    struct
    {
        uint64_t u64UncoreMcaCtl;
        uint64_t u64UncoreMcaStatus;
        uint64_t u64UncoreMcaAddr;
        uint64_t u64UncoreMcaMisc;
        uint64_t u64UncoreMcaMisc2;
        uint64_t u64UncoreMcaCtl2;
    } sReg;
} UUncoreMcaRegs;

typedef struct
{
    UUncoreMcaRegs uRegData;
    bool bInvalid;
} SUncoreMcaRawData;

static const char uncoreMcaRegNames[][UNCORE_MCA_NAME_LEN] = {
    "ctl", "ctl2", "status", "addr", "misc"};

typedef struct
{
    char bankName[UNCORE_MCA_NAME_LEN];
    char regName[UNCORE_MCA_NAME_LEN];
    uint16_t addr;
    uint8_t instance_id;
} SUncoreMcaReg;

typedef struct
{
    CPUModel cpuModel;
    int (*logUncoreMcaVx)(crashdump::CPUInfo& cpuInfo, cJSON* pJsonChild);
} SUncoreMcaLogVx;

int logUncoreMca(crashdump::CPUInfo& cpuInfo, cJSON* pJsonChild);
