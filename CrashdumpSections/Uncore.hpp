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

extern "C" {
#include <cjson/cJSON.h>
#include <stdint.h>
}

/******************************************************************************
 *
 *   Common Defines
 *
 ******************************************************************************/
#define US_JSON_STRING_LEN 64

enum US_REG_SIZE
{
    US_REG_BYTE = 1,
    US_REG_WORD = 2,
    US_REG_DWORD = 4,
    US_REG_QWORD = 8
};

enum US_MMIO_SIZE
{
    US_MMIO_BYTE = 0,
    US_MMIO_WORD = 1,
    US_MMIO_DWORD = 2,
    US_MMIO_QWORD = 3
};

#define US_REG_NAME_LEN 64
#define US_NUM_MCA_DWORDS 10
#define US_NUM_MCA_QWORDS (US_NUM_MCA_DWORDS / 2)

#define US_MCA_NAME_LEN 8
#define US_UNCORE_CRASH_DW_NAME "uncore_crashdump_dw%ld"

#define US_FAILED "N/A"

/******************************************************************************
 *
 *   CPX1 Defines
 *
 ******************************************************************************/
// PECI sequence
#define US_MCA_PARAM 0x1000
#define US_MMIO_PARAM 0x0012
#define US_UCRASH_START 0x3003
#define US_UCRASH_PARAM 0

#define US_BASE_IIO_BANK 6

#define US_MCA_UNMERGE (1 << 22)

/******************************************************************************
 *
 *   ICX1 Defines
 *
 ******************************************************************************/
#define US_PCI_SEG 0
#define US_MMIO_SEG 0

#define MMIO_ROOT_BUS 0xD
#define MMIO_ROOT_DEV 0x0
#define MMIO_ROOT_FUNC 0x02
#define MMIO_ROOT_REG 0xCC

#define US_UA "UA:0x%x"
#define US_UINT64_FMT "0x%" PRIx64 ""

/******************************************************************************
 *
 *   Structures
 *
 ******************************************************************************/
typedef union
{
    uint64_t u64;
    uint32_t u32[2];
} UUncoreStatusRegValue;

typedef struct
{
    UUncoreStatusRegValue uValue;
    bool bInvalid;
} SUncoreStatusRegRawData;

typedef struct
{
    char regName[US_REG_NAME_LEN];
    union
    {
        struct
        {
            uint32_t lenCode : 2;
            uint32_t bar : 2;
            uint32_t bus : 3;
            uint32_t rsvd : 1;
            uint32_t func : 3;
            uint32_t dev : 5;
            uint32_t reg : 16;
        } fields;
        uint32_t raw;
    } uMmioReg;
} SUncoreStatusRegPciMmio;

typedef union
{
    uint64_t u64Raw[US_NUM_MCA_QWORDS];
    uint32_t u32Raw[US_NUM_MCA_DWORDS];
    struct
    {
        uint64_t ctl;
        uint64_t status;
        uint64_t addr;
        uint64_t misc;
        uint64_t misc2;
    } regs;
} UUncoreStatusMcaRegs;

typedef struct
{
    UUncoreStatusMcaRegs uRegData;
    bool bInvalid;
} SUncoreStatusMcaRawData;

typedef struct
{
    char regName[US_REG_NAME_LEN];
    uint8_t u8IioNum;
} SUncoreStatusRegIio;

static const char uncoreStatusMcaRegNames[][US_MCA_NAME_LEN] = {
    "ctl", "status", "addr", "misc", "ctl2"};

typedef int (*UncoreStatusRead)(crashdump::CPUInfo& cpuInfo, cJSON* pJsonChild);

typedef struct
{
    CPUModel cpuModel;
    int (*logUncoreStatusVx)(crashdump::CPUInfo& cpuInfo, cJSON* pJsonChild);
} SUncoreStatusLogVx;

int logUncoreStatus(crashdump::CPUInfo& cpuInfo, cJSON* pJsonChild);
