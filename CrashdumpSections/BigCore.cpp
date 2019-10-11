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

#include "BigCore.hpp"

extern "C" {
#include <cjson/cJSON.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
}

#include "crashdump.hpp"
#include "utils.hpp"

#ifdef COMPILE_UNIT_TESTS
#define static
#endif

static SCrashdumpRegCPX1 sCrashdumpUncoreRegs[CD_REGS_UNCORE] = {
    {"IERRLOGGINGREG", 1, CORE_SCOPE},
    {"MCERRLOGGINGREG", 1, CORE_SCOPE},
    {"THERM_INTERRUPT", 1, CORE_SCOPE},
    {"THERM_STATUS", 1, CORE_SCOPE},
    {"IA32_CLOCK_MODULATION", 1, CORE_SCOPE}};

static SCrashdumpRegCPX1* sCrashdumpCoreRegsCPX1[CD_NUM_GROUPS_CORE] = {
    (SCrashdumpRegCPX1[]){// Group 1
                          {"IA32_X2APIC_CUR_COUNT", 1, THREAD_SCOPE}},
    (SCrashdumpRegCPX1[]){// Group 2
                          {"EXT_XAPIC_SVR", 1, THREAD_SCOPE}},
    (SCrashdumpRegCPX1[]){// Group 3
                          {"IA32_X2APIC_LVT_CMCI", 1, THREAD_SCOPE},
                          {"IA32_X2APIC_LVT_TIMER", 1, THREAD_SCOPE},
                          {"IA32_X2APIC_LVT_THERMAL", 1, THREAD_SCOPE},
                          {"IA32_X2APIC_LVT_PMI", 1, THREAD_SCOPE},
                          {"IA32_X2APIC_LVT_LINT0", 1, THREAD_SCOPE},
                          {"IA32_X2APIC_LVT_LINT1", 1, THREAD_SCOPE},
                          {"IA32_X2APIC_LVT_ERROR", 1, THREAD_SCOPE},
                          {"IA32_X2APIC_ICR", 2, THREAD_SCOPE},
                          {"IA32_X2APIC_INIT_COUNT", 1, THREAD_SCOPE},
                          {"IA32_X2APIC_DIV_CONF", 1, THREAD_SCOPE}},
    (SCrashdumpRegCPX1[]){// Group 4
                          {"EXT_XAPIC_LDR", 1, THREAD_SCOPE}},
    (SCrashdumpRegCPX1[]){// Group 5
                          {"IA32_X2APIC_ISR0", 1, THREAD_SCOPE},
                          {"IA32_X2APIC_ISR1", 1, THREAD_SCOPE},
                          {"IA32_X2APIC_ISR2", 1, THREAD_SCOPE},
                          {"IA32_X2APIC_ISR3", 1, THREAD_SCOPE},
                          {"IA32_X2APIC_ISR4", 1, THREAD_SCOPE},
                          {"IA32_X2APIC_ISR5", 1, THREAD_SCOPE},
                          {"IA32_X2APIC_ISR6", 1, THREAD_SCOPE},
                          {"IA32_X2APIC_ISR7", 1, THREAD_SCOPE},
                          {"IA32_X2APIC_TMR0", 1, THREAD_SCOPE},
                          {"IA32_X2APIC_TMR1", 1, THREAD_SCOPE},
                          {"IA32_X2APIC_TMR2", 1, THREAD_SCOPE},
                          {"IA32_X2APIC_TMR3", 1, THREAD_SCOPE},
                          {"IA32_X2APIC_TMR4", 1, THREAD_SCOPE},
                          {"IA32_X2APIC_TMR5", 1, THREAD_SCOPE},
                          {"IA32_X2APIC_TMR6", 1, THREAD_SCOPE},
                          {"IA32_X2APIC_TMR7", 1, THREAD_SCOPE},
                          {"IA32_X2APIC_IRR0", 1, THREAD_SCOPE},
                          {"IA32_X2APIC_IRR1", 1, THREAD_SCOPE},
                          {"IA32_X2APIC_IRR2", 1, THREAD_SCOPE},
                          {"IA32_X2APIC_IRR3", 1, THREAD_SCOPE},
                          {"IA32_X2APIC_IRR4", 1, THREAD_SCOPE},
                          {"IA32_X2APIC_IRR5", 1, THREAD_SCOPE},
                          {"IA32_X2APIC_IRR6", 1, THREAD_SCOPE},
                          {"IA32_X2APIC_IRR7", 1, THREAD_SCOPE},
                          {"IA32_X2APIC_ESR", 1, THREAD_SCOPE},
                          {"PIC_LEGACY_LOCAL_APIC_ID", 1, THREAD_SCOPE},
                          {"PIC_LEGACY_LOGICAL_DESTINATION", 1, THREAD_SCOPE},
                          {"EXT_XAPIC_LOCAL_APIC_ID", 1, THREAD_SCOPE}},
    (SCrashdumpRegCPX1[]){// Group 6
                          {"ML3_CR_PIC_DESTINATION_FORMAT", 1, THREAD_SCOPE}},
    (SCrashdumpRegCPX1[]){// Group 7
                          {"EXT_XAPIC_PPR", 1, THREAD_SCOPE}},
    (SCrashdumpRegCPX1[]){// Group 8
                          {"IA32_APIC_BASE", 2, THREAD_SCOPE}},
    (SCrashdumpRegCPX1[]){// Group 9
                          {"CR4", 1, THREAD_SCOPE}},
    (SCrashdumpRegCPX1[]){// Group 10
                          {"CR3", 2, THREAD_SCOPE}},
    (SCrashdumpRegCPX1[]){// Group 11
                          {"CR0", 1, THREAD_SCOPE}},
    (SCrashdumpRegCPX1[]){// Group 12
                          {"IA32_FIXED_CTR0", 2, THREAD_SCOPE},
                          {"IA32_FIXED_CTR1", 2, THREAD_SCOPE},
                          {"IA32_FIXED_CTR2", 2, THREAD_SCOPE},
                          {"IA32_PMC0", 2, THREAD_SCOPE},
                          {"IA32_PMC1", 2, THREAD_SCOPE},
                          {"IA32_PMC2", 2, THREAD_SCOPE},
                          {"IA32_PMC3", 2, THREAD_SCOPE}},
    (SCrashdumpRegCPX1[]){// Group 13
                          {"MSR_LASTBRANCH_TOS", 1, THREAD_SCOPE},
                          {"RAT_CR_LAST_INT_FROM_IP", 2, THREAD_SCOPE},
                          {"RAT_CR_LAST_INT_TO_IP", 2, THREAD_SCOPE},
                          {"THD_SPECIFIC_PATCHING_2", 2, THREAD_SCOPE},
                          {"POST_COUNTER", 1, THREAD_SCOPE},
                          {"FLOW_DETAILS", 1, THREAD_SCOPE},
                          {"THD_SPECIFIC_PATCHING_0", 1, THREAD_SCOPE},
                          {"THD_SPECIFIC_PATCHING_1", 1, THREAD_SCOPE},
                          {"IA32_MCG_STATUS", 1, THREAD_SCOPE},
                          {"PSMI_CTRL", 1, THREAD_SCOPE},
                          {"TARGET_SLEEP_STATE", 1, THREAD_SCOPE},
                          {"PATCH_REV_ID", 1, THREAD_SCOPE},
                          {"MISC_FLAGS", 1, THREAD_SCOPE},
                          {"PSMI_BASE", 1, THREAD_SCOPE},
                          {"DR6", 1, THREAD_SCOPE},
                          {"THD_SP_SPARE_74D", 1, THREAD_SCOPE},
                          {"THD_SP_SPARE_74E", 1, THREAD_SCOPE},
                          {"INTER_THREAD_MISC", 1, THREAD_SCOPE},
                          {"IA32_MISC_ENABLES", 2, THREAD_SCOPE}},
    (SCrashdumpRegCPX1[]){// Group 14
                          {"BAC_CR_CS_BASE", 1, THREAD_SCOPE}},
    (SCrashdumpRegCPX1[]){// Group 15
                          {"CORE_CR_EPTP", 2, THREAD_SCOPE}},
    (SCrashdumpRegCPX1[]){// Group 16
                          {"IA32_EFER", 1, THREAD_SCOPE}},
    (SCrashdumpRegCPX1[]){// Group 17
                          {"MSR_MCG_CONTAIN", 1, CORE_SCOPE}},
    (SCrashdumpRegCPX1[]){// Group 18
                          {"IA32_MTRR_PHYSBASE0", 2, CORE_SCOPE},
                          {"IA32_MTRR_PHYSMASK0", 2, CORE_SCOPE},
                          {"IA32_MTRR_PHYSBASE1", 2, CORE_SCOPE},
                          {"IA32_MTRR_PHYSMASK1", 2, CORE_SCOPE},
                          {"IA32_MTRR_PHYSBASE2", 2, CORE_SCOPE},
                          {"IA32_MTRR_PHYSMASK2", 2, CORE_SCOPE},
                          {"IA32_MTRR_PHYSBASE3", 2, CORE_SCOPE},
                          {"IA32_MTRR_PHYSMASK3", 2, CORE_SCOPE},
                          {"IA32_MTRR_PHYSBASE4", 2, CORE_SCOPE},
                          {"IA32_MTRR_PHYSMASK4", 2, CORE_SCOPE},
                          {"IA32_MTRR_PHYSBASE5", 2, CORE_SCOPE},
                          {"IA32_MTRR_PHYSMASK5", 2, CORE_SCOPE},
                          {"IA32_MTRR_PHYSBASE6", 2, CORE_SCOPE},
                          {"IA32_MTRR_PHYSMASK6", 2, CORE_SCOPE},
                          {"IA32_MTRR_PHYSBASE7", 2, CORE_SCOPE},
                          {"IA32_MTRR_PHYSMASK7", 2, CORE_SCOPE},
                          {"IA32_MTRR_FIX64K_00000", 2, CORE_SCOPE},
                          {"IA32_MTRR_FIX16K_80000", 2, CORE_SCOPE},
                          {"IA32_MTRR_FIX16K_A0000", 2, CORE_SCOPE},
                          {"IA32_MTRR_PHYSBASE8", 2, CORE_SCOPE},
                          {"IA32_MTRR_PHYSMASK8", 2, CORE_SCOPE},
                          {"IA32_MTRR_FIX4K_C0000", 2, CORE_SCOPE},
                          {"IA32_MTRR_FIX4K_C8000", 2, CORE_SCOPE},
                          {"IA32_MTRR_FIX4K_D0000", 2, CORE_SCOPE},
                          {"IA32_MTRR_FIX4K_D8000", 2, CORE_SCOPE},
                          {"IA32_MTRR_FIX4K_E0000", 2, CORE_SCOPE},
                          {"IA32_MTRR_FIX4K_E8000", 2, CORE_SCOPE},
                          {"IA32_MTRR_FIX4K_F0000", 2, CORE_SCOPE},
                          {"IA32_MTRR_FIX4K_F8000", 2, CORE_SCOPE}},
    (SCrashdumpRegCPX1[]){// Group 19
                          {"IA32_MTRR_DEF_TYPE", 2, CORE_SCOPE},
                          {"IA32_MTRR_PHYSBASE9", 2, CORE_SCOPE},
                          {"IA32_MTRR_PHYSMASK9", 2, CORE_SCOPE}}};

