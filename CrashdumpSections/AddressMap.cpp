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

#include "AddressMap.hpp"

extern "C" {
#include <cjson/cJSON.h>
#include <stdio.h>
}

#include "crashdump.hpp"

/******************************************************************************
 *
 *   addressMapJson
 *
 *   This function formats the Address Map entry into a JSON object
 *
 ******************************************************************************/
static void addressMapJson(const SAddrMapEntry* sAddrMapEntry,
                           uint64_t* u64Data, cJSON* pJsonChild)
{
    // Add the register if it's valid
    if (u64Data != NULL)
    {
        char jsonItemString[AM_JSON_STRING_LEN];
        cd_snprintf_s(jsonItemString, AM_JSON_STRING_LEN, AM_UINT64_FMT,
                      *u64Data);
        cJSON_AddStringToObject(pJsonChild, sAddrMapEntry->regName,
                                jsonItemString);
    }
    else
    {
        cJSON_AddStringToObject(pJsonChild, sAddrMapEntry->regName, AM_FAILED);
    }
}

/******************************************************************************
 *
 *   logAddressMapEntries
 *
 *   This function gathers the Address Map log from the specified PECI client
 *   and adds it to the debug log
 *
 ******************************************************************************/
int logAddressMapEntries(crashdump::CPUInfo& cpuInfo,
                         const SAddrMapEntry* sAddrMapEntries,
                         int numAddrMapEntries, cJSON* pJsonChild)
{
    int ret = 0;
    int peci_fd = -1;
    uint8_t cc = 0;

    if (peci_Lock(&peci_fd, PECI_WAIT_FOREVER) != PECI_CC_SUCCESS)
    {
        return 1;
    }

    // Get the Address Map register values
    for (uint32_t i = 0; i < numAddrMapEntries; i++)
    {
        UAddrMapRegValue uValue = {};
        uint64_t* pValue = &uValue.u64;
        if (sAddrMapEntries[i].u8Size == 8)
        {
            for (uint8_t u8Dword = 0; u8Dword < 2; u8Dword++)
            {
                if (peci_RdPCIConfigLocal_seq(
                        cpuInfo.clientAddr, sAddrMapEntries[i].u8Bus,
                        sAddrMapEntries[i].u8Dev, sAddrMapEntries[i].u8Func,
                        sAddrMapEntries[i].u16Reg + (u8Dword * 4),
                        sizeof(uint32_t), (uint8_t*)&uValue.u32[u8Dword],
                        peci_fd, &cc) != PECI_CC_SUCCESS)
                {
                    pValue = NULL;
                    ret = 1;
                    break;
                }
            }
        }
        else
        {
            if (peci_RdPCIConfigLocal_seq(
                    cpuInfo.clientAddr, sAddrMapEntries[i].u8Bus,
                    sAddrMapEntries[i].u8Dev, sAddrMapEntries[i].u8Func,
                    sAddrMapEntries[i].u16Reg, sAddrMapEntries[i].u8Size,
                    (uint8_t*)&uValue.u64, peci_fd, &cc) != PECI_CC_SUCCESS)
            {
                pValue = NULL;
                ret = 1;
            }
        }
        addressMapJson(&sAddrMapEntries[i], pValue, pJsonChild);
    }

    peci_Unlock(peci_fd);
    return ret;
}

