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

#include "TorDump.hpp"

extern "C" {
#include <cjson/cJSON.h>
#include <stdio.h>
#include <stdlib.h>
}

#include "crashdump.hpp"

/******************************************************************************
 *
 *   torDumpJsonCPX1
 *
 *   This function formats the TOR dump into a JSON object
 *
 ******************************************************************************/
static void torDumpJsonCPX1(uint32_t u32NumReads, uint32_t* pu32TorDump,
                            cJSON* pJsonChild)
{
    cJSON* channel;
    cJSON* tor;
    char jsonItemName[TD_JSON_STRING_LEN];
    char jsonItemString[TD_JSON_STRING_LEN];
    uint8_t u8ChannelNum = 0;
    uint8_t u8TorNum, u8DwordNum;
    uint32_t u32TorIndex = 0;

    // Add the TOR dump info to the TOR dump JSON structure
    while (u32TorIndex < u32NumReads)
    {
        // Add the channel number item to the TOR dump JSON structure
        cd_snprintf_s(jsonItemName, TD_JSON_STRING_LEN, TD_JSON_CHA_NAME,
                      u8ChannelNum++);
        cJSON_AddItemToObject(pJsonChild, jsonItemName,
                              channel = cJSON_CreateObject());

        // Add the TOR data for this channel
        for (u8TorNum = 0; u8TorNum < TD_TORS_PER_CHA_CPX1; u8TorNum++)
        {
            // Add the TOR number item to the TOR dump JSON structure
            cd_snprintf_s(jsonItemName, TD_JSON_STRING_LEN, TD_JSON_TOR_NAME,
                          u8TorNum);
            cJSON_AddItemToObject(channel, jsonItemName,
                                  tor = cJSON_CreateObject());

            // Add the data for this TOR
            for (u8DwordNum = 0; u8DwordNum < TD_SUBINDEX_PER_TOR_CPX1;
                 u8DwordNum++)
            {
                // Add the DWORD number item to the TOR dump JSON structure
                cd_snprintf_s(jsonItemName, TD_JSON_STRING_LEN,
                              TD_JSON_SUBINDEX_NAME, u8DwordNum);
                cd_snprintf_s(jsonItemString, TD_JSON_STRING_LEN, "0x%x",
                              pu32TorDump[u32TorIndex++]);
                cJSON_AddStringToObject(tor, jsonItemName, jsonItemString);
            }
        }
    }
}

/******************************************************************************
 *
 *    logTorDumpCPX1
 *
 *    BMC performs the TOR dump retrieve from the processor directly via
 *    PECI interface after a platform three (3) strike failure.  The PECI flow
 *is listed below to generate a TOR Dump, and decode it.
 *
 *    WrPkgConfig() -
 *         0x80 0x0003 0x00030002
 *         Open TOR Dump Sequence
 *
 *    WrPkgConfig() -
 *         0x80 0x0001 0x80000000
 *         Set Parameter 0
 *
 *    RdPkgConfig() -
 *         0x80 0x3001
 *         Start TOR dump sequence
 *
 *    RdPkgConfig() -
 *         0x80 0x0002
 *         Returns the number of additional RdPkgConfig() commands required to
 *collect all the data for the dump.
 *
 *    RdPkgConfig() * N -
 *         0x80 0x0002
 *         TOR Dump data [1-N]
 *
 *    WrPkgConfig() -
 *         0x80 0x0004 0x00030002 Close TOR Dump Sequence.
 *
 ******************************************************************************/
