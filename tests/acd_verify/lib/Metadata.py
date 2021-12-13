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


class Metadata(Section):
    def __init__(self, jOutput):
        Section.__init__(self, jOutput, "METADATA")

        self.cpus = []
        self.verifySection()
        self.rootNodes = f"sockets: {len(self.cpus)}"

    @classmethod
    def createMetadata(cls, jOutput):
        if "METADATA" in jOutput:
            return cls(jOutput)
        else:
            warnings.warn(
                f"Metadata section was not found in this file"
            )
            return None

    def verifyCpuID(self):
        # Obtain all cpuIDs
        cpuIDs = [self.sectionInfo[cpu].get('cpuid') for cpu in self.cpus]
        # Eliminate duplicates
        cpuIDs = list(dict.fromkeys(cpuIDs))
        if len(cpuIDs) > 1:   # More than 1 cpuID found
            tmp = {}
            for cpu in self.cpus:
                tmp[cpu] = self.sectionInfo[cpu]["cpuid"] if "cpuid" in self.sectionInfo[cpu] else "Not present"
            return tmp
        return cpuIDs

    def getSummaryInfo(self):
        summaryInfo = {}
        summaryInfo["nCPUs"] = len(self.cpus)
        summaryInfo["IDs"] = self.verifyCpuID()
        summaryInfo["totalTime"] = self.sectionInfo["_total_time"]
        summaryInfo["triggerType"] = self.sectionInfo["trigger_type"]
        summaryInfo["crashcoreCounts"] = self.getCrashcoreCounts()

        return summaryInfo

    def getCrashcoreCounts(self):
        tmp = {}
        for cpu in self.cpus:
            tmp[cpu] = self.sectionInfo[cpu]["crashcore_count"]
        return tmp

    def verifySection(self):
        for key in self.sectionInfo:
            regValueNotStr = (type(self.sectionInfo[key]) != str)
            underScoreReg = key.startswith('_')

            if key.startswith("cpu"):
                self.cpus.append(key)
                # Self checking
                self.makeSelfCheck(key)
            # Considering cpuXXs as the valid sections
            #  for the METADATA region of the output file
            elif not underScoreReg and regValueNotStr:
                warnings.warn(
                    f"Section {key}, not expected in {self.sectionName}")
            self.search(key, self.sectionInfo[key])
        if len(self.cpus) == 0:
            warnings.warn("No cpus found in METADATA")

        # Version Check
        self.verifyVersion()

    def search(self, key, value, regName=None):
        # Want to seach 4 a register value
        if regName:
            if key == regName:
                if type(value) is not str:
                    return value
                else:
                    containsError = self.eHandler.isError(value)
                    if not containsError:
                        return value
            else:
                if type(value) == dict:
                    result = None
                    for vKey in value:
                        result = self.search(vKey, value[vKey], regName)
                        if result:
                            break
                    return result
                # key doesn't match with regName
                elif type(value) == str:
                    return None

        # Normal search()
        else:
            valueIsValidType = ((type(value) == str) or (type(value) == bool) or
                            (type(value) == int))
            if type(value) == dict:
                for vKey in value:
                    self.search(f"{key}.{vKey}", value[vKey])
            elif (valueIsValidType and not value.startswith('_')):
                self.nRegs += 1  # count regs
                if self.eHandler.isError(value):
                    error = self.eHandler.extractError(value)
                    self.eHandler.errors[key] = error

    def makeSelfCheck(self, cpu):
        lErrors = []
        cpuInfo = self.sectionInfo[cpu]
        # cpuid
        self.checkCpuID(cpu)
        # core_mask
        self.checkCoreMask(cpu)
        # cha_count
        self.checkChaCount(cpu)

    def checkCpuID(self, cpu):
        cpuID = self.search(cpu, self.sectionInfo[cpu], "cpuid")
        if cpuID:
            cpuID = cpuID.upper()
            # ICX
            icxValidVal = (cpuID[:-1] == "0X606A")
            # SPR
            sprValidVal = (cpuID[:-1] == "0X806F")
            if (not icxValidVal) and (not sprValidVal):
                errMessage = f"{cpu} has CPUID invalid value {cpuID}"
                self.healthCheckErrors.append(errMessage)
        else:
            errMessage = f"{cpu} cpuid key not present in METADATA"
            self.healthCheckErrors.append(errMessage)

    def checkCoreMask(self, cpu):
        coreMask = self.search(cpu, self.sectionInfo[cpu], "core_mask")
        if coreMask:
            if coreMask == '0x0':
                errMessage = f"{cpu} has core_mask invalid value {coreMask}"
                self.healthCheckErrors.append(errMessage)
        else:
            errMessage = f"{cpu} core_mask key not present in METADATA"
            self.healthCheckErrors.append(errMessage)

    def checkChaCount(self, cpu):
        chaCount = self.search(cpu, self.sectionInfo[cpu], "cha_count")
        if chaCount:
            if chaCount == '0x0':
                errMessage = f"{cpu} has cha_count invalid value {chaCount}"
                self.healthCheckErrors.append(errMessage)
        else:
            errMessage = f"{cpu} cha_count key not present in METADATA"
            self.healthCheckErrors.append(errMessage)

    def verifyVersion(self):
        if "_version" in self.sectionInfo:
            version = self.sectionInfo["_version"]
            if not self.eHandler.isError(version):
                self.checkVersion(version)
        else:
            errMessage = f"_version key not present in {self.sectionName}"
            self.healthCheckErrors.append(errMessage)
