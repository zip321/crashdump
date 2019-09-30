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

#include <ctype.h>
#include <dirent.h>
#include <errno.h>
#include <fcntl.h>
#include <peci.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <systemd/sd-id128.h>
#include <termios.h>
#include <time.h>
#include <unistd.h>

#include <experimental/filesystem>
#include <fstream>
#include <regex>
#include <sdbusplus/asio/object_server.hpp>
#include <sstream>
#include <vector>

extern "C" {
#include <cjson/cJSON.h>

#include "safe_str_lib.h"
}

#include "CrashdumpSections/AddressMap.hpp"
#include "CrashdumpSections/BigCore.hpp"
#include "CrashdumpSections/CoreMca.hpp"
#include "CrashdumpSections/MetaData.hpp"
#include "CrashdumpSections/PowerManagement.hpp"
#include "CrashdumpSections/SqDump.hpp"
#include "CrashdumpSections/TorDump.hpp"
#include "CrashdumpSections/Uncore.hpp"
#include "CrashdumpSections/UncoreMca.hpp"
#include "utils.hpp"

namespace crashdump
{
static boost::asio::io_service io;
static std::shared_ptr<sdbusplus::asio::connection> conn;

constexpr char const* crashdumpService = "com.intel.crashdump";
constexpr char const* crashdumpPath = "/com/intel/crashdump";
constexpr char const* crashdumpInterface = "com.intel.crashdump";
constexpr char const* crashdumpOnDemandPath = "/com/intel/crashdump/OnDemand";
constexpr char const* crashdumpStoredInterface = "com.intel.crashdump.Stored";
constexpr char const* crashdumpOnDemandInterface =
    "com.intel.crashdump.OnDemand";
constexpr char const* crashdumpRawPeciInterface =
    "com.intel.crashdump.SendRawPeci";
static const std::experimental::filesystem::path crashdumpDir =
    "/tmp/crashdumps";

constexpr char const* triggerTypeOnDemand = "On-Demand";

static const std::string getUuid()
{
    std::string ret;
    sd_id128_t appId = SD_ID128_MAKE(e0, e1, 73, 76, 64, 61, 47, da, a5, 0c, d0,
                                     cc, 64, 12, 45, 78);
    sd_id128_t machineId = SD_ID128_NULL;

    if (sd_id128_get_machine_app_specific(appId, &machineId) == 0)
    {
        std::array<char, SD_ID128_STRING_MAX> str;
        ret = sd_id128_to_string(machineId, str.data());
        ret.insert(8, 1, '-');
        ret.insert(13, 1, '-');
        ret.insert(18, 1, '-');
        ret.insert(23, 1, '-');
    }

    return ret;
}

static void getClientAddrs(std::vector<CPUInfo>& cpuInfo)
{
    for (int cpu = 0, addr = MIN_CLIENT_ADDR; addr <= MAX_CLIENT_ADDR;
         cpu++, addr++)
    {
        if (peci_Ping(addr) == PECI_CC_SUCCESS)
        {
            cpuInfo.emplace_back();
            cpuInfo[cpu].clientAddr = addr;
        }
    }
}

static bool getCPUModels(std::vector<CPUInfo>& cpuInfo)
{
    uint8_t cc = 0;

    for (CPUInfo& cpu : cpuInfo)
    {
        if (peci_GetCPUID(cpu.clientAddr, &cpu.model, &cc) != PECI_CC_SUCCESS)
        {
            fprintf(stderr, "Cannot get CPUID!\n");
            continue;
        }

        // Check that it is a supported CPU
        switch (cpu.model)
        {
            case skx:
                fprintf(stderr, "SKX detected (CPUID 0x%x)\n", cpu.model);
                break;
            case clx:
            case clx2:
                fprintf(stderr, "CLX detected (CPUID 0x%x)\n", cpu.model);
                break;
            case cpx:
                fprintf(stderr, "CPX detected (CPUID 0x%x)\n", cpu.model);
                break;
            case icx:
                fprintf(stderr, "ICX detected (CPUID 0x%x)\n", cpu.model);
                break;
            default:
                fprintf(stderr, "Unsupported CPUID 0x%x\n", cpu.model);
                return false;
                break;
        }
    }
    return true;
}

static bool getCoreMasks(std::vector<CPUInfo>& cpuInfo)
{
    uint8_t cc = 0;

    for (CPUInfo& cpu : cpuInfo)
    {
        switch (cpu.model)
        {
            case cpx:
            case clx:
            case clx2:
            case skx:
                // RESOLVED_CORES Local PCI B1:D30:F3 Reg 0xB4
                uint32_t coreMask;
                if (peci_RdPCIConfigLocal(cpu.clientAddr, 1, 30, 3, 0xB4,
                                          sizeof(coreMask), (uint8_t*)&coreMask,
                                          &cc) != PECI_CC_SUCCESS)
                {
                    fprintf(stderr, "Cannot find coreMask!\n");
                    return false;
                }
                cpu.coreMask = coreMask;
                break;
            case icx:
                // RESOLVED_CORES Local PCI B14:D30:F3 Reg 0xD0 and 0xD4
                uint32_t coreMask0;
                if (peci_RdPCIConfigLocal(
                        cpu.clientAddr, 14, 30, 3, 0xD0, sizeof(coreMask0),
                        (uint8_t*)&coreMask0, &cc) != PECI_CC_SUCCESS)
                {
                    fprintf(stderr, "Cannot find coreMask0!\n");
                    return false;
                }
                uint32_t coreMask1;
                if (peci_RdPCIConfigLocal(
                        cpu.clientAddr, 14, 30, 3, 0xD4, sizeof(coreMask1),
                        (uint8_t*)&coreMask1, &cc) != PECI_CC_SUCCESS)
                {
                    fprintf(stderr, "Cannot find coreMask1!\n");
                    return false;
                }
                cpu.coreMask = coreMask1;
                cpu.coreMask <<= 32;
                cpu.coreMask |= coreMask0;
                break;
            default:
                return false;
        }
    }
    return true;
}

static bool getCHACounts(std::vector<CPUInfo>& cpuInfo)
{
    uint8_t cc = 0;

    for (CPUInfo& cpu : cpuInfo)
    {
        switch (cpu.model)
        {
            case cpx:
            case clx:
            case clx2:
            case skx:
                // LLC_SLICE_EN Local PCI B1:D30:F3 Reg 0x9C
                uint32_t chaMask;
                if (peci_RdPCIConfigLocal(cpu.clientAddr, 1, 30, 3, 0x9C,
                                          sizeof(chaMask), (uint8_t*)&chaMask,
                                          &cc) != PECI_CC_SUCCESS)
                {
                    fprintf(stderr, "Cannot find chaMask!\n");
                    return false;
                }
                cpu.chaCount = __builtin_popcount(chaMask);
                break;
            case icx:
                // LLC_SLICE_EN Local PCI B14:D30:F3 Reg 0x9C and 0xA0
                uint32_t chaMask0;
                if (peci_RdPCIConfigLocal(cpu.clientAddr, 14, 30, 3, 0x9C,
                                          sizeof(chaMask0), (uint8_t*)&chaMask0,
                                          &cc) != PECI_CC_SUCCESS)
                {
                    fprintf(stderr, "Cannot find chaMask0!\n");
                    return false;
                }
                uint32_t chaMask1;
                if (peci_RdPCIConfigLocal(cpu.clientAddr, 14, 30, 3, 0xA0,
                                          sizeof(chaMask1), (uint8_t*)&chaMask1,
                                          &cc) != PECI_CC_SUCCESS)
                {
                    fprintf(stderr, "Cannot find chaMask1!\n");
                    return false;
                }
                cpu.chaCount =
                    __builtin_popcount(chaMask0) + __builtin_popcount(chaMask1);
                break;
            default:
                return false;
        }
    }
    return true;
}

static bool getCPUInfo(std::vector<CPUInfo>& cpuInfo)
{
    cpuInfo.reserve(MAX_CPUS);
    getClientAddrs(cpuInfo);
    if (!getCPUModels(cpuInfo))
    {
        return false;
    }
    if (!getCoreMasks(cpuInfo))
    {
        return false;
    }

    return getCHACounts(cpuInfo);
}

static void logTimestamp(cJSON* parent)
{
    char logTime[64];
    time_t curtime;
    struct tm* loctime;

    // Add the timestamp
    curtime = time(NULL);
    loctime = localtime(&curtime);
    if (NULL != loctime)
    {
        strftime(logTime, sizeof(logTime), "%FT%T%z", loctime);
    }
    cJSON_AddStringToObject(parent, "timestamp", logTime);
}

static void logTriggerType(cJSON* parent)
{
    cJSON_AddStringToObject(parent, "trigger_type", triggerTypeOnDemand);
}

static void logPlatformName(cJSON* parent)
{
    cJSON_AddStringToObject(parent, "platform_name", getUuid().c_str());
}

static cJSON*
    addSectionLog(cJSON* parent, crashdump::CPUInfo& cpuInfo,
                  std::string sectionName,
                  const std::function<int(crashdump::CPUInfo& cpuInfo, cJSON*)>
                      sectionLogFunc)
{
    fprintf(stderr, "Logging %s on PECI address %d\n", sectionName.c_str(),
            cpuInfo.clientAddr);

    // Create an empty JSON object for this section if it doesn't already
    // exist
    cJSON* logSectionJson;
    if ((logSectionJson = cJSON_GetObjectItemCaseSensitive(
             parent, sectionName.c_str())) == NULL)
    {
        cJSON_AddItemToObject(parent, sectionName.c_str(),
                              logSectionJson = cJSON_CreateObject());
    }

    // Get the log for this section
    int ret = 0;
    if ((ret = sectionLogFunc(cpuInfo, logSectionJson)) != 0)
    {
        fprintf(stderr, "Error %d during %s log\n", ret, sectionName.c_str());
    }

    // Check if child data is added to the JSON section
    if (logSectionJson->child == NULL)
    {
        // If there was supposed to be child data, add a failed status
        if (ret != 0)
        {
            cJSON_AddStringToObject(logSectionJson,
                                    crashdump::dbgStatusItemName,
                                    crashdump::dbgFailedStatus);
        }
        else
        {
            // Otherwise delete it
            cJSON_DeleteItemFromObjectCaseSensitive(parent,
                                                    sectionName.c_str());
            logSectionJson = NULL;
        }
    }
    return logSectionJson;
}

void createCrashdump(std::string& crashdumpContents)
{
    cJSON* root = NULL;
    cJSON* crashlogData = NULL;
    cJSON* processors = NULL;
    cJSON* cpu = NULL;
    cJSON* logSection = NULL;
    char* out = NULL;
    int ret;

    // Get the list of CPU Info for this log
    std::vector<crashdump::CPUInfo> cpuInfo;

    fprintf(stderr, "Crashdump started...\n");
    if (!crashdump::getCPUInfo(cpuInfo))
    {
        fprintf(stderr, "Failed to get CPU Info!\n");
        return;
    }

    // start the JSON tree for CPU dump
    root = cJSON_CreateObject();

    // Build the CPU Crashdump JSON file
    // Everything is logged under a "crash_data" section
    cJSON_AddItemToObject(root, "crash_data",
                          crashlogData = cJSON_CreateObject());

    // Create the processors section
    cJSON_AddItemToObject(crashlogData, "PROCESSORS",
                          processors = cJSON_CreateObject());

    // Include the version field
    logCrashdumpVersion(processors, cpuInfo[0], record_type::bmcAutonomous);

    // Fill in the Crashdump data in the correct order (uncore to core) for
    // each CPU
    for (int i = 0; i < cpuInfo.size(); i++)
    {
        // Create a section for this cpu
        char cpuString[8];
        cd_snprintf_s(cpuString, sizeof(cpuString), "cpu%d", i);
        cJSON_AddItemToObject(processors, cpuString,
                              cpu = cJSON_CreateObject());

        // Fill in the Core Crashdump
        logSection = addSectionLog(cpu, cpuInfo[i], "big_core", logCrashdump);
        if (logSection)
        {
            // Include the version
            logCrashdumpVersion(logSection, cpuInfo[i],
                                record_type::coreCrashLog);
        }

        // Fill in the SQ dump
        addSectionLog(cpu, cpuInfo[i], "big_core", logSqDump);

        // Fill in the Core MCA data
        addSectionLog(cpu, cpuInfo[i], "MCA", logCoreMca);

        // Fill in the Uncore MCA data
        logSection = addSectionLog(cpu, cpuInfo[i], "MCA", logUncoreMca);
        if (logSection)
        {
            // Include the version
            logCrashdumpVersion(logSection, cpuInfo[i], record_type::mcaLog);
        }

        // Fill in the System Info metadata
        logSection =
            addSectionLog(crashlogData, cpuInfo[i], "METADATA", logSysInfo);
        if (logSection)
        {
            // Include the version and timestamp fields
            logCrashdumpVersion(logSection, cpuInfo[i], record_type::metadata);
            logTimestamp(logSection);
            logTriggerType(logSection);
            logPlatformName(logSection);
        }

        // Fill in the Uncore Status
        logSection = addSectionLog(cpu, cpuInfo[i], "uncore", logUncoreStatus);

        // Fill in the TOR Dump
        logSection = addSectionLog(cpu, cpuInfo[i], "TOR", logTorDump);
        if (logSection)
        {
            // Include the version
            logCrashdumpVersion(logSection, cpuInfo[i], record_type::torDump);
        }

        // Fill in the Power Management Info
        logSection =
            addSectionLog(cpu, cpuInfo[i], "PM_info", logPowerManagement);
        if (logSection)
        {
            // Include the version
            logCrashdumpVersion(logSection, cpuInfo[i], record_type::pmInfo);
        }

        // Fill in the Address Map
        logSection =
            addSectionLog(cpu, cpuInfo[i], "address_map", logAddressMap);
        if (logSection)
        {
            // Include the version
            logCrashdumpVersion(logSection, cpuInfo[i],
                                record_type::addressMap);
        }
    }

    out = cJSON_PrintUnformatted(root);
    if (out != NULL)
    {
        crashdumpContents = out;
        cJSON_free(out);
        fprintf(stderr, "Completed!\n");
    }
    else
    {
        fprintf(stderr, "cJSON_Print Failed\n");
    }

    cJSON_Delete(root);
}

static int scandir_filter(const struct dirent* dirEntry)
{
    // Filter for just the crashdump files
    if (strncmp(dirEntry->d_name, "crashdump_", 9) == 0)
    {
        return 1;
    }
    return 0;
}

static void newOnDemandLog(std::string& crashdumpContents)
{
    // Start the log to the on-demand file
    createCrashdump(crashdumpContents);
}

static void incrementCrashdumpCount()
{
    // Get the current count
    conn->async_method_call(
        [](boost::system::error_code ec,
           const std::variant<uint8_t>& property) {
            if (ec)
            {
                fprintf(stderr, "Failed to get Crashdump count\n");
                return;
            }
            const uint8_t* crashdumpCountVariant =
                std::get_if<uint8_t>(&property);
            if (crashdumpCountVariant == nullptr)
            {
                fprintf(stderr, "Unable to read Crashdump count\n");
                return;
            }
            uint8_t crashdumpCount = *crashdumpCountVariant;
            if (crashdumpCount == std::numeric_limits<uint8_t>::max())
            {
                fprintf(stderr, "Maximum crashdump count reached\n");
                return;
            }
            // Increment the count
            crashdumpCount++;
            conn->async_method_call(
                [](boost::system::error_code ec) {
                    if (ec)
                    {
                        fprintf(stderr, "Failed to set Crashdump count\n");
                    }
                },
                "xyz.openbmc_project.Settings",
                "/xyz/openbmc_project/control/processor_error_config",
                "org.freedesktop.DBus.Properties", "Set",
                "xyz.openbmc_project.Control.Processor.ErrConfig",
                "CrashdumpCount", std::variant<uint8_t>{crashdumpCount});
        },
        "xyz.openbmc_project.Settings",
        "/xyz/openbmc_project/control/processor_error_config",
        "org.freedesktop.DBus.Properties", "Get",
        "xyz.openbmc_project.Control.Processor.ErrConfig", "CrashdumpCount");
}

constexpr int numStoredLogs = 2;
static void newStoredLog(
    sdbusplus::asio::object_server& server,
    std::vector<std::pair<std::string,
                          std::shared_ptr<sdbusplus::asio::dbus_interface>>>&
        logIfaces)
{
    constexpr char const* crashdumpFile = "crashdump_%llu.json";
    struct dirent** namelist;
    uint64_t crashdump_num = 0;
    FILE* fpJson = NULL;
    std::error_code ec;

    // create the crashdumps directory if it doesn't exist
    if (!(std::experimental::filesystem::create_directories(crashdumpDir, ec)))
    {
        if (ec.value() != 0)
        {
            fprintf(stderr, "failed to create %s: %s\n", crashdumpDir.c_str(),
                    ec.message().c_str());
            return;
        }
    }

    // Search the crashdumps directory for existing log files
    int n =
        scandir(crashdumpDir.c_str(), &namelist, scandir_filter, versionsort);
    if (n < 0)
    {
        // scandir failed, so print the error
        perror("scandir");
        return;
    }

    // If this is the first cpu log, just use 0
    if (n > 0)
    {
        // otherwise, get the number of the last log and increment it
        sscanf_s(namelist[n - 1]->d_name, crashdumpFile, &crashdump_num);
        crashdump_num++;
    }
    // Go through each file in directory
    for (int i = 0; i < n; i++)
    {
        // If it's below the number of saved logs, delete it
        if (i < (n - numStoredLogs))
        {
            std::error_code ec;
            if (!(std::experimental::filesystem::remove(
                    crashdumpDir / namelist[i]->d_name, ec)))
            {
                fprintf(stderr, "failed to remove %s: %s\n",
                        namelist[i]->d_name, ec.message().c_str());
            }
            // Now remove the interface for the deleted log
            logIfaces.erase(
                std::remove_if(
                    logIfaces.begin(), logIfaces.end(),
                    [&server, &namelist, &i](auto& log) {
                        if (std::get<0>(log).compare(namelist[i]->d_name) == 0)
                        {
                            server.remove_interface(std::get<1>(log));
                            return true;
                        }
                        return false;
                    }),
                logIfaces.end());
        }
        // Free the file data
        free(namelist[i]);
    }
    // Free the scandir data
    free(namelist);

    // Create the new cpu log filename
    char new_log_filename[256];
    cd_snprintf_s(new_log_filename, sizeof(new_log_filename), crashdumpFile,
                  crashdump_num);
    std::experimental::filesystem::path out_file =
        crashdumpDir / new_log_filename;

    // Start the log to the new file
    std::string crashdumpContents;
    createCrashdump(crashdumpContents);

    // open the JSON file for CPU dump
    fpJson = fopen(out_file.c_str(), "w");
    if (fpJson != NULL)
    {
        fprintf(fpJson, "%s", crashdumpContents.c_str());
        fclose(fpJson);
    }

    // Add the new interface for this log
    std::experimental::filesystem::path path =
        std::experimental::filesystem::path(crashdumpPath) /
        std::to_string(crashdump_num);
    std::shared_ptr<sdbusplus::asio::dbus_interface> ifaceLog =
        server.add_interface(path.c_str(), crashdumpInterface);
    logIfaces.emplace_back(new_log_filename, ifaceLog);
    // Log Property
    ifaceLog->register_property("Log", crashdumpContents);
    ifaceLog->initialize();

    // Increment the count for this completed crashdump
    incrementCrashdumpCount();
}

static int parseLogEntry(const std::string& filename,
                         std::string& crashdumpContents)
{
    if (!std::experimental::filesystem::exists(filename))
    {
        crashdumpContents = "CPU log not found";
        return 1;
    }
    std::ifstream inf(filename);
    if (inf.bad())
    {
        crashdumpContents = "failed to open CPU log";
        return 1;
    }
    std::stringstream buffer;
    buffer << inf.rdbuf();
    crashdumpContents = buffer.str();
    return 0;
}

static bool isPECIAvailable()
{
    std::vector<CPUInfo> cpuInfo;
    getClientAddrs(cpuInfo);
    if (cpuInfo.empty())
    {
        fprintf(stderr, "PECI is not available!\n");
        return false;
    }
    return true;
}

/** Exception for when a log is attempted while power is off. */
struct PowerOffException final : public sdbusplus::exception_t
{
    const char* name() const noexcept override
    {
        return "org.freedesktop.DBus.Error.NotSupported";
    };
    const char* description() const noexcept override
    {
        return "Power off, cannot access peci";
    };
    const char* what() const noexcept override
    {
        return "org.freedesktop.DBus.Error.NotSupported: "
               "Power off, cannot access peci";
    };
};
} // namespace crashdump