int logTorDumpCPX1(crashdump::CPUInfo& cpuInfo, cJSON* pJsonChild)
{
    EPECIStatus ePECIStatus = PECI_CC_SUCCESS;
    int peci_fd = -1;
    uint8_t cc = 0;

    if (peci_Lock(&peci_fd, PECI_WAIT_FOREVER) != PECI_CC_SUCCESS)
    {
        return 1;
    }

    // Start the TOR dump log

    // Open the TOR dump sequence
    ePECIStatus =
        peci_WrPkgConfig_seq(cpuInfo.clientAddr, MBX_INDEX_VCU, VCU_OPEN_SEQ,
                             VCU_TOR_DUMP_SEQ, sizeof(uint32_t), peci_fd, &cc);
    if (ePECIStatus != PECI_CC_SUCCESS)
    {
        // TOR dump sequence failed, abort the sequence
        peci_WrPkgConfig_seq(cpuInfo.clientAddr, MBX_INDEX_VCU, VCU_ABORT_SEQ,
                             VCU_TOR_DUMP_SEQ, sizeof(uint32_t), peci_fd, &cc);
        return 1;
    }

    // Set TOR dump parameter 0
    ePECIStatus =
        peci_WrPkgConfig_seq(cpuInfo.clientAddr, MBX_INDEX_VCU, VCU_SET_PARAM,
                             TD_PARAM_ZERO, sizeof(uint32_t), peci_fd, &cc);
    if (ePECIStatus != PECI_CC_SUCCESS)
    {
        // TOR dump sequence failed, abort the sequence
        peci_WrPkgConfig_seq(cpuInfo.clientAddr, MBX_INDEX_VCU, VCU_ABORT_SEQ,
                             VCU_TOR_DUMP_SEQ, sizeof(uint32_t), peci_fd, &cc);
        return 1;
    }

    // Start the TOR dump
    uint8_t u8StartData[4];
    ePECIStatus =
        peci_RdPkgConfig_seq(cpuInfo.clientAddr, MBX_INDEX_VCU, TD_START_PARAM,
                             sizeof(uint32_t), u8StartData, peci_fd, &cc);
    if (ePECIStatus != PECI_CC_SUCCESS)
    {
        // TOR dump sequence failed, abort the sequence
        peci_WrPkgConfig_seq(cpuInfo.clientAddr, MBX_INDEX_VCU, VCU_ABORT_SEQ,
                             VCU_TOR_DUMP_SEQ, sizeof(uint32_t), peci_fd, &cc);
        return 1;
    }

    // Get the number of dwords to read
    uint32_t u32NumReads = 0;
    ePECIStatus = peci_RdPkgConfig_seq(cpuInfo.clientAddr, MBX_INDEX_VCU,
                                       VCU_READ, sizeof(uint32_t),
                                       (uint8_t*)&u32NumReads, peci_fd, &cc);
    if (ePECIStatus != PECI_CC_SUCCESS)
    {
        // TOR dump sequence failed, abort the sequence
        peci_WrPkgConfig_seq(cpuInfo.clientAddr, MBX_INDEX_VCU, VCU_ABORT_SEQ,
                             VCU_TOR_DUMP_SEQ, sizeof(uint32_t), peci_fd, &cc);
        return 1;
    }

    // Get the raw data
    uint32_t* pu32TorDump;
    pu32TorDump = (uint32_t*)calloc(u32NumReads, sizeof(uint32_t));
    if (pu32TorDump == NULL)
    {
        // calloc failed, abort the sequence
        peci_WrPkgConfig_seq(cpuInfo.clientAddr, MBX_INDEX_VCU, VCU_ABORT_SEQ,
                             VCU_TOR_DUMP_SEQ, sizeof(uint32_t), peci_fd, &cc);
        return 1;
    }
    for (int i = 0; i < u32NumReads; i++)
    {
        ePECIStatus = peci_RdPkgConfig_seq(
            cpuInfo.clientAddr, MBX_INDEX_VCU, VCU_READ, sizeof(uint32_t),
            (uint8_t*)&pu32TorDump[i], peci_fd, &cc);
        if (ePECIStatus != PECI_CC_SUCCESS)
        {
            // TOR dump sequence failed, note the number of dwords read and
            // abort the sequence
            u32NumReads = i;
            peci_WrPkgConfig_seq(cpuInfo.clientAddr, MBX_INDEX_VCU,
                                 VCU_ABORT_SEQ, VCU_TOR_DUMP_SEQ,
                                 sizeof(uint32_t), peci_fd, &cc);
            break;
        }
    }

    // Close the TOR dump sequence
    peci_WrPkgConfig_seq(cpuInfo.clientAddr, MBX_INDEX_VCU, VCU_CLOSE_SEQ,
                         VCU_TOR_DUMP_SEQ, sizeof(uint32_t), peci_fd, &cc);

    // Log the TOR dump
    torDumpJsonCPX1(u32NumReads, pu32TorDump, pJsonChild);

    free(pu32TorDump);

    peci_Unlock(peci_fd);
    return 0;
}

/******************************************************************************
 *
 *   torDumpJsonICX1
 *
 *   This function formats the TOR dump into a JSON object
 *
 ******************************************************************************/
