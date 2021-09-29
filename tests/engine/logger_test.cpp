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

extern "C" {
#include "CrashdumpSections/crashdump.h"
#include "CrashdumpSections/utils.h"
#include "engine/logger.h"
}

#include "gmock/gmock.h"
#include "gtest/gtest.h"

using namespace ::testing;
using std::filesystem::current_path;
using ::testing::Return;
#include <fstream>
namespace fs = std::filesystem;

class ProcessLoggerTestFixture : public ::testing::Test
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

fs::path ProcessLoggerTestFixture::UTFile;
cJSON* ProcessLoggerTestFixture::inRoot;

TEST_F(ProcessLoggerTestFixture, OutputPathTest)
{
    LoggerStruct loggerStruct;
    static const char* OutputStr =
        R"({"OutputPath":"crash_data/PROCESSORS/cpu%d/uncore"})";
    cJSON* OutputPathjson = cJSON_Parse(OutputStr);
    status = GetPath(OutputPathjson, &loggerStruct);
    EXPECT_EQ(ACD_SUCCESS, status);
    status = ParsePath(&loggerStruct);
    EXPECT_EQ(ACD_SUCCESS, status);
    EXPECT_EQ(loggerStruct.pathParsing.numberOfTokens, 4);
    EXPECT_STREQ(loggerStruct.pathParsing.pathLevelToken[0], "crash_data");
    EXPECT_STREQ(loggerStruct.pathParsing.pathLevelToken[1], "PROCESSORS");
    EXPECT_STREQ(loggerStruct.pathParsing.pathLevelToken[2], "cpu%d");
    EXPECT_STREQ(loggerStruct.pathParsing.pathLevelToken[3], "uncore");
}

TEST_F(ProcessLoggerTestFixture, LoggerCpuCycleTest)
{
    static const char* OutputPathStr = R"({"OutputPath":"cpu%d"})";
    cJSON* OutputPathjson = cJSON_Parse(OutputPathStr);
    static const char* OutputStr =
        R"({"Size":"4", "Name":["RegisterName", "SectionName"]})";
    cJSON* Outputjson = cJSON_Parse(OutputStr);
    LoggerStruct loggerStruct;
    CmdInOut cmdInOut;
    cmdInOut.in.outputPath = Outputjson;
    status = GetPath(OutputPathjson, &loggerStruct);
    EXPECT_EQ(ACD_SUCCESS, status);
    status = ParsePath(&loggerStruct);
    EXPECT_EQ(ACD_SUCCESS, status);
    status = ParseNameSection(&cmdInOut, &loggerStruct);
    EXPECT_EQ(ACD_SUCCESS, status);
    cmdInOut.out.ret = PECI_CC_SUCCESS;
    cmdInOut.out.cc = PECI_DEV_CC_SUCCESS;
    loggerStruct.nameProcessing.size = 4;
    loggerStruct.contextLogger.skipFlag = false;
    cmdInOut.out.val.u64 = 0xdeadbeef;
    for (int cpu = 0; cpu < 2; cpu++)
    {
        loggerStruct.contextLogger.cpu = cpu;
        status = Logger(&cmdInOut, outRoot, &loggerStruct);
    }
    EXPECT_EQ(ACD_SUCCESS, status);
    cJSON* Object1 = cJSON_GetObjectItemCaseSensitive(outRoot, "cpu0");
    EXPECT_FALSE(Object1 == NULL);
    cJSON* Object2 = cJSON_GetObjectItemCaseSensitive(outRoot, "cpu1");
    EXPECT_FALSE(Object2 == NULL);
}

TEST_F(ProcessLoggerTestFixture, LoggerChaCycleTest)
{
    static const char* OutputPathStr = R"({"OutputPath":"cbo%d"})";
    cJSON* OutputPathjson = cJSON_Parse(OutputPathStr);
    static const char* OutputStr =
        R"({"Size":"4", "Name":["RegisterName", "SectionName"]})";
    cJSON* Outputjson = cJSON_Parse(OutputStr);
    LoggerStruct loggerStruct;
    CmdInOut cmdInOut;
    cmdInOut.in.outputPath = Outputjson;
    status = GetPath(OutputPathjson, &loggerStruct);
    EXPECT_EQ(ACD_SUCCESS, status);
    status = ParsePath(&loggerStruct);
    EXPECT_EQ(ACD_SUCCESS, status);
    status = ParseNameSection(&cmdInOut, &loggerStruct);
    EXPECT_EQ(ACD_SUCCESS, status);
    cmdInOut.out.ret = PECI_CC_SUCCESS;
    cmdInOut.out.cc = PECI_DEV_CC_SUCCESS;
    loggerStruct.nameProcessing.size = 4;
    loggerStruct.contextLogger.skipFlag = false;
    cmdInOut.out.val.u64 = 0xdeadbeef;
    for (int cha = 0; cha < 2; cha++)
    {
        loggerStruct.contextLogger.cha = cha;
        status = Logger(&cmdInOut, outRoot, &loggerStruct);
    }
    EXPECT_EQ(ACD_SUCCESS, status);
    cJSON* Object1 = cJSON_GetObjectItemCaseSensitive(outRoot, "cbo0");
    EXPECT_FALSE(Object1 == NULL);
    cJSON* Object2 = cJSON_GetObjectItemCaseSensitive(outRoot, "cbo1");
    EXPECT_FALSE(Object2 == NULL);
}

