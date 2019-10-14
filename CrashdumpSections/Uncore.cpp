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

#include "Uncore.hpp"

extern "C" {
#include <cjson/cJSON.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
}

#include "UncoreRegs.hpp"
#include "crashdump.hpp"
static const int SpreadSheetDataVersion = 0x12;
static const SUncoreStatusRegPci sUncoreStatusPci[] = {
    // Register, Bus, Dev, Func, Offset, Size
    {"B00_D00_F0_0x0", 0, 0, 0, 0x0, US_REG_WORD},
    {"B00_D00_F0_0x14C", 0, 0, 0, 0x14C, US_REG_DWORD},
    {"B00_D00_F0_0x158", 0, 0, 0, 0x158, US_REG_DWORD},
    {"B00_D00_F0_0x178", 0, 0, 0, 0x178, US_REG_DWORD},
    {"B00_D00_F0_0x17C", 0, 0, 0, 0x17C, US_REG_DWORD},
    {"B00_D00_F0_0x188", 0, 0, 0, 0x188, US_REG_DWORD},
    {"B00_D00_F0_0x18C", 0, 0, 0, 0x18C, US_REG_DWORD},
    {"B00_D00_F0_0x1B2", 0, 0, 0, 0x1B2, US_REG_WORD},
    {"B00_D00_F0_0x1C2", 0, 0, 0, 0x1C2, US_REG_WORD},
    {"B00_D00_F0_0x1E", 0, 0, 0, 0x1E, US_REG_WORD},
    {"B00_D00_F0_0x2", 0, 0, 0, 0x2, US_REG_WORD},
    {"B00_D00_F0_0x200", 0, 0, 0, 0x200, US_REG_BYTE},
    {"B00_D00_F0_0x204", 0, 0, 0, 0x204, US_REG_BYTE},
    {"B00_D00_F0_0x208", 0, 0, 0, 0x208, US_REG_BYTE},
    {"B00_D00_F0_0x20C", 0, 0, 0, 0x20C, US_REG_BYTE},
    {"B00_D00_F0_0x210", 0, 0, 0, 0x210, US_REG_BYTE},
    {"B00_D00_F0_0x214", 0, 0, 0, 0x214, US_REG_BYTE},
    {"B00_D00_F0_0x218", 0, 0, 0, 0x218, US_REG_BYTE},
    {"B00_D00_F0_0x21C", 0, 0, 0, 0x21C, US_REG_BYTE},
    {"B00_D00_F0_0x220", 0, 0, 0, 0x220, US_REG_BYTE},
    {"B00_D00_F0_0x224", 0, 0, 0, 0x224, US_REG_BYTE},
    {"B00_D00_F0_0x228", 0, 0, 0, 0x228, US_REG_BYTE},
    {"B00_D00_F0_0x230", 0, 0, 0, 0x230, US_REG_BYTE},
    {"B00_D00_F0_0x232", 0, 0, 0, 0x232, US_REG_BYTE},
    {"B00_D00_F0_0x258", 0, 0, 0, 0x258, US_REG_DWORD},
    {"B00_D00_F0_0x288", 0, 0, 0, 0x288, US_REG_DWORD},
    {"B00_D00_F0_0x298", 0, 0, 0, 0x298, US_REG_DWORD},
    {"B00_D00_F0_0x2A4", 0, 0, 0, 0x2A4, US_REG_DWORD},
    {"B00_D00_F0_0x4", 0, 0, 0, 0x4, US_REG_WORD},
    {"B00_D00_F0_0x6", 0, 0, 0, 0x6, US_REG_WORD},
    {"B00_D00_F0_0x9A", 0, 0, 0, 0x9A, US_REG_WORD},
    {"B00_D00_F0_0xA2", 0, 0, 0, 0xA2, US_REG_WORD},
    {"B00_D00_F0_0xAA", 0, 0, 0, 0xAA, US_REG_WORD},
    {"B00_D00_F0_0xC2", 0, 0, 0, 0xC2, US_REG_WORD},
    {"B00_D00_F0_0xF2", 0, 0, 0, 0xF2, US_REG_WORD},
    {"B00_D04_F0_0x148", 0, 4, 0, 0x148, US_REG_DWORD},
    {"B00_D04_F0_0x9A", 0, 4, 0, 0x9A, US_REG_WORD},
    {"B00_D04_F1_0x9A", 0, 4, 1, 0x9A, US_REG_WORD},
    {"B00_D04_F2_0x9A", 0, 4, 2, 0x9A, US_REG_WORD},
    {"B00_D04_F3_0x9A", 0, 4, 3, 0x9A, US_REG_WORD},
    {"B00_D04_F4_0x9A", 0, 4, 4, 0x9A, US_REG_WORD},
    {"B00_D04_F5_0x9A", 0, 4, 5, 0x9A, US_REG_WORD},
    {"B00_D04_F6_0x9A", 0, 4, 6, 0x9A, US_REG_WORD},
    {"B00_D04_F7_0x9A", 0, 4, 7, 0x9A, US_REG_WORD},
    {"B00_D05_F0_0x1A8", 0, 5, 0, 0x1A8, US_REG_DWORD},
    {"B00_D05_F0_0x1B4", 0, 5, 0, 0x1B4, US_REG_DWORD},
    {"B00_D05_F1_0x10E", 0, 5, 1, 0x10E, US_REG_WORD},
    {"B00_D05_F1_0x11E", 0, 5, 1, 0x11E, US_REG_WORD},
    {"B00_D05_F1_0x12E", 0, 5, 1, 0x12E, US_REG_WORD},
    {"B00_D05_F1_0x13E", 0, 5, 1, 0x13E, US_REG_WORD},
    {"B00_D05_F2_0x0", 0, 5, 2, 0x0, US_REG_WORD},
    {"B00_D05_F2_0x19C", 0, 5, 2, 0x19C, US_REG_DWORD},
    {"B00_D05_F2_0x1A0", 0, 5, 2, 0x1A0, US_REG_DWORD},
    {"B00_D05_F2_0x1A4", 0, 5, 2, 0x1A4, US_REG_DWORD},
    {"B00_D05_F2_0x1A8", 0, 5, 2, 0x1A8, US_REG_DWORD},
    {"B00_D05_F2_0x1AC", 0, 5, 2, 0x1AC, US_REG_DWORD},
    {"B00_D05_F2_0x1B4", 0, 5, 2, 0x1B4, US_REG_DWORD},
    {"B00_D05_F2_0x1C0", 0, 5, 2, 0x1C0, US_REG_DWORD},
    {"B00_D05_F2_0x1C4", 0, 5, 2, 0x1C4, US_REG_DWORD},
    {"B00_D05_F2_0x1C8", 0, 5, 2, 0x1C8, US_REG_DWORD},
    {"B00_D05_F2_0x1CC", 0, 5, 2, 0x1CC, US_REG_DWORD},
    {"B00_D05_F2_0x1D0", 0, 5, 2, 0x1D0, US_REG_DWORD},
    {"B00_D05_F2_0x1D4", 0, 5, 2, 0x1D4, US_REG_DWORD},
    {"B00_D05_F2_0x1D8", 0, 5, 2, 0x1D8, US_REG_DWORD},
    {"B00_D05_F2_0x1DC", 0, 5, 2, 0x1DC, US_REG_DWORD},
    {"B00_D05_F2_0x1E8", 0, 5, 2, 0x1E8, US_REG_DWORD},
    {"B00_D05_F2_0x1EC", 0, 5, 2, 0x1EC, US_REG_DWORD},
    {"B00_D05_F2_0x1F8", 0, 5, 2, 0x1F8, US_REG_DWORD},
    {"B00_D05_F2_0x2", 0, 5, 2, 0x2, US_REG_WORD},
    {"B00_D05_F2_0x200", 0, 5, 2, 0x200, US_REG_DWORD},
    {"B00_D05_F2_0x208", 0, 5, 2, 0x208, US_REG_DWORD},
    {"B00_D05_F2_0x20C", 0, 5, 2, 0x20C, US_REG_DWORD},
    {"B00_D05_F2_0x210", 0, 5, 2, 0x210, US_REG_DWORD},
    {"B00_D05_F2_0x214", 0, 5, 2, 0x214, US_REG_DWORD},
    {"B00_D05_F2_0x218", 0, 5, 2, 0x218, US_REG_DWORD},
    {"B00_D05_F2_0x21C", 0, 5, 2, 0x21C, US_REG_DWORD},
    {"B00_D05_F2_0x220", 0, 5, 2, 0x220, US_REG_DWORD},
    {"B00_D05_F2_0x224", 0, 5, 2, 0x224, US_REG_DWORD},
    {"B00_D05_F2_0x228", 0, 5, 2, 0x228, US_REG_DWORD},
    {"B00_D05_F2_0x230", 0, 5, 2, 0x230, US_REG_DWORD},
    {"B00_D05_F2_0x234", 0, 5, 2, 0x234, US_REG_DWORD},
    {"B00_D05_F2_0x238", 0, 5, 2, 0x238, US_REG_DWORD},
    {"B00_D05_F2_0x23C", 0, 5, 2, 0x23C, US_REG_DWORD},
    {"B00_D05_F2_0x240", 0, 5, 2, 0x240, US_REG_DWORD},
    {"B00_D05_F2_0x244", 0, 5, 2, 0x244, US_REG_DWORD},
    {"B00_D05_F2_0x248", 0, 5, 2, 0x248, US_REG_DWORD},
    {"B00_D05_F2_0x24C", 0, 5, 2, 0x24C, US_REG_DWORD},
    {"B00_D05_F2_0x250", 0, 5, 2, 0x250, US_REG_DWORD},
    {"B00_D05_F2_0x254", 0, 5, 2, 0x254, US_REG_DWORD},
    {"B00_D05_F2_0x268", 0, 5, 2, 0x268, US_REG_DWORD},
    {"B00_D05_F2_0x26C", 0, 5, 2, 0x26C, US_REG_DWORD},
    {"B00_D05_F2_0x270", 0, 5, 2, 0x270, US_REG_DWORD},
    {"B00_D05_F2_0x274", 0, 5, 2, 0x274, US_REG_DWORD},
    {"B00_D05_F2_0x278", 0, 5, 2, 0x278, US_REG_DWORD},
    {"B00_D05_F2_0x288", 0, 5, 2, 0x288, US_REG_DWORD},
    {"B00_D05_F2_0x28C", 0, 5, 2, 0x28C, US_REG_DWORD},
    {"B00_D05_F2_0x290", 0, 5, 2, 0x290, US_REG_DWORD},
    {"B00_D05_F2_0x294", 0, 5, 2, 0x294, US_REG_DWORD},
    {"B00_D05_F2_0x298", 0, 5, 2, 0x298, US_REG_DWORD},
    {"B00_D05_F2_0x29C", 0, 5, 2, 0x29C, US_REG_DWORD},
    {"B00_D05_F2_0x2A0", 0, 5, 2, 0x2A0, US_REG_DWORD},
    {"B00_D05_F2_0x2A4", 0, 5, 2, 0x2A4, US_REG_DWORD},
    {"B00_D05_F2_0x2A8", 0, 5, 2, 0x2A8, US_REG_DWORD},
    {"B00_D05_F2_0x2AC", 0, 5, 2, 0x2AC, US_REG_DWORD},
    {"B00_D05_F2_0x2B0", 0, 5, 2, 0x2B0, US_REG_DWORD},
    {"B00_D05_F2_0x2B4", 0, 5, 2, 0x2B4, US_REG_DWORD},
    {"B00_D05_F2_0x2B8", 0, 5, 2, 0x2B8, US_REG_DWORD},
    {"B00_D05_F2_0x2CC", 0, 5, 2, 0x2CC, US_REG_DWORD},
    {"B00_D05_F2_0x2D0", 0, 5, 2, 0x2D0, US_REG_DWORD},
    {"B00_D05_F2_0x2D4", 0, 5, 2, 0x2D4, US_REG_DWORD},
    {"B00_D05_F2_0x2D8", 0, 5, 2, 0x2D8, US_REG_DWORD},
    {"B00_D05_F2_0x2DC", 0, 5, 2, 0x2DC, US_REG_DWORD},
    {"B00_D05_F2_0x2E0", 0, 5, 2, 0x2E0, US_REG_DWORD},
    {"B00_D05_F2_0x2E4", 0, 5, 2, 0x2E4, US_REG_DWORD},
    {"B00_D05_F2_0x2E8", 0, 5, 2, 0x2E8, US_REG_DWORD},
    {"B00_D05_F2_0x2EC", 0, 5, 2, 0x2EC, US_REG_DWORD},
    {"B00_D05_F2_0x2FC", 0, 5, 2, 0x2FC, US_REG_DWORD},
    {"B00_D05_F2_0x304", 0, 5, 2, 0x304, US_REG_DWORD},
    {"B00_D05_F2_0x4", 0, 5, 2, 0x4, US_REG_WORD},
    {"B00_D05_F2_0x6", 0, 5, 2, 0x6, US_REG_WORD},
    {"B00_D05_F2_0x80", 0, 5, 2, 0x80, US_REG_DWORD},
    {"B00_D05_F2_0x94", 0, 5, 2, 0x94, US_REG_DWORD},
    {"B00_D05_F2_0xA0", 0, 5, 2, 0xA0, US_REG_DWORD},
    {"B00_D07_F0_0x4A", 0, 7, 0, 0x4A, US_REG_WORD},
    {"B00_D07_F0_0x52", 0, 7, 0, 0x52, US_REG_WORD},
    {"B00_D07_F0_0x6", 0, 7, 0, 0x6, US_REG_WORD},
    {"B00_D07_F0_0x72", 0, 7, 0, 0x72, US_REG_WORD},
    {"B00_D07_F0_0x7A", 0, 7, 0, 0x7A, US_REG_WORD},
    {"B00_D07_F4_0x4A", 0, 7, 4, 0x4A, US_REG_WORD},
    {"B00_D07_F4_0x52", 0, 7, 4, 0x52, US_REG_WORD},
    {"B00_D07_F4_0x6", 0, 7, 4, 0x6, US_REG_WORD},
    {"B00_D07_F4_0x72", 0, 7, 4, 0x72, US_REG_WORD},
    {"B00_D07_F4_0x7A", 0, 7, 4, 0x7A, US_REG_WORD},
    {"B00_D07_F7_0x358", 0, 7, 7, 0x358, US_REG_DWORD},
    {"B00_D07_F7_0x4A", 0, 7, 7, 0x4A, US_REG_WORD},
    {"B00_D07_F7_0x52", 0, 7, 7, 0x52, US_REG_WORD},
    {"B00_D07_F7_0x6", 0, 7, 7, 0x6, US_REG_WORD},
    {"B00_D07_F7_0x72", 0, 7, 7, 0x72, US_REG_WORD},
    {"B00_D07_F7_0x7A", 0, 7, 7, 0x7A, US_REG_WORD},
    {"B00_D08_F0_0x204", 0, 8, 0, 0x204, US_REG_DWORD},
    {"B00_D08_F0_0x208", 0, 8, 0, 0x208, US_REG_DWORD},
    {"B00_D08_F0_0x20C", 0, 8, 0, 0x20C, US_REG_DWORD},
    {"B00_D08_F0_0x210", 0, 8, 0, 0x210, US_REG_DWORD},
    {"B00_D08_F0_0x214", 0, 8, 0, 0x214, US_REG_DWORD},
    {"B00_D08_F0_0x218", 0, 8, 0, 0x218, US_REG_DWORD},
    {"B00_D08_F0_0x21C", 0, 8, 0, 0x21C, US_REG_DWORD},
    {"B00_D08_F0_0x220", 0, 8, 0, 0x220, US_REG_DWORD},
    {"B00_D08_F0_0x224", 0, 8, 0, 0x224, US_REG_DWORD},
    {"B00_D08_F0_0x228", 0, 8, 0, 0x228, US_REG_DWORD},
    {"B00_D08_F0_0x22C", 0, 8, 0, 0x22C, US_REG_DWORD},
    {"B00_D08_F0_0x230", 0, 8, 0, 0x230, US_REG_DWORD},
    {"B00_D08_F0_0x234", 0, 8, 0, 0x234, US_REG_DWORD},
    {"B00_D08_F0_0x238", 0, 8, 0, 0x238, US_REG_DWORD},
    {"B00_D08_F0_0x90", 0, 8, 0, 0x90, US_REG_DWORD},
    {"B00_D08_F0_0x94", 0, 8, 0, 0x94, US_REG_DWORD},
    {"B00_D08_F0_0xA4", 0, 8, 0, 0xA4, US_REG_DWORD},
    {"B00_D08_F0_0xA8", 0, 8, 0, 0xA8, US_REG_DWORD},
    {"B00_D08_F0_0xB0", 0, 8, 0, 0xB0, US_REG_DWORD},
    {"B00_D08_F0_0xB8", 0, 8, 0, 0xB8, US_REG_DWORD},
    {"B00_D08_F0_0xD8", 0, 8, 0, 0xD8, US_REG_DWORD},
    {"B00_D08_F0_0xE4", 0, 8, 0, 0xE4, US_REG_DWORD},
    {"B00_D08_F0_0xE8", 0, 8, 0, 0xE8, US_REG_DWORD},
    {"B00_D08_F0_0xEC", 0, 8, 0, 0xEC, US_REG_DWORD},
    {"B00_D08_F0_0xF0", 0, 8, 0, 0xF0, US_REG_DWORD},
    {"B00_D08_F2_0x7C", 0, 8, 2, 0x7C, US_REG_DWORD},
    {"B00_D08_F2_0x80", 0, 8, 2, 0x80, US_REG_DWORD},
    {"B00_D08_F2_0x84", 0, 8, 2, 0x84, US_REG_DWORD},
    {"B00_D08_F2_0x88", 0, 8, 2, 0x88, US_REG_DWORD},
    {"B00_D08_F2_0x8C", 0, 8, 2, 0x8C, US_REG_DWORD},
    {"B00_D08_F2_0x90", 0, 8, 2, 0x90, US_REG_DWORD},
    {"B00_D08_F2_0x94", 0, 8, 2, 0x94, US_REG_DWORD},
    {"B00_D08_F2_0x98", 0, 8, 2, 0x98, US_REG_DWORD},
    {"B00_D08_F2_0x9C", 0, 8, 2, 0x9C, US_REG_DWORD},
    {"B00_D08_F2_0xA0", 0, 8, 2, 0xA0, US_REG_DWORD},
    {"B00_D08_F2_0xA4", 0, 8, 2, 0xA4, US_REG_DWORD},
    {"B00_D08_F2_0xA8", 0, 8, 2, 0xA8, US_REG_DWORD},
    {"B00_D08_F2_0xAC", 0, 8, 2, 0xAC, US_REG_DWORD},
    {"B00_D08_F2_0xB0", 0, 8, 2, 0xB0, US_REG_DWORD},
    {"B00_D08_F2_0xB4", 0, 8, 2, 0xB4, US_REG_DWORD},
    {"B00_D08_F2_0xB8", 0, 8, 2, 0xB8, US_REG_DWORD},
    {"B00_D08_F2_0xBC", 0, 8, 2, 0xBC, US_REG_DWORD},
    {"B00_D08_F2_0xC0", 0, 8, 2, 0xC0, US_REG_DWORD},
    {"B00_D08_F2_0xC4", 0, 8, 2, 0xC4, US_REG_DWORD},
    {"B00_D08_F2_0xC8", 0, 8, 2, 0xC8, US_REG_DWORD},
    {"B00_D08_F2_0xCC", 0, 8, 2, 0xCC, US_REG_DWORD},
    {"B00_D08_F2_0xD0", 0, 8, 2, 0xD0, US_REG_DWORD},
    {"B00_D08_F2_0xD4", 0, 8, 2, 0xD4, US_REG_DWORD},
    {"B00_D08_F2_0xD8", 0, 8, 2, 0xD8, US_REG_DWORD},
    {"B00_D08_F2_0xDC", 0, 8, 2, 0xDC, US_REG_DWORD},
    {"B00_D08_F2_0xE8", 0, 8, 2, 0xE8, US_REG_DWORD},
    {"B00_D08_F2_0xEC", 0, 8, 2, 0xEC, US_REG_DWORD},
    {"B00_D08_F2_0xF4", 0, 8, 2, 0xF4, US_REG_DWORD},
    {"B00_D08_F2_0xF8", 0, 8, 2, 0xF8, US_REG_DWORD},
    {"B01_D00_F0_0x0", 1, 0, 0, 0x0, US_REG_WORD},
    {"B01_D00_F0_0x14C", 1, 0, 0, 0x14C, US_REG_DWORD},
    {"B01_D00_F0_0x158", 1, 0, 0, 0x158, US_REG_DWORD},
    {"B01_D00_F0_0x178", 1, 0, 0, 0x178, US_REG_DWORD},
    {"B01_D00_F0_0x17C", 1, 0, 0, 0x17C, US_REG_DWORD},
    {"B01_D00_F0_0x18", 1, 0, 0, 0x18, US_REG_BYTE},
    {"B01_D00_F0_0x188", 1, 0, 0, 0x188, US_REG_DWORD},
    {"B01_D00_F0_0x18C", 1, 0, 0, 0x18C, US_REG_DWORD},
    {"B01_D00_F0_0x19", 1, 0, 0, 0x19, US_REG_BYTE},
    {"B01_D00_F0_0x1A", 1, 0, 0, 0x1A, US_REG_BYTE},
    {"B01_D00_F0_0x1A2", 1, 0, 0, 0x1A2, US_REG_WORD},
    {"B01_D00_F0_0x1AA", 1, 0, 0, 0x1AA, US_REG_WORD},
    {"B01_D00_F0_0x1C", 1, 0, 0, 0x1C, US_REG_BYTE},
    {"B01_D00_F0_0x1C2", 1, 0, 0, 0x1C2, US_REG_WORD},
    {"B01_D00_F0_0x1D", 1, 0, 0, 0x1D, US_REG_BYTE},
    {"B01_D00_F0_0x1E", 1, 0, 0, 0x1E, US_REG_WORD},
    {"B01_D00_F0_0x2", 1, 0, 0, 0x2, US_REG_WORD},
    {"B01_D00_F0_0x20", 1, 0, 0, 0x20, US_REG_WORD},
    {"B01_D00_F0_0x200", 1, 0, 0, 0x200, US_REG_BYTE},
    {"B01_D00_F0_0x204", 1, 0, 0, 0x204, US_REG_BYTE},
    {"B01_D00_F0_0x208", 1, 0, 0, 0x208, US_REG_BYTE},
    {"B01_D00_F0_0x20C", 1, 0, 0, 0x20C, US_REG_BYTE},
    {"B01_D00_F0_0x210", 1, 0, 0, 0x210, US_REG_BYTE},
    {"B01_D00_F0_0x214", 1, 0, 0, 0x214, US_REG_BYTE},
    {"B01_D00_F0_0x218", 1, 0, 0, 0x218, US_REG_BYTE},
    {"B01_D00_F0_0x21C", 1, 0, 0, 0x21C, US_REG_BYTE},
    {"B01_D00_F0_0x22", 1, 0, 0, 0x22, US_REG_WORD},
    {"B01_D00_F0_0x220", 1, 0, 0, 0x220, US_REG_BYTE},
    {"B01_D00_F0_0x224", 1, 0, 0, 0x224, US_REG_BYTE},
    {"B01_D00_F0_0x228", 1, 0, 0, 0x228, US_REG_BYTE},
    {"B01_D00_F0_0x230", 1, 0, 0, 0x230, US_REG_BYTE},
    {"B01_D00_F0_0x232", 1, 0, 0, 0x232, US_REG_BYTE},
    {"B01_D00_F0_0x24", 1, 0, 0, 0x24, US_REG_WORD},
    {"B01_D00_F0_0x258", 1, 0, 0, 0x258, US_REG_DWORD},
    {"B01_D00_F0_0x26", 1, 0, 0, 0x26, US_REG_WORD},
    {"B01_D00_F0_0x28", 1, 0, 0, 0x28, US_REG_DWORD},
    {"B01_D00_F0_0x288", 1, 0, 0, 0x288, US_REG_DWORD},
    {"B01_D00_F0_0x298", 1, 0, 0, 0x298, US_REG_DWORD},
    {"B01_D00_F0_0x2A4", 1, 0, 0, 0x2A4, US_REG_DWORD},
    {"B01_D00_F0_0x2C", 1, 0, 0, 0x2C, US_REG_DWORD},
    {"B01_D00_F0_0x4", 1, 0, 0, 0x4, US_REG_WORD},
    {"B01_D00_F0_0x6", 1, 0, 0, 0x6, US_REG_WORD},
    {"B01_D00_F0_0x9A", 1, 0, 0, 0x9A, US_REG_WORD},
    {"B01_D00_F0_0xA2", 1, 0, 0, 0xA2, US_REG_WORD},
    {"B01_D00_F0_0xAA", 1, 0, 0, 0xAA, US_REG_WORD},
    {"B01_D00_F0_0xC2", 1, 0, 0, 0xC2, US_REG_WORD},
    {"B01_D01_F0_0x0", 1, 1, 0, 0x0, US_REG_WORD},
    {"B01_D01_F0_0x158", 1, 1, 0, 0x158, US_REG_DWORD},
    {"B01_D01_F0_0x178", 1, 1, 0, 0x178, US_REG_DWORD},
    {"B01_D01_F0_0x17C", 1, 1, 0, 0x17C, US_REG_DWORD},
    {"B01_D01_F0_0x18", 1, 1, 0, 0x18, US_REG_BYTE},
    {"B01_D01_F0_0x188", 1, 1, 0, 0x188, US_REG_DWORD},
    {"B01_D01_F0_0x18C", 1, 1, 0, 0x18C, US_REG_DWORD},
    {"B01_D01_F0_0x19", 1, 1, 0, 0x19, US_REG_BYTE},
    {"B01_D01_F0_0x1A", 1, 1, 0, 0x1A, US_REG_BYTE},
    {"B01_D01_F0_0x1C", 1, 1, 0, 0x1C, US_REG_BYTE},
    {"B01_D01_F0_0x1D", 1, 1, 0, 0x1D, US_REG_BYTE},
    {"B01_D01_F0_0x1E", 1, 1, 0, 0x1E, US_REG_WORD},
    {"B01_D01_F0_0x2", 1, 1, 0, 0x2, US_REG_WORD},
    {"B01_D01_F0_0x20", 1, 1, 0, 0x20, US_REG_WORD},
    {"B01_D01_F0_0x200", 1, 1, 0, 0x200, US_REG_BYTE},
    {"B01_D01_F0_0x204", 1, 1, 0, 0x204, US_REG_BYTE},
    {"B01_D01_F0_0x208", 1, 1, 0, 0x208, US_REG_BYTE},
    {"B01_D01_F0_0x20C", 1, 1, 0, 0x20C, US_REG_BYTE},
    {"B01_D01_F0_0x210", 1, 1, 0, 0x210, US_REG_BYTE},
    {"B01_D01_F0_0x214", 1, 1, 0, 0x214, US_REG_BYTE},
    {"B01_D01_F0_0x218", 1, 1, 0, 0x218, US_REG_BYTE},
    {"B01_D01_F0_0x21C", 1, 1, 0, 0x21C, US_REG_BYTE},
    {"B01_D01_F0_0x22", 1, 1, 0, 0x22, US_REG_WORD},
    {"B01_D01_F0_0x220", 1, 1, 0, 0x220, US_REG_BYTE},
    {"B01_D01_F0_0x224", 1, 1, 0, 0x224, US_REG_BYTE},
    {"B01_D01_F0_0x228", 1, 1, 0, 0x228, US_REG_BYTE},
    {"B01_D01_F0_0x230", 1, 1, 0, 0x230, US_REG_BYTE},
    {"B01_D01_F0_0x232", 1, 1, 0, 0x232, US_REG_BYTE},
    {"B01_D01_F0_0x24", 1, 1, 0, 0x24, US_REG_WORD},
    {"B01_D01_F0_0x258", 1, 1, 0, 0x258, US_REG_DWORD},
    {"B01_D01_F0_0x26", 1, 1, 0, 0x26, US_REG_WORD},
    {"B01_D01_F0_0x28", 1, 1, 0, 0x28, US_REG_DWORD},
    {"B01_D01_F0_0x288", 1, 1, 0, 0x288, US_REG_DWORD},
    {"B01_D01_F0_0x298", 1, 1, 0, 0x298, US_REG_DWORD},
    {"B01_D01_F0_0x2A4", 1, 1, 0, 0x2A4, US_REG_DWORD},
    {"B01_D01_F0_0x2C", 1, 1, 0, 0x2C, US_REG_DWORD},
    {"B01_D01_F0_0x4", 1, 1, 0, 0x4, US_REG_WORD},
    {"B01_D01_F0_0x6", 1, 1, 0, 0x6, US_REG_WORD},
    {"B01_D01_F0_0x9A", 1, 1, 0, 0x9A, US_REG_WORD},
    {"B01_D01_F0_0xA2", 1, 1, 0, 0xA2, US_REG_WORD},
    {"B01_D01_F0_0xAA", 1, 1, 0, 0xAA, US_REG_WORD},
    {"B01_D01_F0_0xC2", 1, 1, 0, 0xC2, US_REG_WORD},
    {"B01_D02_F0_0x0", 1, 2, 0, 0x0, US_REG_WORD},
    {"B01_D02_F0_0x14C", 1, 2, 0, 0x14C, US_REG_DWORD},
    {"B01_D02_F0_0x158", 1, 2, 0, 0x158, US_REG_DWORD},
    {"B01_D02_F0_0x178", 1, 2, 0, 0x178, US_REG_DWORD},
    {"B01_D02_F0_0x17C", 1, 2, 0, 0x17C, US_REG_DWORD},
    {"B01_D02_F0_0x18", 1, 2, 0, 0x18, US_REG_BYTE},
    {"B01_D02_F0_0x188", 1, 2, 0, 0x188, US_REG_DWORD},
    {"B01_D02_F0_0x18C", 1, 2, 0, 0x18C, US_REG_DWORD},
    {"B01_D02_F0_0x19", 1, 2, 0, 0x19, US_REG_BYTE},
    {"B01_D02_F0_0x1A", 1, 2, 0, 0x1A, US_REG_BYTE},
    {"B01_D02_F0_0x1C", 1, 2, 0, 0x1C, US_REG_BYTE},
    {"B01_D02_F0_0x1D", 1, 2, 0, 0x1D, US_REG_BYTE},
    {"B01_D02_F0_0x1E", 1, 2, 0, 0x1E, US_REG_WORD},
    {"B01_D02_F0_0x2", 1, 2, 0, 0x2, US_REG_WORD},
    {"B01_D02_F0_0x20", 1, 2, 0, 0x20, US_REG_WORD},
    {"B01_D02_F0_0x200", 1, 2, 0, 0x200, US_REG_BYTE},
    {"B01_D02_F0_0x204", 1, 2, 0, 0x204, US_REG_BYTE},
    {"B01_D02_F0_0x208", 1, 2, 0, 0x208, US_REG_BYTE},
    {"B01_D02_F0_0x20C", 1, 2, 0, 0x20C, US_REG_BYTE},
    {"B01_D02_F0_0x210", 1, 2, 0, 0x210, US_REG_BYTE},
    {"B01_D02_F0_0x214", 1, 2, 0, 0x214, US_REG_BYTE},
    {"B01_D02_F0_0x218", 1, 2, 0, 0x218, US_REG_BYTE},
    {"B01_D02_F0_0x21C", 1, 2, 0, 0x21C, US_REG_BYTE},
    {"B01_D02_F0_0x22", 1, 2, 0, 0x22, US_REG_WORD},
    {"B01_D02_F0_0x220", 1, 2, 0, 0x220, US_REG_BYTE},
    {"B01_D02_F0_0x224", 1, 2, 0, 0x224, US_REG_BYTE},
    {"B01_D02_F0_0x228", 1, 2, 0, 0x228, US_REG_BYTE},
    {"B01_D02_F0_0x230", 1, 2, 0, 0x230, US_REG_BYTE},
    {"B01_D02_F0_0x232", 1, 2, 0, 0x232, US_REG_BYTE},
    {"B01_D02_F0_0x24", 1, 2, 0, 0x24, US_REG_WORD},
    {"B01_D02_F0_0x258", 1, 2, 0, 0x258, US_REG_DWORD},
    {"B01_D02_F0_0x26", 1, 2, 0, 0x26, US_REG_WORD},
    {"B01_D02_F0_0x28", 1, 2, 0, 0x28, US_REG_DWORD},
    {"B01_D02_F0_0x288", 1, 2, 0, 0x288, US_REG_DWORD},
    {"B01_D02_F0_0x298", 1, 2, 0, 0x298, US_REG_DWORD},
    {"B01_D02_F0_0x2A4", 1, 2, 0, 0x2A4, US_REG_DWORD},
    {"B01_D02_F0_0x2C", 1, 2, 0, 0x2C, US_REG_DWORD},
    {"B01_D02_F0_0x4", 1, 2, 0, 0x4, US_REG_WORD},
    {"B01_D02_F0_0x6", 1, 2, 0, 0x6, US_REG_WORD},
    {"B01_D02_F0_0x9A", 1, 2, 0, 0x9A, US_REG_WORD},
    {"B01_D02_F0_0xA2", 1, 2, 0, 0xA2, US_REG_WORD},
    {"B01_D02_F0_0xAA", 1, 2, 0, 0xAA, US_REG_WORD},
    {"B01_D02_F0_0xC2", 1, 2, 0, 0xC2, US_REG_WORD},
    {"B01_D03_F0_0x0", 1, 3, 0, 0x0, US_REG_WORD},
    {"B01_D03_F0_0x158", 1, 3, 0, 0x158, US_REG_DWORD},
    {"B01_D03_F0_0x178", 1, 3, 0, 0x178, US_REG_DWORD},
    {"B01_D03_F0_0x17C", 1, 3, 0, 0x17C, US_REG_DWORD},
    {"B01_D03_F0_0x18", 1, 3, 0, 0x18, US_REG_BYTE},
    {"B01_D03_F0_0x188", 1, 3, 0, 0x188, US_REG_DWORD},
    {"B01_D03_F0_0x18C", 1, 3, 0, 0x18C, US_REG_DWORD},
    {"B01_D03_F0_0x19", 1, 3, 0, 0x19, US_REG_BYTE},
    {"B01_D03_F0_0x1A", 1, 3, 0, 0x1A, US_REG_BYTE},
    {"B01_D03_F0_0x1C", 1, 3, 0, 0x1C, US_REG_BYTE},
    {"B01_D03_F0_0x1D", 1, 3, 0, 0x1D, US_REG_BYTE},
    {"B01_D03_F0_0x1E", 1, 3, 0, 0x1E, US_REG_WORD},
    {"B01_D03_F0_0x2", 1, 3, 0, 0x2, US_REG_WORD},
    {"B01_D03_F0_0x20", 1, 3, 0, 0x20, US_REG_WORD},
    {"B01_D03_F0_0x200", 1, 3, 0, 0x200, US_REG_BYTE},
    {"B01_D03_F0_0x204", 1, 3, 0, 0x204, US_REG_BYTE},
    {"B01_D03_F0_0x208", 1, 3, 0, 0x208, US_REG_BYTE},
    {"B01_D03_F0_0x20C", 1, 3, 0, 0x20C, US_REG_BYTE},
    {"B01_D03_F0_0x210", 1, 3, 0, 0x210, US_REG_BYTE},
    {"B01_D03_F0_0x214", 1, 3, 0, 0x214, US_REG_BYTE},
    {"B01_D03_F0_0x218", 1, 3, 0, 0x218, US_REG_BYTE},
    {"B01_D03_F0_0x21C", 1, 3, 0, 0x21C, US_REG_BYTE},
    {"B01_D03_F0_0x22", 1, 3, 0, 0x22, US_REG_WORD},
    {"B01_D03_F0_0x220", 1, 3, 0, 0x220, US_REG_BYTE},
    {"B01_D03_F0_0x224", 1, 3, 0, 0x224, US_REG_BYTE},
    {"B01_D03_F0_0x228", 1, 3, 0, 0x228, US_REG_BYTE},
    {"B01_D03_F0_0x230", 1, 3, 0, 0x230, US_REG_BYTE},
    {"B01_D03_F0_0x232", 1, 3, 0, 0x232, US_REG_BYTE},
    {"B01_D03_F0_0x24", 1, 3, 0, 0x24, US_REG_WORD},
    {"B01_D03_F0_0x258", 1, 3, 0, 0x258, US_REG_DWORD},
    {"B01_D03_F0_0x26", 1, 3, 0, 0x26, US_REG_WORD},
    {"B01_D03_F0_0x28", 1, 3, 0, 0x28, US_REG_DWORD},
    {"B01_D03_F0_0x288", 1, 3, 0, 0x288, US_REG_DWORD},
    {"B01_D03_F0_0x298", 1, 3, 0, 0x298, US_REG_DWORD},
    {"B01_D03_F0_0x2A4", 1, 3, 0, 0x2A4, US_REG_DWORD},
    {"B01_D03_F0_0x2C", 1, 3, 0, 0x2C, US_REG_DWORD},
    {"B01_D03_F0_0x4", 1, 3, 0, 0x4, US_REG_WORD},
    {"B01_D03_F0_0x6", 1, 3, 0, 0x6, US_REG_WORD},
    {"B01_D03_F0_0x9A", 1, 3, 0, 0x9A, US_REG_WORD},
    {"B01_D03_F0_0xA2", 1, 3, 0, 0xA2, US_REG_WORD},
    {"B01_D03_F0_0xAA", 1, 3, 0, 0xAA, US_REG_WORD},
    {"B01_D03_F0_0xC2", 1, 3, 0, 0xC2, US_REG_WORD},
    {"B01_D05_F0_0x1A8", 1, 5, 0, 0x1A8, US_REG_DWORD},
    {"B01_D05_F0_0x1B4", 1, 5, 0, 0x1B4, US_REG_DWORD},
    {"B01_D05_F2_0x0", 1, 5, 2, 0x0, US_REG_WORD},
    {"B01_D05_F2_0x19C", 1, 5, 2, 0x19C, US_REG_DWORD},
    {"B01_D05_F2_0x1A0", 1, 5, 2, 0x1A0, US_REG_DWORD},
    {"B01_D05_F2_0x1A4", 1, 5, 2, 0x1A4, US_REG_DWORD},
    {"B01_D05_F2_0x1A8", 1, 5, 2, 0x1A8, US_REG_DWORD},
    {"B01_D05_F2_0x1AC", 1, 5, 2, 0x1AC, US_REG_DWORD},
    {"B01_D05_F2_0x1B4", 1, 5, 2, 0x1B4, US_REG_DWORD},
    {"B01_D05_F2_0x1C0", 1, 5, 2, 0x1C0, US_REG_DWORD},
    {"B01_D05_F2_0x1C4", 1, 5, 2, 0x1C4, US_REG_DWORD},
    {"B01_D05_F2_0x1C8", 1, 5, 2, 0x1C8, US_REG_DWORD},
    {"B01_D05_F2_0x1CC", 1, 5, 2, 0x1CC, US_REG_DWORD},
    {"B01_D05_F2_0x1D0", 1, 5, 2, 0x1D0, US_REG_DWORD},
    {"B01_D05_F2_0x1D4", 1, 5, 2, 0x1D4, US_REG_DWORD},
    {"B01_D05_F2_0x1D8", 1, 5, 2, 0x1D8, US_REG_DWORD},
    {"B01_D05_F2_0x1DC", 1, 5, 2, 0x1DC, US_REG_DWORD},
    {"B01_D05_F2_0x1E8", 1, 5, 2, 0x1E8, US_REG_DWORD},
    {"B01_D05_F2_0x1EC", 1, 5, 2, 0x1EC, US_REG_DWORD},
    {"B01_D05_F2_0x1F8", 1, 5, 2, 0x1F8, US_REG_DWORD},
    {"B01_D05_F2_0x2", 1, 5, 2, 0x2, US_REG_WORD},
    {"B01_D05_F2_0x200", 1, 5, 2, 0x200, US_REG_DWORD},
    {"B01_D05_F2_0x208", 1, 5, 2, 0x208, US_REG_DWORD},
    {"B01_D05_F2_0x20C", 1, 5, 2, 0x20C, US_REG_DWORD},
    {"B01_D05_F2_0x210", 1, 5, 2, 0x210, US_REG_DWORD},
    {"B01_D05_F2_0x214", 1, 5, 2, 0x214, US_REG_DWORD},
    {"B01_D05_F2_0x218", 1, 5, 2, 0x218, US_REG_DWORD},
    {"B01_D05_F2_0x21C", 1, 5, 2, 0x21C, US_REG_DWORD},
    {"B01_D05_F2_0x220", 1, 5, 2, 0x220, US_REG_DWORD},
    {"B01_D05_F2_0x224", 1, 5, 2, 0x224, US_REG_DWORD},
    {"B01_D05_F2_0x228", 1, 5, 2, 0x228, US_REG_DWORD},
    {"B01_D05_F2_0x230", 1, 5, 2, 0x230, US_REG_DWORD},
    {"B01_D05_F2_0x234", 1, 5, 2, 0x234, US_REG_DWORD},
    {"B01_D05_F2_0x238", 1, 5, 2, 0x238, US_REG_DWORD},
    {"B01_D05_F2_0x23C", 1, 5, 2, 0x23C, US_REG_DWORD},
    {"B01_D05_F2_0x240", 1, 5, 2, 0x240, US_REG_DWORD},
    {"B01_D05_F2_0x244", 1, 5, 2, 0x244, US_REG_DWORD},
    {"B01_D05_F2_0x248", 1, 5, 2, 0x248, US_REG_DWORD},
    {"B01_D05_F2_0x24C", 1, 5, 2, 0x24C, US_REG_DWORD},
    {"B01_D05_F2_0x250", 1, 5, 2, 0x250, US_REG_DWORD},
    {"B01_D05_F2_0x254", 1, 5, 2, 0x254, US_REG_DWORD},
    {"B01_D05_F2_0x268", 1, 5, 2, 0x268, US_REG_DWORD},
    {"B01_D05_F2_0x26C", 1, 5, 2, 0x26C, US_REG_DWORD},
    {"B01_D05_F2_0x270", 1, 5, 2, 0x270, US_REG_DWORD},
    {"B01_D05_F2_0x274", 1, 5, 2, 0x274, US_REG_DWORD},
    {"B01_D05_F2_0x278", 1, 5, 2, 0x278, US_REG_DWORD},
    {"B01_D05_F2_0x288", 1, 5, 2, 0x288, US_REG_DWORD},
    {"B01_D05_F2_0x28C", 1, 5, 2, 0x28C, US_REG_DWORD},
    {"B01_D05_F2_0x290", 1, 5, 2, 0x290, US_REG_DWORD},
    {"B01_D05_F2_0x294", 1, 5, 2, 0x294, US_REG_DWORD},
    {"B01_D05_F2_0x298", 1, 5, 2, 0x298, US_REG_DWORD},
    {"B01_D05_F2_0x29C", 1, 5, 2, 0x29C, US_REG_DWORD},
    {"B01_D05_F2_0x2A0", 1, 5, 2, 0x2A0, US_REG_DWORD},
    {"B01_D05_F2_0x2A4", 1, 5, 2, 0x2A4, US_REG_DWORD},
    {"B01_D05_F2_0x2A8", 1, 5, 2, 0x2A8, US_REG_DWORD},
    {"B01_D05_F2_0x2AC", 1, 5, 2, 0x2AC, US_REG_DWORD},
    {"B01_D05_F2_0x2B0", 1, 5, 2, 0x2B0, US_REG_DWORD},
    {"B01_D05_F2_0x2B4", 1, 5, 2, 0x2B4, US_REG_DWORD},
    {"B01_D05_F2_0x2B8", 1, 5, 2, 0x2B8, US_REG_DWORD},
    {"B01_D05_F2_0x2CC", 1, 5, 2, 0x2CC, US_REG_DWORD},
    {"B01_D05_F2_0x2D0", 1, 5, 2, 0x2D0, US_REG_DWORD},
    {"B01_D05_F2_0x2D4", 1, 5, 2, 0x2D4, US_REG_DWORD},
    {"B01_D05_F2_0x2D8", 1, 5, 2, 0x2D8, US_REG_DWORD},
    {"B01_D05_F2_0x2DC", 1, 5, 2, 0x2DC, US_REG_DWORD},
    {"B01_D05_F2_0x2E0", 1, 5, 2, 0x2E0, US_REG_DWORD},
    {"B01_D05_F2_0x2E4", 1, 5, 2, 0x2E4, US_REG_DWORD},
    {"B01_D05_F2_0x2E8", 1, 5, 2, 0x2E8, US_REG_DWORD},
    {"B01_D05_F2_0x2EC", 1, 5, 2, 0x2EC, US_REG_DWORD},
    {"B01_D05_F2_0x2FC", 1, 5, 2, 0x2FC, US_REG_DWORD},
    {"B01_D05_F2_0x304", 1, 5, 2, 0x304, US_REG_DWORD},
    {"B01_D05_F2_0x4", 1, 5, 2, 0x4, US_REG_WORD},
    {"B01_D05_F2_0x6", 1, 5, 2, 0x6, US_REG_WORD},
    {"B01_D05_F2_0x80", 1, 5, 2, 0x80, US_REG_DWORD},
    {"B01_D05_F2_0x94", 1, 5, 2, 0x94, US_REG_DWORD},
    {"B01_D05_F2_0xA0", 1, 5, 2, 0xA0, US_REG_DWORD},
    {"B01_D07_F0_0x0", 1, 7, 0, 0x0, US_REG_WORD},
    {"B01_D07_F0_0x2", 1, 7, 0, 0x2, US_REG_WORD},
    {"B01_D07_F0_0x4", 1, 7, 0, 0x4, US_REG_WORD},
    {"B01_D07_F0_0x4A", 1, 7, 0, 0x4A, US_REG_WORD},
    {"B01_D07_F0_0x52", 1, 7, 0, 0x52, US_REG_WORD},
    {"B01_D07_F0_0x6", 1, 7, 0, 0x6, US_REG_WORD},
    {"B01_D07_F0_0x72", 1, 7, 0, 0x72, US_REG_WORD},
    {"B01_D07_F0_0x7A", 1, 7, 0, 0x7A, US_REG_WORD},
    {"B01_D07_F1_0x0", 1, 7, 1, 0x0, US_REG_WORD},
    {"B01_D07_F1_0x2", 1, 7, 1, 0x2, US_REG_WORD},
    {"B01_D07_F1_0x4", 1, 7, 1, 0x4, US_REG_WORD},
    {"B01_D07_F1_0x4A", 1, 7, 1, 0x4A, US_REG_WORD},
    {"B01_D07_F1_0x52", 1, 7, 1, 0x52, US_REG_WORD},
    {"B01_D07_F1_0x6", 1, 7, 1, 0x6, US_REG_WORD},
    {"B01_D07_F1_0x72", 1, 7, 1, 0x72, US_REG_WORD},
    {"B01_D07_F1_0x7A", 1, 7, 1, 0x7A, US_REG_WORD},
    {"B01_D07_F2_0x0", 1, 7, 2, 0x0, US_REG_WORD},
    {"B01_D07_F2_0x2", 1, 7, 2, 0x2, US_REG_WORD},
    {"B01_D07_F2_0x4", 1, 7, 2, 0x4, US_REG_WORD},
    {"B01_D07_F2_0x4A", 1, 7, 2, 0x4A, US_REG_WORD},
    {"B01_D07_F2_0x52", 1, 7, 2, 0x52, US_REG_WORD},
    {"B01_D07_F2_0x6", 1, 7, 2, 0x6, US_REG_WORD},
    {"B01_D07_F2_0x72", 1, 7, 2, 0x72, US_REG_WORD},
    {"B01_D07_F2_0x7A", 1, 7, 2, 0x7A, US_REG_WORD},
    {"B01_D07_F3_0x0", 1, 7, 3, 0x0, US_REG_WORD},
    {"B01_D07_F3_0x2", 1, 7, 3, 0x2, US_REG_WORD},
    {"B01_D07_F3_0x4", 1, 7, 3, 0x4, US_REG_WORD},
    {"B01_D07_F3_0x4A", 1, 7, 3, 0x4A, US_REG_WORD},
    {"B01_D07_F3_0x52", 1, 7, 3, 0x52, US_REG_WORD},
    {"B01_D07_F3_0x6", 1, 7, 3, 0x6, US_REG_WORD},
    {"B01_D07_F3_0x72", 1, 7, 3, 0x72, US_REG_WORD},
    {"B01_D07_F3_0x7A", 1, 7, 3, 0x7A, US_REG_WORD},
    {"B01_D07_F4_0x0", 1, 7, 4, 0x0, US_REG_WORD},
    {"B01_D07_F4_0x2", 1, 7, 4, 0x2, US_REG_WORD},
    {"B01_D07_F4_0x4", 1, 7, 4, 0x4, US_REG_WORD},
    {"B01_D07_F4_0x4A", 1, 7, 4, 0x4A, US_REG_WORD},
    {"B01_D07_F4_0x52", 1, 7, 4, 0x52, US_REG_WORD},
    {"B01_D07_F4_0x6", 1, 7, 4, 0x6, US_REG_WORD},
    {"B01_D07_F4_0x72", 1, 7, 4, 0x72, US_REG_WORD},
    {"B01_D07_F4_0x7A", 1, 7, 4, 0x7A, US_REG_WORD},
    {"B01_D07_F7_0x0", 1, 7, 7, 0x0, US_REG_WORD},
    {"B01_D07_F7_0x2", 1, 7, 7, 0x2, US_REG_WORD},
    {"B01_D07_F7_0x358", 1, 7, 7, 0x358, US_REG_DWORD},
    {"B01_D07_F7_0x4", 1, 7, 7, 0x4, US_REG_WORD},
    {"B01_D07_F7_0x4A", 1, 7, 7, 0x4A, US_REG_WORD},
    {"B01_D07_F7_0x52", 1, 7, 7, 0x52, US_REG_WORD},
    {"B01_D07_F7_0x6", 1, 7, 7, 0x6, US_REG_WORD},
    {"B01_D07_F7_0x72", 1, 7, 7, 0x72, US_REG_WORD},
    {"B01_D07_F7_0x7A", 1, 7, 7, 0x7A, US_REG_WORD},
    {"B01_D30_F2_0xEC", 1, 30, 2, 0xEC, US_REG_DWORD},
    {"B01_D30_F3_0x9C", 1, 30, 3, 0x9C, US_REG_DWORD},
    {"B01_D30_F5_0xA8", 1, 30, 5, 0xA8, US_REG_DWORD},
    {"B01_D30_F5_0xAC", 1, 30, 5, 0xAC, US_REG_DWORD},
    {"B01_D30_F5_0xB0", 1, 30, 5, 0xB0, US_REG_DWORD},
    {"B02_D00_F0_0x0", 2, 0, 0, 0x0, US_REG_WORD},
    {"B02_D00_F0_0x14C", 2, 0, 0, 0x14C, US_REG_DWORD},
    {"B02_D00_F0_0x158", 2, 0, 0, 0x158, US_REG_DWORD},
    {"B02_D00_F0_0x178", 2, 0, 0, 0x178, US_REG_DWORD},
    {"B02_D00_F0_0x17C", 2, 0, 0, 0x17C, US_REG_DWORD},
    {"B02_D00_F0_0x18", 2, 0, 0, 0x18, US_REG_BYTE},
    {"B02_D00_F0_0x188", 2, 0, 0, 0x188, US_REG_DWORD},
    {"B02_D00_F0_0x18C", 2, 0, 0, 0x18C, US_REG_DWORD},
    {"B02_D00_F0_0x19", 2, 0, 0, 0x19, US_REG_BYTE},
    {"B02_D00_F0_0x1A", 2, 0, 0, 0x1A, US_REG_BYTE},
    {"B02_D00_F0_0x1A2", 2, 0, 0, 0x1A2, US_REG_WORD},
    {"B02_D00_F0_0x1AA", 2, 0, 0, 0x1AA, US_REG_WORD},
    {"B02_D00_F0_0x1C", 2, 0, 0, 0x1C, US_REG_BYTE},
    {"B02_D00_F0_0x1C2", 2, 0, 0, 0x1C2, US_REG_WORD},
    {"B02_D00_F0_0x1D", 2, 0, 0, 0x1D, US_REG_BYTE},
    {"B02_D00_F0_0x1E", 2, 0, 0, 0x1E, US_REG_WORD},
    {"B02_D00_F0_0x2", 2, 0, 0, 0x2, US_REG_WORD},
    {"B02_D00_F0_0x20", 2, 0, 0, 0x20, US_REG_WORD},
    {"B02_D00_F0_0x200", 2, 0, 0, 0x200, US_REG_BYTE},
    {"B02_D00_F0_0x204", 2, 0, 0, 0x204, US_REG_BYTE},
    {"B02_D00_F0_0x208", 2, 0, 0, 0x208, US_REG_BYTE},
    {"B02_D00_F0_0x20C", 2, 0, 0, 0x20C, US_REG_BYTE},
    {"B02_D00_F0_0x210", 2, 0, 0, 0x210, US_REG_BYTE},
    {"B02_D00_F0_0x214", 2, 0, 0, 0x214, US_REG_BYTE},
    {"B02_D00_F0_0x218", 2, 0, 0, 0x218, US_REG_BYTE},
    {"B02_D00_F0_0x21C", 2, 0, 0, 0x21C, US_REG_BYTE},
    {"B02_D00_F0_0x22", 2, 0, 0, 0x22, US_REG_WORD},
    {"B02_D00_F0_0x220", 2, 0, 0, 0x220, US_REG_BYTE},
    {"B02_D00_F0_0x224", 2, 0, 0, 0x224, US_REG_BYTE},
    {"B02_D00_F0_0x228", 2, 0, 0, 0x228, US_REG_BYTE},
    {"B02_D00_F0_0x230", 2, 0, 0, 0x230, US_REG_BYTE},
    {"B02_D00_F0_0x232", 2, 0, 0, 0x232, US_REG_BYTE},
    {"B02_D00_F0_0x24", 2, 0, 0, 0x24, US_REG_WORD},
    {"B02_D00_F0_0x258", 2, 0, 0, 0x258, US_REG_DWORD},
    {"B02_D00_F0_0x26", 2, 0, 0, 0x26, US_REG_WORD},
    {"B02_D00_F0_0x28", 2, 0, 0, 0x28, US_REG_DWORD},
    {"B02_D00_F0_0x288", 2, 0, 0, 0x288, US_REG_DWORD},
    {"B02_D00_F0_0x298", 2, 0, 0, 0x298, US_REG_DWORD},
    {"B02_D00_F0_0x2A4", 2, 0, 0, 0x2A4, US_REG_DWORD},
    {"B02_D00_F0_0x2C", 2, 0, 0, 0x2C, US_REG_DWORD},
    {"B02_D00_F0_0x4", 2, 0, 0, 0x4, US_REG_WORD},
    {"B02_D00_F0_0x6", 2, 0, 0, 0x6, US_REG_WORD},
    {"B02_D00_F0_0x9A", 2, 0, 0, 0x9A, US_REG_WORD},
    {"B02_D00_F0_0xA2", 2, 0, 0, 0xA2, US_REG_WORD},
    {"B02_D00_F0_0xAA", 2, 0, 0, 0xAA, US_REG_WORD},
    {"B02_D00_F0_0xC2", 2, 0, 0, 0xC2, US_REG_WORD},
    {"B02_D01_F0_0x0", 2, 1, 0, 0x0, US_REG_WORD},
    {"B02_D01_F0_0x14C", 2, 1, 0, 0x14C, US_REG_DWORD},
    {"B02_D01_F0_0x158", 2, 1, 0, 0x158, US_REG_DWORD},
    {"B02_D01_F0_0x178", 2, 1, 0, 0x178, US_REG_DWORD},
    {"B02_D01_F0_0x17C", 2, 1, 0, 0x17C, US_REG_DWORD},
    {"B02_D01_F0_0x18", 2, 1, 0, 0x18, US_REG_BYTE},
    {"B02_D01_F0_0x188", 2, 1, 0, 0x188, US_REG_DWORD},
    {"B02_D01_F0_0x18C", 2, 1, 0, 0x18C, US_REG_DWORD},
    {"B02_D01_F0_0x19", 2, 1, 0, 0x19, US_REG_BYTE},
    {"B02_D01_F0_0x1A", 2, 1, 0, 0x1A, US_REG_BYTE},
    {"B02_D01_F0_0x1C", 2, 1, 0, 0x1C, US_REG_BYTE},
    {"B02_D01_F0_0x1D", 2, 1, 0, 0x1D, US_REG_BYTE},
    {"B02_D01_F0_0x1E", 2, 1, 0, 0x1E, US_REG_WORD},
    {"B02_D01_F0_0x2", 2, 1, 0, 0x2, US_REG_WORD},
    {"B02_D01_F0_0x20", 2, 1, 0, 0x20, US_REG_WORD},
    {"B02_D01_F0_0x200", 2, 1, 0, 0x200, US_REG_BYTE},
    {"B02_D01_F0_0x204", 2, 1, 0, 0x204, US_REG_BYTE},
    {"B02_D01_F0_0x208", 2, 1, 0, 0x208, US_REG_BYTE},
    {"B02_D01_F0_0x20C", 2, 1, 0, 0x20C, US_REG_BYTE},
    {"B02_D01_F0_0x210", 2, 1, 0, 0x210, US_REG_BYTE},
    {"B02_D01_F0_0x214", 2, 1, 0, 0x214, US_REG_BYTE},
    {"B02_D01_F0_0x218", 2, 1, 0, 0x218, US_REG_BYTE},
    {"B02_D01_F0_0x21C", 2, 1, 0, 0x21C, US_REG_BYTE},
    {"B02_D01_F0_0x22", 2, 1, 0, 0x22, US_REG_WORD},
    {"B02_D01_F0_0x220", 2, 1, 0, 0x220, US_REG_BYTE},
    {"B02_D01_F0_0x224", 2, 1, 0, 0x224, US_REG_BYTE},
    {"B02_D01_F0_0x228", 2, 1, 0, 0x228, US_REG_BYTE},
    {"B02_D01_F0_0x230", 2, 1, 0, 0x230, US_REG_BYTE},
    {"B02_D01_F0_0x232", 2, 1, 0, 0x232, US_REG_BYTE},
    {"B02_D01_F0_0x24", 2, 1, 0, 0x24, US_REG_WORD},
    {"B02_D01_F0_0x258", 2, 1, 0, 0x258, US_REG_DWORD},
    {"B02_D01_F0_0x26", 2, 1, 0, 0x26, US_REG_WORD},
    {"B02_D01_F0_0x28", 2, 1, 0, 0x28, US_REG_DWORD},
    {"B02_D01_F0_0x288", 2, 1, 0, 0x288, US_REG_DWORD},
    {"B02_D01_F0_0x298", 2, 1, 0, 0x298, US_REG_DWORD},
    {"B02_D01_F0_0x2A4", 2, 1, 0, 0x2A4, US_REG_DWORD},
    {"B02_D01_F0_0x2C", 2, 1, 0, 0x2C, US_REG_DWORD},
    {"B02_D01_F0_0x4", 2, 1, 0, 0x4, US_REG_WORD},
    {"B02_D01_F0_0x6", 2, 1, 0, 0x6, US_REG_WORD},
    {"B02_D01_F0_0x9A", 2, 1, 0, 0x9A, US_REG_WORD},
    {"B02_D01_F0_0xA2", 2, 1, 0, 0xA2, US_REG_WORD},
    {"B02_D01_F0_0xAA", 2, 1, 0, 0xAA, US_REG_WORD},
    {"B02_D01_F0_0xC2", 2, 1, 0, 0xC2, US_REG_WORD},
    {"B02_D02_F0_0x0", 2, 2, 0, 0x0, US_REG_WORD},
    {"B02_D02_F0_0x14C", 2, 2, 0, 0x14C, US_REG_DWORD},
    {"B02_D02_F0_0x158", 2, 2, 0, 0x158, US_REG_DWORD},
    {"B02_D02_F0_0x178", 2, 2, 0, 0x178, US_REG_DWORD},
    {"B02_D02_F0_0x17C", 2, 2, 0, 0x17C, US_REG_DWORD},
    {"B02_D02_F0_0x18", 2, 2, 0, 0x18, US_REG_BYTE},
    {"B02_D02_F0_0x188", 2, 2, 0, 0x188, US_REG_DWORD},
    {"B02_D02_F0_0x18C", 2, 2, 0, 0x18C, US_REG_DWORD},
    {"B02_D02_F0_0x19", 2, 2, 0, 0x19, US_REG_BYTE},
    {"B02_D02_F0_0x1A", 2, 2, 0, 0x1A, US_REG_BYTE},
    {"B02_D02_F0_0x1C", 2, 2, 0, 0x1C, US_REG_BYTE},
    {"B02_D02_F0_0x1D", 2, 2, 0, 0x1D, US_REG_BYTE},
    {"B02_D02_F0_0x1E", 2, 2, 0, 0x1E, US_REG_WORD},
    {"B02_D02_F0_0x2", 2, 2, 0, 0x2, US_REG_WORD},
    {"B02_D02_F0_0x20", 2, 2, 0, 0x20, US_REG_WORD},
    {"B02_D02_F0_0x200", 2, 2, 0, 0x200, US_REG_BYTE},
    {"B02_D02_F0_0x204", 2, 2, 0, 0x204, US_REG_BYTE},
    {"B02_D02_F0_0x208", 2, 2, 0, 0x208, US_REG_BYTE},
    {"B02_D02_F0_0x20C", 2, 2, 0, 0x20C, US_REG_BYTE},
    {"B02_D02_F0_0x210", 2, 2, 0, 0x210, US_REG_BYTE},
    {"B02_D02_F0_0x214", 2, 2, 0, 0x214, US_REG_BYTE},
    {"B02_D02_F0_0x218", 2, 2, 0, 0x218, US_REG_BYTE},
    {"B02_D02_F0_0x21C", 2, 2, 0, 0x21C, US_REG_BYTE},
    {"B02_D02_F0_0x22", 2, 2, 0, 0x22, US_REG_WORD},
    {"B02_D02_F0_0x220", 2, 2, 0, 0x220, US_REG_BYTE},
    {"B02_D02_F0_0x224", 2, 2, 0, 0x224, US_REG_BYTE},
    {"B02_D02_F0_0x228", 2, 2, 0, 0x228, US_REG_BYTE},
    {"B02_D02_F0_0x230", 2, 2, 0, 0x230, US_REG_BYTE},
    {"B02_D02_F0_0x232", 2, 2, 0, 0x232, US_REG_BYTE},
    {"B02_D02_F0_0x24", 2, 2, 0, 0x24, US_REG_WORD},
    {"B02_D02_F0_0x258", 2, 2, 0, 0x258, US_REG_DWORD},
    {"B02_D02_F0_0x26", 2, 2, 0, 0x26, US_REG_WORD},
    {"B02_D02_F0_0x28", 2, 2, 0, 0x28, US_REG_DWORD},
    {"B02_D02_F0_0x288", 2, 2, 0, 0x288, US_REG_DWORD},
    {"B02_D02_F0_0x298", 2, 2, 0, 0x298, US_REG_DWORD},
    {"B02_D02_F0_0x2A4", 2, 2, 0, 0x2A4, US_REG_DWORD},
    {"B02_D02_F0_0x2C", 2, 2, 0, 0x2C, US_REG_DWORD},
    {"B02_D02_F0_0x4", 2, 2, 0, 0x4, US_REG_WORD},
    {"B02_D02_F0_0x6", 2, 2, 0, 0x6, US_REG_WORD},
    {"B02_D02_F0_0x9A", 2, 2, 0, 0x9A, US_REG_WORD},
    {"B02_D02_F0_0xA2", 2, 2, 0, 0xA2, US_REG_WORD},
    {"B02_D02_F0_0xAA", 2, 2, 0, 0xAA, US_REG_WORD},
    {"B02_D02_F0_0xC2", 2, 2, 0, 0xC2, US_REG_WORD},
    {"B02_D03_F0_0x0", 2, 3, 0, 0x0, US_REG_WORD},
    {"B02_D03_F0_0x14C", 2, 3, 0, 0x14C, US_REG_DWORD},
    {"B02_D03_F0_0x158", 2, 3, 0, 0x158, US_REG_DWORD},
    {"B02_D03_F0_0x178", 2, 3, 0, 0x178, US_REG_DWORD},
    {"B02_D03_F0_0x17C", 2, 3, 0, 0x17C, US_REG_DWORD},
    {"B02_D03_F0_0x18", 2, 3, 0, 0x18, US_REG_BYTE},
    {"B02_D03_F0_0x188", 2, 3, 0, 0x188, US_REG_DWORD},
    {"B02_D03_F0_0x18C", 2, 3, 0, 0x18C, US_REG_DWORD},
    {"B02_D03_F0_0x19", 2, 3, 0, 0x19, US_REG_BYTE},
    {"B02_D03_F0_0x1A", 2, 3, 0, 0x1A, US_REG_BYTE},
    {"B02_D03_F0_0x1C", 2, 3, 0, 0x1C, US_REG_BYTE},
    {"B02_D03_F0_0x1D", 2, 3, 0, 0x1D, US_REG_BYTE},
    {"B02_D03_F0_0x1E", 2, 3, 0, 0x1E, US_REG_WORD},
    {"B02_D03_F0_0x2", 2, 3, 0, 0x2, US_REG_WORD},
    {"B02_D03_F0_0x20", 2, 3, 0, 0x20, US_REG_WORD},
    {"B02_D03_F0_0x200", 2, 3, 0, 0x200, US_REG_BYTE},
    {"B02_D03_F0_0x204", 2, 3, 0, 0x204, US_REG_BYTE},
    {"B02_D03_F0_0x208", 2, 3, 0, 0x208, US_REG_BYTE},
    {"B02_D03_F0_0x20C", 2, 3, 0, 0x20C, US_REG_BYTE},
    {"B02_D03_F0_0x210", 2, 3, 0, 0x210, US_REG_BYTE},
    {"B02_D03_F0_0x214", 2, 3, 0, 0x214, US_REG_BYTE},
    {"B02_D03_F0_0x218", 2, 3, 0, 0x218, US_REG_BYTE},
    {"B02_D03_F0_0x21C", 2, 3, 0, 0x21C, US_REG_BYTE},
    {"B02_D03_F0_0x22", 2, 3, 0, 0x22, US_REG_WORD},
    {"B02_D03_F0_0x220", 2, 3, 0, 0x220, US_REG_BYTE},
    {"B02_D03_F0_0x224", 2, 3, 0, 0x224, US_REG_BYTE},
    {"B02_D03_F0_0x228", 2, 3, 0, 0x228, US_REG_BYTE},
    {"B02_D03_F0_0x230", 2, 3, 0, 0x230, US_REG_BYTE},
    {"B02_D03_F0_0x232", 2, 3, 0, 0x232, US_REG_BYTE},
    {"B02_D03_F0_0x24", 2, 3, 0, 0x24, US_REG_WORD},
    {"B02_D03_F0_0x258", 2, 3, 0, 0x258, US_REG_DWORD},
    {"B02_D03_F0_0x26", 2, 3, 0, 0x26, US_REG_WORD},
    {"B02_D03_F0_0x28", 2, 3, 0, 0x28, US_REG_DWORD},
    {"B02_D03_F0_0x288", 2, 3, 0, 0x288, US_REG_DWORD},
    {"B02_D03_F0_0x298", 2, 3, 0, 0x298, US_REG_DWORD},
    {"B02_D03_F0_0x2A4", 2, 3, 0, 0x2A4, US_REG_DWORD},
    {"B02_D03_F0_0x2C", 2, 3, 0, 0x2C, US_REG_DWORD},
    {"B02_D03_F0_0x4", 2, 3, 0, 0x4, US_REG_WORD},
    {"B02_D03_F0_0x6", 2, 3, 0, 0x6, US_REG_WORD},
    {"B02_D03_F0_0x9A", 2, 3, 0, 0x9A, US_REG_WORD},
    {"B02_D03_F0_0xA2", 2, 3, 0, 0xA2, US_REG_WORD},
    {"B02_D03_F0_0xAA", 2, 3, 0, 0xAA, US_REG_WORD},
    {"B02_D03_F0_0xC2", 2, 3, 0, 0xC2, US_REG_WORD},
    {"B02_D05_F0_0x1A8", 2, 5, 0, 0x1A8, US_REG_DWORD},
    {"B02_D05_F0_0x1B4", 2, 5, 0, 0x1B4, US_REG_DWORD},
    {"B02_D05_F2_0x0", 2, 5, 2, 0x0, US_REG_WORD},
    {"B02_D05_F2_0x19C", 2, 5, 2, 0x19C, US_REG_DWORD},
    {"B02_D05_F2_0x1A0", 2, 5, 2, 0x1A0, US_REG_DWORD},
    {"B02_D05_F2_0x1A4", 2, 5, 2, 0x1A4, US_REG_DWORD},
    {"B02_D05_F2_0x1A8", 2, 5, 2, 0x1A8, US_REG_DWORD},
    {"B02_D05_F2_0x1AC", 2, 5, 2, 0x1AC, US_REG_DWORD},
    {"B02_D05_F2_0x1B4", 2, 5, 2, 0x1B4, US_REG_DWORD},
    {"B02_D05_F2_0x1C0", 2, 5, 2, 0x1C0, US_REG_DWORD},
    {"B02_D05_F2_0x1C4", 2, 5, 2, 0x1C4, US_REG_DWORD},
    {"B02_D05_F2_0x1C8", 2, 5, 2, 0x1C8, US_REG_DWORD},
    {"B02_D05_F2_0x1CC", 2, 5, 2, 0x1CC, US_REG_DWORD},
    {"B02_D05_F2_0x1D0", 2, 5, 2, 0x1D0, US_REG_DWORD},
    {"B02_D05_F2_0x1D4", 2, 5, 2, 0x1D4, US_REG_DWORD},
    {"B02_D05_F2_0x1D8", 2, 5, 2, 0x1D8, US_REG_DWORD},
    {"B02_D05_F2_0x1DC", 2, 5, 2, 0x1DC, US_REG_DWORD},
    {"B02_D05_F2_0x1E8", 2, 5, 2, 0x1E8, US_REG_DWORD},
    {"B02_D05_F2_0x1EC", 2, 5, 2, 0x1EC, US_REG_DWORD},
    {"B02_D05_F2_0x1F8", 2, 5, 2, 0x1F8, US_REG_DWORD},
    {"B02_D05_F2_0x2", 2, 5, 2, 0x2, US_REG_WORD},
    {"B02_D05_F2_0x200", 2, 5, 2, 0x200, US_REG_DWORD},
    {"B02_D05_F2_0x208", 2, 5, 2, 0x208, US_REG_DWORD},
    {"B02_D05_F2_0x20C", 2, 5, 2, 0x20C, US_REG_DWORD},
    {"B02_D05_F2_0x210", 2, 5, 2, 0x210, US_REG_DWORD},
    {"B02_D05_F2_0x214", 2, 5, 2, 0x214, US_REG_DWORD},
    {"B02_D05_F2_0x218", 2, 5, 2, 0x218, US_REG_DWORD},
    {"B02_D05_F2_0x21C", 2, 5, 2, 0x21C, US_REG_DWORD},
    {"B02_D05_F2_0x220", 2, 5, 2, 0x220, US_REG_DWORD},
    {"B02_D05_F2_0x224", 2, 5, 2, 0x224, US_REG_DWORD},
    {"B02_D05_F2_0x228", 2, 5, 2, 0x228, US_REG_DWORD},
    {"B02_D05_F2_0x230", 2, 5, 2, 0x230, US_REG_DWORD},
    {"B02_D05_F2_0x234", 2, 5, 2, 0x234, US_REG_DWORD},
    {"B02_D05_F2_0x238", 2, 5, 2, 0x238, US_REG_DWORD},
    {"B02_D05_F2_0x23C", 2, 5, 2, 0x23C, US_REG_DWORD},
    {"B02_D05_F2_0x240", 2, 5, 2, 0x240, US_REG_DWORD},
    {"B02_D05_F2_0x244", 2, 5, 2, 0x244, US_REG_DWORD},
    {"B02_D05_F2_0x248", 2, 5, 2, 0x248, US_REG_DWORD},
    {"B02_D05_F2_0x24C", 2, 5, 2, 0x24C, US_REG_DWORD},
    {"B02_D05_F2_0x250", 2, 5, 2, 0x250, US_REG_DWORD},
    {"B02_D05_F2_0x254", 2, 5, 2, 0x254, US_REG_DWORD},
    {"B02_D05_F2_0x268", 2, 5, 2, 0x268, US_REG_DWORD},
    {"B02_D05_F2_0x26C", 2, 5, 2, 0x26C, US_REG_DWORD},
    {"B02_D05_F2_0x270", 2, 5, 2, 0x270, US_REG_DWORD},
    {"B02_D05_F2_0x274", 2, 5, 2, 0x274, US_REG_DWORD},
    {"B02_D05_F2_0x278", 2, 5, 2, 0x278, US_REG_DWORD},
    {"B02_D05_F2_0x288", 2, 5, 2, 0x288, US_REG_DWORD},
    {"B02_D05_F2_0x28C", 2, 5, 2, 0x28C, US_REG_DWORD},
    {"B02_D05_F2_0x290", 2, 5, 2, 0x290, US_REG_DWORD},
    {"B02_D05_F2_0x294", 2, 5, 2, 0x294, US_REG_DWORD},
    {"B02_D05_F2_0x298", 2, 5, 2, 0x298, US_REG_DWORD},
    {"B02_D05_F2_0x29C", 2, 5, 2, 0x29C, US_REG_DWORD},
    {"B02_D05_F2_0x2A0", 2, 5, 2, 0x2A0, US_REG_DWORD},
    {"B02_D05_F2_0x2A4", 2, 5, 2, 0x2A4, US_REG_DWORD},
    {"B02_D05_F2_0x2A8", 2, 5, 2, 0x2A8, US_REG_DWORD},
    {"B02_D05_F2_0x2AC", 2, 5, 2, 0x2AC, US_REG_DWORD},
    {"B02_D05_F2_0x2B0", 2, 5, 2, 0x2B0, US_REG_DWORD},
    {"B02_D05_F2_0x2B4", 2, 5, 2, 0x2B4, US_REG_DWORD},
    {"B02_D05_F2_0x2B8", 2, 5, 2, 0x2B8, US_REG_DWORD},
    {"B02_D05_F2_0x2CC", 2, 5, 2, 0x2CC, US_REG_DWORD},
    {"B02_D05_F2_0x2D0", 2, 5, 2, 0x2D0, US_REG_DWORD},
    {"B02_D05_F2_0x2D4", 2, 5, 2, 0x2D4, US_REG_DWORD},
    {"B02_D05_F2_0x2D8", 2, 5, 2, 0x2D8, US_REG_DWORD},
    {"B02_D05_F2_0x2DC", 2, 5, 2, 0x2DC, US_REG_DWORD},
    {"B02_D05_F2_0x2E0", 2, 5, 2, 0x2E0, US_REG_DWORD},
    {"B02_D05_F2_0x2E4", 2, 5, 2, 0x2E4, US_REG_DWORD},
    {"B02_D05_F2_0x2E8", 2, 5, 2, 0x2E8, US_REG_DWORD},
    {"B02_D05_F2_0x2EC", 2, 5, 2, 0x2EC, US_REG_DWORD},
    {"B02_D05_F2_0x2FC", 2, 5, 2, 0x2FC, US_REG_DWORD},
    {"B02_D05_F2_0x304", 2, 5, 2, 0x304, US_REG_DWORD},
    {"B02_D05_F2_0x4", 2, 5, 2, 0x4, US_REG_WORD},
    {"B02_D05_F2_0x6", 2, 5, 2, 0x6, US_REG_WORD},
    {"B02_D05_F2_0x80", 2, 5, 2, 0x80, US_REG_DWORD},
    {"B02_D05_F2_0x94", 2, 5, 2, 0x94, US_REG_DWORD},
    {"B02_D05_F2_0xA0", 2, 5, 2, 0xA0, US_REG_DWORD},
    {"B02_D07_F0_0x0", 2, 7, 0, 0x0, US_REG_WORD},
    {"B02_D07_F0_0x2", 2, 7, 0, 0x2, US_REG_WORD},
    {"B02_D07_F0_0x4", 2, 7, 0, 0x4, US_REG_WORD},
    {"B02_D07_F0_0x4A", 2, 7, 0, 0x4A, US_REG_WORD},
    {"B02_D07_F0_0x52", 2, 7, 0, 0x52, US_REG_WORD},
    {"B02_D07_F0_0x6", 2, 7, 0, 0x6, US_REG_WORD},
    {"B02_D07_F0_0x72", 2, 7, 0, 0x72, US_REG_WORD},
    {"B02_D07_F0_0x7A", 2, 7, 0, 0x7A, US_REG_WORD},
    {"B02_D07_F1_0x0", 2, 7, 1, 0x0, US_REG_WORD},
    {"B02_D07_F1_0x2", 2, 7, 1, 0x2, US_REG_WORD},
    {"B02_D07_F1_0x4", 2, 7, 1, 0x4, US_REG_WORD},
    {"B02_D07_F1_0x4A", 2, 7, 1, 0x4A, US_REG_WORD},
    {"B02_D07_F1_0x52", 2, 7, 1, 0x52, US_REG_WORD},
    {"B02_D07_F1_0x6", 2, 7, 1, 0x6, US_REG_WORD},
    {"B02_D07_F1_0x72", 2, 7, 1, 0x72, US_REG_WORD},
    {"B02_D07_F1_0x7A", 2, 7, 1, 0x7A, US_REG_WORD},
    {"B02_D07_F2_0x0", 2, 7, 2, 0x0, US_REG_WORD},
    {"B02_D07_F2_0x2", 2, 7, 2, 0x2, US_REG_WORD},
    {"B02_D07_F2_0x4", 2, 7, 2, 0x4, US_REG_WORD},
    {"B02_D07_F2_0x4A", 2, 7, 2, 0x4A, US_REG_WORD},
    {"B02_D07_F2_0x52", 2, 7, 2, 0x52, US_REG_WORD},
    {"B02_D07_F2_0x6", 2, 7, 2, 0x6, US_REG_WORD},
    {"B02_D07_F2_0x72", 2, 7, 2, 0x72, US_REG_WORD},
    {"B02_D07_F2_0x7A", 2, 7, 2, 0x7A, US_REG_WORD},
    {"B02_D07_F3_0x0", 2, 7, 3, 0x0, US_REG_WORD},
    {"B02_D07_F3_0x2", 2, 7, 3, 0x2, US_REG_WORD},
    {"B02_D07_F3_0x4", 2, 7, 3, 0x4, US_REG_WORD},
    {"B02_D07_F3_0x4A", 2, 7, 3, 0x4A, US_REG_WORD},
    {"B02_D07_F3_0x52", 2, 7, 3, 0x52, US_REG_WORD},
    {"B02_D07_F3_0x6", 2, 7, 3, 0x6, US_REG_WORD},
    {"B02_D07_F3_0x72", 2, 7, 3, 0x72, US_REG_WORD},
    {"B02_D07_F3_0x7A", 2, 7, 3, 0x7A, US_REG_WORD},
    {"B02_D07_F4_0x0", 2, 7, 4, 0x0, US_REG_WORD},
    {"B02_D07_F4_0x2", 2, 7, 4, 0x2, US_REG_WORD},
    {"B02_D07_F4_0x4", 2, 7, 4, 0x4, US_REG_WORD},
    {"B02_D07_F4_0x4A", 2, 7, 4, 0x4A, US_REG_WORD},
    {"B02_D07_F4_0x52", 2, 7, 4, 0x52, US_REG_WORD},
    {"B02_D07_F4_0x6", 2, 7, 4, 0x6, US_REG_WORD},
    {"B02_D07_F4_0x72", 2, 7, 4, 0x72, US_REG_WORD},
    {"B02_D07_F4_0x7A", 2, 7, 4, 0x7A, US_REG_WORD},
    {"B02_D07_F7_0x0", 2, 7, 7, 0x0, US_REG_WORD},
    {"B02_D07_F7_0x2", 2, 7, 7, 0x2, US_REG_WORD},
    {"B02_D07_F7_0x358", 2, 7, 7, 0x358, US_REG_DWORD},
    {"B02_D07_F7_0x4", 2, 7, 7, 0x4, US_REG_WORD},
    {"B02_D07_F7_0x4A", 2, 7, 7, 0x4A, US_REG_WORD},
    {"B02_D07_F7_0x52", 2, 7, 7, 0x52, US_REG_WORD},
    {"B02_D07_F7_0x6", 2, 7, 7, 0x6, US_REG_WORD},
    {"B02_D07_F7_0x72", 2, 7, 7, 0x72, US_REG_WORD},
    {"B02_D07_F7_0x7A", 2, 7, 7, 0x7A, US_REG_WORD},
    {"B02_D08_F0_0x140", 2, 8, 0, 0x140, US_REG_DWORD},
    {"B02_D08_F0_0x144", 2, 8, 0, 0x144, US_REG_DWORD},
    {"B02_D09_F0_0x140", 2, 9, 0, 0x140, US_REG_DWORD},
    {"B02_D09_F0_0x144", 2, 9, 0, 0x144, US_REG_DWORD},
    {"B02_D10_F0_0x80", 2, 11, 0, 0x80, US_REG_DWORD},
    {"B02_D10_F0_0x84", 2, 11, 0, 0x84, US_REG_DWORD},
    {"B02_D10_F0_0x88", 2, 11, 0, 0x88, US_REG_DWORD},
    {"B02_D10_F0_0x980", 2, 10, 0, 0x980, US_REG_QWORD},
    {"B02_D10_F0_0x988", 2, 10, 0, 0x988, US_REG_QWORD},
    {"B02_D10_F0_0x990", 2, 10, 0, 0x990, US_REG_QWORD},
    {"B02_D10_F1_0x904", 2, 10, 1, 0x904, US_REG_DWORD},
    {"B02_D10_F1_0xA60", 2, 10, 1, 0xA60, US_REG_DWORD},
    {"B02_D10_F1_0xA64", 2, 10, 1, 0xA64, US_REG_DWORD},
    {"B02_D10_F1_0xA70", 2, 10, 1, 0xA70, US_REG_DWORD},
    {"B02_D10_F1_0xA74", 2, 10, 1, 0xA74, US_REG_DWORD},
    {"B02_D10_F1_0xA98", 2, 10, 1, 0xA98, US_REG_DWORD},
    {"B02_D10_F1_0xAA0", 2, 10, 1, 0xAA0, US_REG_DWORD},
    {"B02_D10_F1_0xAA4", 2, 10, 1, 0xAA4, US_REG_DWORD},
    {"B02_D10_F1_0xD24", 2, 10, 1, 0xD24, US_REG_DWORD},
    {"B02_D10_F1_0xD80", 2, 10, 1, 0xD80, US_REG_DWORD},
    {"B02_D10_F1_0xD84", 2, 10, 1, 0xD84, US_REG_DWORD},
    {"B02_D10_F3_0x104", 2, 10, 3, 0x104, US_REG_DWORD},
    {"B02_D10_F3_0x108", 2, 10, 3, 0x108, US_REG_DWORD},
    {"B02_D10_F3_0x10C", 2, 10, 3, 0x10C, US_REG_DWORD},
    {"B02_D10_F3_0x110", 2, 10, 3, 0x110, US_REG_DWORD},
    {"B02_D10_F3_0x114", 2, 10, 3, 0x114, US_REG_DWORD},
    {"B02_D10_F3_0x118", 2, 10, 3, 0x118, US_REG_DWORD},
    {"B02_D10_F3_0x11C", 2, 10, 3, 0x11C, US_REG_DWORD},
    {"B02_D10_F3_0x120", 2, 10, 3, 0x120, US_REG_DWORD},
    {"B02_D10_F3_0x124", 2, 10, 3, 0x124, US_REG_DWORD},
    {"B02_D10_F3_0x128", 2, 10, 3, 0x128, US_REG_DWORD},
    {"B02_D10_F3_0x12C", 2, 10, 3, 0x12C, US_REG_DWORD},
    {"B02_D10_F3_0x130", 2, 10, 3, 0x130, US_REG_DWORD},
    {"B02_D10_F3_0x134", 2, 10, 3, 0x134, US_REG_DWORD},
    {"B02_D10_F3_0x13C", 2, 10, 3, 0x13C, US_REG_DWORD},
    {"B02_D10_F3_0x140", 2, 10, 3, 0x140, US_REG_QWORD},
    {"B02_D10_F3_0x148", 2, 10, 3, 0x148, US_REG_DWORD},
    {"B02_D10_F3_0x14C", 2, 10, 3, 0x14C, US_REG_DWORD},
    {"B02_D10_F3_0x150", 2, 10, 3, 0x150, US_REG_DWORD},
    {"B02_D10_F3_0x154", 2, 10, 3, 0x154, US_REG_DWORD},
    {"B02_D10_F3_0x15C", 2, 10, 3, 0x15C, US_REG_DWORD},
    {"B02_D10_F3_0x16C", 2, 10, 3, 0x16C, US_REG_DWORD},
    {"B02_D10_F3_0x19c", 2, 10, 3, 0x19c, US_REG_DWORD},
    {"B02_D10_F3_0x308", 2, 10, 3, 0x308, US_REG_DWORD},
    {"B02_D10_F3_0x310", 2, 10, 3, 0x310, US_REG_DWORD},
    {"B02_D10_F3_0x314", 2, 10, 3, 0x314, US_REG_DWORD},
    {"B02_D10_F3_0x318", 2, 10, 3, 0x318, US_REG_DWORD},
    {"B02_D10_F3_0x31C", 2, 10, 3, 0x31C, US_REG_DWORD},
    {"B02_D10_F3_0x400", 2, 10, 3, 0x400, US_REG_DWORD},
    {"B02_D10_F3_0x40C", 2, 10, 3, 0x40C, US_REG_DWORD},
    {"B02_D10_F3_0x420", 2, 10, 3, 0x420, US_REG_DWORD},
    {"B02_D10_F4_0x80", 2, 10, 4, 0x80, US_REG_DWORD},
    {"B02_D10_F4_0x84", 2, 10, 4, 0x84, US_REG_DWORD},
    {"B02_D10_F4_0x88", 2, 10, 4, 0x88, US_REG_DWORD},
    {"B02_D10_F5_0x904", 2, 10, 5, 0x904, US_REG_DWORD},
    {"B02_D10_F5_0xA60", 2, 10, 5, 0xA60, US_REG_DWORD},
    {"B02_D10_F5_0xA64", 2, 10, 5, 0xA64, US_REG_DWORD},
    {"B02_D10_F5_0xA70", 2, 10, 5, 0xA70, US_REG_DWORD},
    {"B02_D10_F5_0xA74", 2, 10, 5, 0xA74, US_REG_DWORD},
    {"B02_D10_F5_0xA98", 2, 10, 5, 0xA98, US_REG_DWORD},
    {"B02_D10_F5_0xAA0", 2, 10, 5, 0xAA0, US_REG_DWORD},
    {"B02_D10_F5_0xAA4", 2, 10, 5, 0xAA4, US_REG_DWORD},
    {"B02_D10_F5_0xD24", 2, 10, 5, 0xD24, US_REG_DWORD},
    {"B02_D10_F5_0xD80", 2, 10, 5, 0xD80, US_REG_DWORD},
    {"B02_D10_F5_0xD84", 2, 10, 5, 0xD84, US_REG_DWORD},
    {"B02_D10_F7_0x104", 2, 10, 7, 0x104, US_REG_DWORD},
    {"B02_D10_F7_0x108", 2, 10, 7, 0x108, US_REG_DWORD},
    {"B02_D10_F7_0x10C", 2, 10, 7, 0x10C, US_REG_DWORD},
    {"B02_D10_F7_0x110", 2, 10, 7, 0x110, US_REG_DWORD},
    {"B02_D10_F7_0x114", 2, 10, 7, 0x114, US_REG_DWORD},
    {"B02_D10_F7_0x118", 2, 10, 7, 0x118, US_REG_DWORD},
    {"B02_D10_F7_0x11C", 2, 10, 7, 0x11C, US_REG_DWORD},
    {"B02_D10_F7_0x120", 2, 10, 7, 0x120, US_REG_DWORD},
    {"B02_D10_F7_0x124", 2, 10, 7, 0x124, US_REG_DWORD},
    {"B02_D10_F7_0x128", 2, 10, 7, 0x128, US_REG_DWORD},
    {"B02_D10_F7_0x12C", 2, 10, 7, 0x12C, US_REG_DWORD},
    {"B02_D10_F7_0x130", 2, 10, 7, 0x130, US_REG_DWORD},
    {"B02_D10_F7_0x134", 2, 10, 7, 0x134, US_REG_DWORD},
    {"B02_D10_F7_0x13C", 2, 10, 7, 0x13C, US_REG_DWORD},
    {"B02_D10_F7_0x140", 2, 10, 7, 0x140, US_REG_QWORD},
    {"B02_D10_F7_0x148", 2, 10, 7, 0x148, US_REG_DWORD},
    {"B02_D10_F7_0x14C", 2, 10, 7, 0x14C, US_REG_DWORD},
    {"B02_D10_F7_0x150", 2, 10, 7, 0x150, US_REG_DWORD},
    {"B02_D10_F7_0x154", 2, 10, 7, 0x154, US_REG_DWORD},
    {"B02_D10_F7_0x15C", 2, 10, 7, 0x15C, US_REG_DWORD},
    {"B02_D10_F7_0x16C", 2, 10, 7, 0x16C, US_REG_DWORD},
    {"B02_D10_F7_0x19c", 2, 10, 7, 0x19c, US_REG_DWORD},
    {"B02_D10_F7_0x308", 2, 10, 7, 0x308, US_REG_DWORD},
    {"B02_D10_F7_0x310", 2, 10, 7, 0x310, US_REG_DWORD},
    {"B02_D10_F7_0x314", 2, 10, 7, 0x314, US_REG_DWORD},
    {"B02_D10_F7_0x318", 2, 10, 7, 0x318, US_REG_DWORD},
    {"B02_D10_F7_0x31C", 2, 10, 7, 0x31C, US_REG_DWORD},
    {"B02_D10_F7_0x400", 2, 10, 7, 0x400, US_REG_DWORD},
    {"B02_D10_F7_0x40C", 2, 10, 7, 0x40C, US_REG_DWORD},
    {"B02_D10_F7_0x420", 2, 10, 7, 0x420, US_REG_DWORD},
    {"B02_D11_F1_0x904", 2, 11, 1, 0x904, US_REG_DWORD},
    {"B02_D11_F1_0xA60", 2, 11, 1, 0xA60, US_REG_DWORD},
    {"B02_D11_F1_0xA64", 2, 11, 1, 0xA64, US_REG_DWORD},
    {"B02_D11_F1_0xA70", 2, 11, 1, 0xA70, US_REG_DWORD},
    {"B02_D11_F1_0xA74", 2, 11, 1, 0xA74, US_REG_DWORD},
    {"B02_D11_F1_0xA98", 2, 11, 1, 0xA98, US_REG_DWORD},
    {"B02_D11_F1_0xAA0", 2, 11, 1, 0xAA0, US_REG_DWORD},
    {"B02_D11_F1_0xAA4", 2, 11, 1, 0xAA4, US_REG_DWORD},
    {"B02_D11_F1_0xD24", 2, 11, 1, 0xD24, US_REG_DWORD},
    {"B02_D11_F1_0xD80", 2, 11, 1, 0xD80, US_REG_DWORD},
    {"B02_D11_F1_0xD84", 2, 11, 1, 0xD84, US_REG_DWORD},
    {"B02_D11_F3_0x104", 2, 11, 3, 0x104, US_REG_DWORD},
    {"B02_D11_F3_0x108", 2, 11, 3, 0x108, US_REG_DWORD},
    {"B02_D11_F3_0x10C", 2, 11, 3, 0x10C, US_REG_DWORD},
    {"B02_D11_F3_0x110", 2, 11, 3, 0x110, US_REG_DWORD},
    {"B02_D11_F3_0x114", 2, 11, 3, 0x114, US_REG_DWORD},
    {"B02_D11_F3_0x118", 2, 11, 3, 0x118, US_REG_DWORD},
    {"B02_D11_F3_0x11C", 2, 11, 3, 0x11C, US_REG_DWORD},
    {"B02_D11_F3_0x120", 2, 11, 3, 0x120, US_REG_DWORD},
    {"B02_D11_F3_0x124", 2, 11, 3, 0x124, US_REG_DWORD},
    {"B02_D11_F3_0x128", 2, 11, 3, 0x128, US_REG_DWORD},
    {"B02_D11_F3_0x12C", 2, 11, 3, 0x12C, US_REG_DWORD},
    {"B02_D11_F3_0x130", 2, 11, 3, 0x130, US_REG_DWORD},
    {"B02_D11_F3_0x134", 2, 11, 3, 0x134, US_REG_DWORD},
    {"B02_D11_F3_0x13C", 2, 11, 3, 0x13C, US_REG_DWORD},
    {"B02_D11_F3_0x140", 2, 11, 3, 0x140, US_REG_QWORD},
    {"B02_D11_F3_0x148", 2, 11, 3, 0x148, US_REG_DWORD},
    {"B02_D11_F3_0x14C", 2, 11, 3, 0x14C, US_REG_DWORD},
    {"B02_D11_F3_0x150", 2, 11, 3, 0x150, US_REG_DWORD},
    {"B02_D11_F3_0x154", 2, 11, 3, 0x154, US_REG_DWORD},
    {"B02_D11_F3_0x15C", 2, 11, 3, 0x15C, US_REG_DWORD},
    {"B02_D11_F3_0x16C", 2, 11, 3, 0x16C, US_REG_DWORD},
    {"B02_D11_F3_0x19c", 2, 11, 3, 0x19c, US_REG_DWORD},
    {"B02_D11_F3_0x308", 2, 11, 3, 0x308, US_REG_DWORD},
    {"B02_D11_F3_0x310", 2, 11, 3, 0x310, US_REG_DWORD},
    {"B02_D11_F3_0x314", 2, 11, 3, 0x314, US_REG_DWORD},
    {"B02_D11_F3_0x318", 2, 11, 3, 0x318, US_REG_DWORD},
    {"B02_D11_F3_0x31C", 2, 11, 3, 0x31C, US_REG_DWORD},
    {"B02_D11_F3_0x400", 2, 11, 3, 0x400, US_REG_DWORD},
    {"B02_D11_F3_0x40C", 2, 11, 3, 0x40C, US_REG_DWORD},
    {"B02_D11_F3_0x420", 2, 11, 3, 0x420, US_REG_DWORD},
    {"B02_D12_F0_0x80", 2, 12, 0, 0x80, US_REG_DWORD},
    {"B02_D12_F0_0x84", 2, 12, 0, 0x84, US_REG_DWORD},
    {"B02_D12_F0_0x88", 2, 12, 0, 0x88, US_REG_DWORD},
    {"B02_D12_F0_0x980", 2, 12, 0, 0x980, US_REG_QWORD},
    {"B02_D12_F0_0x988", 2, 12, 0, 0x988, US_REG_QWORD},
    {"B02_D12_F0_0x990", 2, 12, 0, 0x990, US_REG_QWORD},
    {"B02_D12_F1_0x904", 2, 12, 1, 0x904, US_REG_DWORD},
    {"B02_D12_F1_0xA60", 2, 12, 1, 0xA60, US_REG_DWORD},
    {"B02_D12_F1_0xA64", 2, 12, 1, 0xA64, US_REG_DWORD},
    {"B02_D12_F1_0xA70", 2, 12, 1, 0xA70, US_REG_DWORD},
    {"B02_D12_F1_0xA74", 2, 12, 1, 0xA74, US_REG_DWORD},
    {"B02_D12_F1_0xA98", 2, 12, 1, 0xA98, US_REG_DWORD},
    {"B02_D12_F1_0xAA0", 2, 12, 1, 0xAA0, US_REG_DWORD},
    {"B02_D12_F1_0xAA4", 2, 12, 1, 0xAA4, US_REG_DWORD},
    {"B02_D12_F1_0xD24", 2, 12, 1, 0xD24, US_REG_DWORD},
    {"B02_D12_F1_0xD80", 2, 12, 1, 0xD80, US_REG_DWORD},
    {"B02_D12_F1_0xD84", 2, 12, 1, 0xD84, US_REG_DWORD},
    {"B02_D12_F3_0x104", 2, 12, 3, 0x104, US_REG_DWORD},
    {"B02_D12_F3_0x108", 2, 12, 3, 0x108, US_REG_DWORD},
    {"B02_D12_F3_0x10C", 2, 12, 3, 0x10C, US_REG_DWORD},
    {"B02_D12_F3_0x110", 2, 12, 3, 0x110, US_REG_DWORD},
    {"B02_D12_F3_0x114", 2, 12, 3, 0x114, US_REG_DWORD},
    {"B02_D12_F3_0x118", 2, 12, 3, 0x118, US_REG_DWORD},
    {"B02_D12_F3_0x11C", 2, 12, 3, 0x11C, US_REG_DWORD},
    {"B02_D12_F3_0x120", 2, 12, 3, 0x120, US_REG_DWORD},
    {"B02_D12_F3_0x124", 2, 12, 3, 0x124, US_REG_DWORD},
    {"B02_D12_F3_0x128", 2, 12, 3, 0x128, US_REG_DWORD},
    {"B02_D12_F3_0x12C", 2, 12, 3, 0x12C, US_REG_DWORD},
    {"B02_D12_F3_0x130", 2, 12, 3, 0x130, US_REG_DWORD},
    {"B02_D12_F3_0x134", 2, 12, 3, 0x134, US_REG_DWORD},
    {"B02_D12_F3_0x13C", 2, 12, 3, 0x13C, US_REG_DWORD},
    {"B02_D12_F3_0x140", 2, 12, 3, 0x140, US_REG_QWORD},
    {"B02_D12_F3_0x148", 2, 12, 3, 0x148, US_REG_DWORD},
    {"B02_D12_F3_0x14C", 2, 12, 3, 0x14C, US_REG_DWORD},
    {"B02_D12_F3_0x150", 2, 12, 3, 0x150, US_REG_DWORD},
    {"B02_D12_F3_0x154", 2, 12, 3, 0x154, US_REG_DWORD},
    {"B02_D12_F3_0x15C", 2, 12, 3, 0x15C, US_REG_DWORD},
    {"B02_D12_F3_0x16C", 2, 12, 3, 0x16C, US_REG_DWORD},
    {"B02_D12_F3_0x19c", 2, 12, 3, 0x19c, US_REG_DWORD},
    {"B02_D12_F3_0x308", 2, 12, 3, 0x308, US_REG_DWORD},
    {"B02_D12_F3_0x310", 2, 12, 3, 0x310, US_REG_DWORD},
    {"B02_D12_F3_0x314", 2, 12, 3, 0x314, US_REG_DWORD},
    {"B02_D12_F3_0x318", 2, 12, 3, 0x318, US_REG_DWORD},
    {"B02_D12_F3_0x31C", 2, 12, 3, 0x31C, US_REG_DWORD},
    {"B02_D12_F3_0x400", 2, 12, 3, 0x400, US_REG_DWORD},
    {"B02_D12_F3_0x40C", 2, 12, 3, 0x40C, US_REG_DWORD},
    {"B02_D12_F3_0x420", 2, 12, 3, 0x420, US_REG_DWORD},
    {"B02_D12_F4_0x80", 2, 12, 4, 0x80, US_REG_DWORD},
    {"B02_D12_F4_0x84", 2, 12, 4, 0x84, US_REG_DWORD},
    {"B02_D12_F4_0x88", 2, 12, 4, 0x88, US_REG_DWORD},
    {"B02_D12_F5_0x904", 2, 12, 5, 0x904, US_REG_DWORD},
    {"B02_D12_F5_0xA60", 2, 12, 5, 0xA60, US_REG_DWORD},
    {"B02_D12_F5_0xA64", 2, 12, 5, 0xA64, US_REG_DWORD},
    {"B02_D12_F5_0xA70", 2, 12, 5, 0xA70, US_REG_DWORD},
    {"B02_D12_F5_0xA74", 2, 12, 5, 0xA74, US_REG_DWORD},
    {"B02_D12_F5_0xA98", 2, 12, 5, 0xA98, US_REG_DWORD},
    {"B02_D12_F5_0xAA0", 2, 12, 5, 0xAA0, US_REG_DWORD},
    {"B02_D12_F5_0xAA4", 2, 12, 5, 0xAA4, US_REG_DWORD},
    {"B02_D12_F5_0xD24", 2, 12, 5, 0xD24, US_REG_DWORD},
    {"B02_D12_F5_0xD80", 2, 12, 5, 0xD80, US_REG_DWORD},
    {"B02_D12_F5_0xD84", 2, 12, 5, 0xD84, US_REG_DWORD},
    {"B02_D12_F7_0x104", 2, 12, 7, 0x104, US_REG_DWORD},
    {"B02_D12_F7_0x108", 2, 12, 7, 0x108, US_REG_DWORD},
    {"B02_D12_F7_0x10C", 2, 12, 7, 0x10C, US_REG_DWORD},
    {"B02_D12_F7_0x110", 2, 12, 7, 0x110, US_REG_DWORD},
    {"B02_D12_F7_0x114", 2, 12, 7, 0x114, US_REG_DWORD},
    {"B02_D12_F7_0x118", 2, 12, 7, 0x118, US_REG_DWORD},
    {"B02_D12_F7_0x11C", 2, 12, 7, 0x11C, US_REG_DWORD},
    {"B02_D12_F7_0x120", 2, 12, 7, 0x120, US_REG_DWORD},
    {"B02_D12_F7_0x124", 2, 12, 7, 0x124, US_REG_DWORD},
    {"B02_D12_F7_0x128", 2, 12, 7, 0x128, US_REG_DWORD},
    {"B02_D12_F7_0x12C", 2, 12, 7, 0x12C, US_REG_DWORD},
    {"B02_D12_F7_0x130", 2, 12, 7, 0x130, US_REG_DWORD},
    {"B02_D12_F7_0x134", 2, 12, 7, 0x134, US_REG_DWORD},
    {"B02_D12_F7_0x13C", 2, 12, 7, 0x13C, US_REG_DWORD},
    {"B02_D12_F7_0x140", 2, 12, 7, 0x140, US_REG_QWORD},
    {"B02_D12_F7_0x148", 2, 12, 7, 0x148, US_REG_DWORD},
    {"B02_D12_F7_0x14C", 2, 12, 7, 0x14C, US_REG_DWORD},
    {"B02_D12_F7_0x150", 2, 12, 7, 0x150, US_REG_DWORD},
    {"B02_D12_F7_0x154", 2, 12, 7, 0x154, US_REG_DWORD},
    {"B02_D12_F7_0x15C", 2, 12, 7, 0x15C, US_REG_DWORD},
    {"B02_D12_F7_0x16C", 2, 12, 7, 0x16C, US_REG_DWORD},
    {"B02_D12_F7_0x19c", 2, 12, 7, 0x19c, US_REG_DWORD},
    {"B02_D12_F7_0x308", 2, 12, 7, 0x308, US_REG_DWORD},
    {"B02_D12_F7_0x310", 2, 12, 7, 0x310, US_REG_DWORD},
    {"B02_D12_F7_0x314", 2, 12, 7, 0x314, US_REG_DWORD},
    {"B02_D12_F7_0x318", 2, 12, 7, 0x318, US_REG_DWORD},
    {"B02_D12_F7_0x31C", 2, 12, 7, 0x31C, US_REG_DWORD},
    {"B02_D12_F7_0x400", 2, 12, 7, 0x400, US_REG_DWORD},
    {"B02_D12_F7_0x40C", 2, 12, 7, 0x40C, US_REG_DWORD},
    {"B02_D12_F7_0x420", 2, 12, 7, 0x420, US_REG_DWORD},
    {"B02_D13_F0_0x80", 2, 13, 0, 0x80, US_REG_DWORD},
    {"B02_D13_F0_0x84", 2, 13, 0, 0x84, US_REG_DWORD},
    {"B02_D13_F0_0x88", 2, 13, 0, 0x88, US_REG_DWORD},
    {"B02_D13_F1_0x904", 2, 13, 1, 0x904, US_REG_DWORD},
    {"B02_D13_F1_0xA60", 2, 13, 1, 0xA60, US_REG_DWORD},
    {"B02_D13_F1_0xA64", 2, 13, 1, 0xA64, US_REG_DWORD},
    {"B02_D13_F1_0xA70", 2, 13, 1, 0xA70, US_REG_DWORD},
    {"B02_D13_F1_0xA74", 2, 13, 1, 0xA74, US_REG_DWORD},
    {"B02_D13_F1_0xA98", 2, 13, 1, 0xA98, US_REG_DWORD},
    {"B02_D13_F1_0xAA0", 2, 13, 1, 0xAA0, US_REG_DWORD},
    {"B02_D13_F1_0xAA4", 2, 13, 1, 0xAA4, US_REG_DWORD},
    {"B02_D13_F1_0xD24", 2, 13, 1, 0xD24, US_REG_DWORD},
    {"B02_D13_F1_0xD80", 2, 13, 1, 0xD80, US_REG_DWORD},
    {"B02_D13_F1_0xD84", 2, 13, 1, 0xD84, US_REG_DWORD},
    {"B02_D13_F3_0x104", 2, 13, 3, 0x104, US_REG_DWORD},
    {"B02_D13_F3_0x108", 2, 13, 3, 0x108, US_REG_DWORD},
    {"B02_D13_F3_0x10C", 2, 13, 3, 0x10C, US_REG_DWORD},
    {"B02_D13_F3_0x110", 2, 13, 3, 0x110, US_REG_DWORD},
    {"B02_D13_F3_0x114", 2, 13, 3, 0x114, US_REG_DWORD},
    {"B02_D13_F3_0x118", 2, 13, 3, 0x118, US_REG_DWORD},
    {"B02_D13_F3_0x11C", 2, 13, 3, 0x11C, US_REG_DWORD},
    {"B02_D13_F3_0x120", 2, 13, 3, 0x120, US_REG_DWORD},
    {"B02_D13_F3_0x124", 2, 13, 3, 0x124, US_REG_DWORD},
    {"B02_D13_F3_0x128", 2, 13, 3, 0x128, US_REG_DWORD},
    {"B02_D13_F3_0x12C", 2, 13, 3, 0x12C, US_REG_DWORD},
    {"B02_D13_F3_0x130", 2, 13, 3, 0x130, US_REG_DWORD},
    {"B02_D13_F3_0x134", 2, 13, 3, 0x134, US_REG_DWORD},
    {"B02_D13_F3_0x13C", 2, 13, 3, 0x13C, US_REG_DWORD},
    {"B02_D13_F3_0x140", 2, 13, 3, 0x140, US_REG_QWORD},
    {"B02_D13_F3_0x148", 2, 13, 3, 0x148, US_REG_DWORD},
    {"B02_D13_F3_0x14C", 2, 13, 3, 0x14C, US_REG_DWORD},
    {"B02_D13_F3_0x150", 2, 13, 3, 0x150, US_REG_DWORD},
    {"B02_D13_F3_0x154", 2, 13, 3, 0x154, US_REG_DWORD},
    {"B02_D13_F3_0x15C", 2, 13, 3, 0x15C, US_REG_DWORD},
    {"B02_D13_F3_0x16C", 2, 13, 3, 0x16C, US_REG_DWORD},
    {"B02_D13_F3_0x19c", 2, 13, 3, 0x19c, US_REG_DWORD},
    {"B02_D13_F3_0x308", 2, 13, 3, 0x308, US_REG_DWORD},
    {"B02_D13_F3_0x310", 2, 13, 3, 0x310, US_REG_DWORD},
    {"B02_D13_F3_0x314", 2, 13, 3, 0x314, US_REG_DWORD},
    {"B02_D13_F3_0x318", 2, 13, 3, 0x318, US_REG_DWORD},
    {"B02_D13_F3_0x31C", 2, 13, 3, 0x31C, US_REG_DWORD},
    {"B02_D13_F3_0x400", 2, 13, 3, 0x400, US_REG_DWORD},
    {"B02_D13_F3_0x40C", 2, 13, 3, 0x40C, US_REG_DWORD},
    {"B02_D13_F3_0x420", 2, 13, 3, 0x420, US_REG_DWORD},
    {"B03_D00_F0_0x0", 3, 0, 0, 0x0, US_REG_WORD},
    {"B03_D00_F0_0x14C", 3, 0, 0, 0x14C, US_REG_DWORD},
    {"B03_D00_F0_0x158", 3, 0, 0, 0x158, US_REG_DWORD},
    {"B03_D00_F0_0x178", 3, 0, 0, 0x178, US_REG_DWORD},
    {"B03_D00_F0_0x17C", 3, 0, 0, 0x17C, US_REG_DWORD},
    {"B03_D00_F0_0x18", 3, 0, 0, 0x18, US_REG_BYTE},
    {"B03_D00_F0_0x188", 3, 0, 0, 0x188, US_REG_DWORD},
    {"B03_D00_F0_0x18C", 3, 0, 0, 0x18C, US_REG_DWORD},
    {"B03_D00_F0_0x19", 3, 0, 0, 0x19, US_REG_BYTE},
    {"B03_D00_F0_0x1A", 3, 0, 0, 0x1A, US_REG_BYTE},
    {"B03_D00_F0_0x1A2", 3, 0, 0, 0x1A2, US_REG_WORD},
    {"B03_D00_F0_0x1AA", 3, 0, 0, 0x1AA, US_REG_WORD},
    {"B03_D00_F0_0x1C", 3, 0, 0, 0x1C, US_REG_BYTE},
    {"B03_D00_F0_0x1C2", 3, 0, 0, 0x1C2, US_REG_WORD},
    {"B03_D00_F0_0x1D", 3, 0, 0, 0x1D, US_REG_BYTE},
    {"B03_D00_F0_0x1E", 3, 0, 0, 0x1E, US_REG_WORD},
    {"B03_D00_F0_0x2", 3, 0, 0, 0x2, US_REG_WORD},
    {"B03_D00_F0_0x20", 3, 0, 0, 0x20, US_REG_WORD},
    {"B03_D00_F0_0x200", 3, 0, 0, 0x200, US_REG_BYTE},
    {"B03_D00_F0_0x204", 3, 0, 0, 0x204, US_REG_BYTE},
    {"B03_D00_F0_0x208", 3, 0, 0, 0x208, US_REG_BYTE},
    {"B03_D00_F0_0x20C", 3, 0, 0, 0x20C, US_REG_BYTE},
    {"B03_D00_F0_0x210", 3, 0, 0, 0x210, US_REG_BYTE},
    {"B03_D00_F0_0x214", 3, 0, 0, 0x214, US_REG_BYTE},
    {"B03_D00_F0_0x218", 3, 0, 0, 0x218, US_REG_BYTE},
    {"B03_D00_F0_0x21C", 3, 0, 0, 0x21C, US_REG_BYTE},
    {"B03_D00_F0_0x22", 3, 0, 0, 0x22, US_REG_WORD},
    {"B03_D00_F0_0x220", 3, 0, 0, 0x220, US_REG_BYTE},
    {"B03_D00_F0_0x224", 3, 0, 0, 0x224, US_REG_BYTE},
    {"B03_D00_F0_0x228", 3, 0, 0, 0x228, US_REG_BYTE},
    {"B03_D00_F0_0x230", 3, 0, 0, 0x230, US_REG_BYTE},
    {"B03_D00_F0_0x232", 3, 0, 0, 0x232, US_REG_BYTE},
    {"B03_D00_F0_0x24", 3, 0, 0, 0x24, US_REG_WORD},
    {"B03_D00_F0_0x258", 3, 0, 0, 0x258, US_REG_DWORD},
    {"B03_D00_F0_0x26", 3, 0, 0, 0x26, US_REG_WORD},
    {"B03_D00_F0_0x28", 3, 0, 0, 0x28, US_REG_DWORD},
    {"B03_D00_F0_0x288", 3, 0, 0, 0x288, US_REG_DWORD},
    {"B03_D00_F0_0x298", 3, 0, 0, 0x298, US_REG_DWORD},
    {"B03_D00_F0_0x2A4", 3, 0, 0, 0x2A4, US_REG_DWORD},
    {"B03_D00_F0_0x2C", 3, 0, 0, 0x2C, US_REG_DWORD},
    {"B03_D00_F0_0x4", 3, 0, 0, 0x4, US_REG_WORD},
    {"B03_D00_F0_0x6", 3, 0, 0, 0x6, US_REG_WORD},
    {"B03_D00_F0_0x9A", 3, 0, 0, 0x9A, US_REG_WORD},
    {"B03_D00_F0_0xA2", 3, 0, 0, 0xA2, US_REG_WORD},
    {"B03_D00_F0_0xAA", 3, 0, 0, 0xAA, US_REG_WORD},
    {"B03_D00_F0_0xC2", 3, 0, 0, 0xC2, US_REG_WORD},
    {"B03_D01_F0_0x0", 3, 1, 0, 0x0, US_REG_WORD},
    {"B03_D01_F0_0x14C", 3, 1, 0, 0x14C, US_REG_DWORD},
    {"B03_D01_F0_0x158", 3, 1, 0, 0x158, US_REG_DWORD},
    {"B03_D01_F0_0x178", 3, 1, 0, 0x178, US_REG_DWORD},
    {"B03_D01_F0_0x17C", 3, 1, 0, 0x17C, US_REG_DWORD},
    {"B03_D01_F0_0x18", 3, 1, 0, 0x18, US_REG_BYTE},
    {"B03_D01_F0_0x188", 3, 1, 0, 0x188, US_REG_DWORD},
    {"B03_D01_F0_0x18C", 3, 1, 0, 0x18C, US_REG_DWORD},
    {"B03_D01_F0_0x19", 3, 1, 0, 0x19, US_REG_BYTE},
    {"B03_D01_F0_0x1A", 3, 1, 0, 0x1A, US_REG_BYTE},
    {"B03_D01_F0_0x1C", 3, 1, 0, 0x1C, US_REG_BYTE},
    {"B03_D01_F0_0x1D", 3, 1, 0, 0x1D, US_REG_BYTE},
    {"B03_D01_F0_0x1E", 3, 1, 0, 0x1E, US_REG_WORD},
    {"B03_D01_F0_0x2", 3, 1, 0, 0x2, US_REG_WORD},
    {"B03_D01_F0_0x20", 3, 1, 0, 0x20, US_REG_WORD},
    {"B03_D01_F0_0x200", 3, 1, 0, 0x200, US_REG_BYTE},
    {"B03_D01_F0_0x204", 3, 1, 0, 0x204, US_REG_BYTE},
    {"B03_D01_F0_0x208", 3, 1, 0, 0x208, US_REG_BYTE},
    {"B03_D01_F0_0x20C", 3, 1, 0, 0x20C, US_REG_BYTE},
    {"B03_D01_F0_0x210", 3, 1, 0, 0x210, US_REG_BYTE},
    {"B03_D01_F0_0x214", 3, 1, 0, 0x214, US_REG_BYTE},
    {"B03_D01_F0_0x218", 3, 1, 0, 0x218, US_REG_BYTE},
    {"B03_D01_F0_0x21C", 3, 1, 0, 0x21C, US_REG_BYTE},
    {"B03_D01_F0_0x22", 3, 1, 0, 0x22, US_REG_WORD},
    {"B03_D01_F0_0x220", 3, 1, 0, 0x220, US_REG_BYTE},
    {"B03_D01_F0_0x224", 3, 1, 0, 0x224, US_REG_BYTE},
    {"B03_D01_F0_0x228", 3, 1, 0, 0x228, US_REG_BYTE},
    {"B03_D01_F0_0x230", 3, 1, 0, 0x230, US_REG_BYTE},
    {"B03_D01_F0_0x232", 3, 1, 0, 0x232, US_REG_BYTE},
    {"B03_D01_F0_0x24", 3, 1, 0, 0x24, US_REG_WORD},
    {"B03_D01_F0_0x258", 3, 1, 0, 0x258, US_REG_DWORD},
    {"B03_D01_F0_0x26", 3, 1, 0, 0x26, US_REG_WORD},
    {"B03_D01_F0_0x28", 3, 1, 0, 0x28, US_REG_DWORD},
    {"B03_D01_F0_0x288", 3, 1, 0, 0x288, US_REG_DWORD},
    {"B03_D01_F0_0x298", 3, 1, 0, 0x298, US_REG_DWORD},
    {"B03_D01_F0_0x2A4", 3, 1, 0, 0x2A4, US_REG_DWORD},
    {"B03_D01_F0_0x2C", 3, 1, 0, 0x2C, US_REG_DWORD},
    {"B03_D01_F0_0x4", 3, 1, 0, 0x4, US_REG_WORD},
    {"B03_D01_F0_0x6", 3, 1, 0, 0x6, US_REG_WORD},
    {"B03_D01_F0_0x9A", 3, 1, 0, 0x9A, US_REG_WORD},
    {"B03_D01_F0_0xA2", 3, 1, 0, 0xA2, US_REG_WORD},
    {"B03_D01_F0_0xAA", 3, 1, 0, 0xAA, US_REG_WORD},
    {"B03_D01_F0_0xC2", 3, 1, 0, 0xC2, US_REG_WORD},
    {"B03_D02_F0_0x0", 3, 2, 0, 0x0, US_REG_WORD},
    {"B03_D02_F0_0x14C", 3, 2, 0, 0x14C, US_REG_DWORD},
    {"B03_D02_F0_0x158", 3, 2, 0, 0x158, US_REG_DWORD},
    {"B03_D02_F0_0x178", 3, 2, 0, 0x178, US_REG_DWORD},
    {"B03_D02_F0_0x17C", 3, 2, 0, 0x17C, US_REG_DWORD},
    {"B03_D02_F0_0x18", 3, 2, 0, 0x18, US_REG_BYTE},
    {"B03_D02_F0_0x188", 3, 2, 0, 0x188, US_REG_DWORD},
    {"B03_D02_F0_0x18C", 3, 2, 0, 0x18C, US_REG_DWORD},
    {"B03_D02_F0_0x19", 3, 2, 0, 0x19, US_REG_BYTE},
    {"B03_D02_F0_0x1A", 3, 2, 0, 0x1A, US_REG_BYTE},
    {"B03_D02_F0_0x1C", 3, 2, 0, 0x1C, US_REG_BYTE},
    {"B03_D02_F0_0x1D", 3, 2, 0, 0x1D, US_REG_BYTE},
    {"B03_D02_F0_0x1E", 3, 2, 0, 0x1E, US_REG_WORD},
    {"B03_D02_F0_0x2", 3, 2, 0, 0x2, US_REG_WORD},
    {"B03_D02_F0_0x20", 3, 2, 0, 0x20, US_REG_WORD},
    {"B03_D02_F0_0x200", 3, 2, 0, 0x200, US_REG_BYTE},
    {"B03_D02_F0_0x204", 3, 2, 0, 0x204, US_REG_BYTE},
    {"B03_D02_F0_0x208", 3, 2, 0, 0x208, US_REG_BYTE},
    {"B03_D02_F0_0x20C", 3, 2, 0, 0x20C, US_REG_BYTE},
    {"B03_D02_F0_0x210", 3, 2, 0, 0x210, US_REG_BYTE},
    {"B03_D02_F0_0x214", 3, 2, 0, 0x214, US_REG_BYTE},
    {"B03_D02_F0_0x218", 3, 2, 0, 0x218, US_REG_BYTE},
    {"B03_D02_F0_0x21C", 3, 2, 0, 0x21C, US_REG_BYTE},
    {"B03_D02_F0_0x22", 3, 2, 0, 0x22, US_REG_WORD},
    {"B03_D02_F0_0x220", 3, 2, 0, 0x220, US_REG_BYTE},
    {"B03_D02_F0_0x224", 3, 2, 0, 0x224, US_REG_BYTE},
    {"B03_D02_F0_0x228", 3, 2, 0, 0x228, US_REG_BYTE},
    {"B03_D02_F0_0x230", 3, 2, 0, 0x230, US_REG_BYTE},
    {"B03_D02_F0_0x232", 3, 2, 0, 0x232, US_REG_BYTE},
    {"B03_D02_F0_0x24", 3, 2, 0, 0x24, US_REG_WORD},
    {"B03_D02_F0_0x258", 3, 2, 0, 0x258, US_REG_DWORD},
    {"B03_D02_F0_0x26", 3, 2, 0, 0x26, US_REG_WORD},
    {"B03_D02_F0_0x28", 3, 2, 0, 0x28, US_REG_DWORD},
    {"B03_D02_F0_0x288", 3, 2, 0, 0x288, US_REG_DWORD},
    {"B03_D02_F0_0x298", 3, 2, 0, 0x298, US_REG_DWORD},
    {"B03_D02_F0_0x2A4", 3, 2, 0, 0x2A4, US_REG_DWORD},
    {"B03_D02_F0_0x2C", 3, 2, 0, 0x2C, US_REG_DWORD},
    {"B03_D02_F0_0x4", 3, 2, 0, 0x4, US_REG_WORD},
    {"B03_D02_F0_0x6", 3, 2, 0, 0x6, US_REG_WORD},
    {"B03_D02_F0_0x9A", 3, 2, 0, 0x9A, US_REG_WORD},
    {"B03_D02_F0_0xA2", 3, 2, 0, 0xA2, US_REG_WORD},
    {"B03_D02_F0_0xAA", 3, 2, 0, 0xAA, US_REG_WORD},
    {"B03_D02_F0_0xC2", 3, 2, 0, 0xC2, US_REG_WORD},
    {"B03_D03_F0_0x0", 3, 3, 0, 0x0, US_REG_WORD},
    {"B03_D03_F0_0x14C", 3, 3, 0, 0x14C, US_REG_DWORD},
    {"B03_D03_F0_0x158", 3, 3, 0, 0x158, US_REG_DWORD},
    {"B03_D03_F0_0x178", 3, 3, 0, 0x178, US_REG_DWORD},
    {"B03_D03_F0_0x17C", 3, 3, 0, 0x17C, US_REG_DWORD},
    {"B03_D03_F0_0x18", 3, 3, 0, 0x18, US_REG_BYTE},
    {"B03_D03_F0_0x188", 3, 3, 0, 0x188, US_REG_DWORD},
    {"B03_D03_F0_0x18C", 3, 3, 0, 0x18C, US_REG_DWORD},
    {"B03_D03_F0_0x19", 3, 3, 0, 0x19, US_REG_BYTE},
    {"B03_D03_F0_0x1A", 3, 3, 0, 0x1A, US_REG_BYTE},
    {"B03_D03_F0_0x1C", 3, 3, 0, 0x1C, US_REG_BYTE},
    {"B03_D03_F0_0x1D", 3, 3, 0, 0x1D, US_REG_BYTE},
    {"B03_D03_F0_0x1E", 3, 3, 0, 0x1E, US_REG_WORD},
    {"B03_D03_F0_0x2", 3, 3, 0, 0x2, US_REG_WORD},
    {"B03_D03_F0_0x20", 3, 3, 0, 0x20, US_REG_WORD},
    {"B03_D03_F0_0x200", 3, 3, 0, 0x200, US_REG_BYTE},
    {"B03_D03_F0_0x204", 3, 3, 0, 0x204, US_REG_BYTE},
    {"B03_D03_F0_0x208", 3, 3, 0, 0x208, US_REG_BYTE},
    {"B03_D03_F0_0x20C", 3, 3, 0, 0x20C, US_REG_BYTE},
    {"B03_D03_F0_0x210", 3, 3, 0, 0x210, US_REG_BYTE},
    {"B03_D03_F0_0x214", 3, 3, 0, 0x214, US_REG_BYTE},
    {"B03_D03_F0_0x218", 3, 3, 0, 0x218, US_REG_BYTE},
    {"B03_D03_F0_0x21C", 3, 3, 0, 0x21C, US_REG_BYTE},
    {"B03_D03_F0_0x22", 3, 3, 0, 0x22, US_REG_WORD},
    {"B03_D03_F0_0x220", 3, 3, 0, 0x220, US_REG_BYTE},
    {"B03_D03_F0_0x224", 3, 3, 0, 0x224, US_REG_BYTE},
    {"B03_D03_F0_0x228", 3, 3, 0, 0x228, US_REG_BYTE},
    {"B03_D03_F0_0x230", 3, 3, 0, 0x230, US_REG_BYTE},
    {"B03_D03_F0_0x232", 3, 3, 0, 0x232, US_REG_BYTE},
    {"B03_D03_F0_0x24", 3, 3, 0, 0x24, US_REG_WORD},
    {"B03_D03_F0_0x258", 3, 3, 0, 0x258, US_REG_DWORD},
    {"B03_D03_F0_0x26", 3, 3, 0, 0x26, US_REG_WORD},
    {"B03_D03_F0_0x28", 3, 3, 0, 0x28, US_REG_DWORD},
    {"B03_D03_F0_0x288", 3, 3, 0, 0x288, US_REG_DWORD},
    {"B03_D03_F0_0x298", 3, 3, 0, 0x298, US_REG_DWORD},
    {"B03_D03_F0_0x2A4", 3, 3, 0, 0x2A4, US_REG_DWORD},
    {"B03_D03_F0_0x2C", 3, 3, 0, 0x2C, US_REG_DWORD},
    {"B03_D03_F0_0x4", 3, 3, 0, 0x4, US_REG_WORD},
    {"B03_D03_F0_0x6", 3, 3, 0, 0x6, US_REG_WORD},
    {"B03_D03_F0_0x9A", 3, 3, 0, 0x9A, US_REG_WORD},
    {"B03_D03_F0_0xA2", 3, 3, 0, 0xA2, US_REG_WORD},
    {"B03_D03_F0_0xAA", 3, 3, 0, 0xAA, US_REG_WORD},
    {"B03_D03_F0_0xC2", 3, 3, 0, 0xC2, US_REG_WORD},
    {"B03_D05_F0_0x1A8", 3, 5, 0, 0x1A8, US_REG_DWORD},
    {"B03_D05_F0_0x1B4", 3, 5, 0, 0x1B4, US_REG_DWORD},
    {"B03_D05_F2_0x0", 3, 5, 2, 0x0, US_REG_WORD},
    {"B03_D05_F2_0x19C", 3, 5, 2, 0x19C, US_REG_DWORD},
    {"B03_D05_F2_0x1A0", 3, 5, 2, 0x1A0, US_REG_DWORD},
    {"B03_D05_F2_0x1A4", 3, 5, 2, 0x1A4, US_REG_DWORD},
    {"B03_D05_F2_0x1A8", 3, 5, 2, 0x1A8, US_REG_DWORD},
    {"B03_D05_F2_0x1AC", 3, 5, 2, 0x1AC, US_REG_DWORD},
    {"B03_D05_F2_0x1B4", 3, 5, 2, 0x1B4, US_REG_DWORD},
    {"B03_D05_F2_0x1C0", 3, 5, 2, 0x1C0, US_REG_DWORD},
    {"B03_D05_F2_0x1C4", 3, 5, 2, 0x1C4, US_REG_DWORD},
    {"B03_D05_F2_0x1C8", 3, 5, 2, 0x1C8, US_REG_DWORD},
    {"B03_D05_F2_0x1CC", 3, 5, 2, 0x1CC, US_REG_DWORD},
    {"B03_D05_F2_0x1D0", 3, 5, 2, 0x1D0, US_REG_DWORD},
    {"B03_D05_F2_0x1D4", 3, 5, 2, 0x1D4, US_REG_DWORD},
    {"B03_D05_F2_0x1D8", 3, 5, 2, 0x1D8, US_REG_DWORD},
    {"B03_D05_F2_0x1DC", 3, 5, 2, 0x1DC, US_REG_DWORD},
    {"B03_D05_F2_0x1E8", 3, 5, 2, 0x1E8, US_REG_DWORD},
    {"B03_D05_F2_0x1EC", 3, 5, 2, 0x1EC, US_REG_DWORD},
    {"B03_D05_F2_0x1F8", 3, 5, 2, 0x1F8, US_REG_DWORD},
    {"B03_D05_F2_0x2", 3, 5, 2, 0x2, US_REG_WORD},
    {"B03_D05_F2_0x200", 3, 5, 2, 0x200, US_REG_DWORD},
    {"B03_D05_F2_0x208", 3, 5, 2, 0x208, US_REG_DWORD},
    {"B03_D05_F2_0x20C", 3, 5, 2, 0x20C, US_REG_DWORD},
    {"B03_D05_F2_0x210", 3, 5, 2, 0x210, US_REG_DWORD},
    {"B03_D05_F2_0x214", 3, 5, 2, 0x214, US_REG_DWORD},
    {"B03_D05_F2_0x218", 3, 5, 2, 0x218, US_REG_DWORD},
    {"B03_D05_F2_0x21C", 3, 5, 2, 0x21C, US_REG_DWORD},
    {"B03_D05_F2_0x220", 3, 5, 2, 0x220, US_REG_DWORD},
    {"B03_D05_F2_0x224", 3, 5, 2, 0x224, US_REG_DWORD},
    {"B03_D05_F2_0x228", 3, 5, 2, 0x228, US_REG_DWORD},
    {"B03_D05_F2_0x230", 3, 5, 2, 0x230, US_REG_DWORD},
    {"B03_D05_F2_0x234", 3, 5, 2, 0x234, US_REG_DWORD},
    {"B03_D05_F2_0x238", 3, 5, 2, 0x238, US_REG_DWORD},
    {"B03_D05_F2_0x23C", 3, 5, 2, 0x23C, US_REG_DWORD},
    {"B03_D05_F2_0x240", 3, 5, 2, 0x240, US_REG_DWORD},
    {"B03_D05_F2_0x244", 3, 5, 2, 0x244, US_REG_DWORD},
    {"B03_D05_F2_0x248", 3, 5, 2, 0x248, US_REG_DWORD},
    {"B03_D05_F2_0x24C", 3, 5, 2, 0x24C, US_REG_DWORD},
    {"B03_D05_F2_0x250", 3, 5, 2, 0x250, US_REG_DWORD},
    {"B03_D05_F2_0x254", 3, 5, 2, 0x254, US_REG_DWORD},
    {"B03_D05_F2_0x268", 3, 5, 2, 0x268, US_REG_DWORD},
    {"B03_D05_F2_0x26C", 3, 5, 2, 0x26C, US_REG_DWORD},
    {"B03_D05_F2_0x270", 3, 5, 2, 0x270, US_REG_DWORD},
    {"B03_D05_F2_0x274", 3, 5, 2, 0x274, US_REG_DWORD},
    {"B03_D05_F2_0x278", 3, 5, 2, 0x278, US_REG_DWORD},
    {"B03_D05_F2_0x288", 3, 5, 2, 0x288, US_REG_DWORD},
    {"B03_D05_F2_0x28C", 3, 5, 2, 0x28C, US_REG_DWORD},
    {"B03_D05_F2_0x290", 3, 5, 2, 0x290, US_REG_DWORD},
    {"B03_D05_F2_0x294", 3, 5, 2, 0x294, US_REG_DWORD},
    {"B03_D05_F2_0x298", 3, 5, 2, 0x298, US_REG_DWORD},
    {"B03_D05_F2_0x29C", 3, 5, 2, 0x29C, US_REG_DWORD},
    {"B03_D05_F2_0x2A0", 3, 5, 2, 0x2A0, US_REG_DWORD},
    {"B03_D05_F2_0x2A4", 3, 5, 2, 0x2A4, US_REG_DWORD},
    {"B03_D05_F2_0x2A8", 3, 5, 2, 0x2A8, US_REG_DWORD},
    {"B03_D05_F2_0x2AC", 3, 5, 2, 0x2AC, US_REG_DWORD},
    {"B03_D05_F2_0x2B0", 3, 5, 2, 0x2B0, US_REG_DWORD},
    {"B03_D05_F2_0x2B4", 3, 5, 2, 0x2B4, US_REG_DWORD},
    {"B03_D05_F2_0x2B8", 3, 5, 2, 0x2B8, US_REG_DWORD},
    {"B03_D05_F2_0x2CC", 3, 5, 2, 0x2CC, US_REG_DWORD},
    {"B03_D05_F2_0x2D0", 3, 5, 2, 0x2D0, US_REG_DWORD},
    {"B03_D05_F2_0x2D4", 3, 5, 2, 0x2D4, US_REG_DWORD},
    {"B03_D05_F2_0x2D8", 3, 5, 2, 0x2D8, US_REG_DWORD},
    {"B03_D05_F2_0x2DC", 3, 5, 2, 0x2DC, US_REG_DWORD},
    {"B03_D05_F2_0x2E0", 3, 5, 2, 0x2E0, US_REG_DWORD},
    {"B03_D05_F2_0x2E4", 3, 5, 2, 0x2E4, US_REG_DWORD},
    {"B03_D05_F2_0x2E8", 3, 5, 2, 0x2E8, US_REG_DWORD},
    {"B03_D05_F2_0x2EC", 3, 5, 2, 0x2EC, US_REG_DWORD},
    {"B03_D05_F2_0x2FC", 3, 5, 2, 0x2FC, US_REG_DWORD},
    {"B03_D05_F2_0x304", 3, 5, 2, 0x304, US_REG_DWORD},
    {"B03_D05_F2_0x4", 3, 5, 2, 0x4, US_REG_WORD},
    {"B03_D05_F2_0x6", 3, 5, 2, 0x6, US_REG_WORD},
    {"B03_D05_F2_0x80", 3, 5, 2, 0x80, US_REG_DWORD},
    {"B03_D05_F2_0x94", 3, 5, 2, 0x94, US_REG_DWORD},
    {"B03_D05_F2_0xA0", 3, 5, 2, 0xA0, US_REG_DWORD},
    {"B03_D07_F0_0x0", 3, 7, 0, 0x0, US_REG_WORD},
    {"B03_D07_F0_0x2", 3, 7, 0, 0x2, US_REG_WORD},
    {"B03_D07_F0_0x4", 3, 7, 0, 0x4, US_REG_WORD},
    {"B03_D07_F0_0x4A", 3, 7, 0, 0x4A, US_REG_WORD},
    {"B03_D07_F0_0x52", 3, 7, 0, 0x52, US_REG_WORD},
    {"B03_D07_F0_0x6", 3, 7, 0, 0x6, US_REG_WORD},
    {"B03_D07_F0_0x72", 3, 7, 0, 0x72, US_REG_WORD},
    {"B03_D07_F0_0x7A", 3, 7, 0, 0x7A, US_REG_WORD},
    {"B03_D07_F1_0x0", 3, 7, 1, 0x0, US_REG_WORD},
    {"B03_D07_F1_0x2", 3, 7, 1, 0x2, US_REG_WORD},
    {"B03_D07_F1_0x4", 3, 7, 1, 0x4, US_REG_WORD},
    {"B03_D07_F1_0x4A", 3, 7, 1, 0x4A, US_REG_WORD},
    {"B03_D07_F1_0x52", 3, 7, 1, 0x52, US_REG_WORD},
    {"B03_D07_F1_0x6", 3, 7, 1, 0x6, US_REG_WORD},
    {"B03_D07_F1_0x72", 3, 7, 1, 0x72, US_REG_WORD},
    {"B03_D07_F1_0x7A", 3, 7, 1, 0x7A, US_REG_WORD},
    {"B03_D07_F2_0x0", 3, 7, 2, 0x0, US_REG_WORD},
    {"B03_D07_F2_0x2", 3, 7, 2, 0x2, US_REG_WORD},
    {"B03_D07_F2_0x4", 3, 7, 2, 0x4, US_REG_WORD},
    {"B03_D07_F2_0x4A", 3, 7, 2, 0x4A, US_REG_WORD},
    {"B03_D07_F2_0x52", 3, 7, 2, 0x52, US_REG_WORD},
    {"B03_D07_F2_0x6", 3, 7, 2, 0x6, US_REG_WORD},
    {"B03_D07_F2_0x72", 3, 7, 2, 0x72, US_REG_WORD},
    {"B03_D07_F2_0x7A", 3, 7, 2, 0x7A, US_REG_WORD},
    {"B03_D07_F3_0x4A", 3, 7, 3, 0x4A, US_REG_WORD},
    {"B03_D07_F3_0x52", 3, 7, 3, 0x52, US_REG_WORD},
    {"B03_D07_F3_0x6", 3, 7, 3, 0x6, US_REG_WORD},
    {"B03_D07_F3_0x72", 3, 7, 3, 0x72, US_REG_WORD},
    {"B03_D07_F3_0x7A", 3, 7, 3, 0x7A, US_REG_WORD},
    {"B03_D07_F4_0x0", 3, 7, 4, 0x0, US_REG_WORD},
    {"B03_D07_F4_0x2", 3, 7, 4, 0x2, US_REG_WORD},
    {"B03_D07_F4_0x4", 3, 7, 4, 0x4, US_REG_WORD},
    {"B03_D07_F4_0x4A", 3, 7, 4, 0x4A, US_REG_WORD},
    {"B03_D07_F4_0x52", 3, 7, 4, 0x52, US_REG_WORD},
    {"B03_D07_F4_0x6", 3, 7, 4, 0x6, US_REG_WORD},
    {"B03_D07_F4_0x72", 3, 7, 4, 0x72, US_REG_WORD},
    {"B03_D07_F4_0x7A", 3, 7, 4, 0x7A, US_REG_WORD},
    {"B03_D07_F7_0x0", 3, 7, 7, 0x0, US_REG_WORD},
    {"B03_D07_F7_0x2", 3, 7, 7, 0x2, US_REG_WORD},
    {"B03_D07_F7_0x358", 3, 7, 7, 0x358, US_REG_DWORD},
    {"B03_D07_F7_0x4", 3, 7, 7, 0x4, US_REG_WORD},
    {"B03_D07_F7_0x4A", 3, 7, 7, 0x4A, US_REG_WORD},
    {"B03_D07_F7_0x52", 3, 7, 7, 0x52, US_REG_WORD},
    {"B03_D07_F7_0x6", 3, 7, 7, 0x6, US_REG_WORD},
    {"B03_D07_F7_0x72", 3, 7, 7, 0x72, US_REG_WORD},
    {"B03_D07_F7_0x7A", 3, 7, 7, 0x7A, US_REG_WORD},
    {"B03_D14_F0_0x84", 3, 14, 0, 0x84, US_REG_DWORD},
    {"B03_D14_F0_0x8C", 3, 14, 0, 0x8C, US_REG_DWORD},
    {"B03_D14_F0_0x90", 3, 14, 0, 0x90, US_REG_DWORD},
    {"B03_D14_F0_0x9C", 3, 14, 0, 0x9C, US_REG_BYTE},
    {"B03_D14_F0_0xA0", 3, 14, 0, 0xA0, US_REG_BYTE},
    {"B03_D14_F0_0xA4", 3, 14, 0, 0xA4, US_REG_BYTE},
    {"B03_D14_F0_0xA8", 3, 14, 0, 0xA8, US_REG_QWORD},
    {"B03_D14_F0_0xB0", 3, 14, 0, 0xB0, US_REG_QWORD},
    {"B03_D14_F0_0xB8", 3, 14, 0, 0xB8, US_REG_QWORD},
    {"B03_D14_F0_0xC0", 3, 14, 0, 0xC0, US_REG_DWORD},
    {"B03_D14_F0_0xC4", 3, 14, 0, 0xC4, US_REG_BYTE},
    {"B03_D14_F0_0xC8", 3, 14, 0, 0xC8, US_REG_BYTE},
    {"B03_D14_F0_0xD4", 3, 14, 0, 0xD4, US_REG_DWORD},
    {"B03_D14_F0_0xDC", 3, 14, 0, 0xDC, US_REG_DWORD},
    {"B03_D14_F0_0xE4", 3, 14, 0, 0xE4, US_REG_DWORD},
    {"B03_D14_F1_0x12C", 3, 14, 1, 0x12C, US_REG_DWORD},
    {"B03_D14_F1_0x130", 3, 14, 1, 0x130, US_REG_DWORD},
    {"B03_D14_F1_0x13C", 3, 14, 1, 0x13C, US_REG_DWORD},
    {"B03_D14_F1_0x140", 3, 14, 1, 0x140, US_REG_DWORD},
    {"B03_D14_F1_0x14C", 3, 14, 1, 0x14C, US_REG_DWORD},
    {"B03_D14_F1_0x154", 3, 14, 1, 0x154, US_REG_DWORD},
    {"B03_D14_F1_0x160", 3, 14, 1, 0x160, US_REG_DWORD},
    {"B03_D14_F1_0x164", 3, 14, 1, 0x164, US_REG_DWORD},
    {"B03_D14_F1_0x168", 3, 14, 1, 0x168, US_REG_DWORD},
    {"B03_D14_F1_0x16C", 3, 14, 1, 0x16C, US_REG_DWORD},
    {"B03_D14_F1_0x170", 3, 14, 1, 0x170, US_REG_DWORD},
    {"B03_D14_F1_0x178", 3, 14, 1, 0x178, US_REG_DWORD},
    {"B03_D14_F1_0x1B4", 3, 14, 1, 0x1B4, US_REG_DWORD},
    {"B03_D14_F1_0x1C0", 3, 14, 1, 0x1C0, US_REG_DWORD},
    {"B03_D14_F2_0x3A0", 3, 14, 2, 0x3A0, US_REG_DWORD},
    {"B03_D14_F2_0x3A4", 3, 14, 2, 0x3A4, US_REG_DWORD},
    {"B03_D14_F2_0x3A8", 3, 14, 2, 0x3A8, US_REG_DWORD},
    {"B03_D14_F2_0x3AC", 3, 14, 2, 0x3AC, US_REG_DWORD},
    {"B03_D14_F2_0x800", 3, 14, 2, 0x800, US_REG_DWORD},
    {"B03_D14_F2_0x820", 3, 14, 2, 0x820, US_REG_DWORD},
    {"B03_D14_F2_0x824", 3, 14, 2, 0x824, US_REG_DWORD},
    {"B03_D14_F2_0x82C", 3, 14, 2, 0x82C, US_REG_DWORD},
    {"B03_D14_F2_0x9F0", 3, 14, 2, 0x9F0, US_REG_DWORD},
    {"B03_D14_F2_0x9F4", 3, 14, 2, 0x9F4, US_REG_DWORD},
    {"B03_D14_F2_0x9F8", 3, 14, 2, 0x9F8, US_REG_DWORD},
    {"B03_D14_F4_0x84", 3, 14, 4, 0x84, US_REG_DWORD},
    {"B03_D14_F4_0x8C", 3, 14, 4, 0x8C, US_REG_DWORD},
    {"B03_D14_F4_0x90", 3, 14, 4, 0x90, US_REG_DWORD},
    {"B03_D14_F4_0x9C", 3, 14, 4, 0x9C, US_REG_BYTE},
    {"B03_D14_F4_0xA0", 3, 14, 4, 0xA0, US_REG_BYTE},
    {"B03_D14_F4_0xA4", 3, 14, 4, 0xA4, US_REG_BYTE},
    {"B03_D14_F4_0xA8", 3, 14, 4, 0xA8, US_REG_QWORD},
    {"B03_D14_F4_0xB0", 3, 14, 4, 0xB0, US_REG_QWORD},
    {"B03_D14_F4_0xB8", 3, 14, 4, 0xB8, US_REG_QWORD},
    {"B03_D14_F4_0xC0", 3, 14, 4, 0xC0, US_REG_DWORD},
    {"B03_D14_F4_0xC4", 3, 14, 4, 0xC4, US_REG_BYTE},
    {"B03_D14_F4_0xC8", 3, 14, 4, 0xC8, US_REG_BYTE},
    {"B03_D14_F4_0xD4", 3, 14, 4, 0xD4, US_REG_DWORD},
    {"B03_D14_F4_0xDC", 3, 14, 4, 0xDC, US_REG_DWORD},
    {"B03_D14_F4_0xE4", 3, 14, 4, 0xE4, US_REG_DWORD},
    {"B03_D14_F5_0x12C", 3, 14, 5, 0x12C, US_REG_DWORD},
    {"B03_D14_F5_0x130", 3, 14, 5, 0x130, US_REG_DWORD},
    {"B03_D14_F5_0x13C", 3, 14, 5, 0x13C, US_REG_DWORD},
    {"B03_D14_F5_0x140", 3, 14, 5, 0x140, US_REG_DWORD},
    {"B03_D14_F5_0x14C", 3, 14, 5, 0x14C, US_REG_DWORD},
    {"B03_D14_F5_0x154", 3, 14, 5, 0x154, US_REG_DWORD},
    {"B03_D14_F5_0x160", 3, 14, 5, 0x160, US_REG_DWORD},
    {"B03_D14_F5_0x164", 3, 14, 5, 0x164, US_REG_DWORD},
    {"B03_D14_F5_0x168", 3, 14, 5, 0x168, US_REG_DWORD},
    {"B03_D14_F5_0x16C", 3, 14, 5, 0x16C, US_REG_DWORD},
    {"B03_D14_F5_0x170", 3, 14, 5, 0x170, US_REG_DWORD},
    {"B03_D14_F5_0x178", 3, 14, 5, 0x178, US_REG_DWORD},
    {"B03_D14_F5_0x1B4", 3, 14, 5, 0x1B4, US_REG_DWORD},
    {"B03_D14_F5_0x1C0", 3, 14, 5, 0x1C0, US_REG_DWORD},
    {"B03_D14_F6_0x3A0", 3, 14, 6, 0x3A0, US_REG_DWORD},
    {"B03_D14_F6_0x3A4", 3, 14, 6, 0x3A4, US_REG_DWORD},
    {"B03_D14_F6_0x3A8", 3, 14, 6, 0x3A8, US_REG_DWORD},
    {"B03_D14_F6_0x3AC", 3, 14, 6, 0x3AC, US_REG_DWORD},
    {"B03_D14_F6_0x800", 3, 14, 6, 0x800, US_REG_DWORD},
    {"B03_D14_F6_0x820", 3, 14, 6, 0x820, US_REG_DWORD},
    {"B03_D14_F6_0x824", 3, 14, 6, 0x824, US_REG_DWORD},
    {"B03_D14_F6_0x82C", 3, 14, 6, 0x82C, US_REG_DWORD},
    {"B03_D14_F6_0x9F0", 3, 14, 6, 0x9F0, US_REG_DWORD},
    {"B03_D14_F6_0x9F4", 3, 14, 6, 0x9F4, US_REG_DWORD},
    {"B03_D14_F6_0x9F8", 3, 14, 6, 0x9F8, US_REG_DWORD},
    {"B03_D15_F0_0x84", 3, 15, 0, 0x84, US_REG_DWORD},
    {"B03_D15_F0_0x8C", 3, 15, 0, 0x8C, US_REG_DWORD},
    {"B03_D15_F0_0x90", 3, 15, 0, 0x90, US_REG_DWORD},
    {"B03_D15_F0_0x9C", 3, 15, 0, 0x9C, US_REG_BYTE},
    {"B03_D15_F0_0xA0", 3, 15, 0, 0xA0, US_REG_BYTE},
    {"B03_D15_F0_0xA4", 3, 15, 0, 0xA4, US_REG_BYTE},
    {"B03_D15_F0_0xA8", 3, 15, 0, 0xA8, US_REG_QWORD},
    {"B03_D15_F0_0xB0", 3, 15, 0, 0xB0, US_REG_QWORD},
    {"B03_D15_F0_0xB8", 3, 15, 0, 0xB8, US_REG_QWORD},
    {"B03_D15_F0_0xC0", 3, 15, 0, 0xC0, US_REG_DWORD},
    {"B03_D15_F0_0xC4", 3, 15, 0, 0xC4, US_REG_BYTE},
    {"B03_D15_F0_0xC8", 3, 15, 0, 0xC8, US_REG_BYTE},
    {"B03_D15_F0_0xD4", 3, 15, 0, 0xD4, US_REG_DWORD},
    {"B03_D15_F0_0xDC", 3, 15, 0, 0xDC, US_REG_DWORD},
    {"B03_D15_F0_0xE4", 3, 15, 0, 0xE4, US_REG_DWORD},
    {"B03_D15_F1_0x12C", 3, 15, 1, 0x12C, US_REG_DWORD},
    {"B03_D15_F1_0x130", 3, 15, 1, 0x130, US_REG_DWORD},
    {"B03_D15_F1_0x13C", 3, 15, 1, 0x13C, US_REG_DWORD},
    {"B03_D15_F1_0x140", 3, 15, 1, 0x140, US_REG_DWORD},
    {"B03_D15_F1_0x14C", 3, 15, 1, 0x14C, US_REG_DWORD},
    {"B03_D15_F1_0x154", 3, 15, 1, 0x154, US_REG_DWORD},
    {"B03_D15_F1_0x160", 3, 15, 1, 0x160, US_REG_DWORD},
    {"B03_D15_F1_0x164", 3, 15, 1, 0x164, US_REG_DWORD},
    {"B03_D15_F1_0x168", 3, 15, 1, 0x168, US_REG_DWORD},
    {"B03_D15_F1_0x16C", 3, 15, 1, 0x16C, US_REG_DWORD},
    {"B03_D15_F1_0x170", 3, 15, 1, 0x170, US_REG_DWORD},
    {"B03_D15_F1_0x178", 3, 15, 1, 0x178, US_REG_DWORD},
    {"B03_D15_F1_0x1B4", 3, 15, 1, 0x1B4, US_REG_DWORD},
    {"B03_D15_F1_0x1C0", 3, 15, 1, 0x1C0, US_REG_DWORD},
    {"B03_D15_F2_0x3A0", 3, 15, 2, 0x3A0, US_REG_DWORD},
    {"B03_D15_F2_0x3A4", 3, 15, 2, 0x3A4, US_REG_DWORD},
    {"B03_D15_F2_0x3A8", 3, 15, 2, 0x3A8, US_REG_DWORD},
    {"B03_D15_F2_0x3AC", 3, 15, 2, 0x3AC, US_REG_DWORD},
    {"B03_D15_F2_0x800", 3, 15, 2, 0x800, US_REG_DWORD},
    {"B03_D15_F2_0x820", 3, 15, 2, 0x820, US_REG_DWORD},
    {"B03_D15_F2_0x824", 3, 15, 2, 0x824, US_REG_DWORD},
    {"B03_D15_F2_0x82C", 3, 15, 2, 0x82C, US_REG_DWORD},
    {"B03_D15_F2_0x9F0", 3, 15, 2, 0x9F0, US_REG_DWORD},
    {"B03_D15_F2_0x9F4", 3, 15, 2, 0x9F4, US_REG_DWORD},
    {"B03_D15_F2_0x9F8", 3, 15, 2, 0x9F8, US_REG_DWORD},
    {"B03_D15_F4_0x84", 3, 15, 4, 0x84, US_REG_DWORD},
    {"B03_D15_F4_0x8C", 3, 15, 4, 0x8C, US_REG_DWORD},
    {"B03_D15_F4_0x90", 3, 15, 4, 0x90, US_REG_DWORD},
    {"B03_D15_F4_0x9C", 3, 15, 4, 0x9C, US_REG_BYTE},
    {"B03_D15_F4_0xA0", 3, 15, 4, 0xA0, US_REG_BYTE},
    {"B03_D15_F4_0xA4", 3, 15, 4, 0xA4, US_REG_BYTE},
    {"B03_D15_F4_0xA8", 3, 15, 4, 0xA8, US_REG_QWORD},
    {"B03_D15_F4_0xB0", 3, 15, 4, 0xB0, US_REG_QWORD},
    {"B03_D15_F4_0xB8", 3, 15, 4, 0xB8, US_REG_QWORD},
    {"B03_D15_F4_0xC0", 3, 15, 4, 0xC0, US_REG_DWORD},
    {"B03_D15_F4_0xC4", 3, 15, 4, 0xC4, US_REG_BYTE},
    {"B03_D15_F4_0xC8", 3, 15, 4, 0xC8, US_REG_BYTE},
    {"B03_D15_F4_0xD4", 3, 15, 4, 0xD4, US_REG_DWORD},
    {"B03_D15_F4_0xDC", 3, 15, 4, 0xDC, US_REG_DWORD},
    {"B03_D15_F4_0xE4", 3, 15, 4, 0xE4, US_REG_DWORD},
    {"B03_D15_F5_0x12C", 3, 15, 5, 0x12C, US_REG_DWORD},
    {"B03_D15_F5_0x130", 3, 15, 5, 0x130, US_REG_DWORD},
    {"B03_D15_F5_0x13C", 3, 15, 5, 0x13C, US_REG_DWORD},
    {"B03_D15_F5_0x140", 3, 15, 5, 0x140, US_REG_DWORD},
    {"B03_D15_F5_0x14C", 3, 15, 5, 0x14C, US_REG_DWORD},
    {"B03_D15_F5_0x154", 3, 15, 5, 0x154, US_REG_DWORD},
    {"B03_D15_F5_0x160", 3, 15, 5, 0x160, US_REG_DWORD},
    {"B03_D15_F5_0x164", 3, 15, 5, 0x164, US_REG_DWORD},
    {"B03_D15_F5_0x168", 3, 15, 5, 0x168, US_REG_DWORD},
    {"B03_D15_F5_0x16C", 3, 15, 5, 0x16C, US_REG_DWORD},
    {"B03_D15_F5_0x170", 3, 15, 5, 0x170, US_REG_DWORD},
    {"B03_D15_F5_0x178", 3, 15, 5, 0x178, US_REG_DWORD},
    {"B03_D15_F5_0x1B4", 3, 15, 5, 0x1B4, US_REG_DWORD},
    {"B03_D15_F5_0x1C0", 3, 15, 5, 0x1C0, US_REG_DWORD},
    {"B03_D15_F6_0x3A0", 3, 15, 6, 0x3A0, US_REG_DWORD},
    {"B03_D15_F6_0x3A4", 3, 15, 6, 0x3A4, US_REG_DWORD},
    {"B03_D15_F6_0x3A8", 3, 15, 6, 0x3A8, US_REG_DWORD},
    {"B03_D15_F6_0x3AC", 3, 15, 6, 0x3AC, US_REG_DWORD},
    {"B03_D15_F6_0x800", 3, 15, 6, 0x800, US_REG_DWORD},
    {"B03_D15_F6_0x820", 3, 15, 6, 0x820, US_REG_DWORD},
    {"B03_D15_F6_0x824", 3, 15, 6, 0x824, US_REG_DWORD},
    {"B03_D15_F6_0x82C", 3, 15, 6, 0x82C, US_REG_DWORD},
    {"B03_D15_F6_0x9F0", 3, 15, 6, 0x9F0, US_REG_DWORD},
    {"B03_D15_F6_0x9F4", 3, 15, 6, 0x9F4, US_REG_DWORD},
    {"B03_D15_F6_0x9F8", 3, 15, 6, 0x9F8, US_REG_DWORD},
    {"B03_D16_F0_0x84", 3, 16, 0, 0x84, US_REG_DWORD},
    {"B03_D16_F0_0x8C", 3, 16, 0, 0x8C, US_REG_DWORD},
    {"B03_D16_F0_0x90", 3, 16, 0, 0x90, US_REG_DWORD},
    {"B03_D16_F0_0x9C", 3, 16, 0, 0x9C, US_REG_BYTE},
    {"B03_D16_F0_0xA0", 3, 16, 0, 0xA0, US_REG_BYTE},
    {"B03_D16_F0_0xA4", 3, 16, 0, 0xA4, US_REG_BYTE},
    {"B03_D16_F0_0xA8", 3, 16, 0, 0xA8, US_REG_QWORD},
    {"B03_D16_F0_0xB0", 3, 16, 0, 0xB0, US_REG_QWORD},
    {"B03_D16_F0_0xB8", 3, 16, 0, 0xB8, US_REG_QWORD},
    {"B03_D16_F0_0xC0", 3, 16, 0, 0xC0, US_REG_DWORD},
    {"B03_D16_F0_0xC4", 3, 16, 0, 0xC4, US_REG_BYTE},
    {"B03_D16_F0_0xC8", 3, 16, 0, 0xC8, US_REG_BYTE},
    {"B03_D16_F0_0xD4", 3, 16, 0, 0xD4, US_REG_DWORD},
    {"B03_D16_F0_0xDC", 3, 16, 0, 0xDC, US_REG_DWORD},
    {"B03_D16_F0_0xE4", 3, 16, 0, 0xE4, US_REG_DWORD},
    {"B03_D16_F1_0x12C", 3, 16, 1, 0x12C, US_REG_DWORD},
    {"B03_D16_F1_0x130", 3, 16, 1, 0x130, US_REG_DWORD},
    {"B03_D16_F1_0x13C", 3, 16, 1, 0x13C, US_REG_DWORD},
    {"B03_D16_F1_0x140", 3, 16, 1, 0x140, US_REG_DWORD},
    {"B03_D16_F1_0x14C", 3, 16, 1, 0x14C, US_REG_DWORD},
    {"B03_D16_F1_0x154", 3, 16, 1, 0x154, US_REG_DWORD},
    {"B03_D16_F1_0x160", 3, 16, 1, 0x160, US_REG_DWORD},
    {"B03_D16_F1_0x164", 3, 16, 1, 0x164, US_REG_DWORD},
    {"B03_D16_F1_0x168", 3, 16, 1, 0x168, US_REG_DWORD},
    {"B03_D16_F1_0x16C", 3, 16, 1, 0x16C, US_REG_DWORD},
    {"B03_D16_F1_0x170", 3, 16, 1, 0x170, US_REG_DWORD},
    {"B03_D16_F1_0x178", 3, 16, 1, 0x178, US_REG_DWORD},
    {"B03_D16_F1_0x1B4", 3, 16, 1, 0x1B4, US_REG_DWORD},
    {"B03_D16_F1_0x1C0", 3, 16, 1, 0x1C0, US_REG_DWORD},
    {"B03_D16_F2_0x3A0", 3, 16, 2, 0x3A0, US_REG_DWORD},
    {"B03_D16_F2_0x3A4", 3, 16, 2, 0x3A4, US_REG_DWORD},
    {"B03_D16_F2_0x3A8", 3, 16, 2, 0x3A8, US_REG_DWORD},
    {"B03_D16_F2_0x3AC", 3, 16, 2, 0x3AC, US_REG_DWORD},
    {"B03_D16_F2_0x800", 3, 16, 2, 0x800, US_REG_DWORD},
    {"B03_D16_F2_0x820", 3, 16, 2, 0x820, US_REG_DWORD},
    {"B03_D16_F2_0x824", 3, 16, 2, 0x824, US_REG_DWORD},
    {"B03_D16_F2_0x82C", 3, 16, 2, 0x82C, US_REG_DWORD},
    {"B03_D16_F2_0x9F0", 3, 16, 2, 0x9F0, US_REG_DWORD},
    {"B03_D16_F2_0x9F4", 3, 16, 2, 0x9F4, US_REG_DWORD},
    {"B03_D16_F2_0x9F8", 3, 16, 2, 0x9F8, US_REG_DWORD},
    {"B03_D16_F4_0x84", 3, 16, 4, 0x84, US_REG_DWORD},
    {"B03_D16_F4_0x8C", 3, 16, 4, 0x8C, US_REG_DWORD},
    {"B03_D16_F4_0x90", 3, 16, 4, 0x90, US_REG_DWORD},
    {"B03_D16_F4_0x9C", 3, 16, 4, 0x9C, US_REG_BYTE},
    {"B03_D16_F4_0xA0", 3, 16, 4, 0xA0, US_REG_BYTE},
    {"B03_D16_F4_0xA4", 3, 16, 4, 0xA4, US_REG_BYTE},
    {"B03_D16_F4_0xA8", 3, 16, 4, 0xA8, US_REG_QWORD},
    {"B03_D16_F4_0xB0", 3, 16, 4, 0xB0, US_REG_QWORD},
    {"B03_D16_F4_0xB8", 3, 16, 4, 0xB8, US_REG_QWORD},
    {"B03_D16_F4_0xC0", 3, 16, 4, 0xC0, US_REG_DWORD},
    {"B03_D16_F4_0xC4", 3, 16, 4, 0xC4, US_REG_BYTE},
    {"B03_D16_F4_0xC8", 3, 16, 4, 0xC8, US_REG_BYTE},
    {"B03_D16_F4_0xD4", 3, 16, 4, 0xD4, US_REG_DWORD},
    {"B03_D16_F4_0xDC", 3, 16, 4, 0xDC, US_REG_DWORD},
    {"B03_D16_F4_0xE4", 3, 16, 4, 0xE4, US_REG_DWORD},
    {"B03_D16_F5_0x12C", 3, 16, 5, 0x12C, US_REG_DWORD},
    {"B03_D16_F5_0x130", 3, 16, 5, 0x130, US_REG_DWORD},
    {"B03_D16_F5_0x13C", 3, 16, 5, 0x13C, US_REG_DWORD},
    {"B03_D16_F5_0x140", 3, 16, 5, 0x140, US_REG_DWORD},
    {"B03_D16_F5_0x14C", 3, 16, 5, 0x14C, US_REG_DWORD},
    {"B03_D16_F5_0x154", 3, 16, 5, 0x154, US_REG_DWORD},
    {"B03_D16_F5_0x160", 3, 16, 5, 0x160, US_REG_DWORD},
    {"B03_D16_F5_0x164", 3, 16, 5, 0x164, US_REG_DWORD},
    {"B03_D16_F5_0x168", 3, 16, 5, 0x168, US_REG_DWORD},
    {"B03_D16_F5_0x16C", 3, 16, 5, 0x16C, US_REG_DWORD},
    {"B03_D16_F5_0x170", 3, 16, 5, 0x170, US_REG_DWORD},
    {"B03_D16_F5_0x178", 3, 16, 5, 0x178, US_REG_DWORD},
    {"B03_D16_F5_0x1B4", 3, 16, 5, 0x1B4, US_REG_DWORD},
    {"B03_D16_F5_0x1C0", 3, 16, 5, 0x1C0, US_REG_DWORD},
    {"B03_D16_F6_0x3A0", 3, 16, 6, 0x3A0, US_REG_DWORD},
    {"B03_D16_F6_0x3A4", 3, 16, 6, 0x3A4, US_REG_DWORD},
    {"B03_D16_F6_0x3A8", 3, 16, 6, 0x3A8, US_REG_DWORD},
    {"B03_D16_F6_0x3AC", 3, 16, 6, 0x3AC, US_REG_DWORD},
    {"B03_D16_F6_0x800", 3, 16, 6, 0x800, US_REG_DWORD},
    {"B03_D16_F6_0x820", 3, 16, 6, 0x820, US_REG_DWORD},
    {"B03_D16_F6_0x824", 3, 16, 6, 0x824, US_REG_DWORD},
    {"B03_D16_F6_0x82C", 3, 16, 6, 0x82C, US_REG_DWORD},
    {"B03_D16_F6_0x9F0", 3, 16, 6, 0x9F0, US_REG_DWORD},
    {"B03_D16_F6_0x9F4", 3, 16, 6, 0x9F4, US_REG_DWORD},
    {"B03_D16_F6_0x9F8", 3, 16, 6, 0x9F8, US_REG_DWORD},
    {"B03_D18_F0_0x118", 3, 18, 0, 0x118, US_REG_DWORD},
    {"B03_D18_F0_0x11C", 3, 18, 0, 0x11C, US_REG_DWORD},
    {"B03_D18_F0_0x120", 3, 18, 0, 0x120, US_REG_DWORD},
    {"B03_D18_F0_0x124", 3, 18, 0, 0x124, US_REG_DWORD},
    {"B03_D18_F0_0x128", 3, 18, 0, 0x128, US_REG_DWORD},
    {"B03_D18_F0_0x12C", 3, 18, 0, 0x12C, US_REG_DWORD},
    {"B03_D18_F0_0x130", 3, 18, 0, 0x130, US_REG_DWORD},
    {"B03_D18_F0_0x134", 3, 18, 0, 0x134, US_REG_DWORD},
    {"B03_D18_F0_0x138", 3, 18, 0, 0x138, US_REG_DWORD},
    {"B03_D18_F0_0x13C", 3, 18, 0, 0x13C, US_REG_DWORD},
    {"B03_D18_F0_0x148", 3, 18, 0, 0x148, US_REG_DWORD},
    {"B03_D18_F0_0x158", 3, 18, 0, 0x158, US_REG_DWORD},
    {"B03_D18_F0_0x15C", 3, 18, 0, 0x15C, US_REG_DWORD},
    {"B03_D18_F0_0x160", 3, 18, 0, 0x160, US_REG_DWORD},
    {"B03_D18_F0_0x164", 3, 18, 0, 0x164, US_REG_DWORD},
    {"B03_D18_F0_0x168", 3, 18, 0, 0x168, US_REG_DWORD},
    {"B03_D18_F0_0x16C", 3, 18, 0, 0x16C, US_REG_DWORD},
    {"B03_D18_F4_0x118", 3, 18, 4, 0x118, US_REG_DWORD},
    {"B03_D18_F4_0x11C", 3, 18, 4, 0x11C, US_REG_DWORD},
    {"B03_D18_F4_0x120", 3, 18, 4, 0x120, US_REG_DWORD},
    {"B03_D18_F4_0x124", 3, 18, 4, 0x124, US_REG_DWORD},
    {"B03_D18_F4_0x128", 3, 18, 4, 0x128, US_REG_DWORD},
    {"B03_D18_F4_0x12C", 3, 18, 4, 0x12C, US_REG_DWORD},
    {"B03_D18_F4_0x130", 3, 18, 4, 0x130, US_REG_DWORD},
    {"B03_D18_F4_0x134", 3, 18, 4, 0x134, US_REG_DWORD},
    {"B03_D18_F4_0x138", 3, 18, 4, 0x138, US_REG_DWORD},
    {"B03_D18_F4_0x13C", 3, 18, 4, 0x13C, US_REG_DWORD},
    {"B03_D18_F4_0x148", 3, 18, 4, 0x148, US_REG_DWORD},
    {"B03_D18_F4_0x158", 3, 18, 4, 0x158, US_REG_DWORD},
    {"B03_D18_F4_0x15C", 3, 18, 4, 0x15C, US_REG_DWORD},
    {"B03_D18_F4_0x160", 3, 18, 4, 0x160, US_REG_DWORD},
    {"B03_D18_F4_0x164", 3, 18, 4, 0x164, US_REG_DWORD},
    {"B03_D18_F4_0x168", 3, 18, 4, 0x168, US_REG_DWORD},
    {"B03_D18_F4_0x16C", 3, 18, 4, 0x16C, US_REG_DWORD},
    {"B03_D21_F0_0x60", 3, 21, 0, 0x60, US_REG_DWORD},
    {"B03_D21_F0_0x64", 3, 21, 0, 0x64, US_REG_DWORD},
    {"B03_D21_F0_0x68", 3, 21, 0, 0x68, US_REG_DWORD},
    {"B03_D21_F0_0x6C", 3, 21, 0, 0x6C, US_REG_DWORD},
    {"B03_D21_F0_0xB0", 3, 21, 0, 0xB0, US_REG_DWORD},
    {"B03_D22_F0_0x60", 3, 22, 0, 0x60, US_REG_DWORD},
    {"B03_D22_F0_0x68", 3, 22, 0, 0x68, US_REG_DWORD},
    {"B03_D22_F0_0x6C", 3, 22, 0, 0x6C, US_REG_DWORD},
    {"B03_D22_F0_0xB0", 3, 22, 0, 0xB0, US_REG_DWORD},
    {"B03_D22_F4_0x60", 3, 22, 4, 0x60, US_REG_DWORD},
    {"B03_D22_F4_0x64", 3, 22, 4, 0x64, US_REG_DWORD},
    {"B03_D22_F4_0x68", 3, 22, 4, 0x68, US_REG_DWORD},
    {"B03_D22_F4_0x6C", 3, 22, 4, 0x6C, US_REG_DWORD},
    {"B03_D22_F4_0xA8", 3, 22, 4, 0xA8, US_REG_DWORD},
    {"B03_D22_F4_0xB0", 3, 22, 4, 0xB0, US_REG_DWORD},
    {"B03_D23_F0_0x60", 3, 23, 0, 0x60, US_REG_DWORD},
    {"B03_D23_F0_0x64", 3, 23, 0, 0x64, US_REG_DWORD},
    {"B03_D23_F0_0x68", 3, 23, 0, 0x68, US_REG_DWORD},
    {"B03_D23_F0_0x6C", 3, 23, 0, 0x6C, US_REG_DWORD},
    {"B03_D23_F0_0xB0", 3, 23, 0, 0xB0, US_REG_DWORD},
    {"B04_D00_F0_0x0", 4, 0, 0, 0x0, US_REG_WORD},
    {"B04_D00_F0_0x14C", 4, 0, 0, 0x14C, US_REG_DWORD},
    {"B04_D00_F0_0x158", 4, 0, 0, 0x158, US_REG_DWORD},
    {"B04_D00_F0_0x178", 4, 0, 0, 0x178, US_REG_DWORD},
    {"B04_D00_F0_0x17C", 4, 0, 0, 0x17C, US_REG_DWORD},
    {"B04_D00_F0_0x18", 4, 0, 0, 0x18, US_REG_BYTE},
    {"B04_D00_F0_0x188", 4, 0, 0, 0x188, US_REG_DWORD},
    {"B04_D00_F0_0x18C", 4, 0, 0, 0x18C, US_REG_DWORD},
    {"B04_D00_F0_0x19", 4, 0, 0, 0x19, US_REG_BYTE},
    {"B04_D00_F0_0x1A", 4, 0, 0, 0x1A, US_REG_BYTE},
    {"B04_D00_F0_0x1C", 4, 0, 0, 0x1C, US_REG_BYTE},
    {"B04_D00_F0_0x1D", 4, 0, 0, 0x1D, US_REG_BYTE},
    {"B04_D00_F0_0x1E", 4, 0, 0, 0x1E, US_REG_WORD},
    {"B04_D00_F0_0x2", 4, 0, 0, 0x2, US_REG_WORD},
    {"B04_D00_F0_0x20", 4, 0, 0, 0x20, US_REG_WORD},
    {"B04_D00_F0_0x200", 4, 0, 0, 0x200, US_REG_BYTE},
    {"B04_D00_F0_0x204", 4, 0, 0, 0x204, US_REG_BYTE},
    {"B04_D00_F0_0x208", 4, 0, 0, 0x208, US_REG_BYTE},
    {"B04_D00_F0_0x20C", 4, 0, 0, 0x20C, US_REG_BYTE},
    {"B04_D00_F0_0x210", 4, 0, 0, 0x210, US_REG_BYTE},
    {"B04_D00_F0_0x214", 4, 0, 0, 0x214, US_REG_BYTE},
    {"B04_D00_F0_0x218", 4, 0, 0, 0x218, US_REG_BYTE},
    {"B04_D00_F0_0x21C", 4, 0, 0, 0x21C, US_REG_BYTE},
    {"B04_D00_F0_0x22", 4, 0, 0, 0x22, US_REG_WORD},
    {"B04_D00_F0_0x220", 4, 0, 0, 0x220, US_REG_BYTE},
    {"B04_D00_F0_0x224", 4, 0, 0, 0x224, US_REG_BYTE},
    {"B04_D00_F0_0x228", 4, 0, 0, 0x228, US_REG_BYTE},
    {"B04_D00_F0_0x230", 4, 0, 0, 0x230, US_REG_BYTE},
    {"B04_D00_F0_0x232", 4, 0, 0, 0x232, US_REG_BYTE},
    {"B04_D00_F0_0x24", 4, 0, 0, 0x24, US_REG_WORD},
    {"B04_D00_F0_0x258", 4, 0, 0, 0x258, US_REG_DWORD},
    {"B04_D00_F0_0x26", 4, 0, 0, 0x26, US_REG_WORD},
    {"B04_D00_F0_0x28", 4, 0, 0, 0x28, US_REG_DWORD},
    {"B04_D00_F0_0x288", 4, 0, 0, 0x288, US_REG_DWORD},
    {"B04_D00_F0_0x298", 4, 0, 0, 0x298, US_REG_DWORD},
    {"B04_D00_F0_0x2A4", 4, 0, 0, 0x2A4, US_REG_DWORD},
    {"B04_D00_F0_0x2C", 4, 0, 0, 0x2C, US_REG_DWORD},
    {"B04_D00_F0_0x4", 4, 0, 0, 0x4, US_REG_WORD},
    {"B04_D00_F0_0x6", 4, 0, 0, 0x6, US_REG_WORD},
    {"B04_D00_F0_0x9A", 4, 0, 0, 0x9A, US_REG_WORD},
    {"B04_D00_F0_0xA2", 4, 0, 0, 0xA2, US_REG_WORD},
    {"B04_D00_F0_0xAA", 4, 0, 0, 0xAA, US_REG_WORD},
    {"B04_D00_F0_0xC2", 4, 0, 0, 0xC2, US_REG_WORD},
    {"B04_D01_F0_0x0", 4, 1, 0, 0x0, US_REG_WORD},
    {"B04_D01_F0_0x14C", 4, 1, 0, 0x14C, US_REG_DWORD},
    {"B04_D01_F0_0x158", 4, 1, 0, 0x158, US_REG_DWORD},
    {"B04_D01_F0_0x178", 4, 1, 0, 0x178, US_REG_DWORD},
    {"B04_D01_F0_0x17C", 4, 1, 0, 0x17C, US_REG_DWORD},
    {"B04_D01_F0_0x18", 4, 1, 0, 0x18, US_REG_BYTE},
    {"B04_D01_F0_0x188", 4, 1, 0, 0x188, US_REG_DWORD},
    {"B04_D01_F0_0x18C", 4, 1, 0, 0x18C, US_REG_DWORD},
    {"B04_D01_F0_0x19", 4, 1, 0, 0x19, US_REG_BYTE},
    {"B04_D01_F0_0x1A", 4, 1, 0, 0x1A, US_REG_BYTE},
    {"B04_D01_F0_0x1C", 4, 1, 0, 0x1C, US_REG_BYTE},
    {"B04_D01_F0_0x1D", 4, 1, 0, 0x1D, US_REG_BYTE},
    {"B04_D01_F0_0x1E", 4, 1, 0, 0x1E, US_REG_WORD},
    {"B04_D01_F0_0x2", 4, 1, 0, 0x2, US_REG_WORD},
    {"B04_D01_F0_0x20", 4, 1, 0, 0x20, US_REG_WORD},
    {"B04_D01_F0_0x200", 4, 1, 0, 0x200, US_REG_BYTE},
    {"B04_D01_F0_0x204", 4, 1, 0, 0x204, US_REG_BYTE},
    {"B04_D01_F0_0x208", 4, 1, 0, 0x208, US_REG_BYTE},
    {"B04_D01_F0_0x20C", 4, 1, 0, 0x20C, US_REG_BYTE},
    {"B04_D01_F0_0x210", 4, 1, 0, 0x210, US_REG_BYTE},
    {"B04_D01_F0_0x214", 4, 1, 0, 0x214, US_REG_BYTE},
    {"B04_D01_F0_0x218", 4, 1, 0, 0x218, US_REG_BYTE},
    {"B04_D01_F0_0x21C", 4, 1, 0, 0x21C, US_REG_BYTE},
    {"B04_D01_F0_0x22", 4, 1, 0, 0x22, US_REG_WORD},
    {"B04_D01_F0_0x220", 4, 1, 0, 0x220, US_REG_BYTE},
    {"B04_D01_F0_0x224", 4, 1, 0, 0x224, US_REG_BYTE},
    {"B04_D01_F0_0x228", 4, 1, 0, 0x228, US_REG_BYTE},
    {"B04_D01_F0_0x230", 4, 1, 0, 0x230, US_REG_BYTE},
    {"B04_D01_F0_0x232", 4, 1, 0, 0x232, US_REG_BYTE},
    {"B04_D01_F0_0x24", 4, 1, 0, 0x24, US_REG_WORD},
    {"B04_D01_F0_0x258", 4, 1, 0, 0x258, US_REG_DWORD},
    {"B04_D01_F0_0x26", 4, 1, 0, 0x26, US_REG_WORD},
    {"B04_D01_F0_0x28", 4, 1, 0, 0x28, US_REG_DWORD},
    {"B04_D01_F0_0x288", 4, 1, 0, 0x288, US_REG_DWORD},
    {"B04_D01_F0_0x298", 4, 1, 0, 0x298, US_REG_DWORD},
    {"B04_D01_F0_0x2A4", 4, 1, 0, 0x2A4, US_REG_DWORD},
    {"B04_D01_F0_0x2C", 4, 1, 0, 0x2C, US_REG_DWORD},
    {"B04_D01_F0_0x4", 4, 1, 0, 0x4, US_REG_WORD},
    {"B04_D01_F0_0x6", 4, 1, 0, 0x6, US_REG_WORD},
    {"B04_D01_F0_0x9A", 4, 1, 0, 0x9A, US_REG_WORD},
    {"B04_D01_F0_0xA2", 4, 1, 0, 0xA2, US_REG_WORD},
    {"B04_D01_F0_0xAA", 4, 1, 0, 0xAA, US_REG_WORD},
    {"B04_D01_F0_0xC2", 4, 1, 0, 0xC2, US_REG_WORD},
    {"B04_D02_F0_0x0", 4, 2, 0, 0x0, US_REG_WORD},
    {"B04_D02_F0_0x14C", 4, 2, 0, 0x14C, US_REG_DWORD},
    {"B04_D02_F0_0x158", 4, 2, 0, 0x158, US_REG_DWORD},
    {"B04_D02_F0_0x178", 4, 2, 0, 0x178, US_REG_DWORD},
    {"B04_D02_F0_0x17C", 4, 2, 0, 0x17C, US_REG_DWORD},
    {"B04_D02_F0_0x18", 4, 2, 0, 0x18, US_REG_BYTE},
    {"B04_D02_F0_0x188", 4, 2, 0, 0x188, US_REG_DWORD},
    {"B04_D02_F0_0x18C", 4, 2, 0, 0x18C, US_REG_DWORD},
    {"B04_D02_F0_0x19", 4, 2, 0, 0x19, US_REG_BYTE},
    {"B04_D02_F0_0x1A", 4, 2, 0, 0x1A, US_REG_BYTE},
    {"B04_D02_F0_0x1C", 4, 2, 0, 0x1C, US_REG_BYTE},
    {"B04_D02_F0_0x1D", 4, 2, 0, 0x1D, US_REG_BYTE},
    {"B04_D02_F0_0x1E", 4, 2, 0, 0x1E, US_REG_WORD},
    {"B04_D02_F0_0x2", 4, 2, 0, 0x2, US_REG_WORD},
    {"B04_D02_F0_0x20", 4, 2, 0, 0x20, US_REG_WORD},
    {"B04_D02_F0_0x200", 4, 2, 0, 0x200, US_REG_BYTE},
    {"B04_D02_F0_0x204", 4, 2, 0, 0x204, US_REG_BYTE},
    {"B04_D02_F0_0x208", 4, 2, 0, 0x208, US_REG_BYTE},
    {"B04_D02_F0_0x20C", 4, 2, 0, 0x20C, US_REG_BYTE},
    {"B04_D02_F0_0x210", 4, 2, 0, 0x210, US_REG_BYTE},
    {"B04_D02_F0_0x214", 4, 2, 0, 0x214, US_REG_BYTE},
    {"B04_D02_F0_0x218", 4, 2, 0, 0x218, US_REG_BYTE},
    {"B04_D02_F0_0x21C", 4, 2, 0, 0x21C, US_REG_BYTE},
    {"B04_D02_F0_0x22", 4, 2, 0, 0x22, US_REG_WORD},
    {"B04_D02_F0_0x220", 4, 2, 0, 0x220, US_REG_BYTE},
    {"B04_D02_F0_0x224", 4, 2, 0, 0x224, US_REG_BYTE},
    {"B04_D02_F0_0x228", 4, 2, 0, 0x228, US_REG_BYTE},
    {"B04_D02_F0_0x230", 4, 2, 0, 0x230, US_REG_BYTE},
    {"B04_D02_F0_0x232", 4, 2, 0, 0x232, US_REG_BYTE},
    {"B04_D02_F0_0x24", 4, 2, 0, 0x24, US_REG_WORD},
    {"B04_D02_F0_0x258", 4, 2, 0, 0x258, US_REG_DWORD},
    {"B04_D02_F0_0x26", 4, 2, 0, 0x26, US_REG_WORD},
    {"B04_D02_F0_0x28", 4, 2, 0, 0x28, US_REG_DWORD},
    {"B04_D02_F0_0x288", 4, 2, 0, 0x288, US_REG_DWORD},
    {"B04_D02_F0_0x298", 4, 2, 0, 0x298, US_REG_DWORD},
    {"B04_D02_F0_0x2A4", 4, 2, 0, 0x2A4, US_REG_DWORD},
    {"B04_D02_F0_0x2C", 4, 2, 0, 0x2C, US_REG_DWORD},
    {"B04_D02_F0_0x4", 4, 2, 0, 0x4, US_REG_WORD},
    {"B04_D02_F0_0x6", 4, 2, 0, 0x6, US_REG_WORD},
    {"B04_D02_F0_0x9A", 4, 2, 0, 0x9A, US_REG_WORD},
    {"B04_D02_F0_0xA2", 4, 2, 0, 0xA2, US_REG_WORD},
    {"B04_D02_F0_0xAA", 4, 2, 0, 0xAA, US_REG_WORD},
    {"B04_D02_F0_0xC2", 4, 2, 0, 0xC2, US_REG_WORD},
    {"B04_D03_F0_0x0", 4, 3, 0, 0x0, US_REG_WORD},
    {"B04_D03_F0_0x14C", 4, 3, 0, 0x14C, US_REG_DWORD},
    {"B04_D03_F0_0x158", 4, 3, 0, 0x158, US_REG_DWORD},
    {"B04_D03_F0_0x178", 4, 3, 0, 0x178, US_REG_DWORD},
    {"B04_D03_F0_0x17C", 4, 3, 0, 0x17C, US_REG_DWORD},
    {"B04_D03_F0_0x18", 4, 3, 0, 0x18, US_REG_BYTE},
    {"B04_D03_F0_0x188", 4, 3, 0, 0x188, US_REG_DWORD},
    {"B04_D03_F0_0x18C", 4, 3, 0, 0x18C, US_REG_DWORD},
    {"B04_D03_F0_0x19", 4, 3, 0, 0x19, US_REG_BYTE},
    {"B04_D03_F0_0x1A", 4, 3, 0, 0x1A, US_REG_BYTE},
    {"B04_D03_F0_0x1C", 4, 3, 0, 0x1C, US_REG_BYTE},
    {"B04_D03_F0_0x1D", 4, 3, 0, 0x1D, US_REG_BYTE},
    {"B04_D03_F0_0x1E", 4, 3, 0, 0x1E, US_REG_WORD},
    {"B04_D03_F0_0x2", 4, 3, 0, 0x2, US_REG_WORD},
    {"B04_D03_F0_0x20", 4, 3, 0, 0x20, US_REG_WORD},
    {"B04_D03_F0_0x200", 4, 3, 0, 0x200, US_REG_BYTE},
    {"B04_D03_F0_0x204", 4, 3, 0, 0x204, US_REG_BYTE},
    {"B04_D03_F0_0x208", 4, 3, 0, 0x208, US_REG_BYTE},
    {"B04_D03_F0_0x20C", 4, 3, 0, 0x20C, US_REG_BYTE},
    {"B04_D03_F0_0x210", 4, 3, 0, 0x210, US_REG_BYTE},
    {"B04_D03_F0_0x214", 4, 3, 0, 0x214, US_REG_BYTE},
    {"B04_D03_F0_0x218", 4, 3, 0, 0x218, US_REG_BYTE},
    {"B04_D03_F0_0x21C", 4, 3, 0, 0x21C, US_REG_BYTE},
    {"B04_D03_F0_0x22", 4, 3, 0, 0x22, US_REG_WORD},
    {"B04_D03_F0_0x220", 4, 3, 0, 0x220, US_REG_BYTE},
    {"B04_D03_F0_0x224", 4, 3, 0, 0x224, US_REG_BYTE},
    {"B04_D03_F0_0x228", 4, 3, 0, 0x228, US_REG_BYTE},
    {"B04_D03_F0_0x230", 4, 3, 0, 0x230, US_REG_BYTE},
    {"B04_D03_F0_0x232", 4, 3, 0, 0x232, US_REG_BYTE},
    {"B04_D03_F0_0x24", 4, 3, 0, 0x24, US_REG_WORD},
    {"B04_D03_F0_0x258", 4, 3, 0, 0x258, US_REG_DWORD},
    {"B04_D03_F0_0x26", 4, 3, 0, 0x26, US_REG_WORD},
    {"B04_D03_F0_0x28", 4, 3, 0, 0x28, US_REG_DWORD},
    {"B04_D03_F0_0x288", 4, 3, 0, 0x288, US_REG_DWORD},
    {"B04_D03_F0_0x298", 4, 3, 0, 0x298, US_REG_DWORD},
    {"B04_D03_F0_0x2A4", 4, 3, 0, 0x2A4, US_REG_DWORD},
    {"B04_D03_F0_0x2C", 4, 3, 0, 0x2C, US_REG_DWORD},
    {"B04_D03_F0_0x4", 4, 3, 0, 0x4, US_REG_WORD},
    {"B04_D03_F0_0x6", 4, 3, 0, 0x6, US_REG_WORD},
    {"B04_D03_F0_0x9A", 4, 3, 0, 0x9A, US_REG_WORD},
    {"B04_D03_F0_0xA2", 4, 3, 0, 0xA2, US_REG_WORD},
    {"B04_D03_F0_0xAA", 4, 3, 0, 0xAA, US_REG_WORD},
    {"B04_D03_F0_0xC2", 4, 3, 0, 0xC2, US_REG_WORD},
    {"B04_D05_F2_0x0", 4, 5, 2, 0x0, US_REG_WORD},
    {"B04_D05_F2_0x19C", 4, 5, 2, 0x19C, US_REG_DWORD},
    {"B04_D05_F2_0x1A0", 4, 5, 2, 0x1A0, US_REG_DWORD},
    {"B04_D05_F2_0x1A4", 4, 5, 2, 0x1A4, US_REG_DWORD},
    {"B04_D05_F2_0x1A8", 4, 5, 2, 0x1A8, US_REG_DWORD},
    {"B04_D05_F2_0x1AC", 4, 5, 2, 0x1AC, US_REG_DWORD},
    {"B04_D05_F2_0x1B4", 4, 5, 2, 0x1B4, US_REG_DWORD},
    {"B04_D05_F2_0x1C0", 4, 5, 2, 0x1C0, US_REG_DWORD},
    {"B04_D05_F2_0x1C4", 4, 5, 2, 0x1C4, US_REG_DWORD},
    {"B04_D05_F2_0x1C8", 4, 5, 2, 0x1C8, US_REG_DWORD},
    {"B04_D05_F2_0x1CC", 4, 5, 2, 0x1CC, US_REG_DWORD},
    {"B04_D05_F2_0x1D0", 4, 5, 2, 0x1D0, US_REG_DWORD},
    {"B04_D05_F2_0x1D4", 4, 5, 2, 0x1D4, US_REG_DWORD},
    {"B04_D05_F2_0x1D8", 4, 5, 2, 0x1D8, US_REG_DWORD},
    {"B04_D05_F2_0x1DC", 4, 5, 2, 0x1DC, US_REG_DWORD},
    {"B04_D05_F2_0x1E8", 4, 5, 2, 0x1E8, US_REG_DWORD},
    {"B04_D05_F2_0x1EC", 4, 5, 2, 0x1EC, US_REG_DWORD},
    {"B04_D05_F2_0x1F8", 4, 5, 2, 0x1F8, US_REG_DWORD},
    {"B04_D05_F2_0x2", 4, 5, 2, 0x2, US_REG_WORD},
    {"B04_D05_F2_0x200", 4, 5, 2, 0x200, US_REG_DWORD},
    {"B04_D05_F2_0x208", 4, 5, 2, 0x208, US_REG_DWORD},
    {"B04_D05_F2_0x20C", 4, 5, 2, 0x20C, US_REG_DWORD},
    {"B04_D05_F2_0x210", 4, 5, 2, 0x210, US_REG_DWORD},
    {"B04_D05_F2_0x214", 4, 5, 2, 0x214, US_REG_DWORD},
    {"B04_D05_F2_0x218", 4, 5, 2, 0x218, US_REG_DWORD},
    {"B04_D05_F2_0x21C", 4, 5, 2, 0x21C, US_REG_DWORD},
    {"B04_D05_F2_0x220", 4, 5, 2, 0x220, US_REG_DWORD},
    {"B04_D05_F2_0x224", 4, 5, 2, 0x224, US_REG_DWORD},
    {"B04_D05_F2_0x228", 4, 5, 2, 0x228, US_REG_DWORD},
    {"B04_D05_F2_0x230", 4, 5, 2, 0x230, US_REG_DWORD},
    {"B04_D05_F2_0x234", 4, 5, 2, 0x234, US_REG_DWORD},
    {"B04_D05_F2_0x238", 4, 5, 2, 0x238, US_REG_DWORD},
    {"B04_D05_F2_0x23C", 4, 5, 2, 0x23C, US_REG_DWORD},
    {"B04_D05_F2_0x240", 4, 5, 2, 0x240, US_REG_DWORD},
    {"B04_D05_F2_0x244", 4, 5, 2, 0x244, US_REG_DWORD},
    {"B04_D05_F2_0x248", 4, 5, 2, 0x248, US_REG_DWORD},
    {"B04_D05_F2_0x24C", 4, 5, 2, 0x24C, US_REG_DWORD},
    {"B04_D05_F2_0x250", 4, 5, 2, 0x250, US_REG_DWORD},
    {"B04_D05_F2_0x254", 4, 5, 2, 0x254, US_REG_DWORD},
    {"B04_D05_F2_0x268", 4, 5, 2, 0x268, US_REG_DWORD},
    {"B04_D05_F2_0x26C", 4, 5, 2, 0x26C, US_REG_DWORD},
    {"B04_D05_F2_0x270", 4, 5, 2, 0x270, US_REG_DWORD},
    {"B04_D05_F2_0x274", 4, 5, 2, 0x274, US_REG_DWORD},
    {"B04_D05_F2_0x278", 4, 5, 2, 0x278, US_REG_DWORD},
    {"B04_D05_F2_0x288", 4, 5, 2, 0x288, US_REG_DWORD},
    {"B04_D05_F2_0x28C", 4, 5, 2, 0x28C, US_REG_DWORD},
    {"B04_D05_F2_0x290", 4, 5, 2, 0x290, US_REG_DWORD},
    {"B04_D05_F2_0x294", 4, 5, 2, 0x294, US_REG_DWORD},
    {"B04_D05_F2_0x298", 4, 5, 2, 0x298, US_REG_DWORD},
    {"B04_D05_F2_0x29C", 4, 5, 2, 0x29C, US_REG_DWORD},
    {"B04_D05_F2_0x2A0", 4, 5, 2, 0x2A0, US_REG_DWORD},
    {"B04_D05_F2_0x2A4", 4, 5, 2, 0x2A4, US_REG_DWORD},
    {"B04_D05_F2_0x2A8", 4, 5, 2, 0x2A8, US_REG_DWORD},
    {"B04_D05_F2_0x2AC", 4, 5, 2, 0x2AC, US_REG_DWORD},
    {"B04_D05_F2_0x2B0", 4, 5, 2, 0x2B0, US_REG_DWORD},
    {"B04_D05_F2_0x2B4", 4, 5, 2, 0x2B4, US_REG_DWORD},
    {"B04_D05_F2_0x2B8", 4, 5, 2, 0x2B8, US_REG_DWORD},
    {"B04_D05_F2_0x2CC", 4, 5, 2, 0x2CC, US_REG_DWORD},
    {"B04_D05_F2_0x2D0", 4, 5, 2, 0x2D0, US_REG_DWORD},
    {"B04_D05_F2_0x2D4", 4, 5, 2, 0x2D4, US_REG_DWORD},
    {"B04_D05_F2_0x2D8", 4, 5, 2, 0x2D8, US_REG_DWORD},
    {"B04_D05_F2_0x2DC", 4, 5, 2, 0x2DC, US_REG_DWORD},
    {"B04_D05_F2_0x2E0", 4, 5, 2, 0x2E0, US_REG_DWORD},
    {"B04_D05_F2_0x2E4", 4, 5, 2, 0x2E4, US_REG_DWORD},
    {"B04_D05_F2_0x2E8", 4, 5, 2, 0x2E8, US_REG_DWORD},
    {"B04_D05_F2_0x2EC", 4, 5, 2, 0x2EC, US_REG_DWORD},
    {"B04_D05_F2_0x2FC", 4, 5, 2, 0x2FC, US_REG_DWORD},
    {"B04_D05_F2_0x304", 4, 5, 2, 0x304, US_REG_DWORD},
    {"B04_D05_F2_0x4", 4, 5, 2, 0x4, US_REG_WORD},
    {"B04_D05_F2_0x6", 4, 5, 2, 0x6, US_REG_WORD},
    {"B04_D05_F2_0x80", 4, 5, 2, 0x80, US_REG_DWORD},
    {"B04_D05_F2_0x94", 4, 5, 2, 0x94, US_REG_DWORD},
    {"B04_D05_F2_0xA0", 4, 5, 2, 0xA0, US_REG_DWORD},
    {"B04_D07_F0_0x0", 4, 7, 0, 0x0, US_REG_WORD},
    {"B04_D07_F0_0x2", 4, 7, 0, 0x2, US_REG_WORD},
    {"B04_D07_F0_0x4", 4, 7, 0, 0x4, US_REG_WORD},
    {"B04_D07_F0_0x4A", 4, 7, 0, 0x4A, US_REG_WORD},
    {"B04_D07_F0_0x52", 4, 7, 0, 0x52, US_REG_WORD},
    {"B04_D07_F0_0x6", 4, 7, 0, 0x6, US_REG_WORD},
    {"B04_D07_F0_0x72", 4, 7, 0, 0x72, US_REG_WORD},
    {"B04_D07_F0_0x7A", 4, 7, 0, 0x7A, US_REG_WORD},
    {"B04_D07_F1_0x0", 4, 7, 1, 0x0, US_REG_WORD},
    {"B04_D07_F1_0x2", 4, 7, 1, 0x2, US_REG_WORD},
    {"B04_D07_F1_0x4", 4, 7, 1, 0x4, US_REG_WORD},
    {"B04_D07_F1_0x4A", 4, 7, 1, 0x4A, US_REG_WORD},
    {"B04_D07_F1_0x52", 4, 7, 1, 0x52, US_REG_WORD},
    {"B04_D07_F1_0x6", 4, 7, 1, 0x6, US_REG_WORD},
    {"B04_D07_F1_0x72", 4, 7, 1, 0x72, US_REG_WORD},
    {"B04_D07_F1_0x7A", 4, 7, 1, 0x7A, US_REG_WORD},
    {"B04_D07_F2_0x0", 4, 7, 2, 0x0, US_REG_WORD},
    {"B04_D07_F2_0x2", 4, 7, 2, 0x2, US_REG_WORD},
    {"B04_D07_F2_0x4", 4, 7, 2, 0x4, US_REG_WORD},
    {"B04_D07_F2_0x4A", 4, 7, 2, 0x4A, US_REG_WORD},
    {"B04_D07_F2_0x52", 4, 7, 2, 0x52, US_REG_WORD},
    {"B04_D07_F2_0x6", 4, 7, 2, 0x6, US_REG_WORD},
    {"B04_D07_F2_0x72", 4, 7, 2, 0x72, US_REG_WORD},
    {"B04_D07_F2_0x7A", 4, 7, 2, 0x7A, US_REG_WORD},
    {"B04_D07_F3_0x0", 4, 7, 3, 0x0, US_REG_WORD},
    {"B04_D07_F3_0x2", 4, 7, 3, 0x2, US_REG_WORD},
    {"B04_D07_F3_0x4", 4, 7, 3, 0x4, US_REG_WORD},
    {"B04_D07_F3_0x4A", 4, 7, 3, 0x4A, US_REG_WORD},
    {"B04_D07_F3_0x52", 4, 7, 3, 0x52, US_REG_WORD},
    {"B04_D07_F3_0x6", 4, 7, 3, 0x6, US_REG_WORD},
    {"B04_D07_F3_0x72", 4, 7, 3, 0x72, US_REG_WORD},
    {"B04_D07_F3_0x7A", 4, 7, 3, 0x7A, US_REG_WORD},
    {"B04_D07_F4_0x0", 4, 7, 4, 0x0, US_REG_WORD},
    {"B04_D07_F4_0x2", 4, 7, 4, 0x2, US_REG_WORD},
    {"B04_D07_F4_0x4", 4, 7, 4, 0x4, US_REG_WORD},
    {"B04_D07_F4_0x4A", 4, 7, 4, 0x4A, US_REG_WORD},
    {"B04_D07_F4_0x52", 4, 7, 4, 0x52, US_REG_WORD},
    {"B04_D07_F4_0x6", 4, 7, 4, 0x6, US_REG_WORD},
    {"B04_D07_F4_0x72", 4, 7, 4, 0x72, US_REG_WORD},
    {"B04_D07_F4_0x7A", 4, 7, 4, 0x7A, US_REG_WORD},
    {"B04_D07_F7_0x0", 4, 7, 7, 0x0, US_REG_WORD},
    {"B04_D07_F7_0x2", 4, 7, 7, 0x2, US_REG_WORD},
    {"B04_D07_F7_0x358", 4, 7, 7, 0x358, US_REG_DWORD},
    {"B04_D07_F7_0x4", 4, 7, 7, 0x4, US_REG_WORD},
    {"B04_D07_F7_0x4A", 4, 7, 7, 0x4A, US_REG_WORD},
    {"B04_D07_F7_0x52", 4, 7, 7, 0x52, US_REG_WORD},
    {"B04_D07_F7_0x6", 4, 7, 7, 0x6, US_REG_WORD},
    {"B04_D07_F7_0x72", 4, 7, 7, 0x72, US_REG_WORD},
    {"B04_D07_F7_0x7A", 4, 7, 7, 0x7A, US_REG_WORD},
};