static void torDumpJsonICX2(uint32_t u32Cha, uint32_t u32TorIndex,
                            uint32_t u32TorSubIndex, uint32_t u32PayloadBytes,
                            uint8_t* pu8TorCrashdumpData, cJSON* pJsonChild,
                            bool bInvalid, uint8_t cc)
{
    cJSON* channel;
    cJSON* tor;
    char jsonItemName[TD_JSON_STRING_LEN];
    char jsonItemString[TD_JSON_STRING_LEN];

    // Add the channel number item to the TOR dump JSON structure only if it
    // doesn't already exist
    cd_snprintf_s(jsonItemName, TD_JSON_STRING_LEN, TD_JSON_CHA_NAME, u32Cha);
    if ((channel = cJSON_GetObjectItemCaseSensitive(pJsonChild,
                                                    jsonItemName)) == NULL)
    {
        cJSON_AddItemToObject(pJsonChild, jsonItemName,
                              channel = cJSON_CreateObject());
    }

    // Add the TOR Index item to the TOR dump JSON structure only if it
    // doesn't already exist
    cd_snprintf_s(jsonItemName, TD_JSON_STRING_LEN, TD_JSON_TOR_NAME,
                  u32TorIndex);
    if ((tor = cJSON_GetObjectItemCaseSensitive(channel, jsonItemName)) == NULL)
    {
        cJSON_AddItemToObject(channel, jsonItemName,
                              tor = cJSON_CreateObject());
    }

    if (bInvalid)
    {
        cd_snprintf_s(jsonItemString, TD_JSON_STRING_LEN, TD_NA);
        cJSON_AddStringToObject(tor, jsonItemName, jsonItemString);
        return;
    }

    if (PECI_CC_UA(cc))
    {
        cd_snprintf_s(jsonItemString, TD_JSON_STRING_LEN, TD_UA, cc);
    }
    else
    {
        // Add the SubIndex data to the TOR dump JSON structure
        cd_snprintf_s(jsonItemName, TD_JSON_STRING_LEN, TD_JSON_SUBINDEX_NAME,
                      u32TorSubIndex);

        cd_snprintf_s(jsonItemString, sizeof(jsonItemString), "0x0");
        bool leading = true;
        char* ptr = &jsonItemString[2];

        for (int i = u32PayloadBytes - 1; i >= 0; i--)
        {
            // exclude any leading zeros per schema
            if (leading && pu8TorCrashdumpData[i] == 0)
            {
                continue;
            }
            leading = false;

            ptr += cd_snprintf_s(ptr, u32PayloadBytes, "%02x",
                                 pu8TorCrashdumpData[i]);
        }
    }

    cJSON_AddStringToObject(tor, jsonItemName, jsonItemString);
}

/******************************************************************************
 *
 *    logTorDumpICX1
 *
 *    BMC performs the TOR dump retrieve from the processor directly via
 *    PECI interface after a platform three (3) strike failure.
 *
 ******************************************************************************/
int logTorDumpICX1(crashdump::CPUInfo& cpuInfo, cJSON* pJsonChild)
{
    // Not supported in A0
    return 0;
}

/******************************************************************************
 *
 *    logTorDumpICX2
 *
 *    BMC performs the TOR dump retrieve from the processor directly via
 *    PECI interface after a platform three (3) strike failure.
 *
 ******************************************************************************/