static SCrashdumpRegCPX1 sCrashdumpGpThreadRegs[CD_REGS_GP_THREAD] = {
    {"TSC_ADDR_H", 1, THREAD_SCOPE}, {"TSC_ADDR_L", 1, THREAD_SCOPE},
    {"R15_H", 1, THREAD_SCOPE},      {"R15_L", 1, THREAD_SCOPE},
    {"R14_H", 1, THREAD_SCOPE},      {"R14_L", 1, THREAD_SCOPE},
    {"R13_H", 1, THREAD_SCOPE},      {"R13_L", 1, THREAD_SCOPE},
    {"R12_H", 1, THREAD_SCOPE},      {"R12_L", 1, THREAD_SCOPE},
    {"R11_H", 1, THREAD_SCOPE},      {"R11_L", 1, THREAD_SCOPE},
    {"R10_H", 1, THREAD_SCOPE},      {"R10_L", 1, THREAD_SCOPE},
    {"R9_H", 1, THREAD_SCOPE},       {"R9_L", 1, THREAD_SCOPE},
    {"R8_H", 1, THREAD_SCOPE},       {"R8_L", 1, THREAD_SCOPE},
    {"ESP_H", 1, THREAD_SCOPE},      {"ESP_L", 1, THREAD_SCOPE},
    {"EBP_H", 1, THREAD_SCOPE},      {"EBP_L", 1, THREAD_SCOPE},
    {"EDI_H", 1, THREAD_SCOPE},      {"EDI_L", 1, THREAD_SCOPE},
    {"ESI_H", 1, THREAD_SCOPE},      {"ESI_L", 1, THREAD_SCOPE},
    {"EDX_H", 1, THREAD_SCOPE},      {"EDX_L", 1, THREAD_SCOPE},
    {"ECX_H", 1, THREAD_SCOPE},      {"ECX_L", 1, THREAD_SCOPE},
    {"EBX_H", 1, THREAD_SCOPE},      {"EBX_L", 1, THREAD_SCOPE},
    {"EAX_H", 1, THREAD_SCOPE},      {"EAX_L", 1, THREAD_SCOPE},
    {"LIP_H", 1, THREAD_SCOPE},      {"LIP_L", 1, THREAD_SCOPE}};

static uint8_t u8CrashdumpCoreGroupSizes[] = {
    CD_CORE_GROUP_1_SIZE,  CD_CORE_GROUP_2_SIZE,  CD_CORE_GROUP_3_SIZE,
    CD_CORE_GROUP_4_SIZE,  CD_CORE_GROUP_5_SIZE,  CD_CORE_GROUP_6_SIZE,
    CD_CORE_GROUP_7_SIZE,  CD_CORE_GROUP_8_SIZE,  CD_CORE_GROUP_9_SIZE,
    CD_CORE_GROUP_10_SIZE, CD_CORE_GROUP_11_SIZE, CD_CORE_GROUP_12_SIZE,
    CD_CORE_GROUP_13_SIZE, CD_CORE_GROUP_14_SIZE, CD_CORE_GROUP_15_SIZE,
    CD_CORE_GROUP_16_SIZE, CD_CORE_GROUP_17_SIZE, CD_CORE_GROUP_18_SIZE,
    CD_CORE_GROUP_19_SIZE};

