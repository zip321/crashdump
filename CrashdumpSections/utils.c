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

#include "utils.h"

#include <safe_mem_lib.h>
#include <safe_str_lib.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
struct timespec crashdumpStart;

int cd_snprintf_s(char* str, size_t len, const char* format, ...)
{
    int ret;
    va_list args;
    va_start(args, format);
    ret = vsnprintf_s(str, len, format, args);
    va_end(args);
    return ret;
}

static uint32_t setMask(uint32_t msb, uint32_t lsb)
{
    uint32_t range = (msb - lsb) + 1;
    uint32_t mask = ((1u << range) - 1);
    return mask;
}

void setFields(uint32_t* value, uint32_t msb, uint32_t lsb, uint32_t setVal)
{
    uint32_t mask = setMask(msb, lsb);
    setVal = setVal << lsb;
    *value &= ~(mask << lsb);
    *value |= setVal;
}

uint32_t getFields(uint32_t value, uint32_t msb, uint32_t lsb)
{
    uint32_t mask = setMask(msb, lsb);
    value = value >> lsb;
    return (mask & value);
}

uint32_t bitField(uint32_t offset, uint32_t size, uint32_t val)
{
    uint32_t mask = (1u << size) - 1;
    return (val & mask) << offset;
}

cJSON* readInputFile(const char* filename)
{
    char* buffer = NULL;
    cJSON* jsonBuf = NULL;
    long int length = 0;
    FILE* fp = fopen(filename, "r");
    size_t result = 0;

    if (fp == NULL)
    {
        return NULL;
    }

    fseek(fp, 0, SEEK_END);
    length = ftell(fp);
    if (length == -1L)
    {
        fclose(fp);
        return NULL;
    }
    fseek(fp, 0, SEEK_SET);
    buffer = (char*)calloc(length, sizeof(char));
    if (buffer)
    {
        result = fread(buffer, 1, length, fp);
        if ((int)result != length)
        {
            fprintf(stderr, "fread read %zu bytes, but length is %ld", result,
                    length);
            fclose(fp);
            FREE(buffer);
            return NULL;
        }
    }

    fclose(fp);
    // Convert and return cJSON object from buffer
    jsonBuf = cJSON_Parse(buffer);
    FREE(buffer);
    return jsonBuf;
}

cJSON* getCrashDataSection(cJSON* root, char* section, bool* enable)
{
    *enable = false;
    cJSON* child = cJSON_GetObjectItemCaseSensitive(
        cJSON_GetObjectItemCaseSensitive(root, "crash_data"), section);

    if (child != NULL)
    {
        cJSON* recordEnable = cJSON_GetObjectItem(child, RECORD_ENABLE);
        if (recordEnable == NULL)
        {
            *enable = true;
        }
        else
        {
            *enable = cJSON_IsTrue(recordEnable);
        }
    }

    return child;
}

cJSON* getNewCrashDataSection(cJSON* root, char* section, bool* enable)
{
    *enable = false;
    cJSON* sections = cJSON_GetObjectItemCaseSensitive(
        cJSON_GetObjectItemCaseSensitive(root, "crash_data"), "Sections");

    if (sections != NULL)
    {
        cJSON* child =
            cJSON_GetObjectItemCaseSensitive(sections->child, section);
        if (child != NULL)
        {
            cJSON* recordEnable =
                cJSON_GetObjectItem(child, RECORD_ENABLE_SECTIONS);
            if (recordEnable == NULL)
            {
                *enable = true;
            }
            else
            {
                *enable = cJSON_IsTrue(recordEnable);
            }
        }
        return child;
    }

    return sections;
}

cJSON* getCrashDataSectionRegList(cJSON* root, char* section, char* regType,
                                  bool* enable)
{
    cJSON* child = getCrashDataSection(root, section, enable);

    if (child != NULL)
    {
        return cJSON_GetObjectItemCaseSensitive(
            cJSON_GetObjectItemCaseSensitive(child, regType), "reg_list");
    }

    return child;
}