static const SUncoreStatusRegPciMmio sUncoreStatusPciMmio[] = {
    // Register, Bar, Bus, Dev, Func, Offset, SizeCode
    {"B00_D04_F0_0xA8",
     {{
         .lenCode = US_MMIO_DWORD,
         .bar = 0,
         .bus = 0,
         .rsvd = 0,
         .func = 0,
         .dev = 4,
         .reg = 0xA8,
     }}},
    {"B00_D04_F1_0xA8",
     {{
         .lenCode = US_MMIO_DWORD,
         .bar = 0,
         .bus = 0,
         .rsvd = 0,
         .func = 1,
         .dev = 4,
         .reg = 0xA8,
     }}},
    {"B00_D04_F2_0xA8",
     {{
         .lenCode = US_MMIO_DWORD,
         .bar = 0,
         .bus = 0,
         .rsvd = 0,
         .func = 2,
         .dev = 4,
         .reg = 0xA8,
     }}},
    {"B00_D04_F3_0xA8",
     {{
         .lenCode = US_MMIO_DWORD,
         .bar = 0,
         .bus = 0,
         .rsvd = 0,
         .func = 3,
         .dev = 4,
         .reg = 0xA8,
     }}},
    {"B00_D04_F4_0xA8",
     {{
         .lenCode = US_MMIO_DWORD,
         .bar = 0,
         .bus = 0,
         .rsvd = 0,
         .func = 4,
         .dev = 4,
         .reg = 0xA8,
     }}},
    {"B00_D04_F5_0xA8",
     {{
         .lenCode = US_MMIO_DWORD,
         .bar = 0,
         .bus = 0,
         .rsvd = 0,
         .func = 5,
         .dev = 4,
         .reg = 0xA8,
     }}},
    {"B00_D04_F6_0xA8",
     {{
         .lenCode = US_MMIO_DWORD,
         .bar = 0,
         .bus = 0,
         .rsvd = 0,
         .func = 6,
         .dev = 4,
         .reg = 0xA8,
     }}},
    {"B00_D04_F7_0xA8",
     {{
         .lenCode = US_MMIO_DWORD,
         .bar = 0,
         .bus = 0,
         .rsvd = 0,
         .func = 7,
         .dev = 4,
         .reg = 0xA8,
     }}},
    {"B00_D05_F0_0x101C",
     {{
         .lenCode = US_MMIO_DWORD,
         .bar = 0,
         .bus = 0,
         .rsvd = 0,
         .func = 0,
         .dev = 5,
         .reg = 0x101C,
     }}},
    {"B00_D05_F0_0x1034",
     {{
         .lenCode = US_MMIO_DWORD,
         .bar = 0,
         .bus = 0,
         .rsvd = 0,
         .func = 0,
         .dev = 5,
         .reg = 0x1034,
     }}},
    {"B00_D05_F0_0x1C",
     {{
         .lenCode = US_MMIO_DWORD,
         .bar = 0,
         .bus = 0,
         .rsvd = 0,
         .func = 0,
         .dev = 5,
         .reg = 0x1C,
     }}},
    {"B00_D05_F0_0x34",
     {{
         .lenCode = US_MMIO_DWORD,
         .bar = 0,
         .bus = 0,
         .rsvd = 0,
         .func = 0,
         .dev = 5,
         .reg = 0x34,
     }}},
    {"B01_D05_F0_0x101C",
     {{
         .lenCode = US_MMIO_DWORD,
         .bar = 0,
         .bus = 1,
         .rsvd = 0,
         .func = 0,
         .dev = 5,
         .reg = 0x101C,
     }}},
    {"B01_D05_F0_0x1034",
     {{
         .lenCode = US_MMIO_DWORD,
         .bar = 0,
         .bus = 1,
         .rsvd = 0,
         .func = 0,
         .dev = 5,
         .reg = 0x1034,
     }}},
    {"B01_D05_F0_0x1C",
     {{
         .lenCode = US_MMIO_DWORD,
         .bar = 0,
         .bus = 1,
         .rsvd = 0,
         .func = 0,
         .dev = 5,
         .reg = 0x1C,
     }}},
    {"B01_D05_F0_0x34",
     {{
         .lenCode = US_MMIO_DWORD,
         .bar = 0,
         .bus = 1,
         .rsvd = 0,
         .func = 0,
         .dev = 5,
         .reg = 0x34,
     }}},
    {"B02_D05_F0_0x101C",
     {{
         .lenCode = US_MMIO_DWORD,
         .bar = 0,
         .bus = 2,
         .rsvd = 0,
         .func = 0,
         .dev = 5,
         .reg = 0x101C,
     }}},
    {"B02_D05_F0_0x1034",
     {{
         .lenCode = US_MMIO_DWORD,
         .bar = 0,
         .bus = 2,
         .rsvd = 0,
         .func = 0,
         .dev = 5,
         .reg = 0x1034,
     }}},
    {"B02_D05_F0_0x1C",
     {{
         .lenCode = US_MMIO_DWORD,
         .bar = 0,
         .bus = 2,
         .rsvd = 0,
         .func = 0,
         .dev = 5,
         .reg = 0x1C,
     }}},
    {"B02_D05_F0_0x34",
     {{
         .lenCode = US_MMIO_DWORD,
         .bar = 0,
         .bus = 2,
         .rsvd = 0,
         .func = 0,
         .dev = 5,
         .reg = 0x34,
     }}},
    {"B03_D05_F0_0x101C",
     {{
         .lenCode = US_MMIO_DWORD,
         .bar = 0,
         .bus = 3,
         .rsvd = 0,
         .func = 0,
         .dev = 5,
         .reg = 0x101C,
     }}},
    {"B03_D05_F0_0x1034",
     {{
         .lenCode = US_MMIO_DWORD,
         .bar = 0,
         .bus = 3,
         .rsvd = 0,
         .func = 0,
         .dev = 5,
         .reg = 0x1034,
     }}},
    {"B03_D05_F0_0x1C",
     {{
         .lenCode = US_MMIO_DWORD,
         .bar = 0,
         .bus = 3,
         .rsvd = 0,
         .func = 0,
         .dev = 5,
         .reg = 0x1C,
     }}},
    {"B03_D05_F0_0x34",
     {{
         .lenCode = US_MMIO_DWORD,
         .bar = 0,
         .bus = 3,
         .rsvd = 0,
         .func = 0,
         .dev = 5,
         .reg = 0x34,
     }}},
    {"B04_D05_F0_0x101C",
     {{
         .lenCode = US_MMIO_DWORD,
         .bar = 0,
         .bus = 4,
         .rsvd = 0,
         .func = 0,
         .dev = 5,
         .reg = 0x101C,
     }}},
    {"B04_D05_F0_0x1034",
     {{
         .lenCode = US_MMIO_DWORD,
         .bar = 0,
         .bus = 4,
         .rsvd = 0,
         .func = 0,
         .dev = 5,
         .reg = 0x1034,
     }}},
    {"B04_D05_F0_0x1C",
     {{
         .lenCode = US_MMIO_DWORD,
         .bar = 0,
         .bus = 4,
         .rsvd = 0,
         .func = 0,
         .dev = 5,
         .reg = 0x1C,
     }}},
    {"B04_D05_F0_0x34",
     {{
         .lenCode = US_MMIO_DWORD,
         .bar = 0,
         .bus = 4,
         .rsvd = 0,
         .func = 0,
         .dev = 5,
         .reg = 0x34,
     }}},
};

