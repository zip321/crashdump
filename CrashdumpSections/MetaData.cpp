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

#include "MetaData.hpp"

extern "C" {
#include <cjson/cJSON.h>
#include <stdio.h>
}
#include "crashdump.hpp"
#include "utils.hpp"

/******************************************************************************
 *
 *   fillCPUID
 *
 *   This function reads and fills in the cpu_type JSON info
 *
 ******************************************************************************/
int fillCPUID(crashdump::CPUInfo& cpuInfo, char* cSectionName,
              cJSON* pJsonChild)
{
    cJSON* cpu = NULL;
    uint32_t u32PeciData = 0;
    uint8_t cc = 0;
    char jsonItemName[SI_JSON_STRING_LEN] = {0};
    char jsonItemString[SI_JSON_STRING_LEN] = {0};

    // For now, the CPU number is just the bottom nibble of the PECI client ID
    int cpuNum = cpuInfo.clientAddr & 0xF;

    // Add the CPU number object if it doesn't already exist
    cd_snprintf_s(jsonItemName, SI_JSON_STRING_LEN, SI_JSON_SOCKET_NAME,
                  cpuNum);
    if ((cpu = cJSON_GetObjectItemCaseSensitive(pJsonChild, jsonItemName)) ==
        NULL)
    {
        cJSON_AddItemToObject(pJsonChild, jsonItemName,
                              cpu = cJSON_CreateObject());
    }
    if (peci_RdPkgConfig(cpuInfo.clientAddr, PECI_MBX_INDEX_CPU_ID,
                         PECI_PKG_ID_CPU_ID, sizeof(uint32_t),
                         (uint8_t*)&u32PeciData, &cc) != PECI_CC_SUCCESS)
    {
        cd_snprintf_s(jsonItemString, SI_JSON_STRING_LEN, MD_UA, cc);
        cJSON_AddStringToObject(cpu, cSectionName, jsonItemString);
        return 1;
    }
    cd_snprintf_s(jsonItemString, SI_JSON_STRING_LEN, "0x%x", u32PeciData);
    cJSON_AddStringToObject(cpu, cSectionName, jsonItemString);
    return 0;
}

/******************************************************************************
 *
 *   fillPECI
 *
 *   This function reads and fills in the cpu_stepping JSON info
 *
 ******************************************************************************/
int fillPECI(crashdump::CPUInfo& cpuInfo, char* cSectionName, cJSON* pJsonChild)
{
    cJSON* cpu = NULL;
    char jsonItemName[SI_JSON_STRING_LEN] = {0};
    char jsonItemString[SI_JSON_STRING_LEN] = {0};

    // For now, the CPU number is just the bottom nibble of the PECI client ID
    int cpuNum = cpuInfo.clientAddr & 0xF;

    // Add the CPU number object if it doesn't already exist
    cd_snprintf_s(jsonItemName, SI_JSON_STRING_LEN, SI_JSON_SOCKET_NAME,
                  cpuNum);
    if ((cpu = cJSON_GetObjectItemCaseSensitive(pJsonChild, jsonItemName)) ==
        NULL)
    {
        cJSON_AddItemToObject(pJsonChild, jsonItemName,
                              cpu = cJSON_CreateObject());
    }

    cd_snprintf_s(jsonItemString, SI_JSON_STRING_LEN, "0x%x",
                  cpuInfo.clientAddr);
    cJSON_AddStringToObject(cpu, cSectionName, jsonItemString);
    return 0;
}

/******************************************************************************
 *
 *   fillPackageId
 *
 *   This function reads and fills in the package_id JSON info
 *
 ******************************************************************************/
