extern "C" {
#include <peci.h>
#include <safe_mem_lib.h>
}

class FakePeciBase
{
  public:
    virtual ~FakePeciBase()
    {
    }

    virtual EPECIStatus peci_CrashDump_Discovery(
        uint8_t target, uint8_t subopcode, uint8_t param0, uint16_t param1,
        uint8_t param2, uint8_t u8ReadLen, uint8_t* pData, uint8_t* cc) = 0;

    virtual EPECIStatus peci_CrashDump_GetFrame(
        uint8_t target, uint16_t param0, uint16_t param1, uint16_t param2,
        uint8_t u8ReadLen, uint8_t* pData, uint8_t* cc) = 0;
};

class FakePeci : public FakePeciBase
{
  public:
    virtual ~FakePeci()
    {
    }

    EPECIStatus peci_CrashDump_Discovery(uint8_t target, uint8_t subopcode,
                                         uint8_t param0, uint16_t param1,
                                         uint8_t param2, uint8_t u8ReadLen,
                                         uint8_t* pData, uint8_t* cc) override
    {
        *cc = 0x40;
        uint8_t data[8] = {0};
        memcpy_s(pData, 8, data, 8);
        return PECI_CC_SUCCESS;
    }

    EPECIStatus peci_CrashDump_GetFrame(uint8_t target, uint16_t param0,
                                        uint16_t param1, uint16_t param2,
                                        uint8_t u8ReadLen, uint8_t* pData,
                                        uint8_t* cc) override
    {
        *cc = 0x40;
        uint8_t data[8] = {0xde, 0xad, 0xbe, 0xef};
        memcpy_s(pData, 8, data, 8);
        return PECI_CC_SUCCESS;
    }
};