int main(int argc, char* argv[])
{
    // future to use for long-running tasks
    std::future<void> future;
    // vector to store log interfaces
    std::vector<std::pair<std::string,
                          std::shared_ptr<sdbusplus::asio::dbus_interface>>>
        logIfaces(crashdump::numStoredLogs + 1);
    // setup connection to dbus
    crashdump::conn =
        std::make_shared<sdbusplus::asio::connection>(crashdump::io);
    std::string onDemandLogContents;

    // CPU Debug Log Object
    crashdump::conn->request_name(crashdump::crashdumpService);
    auto server = sdbusplus::asio::object_server(crashdump::conn);

    // Stored Log Interface
    std::shared_ptr<sdbusplus::asio::dbus_interface> ifaceStored =
        server.add_interface(crashdump::crashdumpPath,
                             crashdump::crashdumpStoredInterface);

    // Generate a Stored Log: This is test method that should be removed
    ifaceStored->register_method(
        "GenerateStoredLog", [&server, &logIfaces, &future]() {
            if (!crashdump::isPECIAvailable())
            {
                throw crashdump::PowerOffException();
            }
            if (future.valid() && future.wait_for(std::chrono::seconds(0)) !=
                                      std::future_status::ready)
            {
                return std::string("Log in Progress");
            }
            future = std::async(std::launch::async, [&server, &logIfaces]() {
                crashdump::newStoredLog(server, logIfaces);
            });
            return std::string("Log Started");
        });
    ifaceStored->initialize();

    // OnDemand Log Interface
    std::shared_ptr<sdbusplus::asio::dbus_interface> ifaceImm =
        server.add_interface(crashdump::crashdumpPath,
                             crashdump::crashdumpOnDemandInterface);

    // Generate an OnDemand Log
    ifaceImm->register_method("GenerateOnDemandLog", [&server, &future,
                                                      &onDemandLogContents]() {
        if (!crashdump::isPECIAvailable())
        {
            throw crashdump::PowerOffException();
        }
        // Check if a Log is in progress
        if (future.valid() && future.wait_for(std::chrono::seconds(0)) !=
                                  std::future_status::ready)
        {
            return std::string("Log in Progress");
        }
        // Start the log asynchronously since it can take a long time
        future =
            std::async(std::launch::async, [&server, &onDemandLogContents]() {
                static std::shared_ptr<sdbusplus::asio::dbus_interface>
                    ifaceLogImm = nullptr;
                if (ifaceLogImm != nullptr)
                {
                    server.remove_interface(ifaceLogImm);
                }
                ifaceLogImm =
                    server.add_interface(crashdump::crashdumpOnDemandPath,
                                         crashdump::crashdumpInterface);
                crashdump::newOnDemandLog(onDemandLogContents);
                // Log Property
                ifaceLogImm->register_property("Log", onDemandLogContents);
                ifaceLogImm->initialize();
            });
        // Return success
        return std::string("Log Started");
    });
    ifaceImm->initialize();

    // Build up paths for any existing stored logs
    if (std::experimental::filesystem::exists(crashdump::crashdumpDir))
    {
        std::regex search("crashdump_(\\d+).json");
        std::smatch match;
        for (auto& p : std::experimental::filesystem::directory_iterator(
                 crashdump::crashdumpDir))
        {
            std::string file = p.path().filename();
            if (std::regex_match(file, match, search))
            {
                // Log Interface
                std::experimental::filesystem::path path =
                    std::experimental::filesystem::path(
                        crashdump::crashdumpPath) /
                    match.str(1);
                std::shared_ptr<sdbusplus::asio::dbus_interface> ifaceLog =
                    server.add_interface(path.c_str(),
                                         crashdump::crashdumpInterface);
                logIfaces.emplace_back(file, ifaceLog);
                // Log Property
                std::string crashdumpContents;
                crashdump::parseLogEntry(p.path().string(), crashdumpContents);
                ifaceLog->register_property("Log", crashdumpContents);
                ifaceLog->initialize();
            }
        }
    }

    // Send Raw PECI Interface
    std::shared_ptr<sdbusplus::asio::dbus_interface> ifaceRawPeci =
        server.add_interface(crashdump::crashdumpPath,
                             crashdump::crashdumpRawPeciInterface);

    // Send a Raw PECI command
    ifaceRawPeci->register_method(
        "SendRawPeci", [](const uint8_t& clientAddr, const uint8_t& readLen,
                          const std::vector<uint8_t>& rawCmd) {
            if (readLen > PECI_BUFFER_SIZE)
            {
                throw std::invalid_argument("Read length too large");
            }
            std::vector<uint8_t> rawResp(readLen);
            peci_raw(clientAddr, readLen, rawCmd.data(), rawCmd.size(),
                     rawResp.data(), rawResp.size());
            return rawResp;
        });
    ifaceRawPeci->initialize();

    crashdump::io.run();

    return 0;
}
