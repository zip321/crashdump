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

#include "../test_utils.hpp"
#include "CrashdumpSections/CoreMca.hpp"
#include "crashdump.hpp"

#include <safe_mem_lib.h>

#include "gmock/gmock.h"
#include "gtest/gtest.h"

using namespace ::testing;
using ::testing::Return;
using namespace crashdump;

cJSON* selectAndReadInputFile(crashdump::cpu::Model cpuModel, char** filename,
                              bool isTelemetry);

class CoreMcaMOCK
{
  public:
    MOCK_METHOD5(peci_RdIAMSR,
                 EPECIStatus(uint8_t, uint8_t, uint16_t, uint64_t*, uint8_t*));
};

class CoreMcaTestFixture : public Test
{
  public:
    CoreMcaTestFixture()
    {
        coreMcaMock = std::make_unique<NiceMock<CoreMcaMOCK>>();
    }

    ~CoreMcaTestFixture()
    {
        coreMcaMock.reset();
    }

    void SetUp() override
    {
        // Build a list of cpus
        info.model = crashdump::cpu::spr;
        info.coreMask = 0xa58a87356e08ab3;
        cpus.push_back(info);
        info.model = crashdump::cpu::spr;
        info.coreMask = 0xa58a87356e08ab3;
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
    int ret = ACD_FAILURE;
    char* testFileBuf = readTestFile("/tmp/crashdump/ut/ut_core_mca.json");
    static std::unique_ptr<CoreMcaMOCK> coreMcaMock;
};

std::unique_ptr<CoreMcaMOCK> CoreMcaTestFixture::coreMcaMock;

EPECIStatus peci_RdIAMSR(uint8_t target, uint8_t threadID, uint16_t MSRAddress,
                         uint64_t* u64MsrVal, uint8_t* cc)
{
    // minimal mocking
    uint8_t ccode = 0x40;
    uint64_t data[8] = {0};
    memcpy_s(u64MsrVal, 8, data, 8);
    *cc = ccode;
    return CoreMcaTestFixture::coreMcaMock->peci_RdIAMSR(
        target, threadID, MSRAddress, u64MsrVal, cc);
}

TEST_F(CoreMcaTestFixture, logCoreMcaICXSPRFail)
{
    inputFileInfo.buffers[0] = NULL;

    cpus[0].inputFile.filenamePtr = NULL;
    cpus[0].inputFile.bufferPtr = NULL;

    ret = logCoreMca(cpus[0], root);
    ut = cJSON_GetObjectItem(cJSON_Parse(testFileBuf), "logCoreMcaICXSPRFail");
    EXPECT_EQ(ret, ACD_INVALID_OBJECT);
    EXPECT_EQ(true, cJSON_Compare(ut, root, true));
}

TEST_F(CoreMcaTestFixture, logCoreMcaICXSPRDisable)
{
    bool enable = false;

    inputFileInfo.buffers[0] = selectAndReadInputFile(
        cpus[0].model, inputFileInfo.filenames, isTelemetry);

    cpus[0].inputFile.filenamePtr = inputFileInfo.filenames[0];
    cpus[0].inputFile.bufferPtr = inputFileInfo.buffers[0];

    // modify input file on the fly, set "_record_enable":false
    cJSON* inputMod =
        getCrashDataSection(inputFileInfo.buffers[0], "MCA", &enable);
    cJSON_DeleteItemFromObjectCaseSensitive(inputMod, "_record_enable");
    cJSON_AddFalseToObject(inputMod, RECORD_ENABLE);

    ut = cJSON_GetObjectItem(cJSON_Parse(testFileBuf),
                             "logCoreMcaICXSPRDisable");
    ret = logCoreMca(cpus[0], root);
    EXPECT_EQ(ret, ACD_SUCCESS);
    EXPECT_EQ(true, cJSON_Compare(ut, root, true));
}

TEST_F(CoreMcaTestFixture, logCoreMcaICXSPR)
{
    inputFileInfo.buffers[0] = selectAndReadInputFile(
        cpus[0].model, inputFileInfo.filenames, isTelemetry);

    cpus[0].inputFile.filenamePtr = inputFileInfo.filenames[0];
    cpus[0].inputFile.bufferPtr = inputFileInfo.buffers[0];

    EXPECT_CALL(*coreMcaMock, peci_RdIAMSR).Times(AtLeast(1));

    ret = logCoreMca(cpus[0], root);
    ut = cJSON_GetObjectItem(cJSON_Parse(testFileBuf), "logCoreMcaICXSPR");
    EXPECT_EQ(ret, 0);
    EXPECT_EQ(true, cJSON_Compare(ut, root, true));
}

TEST_F(CoreMcaTestFixture, logUncoreMcaICXSPRFail)
{
    inputFileInfo.buffers[0] = NULL;

    cpus[0].inputFile.filenamePtr = NULL;
    cpus[0].inputFile.bufferPtr = NULL;

    ret = logUncoreMca(cpus[0], root);
    ut =
        cJSON_GetObjectItem(cJSON_Parse(testFileBuf), "logUncoreMcaICXSPRFail");
    EXPECT_EQ(ret, ACD_INVALID_OBJECT);
    EXPECT_EQ(true, cJSON_Compare(ut, root, true));
}

TEST_F(CoreMcaTestFixture, logUncoreMcaICXSPR)
{
    inputFileInfo.buffers[0] = selectAndReadInputFile(
        cpus[0].model, inputFileInfo.filenames, isTelemetry);

    cpus[0].inputFile.filenamePtr = inputFileInfo.filenames[0];
    cpus[0].inputFile.bufferPtr = inputFileInfo.buffers[0];

    EXPECT_CALL(*coreMcaMock, peci_RdIAMSR).Times(AtLeast(1));
    ret = logUncoreMca(cpus[0], root);
    ut = cJSON_GetObjectItem(cJSON_Parse(testFileBuf), "logUncoreMcaICXSPR");
    EXPECT_EQ(ret, ACD_SUCCESS);
    EXPECT_EQ(true, cJSON_Compare(ut, root, true));
}