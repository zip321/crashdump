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

static const SUncoreStatusRegPci sUncoreStatusPci[] = {
    // Register, Bus, Dev, Func, Offset, Size
    {"imc0_c0_correction_debug_corr_data", 2, 10, 3, 0x031c, US_REG_DWORD},
    {"imc0_c0_correction_debug_corr_data_cntl", 2, 10, 3, 0x0318, US_REG_DWORD},
    {"imc0_c0_correction_debug_dev_vec_1", 2, 10, 3, 0x0310, US_REG_DWORD},
    {"imc0_c0_correction_debug_dev_vec_2", 2, 10, 3, 0x0314, US_REG_DWORD},
    {"imc0_c0_correction_debug_log", 2, 10, 3, 0x0130, US_REG_DWORD},
    {"imc0_c0_correction_debug_plus1_log", 2, 10, 3, 0x013c, US_REG_DWORD},
    {"imc0_c0_correrrcnt_0", 2, 10, 3, 0x0104, US_REG_DWORD},
    {"imc0_c0_correrrcnt_1", 2, 10, 3, 0x0108, US_REG_DWORD},
    {"imc0_c0_correrrcnt_2", 2, 10, 3, 0x010c, US_REG_DWORD},
    {"imc0_c0_correrrcnt_3", 2, 10, 3, 0x0110, US_REG_DWORD},
    {"imc0_c0_correrrorstatus", 2, 10, 3, 0x0134, US_REG_DWORD},
    {"imc0_c0_correrrthrshld_0", 2, 10, 3, 0x011c, US_REG_DWORD},
    {"imc0_c0_correrrthrshld_1", 2, 10, 3, 0x0120, US_REG_DWORD},
    {"imc0_c0_correrrthrshld_2", 2, 10, 3, 0x0124, US_REG_DWORD},
    {"imc0_c0_correrrthrshld_3", 2, 10, 3, 0x0128, US_REG_DWORD},
    {"imc0_c0_ddrt_err_log_1st", 2, 10, 1, 0x0d80, US_REG_DWORD},
    {"imc0_c0_ddrt_err_log_next", 2, 10, 1, 0x0d84, US_REG_DWORD},
    {"imc0_c0_ddrt_error", 2, 10, 1, 0x0d24, US_REG_DWORD},
    {"imc0_c0_ddrt_fnv0_event0", 2, 10, 1, 0x0a60, US_REG_DWORD},
    {"imc0_c0_ddrt_fnv0_event1", 2, 10, 1, 0x0a64, US_REG_DWORD},
    {"imc0_c0_ddrt_fnv1_event0", 2, 10, 1, 0x0a70, US_REG_DWORD},
    {"imc0_c0_ddrt_fnv1_event1", 2, 10, 1, 0x0a74, US_REG_DWORD},
    {"imc0_c0_ddrt_retry_fsm_state", 2, 10, 1, 0x0904, US_REG_DWORD},
    {"imc0_c0_ddrt_retry_status", 2, 10, 1, 0x0a98, US_REG_DWORD},
    {"imc0_c0_detection_debug_log", 2, 10, 3, 0x014c, US_REG_DWORD},
    {"imc0_c0_devtag_cntl", 2, 10, 3, 0x0140, US_REG_DWORD},
    {"imc0_c0_link_err_fsm_state", 2, 10, 3, 0x0420, US_REG_DWORD},
    {"imc0_c0_link_error", 2, 10, 3, 0x0308, US_REG_DWORD},
    {"imc0_c0_link_retry_err_limits", 2, 10, 3, 0x040c, US_REG_DWORD},
    {"imc0_c0_link_retry_sb_err_count", 2, 10, 3, 0x0400, US_REG_DWORD},
    {"imc0_c0_retry_rd_err_log", 2, 10, 3, 0x0154, US_REG_DWORD},
    {"imc0_c0_retry_rd_err_log_address1", 2, 10, 3, 0x015c, US_REG_DWORD},
    {"imc0_c0_retry_rd_err_log_address2", 2, 10, 3, 0x0114, US_REG_DWORD},
    {"imc0_c0_retry_rd_err_log_misc", 2, 10, 3, 0x0148, US_REG_DWORD},
    {"imc0_c0_retry_rd_err_log_parity", 2, 10, 3, 0x0150, US_REG_DWORD},
    {"imc0_c1_correction_debug_corr_data", 2, 10, 7, 0x031c, US_REG_DWORD},
    {"imc0_c1_correction_debug_corr_data_cntl", 2, 10, 7, 0x0318, US_REG_DWORD},
    {"imc0_c1_correction_debug_dev_vec_1", 2, 10, 7, 0x0310, US_REG_DWORD},
    {"imc0_c1_correction_debug_dev_vec_2", 2, 10, 7, 0x0314, US_REG_DWORD},
    {"imc0_c1_correction_debug_log", 2, 10, 7, 0x0130, US_REG_DWORD},
    {"imc0_c1_correction_debug_plus1_log", 2, 10, 7, 0x013c, US_REG_DWORD},
    {"imc0_c1_correrrcnt_0", 2, 10, 7, 0x0104, US_REG_DWORD},
    {"imc0_c1_correrrcnt_1", 2, 10, 7, 0x0108, US_REG_DWORD},
    {"imc0_c1_correrrcnt_2", 2, 10, 7, 0x010c, US_REG_DWORD},
    {"imc0_c1_correrrcnt_3", 2, 10, 7, 0x0110, US_REG_DWORD},
    {"imc0_c1_correrrorstatus", 2, 10, 7, 0x0134, US_REG_DWORD},
    {"imc0_c1_correrrthrshld_0", 2, 10, 7, 0x011c, US_REG_DWORD},
    {"imc0_c1_correrrthrshld_1", 2, 10, 7, 0x0120, US_REG_DWORD},
    {"imc0_c1_correrrthrshld_2", 2, 10, 7, 0x0124, US_REG_DWORD},
    {"imc0_c1_correrrthrshld_3", 2, 10, 7, 0x0128, US_REG_DWORD},
    {"imc0_c1_ddrt_err_log_1st", 2, 10, 7, 0x0d80, US_REG_DWORD},
    {"imc0_c1_ddrt_err_log_next", 2, 10, 7, 0x0d84, US_REG_DWORD},
    {"imc0_c1_ddrt_error", 2, 10, 7, 0x0d24, US_REG_DWORD},
    {"imc0_c1_ddrt_fnv0_event0", 2, 10, 7, 0x0a60, US_REG_DWORD},
    {"imc0_c1_ddrt_fnv0_event1", 2, 10, 7, 0x0a64, US_REG_DWORD},
    {"imc0_c1_ddrt_fnv1_event0", 2, 10, 7, 0x0a70, US_REG_DWORD},
    {"imc0_c1_ddrt_fnv1_event1", 2, 10, 7, 0x0a74, US_REG_DWORD},
    {"imc0_c1_ddrt_retry_fsm_state", 2, 10, 7, 0x0904, US_REG_DWORD},
    {"imc0_c1_ddrt_retry_status", 2, 10, 7, 0x0a98, US_REG_DWORD},
    {"imc0_c1_detection_debug_log", 2, 10, 7, 0x014c, US_REG_DWORD},
    {"imc0_c1_devtag_cntl", 2, 10, 7, 0x0140, US_REG_DWORD},
    {"imc0_c1_link_err_fsm_state", 2, 10, 7, 0x0420, US_REG_DWORD},
    {"imc0_c1_link_error", 2, 10, 7, 0x0308, US_REG_DWORD},
    {"imc0_c1_link_retry_err_limits", 2, 10, 7, 0x040c, US_REG_DWORD},
    {"imc0_c1_link_retry_sb_err_count", 2, 10, 7, 0x0400, US_REG_DWORD},
    {"imc0_c1_retry_rd_err_log", 2, 10, 7, 0x0154, US_REG_DWORD},
    {"imc0_c1_retry_rd_err_log_address1", 2, 10, 7, 0x015c, US_REG_DWORD},
    {"imc0_c1_retry_rd_err_log_address2", 2, 10, 7, 0x0114, US_REG_DWORD},
    {"imc0_c1_retry_rd_err_log_misc", 2, 10, 7, 0x0148, US_REG_DWORD},
    {"imc0_c1_retry_rd_err_log_parity", 2, 10, 7, 0x0150, US_REG_DWORD},
    {"imc0_c2_correction_debug_corr_data", 2, 11, 3, 0x031c, US_REG_DWORD},
    {"imc0_c2_correction_debug_corr_data_cntl", 2, 11, 3, 0x0318, US_REG_DWORD},
    {"imc0_c2_correction_debug_dev_vec_1", 2, 11, 3, 0x0310, US_REG_DWORD},
    {"imc0_c2_correction_debug_dev_vec_2", 2, 11, 3, 0x0314, US_REG_DWORD},
    {"imc0_c2_correction_debug_log", 2, 11, 3, 0x0130, US_REG_DWORD},
    {"imc0_c2_correction_debug_plus1_log", 2, 11, 3, 0x013c, US_REG_DWORD},
    {"imc0_c2_correrrcnt_0", 2, 11, 3, 0x0104, US_REG_DWORD},
    {"imc0_c2_correrrcnt_1", 2, 11, 3, 0x0108, US_REG_DWORD},
    {"imc0_c2_correrrcnt_2", 2, 11, 3, 0x010c, US_REG_DWORD},
    {"imc0_c2_correrrcnt_3", 2, 11, 3, 0x0110, US_REG_DWORD},
    {"imc0_c2_correrrorstatus", 2, 11, 3, 0x0134, US_REG_DWORD},
    {"imc0_c2_correrrthrshld_0", 2, 11, 3, 0x011c, US_REG_DWORD},
    {"imc0_c2_correrrthrshld_1", 2, 11, 3, 0x0120, US_REG_DWORD},
    {"imc0_c2_correrrthrshld_2", 2, 11, 3, 0x0124, US_REG_DWORD},
    {"imc0_c2_correrrthrshld_3", 2, 11, 3, 0x0128, US_REG_DWORD},
    {"imc0_c2_ddrt_err_log_1st", 2, 11, 1, 0x0d80, US_REG_DWORD},
    {"imc0_c2_ddrt_err_log_next", 2, 11, 1, 0x0d84, US_REG_DWORD},
    {"imc0_c2_ddrt_error", 2, 11, 1, 0x0d24, US_REG_DWORD},
    {"imc0_c2_ddrt_fnv0_event0", 2, 11, 1, 0x0a60, US_REG_DWORD},
    {"imc0_c2_ddrt_fnv0_event1", 2, 11, 1, 0x0a64, US_REG_DWORD},
    {"imc0_c2_ddrt_fnv1_event0", 2, 11, 1, 0x0a70, US_REG_DWORD},
    {"imc0_c2_ddrt_fnv1_event1", 2, 11, 1, 0x0a74, US_REG_DWORD},
    {"imc0_c2_ddrt_retry_fsm_state", 2, 11, 1, 0x0904, US_REG_DWORD},
    {"imc0_c2_ddrt_retry_status", 2, 11, 1, 0x0a98, US_REG_DWORD},
    {"imc0_c2_detection_debug_log", 2, 11, 3, 0x014c, US_REG_DWORD},
    {"imc0_c2_devtag_cntl", 2, 11, 3, 0x0140, US_REG_DWORD},
    {"imc0_c2_link_err_fsm_state", 2, 11, 3, 0x0420, US_REG_DWORD},
    {"imc0_c2_link_error", 2, 11, 3, 0x0308, US_REG_DWORD},
    {"imc0_c2_link_retry_err_limits", 2, 11, 3, 0x040c, US_REG_DWORD},
    {"imc0_c2_link_retry_sb_err_count", 2, 11, 3, 0x0400, US_REG_DWORD},
    {"imc0_c2_retry_rd_err_log", 2, 11, 3, 0x0154, US_REG_DWORD},
    {"imc0_c2_retry_rd_err_log_address1", 2, 11, 3, 0x015c, US_REG_DWORD},
    {"imc0_c2_retry_rd_err_log_address2", 2, 11, 3, 0x0114, US_REG_DWORD},
    {"imc0_c2_retry_rd_err_log_misc", 2, 11, 3, 0x0148, US_REG_DWORD},
    {"imc0_c2_retry_rd_err_log_parity", 2, 11, 3, 0x0150, US_REG_DWORD},
    {"imc0_imc0_poison_source", 2, 10, 0, 0x0980, US_REG_QWORD},
    {"imc0_imc1_poison_source", 2, 10, 0, 0x0988, US_REG_QWORD},
    {"imc0_imc2_poison_source", 2, 10, 0, 0x0990, US_REG_QWORD},
    {"imc0_m2mem_err_cntr", 2, 8, 0, 0x0144, US_REG_DWORD},
    {"imc0_m2mem_err_cntr_ctl", 2, 8, 0, 0x0140, US_REG_DWORD},
    {"imc1_c0_correction_debug_corr_data", 2, 12, 3, 0x031c, US_REG_DWORD},
    {"imc1_c0_correction_debug_corr_data_cntl", 2, 12, 3, 0x0318, US_REG_DWORD},
    {"imc1_c0_correction_debug_dev_vec_1", 2, 12, 3, 0x0310, US_REG_DWORD},
    {"imc1_c0_correction_debug_dev_vec_2", 2, 12, 3, 0x0314, US_REG_DWORD},
    {"imc1_c0_correction_debug_log", 2, 12, 3, 0x0130, US_REG_DWORD},
    {"imc1_c0_correction_debug_plus1_log", 2, 12, 3, 0x013c, US_REG_DWORD},
    {"imc1_c0_correrrcnt_0", 2, 12, 3, 0x0104, US_REG_DWORD},
    {"imc1_c0_correrrcnt_1", 2, 12, 3, 0x0108, US_REG_DWORD},
    {"imc1_c0_correrrcnt_2", 2, 12, 3, 0x010c, US_REG_DWORD},
    {"imc1_c0_correrrcnt_3", 2, 12, 3, 0x0110, US_REG_DWORD},
    {"imc1_c0_correrrorstatus", 2, 12, 3, 0x0134, US_REG_DWORD},
    {"imc1_c0_correrrthrshld_0", 2, 12, 3, 0x011c, US_REG_DWORD},
    {"imc1_c0_correrrthrshld_1", 2, 12, 3, 0x0120, US_REG_DWORD},
    {"imc1_c0_correrrthrshld_2", 2, 12, 3, 0x0124, US_REG_DWORD},
    {"imc1_c0_correrrthrshld_3", 2, 12, 3, 0x0128, US_REG_DWORD},
    {"imc1_c0_ddrt_err_log_1st", 2, 12, 1, 0x0d80, US_REG_DWORD},
    {"imc1_c0_ddrt_err_log_next", 2, 12, 1, 0x0d84, US_REG_DWORD},
    {"imc1_c0_ddrt_error", 2, 12, 1, 0x0d24, US_REG_DWORD},
    {"imc1_c0_ddrt_fnv0_event0", 2, 12, 1, 0x0a60, US_REG_DWORD},
    {"imc1_c0_ddrt_fnv0_event1", 2, 12, 1, 0x0a64, US_REG_DWORD},
    {"imc1_c0_ddrt_fnv1_event0", 2, 12, 1, 0x0a70, US_REG_DWORD},
    {"imc1_c0_ddrt_fnv1_event1", 2, 12, 1, 0x0a74, US_REG_DWORD},
    {"imc1_c0_ddrt_retry_fsm_state", 2, 12, 1, 0x0904, US_REG_DWORD},
    {"imc1_c0_ddrt_retry_status", 2, 12, 1, 0x0a98, US_REG_DWORD},
    {"imc1_c0_detection_debug_log", 2, 12, 3, 0x014c, US_REG_DWORD},
    {"imc1_c0_devtag_cntl", 2, 12, 3, 0x0140, US_REG_DWORD},
    {"imc1_c0_link_err_fsm_state", 2, 12, 3, 0x0420, US_REG_DWORD},
    {"imc1_c0_link_error", 2, 12, 3, 0x0308, US_REG_DWORD},
    {"imc1_c0_link_retry_err_limits", 2, 12, 3, 0x040c, US_REG_DWORD},
    {"imc1_c0_link_retry_sb_err_count", 2, 12, 3, 0x0400, US_REG_DWORD},
    {"imc1_c0_retry_rd_err_log", 2, 12, 3, 0x0154, US_REG_DWORD},
    {"imc1_c0_retry_rd_err_log_address1", 2, 12, 3, 0x015c, US_REG_DWORD},
    {"imc1_c0_retry_rd_err_log_address2", 2, 12, 3, 0x0114, US_REG_DWORD},
    {"imc1_c0_retry_rd_err_log_misc", 2, 12, 3, 0x0148, US_REG_DWORD},
    {"imc1_c0_retry_rd_err_log_parity", 2, 12, 3, 0x0150, US_REG_DWORD},
    {"imc1_c1_correction_debug_corr_data", 2, 12, 7, 0x031c, US_REG_DWORD},
    {"imc1_c1_correction_debug_corr_data_cntl", 2, 12, 7, 0x0318, US_REG_DWORD},
    {"imc1_c1_correction_debug_dev_vec_1", 2, 12, 7, 0x0310, US_REG_DWORD},
    {"imc1_c1_correction_debug_dev_vec_2", 2, 12, 7, 0x0314, US_REG_DWORD},
    {"imc1_c1_correction_debug_log", 2, 12, 7, 0x0130, US_REG_DWORD},
    {"imc1_c1_correction_debug_plus1_log", 2, 12, 7, 0x013c, US_REG_DWORD},
    {"imc1_c1_correrrcnt_0", 2, 12, 7, 0x0104, US_REG_DWORD},
    {"imc1_c1_correrrcnt_1", 2, 12, 7, 0x0108, US_REG_DWORD},
    {"imc1_c1_correrrcnt_2", 2, 12, 7, 0x010c, US_REG_DWORD},
    {"imc1_c1_correrrcnt_3", 2, 12, 7, 0x0110, US_REG_DWORD},
    {"imc1_c1_correrrorstatus", 2, 12, 7, 0x0134, US_REG_DWORD},
    {"imc1_c1_correrrthrshld_0", 2, 12, 7, 0x011c, US_REG_DWORD},
    {"imc1_c1_correrrthrshld_1", 2, 12, 7, 0x0120, US_REG_DWORD},
    {"imc1_c1_correrrthrshld_2", 2, 12, 7, 0x0124, US_REG_DWORD},
    {"imc1_c1_correrrthrshld_3", 2, 12, 7, 0x0128, US_REG_DWORD},
    {"imc1_c1_ddrt_err_log_1st", 2, 12, 7, 0x0d80, US_REG_DWORD},
    {"imc1_c1_ddrt_err_log_next", 2, 12, 7, 0x0d84, US_REG_DWORD},
    {"imc1_c1_ddrt_error", 2, 12, 7, 0x0d24, US_REG_DWORD},
    {"imc1_c1_ddrt_fnv0_event0", 2, 12, 7, 0x0a60, US_REG_DWORD},
    {"imc1_c1_ddrt_fnv0_event1", 2, 12, 7, 0x0a64, US_REG_DWORD},
    {"imc1_c1_ddrt_fnv1_event0", 2, 12, 7, 0x0a70, US_REG_DWORD},
    {"imc1_c1_ddrt_fnv1_event1", 2, 12, 7, 0x0a74, US_REG_DWORD},
    {"imc1_c1_ddrt_retry_fsm_state", 2, 12, 7, 0x0904, US_REG_DWORD},
    {"imc1_c1_ddrt_retry_status", 2, 12, 7, 0x0a98, US_REG_DWORD},
    {"imc1_c1_detection_debug_log", 2, 12, 7, 0x014c, US_REG_DWORD},
    {"imc1_c1_devtag_cntl", 2, 12, 7, 0x0140, US_REG_DWORD},
    {"imc1_c1_link_err_fsm_state", 2, 12, 7, 0x0420, US_REG_DWORD},
    {"imc1_c1_link_error", 2, 12, 7, 0x0308, US_REG_DWORD},
    {"imc1_c1_link_retry_err_limits", 2, 12, 7, 0x040c, US_REG_DWORD},
    {"imc1_c1_link_retry_sb_err_count", 2, 12, 7, 0x0400, US_REG_DWORD},
    {"imc1_c1_retry_rd_err_log", 2, 12, 7, 0x0154, US_REG_DWORD},
    {"imc1_c1_retry_rd_err_log_address1", 2, 12, 7, 0x015c, US_REG_DWORD},
    {"imc1_c1_retry_rd_err_log_address2", 2, 12, 7, 0x0114, US_REG_DWORD},
    {"imc1_c1_retry_rd_err_log_misc", 2, 12, 7, 0x0148, US_REG_DWORD},
    {"imc1_c1_retry_rd_err_log_parity", 2, 12, 7, 0x0150, US_REG_DWORD},
    {"imc1_c2_correction_debug_corr_data", 2, 13, 3, 0x031c, US_REG_DWORD},
    {"imc1_c2_correction_debug_corr_data_cntl", 2, 13, 3, 0x0318, US_REG_DWORD},
    {"imc1_c2_correction_debug_dev_vec_1", 2, 13, 3, 0x0310, US_REG_DWORD},
    {"imc1_c2_correction_debug_dev_vec_2", 2, 13, 3, 0x0314, US_REG_DWORD},
    {"imc1_c2_correction_debug_log", 2, 13, 3, 0x0130, US_REG_DWORD},
    {"imc1_c2_correction_debug_plus1_log", 2, 13, 3, 0x013c, US_REG_DWORD},
    {"imc1_c2_correrrcnt_0", 2, 13, 3, 0x0104, US_REG_DWORD},
    {"imc1_c2_correrrcnt_1", 2, 13, 3, 0x0108, US_REG_DWORD},
    {"imc1_c2_correrrcnt_2", 2, 13, 3, 0x010c, US_REG_DWORD},
    {"imc1_c2_correrrcnt_3", 2, 13, 3, 0x0110, US_REG_DWORD},
    {"imc1_c2_correrrorstatus", 2, 13, 3, 0x0134, US_REG_DWORD},
    {"imc1_c2_correrrthrshld_0", 2, 13, 3, 0x011c, US_REG_DWORD},
    {"imc1_c2_correrrthrshld_1", 2, 13, 3, 0x0120, US_REG_DWORD},
    {"imc1_c2_correrrthrshld_2", 2, 13, 3, 0x0124, US_REG_DWORD},
    {"imc1_c2_correrrthrshld_3", 2, 13, 3, 0x0128, US_REG_DWORD},
    {"imc1_c2_ddrt_err_log_1st", 2, 13, 1, 0x0d80, US_REG_DWORD},
    {"imc1_c2_ddrt_err_log_next", 2, 13, 1, 0x0d84, US_REG_DWORD},
    {"imc1_c2_ddrt_error", 2, 13, 1, 0x0d24, US_REG_DWORD},
    {"imc1_c2_ddrt_fnv0_event0", 2, 13, 1, 0x0a60, US_REG_DWORD},
    {"imc1_c2_ddrt_fnv0_event1", 2, 13, 1, 0x0a64, US_REG_DWORD},
    {"imc1_c2_ddrt_fnv1_event0", 2, 13, 1, 0x0a70, US_REG_DWORD},
    {"imc1_c2_ddrt_fnv1_event1", 2, 13, 1, 0x0a74, US_REG_DWORD},
    {"imc1_c2_ddrt_retry_fsm_state", 2, 13, 1, 0x0904, US_REG_DWORD},
    {"imc1_c2_ddrt_retry_status", 2, 13, 1, 0x0a98, US_REG_DWORD},
    {"imc1_c2_detection_debug_log", 2, 13, 3, 0x014c, US_REG_DWORD},
    {"imc1_c2_devtag_cntl", 2, 13, 3, 0x0140, US_REG_DWORD},
    {"imc1_c2_link_err_fsm_state", 2, 13, 3, 0x0420, US_REG_DWORD},
    {"imc1_c2_link_error", 2, 13, 3, 0x0308, US_REG_DWORD},
    {"imc1_c2_link_retry_err_limits", 2, 13, 3, 0x040c, US_REG_DWORD},
    {"imc1_c2_link_retry_sb_err_count", 2, 13, 3, 0x0400, US_REG_DWORD},
    {"imc1_c2_retry_rd_err_log", 2, 13, 3, 0x0154, US_REG_DWORD},
    {"imc1_c2_retry_rd_err_log_address1", 2, 13, 3, 0x015c, US_REG_DWORD},
    {"imc1_c2_retry_rd_err_log_address2", 2, 13, 3, 0x0114, US_REG_DWORD},
    {"imc1_c2_retry_rd_err_log_misc", 2, 13, 3, 0x0148, US_REG_DWORD},
    {"imc1_c2_retry_rd_err_log_parity", 2, 13, 3, 0x0150, US_REG_DWORD},
    {"imc1_imc0_poison_source", 2, 12, 0, 0x0980, US_REG_QWORD},
    {"imc1_imc1_poison_source", 2, 12, 0, 0x0988, US_REG_QWORD},
    {"imc1_imc2_poison_source", 2, 12, 0, 0x0990, US_REG_QWORD},
    {"imc1_m2mem_err_cntr", 2, 9, 0, 0x0144, US_REG_DWORD},
    {"imc1_m2mem_err_cntr_ctl", 2, 9, 0, 0x0140, US_REG_DWORD},
    {"m2_mpc1_rpegrctrlconfig4_r2egrerrlog", 3, 22, 4, 0x00b0, US_REG_DWORD},
    {"m2_mpc1_rpegrctrlconfig4_r2egrisoerrlog0", 3, 22, 4, 0x0060,
     US_REG_DWORD},
    {"m2_mpc1_rpegrctrlconfig4_r2egrprqerrlog0", 3, 22, 4, 0x0068,
     US_REG_DWORD},
    {"m2_mpc1_rputlctrlconfig4_r2glerrcfg", 3, 22, 4, 0x00a8, US_REG_DWORD},
    {"m2d_pcie1_rpegrctrlconfig1_r2egrerrlog", 3, 22, 0, 0x00b0, US_REG_DWORD},
    {"m2d_pcie1_rpegrctrlconfig1_r2egrprqerrlog0", 3, 22, 0, 0x0068,
     US_REG_DWORD},
    {"m2u_pcie0_rpegrctrlconfig0_r2egrerrlog", 3, 21, 0, 0x00b0, US_REG_DWORD},
    {"m2u_pcie0_rpegrctrlconfig0_r2egrisoerrlog0", 3, 21, 0, 0x0060,
     US_REG_DWORD},
    {"m2u_pcie0_rpegrctrlconfig0_r2egrprqerrlog0", 3, 21, 0, 0x0068,
     US_REG_DWORD},
    {"b0d5f1_sltsts0", 0, 5, 1, 0x010e, US_REG_WORD},
    {"b0d5f1_sltsts1", 0, 5, 1, 0x011e, US_REG_WORD},
    {"b0d5f1_sltsts2", 0, 5, 1, 0x012e, US_REG_WORD},
    {"b0d5f1_sltsts3", 0, 5, 1, 0x013e, US_REG_WORD},
    {"cb0_devsts", 0, 4, 0, 0x009a, US_REG_WORD},
    {"cb0_dmauncerrsts", 0, 4, 0, 0x0148, US_REG_DWORD},
    {"cb1_devsts", 0, 4, 1, 0x009a, US_REG_WORD},
    {"cb2_devsts", 0, 4, 2, 0x009a, US_REG_WORD},
    {"cb3_devsts", 0, 4, 3, 0x009a, US_REG_WORD},
    {"cb4_devsts", 0, 4, 4, 0x009a, US_REG_WORD},
    {"cb5_devsts", 0, 4, 5, 0x009a, US_REG_WORD},
    {"cb6_devsts", 0, 4, 6, 0x009a, US_REG_WORD},
    {"cb7_devsts", 0, 4, 7, 0x009a, US_REG_WORD},
    {"pxp_b0d00f0_corerrsts", 0, 0, 0, 0x0158, US_REG_DWORD},
    {"pxp_b0d00f0_devsts", 0, 0, 0, 0x009a, US_REG_WORD},
    {"pxp_b0d00f0_errsid", 0, 0, 0, 0x017c, US_REG_DWORD},
    {"pxp_b0d00f0_ler_ctrlsts", 0, 0, 0, 0x0288, US_REG_DWORD},
    {"pxp_b0d00f0_lnerrsts", 0, 0, 0, 0x0258, US_REG_DWORD},
    {"pxp_b0d00f0_lnksts", 0, 0, 0, 0x00a2, US_REG_WORD},
    {"pxp_b0d00f0_lnksts2", 0, 0, 0, 0x00c2, US_REG_WORD},
    {"pxp_b0d00f0_miscctrlsts_0", 0, 0, 0, 0x0188, US_REG_DWORD},
    {"pxp_b0d00f0_miscctrlsts_1", 0, 0, 0, 0x018c, US_REG_DWORD},
    {"pxp_b0d00f0_pcists", 0, 0, 0, 0x0006, US_REG_WORD},
    {"pxp_b0d00f0_rperrsts", 0, 0, 0, 0x0178, US_REG_DWORD},
    {"pxp_b0d00f0_rppioerr_cap", 0, 0, 0, 0x0298, US_REG_DWORD},
    {"pxp_b0d00f0_rppioerr_status", 0, 0, 0, 0x02a4, US_REG_DWORD},
    {"pxp_b0d00f0_secsts", 0, 0, 0, 0x001e, US_REG_WORD},
    {"pxp_b0d00f0_sltsts", 0, 0, 0, 0x00aa, US_REG_WORD},
    {"pxp_b0d00f0_uncerrsts", 0, 0, 0, 0x014c, US_REG_DWORD},
    {"pxp_b0d00f0_xpcorerrsts", 0, 0, 0, 0x0200, US_REG_DWORD},
    {"pxp_b0d00f0_xpglberrptr", 0, 0, 0, 0x0232, US_REG_WORD},
    {"pxp_b0d00f0_xpglberrsts", 0, 0, 0, 0x0230, US_REG_WORD},
    {"pxp_b0d00f0_xpuncerrsts", 0, 0, 0, 0x0208, US_REG_DWORD},
    {"pxp_b0d07f0_devsts", 0, 7, 0, 0x004a, US_REG_WORD},
    {"pxp_b0d07f0_lnksts", 0, 7, 0, 0x0052, US_REG_WORD},
    {"pxp_b0d07f0_lnksts2", 0, 7, 0, 0x0072, US_REG_WORD},
    {"pxp_b0d07f0_pcists", 0, 7, 0, 0x0006, US_REG_WORD},
    {"pxp_b0d07f0_sltsts2", 0, 7, 0, 0x007a, US_REG_WORD},
    {"pxp_b0d07f4_devsts", 0, 7, 4, 0x004a, US_REG_WORD},
    {"pxp_b0d07f4_lnksts", 0, 7, 4, 0x0052, US_REG_WORD},
    {"pxp_b0d07f4_lnksts2", 0, 7, 4, 0x0072, US_REG_WORD},
    {"pxp_b0d07f4_pcists", 0, 7, 4, 0x0006, US_REG_WORD},
    {"pxp_b0d07f4_sltsts2", 0, 7, 4, 0x007a, US_REG_WORD},
    {"pxp_b0d07f7_devsts", 0, 7, 7, 0x004a, US_REG_WORD},
    {"pxp_b0d07f7_lnksts", 0, 7, 7, 0x0052, US_REG_WORD},
    {"pxp_b0d07f7_lnksts2", 0, 7, 7, 0x0072, US_REG_WORD},
    {"pxp_b0d07f7_pcists", 0, 7, 7, 0x0006, US_REG_WORD},
    {"pxp_b0d07f7_sltsts2", 0, 7, 7, 0x007a, US_REG_WORD},
    {"pxp_b0d07f7_tswdbgerrstdis1", 0, 7, 7, 0x0358, US_REG_DWORD},
    {"pxp_b1d00f0_corerrsts", 1, 0, 0, 0x0158, US_REG_DWORD},
    {"pxp_b1d00f0_devsts", 1, 0, 0, 0x009a, US_REG_WORD},
    {"pxp_b1d00f0_errsid", 1, 0, 0, 0x017c, US_REG_DWORD},
    {"pxp_b1d00f0_ler_ctrlsts", 1, 0, 0, 0x0288, US_REG_DWORD},
    {"pxp_b1d00f0_lnerrsts", 1, 0, 0, 0x0258, US_REG_DWORD},
    {"pxp_b1d00f0_lnksts", 1, 0, 0, 0x00a2, US_REG_WORD},
    {"pxp_b1d00f0_lnksts2", 1, 0, 0, 0x00c2, US_REG_WORD},
    {"pxp_b1d00f0_miscctrlsts_0", 1, 0, 0, 0x0188, US_REG_DWORD},
    {"pxp_b1d00f0_miscctrlsts_1", 1, 0, 0, 0x018c, US_REG_DWORD},
    {"pxp_b1d00f0_pcists", 1, 0, 0, 0x0006, US_REG_WORD},
    {"pxp_b1d00f0_rperrsts", 1, 0, 0, 0x0178, US_REG_DWORD},
    {"pxp_b1d00f0_rppioerr_cap", 1, 0, 0, 0x0298, US_REG_DWORD},
    {"pxp_b1d00f0_rppioerr_status", 1, 0, 0, 0x02a4, US_REG_DWORD},
    {"pxp_b1d00f0_secsts", 1, 0, 0, 0x001e, US_REG_WORD},
    {"pxp_b1d00f0_sltsts", 1, 0, 0, 0x00aa, US_REG_WORD},
    {"pxp_b1d00f0_uncerrsts", 1, 0, 0, 0x014c, US_REG_DWORD},
    {"pxp_b1d00f0_xpcorerrsts", 1, 0, 0, 0x0200, US_REG_DWORD},
    {"pxp_b1d00f0_xpglberrptr", 1, 0, 0, 0x0232, US_REG_WORD},
    {"pxp_b1d00f0_xpglberrsts", 1, 0, 0, 0x0230, US_REG_WORD},
    {"pxp_b1d00f0_xpuncerrsts", 1, 0, 0, 0x0208, US_REG_DWORD},
    {"pxp_b1d01f0_corerrsts", 1, 1, 0, 0x0158, US_REG_DWORD},
    {"pxp_b1d01f0_devsts", 1, 1, 0, 0x009a, US_REG_WORD},
    {"pxp_b1d01f0_errsid", 1, 1, 0, 0x017c, US_REG_DWORD},
    {"pxp_b1d01f0_ler_ctrlsts", 1, 1, 0, 0x0288, US_REG_DWORD},
    {"pxp_b1d01f0_lnerrsts", 1, 1, 0, 0x0258, US_REG_DWORD},
    {"pxp_b1d01f0_lnksts", 1, 1, 0, 0x00a2, US_REG_WORD},
    {"pxp_b1d01f0_lnksts2", 1, 1, 0, 0x00c2, US_REG_WORD},
    {"pxp_b1d01f0_miscctrlsts_0", 1, 1, 0, 0x0188, US_REG_DWORD},
    {"pxp_b1d01f0_miscctrlsts_1", 1, 1, 0, 0x018c, US_REG_DWORD},
    {"pxp_b1d01f0_pcists", 1, 1, 0, 0x0006, US_REG_WORD},
    {"pxp_b1d01f0_rperrsts", 1, 1, 0, 0x0178, US_REG_DWORD},
    {"pxp_b1d01f0_rppioerr_cap", 1, 1, 0, 0x0298, US_REG_DWORD},
    {"pxp_b1d01f0_rppioerr_status", 1, 1, 0, 0x02a4, US_REG_DWORD},
    {"pxp_b1d01f0_secsts", 1, 1, 0, 0x001e, US_REG_WORD},
    {"pxp_b1d01f0_sltsts", 1, 1, 0, 0x00aa, US_REG_WORD},
    {"pxp_b1d01f0_xpcorerrsts", 1, 1, 0, 0x0200, US_REG_DWORD},
    {"pxp_b1d01f0_xpglberrptr", 1, 1, 0, 0x0232, US_REG_WORD},
    {"pxp_b1d01f0_xpglberrsts", 1, 1, 0, 0x0230, US_REG_WORD},
    {"pxp_b1d01f0_xpuncerrsts", 1, 1, 0, 0x0208, US_REG_DWORD},
    {"pxp_b1d02f0_corerrsts", 1, 2, 0, 0x0158, US_REG_DWORD},
    {"pxp_b1d02f0_devsts", 1, 2, 0, 0x009a, US_REG_WORD},
    {"pxp_b1d02f0_errsid", 1, 2, 0, 0x017c, US_REG_DWORD},
    {"pxp_b1d02f0_ler_ctrlsts", 1, 2, 0, 0x0288, US_REG_DWORD},
    {"pxp_b1d02f0_lnerrsts", 1, 2, 0, 0x0258, US_REG_DWORD},
    {"pxp_b1d02f0_lnksts", 1, 2, 0, 0x00a2, US_REG_WORD},
    {"pxp_b1d02f0_lnksts2", 1, 2, 0, 0x00c2, US_REG_WORD},
    {"pxp_b1d02f0_miscctrlsts_0", 1, 2, 0, 0x0188, US_REG_DWORD},
    {"pxp_b1d02f0_miscctrlsts_1", 1, 2, 0, 0x018c, US_REG_DWORD},
    {"pxp_b1d02f0_pcists", 1, 2, 0, 0x0006, US_REG_WORD},
    {"pxp_b1d02f0_rperrsts", 1, 2, 0, 0x0178, US_REG_DWORD},
    {"pxp_b1d02f0_rppioerr_cap", 1, 2, 0, 0x0298, US_REG_DWORD},
    {"pxp_b1d02f0_rppioerr_status", 1, 2, 0, 0x02a4, US_REG_DWORD},
    {"pxp_b1d02f0_secsts", 1, 2, 0, 0x001e, US_REG_WORD},
    {"pxp_b1d02f0_sltsts", 1, 2, 0, 0x00aa, US_REG_WORD},
    {"pxp_b1d02f0_uncerrsts", 1, 2, 0, 0x014c, US_REG_DWORD},
    {"pxp_b1d02f0_xpcorerrsts", 1, 2, 0, 0x0200, US_REG_DWORD},
    {"pxp_b1d02f0_xpglberrptr", 1, 2, 0, 0x0232, US_REG_WORD},
    {"pxp_b1d02f0_xpglberrsts", 1, 2, 0, 0x0230, US_REG_WORD},
    {"pxp_b1d03f0_corerrsts", 1, 3, 0, 0x0158, US_REG_DWORD},
    {"pxp_b1d03f0_devsts", 1, 3, 0, 0x009a, US_REG_WORD},
    {"pxp_b1d03f0_errsid", 1, 3, 0, 0x017c, US_REG_DWORD},
    {"pxp_b1d03f0_ler_ctrlsts", 1, 3, 0, 0x0288, US_REG_DWORD},
    {"pxp_b1d03f0_lnerrsts", 1, 3, 0, 0x0258, US_REG_DWORD},
    {"pxp_b1d03f0_lnksts", 1, 3, 0, 0x00a2, US_REG_WORD},
    {"pxp_b1d03f0_lnksts2", 1, 3, 0, 0x00c2, US_REG_WORD},
    {"pxp_b1d03f0_miscctrlsts_0", 1, 3, 0, 0x0188, US_REG_DWORD},
    {"pxp_b1d03f0_miscctrlsts_1", 1, 3, 0, 0x018c, US_REG_DWORD},
    {"pxp_b1d03f0_pcists", 1, 3, 0, 0x0006, US_REG_WORD},
    {"pxp_b1d03f0_rperrsts", 1, 3, 0, 0x0178, US_REG_DWORD},
    {"pxp_b1d03f0_rppioerr_cap", 1, 3, 0, 0x0298, US_REG_DWORD},
    {"pxp_b1d03f0_rppioerr_status", 1, 3, 0, 0x02a4, US_REG_DWORD},
    {"pxp_b1d03f0_secsts", 1, 3, 0, 0x001e, US_REG_WORD},
    {"pxp_b1d03f0_sltsts", 1, 3, 0, 0x00aa, US_REG_WORD},
    {"pxp_b1d03f0_xpcorerrsts", 1, 3, 0, 0x0200, US_REG_DWORD},
    {"pxp_b1d03f0_xpglberrptr", 1, 3, 0, 0x0232, US_REG_WORD},
    {"pxp_b1d03f0_xpglberrsts", 1, 3, 0, 0x0230, US_REG_WORD},
    {"pxp_b1d07f0_devsts", 1, 7, 0, 0x004a, US_REG_WORD},
    {"pxp_b1d07f0_lnksts", 1, 7, 0, 0x0052, US_REG_WORD},
    {"pxp_b1d07f0_lnksts2", 1, 7, 0, 0x0072, US_REG_WORD},
    {"pxp_b1d07f0_pcists", 1, 7, 0, 0x0006, US_REG_WORD},
    {"pxp_b1d07f0_sltsts2", 1, 7, 0, 0x007a, US_REG_WORD},
    {"pxp_b1d07f1_devsts", 1, 7, 1, 0x004a, US_REG_WORD},
    {"pxp_b1d07f1_lnksts", 1, 7, 1, 0x0052, US_REG_WORD},
    {"pxp_b1d07f1_lnksts2", 1, 7, 1, 0x0072, US_REG_WORD},
    {"pxp_b1d07f1_pcists", 1, 7, 1, 0x0006, US_REG_WORD},
    {"pxp_b1d07f1_sltsts2", 1, 7, 1, 0x007a, US_REG_WORD},
    {"pxp_b1d07f2_devsts", 1, 7, 2, 0x004a, US_REG_WORD},
    {"pxp_b1d07f2_lnksts", 1, 7, 2, 0x0052, US_REG_WORD},
    {"pxp_b1d07f2_lnksts2", 1, 7, 2, 0x0072, US_REG_WORD},
    {"pxp_b1d07f2_pcists", 1, 7, 2, 0x0006, US_REG_WORD},
    {"pxp_b1d07f2_sltsts2", 1, 7, 2, 0x007a, US_REG_WORD},
    {"pxp_b1d07f3_devsts", 1, 7, 3, 0x004a, US_REG_WORD},
    {"pxp_b1d07f3_lnksts", 1, 7, 3, 0x0052, US_REG_WORD},
    {"pxp_b1d07f3_lnksts2", 1, 7, 3, 0x0072, US_REG_WORD},
    {"pxp_b1d07f3_pcists", 1, 7, 3, 0x0006, US_REG_WORD},
    {"pxp_b1d07f3_sltsts2", 1, 7, 3, 0x007a, US_REG_WORD},
    {"pxp_b1d07f4_devsts", 1, 7, 4, 0x004a, US_REG_WORD},
    {"pxp_b1d07f4_lnksts", 1, 7, 4, 0x0052, US_REG_WORD},
    {"pxp_b1d07f4_lnksts2", 1, 7, 4, 0x0072, US_REG_WORD},
    {"pxp_b1d07f4_pcists", 1, 7, 4, 0x0006, US_REG_WORD},
    {"pxp_b1d07f4_sltsts2", 1, 7, 4, 0x007a, US_REG_WORD},
    {"pxp_b1d07f7_devsts", 1, 7, 7, 0x004a, US_REG_WORD},
    {"pxp_b1d07f7_lnksts", 1, 7, 7, 0x0052, US_REG_WORD},
    {"pxp_b1d07f7_lnksts2", 1, 7, 7, 0x0072, US_REG_WORD},
    {"pxp_b1d07f7_pcists", 1, 7, 7, 0x0006, US_REG_WORD},
    {"pxp_b1d07f7_sltsts2", 1, 7, 7, 0x007a, US_REG_WORD},
    {"pxp_b1d07f7_tswdbgerrstdis0", 1, 7, 7, 0x0358, US_REG_DWORD},
    {"pxp_b2d00f0_corerrsts", 2, 0, 0, 0x0158, US_REG_DWORD},
    {"pxp_b2d00f0_devsts", 2, 0, 0, 0x009a, US_REG_WORD},
    {"pxp_b2d00f0_errsid", 2, 0, 0, 0x017c, US_REG_DWORD},
    {"pxp_b2d00f0_ler_ctrlsts", 2, 0, 0, 0x0288, US_REG_DWORD},
    {"pxp_b2d00f0_lnerrsts", 2, 0, 0, 0x0258, US_REG_DWORD},
    {"pxp_b2d00f0_lnksts", 2, 0, 0, 0x00a2, US_REG_WORD},
    {"pxp_b2d00f0_lnksts2", 2, 0, 0, 0x00c2, US_REG_WORD},
    {"pxp_b2d00f0_miscctrlsts_0", 2, 0, 0, 0x0188, US_REG_DWORD},
    {"pxp_b2d00f0_miscctrlsts_1", 2, 0, 0, 0x018c, US_REG_DWORD},
    {"pxp_b2d00f0_pcists", 2, 0, 0, 0x0006, US_REG_WORD},
    {"pxp_b2d00f0_rperrsts", 2, 0, 0, 0x0178, US_REG_DWORD},
    {"pxp_b2d00f0_rppioerr_cap", 2, 0, 0, 0x0298, US_REG_DWORD},
    {"pxp_b2d00f0_rppioerr_status", 2, 0, 0, 0x02a4, US_REG_DWORD},
    {"pxp_b2d00f0_secsts", 2, 0, 0, 0x001e, US_REG_WORD},
    {"pxp_b2d00f0_sltsts", 2, 0, 0, 0x00aa, US_REG_WORD},
    {"pxp_b2d00f0_uncerrsts", 2, 0, 0, 0x014c, US_REG_DWORD},
    {"pxp_b2d00f0_xpcorerrsts", 2, 0, 0, 0x0200, US_REG_DWORD},
    {"pxp_b2d00f0_xpglberrptr", 2, 0, 0, 0x0232, US_REG_WORD},
    {"pxp_b2d00f0_xpglberrsts", 2, 0, 0, 0x0230, US_REG_WORD},
    {"pxp_b2d00f0_xpuncerrsts", 2, 0, 0, 0x0208, US_REG_DWORD},
    {"pxp_b2d01f0_corerrsts", 2, 1, 0, 0x0158, US_REG_DWORD},
    {"pxp_b2d01f0_devsts", 2, 1, 0, 0x009a, US_REG_WORD},
    {"pxp_b2d01f0_errsid", 2, 1, 0, 0x017c, US_REG_DWORD},
    {"pxp_b2d01f0_ler_ctrlsts", 2, 1, 0, 0x0288, US_REG_DWORD},
    {"pxp_b2d01f0_lnerrsts", 2, 1, 0, 0x0258, US_REG_DWORD},
    {"pxp_b2d01f0_lnksts", 2, 1, 0, 0x00a2, US_REG_WORD},
    {"pxp_b2d01f0_lnksts2", 2, 1, 0, 0x00c2, US_REG_WORD},
    {"pxp_b2d01f0_miscctrlsts_0", 2, 1, 0, 0x0188, US_REG_DWORD},
    {"pxp_b2d01f0_miscctrlsts_1", 2, 1, 0, 0x018c, US_REG_DWORD},
    {"pxp_b2d01f0_pcists", 2, 1, 0, 0x0006, US_REG_WORD},
    {"pxp_b2d01f0_rperrsts", 2, 1, 0, 0x0178, US_REG_DWORD},
    {"pxp_b2d01f0_rppioerr_cap", 2, 1, 0, 0x0298, US_REG_DWORD},
    {"pxp_b2d01f0_rppioerr_status", 2, 1, 0, 0x02a4, US_REG_DWORD},
    {"pxp_b2d01f0_secsts", 2, 1, 0, 0x001e, US_REG_WORD},
    {"pxp_b2d01f0_sltsts", 2, 1, 0, 0x00aa, US_REG_WORD},
    {"pxp_b2d01f0_uncerrsts", 2, 1, 0, 0x014c, US_REG_DWORD},
    {"pxp_b2d01f0_xpcorerrsts", 2, 1, 0, 0x0200, US_REG_DWORD},
    {"pxp_b2d01f0_xpglberrptr", 2, 1, 0, 0x0232, US_REG_WORD},
    {"pxp_b2d01f0_xpglberrsts", 2, 1, 0, 0x0230, US_REG_WORD},
    {"pxp_b2d01f0_xpuncerrsts", 2, 1, 0, 0x0208, US_REG_DWORD},
    {"pxp_b2d02f0_corerrsts", 2, 2, 0, 0x0158, US_REG_DWORD},
    {"pxp_b2d02f0_devsts", 2, 2, 0, 0x009a, US_REG_WORD},
    {"pxp_b2d02f0_errsid", 2, 2, 0, 0x017c, US_REG_DWORD},
    {"pxp_b2d02f0_ler_ctrlsts", 2, 2, 0, 0x0288, US_REG_DWORD},
    {"pxp_b2d02f0_lnerrsts", 2, 2, 0, 0x0258, US_REG_DWORD},
    {"pxp_b2d02f0_lnksts", 2, 2, 0, 0x00a2, US_REG_WORD},
    {"pxp_b2d02f0_lnksts2", 2, 2, 0, 0x00c2, US_REG_WORD},
    {"pxp_b2d02f0_miscctrlsts_0", 2, 2, 0, 0x0188, US_REG_DWORD},
    {"pxp_b2d02f0_miscctrlsts_1", 2, 2, 0, 0x018c, US_REG_DWORD},
    {"pxp_b2d02f0_pcists", 2, 2, 0, 0x0006, US_REG_WORD},
    {"pxp_b2d02f0_rperrsts", 2, 2, 0, 0x0178, US_REG_DWORD},
    {"pxp_b2d02f0_rppioerr_cap", 2, 2, 0, 0x0298, US_REG_DWORD},
    {"pxp_b2d02f0_rppioerr_status", 2, 2, 0, 0x02a4, US_REG_DWORD},
    {"pxp_b2d02f0_secsts", 2, 2, 0, 0x001e, US_REG_WORD},
    {"pxp_b2d02f0_sltsts", 2, 2, 0, 0x00aa, US_REG_WORD},
    {"pxp_b2d02f0_uncerrsts", 2, 2, 0, 0x014c, US_REG_DWORD},
    {"pxp_b2d02f0_xpcorerrsts", 2, 2, 0, 0x0200, US_REG_DWORD},
    {"pxp_b2d02f0_xpglberrptr", 2, 2, 0, 0x0232, US_REG_WORD},
    {"pxp_b2d02f0_xpglberrsts", 2, 2, 0, 0x0230, US_REG_WORD},
    {"pxp_b2d02f0_xpuncerrsts", 2, 2, 0, 0x0208, US_REG_DWORD},
    {"pxp_b2d03f0_corerrsts", 2, 3, 0, 0x0158, US_REG_DWORD},
    {"pxp_b2d03f0_devsts", 2, 3, 0, 0x009a, US_REG_WORD},
    {"pxp_b2d03f0_errsid", 2, 3, 0, 0x017c, US_REG_DWORD},
    {"pxp_b2d03f0_ler_ctrlsts", 2, 3, 0, 0x0288, US_REG_DWORD},
    {"pxp_b2d03f0_lnerrsts", 2, 3, 0, 0x0258, US_REG_DWORD},
    {"pxp_b2d03f0_lnksts", 2, 3, 0, 0x00a2, US_REG_WORD},
    {"pxp_b2d03f0_lnksts2", 2, 3, 0, 0x00c2, US_REG_WORD},
    {"pxp_b2d03f0_miscctrlsts_0", 2, 3, 0, 0x0188, US_REG_DWORD},
    {"pxp_b2d03f0_miscctrlsts_1", 2, 3, 0, 0x018c, US_REG_DWORD},
    {"pxp_b2d03f0_pcists", 2, 3, 0, 0x0006, US_REG_WORD},
    {"pxp_b2d03f0_rperrsts", 2, 3, 0, 0x0178, US_REG_DWORD},
    {"pxp_b2d03f0_rppioerr_cap", 2, 3, 0, 0x0298, US_REG_DWORD},
    {"pxp_b2d03f0_rppioerr_status", 2, 3, 0, 0x02a4, US_REG_DWORD},
    {"pxp_b2d03f0_secsts", 2, 3, 0, 0x001e, US_REG_WORD},
    {"pxp_b2d03f0_sltsts", 2, 3, 0, 0x00aa, US_REG_WORD},
    {"pxp_b2d03f0_uncerrsts", 2, 3, 0, 0x014c, US_REG_DWORD},
    {"pxp_b2d03f0_xpcorerrsts", 2, 3, 0, 0x0200, US_REG_DWORD},
    {"pxp_b2d03f0_xpglberrptr", 2, 3, 0, 0x0232, US_REG_WORD},
    {"pxp_b2d03f0_xpglberrsts", 2, 3, 0, 0x0230, US_REG_WORD},
    {"pxp_b2d03f0_xpuncerrsts", 2, 3, 0, 0x0208, US_REG_DWORD},
    {"pxp_b2d07f0_devsts", 2, 7, 0, 0x004a, US_REG_WORD},
    {"pxp_b2d07f0_lnksts", 2, 7, 0, 0x0052, US_REG_WORD},
    {"pxp_b2d07f0_lnksts2", 2, 7, 0, 0x0072, US_REG_WORD},
    {"pxp_b2d07f0_pcists", 2, 7, 0, 0x0006, US_REG_WORD},
    {"pxp_b2d07f0_sltsts2", 2, 7, 0, 0x007a, US_REG_WORD},
    {"pxp_b2d07f1_devsts", 2, 7, 1, 0x004a, US_REG_WORD},
    {"pxp_b2d07f1_lnksts", 2, 7, 1, 0x0052, US_REG_WORD},
    {"pxp_b2d07f1_lnksts2", 2, 7, 1, 0x0072, US_REG_WORD},
    {"pxp_b2d07f1_pcists", 2, 7, 1, 0x0006, US_REG_WORD},
    {"pxp_b2d07f1_sltsts2", 2, 7, 1, 0x007a, US_REG_WORD},
    {"pxp_b2d07f2_devsts", 2, 7, 2, 0x004a, US_REG_WORD},
    {"pxp_b2d07f2_lnksts", 2, 7, 2, 0x0052, US_REG_WORD},
    {"pxp_b2d07f2_lnksts2", 2, 7, 2, 0x0072, US_REG_WORD},
    {"pxp_b2d07f2_pcists", 2, 7, 2, 0x0006, US_REG_WORD},
    {"pxp_b2d07f2_sltsts2", 2, 7, 2, 0x007a, US_REG_WORD},
    {"pxp_b2d07f3_devsts", 2, 7, 3, 0x004a, US_REG_WORD},
    {"pxp_b2d07f3_lnksts", 2, 7, 3, 0x0052, US_REG_WORD},
    {"pxp_b2d07f3_lnksts2", 2, 7, 3, 0x0072, US_REG_WORD},
    {"pxp_b2d07f3_pcists", 2, 7, 3, 0x0006, US_REG_WORD},
    {"pxp_b2d07f3_sltsts2", 2, 7, 3, 0x007a, US_REG_WORD},
    {"pxp_b2d07f4_devsts", 2, 7, 4, 0x004a, US_REG_WORD},
    {"pxp_b2d07f4_lnksts", 2, 7, 4, 0x0052, US_REG_WORD},
    {"pxp_b2d07f4_lnksts2", 2, 7, 4, 0x0072, US_REG_WORD},
    {"pxp_b2d07f4_pcists", 2, 7, 4, 0x0006, US_REG_WORD},
    {"pxp_b2d07f4_sltsts2", 2, 7, 4, 0x007a, US_REG_WORD},
    {"pxp_b2d07f7_devsts", 2, 7, 7, 0x004a, US_REG_WORD},
    {"pxp_b2d07f7_lnksts", 2, 7, 7, 0x0052, US_REG_WORD},
    {"pxp_b2d07f7_lnksts2", 2, 7, 7, 0x0072, US_REG_WORD},
    {"pxp_b2d07f7_pcists", 2, 7, 7, 0x0006, US_REG_WORD},
    {"pxp_b2d07f7_sltsts2", 2, 7, 7, 0x007a, US_REG_WORD},
    {"pxp_b2d07f7_tswdbgerrstdis0", 2, 7, 7, 0x0358, US_REG_DWORD},
    {"pxp_b3d00f0_corerrsts", 3, 0, 0, 0x0158, US_REG_DWORD},
    {"pxp_b3d00f0_devsts", 3, 0, 0, 0x009a, US_REG_WORD},
    {"pxp_b3d00f0_errsid", 3, 0, 0, 0x017c, US_REG_DWORD},
    {"pxp_b3d00f0_ler_ctrlsts", 3, 0, 0, 0x0288, US_REG_DWORD},
    {"pxp_b3d00f0_lnerrsts", 3, 0, 0, 0x0258, US_REG_DWORD},
    {"pxp_b3d00f0_lnksts", 3, 0, 0, 0x00a2, US_REG_WORD},
    {"pxp_b3d00f0_lnksts2", 3, 0, 0, 0x00c2, US_REG_WORD},
    {"pxp_b3d00f0_miscctrlsts_0", 3, 0, 0, 0x0188, US_REG_DWORD},
    {"pxp_b3d00f0_miscctrlsts_1", 3, 0, 0, 0x018c, US_REG_DWORD},
    {"pxp_b3d00f0_pcists", 3, 0, 0, 0x0006, US_REG_WORD},
    {"pxp_b3d00f0_rperrsts", 3, 0, 0, 0x0178, US_REG_DWORD},
    {"pxp_b3d00f0_rppioerr_cap", 3, 0, 0, 0x0298, US_REG_DWORD},
    {"pxp_b3d00f0_rppioerr_status", 3, 0, 0, 0x02a4, US_REG_DWORD},
    {"pxp_b3d00f0_secsts", 3, 0, 0, 0x001e, US_REG_WORD},
    {"pxp_b3d00f0_sltsts", 3, 0, 0, 0x00aa, US_REG_WORD},
    {"pxp_b3d00f0_uncerrsts", 3, 0, 0, 0x014c, US_REG_DWORD},
    {"pxp_b3d00f0_xpcorerrsts", 3, 0, 0, 0x0200, US_REG_DWORD},
    {"pxp_b3d00f0_xpglberrptr", 3, 0, 0, 0x0232, US_REG_WORD},
    {"pxp_b3d00f0_xpglberrsts", 3, 0, 0, 0x0230, US_REG_WORD},
    {"pxp_b3d00f0_xpuncerrsts", 3, 0, 0, 0x0208, US_REG_DWORD},
    {"pxp_b3d01f0_corerrsts", 3, 1, 0, 0x0158, US_REG_DWORD},
    {"pxp_b3d01f0_devsts", 3, 1, 0, 0x009a, US_REG_WORD},
    {"pxp_b3d01f0_errsid", 3, 1, 0, 0x017c, US_REG_DWORD},
    {"pxp_b3d01f0_ler_ctrlsts", 3, 1, 0, 0x0288, US_REG_DWORD},
    {"pxp_b3d01f0_lnerrsts", 3, 1, 0, 0x0258, US_REG_DWORD},
    {"pxp_b3d01f0_lnksts", 3, 1, 0, 0x00a2, US_REG_WORD},
    {"pxp_b3d01f0_lnksts2", 3, 1, 0, 0x00c2, US_REG_WORD},
    {"pxp_b3d01f0_miscctrlsts_0", 3, 1, 0, 0x0188, US_REG_DWORD},
    {"pxp_b3d01f0_miscctrlsts_1", 3, 1, 0, 0x018c, US_REG_DWORD},
    {"pxp_b3d01f0_pcists", 3, 1, 0, 0x0006, US_REG_WORD},
    {"pxp_b3d01f0_rperrsts", 3, 1, 0, 0x0178, US_REG_DWORD},
    {"pxp_b3d01f0_rppioerr_cap", 3, 1, 0, 0x0298, US_REG_DWORD},
    {"pxp_b3d01f0_rppioerr_status", 3, 1, 0, 0x02a4, US_REG_DWORD},
    {"pxp_b3d01f0_secsts", 3, 1, 0, 0x001e, US_REG_WORD},
    {"pxp_b3d01f0_sltsts", 3, 1, 0, 0x00aa, US_REG_WORD},
    {"pxp_b3d01f0_uncerrsts", 3, 1, 0, 0x014c, US_REG_DWORD},
    {"pxp_b3d01f0_xpcorerrsts", 3, 1, 0, 0x0200, US_REG_DWORD},
    {"pxp_b3d01f0_xpglberrptr", 3, 1, 0, 0x0232, US_REG_WORD},
    {"pxp_b3d01f0_xpglberrsts", 3, 1, 0, 0x0230, US_REG_WORD},
    {"pxp_b3d01f0_xpuncerrsts", 3, 1, 0, 0x0208, US_REG_DWORD},
    {"pxp_b3d02f0_corerrsts", 3, 2, 0, 0x0158, US_REG_DWORD},
    {"pxp_b3d02f0_devsts", 3, 2, 0, 0x009a, US_REG_WORD},
    {"pxp_b3d02f0_errsid", 3, 2, 0, 0x017c, US_REG_DWORD},
    {"pxp_b3d02f0_ler_ctrlsts", 3, 2, 0, 0x0288, US_REG_DWORD},
    {"pxp_b3d02f0_lnerrsts", 3, 2, 0, 0x0258, US_REG_DWORD},
    {"pxp_b3d02f0_lnksts", 3, 2, 0, 0x00a2, US_REG_WORD},
    {"pxp_b3d02f0_lnksts2", 3, 2, 0, 0x00c2, US_REG_WORD},
    {"pxp_b3d02f0_miscctrlsts_0", 3, 2, 0, 0x0188, US_REG_DWORD},
    {"pxp_b3d02f0_miscctrlsts_1", 3, 2, 0, 0x018c, US_REG_DWORD},
    {"pxp_b3d02f0_pcists", 3, 2, 0, 0x0006, US_REG_WORD},
    {"pxp_b3d02f0_rperrsts", 3, 2, 0, 0x0178, US_REG_DWORD},
    {"pxp_b3d02f0_rppioerr_cap", 3, 2, 0, 0x0298, US_REG_DWORD},
    {"pxp_b3d02f0_rppioerr_status", 3, 2, 0, 0x02a4, US_REG_DWORD},
    {"pxp_b3d02f0_secsts", 3, 2, 0, 0x001e, US_REG_WORD},
    {"pxp_b3d02f0_sltsts", 3, 2, 0, 0x00aa, US_REG_WORD},
    {"pxp_b3d02f0_uncerrsts", 3, 2, 0, 0x014c, US_REG_DWORD},
    {"pxp_b3d02f0_xpcorerrsts", 3, 2, 0, 0x0200, US_REG_DWORD},
    {"pxp_b3d02f0_xpglberrptr", 3, 2, 0, 0x0232, US_REG_WORD},
    {"pxp_b3d02f0_xpglberrsts", 3, 2, 0, 0x0230, US_REG_WORD},
    {"pxp_b3d02f0_xpuncerrsts", 3, 2, 0, 0x0208, US_REG_DWORD},
    {"pxp_b3d03f0_corerrsts", 3, 3, 0, 0x0158, US_REG_DWORD},
    {"pxp_b3d03f0_devsts", 3, 3, 0, 0x009a, US_REG_WORD},
    {"pxp_b3d03f0_errsid", 3, 3, 0, 0x017c, US_REG_DWORD},
    {"pxp_b3d03f0_ler_ctrlsts", 3, 3, 0, 0x0288, US_REG_DWORD},
    {"pxp_b3d03f0_lnerrsts", 3, 3, 0, 0x0258, US_REG_DWORD},
    {"pxp_b3d03f0_lnksts", 3, 3, 0, 0x00a2, US_REG_WORD},
    {"pxp_b3d03f0_lnksts2", 3, 3, 0, 0x00c2, US_REG_WORD},
    {"pxp_b3d03f0_miscctrlsts_0", 3, 3, 0, 0x0188, US_REG_DWORD},
    {"pxp_b3d03f0_miscctrlsts_1", 3, 3, 0, 0x018c, US_REG_DWORD},
    {"pxp_b3d03f0_pcists", 3, 3, 0, 0x0006, US_REG_WORD},
    {"pxp_b3d03f0_rperrsts", 3, 3, 0, 0x0178, US_REG_DWORD},
    {"pxp_b3d03f0_rppioerr_cap", 3, 3, 0, 0x0298, US_REG_DWORD},
    {"pxp_b3d03f0_rppioerr_status", 3, 3, 0, 0x02a4, US_REG_DWORD},
    {"pxp_b3d03f0_secsts", 3, 3, 0, 0x001e, US_REG_WORD},
    {"pxp_b3d03f0_sltsts", 3, 3, 0, 0x00aa, US_REG_WORD},
    {"pxp_b3d03f0_uncerrsts", 3, 3, 0, 0x014c, US_REG_DWORD},
    {"pxp_b3d03f0_xpcorerrsts", 3, 3, 0, 0x0200, US_REG_DWORD},
    {"pxp_b3d03f0_xpglberrptr", 3, 3, 0, 0x0232, US_REG_WORD},
    {"pxp_b3d03f0_xpglberrsts", 3, 3, 0, 0x0230, US_REG_WORD},
    {"pxp_b3d03f0_xpuncerrsts", 3, 3, 0, 0x0208, US_REG_DWORD},
    {"pxp_b3d07f0_devsts", 3, 7, 0, 0x004a, US_REG_WORD},
    {"pxp_b3d07f0_lnksts", 3, 7, 0, 0x0052, US_REG_WORD},
    {"pxp_b3d07f0_lnksts2", 3, 7, 0, 0x0072, US_REG_WORD},
    {"pxp_b3d07f0_pcists", 3, 7, 0, 0x0006, US_REG_WORD},
    {"pxp_b3d07f0_sltsts2", 3, 7, 0, 0x007a, US_REG_WORD},
    {"pxp_b3d07f1_devsts", 3, 7, 1, 0x004a, US_REG_WORD},
    {"pxp_b3d07f1_lnksts", 3, 7, 1, 0x0052, US_REG_WORD},
    {"pxp_b3d07f1_lnksts2", 3, 7, 1, 0x0072, US_REG_WORD},
    {"pxp_b3d07f1_pcists", 3, 7, 1, 0x0006, US_REG_WORD},
    {"pxp_b3d07f1_sltsts2", 3, 7, 1, 0x007a, US_REG_WORD},
    {"pxp_b3d07f2_devsts", 3, 7, 2, 0x004a, US_REG_WORD},
    {"pxp_b3d07f2_lnksts", 3, 7, 2, 0x0052, US_REG_WORD},
    {"pxp_b3d07f2_lnksts2", 3, 7, 2, 0x0072, US_REG_WORD},
    {"pxp_b3d07f2_pcists", 3, 7, 2, 0x0006, US_REG_WORD},
    {"pxp_b3d07f2_sltsts2", 3, 7, 2, 0x007a, US_REG_WORD},
    {"pxp_b3d07f3_devsts", 3, 7, 3, 0x004a, US_REG_WORD},
    {"pxp_b3d07f3_lnksts", 3, 7, 3, 0x0052, US_REG_WORD},
    {"pxp_b3d07f3_lnksts2", 3, 7, 3, 0x0072, US_REG_WORD},
    {"pxp_b3d07f3_pcists", 3, 7, 3, 0x0006, US_REG_WORD},
    {"pxp_b3d07f3_sltsts2", 3, 7, 3, 0x007a, US_REG_WORD},
    {"pxp_b3d07f4_devsts", 3, 7, 4, 0x004a, US_REG_WORD},
    {"pxp_b3d07f4_lnksts", 3, 7, 4, 0x0052, US_REG_WORD},
    {"pxp_b3d07f4_lnksts2", 3, 7, 4, 0x0072, US_REG_WORD},
    {"pxp_b3d07f4_pcists", 3, 7, 4, 0x0006, US_REG_WORD},
    {"pxp_b3d07f4_sltsts2", 3, 7, 4, 0x007a, US_REG_WORD},
    {"pxp_b3d07f7_devsts", 3, 7, 7, 0x004a, US_REG_WORD},
    {"pxp_b3d07f7_lnksts", 3, 7, 7, 0x0052, US_REG_WORD},
    {"pxp_b3d07f7_lnksts2", 3, 7, 7, 0x0072, US_REG_WORD},
    {"pxp_b3d07f7_pcists", 3, 7, 7, 0x0006, US_REG_WORD},
    {"pxp_b3d07f7_sltsts2", 3, 7, 7, 0x007a, US_REG_WORD},
    {"pxp_b3d07f7_tswdbgerrstdis0", 3, 7, 7, 0x0358, US_REG_DWORD},
    {"pxp_b4d07f7_tswdbgerrstdis0", 4, 7, 7, 0x0358, US_REG_DWORD},
    {"pxp_dmi_corerrsts", 0, 0, 0, 0x0158, US_REG_DWORD},
    {"pxp_dmi_devsts", 0, 0, 0, 0x00f2, US_REG_WORD},
    {"pxp_dmi_errsid", 0, 0, 0, 0x017c, US_REG_DWORD},
    {"pxp_dmi_ler_ctrlsts", 0, 0, 0, 0x0288, US_REG_DWORD},
    {"pxp_dmi_lnerrsts", 0, 0, 0, 0x0258, US_REG_DWORD},
    {"pxp_dmi_lnksts", 0, 0, 0, 0x01b2, US_REG_WORD},
    {"pxp_dmi_lnksts2", 0, 0, 0, 0x01c2, US_REG_WORD},
    {"pxp_dmi_miscctrlsts_0", 0, 0, 0, 0x0188, US_REG_DWORD},
    {"pxp_dmi_miscctrlsts_1", 0, 0, 0, 0x018c, US_REG_DWORD},
    {"pxp_dmi_pcists", 0, 0, 0, 0x0006, US_REG_WORD},
    {"pxp_dmi_rperrsts", 0, 0, 0, 0x0178, US_REG_DWORD},
    {"pxp_dmi_rppioerr_cap", 0, 0, 0, 0x0298, US_REG_DWORD},
    {"pxp_dmi_rppioerr_status", 0, 0, 0, 0x02a4, US_REG_DWORD},
    {"pxp_dmi_sltsts", 0, 0, 0, 0x00aa, US_REG_WORD},
    {"pxp_dmi_uncerrsts", 0, 0, 0, 0x014c, US_REG_DWORD},
    {"pxp_dmi_xpcorerrsts", 0, 0, 0, 0x0200, US_REG_DWORD},
    {"pxp_dmi_xpglberrptr", 0, 0, 0, 0x0232, US_REG_WORD},
    {"pxp_dmi_xpglberrsts", 0, 0, 0, 0x0230, US_REG_WORD},
    {"pxp_dmi_xpuncerrsts", 0, 0, 0, 0x0208, US_REG_DWORD},
    {"pcu_cr_mca_err_src_log", 1, 30, 2, 0x00ec, US_REG_DWORD},
    {"pcu_cr_capid6_cfg", 1, 30, 3, 0x009c, US_REG_DWORD},
    {"pcu_cr_smb_status_cfg_0", 1, 30, 5, 0x00a8, US_REG_DWORD},
    {"pcu_cr_smb_status_cfg_1", 1, 30, 5, 0x00ac, US_REG_DWORD},
    {"pcu_cr_smb_status_cfg_2", 1, 30, 5, 0x00b0, US_REG_DWORD},
    {"ubox_ncevents_emca_core_csmi_log_cfg_b0d08f0", 0, 8, 0, 0x00b0,
     US_REG_DWORD},
    {"ubox_ncevents_emca_core_msmi_log_cfg_b0d08f0", 0, 8, 0, 0x00b8,
     US_REG_DWORD},
    {"ubox_ncevents_emca_en_core_ierr_to_msmi_cfg_b0d08f0", 0, 8, 0, 0x00d8,
     US_REG_DWORD},
    {"ubox_ncevents_ierrloggingreg_b0d08f0", 0, 8, 0, 0x00a4, US_REG_DWORD},
    {"ubox_ncevents_iio_errpinctl_cfg_b0d08f0", 0, 8, 0, 0x020c, US_REG_DWORD},
    {"ubox_ncevents_iio_errpindat_cfg_b0d08f0", 0, 8, 0, 0x0214, US_REG_DWORD},
    {"ubox_ncevents_iio_errpinsts_cfg_b0d08f0", 0, 8, 0, 0x0210, US_REG_DWORD},
    {"ubox_ncevents_iio_gc_1st_errst_cfg_b0d08f0", 0, 8, 0, 0x0234,
     US_REG_DWORD},
    {"ubox_ncevents_iio_gc_errst_cfg_b0d08f0", 0, 8, 0, 0x0230, US_REG_DWORD},
    {"ubox_ncevents_iio_gc_nxt_errst_cfg_b0d08f0", 0, 8, 0, 0x0238,
     US_REG_DWORD},
    {"ubox_ncevents_iio_gf_1st_errst_cfg_b0d08f0", 0, 8, 0, 0x021c,
     US_REG_DWORD},
    {"ubox_ncevents_iio_gf_errst_cfg_b0d08f0", 0, 8, 0, 0x0218, US_REG_DWORD},
    {"ubox_ncevents_iio_gf_nxt_errst_cfg_b0d08f0", 0, 8, 0, 0x0220,
     US_REG_DWORD},
    {"ubox_ncevents_iio_gnf_1st_errst_cfg_b0d08f0", 0, 8, 0, 0x0228,
     US_REG_DWORD},
    {"ubox_ncevents_iio_gnf_errst_cfg_b0d08f0", 0, 8, 0, 0x0224, US_REG_DWORD},
    {"ubox_ncevents_iio_gnf_nxt_errst_cfg_b0d08f0", 0, 8, 0, 0x022C,
     US_REG_DWORD},
    {"ubox_ncevents_iio_gsysctl_cfg_b0d08f0", 0, 8, 0, 0x0208, US_REG_DWORD},
    {"ubox_ncevents_iio_gsysst_cfg_b0d08f0", 0, 8, 0, 0x0204, US_REG_DWORD},
    {"ubox_ncevents_mcerrloggingreg_b0d08f0", 0, 8, 0, 0x00a8, US_REG_DWORD},
    {"ubox_ncevents_ubox_cts_err_mask_cfg_b0d08f0", 0, 8, 0, 0x0094,
     US_REG_DWORD},
    {"ubox_ncevents_ubox_gl_err_cfg_cfg_b0d08f0", 0, 8, 0, 0x0090,
     US_REG_DWORD},
    {"ubox_ncevents_uboxerrctl_cfg_b0d08f0", 0, 8, 0, 0x00e8, US_REG_DWORD},
    {"ubox_ncevents_uboxerrmisc_cfg_b0d08f0", 0, 8, 0, 0x00ec, US_REG_DWORD},
    {"ubox_ncevents_uboxerrmisc2_cfg_b0d08f0", 0, 8, 0, 0x00f0, US_REG_DWORD},
    {"ubox_ncevents_uboxerrsts_cfg_b0d08f0", 0, 8, 0, 0x00e4, US_REG_DWORD},
    {"iio_irpp0errcnt_b0d05f2", 0, 5, 2, 0x026c, US_REG_DWORD},
    {"iio_irpp0errcnt_b1d05f2", 1, 5, 2, 0x026c, US_REG_DWORD},
    {"iio_irpp0errcnt_b2d05f2", 2, 5, 2, 0x026c, US_REG_DWORD},
    {"iio_irpp0errcnt_b3d05f2", 3, 5, 2, 0x026c, US_REG_DWORD},
    {"iio_irpp0errcnt_b4d05f2", 4, 5, 2, 0x026c, US_REG_DWORD},
    {"iio_irpp0errst_b0d05f2", 0, 5, 2, 0x023c, US_REG_DWORD},
    {"iio_irpp0errst_b1d05f2", 1, 5, 2, 0x023c, US_REG_DWORD},
    {"iio_irpp0errst_b2d05f2", 2, 5, 2, 0x023c, US_REG_DWORD},
    {"iio_irpp0errst_b3d05f2", 3, 5, 2, 0x023c, US_REG_DWORD},
    {"iio_irpp0errst_b4d05f2", 4, 5, 2, 0x023c, US_REG_DWORD},
    {"iio_irpp0fferrst_b0d05f2", 0, 5, 2, 0x0238, US_REG_DWORD},
    {"iio_irpp0fferrst_b1d05f2", 1, 5, 2, 0x0238, US_REG_DWORD},
    {"iio_irpp0fferrst_b2d05f2", 2, 5, 2, 0x0238, US_REG_DWORD},
    {"iio_irpp0fferrst_b3d05f2", 3, 5, 2, 0x0238, US_REG_DWORD},
    {"iio_irpp0fferrst_b4d05f2", 4, 5, 2, 0x0238, US_REG_DWORD},
    {"iio_irpp0fnerrst_b0d05f2", 0, 5, 2, 0x0230, US_REG_DWORD},
    {"iio_irpp0fnerrst_b1d05f2", 1, 5, 2, 0x0230, US_REG_DWORD},
    {"iio_irpp0fnerrst_b2d05f2", 2, 5, 2, 0x0230, US_REG_DWORD},
    {"iio_irpp0fnerrst_b3d05f2", 3, 5, 2, 0x0230, US_REG_DWORD},
    {"iio_irpringerrst_b1d05f2", 1, 5, 2, 0x0208, US_REG_DWORD},
    {"iio_irpringerrst_b2d05f2", 2, 5, 2, 0x0208, US_REG_DWORD},
    {"iio_irpringerrst_b3d05f2", 3, 5, 2, 0x0208, US_REG_DWORD},
    {"iio_irpringfferrst_b1d05f2", 1, 5, 2, 0x0210, US_REG_DWORD},
    {"iio_irpringfferrst_b2d05f2", 2, 5, 2, 0x0210, US_REG_DWORD},
    {"iio_irpringfferrst_b3d05f2", 3, 5, 2, 0x0210, US_REG_DWORD},
    {"iio_irpringfnerrst_b1d05f2", 1, 5, 2, 0x0214, US_REG_DWORD},
    {"iio_irpringfnerrst_b2d05f2", 2, 5, 2, 0x0214, US_REG_DWORD},
    {"iio_irpringfnerrst_b3d05f2", 3, 5, 2, 0x0214, US_REG_DWORD},
    {"iio_irpringnferrst_b1d05f2", 1, 5, 2, 0x0218, US_REG_DWORD},
    {"iio_irpringnferrst_b2d05f2", 2, 5, 2, 0x0218, US_REG_DWORD},
    {"iio_irpringnferrst_b3d05f2", 3, 5, 2, 0x0218, US_REG_DWORD},
    {"iio_itcfferrst_b0d05f2", 0, 5, 2, 0x02A0, US_REG_DWORD},
    {"iio_itcfferrst_b1d05f2", 1, 5, 2, 0x02A0, US_REG_DWORD},
    {"iio_itcfferrst_b2d05f2", 2, 5, 2, 0x02A0, US_REG_DWORD},
    {"iio_itcfferrst_b3d05f2", 3, 5, 2, 0x02A0, US_REG_DWORD},
    {"iio_itcfferrst_b4d05f2", 4, 5, 2, 0x02A0, US_REG_DWORD},
    {"iio_itcnnerrst_b0d05f2", 0, 5, 2, 0x02cc, US_REG_DWORD},
    {"iio_itcnnerrst_b1d05f2", 1, 5, 2, 0x02cc, US_REG_DWORD},
    {"iio_itcnnerrst_b2d05f2", 2, 5, 2, 0x02cc, US_REG_DWORD},
    {"iio_itcnnerrst_b3d05f2", 3, 5, 2, 0x02cc, US_REG_DWORD},
    {"iio_itcnnerrst_b4d05f2", 4, 5, 2, 0x02cc, US_REG_DWORD},
    {"iio_lcferrst_b0d05f2", 0, 5, 2, 0x01ac, US_REG_DWORD},
    {"iio_lcferrst_b1d05f2", 1, 5, 2, 0x01ac, US_REG_DWORD},
    {"iio_lcferrst_b2d05f2", 2, 5, 2, 0x01ac, US_REG_DWORD},
    {"iio_lcferrst_b3d05f2", 3, 5, 2, 0x01ac, US_REG_DWORD},
    {"iio_lferrst_b0d05f2", 0, 5, 2, 0x01c4, US_REG_DWORD},
    {"iio_lferrst_b1d05f2", 1, 5, 2, 0x01c4, US_REG_DWORD},
    {"iio_lferrst_b2d05f2", 2, 5, 2, 0x01c4, US_REG_DWORD},
    {"iio_lferrst_b3d05f2", 3, 5, 2, 0x01c4, US_REG_DWORD},
    {"iio_lfferrst_b0d05f2", 0, 5, 2, 0x01dc, US_REG_DWORD},
    {"iio_lfferrst_b1d05f2", 1, 5, 2, 0x01dc, US_REG_DWORD},
    {"iio_lfferrst_b2d05f2", 2, 5, 2, 0x01dc, US_REG_DWORD},
    {"iio_lfferrst_b3d05f2", 3, 5, 2, 0x01dc, US_REG_DWORD},
    {"iio_lnferrst_b0d05f2", 0, 5, 2, 0x01ec, US_REG_DWORD},
    {"iio_lnferrst_b1d05f2", 1, 5, 2, 0x01ec, US_REG_DWORD},
    {"iio_lnferrst_b2d05f2", 2, 5, 2, 0x01ec, US_REG_DWORD},
    {"iio_lnferrst_b3d05f2", 3, 5, 2, 0x01ec, US_REG_DWORD},
    {"iio_otcfferrst_b0d05f2", 0, 5, 2, 0x02D0, US_REG_DWORD},
    {"iio_otcfferrst_b1d05f2", 1, 5, 2, 0x02D0, US_REG_DWORD},
    {"iio_otcfferrst_b2d05f2", 2, 5, 2, 0x02D0, US_REG_DWORD},
    {"iio_otcfferrst_b3d05f2", 3, 5, 2, 0x02D0, US_REG_DWORD},
    {"iio_otcfferrst_b4d05f2", 4, 5, 2, 0x02D0, US_REG_DWORD},
    {"iio_tcerrst_b0d05f2", 0, 5, 2, 0x0288, US_REG_DWORD},
    {"iio_tcerrst_b1d05f2", 1, 5, 2, 0x0288, US_REG_DWORD},
    {"iio_tcerrst_b2d05f2", 2, 5, 2, 0x0288, US_REG_DWORD},
    {"iio_tcerrst_b3d05f2", 3, 5, 2, 0x0288, US_REG_DWORD},
    {"iio_tcerrst_b4d05f2", 4, 5, 2, 0x0288, US_REG_DWORD},
    {"iio_vtuncerrptr_b0d05f0", 0, 5, 0, 0x01b4, US_REG_DWORD},
    {"iio_vtuncerrptr_b1d05f0", 1, 5, 0, 0x01b4, US_REG_DWORD},
    {"iio_vtuncerrptr_b2d05f0", 2, 5, 0, 0x01b4, US_REG_DWORD},
    {"iio_vtuncerrptr_b3d05f0", 3, 5, 0, 0x01b4, US_REG_DWORD},
    {"iio_vtuncerrsts_b0d05f0", 0, 5, 0, 0x01a8, US_REG_DWORD},
    {"iio_vtuncerrsts_b1d05f0", 1, 5, 0, 0x01a8, US_REG_DWORD},
    {"iio_vtuncerrsts_b2d05f0", 2, 5, 0, 0x01a8, US_REG_DWORD},
    {"iio_vtuncerrsts_b3d05f0", 3, 5, 0, 0x01a8, US_REG_DWORD},
    {"kti0_bios_err_ad", 3, 14, 0, 0x00b8, US_REG_DWORD},
    {"kti0_bios_err_misc", 3, 14, 0, 0x00b0, US_REG_DWORD},
    {"kti0_bios_err_st", 3, 14, 0, 0x00a8, US_REG_DWORD},
    {"kti0_crcerrcnt", 3, 14, 0, 0x00c0, US_REG_DWORD},
    {"kti0_dbgerrst0", 3, 14, 2, 0x03a0, US_REG_DWORD},
    {"kti0_dbgerrst1", 3, 14, 2, 0x03a8, US_REG_DWORD},
    {"kti0_dbgerrstdis0", 3, 14, 2, 0x03a4, US_REG_DWORD},
    {"kti0_dbgerrstdis1", 3, 14, 2, 0x03ac, US_REG_DWORD},
    {"kti0_dfx_lck_ctl_cfg", 3, 14, 2, 0x0800, US_REG_DWORD},
    {"kti0_errcnt0_cntr", 3, 14, 0, 0x00d4, US_REG_DWORD},
    {"kti0_errcnt1_cntr", 3, 14, 0, 0x00dc, US_REG_DWORD},
    {"kti0_errcnt2_cntr", 3, 14, 0, 0x00e4, US_REG_DWORD},
    {"kti0_lcl", 3, 14, 0, 0x0084, US_REG_DWORD},
    {"kti0_les", 3, 14, 0, 0x0090, US_REG_DWORD},
    {"kti0_ls", 3, 14, 0, 0x008c, US_REG_DWORD},
    {"kti0_ph_rxl0c_err_log0", 3, 14, 2, 0x09f4, US_REG_DWORD},
    {"kti0_ph_rxl0c_err_log1", 3, 14, 2, 0x09f8, US_REG_DWORD},
    {"kti0_ph_rxl0c_err_sts", 3, 14, 2, 0x09f0, US_REG_DWORD},
    {"kti0_pq_csr_mcsr0", 3, 14, 2, 0x0820, US_REG_DWORD},
    {"kti0_pq_csr_mcsr1", 3, 14, 2, 0x0824, US_REG_DWORD},
    {"kti0_pq_csr_mcsr3", 3, 14, 2, 0x082c, US_REG_DWORD},
    {"kti0_reut_ph_cls", 3, 14, 1, 0x0160, US_REG_DWORD},
    {"kti0_reut_ph_ctr1", 3, 14, 1, 0x012c, US_REG_DWORD},
    {"kti0_reut_ph_ctr2", 3, 14, 1, 0x0130, US_REG_DWORD},
    {"kti0_reut_ph_das", 3, 14, 1, 0x0170, US_REG_DWORD},
    {"kti0_reut_ph_kes", 3, 14, 1, 0x016c, US_REG_DWORD},
    {"kti0_reut_ph_ldc", 3, 14, 1, 0x0178, US_REG_DWORD},
    {"kti0_reut_ph_pis", 3, 14, 1, 0x014c, US_REG_DWORD},
    {"kti0_reut_ph_pls", 3, 14, 1, 0x0164, US_REG_DWORD},
    {"kti0_reut_ph_pss", 3, 14, 1, 0x0154, US_REG_DWORD},
    {"kti0_reut_ph_rds", 3, 14, 1, 0x0140, US_REG_DWORD},
    {"kti0_reut_ph_tds", 3, 14, 1, 0x013c, US_REG_DWORD},
    {"kti0_reut_ph_ttlecr", 3, 14, 1, 0x01b4, US_REG_DWORD},
    {"kti0_reut_ph_ttres", 3, 14, 1, 0x01c0, US_REG_DWORD},
    {"kti0_reut_ph_wes", 3, 14, 1, 0x0168, US_REG_DWORD},
    {"kti1_bios_err_ad", 3, 15, 0, 0x00b8, US_REG_DWORD},
    {"kti1_bios_err_misc", 3, 15, 0, 0x00b0, US_REG_DWORD},
    {"kti1_bios_err_st", 3, 15, 0, 0x00a8, US_REG_DWORD},
    {"kti1_crcerrcnt", 3, 15, 0, 0x00c0, US_REG_DWORD},
    {"kti1_dbgerrst0", 3, 15, 2, 0x03a0, US_REG_DWORD},
    {"kti1_dbgerrst1", 3, 15, 2, 0x03a8, US_REG_DWORD},
    {"kti1_dbgerrstdis0", 3, 15, 2, 0x03a4, US_REG_DWORD},
    {"kti1_dbgerrstdis1", 3, 15, 2, 0x03ac, US_REG_DWORD},
    {"kti1_dfx_lck_ctl_cfg", 3, 15, 2, 0x0800, US_REG_DWORD},
    {"kti1_errcnt0_cntr", 3, 15, 0, 0x00d4, US_REG_DWORD},
    {"kti1_errcnt1_cntr", 3, 15, 0, 0x00dc, US_REG_DWORD},
    {"kti1_errcnt2_cntr", 3, 15, 0, 0x00e4, US_REG_DWORD},
    {"kti1_lcl", 3, 15, 0, 0x0084, US_REG_DWORD},
    {"kti1_les", 3, 15, 0, 0x0090, US_REG_DWORD},
    {"kti1_ls", 3, 15, 0, 0x008c, US_REG_DWORD},
    {"kti1_ph_rxl0c_err_log0", 3, 15, 2, 0x09f4, US_REG_DWORD},
    {"kti1_ph_rxl0c_err_log1", 3, 15, 2, 0x09f8, US_REG_DWORD},
    {"kti1_ph_rxl0c_err_sts", 3, 15, 2, 0x09f0, US_REG_DWORD},
    {"kti1_pq_csr_mcsr0", 3, 15, 2, 0x0820, US_REG_DWORD},
    {"kti1_pq_csr_mcsr1", 3, 15, 2, 0x0824, US_REG_DWORD},
    {"kti1_pq_csr_mcsr3", 3, 15, 2, 0x082c, US_REG_DWORD},
    {"kti1_reut_ph_cls", 3, 15, 1, 0x0160, US_REG_DWORD},
    {"kti1_reut_ph_ctr1", 3, 15, 1, 0x012c, US_REG_DWORD},
    {"kti1_reut_ph_ctr2", 3, 15, 1, 0x0130, US_REG_DWORD},
    {"kti1_reut_ph_das", 3, 15, 1, 0x0170, US_REG_DWORD},
    {"kti1_reut_ph_kes", 3, 15, 1, 0x016c, US_REG_DWORD},
    {"kti1_reut_ph_ldc", 3, 15, 1, 0x0178, US_REG_DWORD},
    {"kti1_reut_ph_pis", 3, 15, 1, 0x014c, US_REG_DWORD},
    {"kti1_reut_ph_pls", 3, 15, 1, 0x0164, US_REG_DWORD},
    {"kti1_reut_ph_pss", 3, 15, 1, 0x0154, US_REG_DWORD},
    {"kti1_reut_ph_rds", 3, 15, 1, 0x0140, US_REG_DWORD},
    {"kti1_reut_ph_tds", 3, 15, 1, 0x013c, US_REG_DWORD},
    {"kti1_reut_ph_ttlecr", 3, 15, 1, 0x01b4, US_REG_DWORD},
    {"kti1_reut_ph_ttres", 3, 15, 1, 0x01c0, US_REG_DWORD},
    {"kti1_reut_ph_wes", 3, 15, 1, 0x0168, US_REG_DWORD},
    {"kti2_bios_err_ad", 3, 16, 0, 0x00b8, US_REG_DWORD},
    {"kti2_bios_err_misc", 3, 16, 0, 0x00b0, US_REG_DWORD},
    {"kti2_bios_err_st", 3, 16, 0, 0x00a8, US_REG_DWORD},
    {"kti2_crcerrcnt", 3, 16, 0, 0x00c0, US_REG_DWORD},
    {"kti2_dbgerrst0", 3, 16, 2, 0x03a0, US_REG_DWORD},
    {"kti2_dbgerrst1", 3, 16, 2, 0x03a8, US_REG_DWORD},
    {"kti2_dbgerrstdis0", 3, 16, 2, 0x03a4, US_REG_DWORD},
    {"kti2_dbgerrstdis1", 3, 16, 2, 0x03ac, US_REG_DWORD},
    {"kti2_dfx_lck_ctl_cfg", 3, 16, 2, 0x0800, US_REG_DWORD},
    {"kti2_errcnt0_cntr", 3, 16, 0, 0x00d4, US_REG_DWORD},
    {"kti2_errcnt1_cntr", 3, 16, 0, 0x00dc, US_REG_DWORD},
    {"kti2_errcnt2_cntr", 3, 16, 0, 0x00e4, US_REG_DWORD},
    {"kti2_lcl", 3, 16, 0, 0x0084, US_REG_DWORD},
    {"kti2_les", 3, 16, 0, 0x0090, US_REG_DWORD},
    {"kti2_ls", 3, 16, 0, 0x008c, US_REG_DWORD},
    {"kti2_ph_rxl0c_err_log0", 3, 16, 2, 0x09f4, US_REG_DWORD},
    {"kti2_ph_rxl0c_err_log1", 3, 16, 2, 0x09f8, US_REG_DWORD},
    {"kti2_ph_rxl0c_err_sts", 3, 16, 2, 0x09f0, US_REG_DWORD},
    {"kti2_pq_csr_mcsr0", 3, 16, 2, 0x0820, US_REG_DWORD},
    {"kti2_pq_csr_mcsr1", 3, 16, 2, 0x0824, US_REG_DWORD},
    {"kti2_pq_csr_mcsr3", 3, 16, 2, 0x082c, US_REG_DWORD},
    {"kti2_reut_ph_cls", 3, 16, 1, 0x0160, US_REG_DWORD},
    {"kti2_reut_ph_ctr1", 3, 16, 1, 0x012c, US_REG_DWORD},
    {"kti2_reut_ph_ctr2", 3, 16, 1, 0x0130, US_REG_DWORD},
    {"kti2_reut_ph_das", 3, 16, 1, 0x0170, US_REG_DWORD},
    {"kti2_reut_ph_kes", 3, 16, 1, 0x016c, US_REG_DWORD},
    {"kti2_reut_ph_ldc", 3, 16, 1, 0x0178, US_REG_DWORD},
    {"kti2_reut_ph_pis", 3, 16, 1, 0x014c, US_REG_DWORD},
    {"kti2_reut_ph_pls", 3, 16, 1, 0x0164, US_REG_DWORD},
    {"kti2_reut_ph_pss", 3, 16, 1, 0x0154, US_REG_DWORD},
    {"kti2_reut_ph_rds", 3, 16, 1, 0x0140, US_REG_DWORD},
    {"kti2_reut_ph_tds", 3, 16, 1, 0x013c, US_REG_DWORD},
    {"kti2_reut_ph_ttlecr", 3, 16, 1, 0x01b4, US_REG_DWORD},
    {"kti2_reut_ph_ttres", 3, 16, 1, 0x01c0, US_REG_DWORD},
    {"kti2_reut_ph_wes", 3, 16, 1, 0x0168, US_REG_DWORD},
    {"m3kti0_m3egrerrlog0", 3, 18, 0, 0x0120, US_REG_DWORD},
    {"m3kti0_m3egrerrlog1", 3, 18, 0, 0x0124, US_REG_DWORD},
    {"m3kti0_m3egrerrlog2", 3, 18, 0, 0x0128, US_REG_DWORD},
    {"m3kti0_m3egrerrlog3", 3, 18, 0, 0x012c, US_REG_DWORD},
    {"m3kti0_m3egrerrlog4", 3, 18, 0, 0x0130, US_REG_DWORD},
    {"m3kti0_m3egrerrlog5", 3, 18, 0, 0x0134, US_REG_DWORD},
    {"m3kti0_m3egrerrlog6", 3, 18, 0, 0x0138, US_REG_DWORD},
    {"m3kti0_m3egrerrlog7", 3, 18, 0, 0x013c, US_REG_DWORD},
    {"m3kti0_m3egrerrmsk0123", 3, 18, 0, 0x0118, US_REG_DWORD},
    {"m3kti0_m3egrerrmsk4567", 3, 18, 0, 0x011c, US_REG_DWORD},
    {"m3kti0_m3egrerrmsk8", 3, 18, 0, 0x0148, US_REG_DWORD},
    {"m3kti0_m3ingerrlog0", 3, 18, 0, 0x0160, US_REG_DWORD},
    {"m3kti0_m3ingerrlog1", 3, 18, 0, 0x0164, US_REG_DWORD},
    {"m3kti0_m3ingerrlog2", 3, 18, 0, 0x0168, US_REG_DWORD},
    {"m3kti0_m3ingerrlog3", 3, 18, 0, 0x016c, US_REG_DWORD},
    {"m3kti0_m3ingerrmask0", 3, 18, 0, 0x0158, US_REG_DWORD},
    {"m3kti0_m3ingerrmask1", 3, 18, 0, 0x015c, US_REG_DWORD},
    {"m3kti1_m3egrerrlog0", 3, 18, 4, 0x0120, US_REG_DWORD},
    {"m3kti1_m3egrerrlog1", 3, 18, 4, 0x0124, US_REG_DWORD},
    {"m3kti1_m3egrerrlog2", 3, 18, 4, 0x0128, US_REG_DWORD},
    {"m3kti1_m3egrerrlog3", 3, 18, 4, 0x012c, US_REG_DWORD},
    {"m3kti1_m3egrerrlog4", 3, 18, 4, 0x0130, US_REG_DWORD},
    {"m3kti1_m3egrerrlog5", 3, 18, 4, 0x0134, US_REG_DWORD},
    {"m3kti1_m3egrerrlog6", 3, 18, 4, 0x0138, US_REG_DWORD},
    {"m3kti1_m3egrerrlog7", 3, 18, 4, 0x013c, US_REG_DWORD},
    {"m3kti1_m3egrerrmsk0123", 3, 18, 4, 0x0118, US_REG_DWORD},
    {"m3kti1_m3egrerrmsk4567", 3, 18, 4, 0x011c, US_REG_DWORD},
    {"m3kti1_m3egrerrmsk8", 3, 18, 4, 0x0148, US_REG_DWORD},
    {"m3kti1_m3ingerrlog0", 3, 18, 4, 0x0160, US_REG_DWORD},
    {"m3kti1_m3ingerrlog1", 3, 18, 4, 0x0164, US_REG_DWORD},
    {"m3kti1_m3ingerrlog2", 3, 18, 4, 0x0168, US_REG_DWORD},
    {"m3kti1_m3ingerrlog3", 3, 18, 4, 0x016c, US_REG_DWORD},
    {"m3kti1_m3ingerrmask0", 3, 18, 4, 0x0158, US_REG_DWORD},
    {"m3kti1_m3ingerrmask1", 3, 18, 4, 0x015c, US_REG_DWORD},
    {"ntb_b1d00f0_corerrsts", 1, 0, 0, 0x0158, US_REG_DWORD},
    {"ntb_b1d00f0_devsts", 1, 0, 0, 0x009a, US_REG_WORD},
    {"ntb_b1d00f0_errsid", 1, 0, 0, 0x017c, US_REG_DWORD},
    {"ntb_b1d00f0_ler_ctrlsts", 1, 0, 0, 0x0288, US_REG_DWORD},
    {"ntb_b1d00f0_lnerrsts", 1, 0, 0, 0x0258, US_REG_DWORD},
    {"ntb_b1d00f0_lnksts", 1, 0, 0, 0x01a2, US_REG_WORD},
    {"ntb_b1d00f0_lnksts2", 1, 0, 0, 0x01c2, US_REG_WORD},
    {"ntb_b1d00f0_miscctrlsts_0", 1, 0, 0, 0x0188, US_REG_DWORD},
    {"ntb_b1d00f0_miscctrlsts_1", 1, 0, 0, 0x018c, US_REG_DWORD},
    {"ntb_b1d00f0_rperrsts", 1, 0, 0, 0x0178, US_REG_DWORD},
    {"ntb_b1d00f0_rppioerr_cap", 1, 0, 0, 0x0298, US_REG_DWORD},
    {"ntb_b1d00f0_rppioerr_status", 1, 0, 0, 0x02a4, US_REG_DWORD},
    {"ntb_b1d00f0_sltsts", 1, 0, 0, 0x01aa, US_REG_WORD},
    {"ntb_b1d00f0_uncerrsts", 1, 0, 0, 0x014c, US_REG_DWORD},
    {"ntb_b1d00f0_xpcorerrsts", 1, 0, 0, 0x0200, US_REG_DWORD},
    {"ntb_b1d00f0_xpglberrptr", 1, 0, 0, 0x0232, US_REG_WORD},
    {"ntb_b1d00f0_xpglberrsts", 1, 0, 0, 0x0230, US_REG_WORD},
    {"ntb_b1d00f0_xpuncerrsts", 1, 0, 0, 0x0208, US_REG_DWORD},
    {"ntb_b2d00f0_corerrsts", 2, 0, 0, 0x0158, US_REG_DWORD},
    {"ntb_b2d00f0_devsts", 2, 0, 0, 0x009a, US_REG_WORD},
    {"ntb_b2d00f0_errsid", 2, 0, 0, 0x017c, US_REG_DWORD},
    {"ntb_b2d00f0_ler_ctrlsts", 2, 0, 0, 0x0288, US_REG_DWORD},
    {"ntb_b2d00f0_lnerrsts", 2, 0, 0, 0x0258, US_REG_DWORD},
    {"ntb_b2d00f0_lnksts", 2, 0, 0, 0x01a2, US_REG_WORD},
    {"ntb_b2d00f0_lnksts2", 2, 0, 0, 0x01c2, US_REG_WORD},
    {"ntb_b2d00f0_miscctrlsts_0", 2, 0, 0, 0x0188, US_REG_DWORD},
    {"ntb_b2d00f0_miscctrlsts_1", 2, 0, 0, 0x018c, US_REG_DWORD},
    {"ntb_b2d00f0_rperrsts", 2, 0, 0, 0x0178, US_REG_DWORD},
    {"ntb_b2d00f0_rppioerr_cap", 2, 0, 0, 0x0298, US_REG_DWORD},
    {"ntb_b2d00f0_rppioerr_status", 2, 0, 0, 0x02a4, US_REG_DWORD},
    {"ntb_b2d00f0_sltsts", 2, 0, 0, 0x01aa, US_REG_WORD},
    {"ntb_b2d00f0_uncerrsts", 2, 0, 0, 0x014c, US_REG_DWORD},
    {"ntb_b2d00f0_xpcorerrsts", 2, 0, 0, 0x0200, US_REG_DWORD},
    {"ntb_b2d00f0_xpglberrptr", 2, 0, 0, 0x0232, US_REG_WORD},
    {"ntb_b2d00f0_xpglberrsts", 2, 0, 0, 0x0230, US_REG_WORD},
    {"ntb_b2d00f0_xpuncerrsts", 2, 0, 0, 0x0208, US_REG_DWORD},
    {"ntb_b3d00f0_corerrsts", 3, 0, 0, 0x0158, US_REG_DWORD},
    {"ntb_b3d00f0_devsts", 3, 0, 0, 0x009a, US_REG_WORD},
    {"ntb_b3d00f0_errsid", 3, 0, 0, 0x017c, US_REG_DWORD},
    {"ntb_b3d00f0_ler_ctrlsts", 3, 0, 0, 0x0288, US_REG_DWORD},
    {"ntb_b3d00f0_lnerrsts", 3, 0, 0, 0x0258, US_REG_DWORD},
    {"ntb_b3d00f0_lnksts", 3, 0, 0, 0x01a2, US_REG_WORD},
    {"ntb_b3d00f0_lnksts2", 3, 0, 0, 0x01c2, US_REG_WORD},
    {"ntb_b3d00f0_miscctrlsts_0", 3, 0, 0, 0x0188, US_REG_DWORD},
    {"ntb_b3d00f0_miscctrlsts_1", 3, 0, 0, 0x018c, US_REG_DWORD},
    {"ntb_b3d00f0_rperrsts", 3, 0, 0, 0x0178, US_REG_DWORD},
    {"ntb_b3d00f0_rppioerr_cap", 3, 0, 0, 0x0298, US_REG_DWORD},
    {"ntb_b3d00f0_rppioerr_status", 3, 0, 0, 0x02a4, US_REG_DWORD},
    {"ntb_b3d00f0_sltsts", 3, 0, 0, 0x01aa, US_REG_WORD},
    {"ntb_b3d00f0_uncerrsts", 3, 0, 0, 0x014c, US_REG_DWORD},
    {"ntb_b3d00f0_xpcorerrsts", 3, 0, 0, 0x0200, US_REG_DWORD},
    {"ntb_b3d00f0_xpglberrptr", 3, 0, 0, 0x0232, US_REG_WORD},
    {"ntb_b3d00f0_xpglberrsts", 3, 0, 0, 0x0230, US_REG_WORD},
    {"ntb_b3d00f0_xpuncerrsts", 3, 0, 0, 0x0208, US_REG_DWORD},
};

