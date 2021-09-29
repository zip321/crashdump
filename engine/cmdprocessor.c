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

#include "cmdprocessor.h"

#include <search.h>

#include "../CrashdumpSections/utils.h"
#include "inputparser.h"
#include "validator.h"

static void UpdateInternalVar(CmdInOut* cmdInOut)
{
    if (cmdInOut->internalVarName != NULL)
    {
        char jsonItemString[JSON_VAL_LEN];
        if (cmdInOut->out.size == sizeof(uint64_t))
        {
            cd_snprintf_s(jsonItemString, JSON_VAL_LEN, "0x%" PRIx64 "",
                          cmdInOut->out.val.u64);
        }
        else
        {
            cd_snprintf_s(jsonItemString, JSON_VAL_LEN, "0x%" PRIx32 "",
                          cmdInOut->out.val.u32[0]);
        }
        cJSON_AddItemToObject(cmdInOut->internalVarsTracker,
                              cmdInOut->internalVarName,
                              cJSON_CreateString(jsonItemString));
    }
}

static acdStatus CrashDump_Discovery(CmdInOut* cmdInOut)
{
    struct peci_crashdump_disc_msg params = {0};
    int position = 0;
    cJSON* it = NULL;

    if (!IsCrashDump_DiscoveryParamsValid(cmdInOut->in.params))
    {
        return ACD_INVALID_CRASHDUMP_DISCOVERY_PARAMS;
    }

    cJSON_ArrayForEach(it, cmdInOut->in.params)
    {
        switch (position)
        {
            case 0:
                params.addr = it->valueint;
                break;
            case 1:
                params.subopcode = it->valueint;
                break;
            case 2:
                params.param0 = it->valueint;
                break;
            case 3:
                params.param1 = it->valueint;
                break;
            case 4:
                params.param2 = it->valueint;
                break;
            case 5:
                params.rx_len = it->valueint;
                cmdInOut->out.size = params.rx_len;
                break;
            default:
                return ACD_INVALID_CRASHDUMP_DISCOVERY_PARAMS;
        }
        position++;
    }
    cmdInOut->out.ret = peci_CrashDump_Discovery(
        params.addr, params.subopcode, params.param0, params.param1,
        params.param2, params.rx_len, (uint8_t*)&cmdInOut->out.val.u64,
        &cmdInOut->out.cc);
    UpdateInternalVar(cmdInOut);
    return ACD_SUCCESS;
}

static acdStatus CrashDump_GetFrame(CmdInOut* cmdInOut)
{
    struct peci_crashdump_disc_msg params = {0};
    int position = 0;
    cJSON* it = NULL;

    if (!IsCrashDump_GetFrameParamsValid(cmdInOut->in.params))
    {
        return ACD_INVALID_CRASHDUMP_GETFRAME_PARAMS;
    }

    cJSON_ArrayForEach(it, cmdInOut->in.params)
    {
        switch (position)
        {
            case 0:
                params.addr = it->valueint;
                break;
            case 1:
                params.param0 = it->valueint;
                break;
            case 2:
                params.param1 = it->valueint;
                break;
            case 3:
                params.param2 = it->valueint;
                break;
            case 4:
                params.rx_len = it->valueint;
                cmdInOut->out.size = params.rx_len;
                break;
            default:
                return ACD_INVALID_CRASHDUMP_GETFRAME_PARAMS;
        }
        position++;
    }

    cmdInOut->out.ret = peci_CrashDump_GetFrame(
        params.addr, params.param0, params.param1, params.param2, params.rx_len,
        (uint8_t*)&cmdInOut->out.val.u64, &cmdInOut->out.cc);
    UpdateInternalVar(cmdInOut);
    return ACD_SUCCESS;
}

static acdStatus Ping(CmdInOut* cmdInOut)
{
    if (!IsPingParamsValid(cmdInOut->in.params))
    {
        return ACD_INVALID_PING_PARAMS;
    }

    cmdInOut->out.ret = peci_Ping(cmdInOut->in.params->valueint);
    return ACD_SUCCESS;
}