cJSON* getCrashDataSectionObjectOneLevel(cJSON* root, char* section,
                                         const char* firstLevel, bool* enable)
{
    cJSON* child = getCrashDataSection(root, section, enable);

    if (child != NULL)
    {
        return cJSON_GetObjectItemCaseSensitive(child, firstLevel);
    }

    return child;
}

cJSON* getNewCrashDataSectionObjectOneLevel(cJSON* root, char* section,
                                            const char* firstLevel,
                                            bool* enable)
{
    cJSON* child = getNewCrashDataSection(root, section, enable);

    if (child != NULL)
    {
        return cJSON_GetObjectItemCaseSensitive(child, firstLevel);
    }

    return child;
}

cJSON* getCrashDataSectionObject(cJSON* root, char* section, char* firstLevel,
                                 char* secondLevel, bool* enable)
{
    cJSON* child = getCrashDataSection(root, section, enable);

    if (child != NULL)
    {
        return cJSON_GetObjectItemCaseSensitive(
            cJSON_GetObjectItemCaseSensitive(child, firstLevel), secondLevel);
    }

    return child;
}

int getCrashDataSectionVersion(cJSON* root, char* section)
{
    uint64_t version = 0;
    bool enable = false;
    cJSON* child = getCrashDataSection(root, section, &enable);

    if (child != NULL)
    {
        cJSON* jsonVer = cJSON_GetObjectItem(child, "_version");
        if ((jsonVer != NULL) && cJSON_IsString(jsonVer))
        {
            version = strtoull(jsonVer->valuestring, NULL, 16);
        }
    }

    return version;
}

void updateRecordEnable(cJSON* root, bool enable)
{
    cJSON* logSection = NULL;
    logSection = cJSON_GetObjectItemCaseSensitive(root, RECORD_ENABLE);
    if (logSection == NULL)
    {
        cJSON_AddBoolToObject(root, RECORD_ENABLE, enable);
    }
}

bool isBigCoreRegVersionMatch(cJSON* root, uint32_t version)
{
    bool enable = false;
    cJSON* child = getNewCrashDataSection(root, BIG_CORE_SECTION_NAME, &enable);

    if (child != NULL && enable)
    {
        char jsonItemName[NAME_STR_LEN] = {0};
        cd_snprintf_s(jsonItemName, NAME_STR_LEN, "0x%x", version);

        cJSON* decodeArray = cJSON_GetObjectItemCaseSensitive(child, "Decode");

        if (decodeArray != NULL)
        {
            cJSON* mapItem = NULL;
            cJSON_ArrayForEach(mapItem, decodeArray)
            {
                cJSON* versionArray =
                    cJSON_GetObjectItemCaseSensitive(mapItem, "version");
                if (NULL != versionArray)
                {
                    cJSON* versionItem = NULL;
                    cJSON_ArrayForEach(versionItem, versionArray)
                    {
                        int mismatch = 1;
                        strcmp_s(versionItem->valuestring,
                                 strnlen_s(jsonItemName, NAME_STR_LEN),
                                 jsonItemName, &mismatch);
                        if (0 == mismatch)
                        {
                            return true;
                        }
                    }
                }
            }
        }
    }

    return false;
}

cJSON* getValidBigCoreMap(cJSON* decodeArray, char* version)
{
    if (NULL == decodeArray)
    {
        return NULL;
    }

    cJSON* mapItem = NULL;
    cJSON_ArrayForEach(mapItem, decodeArray)
    {
        cJSON* versionArray =
            cJSON_GetObjectItemCaseSensitive(mapItem, "version");
        if (NULL != versionArray)
        {
            cJSON* versionItem = NULL;
            cJSON_ArrayForEach(versionItem, versionArray)
            {
                int mismatch = 1;
                strcmp_s(versionItem->valuestring,
                         strnlen_s(version, NAME_STR_LEN), version, &mismatch);
                if (0 == mismatch)
                {
                    return mapItem;
                }
            }
        }
    }

    return NULL;
}

