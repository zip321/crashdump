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

#pragma once
#ifdef __cplusplus
extern "C" {
#endif

#include <inttypes.h>
#include <stdbool.h>
#include <stdint.h>
// clang-format off
#include <linux/peci-ioctl.h>
// clang-format on

// PECI Completion Codes
typedef enum
{
    PECI_CC_SUCCESS = 0,
    PECI_CC_TIMEOUT,
    PECI_CC_INVALID_REQ,
    PECI_CC_HW_ERR,
    PECI_CC_DRIVER_ERR,
    PECI_CC_CPU_NOT_PRESENT,
    PECI_CC_WRONG_POWER_STATE,
    PECI_CC_MEM_ERR,
} EPECIStatus;

// PECI Timeout Options
typedef enum
{
    PECI_WAIT_FOREVER = -1,
    PECI_NO_WAIT = 0,
} EPECITimeout;

#define PECI_TIMEOUT_RESOLUTION_MS 10 // 10 ms
#define PECI_TIMEOUT_MS 100           // 100 ms

// VCU Index and Sequence Paramaters
#define VCU_SET_PARAM 0x0001
#define VCU_READ 0x0002
#define VCU_OPEN_SEQ 0x0003
#define VCU_CLOSE_SEQ 0x0004
#define VCU_ABORT_SEQ 0x0005
#define VCU_VERSION 0x0009

typedef enum
{
    VCU_READ_LOCAL_CSR_SEQ = 0x2,
    VCU_READ_LOCAL_MMIO_SEQ = 0x6,
    VCU_EN_SECURE_DATA_SEQ = 0x14,
    VCU_CORE_MCA_SEQ = 0x10000,
    VCU_UNCORE_MCA_SEQ = 0x10000,
    VCU_IOT_BRKPT_SEQ = 0x10010,
    VCU_MBP_CONFIG_SEQ = 0x10026,
    VCU_PWR_MGT_SEQ = 0x1002a,
    VCU_CRASHDUMP_SEQ = 0x10038,
    VCU_ARRAY_DUMP_SEQ = 0x20000,
    VCU_SCAN_DUMP_SEQ = 0x20008,
    VCU_TOR_DUMP_SEQ = 0x30002,
    VCU_SQ_DUMP_SEQ = 0x30004,
    VCU_UNCORE_CRASHDUMP_SEQ = 0x30006,
} EPECISequence;

#define MBX_INDEX_VCU 128 // VCU Index

typedef enum
{
    MMIO_DWORD_OFFSET = 0x05,
    MMIO_QWORD_OFFSET = 0x06,
} EEndPtMmioAddrType;

// Find the specified PCI bus number value
EPECIStatus FindBusNumber(uint8_t u8Bus, uint8_t u8Cpu, uint8_t* pu8BusValue);

// Gets the temperature from the target
// Expressed in signed fixed point value of 1/64 degrees celsius
EPECIStatus peci_GetTemp(uint8_t target, int16_t* temperature);

//  Provides read access to the package configuration space within the processor
EPECIStatus peci_RdPkgConfig(uint8_t target, uint8_t u8Index, uint16_t u16Value,
                             uint8_t u8ReadLen, uint8_t* pPkgConfig,
                             uint8_t* cc);

//  Allows sequential RdPkgConfig with the provided peci file descriptor
EPECIStatus peci_RdPkgConfig_seq(uint8_t target, uint8_t u8Index,
                                 uint16_t u16Value, uint8_t u8ReadLen,
                                 uint8_t* pPkgConfig, int peci_fd, uint8_t* cc);

//  Provides write access to the package configuration space within the
//  processor
EPECIStatus peci_WrPkgConfig(uint8_t target, uint8_t u8Index, uint16_t u16Param,
                             uint32_t u32Value, uint8_t u8WriteLen,
                             uint8_t* cc);

//  Allows sequential WrPkgConfig with the provided peci file descriptor
EPECIStatus peci_WrPkgConfig_seq(uint8_t target, uint8_t u8Index,
                                 uint16_t u16Param, uint32_t u32Value,
                                 uint8_t u8WriteLen, int peci_fd, uint8_t* cc);

// Provides read access to Model Specific Registers
EPECIStatus peci_RdIAMSR(uint8_t target, uint8_t threadID, uint16_t MSRAddress,
                         uint64_t* u64MsrVal, uint8_t* cc);

// Provides read access to PCI Configuration space
EPECIStatus peci_RdPCIConfig(uint8_t target, uint8_t u8Bus, uint8_t u8Device,
                             uint8_t u8Fcn, uint16_t u16Reg, uint8_t* pPCIReg,
                             uint8_t* cc);

//  Allows sequential RdPCIConfig with the provided peci file descriptor
EPECIStatus peci_RdPCIConfig_seq(uint8_t target, uint8_t u8Bus,
                                 uint8_t u8Device, uint8_t u8Fcn,
                                 uint16_t u16Reg, uint8_t* pPCIData,
                                 int peci_fd, uint8_t* cc);

// Provides sequential read access to PCI Configuration space with the provided
// peci file descriptor
EPECIStatus peci_MultipleRdPCIConfig_seq(uint8_t target, uint8_t u8Bus,
                                         uint8_t u8Device, uint8_t u8Fcn,
                                         uint16_t u16Reg, uint8_t u8Len,
                                         uint8_t* pPCIData, int peci_fd,
                                         uint8_t* cc);

// Provides read access to the local PCI Configuration space
EPECIStatus peci_RdPCIConfigLocal(uint8_t target, uint8_t u8Bus,
                                  uint8_t u8Device, uint8_t u8Fcn,
                                  uint16_t u16Reg, uint8_t u8ReadLen,
                                  uint8_t* pPCIReg, uint8_t* cc);

//  Allows sequential RdPCIConfigLocal with the provided peci file descriptor
EPECIStatus peci_RdPCIConfigLocal_seq(uint8_t target, uint8_t u8Bus,
                                      uint8_t u8Device, uint8_t u8Fcn,
                                      uint16_t u16Reg, uint8_t u8ReadLen,
                                      uint8_t* pPCIReg, int peci_fd,
                                      uint8_t* cc);

// Provides write access to the local PCI Configuration space
EPECIStatus peci_WrPCIConfigLocal(uint8_t target, uint8_t u8Bus,
                                  uint8_t u8Device, uint8_t u8Fcn,
                                  uint16_t u16Reg, uint8_t DataLen,
                                  uint32_t DataVal);

// Provides read access to PCI configuration space
EPECIStatus peci_RdEndPointConfigPci(uint8_t target, uint8_t u8Seg,
                                     uint8_t u8Bus, uint8_t u8Device,
                                     uint8_t u8Fcn, uint16_t u16Reg,
                                     uint8_t u8ReadLen, uint8_t* pPCIData,
                                     uint8_t* cc);

// Allows sequential RdEndPointConfig to PCI Configuration space
EPECIStatus peci_RdEndPointConfigPci_seq(uint8_t target, uint8_t u8Seg,
                                         uint8_t u8Bus, uint8_t u8Device,
                                         uint8_t u8Fcn, uint16_t u16Reg,
                                         uint8_t u8ReadLen, uint8_t* pPCIData,
                                         int peci_fd, uint8_t* cc);

// Provides read access to the local PCI configuration space
EPECIStatus peci_RdEndPointConfigPciLocal(uint8_t target, uint8_t u8Seg,
                                          uint8_t u8Bus, uint8_t u8Device,
                                          uint8_t u8Fcn, uint16_t u16Reg,
                                          uint8_t u8ReadLen, uint8_t* pPCIData,
                                          uint8_t* cc);

// Allows sequential RdEndPointConfig to the local PCI Configuration space
EPECIStatus peci_RdEndPointConfigPciLocal_seq(uint8_t target, uint8_t u8Seg,
                                              uint8_t u8Bus, uint8_t u8Device,
                                              uint8_t u8Fcn, uint16_t u16Reg,
                                              uint8_t u8ReadLen,
                                              uint8_t* pPCIData, int peci_fd,
                                              uint8_t* cc);

// Provides read access to PCI MMIO space
EPECIStatus peci_RdEndPointConfigMmio(uint8_t target, uint8_t u8Seg,
                                      uint8_t u8Bus, uint8_t u8Device,
                                      uint8_t u8Fcn, uint8_t u8Bar,
                                      uint8_t u8AddrType, uint64_t u64Offset,
                                      uint8_t u8ReadLen, uint8_t* pMmioData,
                                      uint8_t* cc);

// Allows sequential RdEndPointConfig to PCI MMIO space
EPECIStatus peci_RdEndPointConfigMmio_seq(
    uint8_t target, uint8_t u8Seg, uint8_t u8Bus, uint8_t u8Device,
    uint8_t u8Fcn, uint8_t u8Bar, uint8_t u8AddrType, uint64_t u64Offset,
    uint8_t u8ReadLen, uint8_t* pMmioData, int peci_fd, uint8_t* cc);

// Provides access to the Crashdump Discovery API
EPECIStatus peci_CrashDump_Discovery(uint8_t target, uint8_t subopcode,
                                     uint8_t param0, uint16_t param1,
                                     uint8_t param2, uint8_t u8ReadLen,
                                     uint8_t* pData, uint8_t* cc);

// Provides access to the Crashdump GetFrame API
EPECIStatus peci_CrashDump_GetFrame(uint8_t target, uint16_t param0,
                                    uint16_t param1, uint16_t param2,
                                    uint8_t u8ReadLen, uint8_t* pData,
                                    uint8_t* cc);

// Provides raw PECI command access
EPECIStatus peci_raw(uint8_t target, uint8_t u8ReadLen, const uint8_t* pRawCmd,
                     const uint32_t cmdSize, uint8_t* pRawResp,
                     uint32_t respSize);

EPECIStatus peci_Lock(int* peci_fd, uint32_t timeout_ms);
void peci_Unlock(int peci_fd);
EPECIStatus peci_Ping(uint8_t target);
EPECIStatus peci_Ping_seq(uint8_t target, int peci_fd);

#ifdef __cplusplus
}
#endif