int fillPackageId(crashdump::CPUInfo& cpuInfo, char* cSectionName,
                  cJSON* pJsonChild)
{
    cJSON* cpu = NULL;
    char jsonItemName[SI_JSON_STRING_LEN] = {0};

    // For now, the CPU number is just the bottom nibble of the PECI client ID
    int cpuNum = cpuInfo.clientAddr & 0xF;

    // Add the socket number object if it doesn't already exist
    cd_snprintf_s(jsonItemName, SI_JSON_STRING_LEN, SI_JSON_SOCKET_NAME,
                  cpuNum);
    if ((cpu = cJSON_GetObjectItemCaseSensitive(pJsonChild, jsonItemName)) ==
        NULL)
    {
        cJSON_AddItemToObject(pJsonChild, jsonItemName,
                              cpu = cJSON_CreateObject());
    }

    // Fill in package_id as "N/A" for now
    cJSON_AddStringToObject(cpu, cSectionName, MD_NA);
    return 0;
}

/******************************************************************************
 *
 *   fillCoresPerCpu
 *
 *   This function reads and fills in the cores_per_cpu JSON info
 *
 ******************************************************************************/
int fillCoresPerCpu(crashdump::CPUInfo& cpuInfo, char* cSectionName,
                    cJSON* pJsonChild)
{
    cJSON* cpu = NULL;
    char jsonItemName[SI_JSON_STRING_LEN] = {0};
    char jsonItemString[SI_JSON_STRING_LEN] = {0};

    // For now, the CPU number is just the bottom nibble of the PECI client ID
    int cpuNum = cpuInfo.clientAddr & 0xF;

    // Add the CPU number object if it doesn't already exist
    cd_snprintf_s(jsonItemName, SI_JSON_STRING_LEN, SI_JSON_SOCKET_NAME,
                  cpuNum);
    if ((cpu = cJSON_GetObjectItemCaseSensitive(pJsonChild, jsonItemName)) ==
        NULL)
    {
        cJSON_AddItemToObject(pJsonChild, jsonItemName,
                              cpu = cJSON_CreateObject());
    }

    cd_snprintf_s(jsonItemString, SI_JSON_STRING_LEN, "0x%x",
                  __builtin_popcount(cpuInfo.coreMask));
    cJSON_AddStringToObject(cpu, cSectionName, jsonItemString);
    return 0;
}

/******************************************************************************
 *
 *   fillUCodeVersion
 *
 *   This function reads and fills in the ucode_patch_ver JSON info
 *
 ******************************************************************************/
int fillUCodeVersion(crashdump::CPUInfo& cpuInfo, char* cSectionName,
                     cJSON* pJsonChild)
{
    cJSON* cpu = NULL;
    uint32_t u32PeciData = 0;
    uint8_t cc = 0;
    char jsonItemName[SI_JSON_STRING_LEN] = {0};
    char jsonItemString[SI_JSON_STRING_LEN] = {0};

    // For now, the CPU number is just the bottom nibble of the PECI client ID
    int cpuNum = cpuInfo.clientAddr & 0xF;

    // Add the CPU number object if it doesn't already exist
    cd_snprintf_s(jsonItemName, SI_JSON_STRING_LEN, SI_JSON_SOCKET_NAME,
                  cpuNum);
    if ((cpu = cJSON_GetObjectItemCaseSensitive(pJsonChild, jsonItemName)) ==
        NULL)
    {
        cJSON_AddItemToObject(pJsonChild, jsonItemName,
                              cpu = cJSON_CreateObject());
    }
    // Get the UCode Version
    if (peci_RdPkgConfig(cpuInfo.clientAddr, PECI_MBX_INDEX_CPU_ID,
                         PECI_PKG_ID_MICROCODE_REV, sizeof(uint32_t),
                         (uint8_t*)&u32PeciData, &cc) != PECI_CC_SUCCESS)
    {
        cd_snprintf_s(jsonItemString, SI_JSON_STRING_LEN, MD_UA, cc);
        cJSON_AddStringToObject(cpu, cSectionName, jsonItemString);
        return 1;
    }
    cd_snprintf_s(jsonItemString, SI_JSON_STRING_LEN, "0x%x", u32PeciData);
    cJSON_AddStringToObject(cpu, cSectionName, jsonItemString);
    return 0;
}