cJSON* getCrashDataSectionBigCoreRegList(cJSON* root, char* version)
{
    bool enable = false;
    cJSON* child = getNewCrashDataSection(root, BIG_CORE_SECTION_NAME, &enable);

    if (child != NULL && enable)
    {
        cJSON* decodeObject = cJSON_GetObjectItemCaseSensitive(child, "Decode");

        if (decodeObject != NULL)
        {
            cJSON* mapObject = getValidBigCoreMap(decodeObject, version);
            if (mapObject != NULL)
            {
                return cJSON_GetObjectItemCaseSensitive(mapObject, "reg_list");
            }
            return mapObject;
        }
        return decodeObject;
    }
    return child;
}

uint32_t getCrashDataSectionBigCoreSize(cJSON* root, char* version)
{
    uint32_t size = 0;
    bool enable = false;
    cJSON* child = getNewCrashDataSection(root, BIG_CORE_SECTION_NAME, &enable);

    if (child != NULL && enable)
    {
        cJSON* decodeObject = cJSON_GetObjectItemCaseSensitive(child, "Decode");

        if (decodeObject != NULL)
        {
            cJSON* mapObject = getValidBigCoreMap(decodeObject, version);

            if (mapObject != NULL)
            {
                cJSON* jsonSize =
                    cJSON_GetObjectItemCaseSensitive(mapObject, "_size");

                if ((jsonSize != NULL) && cJSON_IsString(jsonSize))
                {
                    size = strtoul(jsonSize->valuestring, NULL, 16);
                }
            }
        }
    }

    return size;
}

void storeCrashDataSectionBigCoreSize(cJSON* root, char* version,
                                      uint32_t totalSize)
{
    bool enable = false;
    cJSON* child = getNewCrashDataSection(root, BIG_CORE_SECTION_NAME, &enable);

    if (child != NULL && enable)
    {
        char jsonItemName[NAME_STR_LEN] = {0};
        cd_snprintf_s(jsonItemName, NAME_STR_LEN, "0x%x", totalSize);
        cJSON_AddStringToObject(
            getValidBigCoreMap(
                cJSON_GetObjectItemCaseSensitive(child, "Decode"), version),
            "_size", jsonItemName);
    }
}

cJSON* selectAndReadInputFile(Model cpuModel, char** filename, bool isTelemetry)
{
    char cpuStr[CPU_STR_LEN] = {0};
    char nameStr[NAME_STR_LEN] = {0};

    switch (cpuModel)
    {
        case cd_icx:
        case cd_icx2:
        case cd_icxd:
            strcpy_s(cpuStr, sizeof("icx"), "icx");
            break;
        case cd_spr:
            strcpy_s(cpuStr, sizeof("spr"), "spr");
            break;
        case cd_sprhbm:
            strcpy_s(cpuStr, sizeof("sprhbm"), "sprhbm");
            break;
        default:
            CRASHDUMP_PRINT(ERR, stderr,
                            "Error selecting input file (CPUID 0x%x).\n",
                            cpuModel);
            return NULL;
    }

    char* override_file = OVERRIDE_INPUT_FILE;

    if (isTelemetry)
    {
        override_file = OVERRIDE_TELEMETRY_FILE;
    }

    cd_snprintf_s(nameStr, NAME_STR_LEN, override_file, cpuStr);

    if (access(nameStr, F_OK) != -1)
    {
        CRASHDUMP_PRINT(INFO, stderr, "Using override file - %s\n", nameStr);
    }
    else
    {
        char* default_file = DEFAULT_INPUT_FILE;
        if (isTelemetry)
        {
            default_file = DEFAULT_TELEMETRY_FILE;
        }
        cd_snprintf_s(nameStr, NAME_STR_LEN, default_file, cpuStr);
    }
    *filename = (char*)malloc(sizeof(nameStr));
    if (*filename == NULL)
    {
        return NULL;
    }
    strcpy_s(*filename, sizeof(nameStr), nameStr);

    return readInputFile(nameStr);
}

cJSON* getCrashDataSectionAddressMapRegList(cJSON* root)
{
    bool enable = false;
    cJSON* child = getCrashDataSection(root, "address_map", &enable);

    if (child != NULL && enable)
    {
        return cJSON_GetObjectItemCaseSensitive(child, "reg_list");
    }

    return child;
}