static const SUncoreStatusRegIio sUncoreStatusIio[] = {
    {"iio_cstack_mc_%s", 0},  {"iio_pstack0_mc_%s", 1},
    {"iio_pstack1_mc_%s", 2}, {"iio_pstack2_mc_%s", 3},
    {"iio_pstack3_mc_%s", 4},
};

/******************************************************************************
 *
 *   uncoreStatusPciJson
 *
 *   This function formats the Uncore Status PCI registers into a JSON object
 *
 ******************************************************************************/
static void uncoreStatusPciJson(const char* regName,
                                SUncoreStatusRegRawData* sRegData,
                                cJSON* pJsonChild)
{
    char jsonItemString[US_JSON_STRING_LEN];

    // Format the Uncore Status register data out to the .json debug file
    if (sRegData->bInvalid)
    {
        cd_snprintf_s(jsonItemString, US_JSON_STRING_LEN, US_UA, sRegData->cc);
        cJSON_AddStringToObject(pJsonChild, regName, jsonItemString);
    }
    else
    {
        cd_snprintf_s(jsonItemString, US_JSON_STRING_LEN, "0x%llx",
                      sRegData->uValue.u64);
        cJSON_AddStringToObject(pJsonChild, regName, jsonItemString);
    }
}

/******************************************************************************
 *
 *   uncoreStatusPci
 *
 *   This function gathers the Uncore Status PCI registers
 *
 ******************************************************************************/