/******************************************************************************
 *
 *   fillVCodeVersion
 *
 *   This function reads and fills in the vcode_patch_ver JSON info
 *
 ******************************************************************************/
int fillVCodeVersion(crashdump::CPUInfo& cpuInfo, char* cSectionName,
                     cJSON* pJsonChild)
{
    cJSON* cpu = NULL;
    uint32_t u32PeciData = 0;
    uint8_t cc = 0;
    char jsonItemName[SI_JSON_STRING_LEN] = {0};
    char jsonItemString[SI_JSON_STRING_LEN] = {0};

    // For now, the CPU number is just the bottom nibble of the PECI client ID
    int cpuNum = cpuInfo.clientAddr & 0xF;

    // Add the CPU number object if it doesn't already exist
    cd_snprintf_s(jsonItemName, SI_JSON_STRING_LEN, SI_JSON_SOCKET_NAME,
                  cpuNum);
    if ((cpu = cJSON_GetObjectItemCaseSensitive(pJsonChild, jsonItemName)) ==
        NULL)
    {
        cJSON_AddItemToObject(pJsonChild, jsonItemName,
                              cpu = cJSON_CreateObject());
    }
    // Get the VCode Version if available
    if (peci_RdPkgConfig(cpuInfo.clientAddr, MBX_INDEX_VCU, VCU_VERSION,
                         sizeof(uint32_t), (uint8_t*)&u32PeciData,
                         &cc) != PECI_CC_SUCCESS)
    {
        cd_snprintf_s(jsonItemString, SI_JSON_STRING_LEN, MD_UA, cc);
        cJSON_AddStringToObject(cpu, cSectionName, jsonItemString);
        return 1;
    }
    cd_snprintf_s(jsonItemString, SI_JSON_STRING_LEN, "0x%x", u32PeciData);
    cJSON_AddStringToObject(cpu, cSectionName, jsonItemString);
    return 0;
}

/******************************************************************************
 *
 *   fillBmcVersionJson
 *
 *   This function fills in the bmc_fw_ver JSON info
 *
 ******************************************************************************/
int fillBmcVersion(char* cSectionName, cJSON* pJsonChild)
{
    char bmcVersion[SI_BMC_VER_LEN] = {0};
    crashdump::getBMCVersionDBus(bmcVersion, sizeof(bmcVersion));
    // Fill in BMC Version string
    cJSON_AddStringToObject(pJsonChild, cSectionName, bmcVersion);
    return 0;
    // char jsonItemString[SI_JSON_STRING_LEN];

    // cd_snprintf_s(jsonItemString, SI_JSON_STRING_LEN,
    // "%d.%02x.%02x%02x%02x%02x",
    // sSysInfoRawData->bmcFwDeviceId.u8FirmwareMajorVersion & 0x7f,
    // sSysInfoRawData->bmcFwDeviceId.u8FirmwareMinorVersion,
    // sSysInfoRawData->bmcFwDeviceId.u8B3AuxFwRevInfo,
    // sSysInfoRawData->bmcFwDeviceId.u8B2AuxFwRevInfo,
    // sSysInfoRawData->bmcFwDeviceId.u8B1AuxFwRevInfo,
    // sSysInfoRawData->bmcFwDeviceId.u8B0AuxFwRevInfo);
    // cJSON_AddStringToObject(pJsonChild, cSectionName, jsonItemString);
}

/******************************************************************************
 *
 *   fillMeVersionJson
 *
 *   This function fills in the me_fw_ver JSON info
 *
 ******************************************************************************/