static acdStatus GetCPUID(CmdInOut* cmdInOut)
{
    if (!IsGetCPUIDParamsValid(cmdInOut->in.params))
    {
        return ACD_INVALID_PING_PARAMS;
    }

    cmdInOut->out.ret = peci_GetCPUID(
        cmdInOut->in.params->valueint, &cmdInOut->out.cpuID.cpuModel,
        &cmdInOut->out.cpuID.stepping, &cmdInOut->out.cc);
    return ACD_SUCCESS;
}

static acdStatus RdIAMSR(CmdInOut* cmdInOut)
{
    struct peci_rd_ia_msr_msg params = {0};
    int position = 0;
    cJSON* it = NULL;

    if (!IsRdIAMSRParamsValid(cmdInOut->in.params))
    {
        return ACD_INVALID_RDIAMSR_PARAMS;
    }

    cJSON_ArrayForEach(it, cmdInOut->in.params)
    {
        switch (position)
        {
            case 0:
                params.addr = it->valueint;
                break;
            case 1:
                params.thread_id = it->valueint;
                break;
            case 2:
                params.address = it->valueint;
                break;
            default:
                return ACD_INVALID_RDIAMSR_PARAMS;
        }
        position++;
    }
    cmdInOut->out.size = sizeof(uint64_t);
    cmdInOut->out.ret =
        peci_RdIAMSR(params.addr, params.thread_id, params.address,
                     (uint64_t*)&cmdInOut->out.val.u64, &cmdInOut->out.cc);
    UpdateInternalVar(cmdInOut);
    return ACD_SUCCESS;
}

static acdStatus RdPkgConfig(CmdInOut* cmdInOut)
{
    struct peci_rd_pkg_cfg_msg params = {0};
    int position = 0;
    cJSON* it = NULL;

    if (!IsRdPkgConfigParamsValid(cmdInOut->in.params))
    {
        return ACD_INVALID_RDPKGCONFIG_PARAMS;
    }

    cJSON_ArrayForEach(it, cmdInOut->in.params)
    {
        switch (position)
        {
            case 0:
                params.addr = it->valueint;
                break;
            case 1:
                params.index = it->valueint;
                break;
            case 2:
                params.param = it->valueint;
                break;
            case 3:
                params.rx_len = it->valueint;
                cmdInOut->out.size = params.rx_len;
                break;
            default:
                return ACD_INVALID_RDPKGCONFIG_PARAMS;
        }
        position++;
    }

    cmdInOut->out.ret =
        peci_RdPkgConfig(params.addr, params.index, params.param, params.rx_len,
                         (uint8_t*)&cmdInOut->out.val.u64, &cmdInOut->out.cc);
    UpdateInternalVar(cmdInOut);
    return ACD_SUCCESS;
}

static acdStatus RdEndPointConfigPciLocal(CmdInOut* cmdInOut)
{
    struct peci_rd_end_pt_cfg_msg params = {0};
    int position = 0;
    cJSON* it = NULL;
    const int wordFrameSize = (sizeof(uint64_t) / sizeof(uint16_t));

    if (!IsRdEndPointConfigPciLocalParamsValid(cmdInOut->in.params))
    {
        return ACD_INVALID_RDENDPOINTCONFIGPCILOCAL_PARAMS;
    }

    cJSON_ArrayForEach(it, cmdInOut->in.params)
    {
        switch (position)
        {
            case 0:
                params.addr = it->valueint;
                break;
            case 1:
                params.params.pci_cfg.seg = it->valueint;
                break;
            case 2:
                params.params.pci_cfg.bus = it->valueint;
                break;
            case 3:
                params.params.pci_cfg.device = it->valueint;
                break;
            case 4:
                params.params.pci_cfg.function = it->valueint;
                break;
            case 5:
                params.params.pci_cfg.reg = it->valueint;
                break;
            case 6:
                params.rx_len = it->valueint;
                cmdInOut->out.size = params.rx_len;
                break;
            default:
                return ACD_INVALID_RDENDPOINTCONFIGPCILOCAL_PARAMS;
        }
        position++;
    }
    switch (params.rx_len)
    {
        case sizeof(uint8_t):
        case sizeof(uint16_t):
        case sizeof(uint32_t):
            cmdInOut->out.ret = peci_RdEndPointConfigPciLocal(
                params.addr, params.params.pci_cfg.seg,
                params.params.pci_cfg.bus, params.params.pci_cfg.device,
                params.params.pci_cfg.function, params.params.pci_cfg.reg,
                params.rx_len, (uint8_t*)&cmdInOut->out.val.u64,
                &cmdInOut->out.cc);
            break;
        case sizeof(uint64_t):
            for (uint8_t u8Dword = 0; u8Dword < 2; u8Dword++)
            {
                cmdInOut->out.ret = peci_RdEndPointConfigPciLocal(
                    params.addr, params.params.pci_cfg.seg,
                    params.params.pci_cfg.bus, params.params.pci_cfg.device,
                    params.params.pci_cfg.function,
                    params.params.pci_cfg.reg + (u8Dword * wordFrameSize),
                    sizeof(uint32_t), (uint8_t*)&cmdInOut->out.val.u32[u8Dword],
                    &cmdInOut->out.cc);
            }
            break;
    }
    UpdateInternalVar(cmdInOut);
    return ACD_SUCCESS;
}