static int uncoreStatusPci(crashdump::CPUInfo& cpuInfo, cJSON* pJsonChild)
{
    uint8_t cc = 0;

    // Get the Uncore Status PCI Registers
    for (uint32_t i = 0;
         i < (sizeof(sUncoreStatusPci) / sizeof(SUncoreStatusRegPci)); i++)
    {
        SUncoreStatusRegRawData sRegData = {};
        switch (sUncoreStatusPci[i].u8Size)
        {
            case US_REG_BYTE:
            case US_REG_WORD:
            case US_REG_DWORD:
                if (peci_RdPCIConfigLocal(
                        cpuInfo.clientAddr, sUncoreStatusPci[i].u8Bus,
                        sUncoreStatusPci[i].u8Dev, sUncoreStatusPci[i].u8Func,
                        sUncoreStatusPci[i].u16Reg, sUncoreStatusPci[i].u8Size,
                        (uint8_t*)&sRegData.uValue.u64, &cc) != PECI_CC_SUCCESS)
                {
                    sRegData.bInvalid = true;
                    sRegData.cc = cc;
                }
                break;
            case US_REG_QWORD:
                for (uint8_t u8Dword = 0; u8Dword < 2; u8Dword++)
                {
                    if (peci_RdPCIConfigLocal(
                            cpuInfo.clientAddr, sUncoreStatusPci[i].u8Bus,
                            sUncoreStatusPci[i].u8Dev,
                            sUncoreStatusPci[i].u8Func,
                            sUncoreStatusPci[i].u16Reg + (u8Dword * 4),
                            sizeof(uint32_t),
                            (uint8_t*)&sRegData.uValue.u32[u8Dword],
                            &cc) != PECI_CC_SUCCESS)
                    {
                        sRegData.bInvalid = true;
                        sRegData.cc = cc;
                    }
                }
                break;
        }
        // Log this Uncore Status PCI Register
        uncoreStatusPciJson(sUncoreStatusPci[i].regName, &sRegData, pJsonChild);
    }
    return 0;
}