static const SAddrMapEntry sAddrMapEntriesCPX1[] = {
    // Register, Bus, Dev, Fun, Offset, Size
    {"DRAM_RULE_CFG_0", 1, 29, 0, 0x60, 4},
    {"DRAM_RULE_CFG_1", 1, 29, 0, 0x68, 4},
    {"DRAM_RULE_CFG_10", 1, 29, 0, 0xB0, 4},
    {"DRAM_RULE_CFG_11", 1, 29, 0, 0xB8, 4},
    {"DRAM_RULE_CFG_12", 1, 29, 0, 0xC0, 4},
    {"DRAM_RULE_CFG_13", 1, 29, 0, 0xC8, 4},
    {"DRAM_RULE_CFG_14", 1, 29, 0, 0xD0, 4},
    {"DRAM_RULE_CFG_15", 1, 29, 0, 0xD8, 4},
    {"DRAM_RULE_CFG_16", 1, 29, 0, 0xE0, 4},
    {"DRAM_RULE_CFG_17", 1, 29, 0, 0xE8, 4},
    {"DRAM_RULE_CFG_18", 1, 29, 0, 0xF0, 4},
    {"DRAM_RULE_CFG_19", 1, 29, 0, 0xF8, 4},
    {"DRAM_RULE_CFG_2", 1, 29, 0, 0x70, 4},
    {"DRAM_RULE_CFG_20", 1, 29, 0, 0x100, 4},
    {"DRAM_RULE_CFG_21", 1, 29, 0, 0x108, 4},
    {"DRAM_RULE_CFG_22", 1, 29, 0, 0x110, 4},
    {"DRAM_RULE_CFG_23", 1, 29, 0, 0x118, 4},
    {"DRAM_RULE_CFG_3", 1, 29, 0, 0x78, 4},
    {"DRAM_RULE_CFG_4", 1, 29, 0, 0x80, 4},
    {"DRAM_RULE_CFG_5", 1, 29, 0, 0x88, 4},
    {"DRAM_RULE_CFG_6", 1, 29, 0, 0x90, 4},
    {"DRAM_RULE_CFG_7", 1, 29, 0, 0x98, 4},
    {"DRAM_RULE_CFG_8", 1, 29, 0, 0xA0, 4},
    {"DRAM_RULE_CFG_9", 1, 29, 0, 0xA8, 4},
    {"INTERLEAVE_LIST_CFG_0", 1, 29, 0, 0x64, 4},
    {"INTERLEAVE_LIST_CFG_1", 1, 29, 0, 0x6C, 4},
    {"INTERLEAVE_LIST_CFG_10", 1, 29, 0, 0xB4, 4},
    {"INTERLEAVE_LIST_CFG_11", 1, 29, 0, 0xBC, 4},
    {"INTERLEAVE_LIST_CFG_12", 1, 29, 0, 0xC4, 4},
    {"INTERLEAVE_LIST_CFG_13", 1, 29, 0, 0xCC, 4},
    {"INTERLEAVE_LIST_CFG_14", 1, 29, 0, 0xD4, 4},
    {"INTERLEAVE_LIST_CFG_15", 1, 29, 0, 0xDC, 4},
    {"INTERLEAVE_LIST_CFG_16", 1, 29, 0, 0xE4, 4},
    {"INTERLEAVE_LIST_CFG_17", 1, 29, 0, 0xEC, 4},
    {"INTERLEAVE_LIST_CFG_18", 1, 29, 0, 0xF4, 4},
    {"INTERLEAVE_LIST_CFG_19", 1, 29, 0, 0xFC, 4},
    {"INTERLEAVE_LIST_CFG_2", 1, 29, 0, 0x74, 4},
    {"INTERLEAVE_LIST_CFG_20", 1, 29, 0, 0x104, 4},
    {"INTERLEAVE_LIST_CFG_21", 1, 29, 0, 0x10C, 4},
    {"INTERLEAVE_LIST_CFG_22", 1, 29, 0, 0x114, 4},
    {"INTERLEAVE_LIST_CFG_23", 1, 29, 0, 0x11C, 4},
    {"INTERLEAVE_LIST_CFG_3", 1, 29, 0, 0x7C, 4},
    {"INTERLEAVE_LIST_CFG_4", 1, 29, 0, 0x84, 4},
    {"INTERLEAVE_LIST_CFG_5", 1, 29, 0, 0x8C, 4},
    {"INTERLEAVE_LIST_CFG_6", 1, 29, 0, 0x94, 4},
    {"INTERLEAVE_LIST_CFG_7", 1, 29, 0, 0x9C, 4},
    {"INTERLEAVE_LIST_CFG_8", 1, 29, 0, 0xA4, 4},
    {"INTERLEAVE_LIST_CFG_9", 1, 29, 0, 0xAC, 4},
    {"IOAPIC_TARGET_LIST_CFG_0", 1, 29, 1, 0xD4, 4},
    {"IOAPIC_TARGET_LIST_CFG_1", 1, 29, 1, 0xD8, 4},
    {"IOAPIC_TARGET_LIST_CFG_2", 1, 29, 1, 0xDC, 4},
    {"IOAPIC_TARGET_LIST_CFG_3", 1, 29, 1, 0xE0, 4},
    {"IOPORT_TARGET_LIST_CFG_0", 1, 29, 0, 0x2B0, 4},
    {"IOPORT_TARGET_LIST_CFG_1", 1, 29, 0, 0x2B4, 4},
    {"IOPORT_TARGET_LIST_CFG_2", 1, 29, 0, 0x2B8, 4},
    {"IOPORT_TARGET_LIST_CFG_3", 1, 29, 0, 0x2BC, 4},
    {"LT_CONTROL", 1, 29, 1, 0xD0, 4},
    {"MENCMEM_BASE", 0, 5, 0, 0xF0, 8},
    {"MENCMEM_LIMIT", 0, 5, 0, 0xF8, 8},
    {"MESEG_BASE", 1, 29, 0, 0x50, 8},
    {"MESEG_LIMIT", 1, 29, 0, 0x58, 8},
    {"MMCFG_BASE", 0, 5, 0, 0x90, 8},
    {"MMCFG_LIMIT", 0, 5, 0, 0x98, 8},
    {"MMCFG_LOCAL_RULE", 1, 29, 1, 0xE4, 4},
    {"MMCFG_LOCAL_RULE_ADDRESS_CFG_0", 1, 29, 1, 0xC8, 4},
    {"MMCFG_LOCAL_RULE_ADDRESS_CFG_1", 1, 29, 1, 0xCC, 4},
    {"MMCFG_RULE", 1, 29, 1, 0xC0, 8},
    {"MMCFG_TARGET_LIST", 1, 29, 1, 0xEC, 4},
    {"MMIO_RULE_CFG_0", 1, 29, 1, 0x40, 8},
    {"MMIO_RULE_CFG_1", 1, 29, 1, 0x48, 8},
    {"MMIO_RULE_CFG_10", 1, 29, 1, 0x90, 8},
    {"MMIO_RULE_CFG_11", 1, 29, 1, 0x98, 8},
    {"MMIO_RULE_CFG_12", 1, 29, 1, 0xA0, 8},
    {"MMIO_RULE_CFG_13", 1, 29, 1, 0xA8, 8},
    {"MMIO_RULE_CFG_14", 1, 29, 1, 0xB0, 8},
    {"MMIO_RULE_CFG_15", 1, 29, 1, 0xB8, 8},
    {"MMIO_RULE_CFG_2", 1, 29, 1, 0x50, 8},
    {"MMIO_RULE_CFG_3", 1, 29, 1, 0x58, 8},
    {"MMIO_RULE_CFG_4", 1, 29, 1, 0x60, 8},
    {"MMIO_RULE_CFG_5", 1, 29, 1, 0x68, 8},
    {"MMIO_RULE_CFG_6", 1, 29, 1, 0x70, 8},
    {"MMIO_RULE_CFG_7", 1, 29, 1, 0x78, 8},
    {"MMIO_RULE_CFG_8", 1, 29, 1, 0x80, 8},
    {"MMIO_RULE_CFG_9", 1, 29, 1, 0x88, 8},
    {"MMIO_TARGET_INTERLEAVE_LIST_CFG_0", 1, 29, 1, 0x20, 4},
    {"MMIO_TARGET_INTERLEAVE_LIST_CFG_1", 1, 29, 1, 0x20, 4},
    {"MMIO_TARGET_INTERLEAVE_LIST_CFG_2", 1, 29, 1, 0x20, 4},
    {"MMIO_TARGET_INTERLEAVE_LIST_CFG_3", 1, 29, 1, 0x21, 4},
    {"MMIO_TARGET_LIST_CFG_0", 1, 29, 1, 0xE8, 4},
    {"MMIO_TARGET_LIST_CFG_1", 1, 29, 1, 0xF8, 4},
    {"MMIOH_INTERLEAVE_RULE", 1, 29, 1, 0x10, 8},
    {"MMIOH_NONINTERLEAVE_RULE", 1, 29, 1, 0x10, 8},
    {"NCMEM_BASE", 0, 5, 0, 0xE0, 8},
    {"NCMEM_LIMIT", 0, 5, 0, 0xE8, 8},
    {"PAM0123", 1, 29, 0, 0x40, 4},
    {"PAM456", 1, 29, 0, 0x44, 4},
    {"PCIE0_MMIOH_INTERLEAVE", 0, 5, 0, 0x348, 4},
    {"PCIE0_MMIOH_NON_INTERLEAVE", 0, 5, 0, 0x340, 8},
    {"PCIE1_MMIOH_INTERLEAVE", 1, 5, 0, 0x348, 4},
    {"PCIE1_MMIOH_NON_INTERLEAVE", 1, 5, 0, 0x340, 8},
    {"PCIE2_MMIOH_INTERLEAVE", 2, 5, 0, 0x348, 4},
    {"PCIE2_MMIOH_NON_INTERLEAVE", 2, 5, 0, 0x340, 8},
    {"PCIE3_MMIOH_INTERLEAVE", 3, 5, 0, 0x348, 4},
    {"PCIE3_MMIOH_NON_INTERLEAVE", 3, 5, 0, 0x340, 8},
    {"SAD_CONTROL", 1, 29, 1, 0xF4, 4},
    {"SAD_TARGET", 1, 29, 1, 0xF0, 4},
    {"TOHM", 0, 5, 0, 0xD8, 8},
    {"TOLM", 0, 5, 0, 0xD0, 8},
    {"TSEG", 0, 5, 0, 0xA8, 8},
};

