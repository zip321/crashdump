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

#include "utils_triage.hpp"

#include "bafi/triage.hpp"

extern "C" {
#include <cjson/cJSON.h>

#include "CrashdumpSections/crashdump.h"
#include "CrashdumpSections/utils.h"
}

void appendTriageSection(std::string& storedLogContents)
{
    char* triageInfo = NULL;
    size_t triageInfoSize = 0;

    CRASHDUMP_PRINT(INFO, stderr, "Logging triage section\n");
    uint32_t status = getTriageInformation((char*)storedLogContents.c_str(),
                                           storedLogContents.length() + 1,
                                           triageInfo, &triageInfoSize, NULL);
    cJSON* content = cJSON_Parse(storedLogContents.c_str());
    if (status == 0)
    {
        cJSON* triage =
            cJSON_GetObjectItem(cJSON_Parse(triageInfo), TRIAGE_KEY);
        if (triage == NULL)
        {
            cJSON_AddStringToObject(content, TRIAGE_KEY, TRIAGE_ERR);
            CRASHDUMP_PRINT(ERR, stderr, "Triage info not found!\n");
        }
        else
        {
            cJSON_AddItemToObject(content, TRIAGE_KEY, triage);
        }
    }
    else
    {
        char jsonStr[JSON_STR_LEN];
        cd_snprintf_s(jsonStr, JSON_STR_LEN, BAFI_RC, status);
        cJSON_AddStringToObject(content, TRIAGE_KEY, jsonStr);
        CRASHDUMP_PRINT(ERR, stderr, "Get triage info failed!\n");
    }
    storedLogContents.assign(cJSON_Print(content));
    cJSON_Delete(content);
    if (triageInfo != NULL)
    {
        free(triageInfo);
    }
}

void appendSummarySection(std::string& storedLogContents)
{
    char* summaryInfo = NULL;
    size_t summaryInfoSize = 0;

    CRASHDUMP_PRINT(INFO, stderr, "Logging summary section\n");
    uint32_t status = getFullOutput((char*)storedLogContents.c_str(),
                                    storedLogContents.length() + 1, summaryInfo,
                                    &summaryInfoSize, NULL);

    cJSON* content = cJSON_Parse(storedLogContents.c_str());
    if (status == 0)
    {
        cJSON* summary = cJSON_Parse(summaryInfo);
        if (summary == NULL)
        {
            cJSON_AddStringToObject(content, SUMMARY_KEY, SUMMARY_ERR);
            CRASHDUMP_PRINT(ERR, stderr, "Summary info not found!\n");
        }
        else
        {
            cJSON_AddItemToObject(content, SUMMARY_KEY, summary);
        }
    }
    else
    {
        char jsonStr[JSON_STR_LEN];
        cd_snprintf_s(jsonStr, JSON_STR_LEN, BAFI_RC, status);
        cJSON_AddStringToObject(content, SUMMARY_KEY, jsonStr);
        CRASHDUMP_PRINT(ERR, stderr, "Get summary info failed!\n");
    }
    storedLogContents.assign(cJSON_Print(content));
    cJSON_Delete(content);
    if (summaryInfo != NULL)
    {
        free(summaryInfo);
    }
}