/******************************************************************************
 *
 *   uncoreStatusMmioRead
 *
 *   This function gathers the Uncore Status PCI MMIO registers
 *
 ******************************************************************************/
static int uncoreStatusMmioRead(
    crashdump::CPUInfo& cpuInfo, uint32_t u32Param, uint8_t u8NumDwords,
    SUncoreStatusRegRawData* sUncoreStatusMmioRawData, int peci_fd)
{
    uint8_t cc = 0;

    // Open the MMIO dump sequence
    if (peci_WrPkgConfig_seq(cpuInfo.clientAddr, MBX_INDEX_VCU, VCU_OPEN_SEQ,
                             VCU_READ_LOCAL_MMIO_SEQ, sizeof(uint32_t), peci_fd,
                             &cc) != PECI_CC_SUCCESS)
    {
        // MMIO sequence failed, abort the sequence
        sUncoreStatusMmioRawData->cc = cc;
        peci_WrPkgConfig_seq(cpuInfo.clientAddr, MBX_INDEX_VCU, VCU_ABORT_SEQ,
                             VCU_READ_LOCAL_MMIO_SEQ, sizeof(uint32_t), peci_fd,
                             &cc);
        return -1;
    }

    // Set MMIO address
    if (peci_WrPkgConfig_seq(cpuInfo.clientAddr, MBX_INDEX_VCU, US_MMIO_PARAM,
                             u32Param, sizeof(uint32_t), peci_fd,
                             &cc) != PECI_CC_SUCCESS)
    {
        // MMIO sequence failed, abort the sequence
        sUncoreStatusMmioRawData->cc = cc;
        peci_WrPkgConfig_seq(cpuInfo.clientAddr, MBX_INDEX_VCU, VCU_ABORT_SEQ,
                             VCU_READ_LOCAL_MMIO_SEQ, sizeof(uint32_t), peci_fd,
                             &cc);
        return -1;
    }

    // Get the MMIO data
    for (uint8_t u8Dword = 0; u8Dword < u8NumDwords; u8Dword++)
    {
        if (peci_RdPkgConfig_seq(
                cpuInfo.clientAddr, MBX_INDEX_VCU, VCU_READ, sizeof(uint32_t),
                (uint8_t*)&sUncoreStatusMmioRawData->uValue.u32[u8Dword],
                peci_fd, &cc) != PECI_CC_SUCCESS)
        {
            // MMIO sequence failed, abort the sequence
            sUncoreStatusMmioRawData->cc = cc;
            peci_WrPkgConfig_seq(cpuInfo.clientAddr, MBX_INDEX_VCU,
                                 VCU_ABORT_SEQ, VCU_READ_LOCAL_MMIO_SEQ,
                                 sizeof(uint32_t), peci_fd, &cc);
            sUncoreStatusMmioRawData->bInvalid = true;
            return -1;
        }
    }

    // Close the MMIO sequence
    peci_WrPkgConfig_seq(cpuInfo.clientAddr, MBX_INDEX_VCU, VCU_CLOSE_SEQ,
                         VCU_READ_LOCAL_MMIO_SEQ, sizeof(uint32_t), peci_fd,
                         &cc);

    return 0;
}