int fillMeVersion(char* cSectionName, cJSON* pJsonChild)
{
    // Fill in as N/A for now
    cJSON_AddStringToObject(pJsonChild, cSectionName, MD_NA);
    return 0;
    // char jsonItemString[SI_JSON_STRING_LEN];

    // cd_snprintf_s(jsonItemString, SI_JSON_STRING_LEN,
    // "%02d.%02x.%02x.%02x%x",
    // sSysInfoRawData->meFwDeviceId.u8FirmwareMajorVersion & 0x7f,
    // sSysInfoRawData->meFwDeviceId.u8B2AuxFwRevInfo & 0x0f,
    // sSysInfoRawData->meFwDeviceId.u8FirmwareMinorVersion,
    // sSysInfoRawData->meFwDeviceId.u8B1AuxFwRevInfo,
    // sSysInfoRawData->meFwDeviceId.u8B2AuxFwRevInfo >> 4);
    // cJSON_AddStringToObject(pJsonChild, cSectionName, jsonItemString);
}

/******************************************************************************
 *
 *   fillBiosIdJson
 *
 *   This function fills in the bios_id JSON info
 *
 ******************************************************************************/
int fillBiosId(char* cSectionName, cJSON* pJsonChild)
{
    // Fill in as N/A for now
    cJSON_AddStringToObject(pJsonChild, cSectionName, MD_NA);
    return 0;
    // char jsonItemString[SI_JSON_STRING_LEN];

    // cd_snprintf_s(jsonItemString, SI_JSON_STRING_LEN, "%s",
    // sSysInfoRawData->u8BiosId); cJSON_AddStringToObject(pJsonChild,
    // cSectionName, jsonItemString);
}

/******************************************************************************
 *
 *   fillMcaErrSrcLog
 *
 *   This function fills in the mca_err_src_log JSON info
 *
 ******************************************************************************/
int fillMcaErrSrcLog(crashdump::CPUInfo& cpuInfo, char* cSectionName,
                     cJSON* pJsonChild)
{
    cJSON* cpu = NULL;
    uint32_t u32PeciData = 0;
    uint8_t cc = 0;
    char jsonItemName[SI_JSON_STRING_LEN] = {0};
    char jsonItemString[SI_JSON_STRING_LEN] = {0};

    // For now, the CPU number is just the bottom nibble of the PECI client ID
    int cpuNum = cpuInfo.clientAddr & 0xF;

    // Add the CPU number object if it doesn't already exist
    cd_snprintf_s(jsonItemName, SI_JSON_STRING_LEN, SI_JSON_SOCKET_NAME,
                  cpuNum);
    if ((cpu = cJSON_GetObjectItemCaseSensitive(pJsonChild, jsonItemName)) ==
        NULL)
    {
        cJSON_AddItemToObject(pJsonChild, jsonItemName,
                              cpu = cJSON_CreateObject());
    }
    if (peci_RdPkgConfig(cpuInfo.clientAddr, PECI_MBX_INDEX_CPU_ID,
                         PECI_PKG_ID_MACHINE_CHECK_STATUS, sizeof(uint32_t),
                         (uint8_t*)&u32PeciData, &cc) != PECI_CC_SUCCESS)
    {
        cd_snprintf_s(jsonItemString, SI_JSON_STRING_LEN, MD_UA, cc);
        cJSON_AddStringToObject(cpu, cSectionName, jsonItemString);
        return 1;
    }
    cd_snprintf_s(jsonItemString, SI_JSON_STRING_LEN, "0x%x", u32PeciData);
    cJSON_AddStringToObject(cpu, cSectionName, jsonItemString);
    return 0;
}

/******************************************************************************
 *
 *   fillCrashdumpVersion
 *
 *   This function fills in the crashdump_ver JSON info
 *
 ******************************************************************************/
int fillCrashdumpVersion(char* cSectionName, cJSON* pJsonChild)
{
    cJSON_AddStringToObject(pJsonChild, cSectionName, SI_CRASHDUMP_VER);
    return 0;
}

/******************************************************************************
 *
 *   fillPpinJson
 *
 *   This function fills in the PPIN JSON info
 *
 ******************************************************************************/
