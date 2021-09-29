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

#ifndef LOGGER_H
#define LOGGER_H
#include "../CrashdumpSections/crashdump.h"
#include "CrashdumpSections/utils.h"
#include "cmdprocessor.h"
#define LOGGER_JSON_PATH_STRING_LEN 128
#define MAX_NUM_PATH_LEVELS 11

typedef struct
{
    char* pathStringToken;
    char pathString[LOGGER_JSON_PATH_STRING_LEN];
    char* pathLevelToken[MAX_NUM_PATH_LEVELS];
    int numberOfTokens;
} PathParsing;

typedef struct
{
    char* registerName;
    char* sectionName;
    bool extraLevel;
    bool logRegister;
    uint8_t size;
    bool sizeFromOutput;
    cJSON* lastLevel;
    int rootAtLevel;
    cJSON* rootCommonJson;
} NameProcessing;

typedef struct
{
    uint8_t cpu;
    uint8_t core;
    uint8_t thread;
    uint8_t cha;
    bool skipFlag;
    bool skipOnFailFromInputFile;
    int version;
} ContextLogger;

typedef struct
{
    PathParsing pathParsing;
    NameProcessing nameProcessing;
    ContextLogger contextLogger;
} LoggerStruct;

void Logger(CmdInOut* cmdInOut, cJSON* root, LoggerStruct* loggerStruct);
acdStatus GetPath(cJSON* OutputJSON, LoggerStruct* loggerStruct);
acdStatus ParsePath(LoggerStruct* loggerStruct);
acdStatus ParseNameSection(CmdInOut* cmdInOut, LoggerStruct* loggerStruct);
void logVersion(CmdInOut* cmdInOut, cJSON* root, LoggerStruct* loggerStruct);
void GenerateVersion(cJSON* Section, int* Version);
void logSectionRunTime(cJSON* parent, struct timespec* start, char* key);
void logRecordDisabled(CmdInOut* cmdInOut, cJSON* root,
                       LoggerStruct* loggerStruct);

#define LOGGER_JSON_STRING_LEN 64
#define LOGGER_DATA_64bits "0x%" PRIx64 ""
#define LOGGER_DATA_64_bits_CC_RC "0x%" PRIx64 ",CC:0x%x,RC:0x%x"
#define LOGGER_FIXED_DATA_CC_RC "0x0,CC:0x%x,RC:0x%x"
#define LOGGER_VERSION_STRING "_version"
#define LOGGER_NA "N/A"
#define RECORD_ENABLE "_record_enable"
#define namePosition 0
#define sectionPosition 1

#define LOGGER_CPU "cpu%d"
#define LOGGER_CORE "core%d"
#define LOGGER_THREAD "thread%d"
#define LOGGER_CBO "cbo%d"
#define LOGGER_CBO_UC "CBO%d"
#define LOGGER_CHA "cha%d"

#endif // LOGGER_H