/******************************************************************************
 *
 *  crashdumpJsonCPX1
 *
 *  This function formats the Crashdump into a JSON object
 *
 ******************************************************************************/
static void crashdumpJsonCPX1(uint32_t u32NumReads, SCrashdump* sCrashdump,
                              cJSON* pJsonChild)
{
    char jsonItemName[CD_JSON_STRING_LEN] = {0};
    char jsonItemString[CD_JSON_STRING_LEN] = {0};
    uint32_t u32CrashIndex = 0;
    uint64_t u64RegValue = 0;
    uint8_t CoreHigh = 0;
    uint8_t CoreLow = 0;
    // Get the number of cores included in the crashdump
    uint32_t u32CoreMask = ((sCrashdump->header.data[3] & 0xFFFF) << 16) |
                           (sCrashdump->header.data[2] & 0xFFFF);
    uint32_t u32CoreMaskLow = (sCrashdump->header.data[2] & 0xFFFF);
    uint32_t u32CoreMaskHigh = u32CoreMask;
    uint32_t u32CoreCount = __builtin_popcount(u32CoreMask);

    // Use the core count to calculate the expected number of reads and compare
    // to the returned number of reads
    uint32_t u32ExpectedNumReads = CD_DWORDS_UNCORE +
                                   (CD_DWORDS_CORE * u32CoreCount) +
                                   (CD_DWORDS_GP * u32CoreCount);

    // Add the crashdump header contents to the Crashdump JSON structure
    for (int i = 0; i < (sCrashdump->header.size - 1); i++)
    {
        cd_snprintf_s(jsonItemName, CD_JSON_STRING_LEN, CD_JSON_HEADER_ENTRY,
                      i);
        cd_snprintf_s(jsonItemString, CD_JSON_STRING_LEN, "0x%x",
                      sCrashdump->header.data[i]);
        cJSON_AddStringToObject(pJsonChild, jsonItemName, jsonItemString);
    }
    // Add the crashdump data size
    cd_snprintf_s(jsonItemString, CD_JSON_STRING_LEN, "0x%x", u32NumReads);
    cJSON_AddStringToObject(pJsonChild, CD_JSON_DATA_SIZE_NAME, jsonItemString);

    // Check that we have the correct amount of data
    if (!u32NumReads || u32ExpectedNumReads != u32NumReads)
    {
        // We don't have the right amount of data to parse, so just dump it and
        // bail
        for (uint8_t u8CoreNum = CD_CORES_IN_MASK; u8CoreNum > 0; u8CoreNum--)
        {
            if (!(u32CoreMask & (1 << (u8CoreNum - 1))))
                continue;
            // Add the core to the Crashdump JSON structure only if it doesn't
            // exist
            cJSON* core;
            cd_snprintf_s(jsonItemName, CD_JSON_STRING_LEN, CD_JSON_CORE_NAME,
                          (u8CoreNum - 1));
            if ((core = cJSON_GetObjectItemCaseSensitive(pJsonChild,
                                                         jsonItemName)) == NULL)
            {
                cJSON_AddItemToObject(pJsonChild, jsonItemName,
                                      core = cJSON_CreateObject());
            }
            cJSON* rawData = cJSON_CreateIntArray(
                reinterpret_cast<const int*>(sCrashdump->data), u32NumReads);
            cJSON_AddItemToObject(core, "raw_data", rawData);
        }
        return;
    }

    // Save the uncore data for later
    cJSON* uncore = cJSON_CreateObject();
    for (uint32_t u32RegNum = 0; u32RegNum < CD_REGS_UNCORE; u32RegNum++)
    {
        if (sCrashdumpUncoreRegs[u32RegNum].dwords == 1)
        {
            cd_snprintf_s(jsonItemString, CD_JSON_STRING_LEN, "0x%x",
                          sCrashdump->data[u32CrashIndex++]);
        }
        else if (sCrashdumpUncoreRegs[u32RegNum].dwords == 2)
        {
            u64RegValue = sCrashdump->data[u32CrashIndex++];
            u64RegValue |= (uint64_t)(sCrashdump->data[u32CrashIndex++]) << 32;
            cd_snprintf_s(jsonItemString, CD_JSON_STRING_LEN, "0x%llx",
                          u64RegValue);
        }
        cJSON_AddStringToObject(uncore, sCrashdumpUncoreRegs[u32RegNum].name,
                                jsonItemString);
    }
    // CoreMask Group 0 (15 - 0) and Core Group 1 (27 - 16)
    for (uint8_t u8CoreGroup = 0; u8CoreGroup < CD_NUM_COREGROUP_CORE;
         u8CoreGroup++)
    {
        if (u8CoreGroup == 0)
        {
            CoreHigh = CORE_GROUP0_HIGH;
            CoreLow = CORE_GROUP0_LOW;
            u32CoreMask = u32CoreMaskLow;
        }
        else
        {
            CoreHigh = CORE_GROUP1_HIGH;
            CoreLow = CORE_GROUP1_LOW;
            u32CoreMask = u32CoreMaskHigh;
        }
        // For each register Group
        for (uint8_t u8GroupNum = 0; u8GroupNum < CD_NUM_GROUPS_CORE;
             u8GroupNum++)
        {
            // Add the thread data to the core
            for (uint8_t u8ThreadNum = 0; u8ThreadNum < CD_THREADS_PER_CORE;
                 u8ThreadNum++)
            {
                // Write out the core register data
                for (uint8_t u8CoreNum = CoreHigh; u8CoreNum > CoreLow;
                     u8CoreNum--)
                {
                    if (!(u32CoreMask & (1 << (u8CoreNum - 1))))
                        continue;
                    // Add the core info to the Crashdump JSON structure for
                    // this group only if it doesn't exist
                    cJSON* core;
                    cd_snprintf_s(jsonItemName, CD_JSON_STRING_LEN,
                                  CD_JSON_CORE_NAME, (u8CoreNum - 1));
                    if ((core = cJSON_GetObjectItemCaseSensitive(
                             pJsonChild, jsonItemName)) == NULL)
                    {
                        cJSON_AddItemToObject(pJsonChild, jsonItemName,
                                              core = cJSON_CreateObject());
                    }
                    // Add the thread info to the Crashdump JSON structure for
                    // this group only if it doesn't exist
                    cJSON *thread, *thread_temp;
                    cd_snprintf_s(jsonItemName, CD_JSON_STRING_LEN,
                                  CD_JSON_THREAD_NAME, u8ThreadNum);
                    if ((thread = cJSON_GetObjectItemCaseSensitive(
                             core, jsonItemName)) == NULL)
                    {
                        cJSON_AddItemToObject(core, jsonItemName,
                                              thread = cJSON_CreateObject());
                    }
                    if ((thread_temp = cJSON_GetObjectItemCaseSensitive(
                             core, CD_JSON_THREAD_1)) == NULL)
                    {
                        cJSON_AddItemToObject(core, CD_JSON_THREAD_1,
                                              thread_temp =
                                                  cJSON_CreateObject());
                    }
                    for (uint32_t u32RegNum = 0;
                         u32RegNum < u8CrashdumpCoreGroupSizes[u8GroupNum];
                         u32RegNum++)
                    {
                        // Thread scope registers are dumped for each thread, so
                        // add them to a thread object
                        if (sCrashdumpCoreRegsCPX1[u8GroupNum][u32RegNum]
                                .scope == THREAD_SCOPE)
                        {
                            if (sCrashdumpCoreRegsCPX1[u8GroupNum][u32RegNum]
                                    .dwords == 1)
                            {
                                cd_snprintf_s(
                                    jsonItemString, CD_JSON_STRING_LEN, "0x%x",
                                    sCrashdump->data[u32CrashIndex++]);
                            }
                            else if (sCrashdumpCoreRegsCPX1[u8GroupNum]
                                                           [u32RegNum]
                                                               .dwords == 2)
                            {
                                u64RegValue = sCrashdump->data[u32CrashIndex++];
                                u64RegValue |=
                                    (uint64_t)(
                                        sCrashdump->data[u32CrashIndex++])
                                    << 32;
                                cd_snprintf_s(jsonItemString,
                                              CD_JSON_STRING_LEN, "0x%llx",
                                              u64RegValue);
                            }
                            cd_snprintf_s(
                                jsonItemName, CD_JSON_STRING_LEN, "%s",
                                sCrashdumpCoreRegsCPX1[u8GroupNum][u32RegNum]
                                    .name);
                            cJSON_AddStringToObject(thread, jsonItemName,
                                                    jsonItemString);
                        }
                        // Core scope registers are only dumped for thread 0
                        else if (sCrashdumpCoreRegsCPX1[u8GroupNum][u32RegNum]
                                         .scope == CORE_SCOPE &&
                                 u8ThreadNum == 0)
                        {
                            if (sCrashdumpCoreRegsCPX1[u8GroupNum][u32RegNum]
                                    .dwords == 1)
                            {
                                cd_snprintf_s(
                                    jsonItemString, CD_JSON_STRING_LEN, "0x%x",
                                    sCrashdump->data[u32CrashIndex++]);
                            }
                            else if (sCrashdumpCoreRegsCPX1[u8GroupNum]
                                                           [u32RegNum]
                                                               .dwords == 2)
                            {
                                u64RegValue = sCrashdump->data[u32CrashIndex++];
                                u64RegValue |=
                                    (uint64_t)(
                                        sCrashdump->data[u32CrashIndex++])
                                    << 32;
                                cd_snprintf_s(jsonItemString,
                                              CD_JSON_STRING_LEN, "0x%llx",
                                              u64RegValue);
                            }
                            cd_snprintf_s(
                                jsonItemName, CD_JSON_STRING_LEN, "%s",
                                sCrashdumpCoreRegsCPX1[u8GroupNum][u32RegNum]
                                    .name);
                            cJSON_AddStringToObject(thread, jsonItemName,
                                                    jsonItemString);

                            cJSON_AddStringToObject(thread_temp, jsonItemName,
                                                    jsonItemString);
                        }
                    }
                }
            }
        }
    }

    // Write out the GP register data to the debug log
    for (uint8_t u8CoreNum = CD_CORES_IN_MASK; u8CoreNum > 0; u8CoreNum--)
    {
        if (!(u32CoreMask & (1 << (u8CoreNum - 1))))
            continue;
        // Add the core to the Crashdump JSON structure only if it doesn't
        // exist
        cJSON* core;
        cd_snprintf_s(jsonItemName, CD_JSON_STRING_LEN, CD_JSON_CORE_NAME,
                      (u8CoreNum - 1));
        if ((core = cJSON_GetObjectItemCaseSensitive(pJsonChild,
                                                     jsonItemName)) == NULL)
        {
            cJSON_AddItemToObject(pJsonChild, jsonItemName,
                                  core = cJSON_CreateObject());
        }

        for (uint8_t u8ThreadNum = CD_THREADS_PER_CORE; u8ThreadNum > 0;
             u8ThreadNum--)
        {
            cd_snprintf_s(jsonItemName, CD_JSON_STRING_LEN, CD_JSON_THREAD_NAME,
                          u8ThreadNum - 1);
            // Add the thread to the Crashdump JSON structure only if it
            // doesn't exist
            cJSON* thread;
            if ((thread = cJSON_GetObjectItemCaseSensitive(
                     core, jsonItemName)) == NULL)
            {
                cJSON_AddItemToObject(core, jsonItemName,
                                      thread = cJSON_CreateObject());
            }

            // Add the saved uncore data to the thread
            cJSON* uncore_reg = NULL;
            cJSON_ArrayForEach(uncore_reg, uncore)
            {
                cJSON_AddStringToObject(thread, uncore_reg->string,
                                        cJSON_GetStringValue(uncore_reg));
            }

            for (uint32_t u32RegNum = 0; u32RegNum < CD_REGS_GP_THREAD;
                 u32RegNum++)
            {
                if (sCrashdumpGpThreadRegs[u32RegNum].dwords == 1)
                {
                    cd_snprintf_s(jsonItemString, CD_JSON_STRING_LEN, "0x%x",
                                  sCrashdump->data[u32CrashIndex++]);
                }
                else if (sCrashdumpGpThreadRegs[u32RegNum].dwords == 2)
                {
                    u64RegValue = sCrashdump->data[u32CrashIndex++];
                    u64RegValue |= (uint64_t)(sCrashdump->data[u32CrashIndex++])
                                   << 32;
                    cd_snprintf_s(jsonItemString, CD_JSON_STRING_LEN, "0x%llx",
                                  u64RegValue);
                }
                cJSON_AddStringToObject(thread,
                                        sCrashdumpGpThreadRegs[u32RegNum].name,
                                        jsonItemString);
            }
        }
    }
}

