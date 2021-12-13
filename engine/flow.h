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

#ifndef CRASHDUMP_FLOW_H
#define CRASHDUMP_FLOW_H
#define MAX_CORE_MASK 64
#include "../CrashdumpSections/crashdump.h"
#include "engine/cmdprocessor.h"
#include "engine/inputparser.h"
#include "engine/logger.h"
#include "engine/validator.h"

void ProcessPECICmds(ENTRY* entry, CPUInfo* cpuInfo, cJSON* peciCmds,
                     CmdInOut* cmdInOut, InputParserErrInfo* errInfo,
                     LoggerStruct* loggerStruct, cJSON* outRoot,
                     struct timespec* start, uint32_t* maxTime);
acdStatus fillNewSection(cJSON* root, CPUInfo* cpuInfo, uint8_t cpu,
                         char* sectionName, struct timespec* sectionStart,
                         char* timeStr);

#endif // CRASHDUMP_FLOW_H
