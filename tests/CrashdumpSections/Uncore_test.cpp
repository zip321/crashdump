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
#include "../mock/test_crashdump.hpp"

extern "C" {
#include "CrashdumpSections/Uncore.h"
#include "CrashdumpSections/crashdump.h"
#include "CrashdumpSections/utils.h"
}

#include "gmock/gmock.h"
#include "gtest/gtest.h"

using namespace ::testing;
using ::testing::Return;

class UncoreTestFixture : public Test
{
  public:
    void SetUp() override
    {
        // Build a list of cpus
        info.model = cd_icx;
        cpus.push_back(info);
        info.model = cd_icx;
        cpus.push_back(info);

        // Initialize json object
        root = cJSON_CreateObject();
    }

    void TearDown() override
    {
        FREE(jsonStr);
        cJSON_Delete(root);
        cJSON_Delete(ut);
    }

    CPUInfo info = {};
    std::vector<CPUInfo> cpus;
    InputFileInfo inputFileInfo = {
        .unique = true, .filenames = {NULL}, .buffers = {NULL}};
    cJSON* root = NULL;
    cJSON* ut = NULL;
    char* jsonStr = NULL;
    char* defaultFile = NULL;
    char* overrideFile = NULL;
    bool isTelemetry = false;
};

TEST_F(UncoreTestFixture, DISABLED_getCrashDataSection)
{
    bool enable = false;
    inputFileInfo.buffers[0] = selectAndReadInputFile(
        cpus[0].model, inputFileInfo.filenames, isTelemetry);
    getCrashDataSection(inputFileInfo.buffers[0], "uncore", &enable);
    EXPECT_TRUE(enable);
}

TEST_F(UncoreTestFixture, DISABLED_getCrashDataSectionVersion)
{
    inputFileInfo.buffers[0] = selectAndReadInputFile(
        cpus[0].model, inputFileInfo.filenames, isTelemetry);
    int version =
        getCrashDataSectionVersion(inputFileInfo.buffers[0], "uncore");
    EXPECT_EQ(version, 0x24);
}

TEST(UncoreTest, logUncoreStatus_Invalid_Model)
{
    int ret;
    CPUInfo cpuInfo = {.clientAddr = 48,
                       .model = cd_numberOfModels, // invalid model#
                       .coreMask = 0x0000db7e,
                       .crashedCoreMask = 0x0,
                       .sectionMask = 0,
                       .chaCount = 0,
                       .initialPeciWake = ON,
                       .inputFile = {},
                       .cpuidRead = {},
                       .chaCountRead = {},
                       .coreMaskRead = {},
                       .launchDelay = {}};
    cJSON* root = cJSON_CreateObject();
    ret = logUncoreStatus(&cpuInfo, root);
    EXPECT_EQ(ret, ACD_FAILURE);
}

TEST(UncoreTest, logUncoreStatus_NULL_JsonPtr)
{
    int ret;

    TestCrashdump crashdump(cd_icx);
    for (auto cpuinfo : crashdump.cpusInfo)
    {
        ret = logUncoreStatus(&cpuinfo, NULL);
        EXPECT_EQ(ret, ACD_INVALID_OBJECT);
    }
}

TEST(UncoreTest, logCrashdumpVersion_validRecordType_SPR)
{
    CPUInfo cpuInfo = {.clientAddr = 48,
                       .model = cd_spr,
                       .coreMask = 0x0000db7e,
                       .crashedCoreMask = 0x0,
                       .sectionMask = 0,
                       .chaCount = 0,
                       .initialPeciWake = ON,
                       .inputFile = {},
                       .cpuidRead = {},
                       .chaCountRead = {},
                       .coreMaskRead = {},
                       .launchDelay = {}};
    std::vector<CPUInfo> cpusInfo = {cpuInfo};
    TestCrashdump crashdump(cpusInfo);

    revision_uncore = getCrashDataSectionVersion(
        crashdump.cpusInfo[0].inputFile.bufferPtr, "uncore");

    cJSON* uncore_root = cJSON_CreateObject();
    logCrashdumpVersion(uncore_root, &cpuInfo, RECORD_TYPE_UNCORESTATUSLOG);
    cJSON* actual = cJSON_GetObjectItemCaseSensitive(uncore_root, "_version");
    ASSERT_NE(actual, nullptr);
    EXPECT_STREQ(actual->valuestring, "0x801c009");
}