TEST_F(ProcessLoggerTestFixture, LoggerCoreCycleTest)
{
    static const char* OutputPathStr = R"({"OutputPath":"core%d"})";
    cJSON* OutputPathjson = cJSON_Parse(OutputPathStr);
    static const char* OutputStr =
        R"({"Size":"4", "Name":["RegisterName", "SectionName"]})";
    cJSON* Outputjson = cJSON_Parse(OutputStr);
    LoggerStruct loggerStruct;
    CmdInOut cmdInOut;
    cmdInOut.in.outputPath = Outputjson;
    status = GetPath(OutputPathjson, &loggerStruct);
    EXPECT_EQ(ACD_SUCCESS, status);
    status = ParsePath(&loggerStruct);
    EXPECT_EQ(ACD_SUCCESS, status);
    status = ParseNameSection(&cmdInOut, &loggerStruct);
    EXPECT_EQ(ACD_SUCCESS, status);
    cmdInOut.out.ret = PECI_CC_SUCCESS;
    cmdInOut.out.cc = PECI_DEV_CC_SUCCESS;
    loggerStruct.nameProcessing.size = 4;
    loggerStruct.contextLogger.skipFlag = false;
    cmdInOut.out.val.u64 = 0xdeadbeef;
    for (int core = 0; core < 2; core++)
    {
        loggerStruct.contextLogger.core = core;
        status = Logger(&cmdInOut, outRoot, &loggerStruct);
    }
    EXPECT_EQ(ACD_SUCCESS, status);
    cJSON* Object1 = cJSON_GetObjectItemCaseSensitive(outRoot, "core0");
    EXPECT_FALSE(Object1 == NULL);
    cJSON* Object2 = cJSON_GetObjectItemCaseSensitive(outRoot, "core1");
    EXPECT_FALSE(Object2 == NULL);
}

TEST_F(ProcessLoggerTestFixture, LoggerThreadCycleTest)
{
    static const char* OutputPathStr = R"({"OutputPath":"thread%d"})";
    cJSON* OutputPathjson = cJSON_Parse(OutputPathStr);
    static const char* OutputStr =
        R"({"Size":"4", "Name":["RegisterName", "SectionName"]})";
    cJSON* Outputjson = cJSON_Parse(OutputStr);
    LoggerStruct loggerStruct;
    CmdInOut cmdInOut;
    cmdInOut.in.outputPath = Outputjson;
    status = GetPath(OutputPathjson, &loggerStruct);
    EXPECT_EQ(ACD_SUCCESS, status);
    status = ParsePath(&loggerStruct);
    EXPECT_EQ(ACD_SUCCESS, status);
    status = ParseNameSection(&cmdInOut, &loggerStruct);
    EXPECT_EQ(ACD_SUCCESS, status);
    cmdInOut.out.ret = PECI_CC_SUCCESS;
    cmdInOut.out.cc = PECI_DEV_CC_SUCCESS;
    loggerStruct.contextLogger.skipFlag = false;
    cmdInOut.out.val.u64 = 0xdeadbeef;
    for (int thread = 0; thread < 2; thread++)
    {
        loggerStruct.contextLogger.thread = thread;
        status = Logger(&cmdInOut, outRoot, &loggerStruct);
    }
    EXPECT_EQ(ACD_SUCCESS, status);
    cJSON* Object1 = cJSON_GetObjectItemCaseSensitive(outRoot, "thread0");
    EXPECT_FALSE(Object1 == NULL);
    cJSON* Object2 = cJSON_GetObjectItemCaseSensitive(outRoot, "thread1");
    EXPECT_FALSE(Object2 == NULL);
}

TEST_F(ProcessLoggerTestFixture, LoggerPrintSkip)
{
    LoggerStruct loggerStruct;
    CmdInOut cmdInOut;
    loggerStruct.pathParsing.numberOfTokens = 0;
    loggerStruct.nameProcessing.extraLevel = false;
    loggerStruct.nameProcessing.registerName = "RegisterName";
    cmdInOut.out.ret = PECI_CC_DRIVER_ERR;
    cmdInOut.out.cc = PECI_DEV_CC_MCA_ERROR;
    loggerStruct.nameProcessing.size = 4;
    loggerStruct.nameProcessing.sizeFromOutput = true;
    loggerStruct.contextLogger.skipFlag = true;
    cmdInOut.out.val.u64 = 0xdeadbeef;
    status = Logger(&cmdInOut, outRoot, &loggerStruct);
    EXPECT_EQ(ACD_SUCCESS, status);
    cJSON* Object1 = cJSON_GetObjectItemCaseSensitive(outRoot, "RegisterName");
    EXPECT_FALSE(Object1 == NULL);
    EXPECT_STREQ(Object1->valuestring, "N/A");
}