/******************************************************************************
 *
 *   uncoreStatusPciMmioCPX1
 *
 *   This function gathers the Uncore Status PCI MMIO registers
 *
 ******************************************************************************/
static int uncoreStatusPciMmioCPX1(crashdump::CPUInfo& cpuInfo,
                                   cJSON* pJsonChild)
{
    int peci_fd = -1;

    if (peci_Lock(&peci_fd, PECI_WAIT_FOREVER) != PECI_CC_SUCCESS)
    {
        return -1;
    }

    // Get the Uncore Status PCI MMIO Registers
    for (uint32_t i = 0;
         i < (sizeof(sUncoreStatusPciMmio) / sizeof(SUncoreStatusRegPciMmio));
         i++)
    {
        SUncoreStatusRegRawData sRegData = {};
        uint32_t u32MmioParam = sUncoreStatusPciMmio[i].uMmioReg.raw;
        uint8_t u8NumDwords =
            sUncoreStatusPciMmio[i].uMmioReg.fields.lenCode == US_MMIO_QWORD
                ? 2
                : 1;

        // Get the MMIO data
        if (uncoreStatusMmioRead(cpuInfo, u32MmioParam, u8NumDwords, &sRegData,
                                 peci_fd) == 0)
        {
        }

        // Log this Uncore Status PCI Register
        uncoreStatusPciJson(sUncoreStatusPciMmio[i].regName, &sRegData,
                            pJsonChild);
    }
    peci_Unlock(peci_fd);
    return 0;
}

