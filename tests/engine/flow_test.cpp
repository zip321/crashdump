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

class FlowTestFixture : public ::testing::Test
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

fs::path FlowTestFixture::UTFile;
cJSON* FlowTestFixture::inRoot;

TEST_F(FlowTestFixture, fillNewSectionSingleSectionTest)
{
    TestCrashdump crashdump(cpusInfo);
    cpusInfo[0].inputFile.bufferPtr = inRoot;
    uint8_t busValid[4] = {0x3f, 0x0f, 0x00, 0xc0}; // 0xc0000f3f;
    uint8_t busno7[4] = {0x00, 0x00, 0x7e, 0x7f};   // 0x7f7e0000;

    EXPECT_CALL(*crashdump.libPeciMock, peci_RdEndPointConfigPciLocal)
        .WillOnce(DoAll(SetArrayArgument<7>(busValid, busValid + 4),
                        SetArgPointee<8>(0x40), Return(PECI_CC_SUCCESS)))
        .WillOnce(DoAll(SetArrayArgument<7>(busno7, busno7 + 4),
                        SetArgPointee<8>(0x40), Return(PECI_CC_SUCCESS)))
        .WillOnce(DoAll(SetArrayArgument<7>(busValid, busValid + 4),
                        SetArgPointee<8>(0x40), Return(PECI_CC_SUCCESS)))
        .WillOnce(DoAll(SetArrayArgument<7>(busno7, busno7 + 4),
                        SetArgPointee<8>(0x40), Return(PECI_CC_SUCCESS)))
        .WillRepeatedly(DoAll(SetArrayArgument<7>(busValid, busValid + 4),
                              SetArgPointee<8>(0x40), Return(PECI_CC_SUCCESS)));
    EXPECT_CALL(*crashdump.libPeciMock, peci_RdIAMSR)
        .WillRepeatedly(DoAll(SetArgPointee<3>(0x1122334455667788),
                              SetArgPointee<4>(0x40), Return(PECI_CC_SUCCESS)));
    EXPECT_CALL(*crashdump.libPeciMock, peci_RdEndPointConfigMmio)
        .WillRepeatedly(
            DoAll(SetArgPointee<10>(0x40), Return(PECI_CC_SUCCESS)));

    struct timespec sectionStart;
    clock_gettime(CLOCK_MONOTONIC, &sectionStart);
    for (int cpu = 0; cpu < (int)cpusInfo.size(); cpu++)
    {
        sleep(1);
        status = fillNewSection(outRoot, &cpusInfo[cpu], cpu, "Uncore",
                                &sectionStart, "_time:");
    }
    cJSON* uncore = cJSON_GetObjectItem(
        cJSON_GetObjectItem(
            cJSON_GetObjectItem(cJSON_GetObjectItem(outRoot, "crash_data"),
                                "PROCESSORS"),
            "cpu0"),
        "uncore");
    cJSON* version = cJSON_GetObjectItem(uncore, "_version");
    if (version != NULL)
    {
        EXPECT_STREQ(version->valuestring, "0x801c016");
    }
    cJSON* time = cJSON_GetObjectItem(uncore, "_time:");

    if (time != NULL)
    {
        EXPECT_STREQ(time->valuestring, "1.00s");
    }
}