TEST_F(ProcessLoggerTestFixture, LoggerPrintBadRet)
{
    LoggerStruct loggerStruct;
    CmdInOut cmdInOut;
    loggerStruct.pathParsing.numberOfTokens = 0;
    loggerStruct.nameProcessing.extraLevel = false;
    loggerStruct.nameProcessing.registerName = "RegisterName";
    cmdInOut.out.ret = PECI_CC_DRIVER_ERR;
    cmdInOut.out.cc = PECI_DEV_CC_MCA_ERROR;
    loggerStruct.nameProcessing.size = 4;
    loggerStruct.nameProcessing.sizeFromOutput = true;
    loggerStruct.contextLogger.skipFlag = false;
    cmdInOut.out.val.u64 = 0xdeadbeef;
    status = Logger(&cmdInOut, outRoot, &loggerStruct);
    EXPECT_EQ(ACD_SUCCESS, status);
    cJSON* Object1 = cJSON_GetObjectItemCaseSensitive(outRoot, "RegisterName");
    EXPECT_FALSE(Object1 == NULL);
    EXPECT_STREQ(Object1->valuestring, "0x0,CC:0x91,RC:0x3");
}

TEST_F(ProcessLoggerTestFixture, LoggerBadCC)
{
    LoggerStruct loggerStruct;
    CmdInOut cmdInOut;
    loggerStruct.pathParsing.numberOfTokens = 0;
    loggerStruct.nameProcessing.extraLevel = false;
    loggerStruct.nameProcessing.registerName = "RegisterName";
    cmdInOut.out.ret = PECI_CC_SUCCESS;
    cmdInOut.out.cc = PECI_DEV_CC_INVALID_REQ;
    loggerStruct.nameProcessing.size = 4;
    loggerStruct.nameProcessing.sizeFromOutput = true;
    loggerStruct.contextLogger.skipFlag = false;
    cmdInOut.out.val.u64 = 0xdeadbeefdeadbeef;
    status = Logger(&cmdInOut, outRoot, &loggerStruct);
    EXPECT_EQ(ACD_SUCCESS, status);
    cJSON* Object1 = cJSON_GetObjectItemCaseSensitive(outRoot, "RegisterName");
    EXPECT_FALSE(Object1 == NULL);
    EXPECT_STREQ(Object1->valuestring, "0xdeadbeef,CC:0x90,RC:0x0");
}

TEST_F(ProcessLoggerTestFixture, LoggerGoodSize)
{
    LoggerStruct loggerStruct;
    CmdInOut cmdInOut;
    loggerStruct.pathParsing.numberOfTokens = 0;
    loggerStruct.nameProcessing.extraLevel = false;
    cmdInOut.out.ret = PECI_CC_SUCCESS;
    cmdInOut.out.cc = PECI_DEV_CC_SUCCESS;
    loggerStruct.nameProcessing.sizeFromOutput = true;
    loggerStruct.contextLogger.skipFlag = false;
    loggerStruct.nameProcessing.registerName = "RegisterName32";
    loggerStruct.nameProcessing.size = 4;
    cmdInOut.out.val.u64 = 0xdeadbeef;
    status = Logger(&cmdInOut, outRoot, &loggerStruct);
    EXPECT_EQ(ACD_SUCCESS, status);
    cJSON* Object1 =
        cJSON_GetObjectItemCaseSensitive(outRoot, "RegisterName32");
    EXPECT_FALSE(Object1 == NULL);
    EXPECT_STREQ(Object1->valuestring, "0xdeadbeef");
    loggerStruct.nameProcessing.registerName = "RegisterName64";
    cmdInOut.out.val.u64 = 0xdeadbeefdeadbeef;
    loggerStruct.nameProcessing.size = 8;
    status = Logger(&cmdInOut, outRoot, &loggerStruct);
    EXPECT_EQ(ACD_SUCCESS, status);
    cJSON* Object2 =
        cJSON_GetObjectItemCaseSensitive(outRoot, "RegisterName64");
    EXPECT_FALSE(Object2 == NULL);
    EXPECT_STREQ(Object2->valuestring, "0xdeadbeefdeadbeef");
    loggerStruct.nameProcessing.registerName = "RegisterName16";
    cmdInOut.out.val.u64 = 0xbeef;
    loggerStruct.nameProcessing.size = 2;
    status = Logger(&cmdInOut, outRoot, &loggerStruct);
    EXPECT_EQ(ACD_SUCCESS, status);
    cJSON* Object3 =
        cJSON_GetObjectItemCaseSensitive(outRoot, "RegisterName16");
    EXPECT_FALSE(Object3 == NULL);
    EXPECT_STREQ(Object3->valuestring, "0xbeef");
    loggerStruct.nameProcessing.registerName = "RegisterName8";
    cmdInOut.out.val.u64 = 0xef;
    loggerStruct.nameProcessing.size = 1;
    status = Logger(&cmdInOut, outRoot, &loggerStruct);
    EXPECT_EQ(ACD_SUCCESS, status);
    cJSON* Object4 = cJSON_GetObjectItemCaseSensitive(outRoot, "RegisterName8");
    EXPECT_FALSE(Object4 == NULL);
    EXPECT_STREQ(Object4->valuestring, "0xef");
}