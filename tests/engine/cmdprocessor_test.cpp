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

#include "../mock/test_crashdump.hpp"
#include "../test_utils.hpp"

extern "C" {
#include <search.h>

#include "CrashdumpSections/crashdump.h"
#include "CrashdumpSections/utils.h"
#include "engine/cmdprocessor.h"
#include "engine/flow.h"
#include "engine/inputparser.h"
#include "engine/logger.h"
}

#include "gmock/gmock.h"
#include "gtest/gtest.h"

using namespace ::testing;
using std::filesystem::current_path;
using ::testing::Return;
#include <fstream>
namespace fs = std::filesystem;

class ProcessCmdTestFixture : public ::testing::Test
{
  protected:
    static cJSON* inRoot;
    static fs::path UTFile;
    static void readUncoreSampleFile()
    {
        UTFile = fs::current_path();
        UTFile = UTFile.parent_path();
        UTFile /= "tests/UnitTestFiles/ut_uncore_input_sample_spr.json";
        inRoot = readInputFile(UTFile.c_str());
        if (inRoot == NULL)
        {
            EXPECT_TRUE(inRoot != NULL);
        }
    }
    static void SetUpTestSuite()
    {
        readUncoreSampleFile();
    }

    static void TearDownTestSuite()
    {
        cJSON_Delete(inRoot);
    }

    void SetUp() override
    {
        // Build a list of cpus
        CPUInfo cpuInfo = {};
        cpuInfo.model = cd_spr;
        cpuInfo.clientAddr = 0x30;
        cpusInfo.push_back(cpuInfo);
        cpuInfo.model = cd_spr;
        cpuInfo.clientAddr = 0x31;
        cpusInfo.push_back(cpuInfo);
    }

    void TearDown() override
    {
        FREE(jsonStr);
        cJSON_Delete(outRoot);
    }

  public:
    cJSON* outRoot = cJSON_CreateObject();
    char* jsonStr = NULL;
    uint8_t cc = 0;
    bool enable = false;
    acdStatus status;
    std::vector<CPUInfo> cpusInfo;
};

fs::path ProcessCmdTestFixture::UTFile;
cJSON* ProcessCmdTestFixture::inRoot;

TEST_F(ProcessCmdTestFixture, SimpleCmdPassFailTest)
{
    TestCrashdump crashdump(cpusInfo);

    // Build RdIAMSR params in cJSON form
    cJSON* params = cJSON_CreateObject();
    int paramsRdIAMSR[] = {0x30, 0x0, 0x435};
    cJSON_AddItemToObject(params, "Params",
                          cJSON_CreateIntArray(paramsRdIAMSR, 3));
    uint8_t cc = PECI_DEV_CC_SUCCESS;
    EXPECT_CALL(*crashdump.libPeciMock, peci_RdIAMSR)
        .Times(1)
        .WillRepeatedly(DoAll(SetArgPointee<3>(0x1122334455667788),
                              SetArgPointee<4>(cc), Return(PECI_CC_SUCCESS)));

    ENTRY entry;
    CmdInOut cmdInOut = {};
    cmdInOut.out.ret = PECI_CC_INVALID_REQ;
    cmdInOut.in.params = cJSON_GetObjectItem(params, "Params");
    cmdInOut.internalVarName = "UINT64Var";

    status = BuildCmdsTable(&entry);
    EXPECT_EQ(ACD_SUCCESS, status);

    entry.key = "RdIAMSR";
    cmdInOut.paramsTracker = cJSON_CreateObject();
    cmdInOut.internalVarsTracker = cJSON_CreateObject();
    status = Execute(&entry, &cmdInOut);
    EXPECT_EQ(ACD_SUCCESS, status);
    EXPECT_EQ(cmdInOut.out.ret, PECI_CC_SUCCESS);
    EXPECT_EQ(cmdInOut.out.cc, cc);
    EXPECT_EQ((uint64_t)cmdInOut.out.val.u64, (uint64_t)0x1122334455667788);

    entry.key = "RdIAMSRInvalid";
    status = Execute(&entry, &cmdInOut);
    EXPECT_EQ(ACD_INVALID_CMD, status);

    Logging(cmdInOut.internalVarsTracker, "internalVarsTracker:");
    cJSON* val = cJSON_GetObjectItem(cmdInOut.internalVarsTracker, "UINT64Var");
    EXPECT_STREQ(val->valuestring, "0x1122334455667788");
    hdestroy();
}

