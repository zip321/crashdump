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
#include <stdbool.h>
#include <stdint.h>

#define SI_THREADS_PER_CORE 2

#define SI_JSON_STRING_LEN 48
#define SI_JSON_SOCKET_NAME "cpu%d"

#define SI_IPMI_DEV_ID_NETFN 0x06
#define SI_IPMI_DEV_ID_CMD 0x01
#define SI_IPMI_DEV_ID_LUN 0

#define SI_BIOS_ID_LEN 64
#define SI_BMC_VER_LEN 64

#define SI_CPU_NAME_LEN 8
#define SI_CRASHDUMP_VER_LEN 8
#define SI_CRASHDUMP_VER "BMC_1933"

#define SI_PECI_PPIN_IDX 19
#define SI_PECI_PPIN_LOWER 0x01
#define SI_PECI_PPIN_UPPER 0x02

#define MD_NA "N/A"

/******************************************************************************
 *
 *   Structures
 *
 ******************************************************************************/
typedef struct
{
    crashdump::CPUModel cpuModel;
    char cpuModelName[SI_CPU_NAME_LEN];
} SCpuIdName;

typedef struct
{
    struct
    {
        uint32_t clientID;
        uint32_t cpuID;
        uint32_t packageID;
        uint32_t u32CoresPerCpu;
        uint32_t u32UCodeVer;
        uint32_t u32VCodeVer;
        uint32_t u32McaErrSrcLog;
        uint64_t ppin;
        bool isPpinNA;
    } sCpuData;
    bool systemData;
    char bmcVersion[SI_BMC_VER_LEN];
    char crashdumpVersion[SI_CRASHDUMP_VER_LEN];
    uint8_t u8BiosId[SI_BIOS_ID_LEN];
} SSysInfoRawData;

typedef struct
{
    char sectionName[SI_JSON_STRING_LEN];
    void (*FillSysInfoJson)(SSysInfoRawData* sSysInfoRawData,
                            char* cSectionName, cJSON* pJsonChild);
} SSysInfoSection;

typedef struct
{
    crashdump::CPUModel cpuModel;
    void (*getPpinVx)(crashdump::CPUInfo& cpuInfo,
                      SSysInfoRawData* sSysInfoRawData, cJSON* pJsonChild);
} SPpinVx;

int logSysInfo(crashdump::CPUInfo& cpuInfo, cJSON* pJsonChild);