int logTorDumpICX2(crashdump::CPUInfo& cpuInfo, cJSON* pJsonChild)
{
    EPECIStatus ePeciStatus = PECI_CC_SUCCESS;
    uint8_t u8CrashdumpEnabled = 1;
    uint16_t u16CrashdumpNumAgents;
    uint64_t u64UniqueId;
    uint64_t u64PayloadExp;
    int ret = 0;
    uint8_t cc = 0;

    // Crashdump Discovery
    // Crashdump Enabled
    ePeciStatus = peci_CrashDump_Discovery(
        cpuInfo.clientAddr, PECI_CRASHDUMP_ENABLED, 0, 0, 0, sizeof(uint8_t),
        &u8CrashdumpEnabled, &cc);
    if ((ePeciStatus != PECI_CC_SUCCESS) || (u8CrashdumpEnabled != 0))
    {
        fprintf(stderr,
                "Tor Crashdump is disabled (%d) during discovery "
                "(disabled:%d)\n",
                ePeciStatus, u8CrashdumpEnabled);
        return 1;
    }

    // Crashdump Number of Agents
    ePeciStatus = peci_CrashDump_Discovery(
        cpuInfo.clientAddr, PECI_CRASHDUMP_NUM_AGENTS, 0, 0, 0,
        sizeof(uint16_t), (uint8_t*)&u16CrashdumpNumAgents, &cc);
    if (ePeciStatus != PECI_CC_SUCCESS ||
        u16CrashdumpNumAgents <= PECI_CRASHDUMP_TOR)
    {
        fprintf(stderr, "Error (%d) during discovery (num of agents:%d)\n",
                ePeciStatus, u16CrashdumpNumAgents);
        return 1;
    }

    // Crashdump Agent Data
    // Agent Unique ID
    ePeciStatus = peci_CrashDump_Discovery(
        cpuInfo.clientAddr, PECI_CRASHDUMP_AGENT_DATA, PECI_CRASHDUMP_AGENT_ID,
        PECI_CRASHDUMP_TOR, 0, sizeof(uint64_t), (uint8_t*)&u64UniqueId, &cc);
    if (ePeciStatus != PECI_CC_SUCCESS)
    {
        fprintf(stderr, "Error (%d) during discovery (id:0x%" PRIx64 ")\n",
                ePeciStatus, u64UniqueId);
        return 1;
    }

    // Agent Payload Size
    ePeciStatus =
        peci_CrashDump_Discovery(cpuInfo.clientAddr, PECI_CRASHDUMP_AGENT_DATA,
                                 PECI_CRASHDUMP_AGENT_PARAM, PECI_CRASHDUMP_TOR,
                                 PECI_CRASHDUMP_PAYLOAD_SIZE, sizeof(uint64_t),
                                 (uint8_t*)&u64PayloadExp, &cc);
    if (ePeciStatus != PECI_CC_SUCCESS)
    {
        fprintf(stderr, "Error (%d) during discovery (payload:0x%" PRIx64 ")\n",
                ePeciStatus, u64PayloadExp);
        return 1;
    }

    uint32_t u32PayloadBytes = 1 << u64PayloadExp;

    // Crashdump Get Frames
    for (uint32_t u32Cha = 0; u32Cha < cpuInfo.chaCount; u32Cha++)
    {
        for (uint32_t u32TorIndex = 0; u32TorIndex < TD_TORS_PER_CHA_ICX1;
             u32TorIndex++)
        {
            for (uint32_t u32TorSubIndex = 0;
                 u32TorSubIndex < TD_SUBINDEX_PER_TOR_ICX1; u32TorSubIndex++)
            {
                uint8_t* pu8TorCrashdumpData =
                    (uint8_t*)(calloc(u32PayloadBytes, sizeof(uint8_t)));
                bool bInvalid = false;
                if (pu8TorCrashdumpData == NULL)
                {
                    fprintf(stderr, "Error (%d) memory allocation\n",
                            ePeciStatus);
                    return 1;
                }
                ePeciStatus = peci_CrashDump_GetFrame(
                    cpuInfo.clientAddr, PECI_CRASHDUMP_TOR, u32Cha,
                    (u32TorIndex | (u32TorSubIndex << 8)), u32PayloadBytes,
                    pu8TorCrashdumpData, &cc);

                if (ePeciStatus != PECI_CC_SUCCESS)
                {
                    bInvalid = true;
                    fprintf(stderr,
                            "Error (%d) during GetFrame"
                            "(cha:%d index:%d sub-index:%d)\n",
                            ePeciStatus, u32Cha, u32TorIndex, u32TorSubIndex);
                    ret = 1;
                }
                torDumpJsonICX2(u32Cha, u32TorIndex, u32TorSubIndex,
                                u32PayloadBytes, pu8TorCrashdumpData,
                                pJsonChild, bInvalid, cc);

                free(pu8TorCrashdumpData);
            }
        }
    }

    return ret;
}

static const STorDumpVx sTorDumpVx[] = {
    {crashdump::cpu::clx, logTorDumpCPX1},
    {crashdump::cpu::cpx, logTorDumpCPX1},
    {crashdump::cpu::skx, logTorDumpCPX1},
    {crashdump::cpu::icx, logTorDumpICX1},
    {crashdump::cpu::icx2, logTorDumpICX2},
};

/******************************************************************************
 *
 *    logTorDump
 *
 *    BMC performs the TOR dump retrieve from the processor directly via
 *    PECI interface after a platform three (3) strike failure.
 *
 ******************************************************************************/
int logTorDump(crashdump::CPUInfo& cpuInfo, cJSON* pJsonChild)
{
    if (pJsonChild == NULL)
    {
        return 1;
    }

    for (uint32_t i = 0; i < (sizeof(sTorDumpVx) / sizeof(STorDumpVx)); i++)
    {
        if (cpuInfo.model == sTorDumpVx[i].cpuModel)
        {
            return sTorDumpVx[i].logTorDumpVx(cpuInfo, pJsonChild);
        }
    }

    fprintf(stderr, "Cannot find version for %s\n", __FUNCTION__);
    return 1;
}
