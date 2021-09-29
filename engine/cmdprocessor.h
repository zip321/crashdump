/******************************************************************************
 *
 * INTEL CONFIDENTIAL
 *
 * Copyright 2021 Intel Corporation.
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

#ifndef CRASHDUMP_CMDPROCESSOR_H
#define CRASHDUMP_CMDPROCESSOR_H

#include <search.h>

#include "../CrashdumpSections/crashdump.h"

#define CD_NUM_OF_PECI_CMDS 10

typedef struct
{
    CPUModel cpuModel;
    uint8_t stepping;
} PECICPUID;

typedef struct
{
    cJSON* params;
    cJSON* outputPath;
} PECICmdInput;

typedef union
{
    uint64_t u64;
    uint32_t u32[2];
} RegVal;

typedef struct
{
    EPECIStatus ret;
    uint8_t cc;
    RegVal val;
    uint8_t size;
    PECICPUID cpuID;
} PECICmdOutput;

typedef struct
{
    PECICmdInput in;
    PECICmdOutput out;
    cJSON* paramsTracker;
    cJSON* internalVarsTracker;
    char* internalVarName;
} CmdInOut;

typedef struct
{
    uint32_t cpubusno_valid;
    uint32_t cpubusno;
    int bitToCheck;
} PostEnumBus;

typedef struct
{
    uint64_t chaMask0;
    uint64_t chaMask1;
} ChaCount;

acdStatus BuildCmdsTable(ENTRY* entry);
acdStatus Execute(ENTRY* entry, CmdInOut* cmdInOut);

#endif // CRASHDUMP_CMDPROCESSOR_H
