#pragma once

#include "gmock/gmock.h"
#include "gtest/gtest.h"

extern "C" {
#include "fake_peci.hpp"

#include "CrashdumpSections/crashdump.h"
}

class LibPeciMock
{
  public:
    LibPeciMock()
    {
        fake_ = std::make_unique<FakePeci>();
    }

    virtual ~LibPeciMock()
    {
    }

    MOCK_METHOD8(peci_CrashDump_Discovery,
                 EPECIStatus(uint8_t, uint8_t, uint8_t, uint16_t, uint8_t,
                             uint8_t, uint8_t*, uint8_t*));

    MOCK_METHOD5(peci_RdIAMSR,
                 EPECIStatus(uint8_t, uint8_t, uint16_t, uint64_t*, uint8_t*));

    MOCK_METHOD6(peci_RdPkgConfig, EPECIStatus(uint8_t, uint8_t, uint16_t,
                                               uint8_t, uint8_t*, uint8_t*));

    MOCK_METHOD7(peci_CrashDump_GetFrame,
                 EPECIStatus(uint8_t, uint16_t, uint16_t, uint16_t, uint8_t,
                             uint8_t*, uint8_t*));

    MOCK_METHOD1(peci_Ping, EPECIStatus(uint8_t));

    MOCK_METHOD4(peci_GetCPUID,
                 EPECIStatus(const uint8_t, CPUModel*, uint8_t*, uint8_t*));

    MOCK_METHOD1(peci_Unlock, void(int));

    MOCK_METHOD2(peci_Lock, EPECIStatus(int*, int));

    MOCK_METHOD10(peci_RdEndPointConfigPciLocal_seq,
                  EPECIStatus(uint8_t, uint8_t, uint8_t, uint8_t, uint8_t,
                              uint16_t, uint8_t, uint8_t*, int, uint8_t*));

    MOCK_METHOD(EPECIStatus, peci_RdEndPointConfigMmio_seq,
                (uint8_t, uint8_t, uint8_t, uint8_t, uint8_t, uint8_t, uint8_t,
                 uint64_t, uint8_t, uint8_t*, int, uint8_t*));

    MOCK_METHOD(EPECIStatus, peci_RdEndPointConfigPciLocal,
                (uint8_t, uint8_t, uint8_t, uint8_t, uint8_t, uint16_t, uint8_t,
                 uint8_t*, uint8_t*));

    void DelegateToFakeCrashdumpDiscovery()
    {
        ON_CALL(*this, peci_CrashDump_Discovery)
            .WillByDefault([this](uint8_t target, uint8_t subopcode,
                                  uint8_t param0, uint16_t param1,
                                  uint8_t param2, uint8_t u8ReadLen,
                                  uint8_t* pData, uint8_t* cc) {
                return fake_->peci_CrashDump_Discovery(target, subopcode,
                                                       param0, param1, param2,
                                                       u8ReadLen, pData, cc);
            });
    }

    void DelegateToFakeGetFrame()
    {
        ON_CALL(*this, peci_CrashDump_GetFrame)
            .WillByDefault([this](uint8_t target, uint16_t param0,
                                  uint16_t param1, uint16_t param2,
                                  uint8_t u8ReadLen, uint8_t* pData,
                                  uint8_t* cc) {
                return fake_->peci_CrashDump_GetFrame(
                    target, param0, param1, param2, u8ReadLen, pData, cc);
            });
    }

  private:
    std::unique_ptr<FakePeciBase> fake_;
};