static const SAddrMapEntry sAddrMapEntriesICX1[] = {
    // Register, Bus, Dev, Fun, Offset, Size
    {"TSEG", 0, 0, 0, 0xA8, 8},
    {"MESEG_BASE", 14, 29, 0, 0x90, 8},
    {"MESEG_LIMIT", 14, 29, 0, 0x98, 8},
    {"MMCFG_BASE", 0, 0, 0, 0x90, 8},
    {"MMCFG_LIMIT", 0, 0, 0, 0x98, 8},
    {"MMCFG_Rule", 13, 0, 2, 0xC8, 4},
    {"TOLM", 0, 0, 0, 0xD0, 4},
    {"TOHM", 0, 0, 0, 0xC8, 8},
    {"MMIO_RULE_CFG_0", 14, 29, 1, 0x108, 8},
    {"MMIO_RULE_CFG_1", 14, 29, 1, 0x110, 8},
    {"MMIO_RULE_CFG_2", 14, 29, 1, 0x118, 8},
    {"MMIO_RULE_CFG_3", 14, 29, 1, 0x120, 8},
    {"MMIO_RULE_CFG_4", 14, 29, 1, 0x128, 8},
    {"MMIO_RULE_CFG_5", 14, 29, 1, 0x130, 8},
    {"MMIO_RULE_CFG_6", 14, 29, 1, 0x138, 8},
    {"MMIO_RULE_CFG_7", 14, 29, 1, 0x140, 8},
    {"MMIO_RULE_CFG_8", 14, 29, 1, 0x148, 8},
    {"MMIO_RULE_CFG_9", 14, 29, 1, 0x150, 8},
    {"MMIO_RULE_CFG_10", 14, 29, 1, 0x158, 8},
    {"MMIO_RULE_CFG_11", 14, 29, 1, 0x160, 8},
    {"MMIO_RULE_CFG_12", 14, 29, 1, 0x168, 8},
    {"MMIO_RULE_CFG_13", 14, 29, 1, 0x170, 8},
};