int getPpinDataICX1(crashdump::CPUInfo& cpuInfo, char* cSectionName,
                    cJSON* pJsonChild)
{
    cJSON* cpu = NULL;
    uint32_t ppinUpper = 0;
    uint32_t ppinLower = 0;
    uint8_t cc = 0;
    uint64_t ppin = 0;
    char jsonItemName[SI_JSON_STRING_LEN] = {0};
    char jsonItemString[SI_JSON_STRING_LEN] = {0};

    // For now, the CPU number is just the bottom nibble of the PECI client ID
    int cpuNum = cpuInfo.clientAddr & 0xF;

    // Add the CPU number object if it doesn't already exist
    cd_snprintf_s(jsonItemName, SI_JSON_STRING_LEN, SI_JSON_SOCKET_NAME,
                  cpuNum);
    if ((cpu = cJSON_GetObjectItemCaseSensitive(pJsonChild, jsonItemName)) ==
        NULL)
    {
        cJSON_AddItemToObject(pJsonChild, jsonItemName,
                              cpu = cJSON_CreateObject());
    }

    peci_RdPkgConfig(cpuInfo.clientAddr, SI_PECI_PPIN_IDX, SI_PECI_PPIN_LOWER,
                     sizeof(uint32_t), (uint8_t*)&ppinLower, &cc);
    if (PECI_CC_UA(cc))
    {
        cd_snprintf_s(jsonItemString, SI_JSON_STRING_LEN, "UA:0x%x", cc);
        cJSON_AddStringToObject(cpu, cSectionName, jsonItemString);
        return 1;
    }

    peci_RdPkgConfig(cpuInfo.clientAddr, SI_PECI_PPIN_IDX, SI_PECI_PPIN_UPPER,
                     sizeof(uint32_t), (uint8_t*)&ppinUpper, &cc);

    if (PECI_CC_UA(cc))
    {
        cd_snprintf_s(jsonItemString, SI_JSON_STRING_LEN, "UA:0x%x", cc);
        cJSON_AddStringToObject(cpu, cSectionName, jsonItemString);
        return 1;
    }
    ppin = ppinUpper;
    ppin <<= 32;
    ppin |= ppinLower;

    cd_snprintf_s(jsonItemString, SI_JSON_STRING_LEN, "0x%" PRIx64 "", ppin);
    cJSON_AddStringToObject(cpu, cSectionName, jsonItemString);
    return 0;
}

/******************************************************************************
 *
 *   getPpinDataCPX1
 *
 *   This function gets PPIN info
 *
 ******************************************************************************/
int getPpinDataCPX1(crashdump::CPUInfo& cpuInfo, char* cSectionName,
                    cJSON* pJsonChild)
{
    cJSON* cpu = NULL;
    char jsonItemName[SI_JSON_STRING_LEN] = {0};
    char jsonItemString[SI_JSON_STRING_LEN] = {0};
    // For now, the CPU number is just the bottom nibble of the PECI client ID
    int cpuNum = cpuInfo.clientAddr & 0xF;

    // Add the CPU number object if it doesn't already exist
    cd_snprintf_s(jsonItemName, SI_JSON_STRING_LEN, SI_JSON_SOCKET_NAME,
                  cpuNum);
    if ((cpu = cJSON_GetObjectItemCaseSensitive(pJsonChild, jsonItemName)) ==
        NULL)
    {
        cJSON_AddItemToObject(pJsonChild, jsonItemName,
                              cpu = cJSON_CreateObject());
    }
    cJSON_AddStringToObject(cpu, cSectionName, MD_NA);
    return 0;
}

static const SPpinVx sSPpinVx[] = {
    {crashdump::cpu::clx, getPpinDataCPX1},
    {crashdump::cpu::cpx, getPpinDataCPX1},
    {crashdump::cpu::skx, getPpinDataCPX1},
    {crashdump::cpu::icx, getPpinDataICX1},
    {crashdump::cpu::icx2, getPpinDataICX1},
};

