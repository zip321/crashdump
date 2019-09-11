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
 *   fillCPUIDJson
 *
 *   This function fills in the cpu_type JSON info
 *
 ******************************************************************************/
void fillCPUIDJson(SSysInfoRawData* sSysInfoRawData, char* cSectionName,
                   cJSON* pJsonChild)
{
    cJSON* cpu;
    char jsonItemName[SI_JSON_STRING_LEN];
    char jsonItemString[SI_JSON_STRING_LEN];

    // For now, the CPU number is just the bottom nibble of the PECI client ID
    int cpuNum = sSysInfoRawData->sCpuData.clientID & 0xF;

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
                  sSysInfoRawData->sCpuData.cpuID);

    cJSON_AddStringToObject(cpu, cSectionName, jsonItemString);
}

/******************************************************************************
 *
 *   fillPECIIDJson
 *
 *   This function fills in the cpu_stepping JSON info
 *
 ******************************************************************************/
void fillPECIIDJson(SSysInfoRawData* sSysInfoRawData, char* cSectionName,
                    cJSON* pJsonChild)
{
    cJSON* cpu;
    char jsonItemName[SI_JSON_STRING_LEN];
    char jsonItemString[SI_JSON_STRING_LEN];

    // For now, the CPU number is just the bottom nibble of the PECI client ID
    int cpuNum = sSysInfoRawData->sCpuData.clientID & 0xF;

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
                  sSysInfoRawData->sCpuData.clientID);
    cJSON_AddStringToObject(cpu, cSectionName, jsonItemString);
}

/******************************************************************************
 *
 *   fillPackageIdJson
 *
 *   This function fills in the package_id JSON info
 *
 ******************************************************************************/
void fillPackageIdJson(SSysInfoRawData* sSysInfoRawData, char* cSectionName,
                       cJSON* pJsonChild)
{
    cJSON* cpu;
    char jsonItemName[SI_JSON_STRING_LEN];

    // For now, the CPU number is just the bottom nibble of the PECI client ID
    int cpuNum = sSysInfoRawData->sCpuData.clientID & 0xF;

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
}

/******************************************************************************
 *
 *   fillCoresPerCpuJson
 *
 *   This function fills in the cores_per_cpu JSON info
 *
 ******************************************************************************/
void fillCoresPerCpuJson(SSysInfoRawData* sSysInfoRawData, char* cSectionName,
                         cJSON* pJsonChild)
{
    cJSON* cpu;
    char jsonItemName[SI_JSON_STRING_LEN];
    char jsonItemString[SI_JSON_STRING_LEN];

    // For now, the CPU number is just the bottom nibble of the PECI client ID
    int cpuNum = sSysInfoRawData->sCpuData.clientID & 0xF;

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
                  sSysInfoRawData->sCpuData.u32CoresPerCpu);
    cJSON_AddStringToObject(cpu, cSectionName, jsonItemString);
}

/******************************************************************************
 *
 *   fillUCodeVersionJson
 *
 *   This function fills in the ucode_patch_ver JSON info
 *
 ******************************************************************************/
void fillUCodeVersionJson(SSysInfoRawData* sSysInfoRawData, char* cSectionName,
                          cJSON* pJsonChild)
{
    cJSON* cpu;
    char jsonItemName[SI_JSON_STRING_LEN];
    char jsonItemString[SI_JSON_STRING_LEN];

    // For now, the CPU number is just the bottom nibble of the PECI client ID
    int cpuNum = sSysInfoRawData->sCpuData.clientID & 0xF;

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
                  sSysInfoRawData->sCpuData.u32UCodeVer);
    cJSON_AddStringToObject(cpu, cSectionName, jsonItemString);
}

/******************************************************************************
 *
 *   fillVCodeVersionJson
 *
 *   This function fills in the vcode_patch_ver JSON info
 *
 ******************************************************************************/
void fillVCodeVersionJson(SSysInfoRawData* sSysInfoRawData, char* cSectionName,
                          cJSON* pJsonChild)
{
    cJSON* cpu;
    char jsonItemName[SI_JSON_STRING_LEN];
    char jsonItemString[SI_JSON_STRING_LEN];

    // Only add the VCode version if available
    if (!sSysInfoRawData->sCpuData.u32VCodeVer)
    {
        return;
    }

    // For now, the CPU number is just the bottom nibble of the PECI client ID
    int cpuNum = sSysInfoRawData->sCpuData.clientID & 0xF;

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
                  sSysInfoRawData->sCpuData.u32VCodeVer);
    cJSON_AddStringToObject(cpu, cSectionName, jsonItemString);
}