TEST_F(ProcessCmdTestFixture, InputFileUncoreFullTest)
{
    TestCrashdump crashdump(cpusInfo);
    uint8_t cc = PECI_DEV_CC_SUCCESS;
    ENTRY entry;
    LoggerStruct loggerStruct;
    status = BuildCmdsTable(&entry);
    EXPECT_EQ(ACD_SUCCESS, status);
    struct timespec sectionStart;
    uint32_t maxTime = 0xFFFFFFFF;
    uint8_t threadsPerCore = 1;
    // Notes: peci_cmds values for post enum bus registers
    // peci_cmds RdEndpointConfigPCILocal 0 8 3 0 0x1a0
    //    cc:0x40 0xc0000f3f
    // peci_cmds -a 0x30 RdEndpointConfigPCILocal 0 8 3 0 0x1cc
    //    cc:0x40 0x7f7e0000

    uint8_t busValid[4] = {0x3f, 0x0f, 0x00, 0xc0}; // 0xc0000f3f;
    uint8_t busno7[4] = {0x00, 0x00, 0x7e, 0x7f};   // 0x7f7e0000;

    EXPECT_CALL(*crashdump.libPeciMock, peci_RdEndPointConfigPciLocal)
        .WillOnce(DoAll(SetArrayArgument<7>(busValid, busValid + 4),
                        SetArgPointee<8>(cc), Return(PECI_CC_SUCCESS)))
        .WillOnce(DoAll(SetArrayArgument<7>(busno7, busno7 + 4),
                        SetArgPointee<8>(cc), Return(PECI_CC_SUCCESS)))
        .WillOnce(DoAll(SetArrayArgument<7>(busValid, busValid + 4),
                        SetArgPointee<8>(cc), Return(PECI_CC_SUCCESS)))
        .WillOnce(DoAll(SetArrayArgument<7>(busno7, busno7 + 4),
                        SetArgPointee<8>(cc), Return(PECI_CC_SUCCESS)))
        .WillRepeatedly(DoAll(SetArrayArgument<7>(busValid, busValid + 4),
                              SetArgPointee<8>(cc), Return(PECI_CC_SUCCESS)));
    EXPECT_CALL(*crashdump.libPeciMock, peci_RdIAMSR)
        .WillRepeatedly(DoAll(SetArgPointee<3>(0x1122334455667788),
                              SetArgPointee<4>(cc), Return(PECI_CC_SUCCESS)));
    EXPECT_CALL(*crashdump.libPeciMock, peci_RdEndPointConfigMmio)
        .WillRepeatedly(DoAll(SetArgPointee<10>(cc), Return(PECI_CC_SUCCESS)));

    loggerStruct.contextLogger.skipOnFailFromInputFile = false;
    InputParserErrInfo errInfo = {};
    errInfo.sectionName = "Uncore";
    cJSON* sections = getCrashDataSection(inRoot, "Sections", &enable);
    cJSON* section = NULL;

    cJSON_ArrayForEach(section, sections)
    {
        CmdInOut cmdInOut = {};
        LoopOnFlags loopOnFlags;
        cmdInOut.out.ret = PECI_CC_INVALID_REQ;
        CmdInOut preReqCmdInOut = {};
        preReqCmdInOut.out.ret = PECI_CC_INVALID_REQ;
        ReadLoops(cJSON_GetObjectItemCaseSensitive(section, "Uncore"),
                  &loopOnFlags);
        GenerateVersion(cJSON_GetObjectItemCaseSensitive(section, "Uncore"),
                        &loggerStruct.contextLogger.version);
        GetPath(cJSON_GetObjectItemCaseSensitive(section, "Uncore"),
                &loggerStruct);

        EXPECT_EQ(ACD_SUCCESS, status);
        status = ParsePath(&loggerStruct);
        EXPECT_EQ(ACD_SUCCESS, status);
        cJSON* maxTimeJson = cJSON_GetObjectItemCaseSensitive(
            cJSON_GetObjectItemCaseSensitive(section, "Uncore"), "MaxTimeSec");
        if (maxTimeJson != NULL)
        {
            maxTime = maxTimeJson->valueint;
        }
        cJSON* skipOnErrorJson = cJSON_GetObjectItemCaseSensitive(
            cJSON_GetObjectItemCaseSensitive(section, "Uncore"), "SkipOnFail");
        if (skipOnErrorJson != NULL)
        {
            loggerStruct.contextLogger.skipOnFailFromInputFile =
                cJSON_IsTrue(skipOnErrorJson);
        }
        clock_gettime(CLOCK_MONOTONIC, &sectionStart);
        cJSON* preReq = cJSON_GetObjectItemCaseSensitive(
            cJSON_GetObjectItemCaseSensitive(
                cJSON_GetObjectItemCaseSensitive(section, "Uncore"), "PreReq"),
            "PECICmds");
        cJSON* peciCmds = cJSON_GetObjectItemCaseSensitive(
            cJSON_GetObjectItemCaseSensitive(section, "Uncore"), "PECICmds");
        for (int cpu = 0; cpu < (int)cpusInfo.size(); cpu++)
        {
            loggerStruct.contextLogger.cpu = cpu;
            logVersion(&cmdInOut, outRoot, &loggerStruct);
            loggerStruct.contextLogger.skipFlag = false;
            // Process Uncore PreReq PECICmds section
            preReqCmdInOut.internalVarsTracker = cJSON_CreateObject();
            ProcessPECICmds(&entry, &cpusInfo[cpu], preReq, &preReqCmdInOut,
                            &errInfo, &loggerStruct, outRoot, &sectionStart,
                            &maxTime);

            // Make PreReq internalVars visible to Sections->PECICmds
            cmdInOut.internalVarsTracker = preReqCmdInOut.internalVarsTracker;

            if (loopOnFlags.loopOnCHA)
            {
                for (size_t cha = 0; cha < cpusInfo[cpu].chaCount; cha++)
                {
                    loggerStruct.contextLogger.cha = (uint8_t)cha;
                    ProcessPECICmds(&entry, &cpusInfo[cpu], peciCmds, &cmdInOut,
                                    &errInfo, &loggerStruct, outRoot,
                                    &sectionStart, &maxTime);
                    loggerStruct.contextLogger.skipFlag = false;
                }
            }
            else if (loopOnFlags.loopOnCore)
            {
                for (uint32_t u32CoreNum = 0;
                     (cpusInfo[cpu].coreMask >> u32CoreNum) != 0; u32CoreNum++)
                {
                    if (!CHECK_BIT(cpusInfo[cpu].coreMask, u32CoreNum))
                    {
                        continue;
                    }
                    loggerStruct.contextLogger.core = (uint8_t)u32CoreNum;
                    if (loopOnFlags.loopOnThread)
                    {
                        threadsPerCore = 2;
                    }
                    for (uint8_t threadNum = 0; threadNum < threadsPerCore;
                         threadNum++)
                    {
                        loggerStruct.contextLogger.thread = threadNum;
                        ProcessPECICmds(&entry, &cpusInfo[cpu], peciCmds,
                                        &cmdInOut, &errInfo, &loggerStruct,
                                        outRoot, &sectionStart, &maxTime);
                        loggerStruct.contextLogger.skipFlag = false;
                    }
                }
            }
            else
            {
                ProcessPECICmds(&entry, &cpusInfo[cpu], peciCmds, &cmdInOut,
                                &errInfo, &loggerStruct, outRoot, &sectionStart,
                                &maxTime);
                loggerStruct.contextLogger.skipFlag = false;
            }
            logSectionRunTime(loggerStruct.nameProcessing.lastLevel,
                              &sectionStart, "_time");
            cJSON_Delete(cmdInOut.internalVarsTracker);
        }
    }
    hdestroy();
    Logging(outRoot, "outRoot:");
}