/******************************************************************************
 *
 *  logCrashdumpCPX1
 *
 *  BMC performs the crashdump retrieve from the processor directly via
 *  PECI interface for internal state of the cores and Cbos after a platform
 *  three (3) strike failure. The Crash Dump from CPU will be empty (size 0)
 *  if no cores qualify to be dumped. A core will not be dumped if the power
 *  state is such that it cannot be accessed. A core will be dumped only if
 *  it has experienced a "3-strike" Machine Check Error. The PECI flow is
 *  listed below to generate a Crash Dump, and decode it.
 *
 *    WrPkgConfig() -
 *         0x80 0x0003 0x00010038
 *         Open Crash Dump Sequence.
 *
 *    RdPkgConfig() -
 *         0x80 0x1020 HEADER_SIZE
 *         Data of 0x5 should be value for Crash Dump.
 *
 *    RdPkgConfig() -
 *         0x80 0x0002 CRASHDUMP_HEADER0
 *
 *    RdPkgConfig() -
 *         0x80 0x0002 CRASHDUMP_HEADER1
 *
 *    RdPkgConfig() -
 *         0x80 0x0002 CRASHDUMP_HEADER2
 *
 *    RdPkgConfig() -
 *         0x80 0x0002 CRASHDUMP_HEADER3
 *
 *    RdPkgConfig() -
 *         0x80 0x0002 NUMBER_OF_READS
 *         Data N is the number of additional RdPkgConfig() commands
 *         required to collect all the data for the dump. This value
 *         depends on the number of cores flagged in the core mask,
 *         and the content to be dumped.
 *
 *    RdPkgConfig() * N -
 *         0x80 0x0002 DataN
 *         Crash Dump data [1-N]. Use decoding table for decoding.
 *
 *    WrPkgConfig() -
 *         0x80 0x0004 0x00010038 Close Crash Dump Sequence.
 *
 ******************************************************************************/