/******************************************************************************
 *
 *   uncoreStatusMcaRead
 *
 *   This function gathers the Uncore Status MCA registers
 *
 ******************************************************************************/
static int uncoreStatusMcaRead(crashdump::CPUInfo& cpuInfo, uint32_t u32Param,
                               SUncoreStatusMcaRawData* sUncoreStatusMcaRawData,
                               int peci_fd)
{
    uint8_t cc = 0;

    // Open the MCA Bank dump sequence
    if (peci_WrPkgConfig_seq(cpuInfo.clientAddr, MBX_INDEX_VCU, VCU_OPEN_SEQ,
                             VCU_UNCORE_MCA_SEQ, sizeof(uint32_t), peci_fd,
                             &cc) != PECI_CC_SUCCESS)
    {
        // MCA Bank sequence failed, abort the sequence
        sUncoreStatusMcaRawData->cc = cc;
        peci_WrPkgConfig_seq(cpuInfo.clientAddr, MBX_INDEX_VCU, VCU_ABORT_SEQ,
                             VCU_UNCORE_MCA_SEQ, sizeof(uint32_t), peci_fd,
                             &cc);

        return -1;
    }

    // Set MCA Bank number
    if (peci_WrPkgConfig_seq(cpuInfo.clientAddr, MBX_INDEX_VCU, US_MCA_PARAM,
                             u32Param, sizeof(uint32_t), peci_fd,
                             &cc) != PECI_CC_SUCCESS)
    {
        // MCA Bank sequence failed, abort the sequence
        sUncoreStatusMcaRawData->cc = cc;
        peci_WrPkgConfig_seq(cpuInfo.clientAddr, MBX_INDEX_VCU, VCU_ABORT_SEQ,
                             VCU_UNCORE_MCA_SEQ, sizeof(uint32_t), peci_fd,
                             &cc);

        return -1;
    }

    // Get the MCA Bank Registers
    for (uint8_t u8Dword = 0; u8Dword < US_NUM_MCA_DWORDS; u8Dword++)
    {
        if (peci_RdPkgConfig_seq(
                cpuInfo.clientAddr, MBX_INDEX_VCU, VCU_READ, sizeof(uint32_t),
                (uint8_t*)&sUncoreStatusMcaRawData->uRegData.u32Raw[u8Dword],
                peci_fd, &cc) != PECI_CC_SUCCESS)
        {
            // MCA Bank sequence failed, abort the sequence
            sUncoreStatusMcaRawData->cc = cc;
            peci_WrPkgConfig_seq(cpuInfo.clientAddr, MBX_INDEX_VCU,
                                 VCU_ABORT_SEQ, VCU_UNCORE_MCA_SEQ,
                                 sizeof(uint32_t), peci_fd, &cc);
            return -1;
        }
    }

    // Close the MCA Bank sequence
    peci_WrPkgConfig_seq(cpuInfo.clientAddr, MBX_INDEX_VCU, VCU_CLOSE_SEQ,
                         VCU_UNCORE_MCA_SEQ, sizeof(uint32_t), peci_fd, &cc);

    return 0;
}

