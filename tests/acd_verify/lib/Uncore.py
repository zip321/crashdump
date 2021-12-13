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
import collections


class Uncore(Section):
    def __init__(self, jOutput):
        Section.__init__(self, jOutput, "uncore")

        self.bdfs = {}
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
        valueIsValidType = ((type(value) == str) or (type(value) == bool) or
                            (type(value) == int))
        if type(value) == dict:
            for vKey in value:
                self.search(f"{key}.{vKey}", value[vKey])
        elif (valueIsValidType and not key.startswith('_')):
            self.nRegs += 1  # count regs
            lastKey = (key.split(".")[-1]) if ("." in key) else key
            bdf = "_".join(lastKey.split('_')[0:-1])
            offset = lastKey.split('_')[-1].upper()

            offsetIs0x0 = offset.endswith("0X0")

            if (bdf not in self.bdfs) and (not bdf.startswith("RDIAMSR")):
                self.bdfs[bdf] = {
                    "total": 1,
                    "regsWE": 0,
                    "offsetsWE": []
                }
            elif not bdf.startswith("RDIAMSR"):
                self.bdfs[bdf]["total"] += 1

            if self.eHandler.isError(value):
                error = self.eHandler.extractError(value)
                self.eHandler.errors[key] = error

                if not bdf.startswith("RDIAMSR"):
                    self.bdfs[bdf]["regsWE"] += 1
                    self.bdfs[bdf]["offsetsWE"].append(error)

            # Self-check
            elif offsetIs0x0:
                if (value.upper() != "0X8086") and (value.upper() != "0X0"):
                    sValue = f"{key} has invalid value {value}"
                    self.healthCheckErrors.append(sValue)

    def getTableInfo(self):
        lUncoreObjs = []

        uncoreTableInfo = {
            "Section": self.sectionName,
            "rootNodes": self.rootNodes,
            "regs": self.nRegs
        }

        level = dict(collections.Counter(self.eHandler.errors.values()))
        for error in level:
            if error not in uncoreTableInfo:
                uncoreTableInfo[error] = level[error]
            elif error in uncoreTableInfo:
                uncoreTableInfo[error] = uncoreTableInfo[error] + level[error]

        for bdf in self.bdfs:
            tableInfo = {
                "Section": f"{self.sectionName}-{bdf}",
                "rootNodes": f"offsets: {self.bdfs[bdf]['total']}",
                "regsWE": self.bdfs[bdf]['regsWE'],
                "regs": ""
            }
            hasErrors = len(self.bdfs[bdf]['offsetsWE']) > 0
            if hasErrors:
                level = dict(collections.Counter(self.bdfs[bdf]['offsetsWE']))
                for error in level:
                    if error not in tableInfo:
                        percentage = round((level[error]*100)/self.bdfs[bdf]['total'], 1)
                        tableInfo[error] = f"{percentage}% ({level[error]})"
                lUncoreObjs.append(tableInfo)

            lUncoreObjs.insert(0, uncoreTableInfo)

        return lUncoreObjs