int logCrashdumpCPX1(crashdump::CPUInfo& cpuInfo, cJSON* pJsonChild)
{
    EPECIStatus ePECIStatus = PECI_CC_SUCCESS;
    int ret = 0;
    int peci_fd = -1;
    uint8_t cc = 0;

    if (peci_Lock(&peci_fd, PECI_WAIT_FOREVER) != PECI_CC_SUCCESS)
    {
        return 1;
    }

    // Start the crashdump log
    SCrashdump sCrashdump = {};

    // Open the Crashdump sequence
    ePECIStatus =
        peci_WrPkgConfig_seq(cpuInfo.clientAddr, MBX_INDEX_VCU, VCU_OPEN_SEQ,
                             VCU_CRASHDUMP_SEQ, sizeof(uint32_t), peci_fd, &cc);
    if (ePECIStatus != PECI_CC_SUCCESS)
    {
        // Crashdump sequence failed, abort the sequence
        peci_WrPkgConfig_seq(cpuInfo.clientAddr, MBX_INDEX_VCU, VCU_ABORT_SEQ,
                             VCU_CRASHDUMP_SEQ, sizeof(uint32_t), peci_fd, &cc);
        return 1;
    }

    // Get the Header Data
    ePECIStatus = peci_RdPkgConfig_seq(
        cpuInfo.clientAddr, MBX_INDEX_VCU, CD_HEADER_PARAM, sizeof(uint32_t),
        (uint8_t*)&sCrashdump.header.size, peci_fd, &cc);
    if (ePECIStatus != PECI_CC_SUCCESS)
    {
        // Crashdump sequence failed, abort the sequence
        peci_WrPkgConfig_seq(cpuInfo.clientAddr, MBX_INDEX_VCU, VCU_ABORT_SEQ,
                             VCU_CRASHDUMP_SEQ, sizeof(uint32_t), peci_fd, &cc);
        return 1;
    }
    for (uint32_t i = 0; i < CD_DWORDS_HEADER; i++)
    {
        ePECIStatus = peci_RdPkgConfig_seq(
            cpuInfo.clientAddr, MBX_INDEX_VCU, VCU_READ, sizeof(uint32_t),
            (uint8_t*)&sCrashdump.header.data[i], peci_fd, &cc);
        if (ePECIStatus != PECI_CC_SUCCESS)
        {
            // Crashdump sequence failed, abort the sequence and break out
            // of the header loop to go to the next CPU
            peci_WrPkgConfig_seq(cpuInfo.clientAddr, MBX_INDEX_VCU,
                                 VCU_ABORT_SEQ, VCU_CRASHDUMP_SEQ,
                                 sizeof(uint32_t), peci_fd, &cc);
            return 1;
        }
    }

    // Get the number of remaining dword reads
    uint32_t u32NumReads = 0;
    ePECIStatus = peci_RdPkgConfig_seq(cpuInfo.clientAddr, MBX_INDEX_VCU,
                                       VCU_READ, sizeof(uint32_t),
                                       (uint8_t*)&u32NumReads, peci_fd, &cc);
    if (ePECIStatus != PECI_CC_SUCCESS)
    {
        // Crashdump sequence failed, abort the sequence and go to the next CPU
        peci_WrPkgConfig_seq(cpuInfo.clientAddr, MBX_INDEX_VCU, VCU_ABORT_SEQ,
                             VCU_CRASHDUMP_SEQ, sizeof(uint32_t), peci_fd, &cc);
        return 1;
    }

    // Get the raw data
    sCrashdump.data = (uint32_t*)calloc(u32NumReads, sizeof(uint32_t));
    if (sCrashdump.data == NULL)
    {
        // calloc failed, abort the sequence
        peci_WrPkgConfig_seq(cpuInfo.clientAddr, MBX_INDEX_VCU, VCU_ABORT_SEQ,
                             VCU_CRASHDUMP_SEQ, sizeof(uint32_t), peci_fd, &cc);
        return 1;
    }
    for (uint32_t i = 0; i < u32NumReads; i++)
    {
        ePECIStatus = peci_RdPkgConfig_seq(
            cpuInfo.clientAddr, MBX_INDEX_VCU, VCU_READ, sizeof(uint32_t),
            (uint8_t*)&sCrashdump.data[i], peci_fd, &cc);
        if (ePECIStatus != PECI_CC_SUCCESS)
        {
            // Crashdump sequence failed, note the number of dwords read and
            // abort the sequence
            u32NumReads = i;
            peci_WrPkgConfig_seq(cpuInfo.clientAddr, MBX_INDEX_VCU,
                                 VCU_ABORT_SEQ, VCU_CRASHDUMP_SEQ,
                                 sizeof(uint32_t), peci_fd, &cc);
            ret = 1;
            break;
        }
    }

    // Close the Crashdump sequence
    peci_WrPkgConfig_seq(cpuInfo.clientAddr, MBX_INDEX_VCU, VCU_CLOSE_SEQ,
                         VCU_CRASHDUMP_SEQ, sizeof(uint32_t), peci_fd, &cc);

    // Log the Crashdump for this CPU
    crashdumpJsonCPX1(u32NumReads, &sCrashdump, pJsonChild);

    free(sCrashdump.data);

    peci_Unlock(peci_fd);
    return ret;
}

static SCrashdumpRegICX1 sCrashdumpCoreHeaderICX1[] = {
    // Register-name, Size
    {"crashlog_version", 4},     {"size", 4},
    {"art_ctc_timestamp", 8},    {"patch_revid", 4},
    {"shutdown_reason", 4},      {"whoami", 4},
    {"miscellaneous_status", 4},
};