/******************************************************************************
 *
 *   logAddressMapCPX1
 *
 *   This function logs the CPX1 Address Map
 *
 ******************************************************************************/
int logAddressMapCPX1(crashdump::CPUInfo& cpuInfo, cJSON* pJsonChild)
{
    return logAddressMapEntries(
        cpuInfo, sAddrMapEntriesCPX1,
        sizeof(sAddrMapEntriesCPX1) / sizeof(SAddrMapEntry), pJsonChild);
}

/******************************************************************************
 *
 *   logAddressMapICX1
 *
 *   This function logs the ICX1 Address Map
 *
 ******************************************************************************/
int logAddressMapICX1(crashdump::CPUInfo& cpuInfo, cJSON* pJsonChild)
{
    // TODO: feature enablement
    return 0;
}

static const SAddrMapVx sAddrMapVx[] = {
    {clx, logAddressMapCPX1}, {clx2, logAddressMapCPX1},
    {cpx, logAddressMapCPX1}, {skx, logAddressMapCPX1},
    {icx, logAddressMapICX1},
};

/******************************************************************************
 *
 *   logAddressMap
 *
 *   This function gathers the Address Map log and adds it to the debug log
 *
 ******************************************************************************/
int logAddressMap(crashdump::CPUInfo& cpuInfo, cJSON* pJsonChild)
{
    if (pJsonChild == NULL)
    {
        return 1;
    }

    for (uint32_t i = 0; i < (sizeof(sAddrMapVx) / sizeof(SAddrMapVx)); i++)
    {
        if (cpuInfo.model == sAddrMapVx[i].cpuModel)
        {
            return sAddrMapVx[i].logAddrMapVx(cpuInfo, pJsonChild);
        }
    }

    fprintf(stderr, "Cannot find version for %s\n", __FUNCTION__);
    return 1;
}
