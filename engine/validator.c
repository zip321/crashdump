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

#include "validator.h"

bool IsMaxTimeValid(uint32_t maxTime)
{
    // TODO: Validate params
    return true;
}

bool IsCrashDump_DiscoveryParamsValid(cJSON* params)
{
    // TODO: Validate params
    return true;
}

bool IsCrashDump_GetFrameParamsValid(cJSON* params)
{
    // TODO: Validate params
    return true;
}

bool IsGetCPUIDParamsValid(cJSON* params)
{
    // TODO: Validate params
    return true;
}

bool IsPingParamsValid(cJSON* params)
{
    // TODO: Validate params
    return true;
}

bool IsRdEndPointConfigMmioParamsValid(cJSON* params)
{
    // TODO: Validate params
    return true;
}

bool IsRdChaCountParamsValid(cJSON* params)
{
    // TODO: Validate params
    return true;
}

bool IsRdIAMSRParamsValid(cJSON* params)
{
    // TODO: Validate params
    return true;
}

bool IsRdEndPointConfigPciLocalParamsValid(cJSON* params)
{
    // TODO: Validate params
    return true;
}

bool IsWrEndPointConfigPciLocalParamsValid(cJSON* params)
{
    // TODO: Validate params
    return true;
}

bool IsRdPkgConfigParamsValid(cJSON* params)
{
    // TODO: Validate params
    return true;
}

bool IsRdPkgConfigCoreParamsValid(cJSON* params)
{
    // TODO: Validate params
    return true;
}
bool IsRdPostEnumBusParamsValid(cJSON* params)
{
    // TODO: Validate params
    return true;
}

bool IsTelemetry_DiscoveryParamsValid(cJSON* params)
{
    // TODO: Validate params
    return true;
}

bool IsRdAndConcatenateParamsValid(cJSON* params)
{
    // TODO: Validate params
    return true;
}

bool IsRdGlobalVarsValid(cJSON* params)
{
    // TODO: Validate params
    return true;
}

bool IsSaveStrVarsValid(cJSON* params)
{
    // TODO: Validate params
    return true;
}

bool IsValidHexString(char* str)
{
    if (*str != '0')
    {
        return false;
    }
    ++str;
    if (*str != 'x')
    {
        return false;
    }
    ++str;
    if (*str == 0)
    {
        return false;
    }
    if (str[strspn(str, "0123456789abcdefABCDEF")] == 0)
    {
        return true;
    }
    else
    {
        return false;
    }
}