cJSON* getPeciAccessType(cJSON* root)
{
    if (root != NULL)
    {
        cJSON* child = cJSON_GetObjectItem(root, "crash_data");
        if (child != NULL)
        {
            cJSON* accessType = cJSON_GetObjectItem(child, "AccessMethod");
            if (accessType != NULL)
            {
                return accessType;
            }
        }
    }
    return NULL;
}

uint64_t tsToNanosecond(struct timespec* ts)
{
    return ((ts->tv_sec * (uint64_t)1e9) + ts->tv_nsec);
}

inline struct timespec
    calculateTimeRemaining(uint32_t maxWaitTimeFromInputFileInSec)
{
    struct timespec current = {0, 0};
    clock_gettime(CLOCK_MONOTONIC, &current);

    uint64_t runTimeInNs =
        tsToNanosecond(&current) - tsToNanosecond(&crashdumpStart);

    uint64_t maxWaitTimeFromInputFileInNs =
        maxWaitTimeFromInputFileInSec * (uint64_t)1e9;

    struct timespec timeRemaining = {0, 0};
    if (runTimeInNs < maxWaitTimeFromInputFileInNs)
    {
        timeRemaining.tv_sec =
            (maxWaitTimeFromInputFileInNs - runTimeInNs) / 1e9;
        timeRemaining.tv_nsec =
            (maxWaitTimeFromInputFileInNs - runTimeInNs) % (uint64_t)1e9;

        return timeRemaining;
    }

    return timeRemaining;
}

inline uint32_t getDelayFromInputFile(CPUInfo* cpuInfo, char* sectionName)
{
    bool enable = false;
    cJSON* inputDelayField = getNewCrashDataSectionObjectOneLevel(
        cpuInfo->inputFile.bufferPtr, sectionName, "MaxWaitSec", &enable);

    if (inputDelayField != NULL && enable)
    {
        return (uint32_t)inputDelayField->valueint;
    }

    return 0;
}

uint32_t getCollectionTimeFromInputFile(CPUInfo* cpuInfo)
{
    bool enable = false;
    cJSON* inputField = getNewCrashDataSectionObjectOneLevel(
        cpuInfo->inputFile.bufferPtr, BIG_CORE_SECTION_NAME, "MaxCollectionSec",
        &enable);

    if (inputField != NULL && enable)
    {
        return (uint32_t)inputField->valueint;
    }

    return 0;
}

inline bool getSkipFromInputFile(CPUInfo* cpuInfo, char* sectionName)
{
    bool enable = false;
    cJSON* skipIfFailRead = getCrashDataSectionObjectOneLevel(
        cpuInfo->inputFile.bufferPtr, sectionName, "_skip_on_fail", &enable);

    if (skipIfFailRead != NULL && enable)
    {
        return cJSON_IsTrue(skipIfFailRead);
    }

    return false;
}

inline bool getSkipFromNewInputFile(CPUInfo* cpuInfo, char* sectionName)
{
    bool enable = false;
    cJSON* torSection = getNewCrashDataSection(cpuInfo->inputFile.bufferPtr,
                                               sectionName, &enable);
    cJSON* skipIfFailRead =
        cJSON_GetObjectItemCaseSensitive(torSection, "_skip_on_fail");

    if (skipIfFailRead != NULL && enable)
    {
        return cJSON_IsTrue(skipIfFailRead);
    }

    return false;
}

void updateMcaRunTime(cJSON* root, struct timespec* start)
{
    char* key = "_time";
    cJSON* mcaUncoreTime = cJSON_GetObjectItemCaseSensitive(root, key);

    if (mcaUncoreTime != NULL)
    {
        char timeString[64];
        struct timespec finish = {};

        clock_gettime(CLOCK_MONOTONIC, &finish);
        uint64_t mcaCoreRunTimeInNs =
            tsToNanosecond(&finish) - tsToNanosecond(start);

        // Remove unit "s" from logged "_time"
        char* str = mcaUncoreTime->valuestring;
        str[strnlen_s(str, sizeof(str)) - 1] = '\0';

        // Calculate, replace "_time" from uncore MCA,
        // and log total MCA run time
        double macUncoreTimeInSec = atof(mcaUncoreTime->valuestring);
        double totalMcaRunTimeInSec =
            (double)mcaCoreRunTimeInNs / 1e9 + macUncoreTimeInSec;
        cd_snprintf_s(timeString, sizeof(timeString), "%.2fs",
                      totalMcaRunTimeInSec);
        cJSON_DeleteItemFromObjectCaseSensitive(root, key);
        cJSON_AddStringToObject(root, key, timeString);
    }

    clock_gettime(CLOCK_MONOTONIC, start);
}