static acdStatus RdEndPointConfigMmio(CmdInOut* cmdInOut)
{
    struct peci_rd_end_pt_cfg_msg params = {0};
    int position = 0;
    cJSON* it = NULL;

    if (!IsRdEndPointConfigMmioParamsValid(cmdInOut->in.params))
    {
        return ACD_INVALID_RDENDPOINTCONFIGMMIO_PARAMS;
    }

    cJSON_ArrayForEach(it, cmdInOut->in.params)
    {
        switch (position)
        {
            case 0:
                params.addr = it->valueint;
                break;
            case 1:
                params.params.mmio.seg = it->valueint;
                break;
            case 2:
                params.params.mmio.bus = it->valueint;
                break;
            case 3:
                params.params.mmio.device = it->valueint;
                break;
            case 4:
                params.params.mmio.function = it->valueint;
                break;
            case 5:
                params.params.mmio.bar = it->valueint;
                break;
            case 6:
                params.params.mmio.addr_type = it->valueint;
                break;
            case 7:
                params.params.mmio.offset = (uint64_t)it->valueint;
                break;
            case 8:
                params.rx_len = it->valueint;
                cmdInOut->out.size = params.rx_len;
                break;
            default:
                return ACD_INVALID_RDENDPOINTCONFIGMMIO_PARAMS;
        }
        position++;
    }
    cmdInOut->out.ret = peci_RdEndPointConfigMmio(
        params.addr, params.params.mmio.seg, params.params.mmio.bus,
        params.params.mmio.device, params.params.mmio.function,
        params.params.mmio.bar, params.params.mmio.addr_type,
        params.params.mmio.offset, params.rx_len,
        (uint8_t*)&cmdInOut->out.val.u64, &cmdInOut->out.cc);
    UpdateInternalVar(cmdInOut);
    return ACD_SUCCESS;
}

static acdStatus RdPostEnumBus(CmdInOut* cmdInOut)
{
    PostEnumBus params = {0};
    cJSON* it;
    int position = 0;

    if (!IsRdPostEnumBusParamsValid(cmdInOut->in.params))
    {
        return ACD_INVALID_RDPOSTENUMBUS_PARAMS;
    }
    cJSON_ArrayForEach(it, cmdInOut->in.params)
    {
        switch (position)
        {
            case 0:
                params.cpubusno_valid = (uint32_t)it->valuedouble;
                break;
            case 1:
                params.cpubusno = (uint32_t)it->valuedouble;
                break;
            case 2:
                params.bitToCheck = it->valueint;
                break;
            default:
                return ACD_INVALID_RDPOSTENUMBUS_PARAMS;
        }
        position++;
    }
    if (0 == CHECK_BIT(params.cpubusno_valid, params.bitToCheck))
    {
        CRASHDUMP_PRINT(ERR, stderr,
                        "Bus 30 does not contain valid post enumerated bus"
                        "number! (0x%x)\n",
                        params.cpubusno_valid);
        return ACD_FAILURE;
    }
    cmdInOut->out.size = sizeof(uint8_t);
    cmdInOut->out.val.u32[0] = ((params.cpubusno >> 16) & 0xff);
    UpdateInternalVar(cmdInOut);

    // Set ret & cc to success for logger
    cmdInOut->out.ret = PECI_CC_SUCCESS;
    cmdInOut->out.cc = PECI_DEV_CC_SUCCESS;
    return ACD_SUCCESS;
}

