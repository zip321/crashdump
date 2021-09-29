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

#include "PchCrashlog.h"

#include "PchCrashlogInternal.h"
#include "base64Encode.h"
#include "utils.h"

guidCrashlogSectionMapping agentMap[AgentTotalSize] = {
    {0x258e3c90, "PMC_CRASH_INDEX"},
    {0x8899069f, "PMC_RESET_INDEX"},
    {0x284b9c1f, "PMC_TRACE_INDEX"},
    {0, "NONE"},
    {0, "NONE"},
    {0, "NONE"},
    {0, "NONE"},
    {0, "NONE"}};

acdStatus getAgentName(const crashlogAgentDetails* const agentDetails,
                       char* sectionNameString, const uint16_t length)
{
    for (uint8_t index = 0; index < AgentTotalSize; index++)
    {
        if (agentDetails->uniqueId == agentMap[index].guid)
        {
            cd_snprintf_s(sectionNameString, length, "%s",
                          agentMap[index].crashlogSectionName);
            return ACD_SUCCESS;
        }
    }

    cd_snprintf_s(sectionNameString, length, "%x", agentDetails->uniqueId);
    return ACD_SUCCESS;
}

acdStatus initiateCrashlogTriggerRearm(const CPUInfo* const cpuInfo)
{
    uint64_t watcherData = 0;
    uint8_t cc;

    EPECIStatus ret = peci_Telemetry_ConfigWatcherRd(
        cpuInfo->clientAddr, REARM_CONFIG_WATCHER_ID, CONFIG_WATCHER_OFFSET,
        sizeof(watcherData), &watcherData, &cc);

    if (ret != PECI_CC_SUCCESS || (PECI_CC_UA(cc)))
    {
        CRASHDUMP_PRINT(
            ERR, stderr,
            "Error while reading the Config Watcher for Trigger Rearm \n");
        return ACD_FAILURE;
    }

    watcherData = (watcherData | REARM_TRIGGER_MASK);

    ret = peci_Telemetry_ConfigWatcherWr(
        cpuInfo->clientAddr, REARM_CONFIG_WATCHER_ID, CONFIG_WATCHER_OFFSET,
        sizeof(watcherData), &watcherData, &cc);

    if (ret != PECI_CC_SUCCESS || (PECI_CC_UA(cc)))
    {
        CRASHDUMP_PRINT(
            ERR, stderr,
            "Error while writing the Config Watcher for Trigger Rearm \n");
        return ACD_FAILURE;
    }

    return ACD_SUCCESS;
}

acdStatus storeCrashlog(cJSON* const pJsonChild, const uint16_t agent,
                        const crashlogAgentDetails* const agentDetails,
                        const uint64_t* const rawCrashlog)
{
    char guidString[CRASHLOG_ERROR_JSON_STRING_LEN] = {0};
    cd_snprintf_s(guidString, CRASHLOG_ERROR_JSON_STRING_LEN, "agent_id_0x%x",
                  agentDetails->uniqueId);

    cJSON* guidItem = NULL;
    if ((guidItem = cJSON_GetObjectItemCaseSensitive(pJsonChild, guidString)) ==
        NULL)
    {
        cJSON_AddItemToObject(pJsonChild, guidString,
                              guidItem = cJSON_CreateObject());
    }

    char jsonItemString[CRASHLOG_JSON_STRING_LEN] = {0};
    // agentDetails->crashSpace is the size in DWORDs
    if ((agentDetails->crashSpace * sizeof(uint32_t)) > sizeof(jsonItemString))
    {
        logError(guidItem, "String buffer too small");
        return ACD_FAILURE;
    }

    uint64_t error = base64Encode((uint8_t*)rawCrashlog,
                                  (agentDetails->crashSpace * sizeof(uint32_t)),
                                  jsonItemString);

    if (error)
    {
        char jsonErrorString[CRASHLOG_ERROR_JSON_STRING_LEN] = {0};
        cd_snprintf_s(jsonErrorString, CRASHLOG_ERROR_JSON_STRING_LEN,
                      "Could not encode full crashlog (error=0x%x) for "
                      "agent=0x%x, uniqueId=0x%x",
                      error, agent, agentDetails->uniqueId);
        logError(guidItem, jsonErrorString);
    }

    char sectionNameString[CRASHLOG_ERROR_JSON_STRING_LEN] = {0};
    acdStatus status = getAgentName(agentDetails, &sectionNameString,
                                    CRASHLOG_ERROR_JSON_STRING_LEN);

    char recordNameString[CRASHLOG_ERROR_JSON_STRING_LEN] = {0};
    cd_snprintf_s(recordNameString, CRASHLOG_ERROR_JSON_STRING_LEN, "#data_%s",
                  sectionNameString);

    cJSON_AddStringToObject(guidItem, recordNameString, jsonItemString);
    return ACD_SUCCESS;
}