uint8_t getBus(Model model, uint8_t u8index)
{
    if (model == cd_icx || model == cd_icx2 || model == cd_icxd)
    {
        if (sPciReg[u8index].u8Bus == 30)
        {
            return 13;
        }
        if (sPciReg[u8index].u8Bus == 31)
        {
            return 14;
        }
    }
    return sPciReg[u8index].u8Bus;
}

int getPciRegister(CPUInfo* cpuInfo, SRegRawData* sRegData, uint8_t u8index)
{
    int peci_fd = -1;
    int ret = 0;
    uint16_t u16Offset = 0;
    uint8_t u8Size = 0;
    uint8_t u8Bus = getBus(cpuInfo->model, u8index);
    ret = peci_Lock(&peci_fd, PECI_WAIT_FOREVER);
    if (ret != PECI_CC_SUCCESS)
    {
        sRegData->ret = ret;
        return ACD_FAILURE;
    }
    switch (sPciReg[u8index].u8Size)
    {
        case UT_REG_DWORD:
            ret = peci_RdEndPointConfigPciLocal_seq(
                cpuInfo->clientAddr, sPciReg[u8index].u8Seg, u8Bus,
                sPciReg[u8index].u8Dev, sPciReg[u8index].u8Func,
                sPciReg[u8index].u16Reg, sPciReg[u8index].u8Size,
                (uint8_t*)&sRegData->uValue.u64, peci_fd, &sRegData->cc);
            sRegData->ret = ret;
            if (ret != PECI_CC_SUCCESS)
            {
                peci_Unlock(peci_fd);
                return ACD_FAILURE;
            }
            sRegData->uValue.u64 &= 0xFFFFFFFF;
            break;
        case UT_REG_QWORD:
            for (uint8_t u8Dword = 0; u8Dword < 2; u8Dword++)
            {
                u16Offset = ((sPciReg[u8index].u16Reg) >> (u8Dword * 8)) & 0xFF;
                u8Size = sPciReg[u8index].u8Size / 2;
                ret = peci_RdEndPointConfigPciLocal_seq(
                    cpuInfo->clientAddr, sPciReg[u8index].u8Seg, u8Bus,
                    sPciReg[u8index].u8Dev, sPciReg[u8index].u8Func, u16Offset,
                    u8Size, (uint8_t*)&sRegData->uValue.u32[u8Dword], peci_fd,
                    &sRegData->cc);
                sRegData->ret = ret;
                if (ret != PECI_CC_SUCCESS)
                {
                    peci_Unlock(peci_fd);
                    return ACD_FAILURE;
                }
            }
            break;
        default:
            ret = ACD_FAILURE;
    }
    peci_Unlock(peci_fd);
    return ACD_SUCCESS;
}

inputField getFlagValueFromInputFile(CPUInfo* cpuInfo, char* sectionName,
                                     char* flagName)
{
    bool enable;
    cJSON* flagField = getNewCrashDataSectionObjectOneLevel(
        cpuInfo->inputFile.bufferPtr, sectionName, flagName, &enable);

    if (flagField != NULL && enable)
    {
        return (cJSON_IsTrue(flagField) ? FLAG_ENABLE : FLAG_DISABLE);
    }

    return FLAG_NOT_PRESENT;
}

static struct timespec
    calculateTimeRemainingFromStart(uint32_t maxTimeInSec,
                                    struct timespec sectionStartTime)
{
    struct timespec current = {0, 0};
    clock_gettime(CLOCK_MONOTONIC, &current);

