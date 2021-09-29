###############################################################################
# INTEL CONFIDENTIAL                                                          #
#                                                                             #
# Copyright 2021 Intel Corporation.                                           #
#                                                                             #
# This software and the related documents are Intel copyrighted materials,    #
# and your use of them is governed by the express license under which they    #
# were provided to you ("License"). Unless the License provides otherwise,    #
# you may not use, modify, copy, publish, distribute, disclose or transmit    #
# this software or the related documents without Intel's prior written        #
# permission.                                                                 #
#                                                                             #
# This software and the related documents are provided as is, with no express #
# or implied warranties, other than those that are expressly stated in the    #
# License.                                                                    #
###############################################################################

from lib.Section import Section

import warnings


class Uncore(Section):
    def __init__(self, jOutput):
        Section.__init__(self, jOutput, "uncore")

        self.verifySection()
        self.rootNodes = ""

    @classmethod
    def createUncore(cls, jOutput):
        if "uncore" in jOutput:
            return cls(jOutput)
        else:
            warnings.warn(
                f"Uncore section was not found in this file"
            )
            return None

    def search(self, key, value):
        if type(value) == dict:
            for vKey in value:
                self.search(f"{key}.{vKey}", value[vKey])
        elif (type(value) == str) and not value.startswith('_'):
            self.nRegs += 1  # count regs
            lastKey = (key.split(".")[-1]) if ("." in key) else key
            offset0x0 = lastKey.upper().endswith("0X0")
            if self.eHandler.isError(value):
                error = self.eHandler.extractError(value)
                self.eHandler.errors[key] = error
            # Self-check
            elif offset0x0:
                if (value.upper() != "0X8086") and (value.upper() != "0X0"):
                    sValue = f"{key} has invalid value {value}"
                    self.healthCheckErrors.append(sValue)