TEST(UncoreTest, logCrashdumpVersion_validRecordType_ICX)
{
    CPUInfo cpuInfo = {.clientAddr = 48,
                       .model = cd_icx,
                       .coreMask = 0x0000db7e,
                       .crashedCoreMask = 0x0,
                       .sectionMask = 0,
                       .chaCount = 0,
                       .initialPeciWake = ON,
                       .inputFile = {},
                       .cpuidRead = {},
                       .chaCountRead = {},
                       .coreMaskRead = {},
                       .launchDelay = {}};

    revision_uncore = 0xbb;

    cJSON* uncore_root = cJSON_CreateObject();
    logCrashdumpVersion(uncore_root, &cpuInfo, RECORD_TYPE_UNCORESTATUSLOG);
    cJSON* actual = cJSON_GetObjectItemCaseSensitive(uncore_root, "_version");
    ASSERT_NE(actual, nullptr);
    EXPECT_STREQ(actual->valuestring, "0x801a0bb");
}

TEST(UncoreTest, logUncoreStatus_fullFlow)
{
    int ret;
    CPUInfo cpuInfo = {.clientAddr = 48,
                       .model = cd_spr,
                       .coreMask = 0x0000db7e,
                       .crashedCoreMask = 0x0,
                       .sectionMask = 0,
                       .chaCount = 0,
                       .initialPeciWake = ON,
                       .inputFile = {},
                       .cpuidRead = {},
                       .chaCountRead = {},
                       .coreMaskRead = {},
                       .launchDelay = {}};

    std::vector<CPUInfo> cpusInfo = {cpuInfo};
    TestCrashdump crashdump(cpusInfo);

    EXPECT_CALL(*crashdump.libPeciMock, peci_Lock)
        .WillRepeatedly(Return(PECI_CC_SUCCESS));

    uint8_t returnValue[2] = {0x86, 0x80}; // 0x8086;
    uint8_t cc = 0x94;
    EXPECT_CALL(*crashdump.libPeciMock, peci_RdEndPointConfigPciLocal_seq)
        .WillRepeatedly(DoAll(SetArrayArgument<7>(returnValue, returnValue + 2),
                              SetArgPointee<9>(cc), Return(PECI_CC_SUCCESS)));

    uint8_t returnBusValidQuery[4] = {0x3f, 0x0f, 0x00,
                                      0xc0}; // 0xc0000f3f; bit 30 set
    uint8_t returnEnumeratedBus[4] = {0x00, 0x00, 0x7e, 0x7f}; // 0x7f7e0000;
    cc = 0x94;
    EXPECT_CALL(*crashdump.libPeciMock, peci_RdEndPointConfigPciLocal)
        .WillOnce(DoAll(
            SetArrayArgument<7>(returnBusValidQuery, returnBusValidQuery + 4),
            SetArgPointee<8>(cc), Return(PECI_CC_SUCCESS)))
        .WillOnce(DoAll(
            SetArrayArgument<7>(returnEnumeratedBus, returnEnumeratedBus + 4),
            SetArgPointee<8>(cc), Return(PECI_CC_SUCCESS)));

    for (auto cpuinfo : crashdump.cpusInfo)
    {
        ret = logUncoreStatus(&cpuinfo, crashdump.root);
        EXPECT_EQ(ret, ACD_SUCCESS);
    }
}

TEST(UncoreTest, logUncoreStatus_fullFlow_ICX)
{
    int ret;
    CPUInfo cpuInfo = {.clientAddr = 48,
                       .model = cd_icx,
                       .coreMask = 0x0000db7e,
                       .crashedCoreMask = 0x0,
                       .sectionMask = 0,
                       .chaCount = 0,
                       .initialPeciWake = ON,
                       .inputFile = {},
                       .cpuidRead = {},
                       .chaCountRead = {},
                       .coreMaskRead = {},
                       .launchDelay = {}};

    std::vector<CPUInfo> cpusInfo = {cpuInfo};
    TestCrashdump crashdump(cpusInfo);

    EXPECT_CALL(*crashdump.libPeciMock, peci_Lock)
        .WillRepeatedly(Return(PECI_CC_SUCCESS));

    uint8_t returnValue[2] = {0x86, 0x80}; // 0x8086;
    uint8_t cc = 0x94;
    EXPECT_CALL(*crashdump.libPeciMock, peci_RdEndPointConfigPciLocal_seq)
        .WillRepeatedly(DoAll(SetArrayArgument<7>(returnValue, returnValue + 2),
                              SetArgPointee<9>(cc), Return(PECI_CC_SUCCESS)));

    uint8_t firstReturnEnumBus[2] = {0x00, 0x08};          // 0x0800; BIT 11 set
    uint8_t returnBusNumber[4] = {0x00, 0x00, 0xdd, 0x00}; // bus# 0xdd [23:16]

    EXPECT_CALL(*crashdump.libPeciMock, peci_RdPkgConfig)
        .WillOnce(DoAll(
            SetArrayArgument<4>(firstReturnEnumBus, firstReturnEnumBus + 2),
            SetArgPointee<5>(cc), Return(PECI_CC_SUCCESS)))
        .WillOnce(
            DoAll(SetArrayArgument<4>(returnBusNumber, returnBusNumber + 4),
                  SetArgPointee<5>(cc), Return(PECI_CC_SUCCESS)));

    for (auto cpuinfo : crashdump.cpusInfo)
    {
        ret = logUncoreStatus(&cpuinfo, crashdump.root);
        EXPECT_EQ(ret, ACD_SUCCESS);
    }
}