    uint64_t runTimeInNs =
        tsToNanosecond(&current) - tsToNanosecond(&sectionStartTime);

    uint64_t maxTimeInNs = maxTimeInSec * (uint64_t)1e9;

    struct timespec timeRemaining = {0, 0};
    if (runTimeInNs < maxTimeInNs)
    {
        timeRemaining.tv_sec = (maxTimeInNs - runTimeInNs) / 1e9;
        timeRemaining.tv_nsec = (maxTimeInNs - runTimeInNs) % (uint64_t)1e9;
    }

    return timeRemaining;
}

executionStatus checkMaxTimeElapsed(uint32_t maxTime,
                                    struct timespec sectionStartTime)
{
    struct timespec timeRemaining =
        calculateTimeRemainingFromStart(maxTime, sectionStartTime);

    if (0 == tsToNanosecond(&timeRemaining))
    {
        return EXECUTION_ABORTED;
    }

    return EXECUTION_TILL_ABORT;
}

cJSON* getNVDSection(cJSON* root, const char* const section, bool* const enable)
{
    *enable = false;
    cJSON* child = cJSON_GetObjectItemCaseSensitive(
        cJSON_GetObjectItemCaseSensitive(root, "NVD"), section);

    if (child != NULL)
    {
        cJSON* recordEnable = cJSON_GetObjectItem(child, RECORD_ENABLE);
        if (recordEnable == NULL)
        {
            *enable = true;
        }
        else
        {
            *enable = cJSON_IsTrue(recordEnable);
        }
    }

    return child;
}

cJSON* getNVDSectionRegList(cJSON* root, const char* const section,
                            bool* const enable)
{
    cJSON* child = getNVDSection(root, section, enable);

    if (child != NULL)
    {
        return cJSON_GetObjectItemCaseSensitive(child, "reg_list");
    }

    return child;
}

cJSON* getPMEMSectionErrLogList(cJSON* root, const char* const section,
                                bool* const enable)
{
    cJSON* child = getNVDSection(root, section, enable);

    if (child != NULL)
    {
        return cJSON_GetObjectItemCaseSensitive(child, "log_list");
    }

    return child;
}

cJSON* getCrashlogExcludeList(cJSON* root)
{
    bool enable = false;
    cJSON* crashlogSection = getNewCrashDataSection(root, "crashlog", &enable);

    if (crashlogSection != NULL && enable)
    {
        cJSON* excludeAgentsList = cJSON_GetObjectItemCaseSensitive(
            crashlogSection, "ExcludeAgentIDs");
        return excludeAgentsList;
    }

    return NULL;
}

bool isSprHbm(const CPUInfo* cpuInfo)
{
    EPECIStatus status;
    uint8_t cc = 0;
    uint32_t val = 0;

    // Notes: See doc 611488 SPR EDS Volume 1, Table 91 for PLATFORM ID details
    status = peci_RdPkgConfig(cpuInfo->clientAddr, 0, 1, sizeof(uint32_t),
                              (uint8_t*)&val, &cc);
    if (status != PECI_CC_SUCCESS || (PECI_CC_UA(cc)))
    {
        return false;
    }
    else
    {
        return (val == SPR_HBM_PLATFORM_ID) ? true : false;
    }
}

cJSON* getCrashlogAgentsFromInputFile(cJSON* root)
{
    bool enable = false;
    cJSON* crashlogSection = getNewCrashDataSection(root, "crashlog", &enable);

    if (crashlogSection != NULL && enable)
    {
        return cJSON_GetObjectItemCaseSensitive(crashlogSection, "Agent");
    }

    return NULL;
}

uint8_t getMaxCollectionCoresFromInputFile(CPUInfo* cpuInfo)
{
    bool enable = false;
    cJSON* inputField = getNewCrashDataSectionObjectOneLevel(
        cpuInfo->inputFile.bufferPtr, BIG_CORE_SECTION_NAME,
        "MaxCollectionCores", &enable);

    if (inputField != NULL && enable)
    {
        return (uint8_t)inputField->valueint;
    }

    return 0;
}