/******************************************************************************
 *
 *   uncoreStatusIioJson
 *
 *   This function formats the Uncore Status IIO MCA registers into a JSON
 *   object
 *
 ******************************************************************************/
static void uncoreStatusIioJson(const char* regName,
                                SUncoreStatusMcaRawData* sMcaData,
                                cJSON* pJsonChild)
{
    char jsonItemString[US_JSON_STRING_LEN];
    char jsonNameString[US_JSON_STRING_LEN];
    uint32_t i;

    // Format the Uncore Status IIO MCA data out to the .json debug file
    // Fill in NULL for this IIO MCA if it's not valid
    if (sMcaData->bInvalid)
    {
        for (i = 0; i < US_NUM_MCA_QWORDS; i++)
        {
            cd_snprintf_s(jsonNameString, US_JSON_STRING_LEN, regName,
                          uncoreStatusMcaRegNames[i]);
            cd_snprintf_s(jsonItemString, US_JSON_STRING_LEN, US_UA,
                          sMcaData->cc);
            cJSON_AddStringToObject(pJsonChild, jsonNameString, jsonItemString);
        }
        // Otherwise fill in the register data
    }
    else
    {
        for (i = 0; i < US_NUM_MCA_QWORDS; i++)
        {
            cd_snprintf_s(jsonNameString, US_JSON_STRING_LEN, regName,
                          uncoreStatusMcaRegNames[i]);
            cd_snprintf_s(jsonItemString, US_JSON_STRING_LEN, "0x%llx",
                          sMcaData->uRegData.u64Raw[i]);
            cJSON_AddStringToObject(pJsonChild, jsonNameString, jsonItemString);
        }
    }
}

/******************************************************************************
 *
 *   uncoreStatusIio
 *
 *   This function gathers the Uncore Status IIO MCA registers
 *
 ******************************************************************************/
static int uncoreStatusIio(crashdump::CPUInfo& cpuInfo, cJSON* pJsonChild)
{
    int peci_fd = -1;
    if (peci_Lock(&peci_fd, PECI_WAIT_FOREVER) != PECI_CC_SUCCESS)
    {
        return -1;
    }

    // Go through each IIO in this CPU
    for (uint32_t i = 0;
         i < (sizeof(sUncoreStatusIio) / sizeof(SUncoreStatusRegIio)); i++)
    {
        SUncoreStatusMcaRawData sMcaData = {};
        // Build the MCA parameter for this IIO
        uint32_t u32IioParam = (sUncoreStatusIio[i].u8IioNum << 24 |
                                US_MCA_UNMERGE | US_BASE_IIO_BANK);

        // Get the IIO MCA data
        ;
        if (uncoreStatusMcaRead(cpuInfo, u32IioParam, &sMcaData, peci_fd) !=
            PECI_CC_SUCCESS)
        {
            sMcaData.bInvalid = true;
        }

        // Log the MCA for this IIO
        uncoreStatusIioJson(sUncoreStatusIio[i].regName, &sMcaData, pJsonChild);
    }

    peci_Unlock(peci_fd);
    return 0;
}

/******************************************************************************
 *
 *   uncoreStatusCrashdumpJson
 *
 *   This function formats the Uncore Status Crashdump into a JSON object
 *
 ******************************************************************************/
static void uncoreStatusCrashdumpJson(uint32_t u32NumReads,
                                      uint32_t* pu32UncoreCrashdump,
                                      cJSON* pJsonChild)
{
    char jsonItemString[US_JSON_STRING_LEN];
    char jsonNameString[US_JSON_STRING_LEN];

    // Add the Uncore Crashdump dump info to the Uncore Status dump JSON
    // structure
    for (uint32_t i = 0; i < u32NumReads; i++)
    {
        cd_snprintf_s(jsonNameString, US_JSON_STRING_LEN,
                      US_UNCORE_CRASH_DW_NAME, i);
        cd_snprintf_s(jsonItemString, US_JSON_STRING_LEN, "0x%lx",
                      pu32UncoreCrashdump[i]);
        cJSON_AddStringToObject(pJsonChild, jsonNameString, jsonItemString);
    }
}

/******************************************************************************
 *
 *   uncoreStatusCrashdump
 *
 *   This function gathers the Uncore Status Crashdump
 *
 ******************************************************************************/
static int uncoreStatusCrashdump(crashdump::CPUInfo& cpuInfo, cJSON* pJsonChild)
{
    int peci_fd = -1;
    uint8_t cc = 0;

    if (peci_Lock(&peci_fd, PECI_WAIT_FOREVER) != PECI_CC_SUCCESS)
    {
        return -1;
    }

    // Start the Uncore Crashdump dump log

    // Open the Uncore Crashdump dump sequence
    if (peci_WrPkgConfig_seq(cpuInfo.clientAddr, MBX_INDEX_VCU, VCU_OPEN_SEQ,
                             VCU_UNCORE_CRASHDUMP_SEQ, sizeof(uint32_t),
                             peci_fd, &cc) != PECI_CC_SUCCESS)
    {
        // Uncore Crashdump dump sequence failed, abort the sequence
        peci_WrPkgConfig_seq(cpuInfo.clientAddr, MBX_INDEX_VCU, VCU_ABORT_SEQ,
                             VCU_UNCORE_CRASHDUMP_SEQ, sizeof(uint32_t),
                             peci_fd, &cc);
        return 1;
    }

    // Set Uncore Crashdump dump parameter
    if (peci_WrPkgConfig_seq(cpuInfo.clientAddr, MBX_INDEX_VCU, VCU_SET_PARAM,
                             US_UCRASH_PARAM, sizeof(uint32_t), peci_fd,
                             &cc) != PECI_CC_SUCCESS)
    {
        // Uncore Crashdump dump sequence failed, abort the sequence
        peci_WrPkgConfig_seq(cpuInfo.clientAddr, MBX_INDEX_VCU, VCU_ABORT_SEQ,
                             VCU_UNCORE_CRASHDUMP_SEQ, sizeof(uint32_t),
                             peci_fd, &cc);
        return 1;
    }

    // Get the number of dwords to read
    uint32_t u32NumReads = 0;
    if (peci_RdPkgConfig_seq(cpuInfo.clientAddr, MBX_INDEX_VCU, US_UCRASH_START,
                             sizeof(uint32_t), (uint8_t*)&u32NumReads, peci_fd,
                             &cc) != PECI_CC_SUCCESS)
    {
        // Uncore Crashdump dump sequence failed, abort the sequence
        peci_WrPkgConfig_seq(cpuInfo.clientAddr, MBX_INDEX_VCU, VCU_ABORT_SEQ,
                             VCU_UNCORE_CRASHDUMP_SEQ, sizeof(uint32_t),
                             peci_fd, &cc);
        return 1;
    }

    // Get the API version number
    uint32_t u32ApiVersion = 0;
    if (peci_RdPkgConfig_seq(cpuInfo.clientAddr, MBX_INDEX_VCU, VCU_READ,
                             sizeof(uint32_t), (uint8_t*)&u32ApiVersion,
                             peci_fd, &cc) != PECI_CC_SUCCESS)
    {
        // Uncore Crashdump dump sequence failed, abort the sequence
        peci_WrPkgConfig_seq(cpuInfo.clientAddr, MBX_INDEX_VCU, VCU_ABORT_SEQ,
                             VCU_UNCORE_CRASHDUMP_SEQ, sizeof(uint32_t),
                             peci_fd, &cc);
        return 1;
    }
    // API version is included in the number of reads, so decrement by one
    u32NumReads--;

    // Get the raw data
    uint32_t* pu32UncoreCrashdump =
        (uint32_t*)calloc(u32NumReads, sizeof(uint32_t));
    if (pu32UncoreCrashdump == NULL)
    {
        // calloc failed, abort the sequence
        peci_WrPkgConfig_seq(cpuInfo.clientAddr, MBX_INDEX_VCU, VCU_ABORT_SEQ,
                             VCU_UNCORE_CRASHDUMP_SEQ, sizeof(uint32_t),
                             peci_fd, &cc);
        return 1;
    }
    for (uint32_t i = 0; i < u32NumReads; i++)
    {
        if (peci_RdPkgConfig_seq(cpuInfo.clientAddr, MBX_INDEX_VCU, VCU_READ,
                                 sizeof(uint32_t),
                                 (uint8_t*)&pu32UncoreCrashdump[i], peci_fd,
                                 &cc) != PECI_CC_SUCCESS)
        {
            // Uncore Crashdump dump sequence failed, note the number of dwords
            // read and abort the sequence
            u32NumReads = i;
            peci_WrPkgConfig_seq(cpuInfo.clientAddr, MBX_INDEX_VCU,
                                 VCU_ABORT_SEQ, VCU_UNCORE_CRASHDUMP_SEQ,
                                 sizeof(uint32_t), peci_fd, &cc);
            break;
        }
    }

    // Close the Uncore Crashdump dump sequence
    peci_WrPkgConfig_seq(cpuInfo.clientAddr, MBX_INDEX_VCU, VCU_CLOSE_SEQ,
                         VCU_UNCORE_CRASHDUMP_SEQ, sizeof(uint32_t), peci_fd,
                         &cc);

    // Log the Uncore Crashdump
    uncoreStatusCrashdumpJson(u32NumReads, pu32UncoreCrashdump, pJsonChild);

    free(pu32UncoreCrashdump);

    peci_Unlock(peci_fd);
    return 0;
}

static UncoreStatusRead UncoreStatusTypesCPX1[] = {
    uncoreStatusCrashdump,
    uncoreStatusPci,
    uncoreStatusPciMmioCPX1,
    uncoreStatusIio,
};

/******************************************************************************
 *
 *   logUncoreStatusCPX1
 *
 *   This function gathers the Uncore Status register contents and adds them to
 *   the debug log.
 *
 ******************************************************************************/
int logUncoreStatusCPX1(crashdump::CPUInfo& cpuInfo, cJSON* pJsonChild)
{
    int ret = 0;

    for (uint32_t i = 0;
         i < (sizeof(UncoreStatusTypesCPX1) / sizeof(UncoreStatusTypesCPX1[0]));
         i++)
    {
        if (UncoreStatusTypesCPX1[i](cpuInfo, pJsonChild) != 0)
        {
            ret = 1;
        }
    }

    return ret;
}

/******************************************************************************
 *
 *   uncoreStatusJsonICX
 *
 *   This function formats the Uncore Status PCI registers into a JSON object
 *
 ******************************************************************************/
static void uncoreStatusJsonICX(const char* regName,
                                SUncoreStatusRegRawData* sRegData,
                                cJSON* pJsonChild, uint8_t cc)
{
    char jsonItemString[US_JSON_STRING_LEN];

    if (sRegData->bInvalid)
    {
        cd_snprintf_s(jsonItemString, US_JSON_STRING_LEN, US_NA);
        cJSON_AddStringToObject(pJsonChild, regName, jsonItemString);
        return;
    }

    if (PECI_CC_UA(cc))
    {
        cd_snprintf_s(jsonItemString, US_JSON_STRING_LEN, US_UA, cc);
    }
    else
    {
        cd_snprintf_s(jsonItemString, US_JSON_STRING_LEN, US_UINT64_FMT,
                      sRegData->uValue.u64, cc);
    }

    cJSON_AddStringToObject(pJsonChild, regName, jsonItemString);
}

/******************************************************************************
 *
 *   uncoreStatusPciICX
 *
 *   This function gathers the ICX Uncore Status PCI registers
 *
 ******************************************************************************/
static int uncoreStatusPciICX1(crashdump::CPUInfo& cpuInfo, cJSON* pJsonChild)
{
    int peci_fd = -1;
    int ret = 0;

    if (peci_Lock(&peci_fd, PECI_WAIT_FOREVER) != PECI_CC_SUCCESS)
    {
        return 1;
    }

    for (uint32_t i = 0;
         i < (sizeof(sUncoreStatusPciICX1) / sizeof(SUncoreStatusRegPci)); i++)
    {
        SUncoreStatusRegRawData sRegData = {};
        uint8_t cc = 0;
        uint8_t bus = 0;

        // ICX EDS Reference Section: PCI Configuration Space Registers
        // Note that registers located in Bus 30 and 31
        // have been translated to Bus 13 and 14 respectively for PECI access.
        if (sUncoreStatusPciICX1[i].u8Bus == 30)
        {
            bus = 13;
        }
        else if (sUncoreStatusPciICX1[i].u8Bus == 31)
        {
            bus = 14;
        }
        else
        {
            bus = sUncoreStatusPciICX1[i].u8Bus;
        }

        switch (sUncoreStatusPciICX1[i].u8Size)
        {
            case US_REG_BYTE:
            case US_REG_WORD:
            case US_REG_DWORD:
                if (peci_RdEndPointConfigPciLocal_seq(
                        cpuInfo.clientAddr, US_PCI_SEG, bus,
                        sUncoreStatusPciICX1[i].u8Dev,
                        sUncoreStatusPciICX1[i].u8Func,
                        sUncoreStatusPciICX1[i].u16Reg,
                        sUncoreStatusPciICX1[i].u8Size,
                        (uint8_t*)&sRegData.uValue.u64, peci_fd,
                        &cc) != PECI_CC_SUCCESS)
                {
                    sRegData.bInvalid = true;
                    ret = 1;
                }
                break;
            case US_REG_QWORD:
                for (uint8_t u8Dword = 0; u8Dword < 2; u8Dword++)
                {
                    if (peci_RdEndPointConfigPciLocal_seq(
                            cpuInfo.clientAddr, US_PCI_SEG, bus,
                            sUncoreStatusPciICX1[i].u8Dev,
                            sUncoreStatusPciICX1[i].u8Func,
                            sUncoreStatusPciICX1[i].u16Reg + (u8Dword * 4),
                            sizeof(uint32_t),
                            (uint8_t*)&sRegData.uValue.u32[u8Dword], peci_fd,
                            &cc) != PECI_CC_SUCCESS)
                    {
                        sRegData.bInvalid = true;
                        ret = 1;
                        break;
                    }
                }
                break;
            default:
                sRegData.bInvalid = true;
                ret = 1;
        }

        uncoreStatusJsonICX(sUncoreStatusPciICX1[i].regName, &sRegData,
                            pJsonChild, cc);
    }
    peci_Unlock(peci_fd);
    return ret;
}

/******************************************************************************
 *
 *   bus30ToPostEnumeratedBus()
 *
 *   This function is dedicated to converting bus 30 to post enumerated
 *   bus number for MMIO read.
 *
 ******************************************************************************/
static int bus30ToPostEnumeratedBus(uint32_t addr, uint8_t* postEnumBus)
{
    uint32_t cpubusno_valid = 0;
    uint32_t cpubusno2 = 0;
    uint8_t cc = 0;

    // Use PCS Service 76, Parameter 5 to check valid post enumerated bus#
    if ((peci_RdPkgConfig(addr, 76, 5, sizeof(uint32_t),
                          (uint8_t*)&cpubusno_valid, &cc) != PECI_CC_SUCCESS) ||
        (PECI_CC_UA(cc)))
    {
        fprintf(stderr, "Unable to read cpubusno_valid - cc: 0x%x\n", cc);
        return 1;
    }

    // Bit 11 is for checking bus 30 contains valid post enumerated bus#
    if (0 == CHECK_BIT(cpubusno_valid, 11))
    {
        fprintf(stderr,
                "Bus 30 does not contain valid post enumerated bus"
                "number! (0x%x)\n",
                cpubusno_valid);
        return 1;
    }

    // Use PCS Service 76, Parameter 4 to get raw post enumerated buses value
    if ((peci_RdPkgConfig(addr, 76, 4, sizeof(uint32_t), (uint8_t*)&cpubusno2,
                          &cc) != PECI_CC_SUCCESS) ||
        (PECI_CC_UA(cc)))
    {
        fprintf(stderr, "Unable to read cpubusno2 - cc: 0x%x\n", cc);
        return 1;
    }

    // CPUBUSNO2[23:16] for Bus 30
    *postEnumBus = ((cpubusno2 >> 16) & 0xff);

    return 0;
}

/******************************************************************************
 *
 *   uncoreStatusPciMmioICX1
 *
 *   This function gathers the Uncore Status PCI MMIO registers
 *
 ******************************************************************************/
static int uncoreStatusPciMmioICX1(crashdump::CPUInfo& cpuInfo,
                                   cJSON* pJsonChild)
{
    char jsonNameString[US_REG_NAME_LEN];
    int peci_fd = -1;
    int ret = 0;
    uint8_t cc = 0;
    uint8_t postEnumBus = 0;

    if (0 != bus30ToPostEnumeratedBus(cpuInfo.clientAddr, &postEnumBus))
    {
        return 1;
    }

    if (peci_Lock(&peci_fd, PECI_WAIT_FOREVER) != PECI_CC_SUCCESS)
    {
        return 1;
    }

    for (uint32_t i = 0; i < (sizeof(sUncoreStatusPciMmioICX1) /
                              sizeof(SUncoreStatusRegPciMmioICX1));
         i++)
    {
        SUncoreStatusRegRawData sRegData = {};
        uint8_t addrType = PECI_ENDPTCFG_ADDR_TYPE_MMIO_D;
        uint8_t readLen = 0;

        if (sUncoreStatusPciMmioICX1[i].u64Offset > UINT32_MAX)
        {
            addrType = PECI_ENDPTCFG_ADDR_TYPE_MMIO_Q;
        }
        switch (sUncoreStatusPciMmioICX1[i].u8Size)
        {
            case US_REG_BYTE:
            case US_REG_WORD:
            case US_REG_DWORD:
                readLen = US_REG_DWORD;
                break;
            case US_REG_QWORD:
                readLen = US_REG_QWORD;
                break;
            default:
                sRegData.bInvalid = true;
                ret = 1;
        }

        if (peci_RdEndPointConfigMmio_seq(
                cpuInfo.clientAddr, US_MMIO_SEG, postEnumBus,
                sUncoreStatusPciMmioICX1[i].u8Dev,
                sUncoreStatusPciMmioICX1[i].u8Func,
                sUncoreStatusPciMmioICX1[i].u8Bar, addrType,
                sUncoreStatusPciMmioICX1[i].u64Offset, readLen,
                (uint8_t*)&sRegData.uValue.u64, peci_fd,
                &cc) != PECI_CC_SUCCESS)
        {
            sRegData.bInvalid = true;
            ret = 1;
        }

        cd_snprintf_s(jsonNameString, US_JSON_STRING_LEN,
                      sUncoreStatusPciMmioICX1[i].regName, postEnumBus);
        uncoreStatusJsonICX(jsonNameString, &sRegData, pJsonChild, cc);
    }
    peci_Unlock(peci_fd);
    return ret;
}

static UncoreStatusRead UncoreStatusTypesICX1[] = {
    uncoreStatusPciICX1,
    uncoreStatusPciMmioICX1,
};

/******************************************************************************
 *
 *   logUncoreStatusICX1
 *
 *   This function gathers the Uncore Status register contents and adds them to
 *   the debug log.
 *
 ******************************************************************************/
int logUncoreStatusICX1(crashdump::CPUInfo& cpuInfo, cJSON* pJsonChild)
{
    int ret = 0;

    for (uint32_t i = 0;
         i < (sizeof(UncoreStatusTypesICX1) / sizeof(UncoreStatusTypesICX1[0]));
         i++)
    {
        if (UncoreStatusTypesICX1[i](cpuInfo, pJsonChild) != 0)
        {
            ret = 1;
        }
    }

    return ret;
}

static const SUncoreStatusLogVx sUncoreStatusLogVx[] = {
    {clx, logUncoreStatusCPX1}, {clx2, logUncoreStatusCPX1},
    {cpx, logUncoreStatusCPX1}, {skx, logUncoreStatusCPX1},
    {icx, logUncoreStatusICX1},
};

/******************************************************************************
 *
 *   logUncoreStatus
 *
 *   This function gathers the Uncore Status register contents and adds them to
 *   the debug log.
 *
 ******************************************************************************/
int logUncoreStatus(crashdump::CPUInfo& cpuInfo, cJSON* pJsonChild)
{
    if (pJsonChild == NULL)
    {
        return 1;
    }
    revision::revision_uncore = SpreadSheetDataVersion;
    for (uint32_t i = 0;
         i < (sizeof(sUncoreStatusLogVx) / sizeof(SUncoreStatusLogVx)); i++)
    {
        if (cpuInfo.model == sUncoreStatusLogVx[i].cpuModel)
        {
            logCrashdumpVersion(pJsonChild, cpuInfo,
                                record_type::uncoreStatusLog);
            return sUncoreStatusLogVx[i].logUncoreStatusVx(cpuInfo, pJsonChild);
        }
    }

    fprintf(stderr, "Cannot find version for %s\n", __FUNCTION__);
    return 1;
}