TEST(UncoreTest, bus30ToPostEnumeratedBusSPR_disabledbus30)
{
    int ret;
    CPUInfo cpuInfo = {.clientAddr = 48,
                       .model = cd_spr,
                       .coreMask = 0x0000db7e,
                       .crashedCoreMask = 0x0,
                       .sectionMask = 0,
                       .chaCount = 0,
                       .initialPeciWake = ON,
                       .inputFile = {},
                       .cpuidRead = {},
                       .chaCountRead = {},
                       .coreMaskRead = {},
                       .launchDelay = {}};

    std::vector<CPUInfo> cpusInfo = {cpuInfo};
    TestCrashdump crashdump(cpusInfo);

    EXPECT_CALL(*crashdump.libPeciMock, peci_Lock)
        .WillRepeatedly(Return(PECI_CC_SUCCESS));

    uint8_t returnValue[2] = {0x86, 0x80}; // 0x8086;
    uint8_t cc = 0x94;
    EXPECT_CALL(*crashdump.libPeciMock, peci_RdEndPointConfigPciLocal_seq)
        .WillRepeatedly(DoAll(SetArrayArgument<7>(returnValue, returnValue + 2),
                              SetArgPointee<9>(cc), Return(PECI_CC_SUCCESS)));

    uint8_t returnBusValidQuery[4] = {0x3f, 0x0f, 0x00,
                                      0x00}; // 0x00000f3f; bit 30 not set
    cc = 0x94;
    EXPECT_CALL(*crashdump.libPeciMock, peci_RdEndPointConfigPciLocal)
        .WillOnce(DoAll(
            SetArrayArgument<7>(returnBusValidQuery, returnBusValidQuery + 4),
            SetArgPointee<8>(cc), Return(PECI_CC_SUCCESS)));

    for (auto cpuinfo : crashdump.cpusInfo)
    {
        ret = logUncoreStatus(&cpuinfo, crashdump.root);
        EXPECT_EQ(ret, ACD_FAILURE);
    }
}

TEST(UncoreTest, bus30ToPostEnumeratedBusICX_disabledbus30)
{
    int ret;
    CPUInfo cpuInfo = {.clientAddr = 48,
                       .model = cd_icx,
                       .coreMask = 0x0000db7e,
                       .crashedCoreMask = 0x0,
                       .sectionMask = 0,
                       .chaCount = 0,
                       .initialPeciWake = ON,
                       .inputFile = {},
                       .cpuidRead = {},
                       .chaCountRead = {},
                       .coreMaskRead = {},
                       .launchDelay = {}};

    std::vector<CPUInfo> cpusInfo = {cpuInfo};
    TestCrashdump crashdump(cpusInfo);

    EXPECT_CALL(*crashdump.libPeciMock, peci_Lock)
        .WillRepeatedly(Return(PECI_CC_SUCCESS));

    uint8_t returnValue[2] = {0x86, 0x80}; // 0x8086;
    uint8_t cc = 0x94;
    EXPECT_CALL(*crashdump.libPeciMock, peci_RdEndPointConfigPciLocal_seq)
        .WillRepeatedly(DoAll(SetArrayArgument<7>(returnValue, returnValue + 2),
                              SetArgPointee<9>(cc), Return(PECI_CC_SUCCESS)));

    uint8_t firstReturnEnumBus[2] = {0x00, 0x00}; // 0x0000; BIT 11 not set
    EXPECT_CALL(*crashdump.libPeciMock, peci_RdPkgConfig)
        .WillOnce(DoAll(
            SetArrayArgument<4>(firstReturnEnumBus, firstReturnEnumBus + 2),
            SetArgPointee<5>(cc), Return(PECI_CC_SUCCESS)));

    for (auto cpuinfo : crashdump.cpusInfo)
    {
        ret = logUncoreStatus(&cpuinfo, crashdump.root);
        EXPECT_EQ(ret, ACD_FAILURE);
    }
}