TEST_F(FlowTestFixture, fillNewSectionDoubleSectionTest)
{
    TestCrashdump crashdump(cpusInfo);
    cpusInfo[0].chaCount = 2;
    cpusInfo[0].inputFile.bufferPtr = inRoot;
    uint8_t busValid[4] = {0x3f, 0x0f, 0x00, 0xc0}; // 0xc0000f3f;
    uint8_t busno7[4] = {0x00, 0x00, 0x7e, 0x7f};   // 0x7f7e0000;

    EXPECT_CALL(*crashdump.libPeciMock, peci_RdEndPointConfigPciLocal)
        .WillOnce(DoAll(SetArrayArgument<7>(busValid, busValid + 4),
                        SetArgPointee<8>(0x40), Return(PECI_CC_SUCCESS)))
        .WillOnce(DoAll(SetArrayArgument<7>(busno7, busno7 + 4),
                        SetArgPointee<8>(0x40), Return(PECI_CC_SUCCESS)))
        .WillOnce(DoAll(SetArrayArgument<7>(busValid, busValid + 4),
                        SetArgPointee<8>(0x40), Return(PECI_CC_SUCCESS)))
        .WillOnce(DoAll(SetArrayArgument<7>(busno7, busno7 + 4),
                        SetArgPointee<8>(0x40), Return(PECI_CC_SUCCESS)))
        .WillRepeatedly(DoAll(SetArrayArgument<7>(busValid, busValid + 4),
                              SetArgPointee<8>(0x40), Return(PECI_CC_SUCCESS)));
    EXPECT_CALL(*crashdump.libPeciMock, peci_RdIAMSR)
        .WillRepeatedly(DoAll(SetArgPointee<3>(0x1122334455667788),
                              SetArgPointee<4>(0x40), Return(PECI_CC_SUCCESS)));
    EXPECT_CALL(*crashdump.libPeciMock, peci_RdEndPointConfigMmio)
        .WillRepeatedly(
            DoAll(SetArgPointee<10>(0x40), Return(PECI_CC_SUCCESS)));

    struct timespec sectionStart;
    clock_gettime(CLOCK_MONOTONIC, &sectionStart);
    for (int cpu = 0; cpu < (int)cpusInfo.size(); cpu++)
    {
        sleep(1);
        status = fillNewSection(outRoot, &cpusInfo[cpu], cpu, "Uncore",
                                &sectionStart, "_time:");
        sleep(1);
        status = fillNewSection(outRoot, &cpusInfo[cpu], cpu,
                                "Uncore_RdIAMSR_CHA", &sectionStart, "_time:");
    }

    cJSON* uncore = cJSON_GetObjectItem(
        cJSON_GetObjectItem(
            cJSON_GetObjectItem(cJSON_GetObjectItem(outRoot, "crash_data"),
                                "PROCESSORS"),
            "cpu0"),
        "uncore");
    cJSON* time = cJSON_GetObjectItem(uncore, "_time:");
    if (time != NULL)
    {
        EXPECT_STREQ(time->valuestring, "2.00s");
    }
}

TEST_F(FlowTestFixture, fillNewSectionBadCCNodeTest)
{
    TestCrashdump crashdump(cpusInfo);
    cpusInfo[0].chaCount = 2;
    cpusInfo[0].inputFile.bufferPtr = inRoot;

    EXPECT_CALL(*crashdump.libPeciMock, peci_RdIAMSR)
        .WillOnce(DoAll(SetArgPointee<3>(0x1122334455667788),
                        SetArgPointee<4>(0x40), Return(PECI_CC_SUCCESS)))
        .WillOnce(DoAll(SetArgPointee<3>(0x1122334455667788),
                        SetArgPointee<4>(0x90),
                        Return(PECI_CC_SUCCESS))) // ERROR
        .WillOnce(DoAll(SetArgPointee<3>(0x1122334455667788),
                        SetArgPointee<4>(0x40), Return(PECI_CC_SUCCESS)))
        .WillOnce(DoAll(SetArgPointee<3>(0x1122334455667788),
                        SetArgPointee<4>(0x40), Return(PECI_CC_SUCCESS)))
        .WillOnce(DoAll(SetArgPointee<3>(0x1122334455667788),
                        SetArgPointee<4>(0x40), Return(PECI_CC_SUCCESS)))
        .WillOnce(DoAll(SetArgPointee<3>(0x1122334455667788),
                        SetArgPointee<4>(0x40), Return(PECI_CC_SUCCESS)))
        .WillOnce(DoAll(SetArgPointee<3>(0x1122334455667788),
                        SetArgPointee<4>(0x40), Return(PECI_CC_SUCCESS)));

    struct timespec sectionStart;
    clock_gettime(CLOCK_MONOTONIC, &sectionStart);
    for (int cpu = 0; cpu < (int)cpusInfo.size(); cpu++)
    {
        status = fillNewSection(outRoot, &cpusInfo[cpu], cpu,
                                "Uncore_RdIAMSR_CHA", &sectionStart, "_time:");
    }
    cJSON* uncore = cJSON_GetObjectItem(
        cJSON_GetObjectItem(
            cJSON_GetObjectItem(cJSON_GetObjectItem(outRoot, "crash_data"),
                                "PROCESSORS"),
            "cpu0"),
        "uncore");
    cJSON* errorTransaction =
        cJSON_GetObjectItem(uncore, "RDIAMSR_cha0_0x8002");
    if (errorTransaction != NULL)
    {
        EXPECT_STREQ(errorTransaction->valuestring,
                     "0x1122334455667788,CC:0x90,RC:0x0");
    }
    cJSON* naTransaction = cJSON_GetObjectItem(uncore, "RDIAMSR_cha0_0x8003");
    if (naTransaction != NULL)
    {
        EXPECT_STREQ(naTransaction->valuestring, "N/A");
    }
    cJSON* postNodeTransaction =
        cJSON_GetObjectItem(uncore, "RDIAMSR_cha1_0x8000");
    if (postNodeTransaction != NULL)
    {
        EXPECT_STREQ(postNodeTransaction->valuestring, "0x1122334455667788");
    }
}