void logError(cJSON* const pJsonChild, const char* errorString)
{
    char jsonErrorString[CRASHLOG_ERROR_JSON_STRING_LEN] = {0};
    cd_snprintf_s(jsonErrorString, CRASHLOG_ERROR_JSON_STRING_LEN, "%s",
                  errorString);
    cJSON_AddStringToObject(pJsonChild, "_error", jsonErrorString);
}

acdStatus collectCrashlogForPMCAgent(
    const CPUInfo* const cpuInfo, const uint16_t agent,
    const crashlogAgentDetails* const agentDetails, uint64_t* const rawCrashlog)
{
    uint8_t cc;
    uint64_t data = 0;
    acdStatus error = ACD_SUCCESS;

    if (NULL == agentDetails || NULL == cpuInfo || NULL == rawCrashlog)
    {
        return ACD_INVALID_OBJECT;
    }

    // agentDetails->crashSpace is the size in DWORDs and each GetCrashlogSample
    // returns 2 DWORDs, so we only need half that number of samples
    for (uint16_t sampleId = 0; sampleId < (agentDetails->crashSpace / 2);
         sampleId++)
    {
        EPECIStatus ret = peci_Telemetry_GetCrashlogSample(
            cpuInfo->clientAddr, agent, sampleId, sizeof(uint64_t),
            (uint8_t*)&data, &cc);
        if (ret != PECI_CC_SUCCESS || (PECI_CC_UA(cc)))
        {
            CRASHDUMP_PRINT(
                ERR, stderr,
                "Error while collecting crashlog for sampleID=0x%x\n",
                sampleId);

            return ACD_FAILURE;
        }
        rawCrashlog[sampleId] = data;
    }

    return error;
}

acdStatus isValidPCHAgent(const crashlogAgentDetails* const agentDetails)
{
    for (uint8_t index = 0; index < AgentTotalSize; index++)
    {
        if (agentDetails->uniqueId == agentMap[index].guid)
        {
            return ACD_SUCCESS;
        }
    }

    return ACD_FAILURE;
}

crashlogAgentDetails getCrashlogDetailsForPMCAgent(const CPUInfo* const cpuInfo,
                                                   const uint16_t agent)
{
    uint64_t crashlogDetails = 0;
    uint8_t cc;

    EPECIStatus ret = peci_Telemetry_Discovery(
        cpuInfo->clientAddr, 2, 0x4, agent, 0, sizeof(crashlogDetails),
        (uint8_t*)&crashlogDetails, &cc);
    if (ret != PECI_CC_SUCCESS || (PECI_CC_UA(cc)))
    {
        CRASHDUMP_PRINT(ERR, stderr, "Error getting crashlog size\n");
        crashlogAgentDetails agentDetails = {
            .entryType = 0, .crashType = 0, .uniqueId = 0, .crashSpace = 0};
        return agentDetails;
    }

    crashlogAgentDetails agentDetails = {
        .entryType = crashlogDetails & 0xFF,
        .crashType = (crashlogDetails >> 8) & 0xFF,
        .uniqueId = (crashlogDetails >> 16) & 0xFFFFFFFF,
        .crashSpace = (crashlogDetails >> 48) & 0xFFFF};

    return agentDetails;
}

uint16_t getNumberOfCrashlogAgents(const CPUInfo* const cpuInfo)
{
    uint16_t crashlogAgents = NO_CRASHLOG_AGENTS;
    uint8_t cc;

    EPECIStatus ret = peci_Telemetry_Discovery(cpuInfo->clientAddr, 1, 0x4, 0,
                                               0, sizeof(crashlogAgents),
                                               (uint8_t*)&crashlogAgents, &cc);

    if (ret != PECI_CC_SUCCESS || (PECI_CC_UA(cc)))
    {
        CRASHDUMP_PRINT(ERR, stderr,
                        "Error getting number of crashlog agents\n");

        return NO_CRASHLOG_AGENTS;
    }

    return crashlogAgents;
}