/******************************************************************************
 *
 *   fillPpin
 *
 *   This function gets PPIN info
 *
 ******************************************************************************/
int fillPpin(crashdump::CPUInfo& cpuInfo, char* cSectionName, cJSON* pJsonChild)
{
    int r = 0;
    int ret = 0;
    for (int i = 0; i < (sizeof(sSPpinVx) / sizeof(SPpinVx)); i++)
    {
        if (cpuInfo.model == sSPpinVx[i].cpuModel)
        {
            r = sSPpinVx[i].getPpinVx(cpuInfo, cSectionName, pJsonChild);
            if (r == 1)
                ret = 1;
        }
    }
    return ret;
}

/******************************************************************************
 *
 *   getBIOSID
 *
 *   This function gets the BIOS ID
 *
 ******************************************************************************/
static int getBIOSID(uint8_t* biosID)
{
    return 0;
}

/******************************************************************************
 *
 *   getMeDeviceId
 *
 *   This function gets the Device ID from the Intel ME
 *
 ******************************************************************************/
static int getMeDeviceId(void)
{
    return 0;
}

static SSysInfoSection sSysInfoTable[] = {
    {"peci_id", fillPECI},
    {"cpuid", fillCPUID},
    {"package_id", fillPackageId},
    {"cores_per_cpu", fillCoresPerCpu},
    {"ucode_patch_ver", fillUCodeVersion},
    {"vcode_ver", fillVCodeVersion},
    {"mca_err_src_log", fillMcaErrSrcLog},
    {"ppin", fillPpin},
};

static SSysInfoCommonSection sSysInfoCommonTable[] = {
    {"me_fw_ver", fillMeVersion},
    {"bmc_fw_ver", fillBmcVersion},
    {"bios_id", fillBiosId},
    {"crashdump_ver", fillCrashdumpVersion},
};

/******************************************************************************
 *
 *   sysInfoJson
 *
 *   This function formats the system information into a JSON object
 *
 ******************************************************************************/
static int sysInfoJson(crashdump::CPUInfo& cpuInfo, cJSON* pJsonChild)
{
    uint32_t i = 0;
    int r = 0;
    int ret = 0;

    for (i = 0; i < (sizeof(sSysInfoTable) / sizeof(SSysInfoSection)); i++)
    {
        r = sSysInfoTable[i].FillSysInfoJson(
            cpuInfo, sSysInfoTable[i].sectionName, pJsonChild);
        if (r == 1)
            ret = 1;
    }
    return ret;
}

/******************************************************************************
 *
 *   sysInfoCommonJson
 *
 *   This function formats the system information into a JSON object
 *
 ******************************************************************************/
static int sysInfoCommonJson(cJSON* pJsonChild)
{
    uint32_t i = 0;
    int r = 0;
    int ret = 0;

    for (i = 0;
         i < (sizeof(sSysInfoCommonTable) / sizeof(SSysInfoCommonSection)); i++)
    {
        r = sSysInfoCommonTable[i].FillSysInfoJson(
            sSysInfoCommonTable[i].sectionName, pJsonChild);
        if (r == 1)
            ret = 1;
    }
    return ret;
}

/******************************************************************************
 *
 *   logSysInfo
 *
 *   This function gathers various bits of system information and compiles them
 *   into a single group
 *
 ******************************************************************************/
int logSysInfo(crashdump::CPUInfo& cpuInfo, cJSON* pJsonChild)
{
    if (pJsonChild == NULL)
    {
        return 1;
    }
    // Log the System Info
    return sysInfoJson(cpuInfo, pJsonChild);
}

/******************************************************************************
 *
 *   logSysInfoCommon
 *
 *   This function gathers various bits of system information and compiles them
 *   into a single group
 *
 ******************************************************************************/
int logSysInfoCommon(cJSON* pJsonChild)
{
    if (pJsonChild == NULL)
    {
        return 1;
    }
    // Log the System Info
    return sysInfoCommonJson(pJsonChild);
}