static SCrashdumpRegICX1 sCrashdumpCoreRegsICX1[] = {
    // Register-name, Size
    {"CR0", 4},
    {"CR4", 4},
    {"CR2", 8},
    {"CR3", 8},
    {"EFER", 4},
    {"CS_SELECTOR", 2},
    {"SS_SELECTOR", 2},
    {"DS_SELECTOR", 2},
    {"ES_SELECTOR", 2},
    {"FS_SELECTOR", 2},
    {"GS_SELECTOR", 2},
    {"EFLAGS", 8},
    {"LIP", 8},
    {"RAX", 8},
    {"RCX", 8},
    {"RDX", 8},
    {"RBX", 8},
    {"RSP", 8},
    {"RBP", 8},
    {"RSI", 8},
    {"RDI", 8},
    {"R8", 8},
    {"R9", 8},
    {"R10", 8},
    {"R11", 8},
    {"R12", 8},
    {"R13", 8},
    {"R14", 8},
    {"R15", 8},
    {"R16", 8},
    {"R17", 8},
    {"R18", 8},
    {"R19", 8},
    {"R20", 8},
    {"R21", 8},
    {"R22", 8},
    {"R23", 8},
    {"ifu_cr_mc0_ctl", 8},
    {"ifu_cr_mc0_status", 8},
    {"ifu_cr_mc0_addr", 8},
    {"ifu_cr_mc0_misc", 8},
    {"dcu_cr_mc1_ctl", 8},
    {"dcu_cr_mc1_status", 8},
    {"dcu_cr_mc1_addr", 8},
    {"dcu_cr_mc1_misc", 8},
    {"dtlb_cr_mc2_ctl", 8},
    {"dtlb_cr_mc2_status", 8},
    {"dtlb_cr_mc2_addr", 8},
    {"dtlb_cr_mc2_misc", 8},
    {"ml2_cr_mc3_ctl", 8},
    {"ml2_cr_mc3_status", 8},
    {"ml2_cr_mc3_addr", 8},
    {"ml2_cr_mc3_misc", 8},
    {"PMH_CR_MTRRVARBASE0", 8},
    {"PMH_CR_MTRRVARMASK0", 8},
    {"PMH_CR_MTRRVARBASE1", 8},
    {"PMH_CR_MTRRVARMASK1", 8},
    {"PMH_CR_MTRRVARBASE2", 8},
    {"PMH_CR_MTRRVARMASK2", 8},
    {"PMH_CR_MTRRVARBASE3", 8},
    {"PMH_CR_MTRRVARMASK3", 8},
    {"PMH_CR_MTRRVARBASE4", 8},
    {"PMH_CR_MTRRVARMASK4", 8},
    {"PMH_CR_MTRRVARBASE5", 8},
    {"PMH_CR_MTRRVARMASK5", 8},
    {"PMH_CR_MTRRVARBASE6", 8},
    {"PMH_CR_MTRRVARMASK6", 8},
    {"PMH_CR_MTRRVARBASE7", 8},
    {"PMH_CR_MTRRVARMASK7", 8},
    {"PMH_CR_MTRRFIX64K", 8},
    {"PMH_CR_MTRRFIX16K8", 8},
    {"PMH_CR_MTRRFIX16KA", 8},
    {"PMH_CR_MTRRVARBASE8", 8},
    {"PMH_CR_MTRRVARMASK8", 8},
    {"PMH_CR_MTRRFIX4KC0", 8},
    {"PMH_CR_MTRRFIX4KC8", 8},
    {"PMH_CR_MTRRFIX4KD0", 8},
    {"PMH_CR_MTRRFIX4KD8", 8},
    {"PMH_CR_MTRRFIX4KE0", 8},
    {"PMH_CR_MTRRFIX4KE8", 8},
    {"PMH_CR_MTRRFIX4KF0", 8},
    {"PMH_CR_MTRRFIX4KF8", 8},
    {"PMH_CR_MTRRDEFAULT", 8},
    {"PMH_CR_MTRRVARBASE9", 8},
    {"PMH_CR_MTRRVARMASK9", 8},
    {"CORE_CR_APIC_BASE", 8},
    {"CORE_CR_EPTP", 8},
    {"CORE_CR_MISC_ENABLES", 8},
    {"SCP_CR_THD_SPECIFIC_PATCHING_2", 8},
    {"SCP_CR_POST_COUNTER", 4},
    {"SCP_CR_FLOW_DETAILS", 4},
    {"SCP_CR_THD_SPECIFIC_PATCHING_0", 4},
    {"SCP_CR_THD_SPECIFIC_PATCHING_1", 4},
    {"SCP_CR_MCG_STATUS", 4},
    {"SCP_CR_PSMI_CTRL", 4},
    {"SCP_CR_TARGET_SLEEP_STATE", 4},
    {"SCP_CR_PATCH_REV_ID", 4},
    {"DebugCTLMSR", 8},
    {"LER_FROM", 8},
    {"LER_TO", 8},
    {"LER_INFO", 8},
};

/******************************************************************************
 *
 *  getJsonDataString
 *
 *  This function takes the number of requested bytes at the index of the given
 *  data and writes it to the given string.  It returns the number of bytes
 *  written.
 *
 ******************************************************************************/
static int getJsonDataString(uint8_t* u8Data, uint32_t u32DataSize,
                             uint32_t u32DataIndex, uint32_t u32NumBytes,
                             char* pDataString, uint32_t u32StringSize)
{
    // check that we have enough data
    if (u32DataIndex + u32NumBytes > u32DataSize)
    {
        return -1;
    }
    // check that our string buffer is large enough
    if (u32NumBytes * 2 + SIZE_OF_0x0 > u32StringSize)
    {
        cd_snprintf_s(pDataString, u32StringSize, "String buffer too small");
        return u32NumBytes;
    }
    // initialize the string to "0"
    cd_snprintf_s(pDataString, u32StringSize, "0x0");
    // handle leading zeros
    bool leading = true;
    char* ptr = &pDataString[2];
    for (int i = u32NumBytes - 1; i >= 0; i--)
    {
        // exclude any leading zeros per the schema
        if (leading && u8Data[u32DataIndex + i] == 0)
        {
            continue;
        }
        leading = false;
        ptr += cd_snprintf_s(ptr, (pDataString + u32StringSize) - ptr, "%02x",
                             u8Data[u32DataIndex + i]);
    }
    return u32NumBytes;
}

/******************************************************************************
 *
 *  crashdumpJsonICX1
 *
 *  This function formats the Crashdump into a JSON object
 *
 ******************************************************************************/