bool isTelemetrySupported(const CPUInfo* const cpuInfo)
{
    uint8_t telemetrySupported = TELEMETRY_UNSUPPORTED;
    uint8_t cc;
    EPECIStatus ret = peci_Telemetry_Discovery(cpuInfo->clientAddr, 0, 0, 0, 0,
                                               sizeof(telemetrySupported),
                                               &telemetrySupported, &cc);

    if (ret != PECI_CC_SUCCESS || (PECI_CC_UA(cc)) ||
        TELEMETRY_UNSUPPORTED == telemetrySupported)
    {
        CRASHDUMP_PRINT(ERR, stderr,
                        "Telemetry is not supported, Returned (%d) during "
                        "discovery (disabled:%d)\n",
                        ret, telemetrySupported);

        return TELEMETRY_UNSUPPORTED;
    }

    return telemetrySupported;
}

/******************************************************************************
 *  logCrashlogEBG
 *
 ******************************************************************************/
acdStatus logCrashlogEBG(const CPUInfo* const cpuInfo, cJSON* const pJsonChild)
{
    acdStatus error = ACD_SUCCESS;

    if (isTelemetrySupported(cpuInfo))
    {
        bool isCrashlogExtracted = false;
        uint16_t agentsNum = getNumberOfCrashlogAgents(cpuInfo);

        for (uint16_t agent = AgentPCodeCrash; agent < agentsNum; agent++)
        {
            crashlogAgentDetails agentDetails =
                getCrashlogDetailsForPMCAgent(cpuInfo, agent);
            uint16_t crashlogSize = agentDetails.crashSpace;

            if (0 == agentDetails.crashSpace)
            {
                continue;
            }

            if (ACD_SUCCESS != isValidPCHAgent(&agentDetails))
            {
                continue;
            }

            uint64_t* rawCrashlog =
                (uint64_t*)(calloc(agentDetails.crashSpace, sizeof(uint32_t)));

            if (rawCrashlog == NULL)
            {
                CRASHDUMP_PRINT(
                    ERR, stderr,
                    "Couldn't allocate memory for crashlog of size=0x%x\n",
                    agentDetails.crashSpace);

                continue;
            }

            error = collectCrashlogForPMCAgent(cpuInfo, agent, &agentDetails,
                                               rawCrashlog);
            if (error)
            {
                logError(pJsonChild, "Error collecting crashlog");
            }
            else
            {
                isCrashlogExtracted = true;
                error = storeCrashlog(pJsonChild, agent, &agentDetails,
                                      rawCrashlog);
                if (error)
                {
                    CRASHDUMP_PRINT(ERR, stderr,
                                    "Error storing crashlog for agent 0x%x in "
                                    "output file\n",
                                    agent);
                }
            }

            FREE(rawCrashlog);
        }

        if (isCrashlogExtracted)
        {
            // Processor patching not ready at time of release. Untested.
            // error = initiateCrashlogTriggerRearm(cpuInfo);
        }
    }
    else
    {
        CRASHDUMP_PRINT(ERR, stderr,
                        "Telemetry is not supported. Crashlog collection is "
                        "not possible\n");
        return ACD_FAILURE;
    }

    return error;
}

static const PchCrashlogVx pchCrashlogVx[] = {
    {cd_spr, logCrashlogEBG},
};

/******************************************************************************
 *
 *  logPCHCrashlog
 *
 *
 ******************************************************************************/
int logPCHCrashlog(CPUInfo* cpuInfo, cJSON* pJsonChild)
{
    if (pJsonChild == NULL)
    {
        return ACD_INVALID_OBJECT;
    }

    for (uint32_t i = 0; i < (sizeof(PchCrashlogVx) / sizeof(PchCrashlogVx));
         i++)
    {
        if (cpuInfo->model == pchCrashlogVx[i].pchModel)
        {
            return pchCrashlogVx[i].logCrashlogVx(cpuInfo, pJsonChild);
        }
    }

    CRASHDUMP_PRINT(ERR, stderr, "Cannot find version for %s\n", __FUNCTION__);
    return ACD_FAILURE;
}