/******************************************************************************
 *
 *   fillBmcVersionJson
 *
 *   This function fills in the bmc_fw_ver JSON info
 *
 ******************************************************************************/
void fillBmcVersionJson(SSysInfoRawData* sSysInfoRawData, char* cSectionName,
                        cJSON* pJsonChild)
{
    // Only include system data if available
    if (!sSysInfoRawData->systemData)
    {
        return;
    }

    // Fill in BMC Version string
    cJSON_AddStringToObject(pJsonChild, cSectionName,
                            sSysInfoRawData->bmcVersion);
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
void fillMeVersionJson(SSysInfoRawData* sSysInfoRawData, char* cSectionName,
                       cJSON* pJsonChild)
{
    // Only include system data if available
    if (!sSysInfoRawData->systemData)
    {
        return;
    }

    // Fill in as N/A for now
    cJSON_AddStringToObject(pJsonChild, cSectionName, MD_NA);
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
void fillBiosIdJson(SSysInfoRawData* sSysInfoRawData, char* cSectionName,
                    cJSON* pJsonChild)
{
    // Only include system data if available
    if (!sSysInfoRawData->systemData)
    {
        return;
    }

    // Fill in as N/A for now
    cJSON_AddStringToObject(pJsonChild, cSectionName, MD_NA);
    // char jsonItemString[SI_JSON_STRING_LEN];

    // cd_snprintf_s(jsonItemString, SI_JSON_STRING_LEN, "%s",
    // sSysInfoRawData->u8BiosId); cJSON_AddStringToObject(pJsonChild,
    // cSectionName, jsonItemString);
}

/******************************************************************************
 *
 *   fillMcaErrSrcLogJson
 *
 *   This function fills in the mca_err_src_log JSON info
 *
 ******************************************************************************/
void fillMcaErrSrcLogJson(SSysInfoRawData* sSysInfoRawData, char* cSectionName,
                          cJSON* pJsonChild)
{
    cJSON* cpu;
    char jsonItemName[SI_JSON_STRING_LEN];
    char jsonItemString[SI_JSON_STRING_LEN];

    // For now, the CPU number is just the bottom nibble of the PECI client ID
    int cpuNum = sSysInfoRawData->sCpuData.clientID & 0xF;

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
                  sSysInfoRawData->sCpuData.u32McaErrSrcLog);
    cJSON_AddStringToObject(cpu, cSectionName, jsonItemString);
}

/******************************************************************************
 *
 *   fillCrashdumpVersionJson
 *
 *   This function fills in the crashdump_ver JSON info
 *
 ******************************************************************************/
void fillCrashdumpVersionJson(SSysInfoRawData* sSysInfoRawData,
                              char* cSectionName, cJSON* pJsonChild)
{
    // Only include system data if available
    if (sSysInfoRawData->systemData)
    {
        cJSON_AddStringToObject(pJsonChild, cSectionName, SI_CRASHDUMP_VER);
    }
}

/******************************************************************************
 *
 *   fillPpinJson
 *
 *   This function fills in the PPIN JSON info
 *
 ******************************************************************************/
void fillPpinJson(SSysInfoRawData* sSysInfoRawData, char* cSectionName,
                  cJSON* pJsonChild, uint8_t cc)
{
    cJSON* cpu;
    char jsonItemName[SI_JSON_STRING_LEN];
    char jsonItemString[SI_JSON_STRING_LEN];

    // For now, the CPU number is just the bottom nibble of the PECI client ID
    int cpuNum = sSysInfoRawData->sCpuData.clientID & 0xF;

    // Add the CPU number object if it doesn't already exist
    cd_snprintf_s(jsonItemName, SI_JSON_STRING_LEN, SI_JSON_SOCKET_NAME,
                  cpuNum);
    if ((cpu = cJSON_GetObjectItemCaseSensitive(pJsonChild, jsonItemName)) ==
        NULL)
    {
        cJSON_AddItemToObject(pJsonChild, jsonItemName,
                              cpu = cJSON_CreateObject());
    }

    if (PECI_CC_UA(cc))
    {
        cd_snprintf_s(jsonItemString, SI_JSON_STRING_LEN, "UA:0x%x", cc);
    }
    else
    {
        cd_snprintf_s(jsonItemString, SI_JSON_STRING_LEN, "0x%" PRIx64 "",
                      sSysInfoRawData->sCpuData.ppin);
    }
    if (sSysInfoRawData->sCpuData.isPpinNA == false)
        cJSON_AddStringToObject(cpu, cSectionName, jsonItemString);
    else
        cJSON_AddStringToObject(cpu, cSectionName, MD_NA);
}

/******************************************************************************
 *
 *   getPpinDataICX1
 *
 *   This function gets PPIN info
 *
 ******************************************************************************/
void getPpinDataICX1(crashdump::CPUInfo& cpuInfo,
                     SSysInfoRawData* sSysInfoRawData, cJSON* pJsonChild)
{
    uint32_t ppinUpper;
    uint32_t ppinLower;
    uint8_t cc = 0;

    peci_RdPkgConfig(cpuInfo.clientAddr, SI_PECI_PPIN_IDX, SI_PECI_PPIN_LOWER,
                     sizeof(uint32_t), (uint8_t*)&ppinLower, &cc);
    sSysInfoRawData->sCpuData.isPpinNA = false;
    if (PECI_CC_UA(cc))
    {
        fillPpinJson(sSysInfoRawData, "ppin", pJsonChild, cc);
        return;
    }

    peci_RdPkgConfig(cpuInfo.clientAddr, SI_PECI_PPIN_IDX, SI_PECI_PPIN_UPPER,
                     sizeof(uint32_t), (uint8_t*)&ppinUpper, &cc);

    if (PECI_CC_UA(cc))
    {
        fillPpinJson(sSysInfoRawData, "ppin", pJsonChild, cc);
        return;
    }
    sSysInfoRawData->sCpuData.ppin = ppinUpper;
    sSysInfoRawData->sCpuData.ppin <<= 32;
    sSysInfoRawData->sCpuData.ppin |= ppinLower;

    fillPpinJson(sSysInfoRawData, "ppin", pJsonChild, cc);
}

/******************************************************************************
 *
 *   getPpinDataCPX1
 *
 *   This function gets PPIN info
 *
 ******************************************************************************/
void getPpinDataCPX1(crashdump::CPUInfo& cpuInfo,
                     SSysInfoRawData* sSysInfoRawData, cJSON* pJsonChild)
{
    uint8_t cc = 0;
    sSysInfoRawData->sCpuData.isPpinNA = true;
    fillPpinJson(sSysInfoRawData, "ppin", pJsonChild, cc);
}

static const SPpinVx sSPpinVx[] = {
    {clx, getPpinDataCPX1}, {clx2, getPpinDataCPX1}, {cpx, getPpinDataCPX1},
    {skx, getPpinDataCPX1}, {icx, getPpinDataICX1},
};

/******************************************************************************
 *
 *   getCPUData
 *
 *   This function gets the CPU-specific data for this CPU
 *
 ******************************************************************************/
static int getCPUData(crashdump::CPUInfo& cpuInfo,
                      SSysInfoRawData* sSysInfoRawData, cJSON* pJsonChild)
{
    int ret = 0;
    uint32_t u32PeciData;
    uint8_t cc = 0;

    if (peci_RdPkgConfig(cpuInfo.clientAddr, PECI_MBX_INDEX_CPU_ID,
                         PECI_PKG_ID_CPU_ID, sizeof(uint32_t),
                         (uint8_t*)&u32PeciData, &cc) != PECI_CC_SUCCESS)
    {
        ret = 1;
    }
    sSysInfoRawData->sCpuData.clientID = cpuInfo.clientAddr;
    sSysInfoRawData->sCpuData.cpuID = u32PeciData;

    // Get the core count
    sSysInfoRawData->sCpuData.u32CoresPerCpu =
        __builtin_popcount(cpuInfo.coreMask);

    // Get the UCode Version
    if (peci_RdPkgConfig(cpuInfo.clientAddr, PECI_MBX_INDEX_CPU_ID,
                         PECI_PKG_ID_MICROCODE_REV, sizeof(uint32_t),
                         (uint8_t*)&u32PeciData, &cc) != PECI_CC_SUCCESS)
    {
        ret = 1;
    }
    sSysInfoRawData->sCpuData.u32UCodeVer = u32PeciData;

    // Get the VCode Version if available
    if (peci_RdPkgConfig(cpuInfo.clientAddr, MBX_INDEX_VCU, VCU_VERSION,
                         sizeof(uint32_t), (uint8_t*)&u32PeciData,
                         &cc) == PECI_CC_SUCCESS)
    {
        sSysInfoRawData->sCpuData.u32VCodeVer = u32PeciData;
    }

    // Get the MCA_ERR_SRC_LOG
    if (peci_RdPkgConfig(cpuInfo.clientAddr, PECI_MBX_INDEX_CPU_ID,
                         PECI_PKG_ID_MACHINE_CHECK_STATUS, sizeof(uint32_t),
                         (uint8_t*)&u32PeciData, &cc) != PECI_CC_SUCCESS)
    {
        ret = 1;
    }
    sSysInfoRawData->sCpuData.u32McaErrSrcLog = u32PeciData;

    // Get Ppin info
    for (int i = 0; i < (sizeof(sSPpinVx) / sizeof(SPpinVx)); i++)
    {
        if (cpuInfo.model == sSPpinVx[i].cpuModel)
        {
            sSPpinVx[i].getPpinVx(cpuInfo, sSysInfoRawData, pJsonChild);
        }
    }

    return ret;
}

/******************************************************************************
 *
 *   getBMCVersion
 *
 *   This function gets the BMC Version
 *
 ******************************************************************************/
static int getBMCVersion(SSysInfoRawData* sSysInfoRawData)
{
    return crashdump::getBMCVersionDBus(sSysInfoRawData->bmcVersion,
                                        sizeof(sSysInfoRawData->bmcVersion));
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

/******************************************************************************
 *
 *   getSystemInfo
 *
 *   This function gets the System Info
 *
 ******************************************************************************/
static int getSystemInfo(SSysInfoRawData* sSysInfoRawData)
{
    int ret = 0;

    // Get the BIOS ID
    if (getBIOSID(sSysInfoRawData->u8BiosId) != 0)
    {
        ret = 1;
    }

    // Get the ME Device ID
    if (getMeDeviceId() != 0)
    {
        ret = 1;
    }

    // Get the BMC Version
    if (getBMCVersion(sSysInfoRawData) != 0)
    {
        ret = 1;
    }

    // Mark that system info is available
    sSysInfoRawData->systemData = true;

    return ret;
}

static SSysInfoSection sSysInfoTable[] = {
    {"peci_id", fillPECIIDJson},
    {"cpuid", fillCPUIDJson},
    {"package_id", fillPackageIdJson},
    {"cores_per_cpu", fillCoresPerCpuJson},
    {"ucode_patch_ver", fillUCodeVersionJson},
    {"vcode_ver", fillVCodeVersionJson},
    {"bmc_fw_ver", fillBmcVersionJson},
    {"me_fw_ver", fillMeVersionJson},
    {"bios_id", fillBiosIdJson},
    {"mca_err_src_log", fillMcaErrSrcLogJson},
    {"crashdump_ver", fillCrashdumpVersionJson},
};

/******************************************************************************
 *
 *   sysInfoJson
 *
 *   This function formats the system information into a JSON object
 *
 ******************************************************************************/
static void sysInfoJson(SSysInfoRawData* sSysInfoRawData, cJSON* pJsonChild)
{
    uint32_t i;

    for (i = 0; i < (sizeof(sSysInfoTable) / sizeof(SSysInfoSection)); i++)
    {
        sSysInfoTable[i].FillSysInfoJson(
            sSysInfoRawData, sSysInfoTable[i].sectionName, pJsonChild);
    }
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

    SSysInfoRawData sSysInfoRawData = {0};
    int ret = 0;

    ret = getCPUData(cpuInfo, &sSysInfoRawData, pJsonChild);

    // Only log the system info once
    if (cpuInfo.clientAddr == MIN_CLIENT_ADDR)
    {
        ret = getSystemInfo(&sSysInfoRawData);
    }

    // Log the System Info
    sysInfoJson(&sSysInfoRawData, pJsonChild);

    return ret;
}