static void crashdumpJsonICX1(uint32_t u32CoreNum, uint32_t u32ThreadNum,
                              uint32_t u32CrashSize, uint8_t* pu8Crashdump,
                              cJSON* pJsonChild, uint8_t cc)
{
    char jsonItemName[CD_JSON_STRING_LEN] = {0};
    char jsonItemString[CD_JSON_STRING_LEN] = {0};

    // Add the core number item to the Crashdump JSON structure only if it
    // doesn't already exist
    cJSON* core = NULL;
    cd_snprintf_s(jsonItemName, CD_JSON_STRING_LEN, CD_JSON_CORE_NAME,
                  u32CoreNum);
    if ((core = cJSON_GetObjectItemCaseSensitive(pJsonChild, jsonItemName)) ==
        NULL)
    {
        cJSON_AddItemToObject(pJsonChild, jsonItemName,
                              core = cJSON_CreateObject());
    }

    // Add the thread number item to the Crashdump JSON structure
    cJSON* thread;
    cd_snprintf_s(jsonItemName, CD_JSON_STRING_LEN, CD_JSON_THREAD_NAME,
                  u32ThreadNum);
    cJSON_AddItemToObject(core, jsonItemName, thread = cJSON_CreateObject());

    // set up the data index
    uint32_t u32DataIndex = 0;
    // Add the header data
    for (int i = 0;
         i < (sizeof(sCrashdumpCoreHeaderICX1) / sizeof(SCrashdumpRegICX1));
         i++)
    {
        int ret = getJsonDataString(pu8Crashdump, u32CrashSize, u32DataIndex,
                                    sCrashdumpCoreHeaderICX1[i].size,
                                    jsonItemString, sizeof(jsonItemString));
        if (ret < 0)
        {
            return;
        }
        u32DataIndex += ret;
        if (PECI_CC_UA(cc))
        {
            cd_snprintf_s(jsonItemString, CD_JSON_STRING_LEN, CD_JSON_UA, cc);
            cJSON_AddStringToObject(thread, sCrashdumpCoreHeaderICX1[i].name,
                                    jsonItemString);
            return;
        }
        cJSON_AddStringToObject(thread, sCrashdumpCoreHeaderICX1[i].name,
                                jsonItemString);
    }

    // Add the arch state data
    for (int i = 0;
         i < (sizeof(sCrashdumpCoreRegsICX1) / sizeof(SCrashdumpRegICX1)); i++)
    {
        int ret = getJsonDataString(pu8Crashdump, u32CrashSize, u32DataIndex,
                                    sCrashdumpCoreRegsICX1[i].size,
                                    jsonItemString, sizeof(jsonItemString));
        if (ret < 0)
        {
            return;
        }
        u32DataIndex += ret;

        if (PECI_CC_UA(cc))
        {
            cd_snprintf_s(jsonItemString, CD_JSON_STRING_LEN, CD_JSON_UA, cc);
            cJSON_AddStringToObject(thread, sCrashdumpCoreRegsICX1[i].name,
                                    jsonItemString);
            return;
        }
        cJSON_AddStringToObject(thread, sCrashdumpCoreRegsICX1[i].name,
                                jsonItemString);
    }

    // Add the LBR data
    for (int i = 0; i < CD_LBRS_PER_CORE; i++)
    {
        const char* cLbrName[CD_ENTRIES_PER_LBR] = {
            CD_JSON_LBR_NAME_FROM,
            CD_JSON_LBR_NAME_TO,
            CD_JSON_LBR_NAME_INFO,
        };
        for (int j = 0; j < CD_ENTRIES_PER_LBR; j++)
        {
            int ret = getJsonDataString(pu8Crashdump, u32CrashSize,
                                        u32DataIndex, CD_LBR_SIZE,
                                        jsonItemString, sizeof(jsonItemString));
            if (ret < 0)
            {
                return;
            }
            u32DataIndex += ret;
            cd_snprintf_s(jsonItemName, CD_JSON_STRING_LEN, cLbrName[j], i);
            if (PECI_CC_UA(cc))
            {
                cd_snprintf_s(jsonItemString, CD_JSON_STRING_LEN, CD_JSON_UA,
                              cc);
                cJSON_AddStringToObject(thread, jsonItemName, jsonItemString);
                return;
            }
            cJSON_AddStringToObject(thread, jsonItemName, jsonItemString);
        }
    }

    // if there is no more data, exit
    if (u32DataIndex >= u32CrashSize)
    {
        return;
    }

    // if there is still data, it's the SQ data, so dump it
    // Add the SQ Dump item to the Crashdump JSON structure
    cJSON* sqDump;
    cJSON_AddItemToObject(core, "SQ", sqDump = cJSON_CreateObject());

    // Add the SQ data
    for (int i = 0; i < CD_JSON_NUM_SQ_ENTRIES; i++)
    {
        int ret = getJsonDataString(pu8Crashdump, u32CrashSize, u32DataIndex,
                                    CD_JSON_SQ_ENTRY_SIZE, jsonItemString,
                                    sizeof(jsonItemString));
        if (ret < 0)
        {
            return;
        }
        u32DataIndex += ret;

        if (PECI_CC_UA(cc))
        {
            cd_snprintf_s(jsonItemName, CD_JSON_STRING_LEN,
                          CD_JSON_SQ_ENTRY_NAME, i);
            cd_snprintf_s(jsonItemString, CD_JSON_STRING_LEN, CD_JSON_UA, cc);
            cJSON_AddStringToObject(sqDump, jsonItemName, jsonItemString);
            return;
        }
        cd_snprintf_s(jsonItemName, CD_JSON_STRING_LEN, CD_JSON_SQ_ENTRY_NAME,
                      i);
        cJSON_AddStringToObject(sqDump, jsonItemName, jsonItemString);
    }
}

/******************************************************************************
 *
 *  logCrashdumpICX1
 *
 *  BMC performs the crashdump retrieve from the processor directly via
 *  PECI interface for internal state of the cores and Cbos after a platform
 *  three (3) strike failure. The Crash Dump from CPU will be empty (size 0)
 *  if no cores qualify to be dumped. A core will not be dumped if the power
 *  state is such that it cannot be accessed. A core will be dumped only if it
 *  has experienced a "3-strike" Machine Check Error.
 *
 ******************************************************************************/