static acdStatus RdChaCount(CmdInOut* cmdInOut)
{
    ChaCount params = {0};
    cJSON* it;
    int position = 0;
    uint8_t chaCountValue;

    if (!IsRdChaCountParamsValid(cmdInOut->in.params))
    {
        return ACD_INVALID_RDCHACOUNT_PARAMS;
    }
    cJSON_ArrayForEach(it, cmdInOut->in.params)
    {
        switch (position)
        {
            case 0:
                params.chaMask0 = (uint32_t)it->valuedouble;
                break;
            case 1:
                params.chaMask1 = (uint32_t)it->valuedouble;
                break;
            default:
                return ACD_INVALID_RDCHACOUNT_PARAMS;
        }
        position++;
    }

    cmdInOut->out.size = sizeof(uint8_t);
    cmdInOut->out.val.u32[0] = __builtin_popcount((int)params.chaMask0) +
                               __builtin_popcount((int)params.chaMask1);
    UpdateInternalVar(cmdInOut);

    // Set ret & cc to success for logger
    cmdInOut->out.ret = PECI_CC_SUCCESS;
    cmdInOut->out.cc = PECI_DEV_CC_SUCCESS;
    return ACD_SUCCESS;
}

static acdStatus (*cmds[CD_NUM_OF_PECI_CMDS])() = {
    (acdStatus(*)())CrashDump_Discovery,
    (acdStatus(*)())CrashDump_GetFrame,
    (acdStatus(*)())Ping,
    (acdStatus(*)())GetCPUID,
    (acdStatus(*)())RdEndPointConfigMmio,
    (acdStatus(*)())RdEndPointConfigPciLocal,
    (acdStatus(*)())RdIAMSR,
    (acdStatus(*)())RdPkgConfig,
    (acdStatus(*)())RdPostEnumBus,
    (acdStatus(*)())RdChaCount,
};

static char* inputCMDs[CD_NUM_OF_PECI_CMDS] = {
    "CrashDumpDiscovery",
    "CrashDumpGetFrame",
    "Ping",
    "GetCPUID",
    "RdEndpointConfigMMIO",
    "RdEndpointConfigPCILocal",
    "RdIAMSR",
    "RdPkgConfig",
    "RdPostEnumBus",
    "RdChaCount",
};

acdStatus BuildCmdsTable(ENTRY* entry)
{
    ENTRY* ep;

    hcreate(CD_NUM_OF_PECI_CMDS);
    for (int i = 0; i < CD_NUM_OF_PECI_CMDS; i++)
    {
        entry->key = inputCMDs[i];
        entry->data = (void*)(size_t)i;
        ep = hsearch(*entry, ENTER);
        if (ep == NULL)
        {
            CRASHDUMP_PRINT(ERR, stderr, "Fail adding (%s) to commands table\n",
                            entry->key);
            return ACD_FAILURE_CMD_TABLE;
        }
    }
    return ACD_SUCCESS;
}

acdStatus Execute(ENTRY* entry, CmdInOut* cmdInOut)
{
    ENTRY* ep;

    ep = hsearch(*entry, FIND);
    if (ep == NULL)
    {
        CRASHDUMP_PRINT(ERR, stderr, "Invalid PECICmd:(%s)\n", entry->key);
        return ACD_INVALID_CMD;
    }
    cmds[(size_t)(ep->data)](cmdInOut);
    return ACD_SUCCESS;
}
