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

import os


class Postprocessor():
    def __init__(self, reportObj, filePath, fileType, verbose=False,
                 compareFileName=None):
        self.fileType = fileType
        baseFileName = os.path.splitext(os.path.basename(filePath))[0]
        self.fileName = f"{baseFileName}_report.{fileType}"
        baseFilePath = os.path.dirname(filePath)
        self.filePath = os.path.join(baseFilePath, self.fileName)
        self.report = reportObj
        self.verbose = verbose
        self.compareFileName = compareFileName

    def formatTXT(self):
        sReport = ""
        for region in self.report:
            if region == "summary":
                sReport = sReport + self.txtSummary(self.report[region])
            elif region == "table":
                sReport = sReport + "\n\n"+self.txtErrorsTable(self.report[region])
            elif region == "tableInfo":
                sReport = sReport + "\n" + \
                    self.txtTableDetails(self.report[region])
            elif region == "selfCheck":
                sReport = sReport + "\n" + \
                    self.txtSelfCheck(self.report[region])
            elif region == "missCompares":
                sReport = sReport + "\n" + \
                    self.txtCompareInfo(self.report[region])
        try:
            f = open(self.filePath, "w")
            f.write(sReport)
            f.close()
            print(f"Report saved: {self.filePath}\n")
        except Exception as e:
            eMessage = "An error ocurred while trying to save " +\
                         "the file {self.filePath}"
            print(eMessage)
            raise e

    def saveFile(self):
        if self.fileType == "txt":
            self.formatTXT()
        else:
            raise Exception("File type not supported")

    def txtSummary(self, summary):
        title = "Summary of File Contents:"
        nCPUs = "\n\tNumber of CPUs: {0}".format(summary["nCPUs"])
        cpuID = "\n\tcpuid: {0}".format(self.txtCpuId(summary["IDs"]))
        totalTime = "\n\t_total_time: {0}".format(summary["totalTime"])
        triggerType = "\n\ttrigger_type: {0}".format(summary["triggerType"])
        crashcoreCounts = self.txtCrashcoreCounts(summary["crashcoreCounts"])

        sSummary = title + nCPUs + cpuID + totalTime + triggerType + \
            crashcoreCounts
        return sSummary

    def txtCpuId(self, IDs):
        if len(IDs) > 1:
            sIDs = ""
            for cpu in IDs:
                cpuID = "\n\t\t{0} ID: {1}".format(cpu, IDs[cpu])
                sIDs = sIDs + cpuID
            return sIDs
        else:
            return IDs[0]

    def txtCrashcoreCounts(self, counts):
        sCounts = ""
        for cpu in counts:
            sCPU = "\n\tcrashcoreCount {0}: {1}".format(
                cpu.upper(), counts[cpu])
            sCounts = sCounts+sCPU
        return sCounts

    def txtErrorsTable(self, table):
        tInfo = {}
        sTable = ""

        # Get the headers
        headers = self.getTableHeaders(table)
        for header in headers:
            tInfo[header] = []

        # Fill the table
        for key in table:
            # metadata
            if key == "metadata":
                keys = table[key].keys()
                for header in headers:
                    if header in keys:
                        tInfo[header].append(str(table[key][header]))
                    else:
                        tInfo[header].append("")
            # Socket
            if "cpu" in key:
                socket = table[key]
                for section in socket:
                    keys = socket[section].keys()
                    for header in headers:
                        if header in keys:
                            if header == "Section":
                                value = f"{key}:{socket[section][header]}"
                                tInfo[header].append(value)
                            else:
                                value = str(socket[section][header])
                                tInfo[header].append(value)
                        else:
                            tInfo[header].append("")

        columns = [tInfo[x] for x in tInfo]
        return self.txtTable(headers, columns)

    def txtTableDetails(self, errorList):
        regsList = "\t"

        if self.verbose:
            regsList += "All errors found"
            fullList = self.getFullErrorList(errorList)
            for reg in fullList:
                regsList = regsList + f"\n\t\t{reg}: {fullList[reg]}"
        elif not self.verbose:
            regsList += "First 3 errors of each record are:" +\
                        "    (use –-verbose for all)"
            for key in errorList:
                if key == "metadata":
                    count = 0
                    for reg in errorList[key]:
                        regsList += f"\n\t\t{key}.{reg}: {errorList[key][reg]}"
                        if count == 2:
                            break
                        count += 1
                if "cpu" in key:
                    for section in errorList[key]:
                        count = 0
                        for reg in errorList[key][section]:
                            regsList += f"\n\t\t{key}.{section}.{reg}:" + \
                                        f" {errorList[key][section][reg]}"
                            if count == 2:
                                break
                            count += 1

        return regsList

    def getFullErrorList(self, errorList):
        fullList = {}
        for key in errorList:
            if key == "metadata":
                for reg in errorList[key]:
                    regName = f"{key}.{reg}"
                    regValue = errorList[key][reg]
                    fullList[regName] = regValue
            elif "cpu" in key:
                for section in errorList[key]:
                    for reg in errorList[key][section]:
                        regName = f"{key}.{section}.{reg}"
                        regValue = errorList[key][section][reg]
                        fullList[regName] = regValue
        return fullList

    def getTableHeaders(self, table):
        headers = ["Section"]
        for key in table:
            # metadata
            if key == "metadata":
                keys = table[key].keys()
                for header in keys:
                    if header not in headers:
                        headers.append(header)
            # Socket
            if "cpu" in key:
                socket = table[key]
                for section in socket:
                    keys = socket[section].keys()
                    for header in keys:
                        if header not in headers:
                            headers.append(header)
        return headers

    def txtSelfCheck(self, errorList):
        sErrors = "\nSelf-check:\t"
        errors = 0

        if self.verbose:
            sErrors += "All errors found"
        else:
            sErrors += "First 3 errors of each record are:" +\
                        "    (use –-verbose for all)"

        for key in errorList:
            if key == "metadata":
                count = 3
                for error in errorList[key]:
                    if count and not self.verbose:
                        sErrors += f"\n\t\t{key}: {error}"
                        count -= 1
                        errors += 1
                    elif not count and not self.verbose:
                        break
                    else:
                        sErrors += f"\n\t\t{key}: {error}"
                        errors += 1
            if key.startswith("cpu"):
                for section in errorList[key]:
                    count = 3
                    for error in errorList[key][section]:
                        if count and not self.verbose:
                            sErrors += f"\n\t\t{key}.{section}: {error}"
                            count -= 1
                            errors += 1
                        elif not count and not self.verbose:
                            break
                        else:
                            sErrors += f"\n\t\t{key}.{section}: {error}"
                            errors += 1
        if errors == 0:
            sErrors += "\n\t\tPASS"

        return sErrors

    def txtCompareInfo(self, missCompares):
        sErrors = "\nCompare Data:\t"

        nDiffsTable, specialSections = self.txtNDiffsTable()
        if len(specialSections) > 0:
            sErrors += "\n Comments:\n  - Not comparing data for: " +\
                ', '.join([x for x in specialSections]) +\
                " at this time"
        sErrors += "\n\n" + nDiffsTable + "\n"
        if self.verbose:
            sErrors += "All errors found:\n\n"
        else:
            sErrors += "Showing first 3 errors of each record:" +\
                        "    (use –-verbose for all)\n\n"
        sErrors += self.txtDiffsTable(missCompares)

        return sErrors

    def txtTable(self, headers, columns):
        sTable = ""
        for column in range(len(columns)):
            # Add headers row to columns
            columns[column].insert(0, headers[column])

            # Get longest string in column
            maxWidth = len(max(columns[column], key=len))
            for i in range(len(columns[column])):
                element = columns[column][i]
                columns[column][i] = element.rjust(maxWidth + 1)

        nRows = len(columns[0])

        # Iterate through all columns in the same row
        #  and form the string for each row
        for row in range(nRows):
            sRow = '|'
            for column in range(len(columns)):
                sRow += columns[column][row]+'|'
            # Configure headers
            if row == 0:
                sRow = '='*(len(sRow)) + '\n' + sRow + '\n' +\
                         '='*(len(sRow)) + '\n'
            # Configure rest of the rows
            else:
                sRow += '\n' + '-'*(len(sRow)) + '\n'
            # Append row
            sTable += sRow
        return sTable

    def txtDiffsTable(self, lDiffs):
        lDiff = {
            "Section": [],
            "fProcess": [],
            "fCompare": []
        }

        headers = ["Section", "fProcess", "fCompare"]

        for key in lDiffs:
            if key == "metadata":
                sectionInfo = lDiffs["metadata"]
                if self.verbose:
                    for reg in sectionInfo:
                        lDiff["Section"].append(f"{key}.{reg}")
                        lDiff["fProcess"].append(sectionInfo[reg][0])
                        lDiff["fCompare"].append(sectionInfo[reg][1])
                else:
                    sectionInfo = {k: sectionInfo[k] for k in list(sectionInfo)[:3]}
                    for reg in sectionInfo:
                        lDiff["Section"].append(f"{key}.{reg}")
                        lDiff["fProcess"].append(sectionInfo[reg][0])
                        lDiff["fCompare"].append(sectionInfo[reg][1])
            if key.startswith("cpu"):
                for section in lDiffs[key]:
                    sectionInfo = lDiffs[key][section]
                    if self.verbose:
                        for reg in sectionInfo:
                            lDiff["Section"].append(f"{key}.{section}.{reg}")
                            lDiff["fProcess"].append(sectionInfo[reg][0])
                            lDiff["fCompare"].append(sectionInfo[reg][1])
                    else:
                        sectionInfo = {k: sectionInfo[k] for k in list(sectionInfo)[:3]}
                        for reg in sectionInfo:
                            lDiff["Section"].append(f"{key}.{section}.{reg}")
                            lDiff["fProcess"].append(sectionInfo[reg][0])
                            lDiff["fCompare"].append(sectionInfo[reg][1])

        columns = [lDiff[x] for x in lDiff]

        # Names of files
        fProcessed = os.path.splitext(self.fileName)[0].rstrip("_report")

        tableHeaders = ["Item", fProcessed[:20], self.compareFileName[:20]]

        return self.txtTable(tableHeaders, columns)

    def txtNDiffsTable(self):
        lDiffs = (self.report["nDiffs"])
        lDiff = {
            "Section": [],
            "nDiffs": [],
        }

        foundSpecialSections = []

        for key in lDiffs:
            if key == "metadata":
                lDiff["Section"].append(key)
                lDiff["nDiffs"].append(str(lDiffs[key]))
            if key.startswith("cpu"):
                for section in lDiffs[key]:
                    lDiff["Section"].append(f"{key}.{section}")
                    lDiff["nDiffs"].append(str(lDiffs[key][section]))

        columns = [lDiff[x] for x in lDiff]
        headers = ["Section", "Differences"]

        return self.txtTable(headers, columns), self.report["specialCompSections"]