int logCrashdumpICX1(crashdump::CPUInfo& cpuInfo, cJSON* pJsonChild)
{
    int ret = 0;
    EPECIStatus ePeciStatus;
    uint8_t cc = 0;

    // Crashdump Discovery
    uint8_t u8CrashdumpDisabled = ICX_A0_CRASHDUMP_DISABLED;
    ePeciStatus = peci_CrashDump_Discovery(
        cpuInfo.clientAddr, PECI_CRASHDUMP_ENABLED, 0, 0, 0, sizeof(uint8_t),
        &u8CrashdumpDisabled, &cc);
    if (ePeciStatus != PECI_CC_SUCCESS ||
        u8CrashdumpDisabled == ICX_A0_CRASHDUMP_DISABLED)
    {
        fprintf(stderr,
                "Crashdump is disabled (%d) during discovery "
                "(disabled:%d)\n",
                ePeciStatus, u8CrashdumpDisabled);
        return 1;
    }

    // Crashdump Number of Agents
    uint16_t u16CrashdumpNumAgents;
    ePeciStatus = peci_CrashDump_Discovery(
        cpuInfo.clientAddr, PECI_CRASHDUMP_NUM_AGENTS, 0, 0, 0,
        sizeof(uint16_t), (uint8_t*)&u16CrashdumpNumAgents, &cc);
    if (ePeciStatus != PECI_CC_SUCCESS ||
        u16CrashdumpNumAgents <= PECI_CRASHDUMP_CORE)
    {
        fprintf(stderr, "Error (%d) during discovery (num of agents:%d)\n",
                ePeciStatus, u16CrashdumpNumAgents);
        return 1;
    }

    // Crashdump Agent Data
    // Agent Unique ID
    uint64_t u64UniqueId;
    ePeciStatus = peci_CrashDump_Discovery(
        cpuInfo.clientAddr, PECI_CRASHDUMP_AGENT_DATA, PECI_CRASHDUMP_AGENT_ID,
        PECI_CRASHDUMP_CORE, 0, sizeof(uint64_t), (uint8_t*)&u64UniqueId, &cc);
    if (ePeciStatus != PECI_CC_SUCCESS)
    {
        fprintf(stderr, "Error (%d) during discovery (id:0x%" PRIx64 ")\n",
                ePeciStatus, u64UniqueId);
        return 1;
    }

    // Agent Payload Size
    uint64_t u64PayloadExp;
    ePeciStatus = peci_CrashDump_Discovery(
        cpuInfo.clientAddr, PECI_CRASHDUMP_AGENT_DATA,
        PECI_CRASHDUMP_AGENT_PARAM, PECI_CRASHDUMP_CORE,
        PECI_CRASHDUMP_PAYLOAD_SIZE, sizeof(uint64_t), (uint8_t*)&u64PayloadExp,
        &cc);
    if (ePeciStatus != PECI_CC_SUCCESS)
    {
        fprintf(stderr, "Error (%d) during discovery (payload:0x%" PRIx64 ")\n",
                ePeciStatus, u64PayloadExp);
        return 1;
    }

    // Crashdump Get Frames
    // Go through each enabled core
    for (uint32_t u32CoreNum = 0; (cpuInfo.coreMask >> u32CoreNum) != 0;
         u32CoreNum++)
    {
        if (!(cpuInfo.coreMask & (1 << u32CoreNum)))
        {
            continue;
        }
        uint32_t u32Threads = CD_ST_THREADS_PER_CORE;
        for (uint32_t u32ThreadNum = 0; u32ThreadNum < u32Threads;
             u32ThreadNum++)
        {
            // Get the crashdump size for this thread from the first read
            UCrashdumpVerSize uCrashdumpVerSize;
            ePeciStatus = peci_CrashDump_GetFrame(
                cpuInfo.clientAddr, PECI_CRASHDUMP_CORE, u32CoreNum, 0,
                sizeof(uint64_t), (uint8_t*)&uCrashdumpVerSize.raw, &cc);

            if (ePeciStatus != PECI_CC_SUCCESS)
            {
                ret = 1;
                fprintf(stderr,
                        "Error (%d) during GetFrame 0 (0x%" PRIx64 ")\n",
                        ePeciStatus, uCrashdumpVerSize.raw);
                continue;
            }

            if (PECI_CC_SKIP_CORE(cc))
            {
                break;
            }
            if (PECI_CC_SKIP_SOCKET(cc))
            {
                return 0;
            }

            uint32_t u32CrashdumpSize = uCrashdumpVerSize.field.regDumpSize +
                                        uCrashdumpVerSize.field.sqDumpSize +
                                        ICX_A0_FRAME_BYTE_OFFSET;
            uint64_t* pu64Crashdump = (uint64_t*)(calloc(u32CrashdumpSize, 1));
            if (pu64Crashdump == NULL)
            {
                // calloc failed, exit
                fprintf(stderr, "Error (%d) allocating memory (size:%d)\n",
                        ePeciStatus, u32CrashdumpSize);
                return 1;
            }
            pu64Crashdump[0] = uCrashdumpVerSize.raw;

            // log crashed core
            SET_BIT(cpuInfo.crashedCoreMask, u32CoreNum);

            // Get the rest of the crashdump data
            for (uint32_t i = 1; i < (u32CrashdumpSize / sizeof(uint64_t)); i++)
            {
                ePeciStatus = peci_CrashDump_GetFrame(
                    cpuInfo.clientAddr, PECI_CRASHDUMP_CORE, u32CoreNum, 0,
                    sizeof(uint64_t), (uint8_t*)&pu64Crashdump[i], &cc);

                if (PECI_CC_SKIP_CORE(cc))
                {
                    fprintf(stderr, "Error (%d) during GetFrame (num:%d)\n",
                            ePeciStatus, i);
                    crashdumpJsonICX1(u32CoreNum, u32ThreadNum,
                                      u32CrashdumpSize, (uint8_t*)pu64Crashdump,
                                      pJsonChild, cc);
                    FREE(pu64Crashdump);
                    goto nextCore;
                }

                if (PECI_CC_SKIP_SOCKET(cc))
                {
                    fprintf(stderr, "Error (%d) during GetFrame (num:%d)\n",
                            ePeciStatus, i);
                    crashdumpJsonICX1(u32CoreNum, u32ThreadNum,
                                      u32CrashdumpSize, (uint8_t*)pu64Crashdump,
                                      pJsonChild, cc);
                    FREE(pu64Crashdump);
                    return 0;
                }

                if (ePeciStatus != PECI_CC_SUCCESS)
                {
                    fprintf(stderr, "Error (%d) during GetFrame (num:%d)\n",
                            ePeciStatus, i);
                    crashdumpJsonICX1(u32CoreNum, u32ThreadNum,
                                      u32CrashdumpSize, (uint8_t*)pu64Crashdump,
                                      pJsonChild, cc);
                    FREE(pu64Crashdump);
                    goto nextCore;
                }
            }
            // Check if this core is multi-threaded, if available
            if (u32CrashdumpSize >= (CD_WHO_MISC_OFFSET + 1) * sizeof(uint64_t))
            {
                UCrashdumpWhoMisc uCrashdumpWhoMisc;
                uCrashdumpWhoMisc.raw = pu64Crashdump[CD_WHO_MISC_OFFSET];
                if (uCrashdumpWhoMisc.field.multithread)
                {
                    u32Threads = CD_MT_THREADS_PER_CORE;
                }
            }
            // Log this Crashdump
            crashdumpJsonICX1(u32CoreNum, u32ThreadNum, u32CrashdumpSize,
                              (uint8_t*)pu64Crashdump, pJsonChild, cc);
            FREE(pu64Crashdump);
        }
    nextCore:;
    }
    return ret;
}

static const SCrashdumpVx sCrashdumpVx[] = {
    {clx, logCrashdumpCPX1}, {clx2, logCrashdumpCPX1}, {cpx, logCrashdumpCPX1},
    {skx, logCrashdumpCPX1}, {icx, logCrashdumpICX1},
};

/******************************************************************************
 *
 *  logCrashdump
 *
 *  BMC performs the crashdump retrieve from the processor directly via
 *  PECI interface for internal state of the cores and Cbos after a platform
 *  three (3) strike failure. The Crash Dump from CPU will be empty (size 0)
 *  if no cores qualify to be dumped. A core will not be dumped if the power
 *  state is such that it cannot be accessed. A core will be dumped only if
 *  it has experienced a "3-strike" Machine Check Error.
 *
 ******************************************************************************/
int logCrashdump(crashdump::CPUInfo& cpuInfo, cJSON* pJsonChild)
{
    if (pJsonChild == NULL)
    {
        return 1;
    }

    for (uint32_t i = 0; i < (sizeof(sCrashdumpVx) / sizeof(SCrashdumpVx)); i++)
    {
        if (cpuInfo.model == sCrashdumpVx[i].cpuModel)
        {
            return sCrashdumpVx[i].logCrashdumpVx(cpuInfo, pJsonChild);
        }
    }

    fprintf(stderr, "Cannot find version for %s\n", __FUNCTION__);
    return 1;
}