static const SUncoreStatusRegPciMmio sUncoreStatusPciMmio[] = {
    // Register, Bar, Bus, Dev, Func, Offset, SizeCode
    {"cb_bar0_chanerr",
     {{
         .lenCode = US_MMIO_DWORD,
         .bar = 0,
         .bus = 0,
         .rsvd = 0,
         .func = 0,
         .dev = 4,
         .reg = 0x00a8,
     }}},
    {"cb_bar1_chanerr",
     {{
         .lenCode = US_MMIO_DWORD,
         .bar = 0,
         .bus = 0,
         .rsvd = 0,
         .func = 1,
         .dev = 4,
         .reg = 0x00a8,
     }}},
    {"cb_bar2_chanerr",
     {{
         .lenCode = US_MMIO_DWORD,
         .bar = 0,
         .bus = 0,
         .rsvd = 0,
         .func = 2,
         .dev = 4,
         .reg = 0x00a8,
     }}},
    {"cb_bar3_chanerr",
     {{
         .lenCode = US_MMIO_DWORD,
         .bar = 0,
         .bus = 0,
         .rsvd = 0,
         .func = 3,
         .dev = 4,
         .reg = 0x00a8,
     }}},
    {"cb_bar4_chanerr",
     {{
         .lenCode = US_MMIO_DWORD,
         .bar = 0,
         .bus = 0,
         .rsvd = 0,
         .func = 4,
         .dev = 4,
         .reg = 0x00a8,
     }}},
    {"cb_bar5_chanerr",
     {{
         .lenCode = US_MMIO_DWORD,
         .bar = 0,
         .bus = 0,
         .rsvd = 0,
         .func = 5,
         .dev = 4,
         .reg = 0x00a8,
     }}},
    {"cb_bar6_chanerr",
     {{
         .lenCode = US_MMIO_DWORD,
         .bar = 0,
         .bus = 0,
         .rsvd = 0,
         .func = 6,
         .dev = 4,
         .reg = 0x00a8,
     }}},
    {"cb_bar7_chanerr",
     {{
         .lenCode = US_MMIO_DWORD,
         .bar = 0,
         .bus = 0,
         .rsvd = 0,
         .func = 7,
         .dev = 4,
         .reg = 0x00a8,
     }}},
    {"iio_vtd0_fltsts_b0d05f0",
     {{
         .lenCode = US_MMIO_DWORD,
         .bar = 0,
         .bus = 0,
         .rsvd = 0,
         .func = 0,
         .dev = 5,
         .reg = 0x0034,
     }}},
    {"iio_vtd0_fltsts_b1d05f0",
     {{
         .lenCode = US_MMIO_DWORD,
         .bar = 0,
         .bus = 1,
         .rsvd = 0,
         .func = 0,
         .dev = 5,
         .reg = 0x0034,
     }}},
    {"iio_vtd0_fltsts_b2d05f0",
     {{
         .lenCode = US_MMIO_DWORD,
         .bar = 0,
         .bus = 2,
         .rsvd = 0,
         .func = 0,
         .dev = 5,
         .reg = 0x0034,
     }}},
    {"iio_vtd0_fltsts_b3d05f0",
     {{
         .lenCode = US_MMIO_DWORD,
         .bar = 0,
         .bus = 3,
         .rsvd = 0,
         .func = 0,
         .dev = 5,
         .reg = 0x0034,
     }}},
    {"iio_vtd0_glbsts_b0d05f0",
     {{
         .lenCode = US_MMIO_DWORD,
         .bar = 0,
         .bus = 0,
         .rsvd = 0,
         .func = 0,
         .dev = 5,
         .reg = 0x001c,
     }}},
    {"iio_vtd0_glbsts_b1d05f0",
     {{
         .lenCode = US_MMIO_DWORD,
         .bar = 0,
         .bus = 1,
         .rsvd = 0,
         .func = 0,
         .dev = 5,
         .reg = 0x001c,
     }}},
    {"iio_vtd0_glbsts_b2d05f0",
     {{
         .lenCode = US_MMIO_DWORD,
         .bar = 0,
         .bus = 2,
         .rsvd = 0,
         .func = 0,
         .dev = 5,
         .reg = 0x001c,
     }}},
    {"iio_vtd0_glbsts_b3d05f0",
     {{
         .lenCode = US_MMIO_DWORD,
         .bar = 0,
         .bus = 3,
         .rsvd = 0,
         .func = 0,
         .dev = 5,
         .reg = 0x001c,
     }}},
    {"iio_vtd1_fltsts_b0d05f0",
     {{
         .lenCode = US_MMIO_DWORD,
         .bar = 0,
         .bus = 0,
         .rsvd = 0,
         .func = 0,
         .dev = 5,
         .reg = 0x1034,
     }}},
    {"iio_vtd1_fltsts_b1d05f0",
     {{
         .lenCode = US_MMIO_DWORD,
         .bar = 0,
         .bus = 1,
         .rsvd = 0,
         .func = 0,
         .dev = 5,
         .reg = 0x1034,
     }}},
    {"iio_vtd1_fltsts_b2d05f0",
     {{
         .lenCode = US_MMIO_DWORD,
         .bar = 0,
         .bus = 2,
         .rsvd = 0,
         .func = 0,
         .dev = 5,
         .reg = 0x1034,
     }}},
    {"iio_vtd1_fltsts_b3d05f0",
     {{
         .lenCode = US_MMIO_DWORD,
         .bar = 0,
         .bus = 3,
         .rsvd = 0,
         .func = 0,
         .dev = 5,
         .reg = 0x1034,
     }}},
    {"iio_vtd1_glbsts_b0d05f0",
     {{
         .lenCode = US_MMIO_DWORD,
         .bar = 0,
         .bus = 0,
         .rsvd = 0,
         .func = 0,
         .dev = 5,
         .reg = 0x101c,
     }}},
    {"iio_vtd1_glbsts_b1d05f0",
     {{
         .lenCode = US_MMIO_DWORD,
         .bar = 0,
         .bus = 1,
         .rsvd = 0,
         .func = 0,
         .dev = 5,
         .reg = 0x101c,
     }}},
    {"iio_vtd1_glbsts_b2d05f0",
     {{
         .lenCode = US_MMIO_DWORD,
         .bar = 0,
         .bus = 2,
         .rsvd = 0,
         .func = 0,
         .dev = 5,
         .reg = 0x101c,
     }}},
    {"iio_vtd1_glbsts_b3d05f0",
     {{
         .lenCode = US_MMIO_DWORD,
         .bar = 0,
         .bus = 3,
         .rsvd = 0,
         .func = 0,
         .dev = 5,
         .reg = 0x101c,
     }}},
    {"ntb_b1d00f0_devsts_pb01base",
     {{
         .lenCode = US_MMIO_WORD,
         .bar = 0,
         .bus = 1,
         .rsvd = 0,
         .func = 0,
         .dev = 0,
         .reg = 0x459a,
     }}},
    {"ntb_b1d00f0_devsts_sb01base",
     {{
         .lenCode = US_MMIO_WORD,
         .bar = 2,
         .bus = 1,
         .rsvd = 0,
         .func = 0,
         .dev = 0,
         .reg = 0x059a,
     }}},
    {"ntb_b1d00f0_lnksts_pb01base",
     {{
         .lenCode = US_MMIO_WORD,
         .bar = 0,
         .bus = 1,
         .rsvd = 0,
         .func = 0,
         .dev = 0,
         .reg = 0x45a2,
     }}},
    {"ntb_b1d00f0_lnksts_sb01base",
     {{
         .lenCode = US_MMIO_WORD,
         .bar = 2,
         .bus = 1,
         .rsvd = 0,
         .func = 0,
         .dev = 0,
         .reg = 0x05a2,
     }}},
    {"ntb_b2d00f0_devsts_pb01base",
     {{
         .lenCode = US_MMIO_WORD,
         .bar = 0,
         .bus = 2,
         .rsvd = 0,
         .func = 0,
         .dev = 0,
         .reg = 0x459a,
     }}},
    {"ntb_b2d00f0_devsts_sb01base",
     {{
         .lenCode = US_MMIO_WORD,
         .bar = 2,
         .bus = 2,
         .rsvd = 0,
         .func = 0,
         .dev = 0,
         .reg = 0x059a,
     }}},
    {"ntb_b2d00f0_lnksts_pb01base",
     {{
         .lenCode = US_MMIO_WORD,
         .bar = 0,
         .bus = 2,
         .rsvd = 0,
         .func = 0,
         .dev = 0,
         .reg = 0x45a2,
     }}},
    {"ntb_b2d00f0_lnksts_sb01base",
     {{
         .lenCode = US_MMIO_WORD,
         .bar = 2,
         .bus = 2,
         .rsvd = 0,
         .func = 0,
         .dev = 0,
         .reg = 0x05a2,
     }}},
    {"ntb_b3d00f0_devsts_pb01base",
     {{
         .lenCode = US_MMIO_WORD,
         .bar = 0,
         .bus = 3,
         .rsvd = 0,
         .func = 0,
         .dev = 0,
         .reg = 0x459a,
     }}},
    {"ntb_b3d00f0_devsts_sb01base",
     {{
         .lenCode = US_MMIO_WORD,
         .bar = 2,
         .bus = 3,
         .rsvd = 0,
         .func = 0,
         .dev = 0,
         .reg = 0x059a,
     }}},
    {"ntb_b3d00f0_lnksts_pb01base",
     {{
         .lenCode = US_MMIO_WORD,
         .bar = 0,
         .bus = 3,
         .rsvd = 0,
         .func = 0,
         .dev = 0,
         .reg = 0x45a2,
     }}},
    {"ntb_b3d00f0_lnksts_sb01base",
     {{
         .lenCode = US_MMIO_WORD,
         .bar = 2,
         .bus = 3,
         .rsvd = 0,
         .func = 0,
         .dev = 0,
         .reg = 0x05a2,
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
static void uncoreStatusPciJson(const char *regName,
                                SUncoreStatusRegRawData *sRegData,
                                cJSON *pJsonChild)
{
    char jsonItemString[US_JSON_STRING_LEN];

    // Format the Uncore Status register data out to the .json debug file
    if (sRegData->bInvalid)
    {
        cJSON_AddStringToObject(pJsonChild, regName, US_FAILED);
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
static int uncoreStatusPci(crashdump::CPUInfo &cpuInfo, cJSON *pJsonChild)
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
                        (uint8_t *)&sRegData.uValue.u64,
                        &cc) != PECI_CC_SUCCESS)
                {
                    sRegData.bInvalid = true;
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
                            (uint8_t *)&sRegData.uValue.u32[u8Dword],
                            &cc) != PECI_CC_SUCCESS)
                    {
                        sRegData.bInvalid = true;
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
    crashdump::CPUInfo &cpuInfo, uint32_t u32Param, uint8_t u8NumDwords,
    SUncoreStatusRegRawData *sUncoreStatusMmioRawData, int peci_fd)
{
    uint8_t cc = 0;

    // Open the MMIO dump sequence
    if (peci_WrPkgConfig_seq(cpuInfo.clientAddr, MBX_INDEX_VCU, VCU_OPEN_SEQ,
                             VCU_READ_LOCAL_MMIO_SEQ, sizeof(uint32_t), peci_fd,
                             &cc) != PECI_CC_SUCCESS)
    {
        // MMIO sequence failed, abort the sequence
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
                (uint8_t *)&sUncoreStatusMmioRawData->uValue.u32[u8Dword],
                peci_fd, &cc) != PECI_CC_SUCCESS)
        {
            // MMIO sequence failed, abort the sequence
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
static int uncoreStatusPciMmioCPX1(crashdump::CPUInfo &cpuInfo,
                                   cJSON *pJsonChild)
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
static int uncoreStatusMcaRead(crashdump::CPUInfo &cpuInfo, uint32_t u32Param,
                               SUncoreStatusMcaRawData *sUncoreStatusMcaRawData,
                               int peci_fd)
{
    uint8_t cc = 0;

    // Open the MCA Bank dump sequence
    if (peci_WrPkgConfig_seq(cpuInfo.clientAddr, MBX_INDEX_VCU, VCU_OPEN_SEQ,
                             VCU_UNCORE_MCA_SEQ, sizeof(uint32_t), peci_fd,
                             &cc) != PECI_CC_SUCCESS)
    {
        // MCA Bank sequence failed, abort the sequence
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
                (uint8_t *)&sUncoreStatusMcaRawData->uRegData.u32Raw[u8Dword],
                peci_fd, &cc) != PECI_CC_SUCCESS)
        {
            // MCA Bank sequence failed, abort the sequence
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
static void uncoreStatusIioJson(const char *regName,
                                SUncoreStatusMcaRawData *sMcaData,
                                cJSON *pJsonChild)
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
            cJSON_AddStringToObject(pJsonChild, jsonNameString, US_FAILED);
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
static int uncoreStatusIio(crashdump::CPUInfo &cpuInfo, cJSON *pJsonChild)
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
        if (uncoreStatusMcaRead(cpuInfo, u32IioParam, &sMcaData, peci_fd) != 0)
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
                                      uint32_t *pu32UncoreCrashdump,
                                      cJSON *pJsonChild)
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
static int uncoreStatusCrashdump(crashdump::CPUInfo &cpuInfo, cJSON *pJsonChild)
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
                             sizeof(uint32_t), (uint8_t *)&u32NumReads, peci_fd,
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
                             sizeof(uint32_t), (uint8_t *)&u32ApiVersion,
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
    uint32_t *pu32UncoreCrashdump =
        (uint32_t *)calloc(u32NumReads, sizeof(uint32_t));
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
                                 (uint8_t *)&pu32UncoreCrashdump[i], peci_fd,
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
    uncoreStatusPci,
    uncoreStatusPciMmioCPX1,
    uncoreStatusIio,
    uncoreStatusCrashdump,
};

/******************************************************************************
 *
 *   logUncoreStatusCPX1
 *
 *   This function gathers the Uncore Status register contents and adds them to
 *   the debug log.
 *
 ******************************************************************************/
int logUncoreStatusCPX1(crashdump::CPUInfo &cpuInfo, cJSON *pJsonChild)
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
static void uncoreStatusJsonICX(const char *regName,
                                SUncoreStatusRegRawData *sRegData,
                                cJSON *pJsonChild, uint8_t cc)
{
    char jsonItemString[US_JSON_STRING_LEN];

    if (sRegData->bInvalid)
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
static int uncoreStatusPciICX1(crashdump::CPUInfo &cpuInfo, cJSON *pJsonChild)
{
    for (uint32_t i = 0;
         i < (sizeof(sUncoreStatusPciICX1) / sizeof(SUncoreStatusRegPci)); i++)
    {
        SUncoreStatusRegRawData sRegData = {};
        uint8_t cc = 0;
        switch (sUncoreStatusPciICX1[i].u8Size)
        {
            case US_REG_BYTE:
            case US_REG_WORD:
            case US_REG_DWORD:
                peci_RdEndPointConfigPciLocal(cpuInfo.clientAddr, US_PCI_SEG,
                                              sUncoreStatusPciICX1[i].u8Bus,
                                              sUncoreStatusPciICX1[i].u8Dev,
                                              sUncoreStatusPciICX1[i].u8Func,
                                              sUncoreStatusPciICX1[i].u16Reg,
                                              sUncoreStatusPciICX1[i].u8Size,
                                              (uint8_t *)&sRegData.uValue.u64,
                                              &cc);
                if (PECI_CC_UA(cc))
                {
                    sRegData.bInvalid = true;
                }
                break;
            case US_REG_QWORD:
                for (uint8_t u8Dword = 0; u8Dword < 2; u8Dword++)
                {
                    peci_RdEndPointConfigPciLocal(
                        cpuInfo.clientAddr, US_PCI_SEG,
                        sUncoreStatusPciICX1[i].u8Bus,
                        sUncoreStatusPciICX1[i].u8Dev,
                        sUncoreStatusPciICX1[i].u8Func,
                        sUncoreStatusPciICX1[i].u16Reg + (u8Dword * 4),
                        sizeof(uint32_t),
                        (uint8_t *)&sRegData.uValue.u32[u8Dword], &cc);
                    if (PECI_CC_UA(cc))
                    {
                        sRegData.bInvalid = true;
                        break;
                    }
                }
                break;
            default:
                sRegData.bInvalid = true;
        }

        uncoreStatusJsonICX(sUncoreStatusPciICX1[i].regName, &sRegData,
                            pJsonChild, cc);
    }
    return 0;
}

/******************************************************************************
 *
 *   uncoreStatusPciMmioICX1
 *
 *   This function gathers the Uncore Status PCI MMIO registers
 *
 ******************************************************************************/
static int uncoreStatusPciMmioICX1(crashdump::CPUInfo &cpuInfo,
                                   cJSON *pJsonChild)
{
    char jsonNameString[US_REG_NAME_LEN];

    for (uint32_t i = 0; i < (sizeof(sUncoreStatusPciMmioICX1) /
                              sizeof(SUncoreStatusRegPciMmioICX1));
         i++)
    {
        SUncoreStatusRegRawData sRegData = {};
        uint8_t addrType = PECI_ENDPTCFG_ADDR_TYPE_MMIO_D;
        uint8_t cc = 0;
        uint8_t readLen;
        uint8_t bus;
        peci_RdEndPointConfigPciLocal(
            cpuInfo.clientAddr, US_MMIO_SEG, MMIO_ROOT_BUS, MMIO_ROOT_DEV,
            MMIO_ROOT_FUNC, MMIO_ROOT_REG, US_REG_DWORD, (uint8_t *)&bus, &cc);

        if (PECI_CC_UA(cc))
        {
            sRegData.bInvalid = true;
        }
        else
        {
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
            }

            peci_RdEndPointConfigMmio(
                cpuInfo.clientAddr, US_MMIO_SEG, bus,
                sUncoreStatusPciMmioICX1[i].u8Dev,
                sUncoreStatusPciMmioICX1[i].u8Func,
                sUncoreStatusPciMmioICX1[i].u8Bar, addrType,
                sUncoreStatusPciMmioICX1[i].u64Offset, readLen,
                (uint8_t *)&sRegData.uValue.u64, &cc);

            if (PECI_CC_UA(cc))
            {
                sRegData.bInvalid = true;
            }
        }
        cd_snprintf_s(jsonNameString, US_JSON_STRING_LEN,
                      sUncoreStatusPciMmioICX1[i].regName, bus);
        uncoreStatusJsonICX(jsonNameString, &sRegData, pJsonChild, cc);
    }
    return 0;
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
int logUncoreStatusICX1(crashdump::CPUInfo &cpuInfo, cJSON *pJsonChild)
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
    {crashdump::CPUModel::clx_b0, logUncoreStatusCPX1},
    {crashdump::CPUModel::clx_b1, logUncoreStatusCPX1},
    {crashdump::CPUModel::cpx_a0, logUncoreStatusCPX1},
    {crashdump::CPUModel::skx_h0, logUncoreStatusCPX1},
    {crashdump::CPUModel::icx_a0, logUncoreStatusICX1},
};

/******************************************************************************
 *
 *   logUncoreStatus
 *
 *   This function gathers the Uncore Status register contents and adds them to
 *   the debug log.
 *
 ******************************************************************************/
int logUncoreStatus(crashdump::CPUInfo &cpuInfo, cJSON *pJsonChild)
{
    if (pJsonChild == NULL)
    {
        return 1;
    }

    for (uint32_t i = 0;
         i < (sizeof(sUncoreStatusLogVx) / sizeof(SUncoreStatusLogVx)); i++)
    {
        if (cpuInfo.model == sUncoreStatusLogVx[i].cpuModel)
        {
            return sUncoreStatusLogVx[i].logUncoreStatusVx(cpuInfo, pJsonChild);
        }
    }

    fprintf(stderr, "Cannot find version for %s\n", __FUNCTION__);
    return 1;
}
