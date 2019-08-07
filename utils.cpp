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

#include "utils.hpp"

#include <boost/container/flat_map.hpp>
#include <sdbusplus/bus.hpp>
#include <sdbusplus/message.hpp>
#include <variant>

#ifdef __cplusplus
extern "C" {
#endif

#include <safe_mem_lib.h>
#include <safe_str_lib.h>

#ifdef __cplusplus
}
#endif

namespace crashdump
{
int getBMCVersionDBus(char *bmcVerStr, size_t bmcVerStrSize)
{
    using ManagedObjectType = boost::container::flat_map<
        sdbusplus::message::object_path,
        boost::container::flat_map<
            std::string, boost::container::flat_map<
                             std::string, std::variant<std::string>>>>;

    if (bmcVerStr == nullptr)
    {
        return 1;
    }

    sdbusplus::bus::bus dbus = sdbusplus::bus::new_default_system();
    sdbusplus::message::message getObjects = dbus.new_method_call(
        "xyz.openbmc_project.Software.BMC.Updater",
        "/xyz/openbmc_project/software", "org.freedesktop.DBus.ObjectManager",
        "GetManagedObjects");
    ManagedObjectType bmcUpdaterIntfs;
    try
    {
        sdbusplus::message::message resp = dbus.call(getObjects);
        resp.read(bmcUpdaterIntfs);
    }
    catch (sdbusplus::exception_t &e)
    {
        return 1;
    }

    for (const std::pair<
             sdbusplus::message::object_path,
             boost::container::flat_map<
                 std::string, boost::container::flat_map<
                                  std::string, std::variant<std::string>>>>
             &pathPair : bmcUpdaterIntfs)
    {
        boost::container::flat_map<
            std::string,
            boost::container::flat_map<
                std::string, std::variant<std::string>>>::const_iterator
            softwareVerIt =
                pathPair.second.find("xyz.openbmc_project.Software.Version");
        if (softwareVerIt != pathPair.second.end())
        {
            boost::container::flat_map<std::string, std::variant<std::string>>::
                const_iterator versionIt =
                    softwareVerIt->second.find("Version");
            if (versionIt != softwareVerIt->second.end())
            {
                const std::string *bmcVersion =
                    std::get_if<std::string>(&versionIt->second);
                if (bmcVersion != nullptr)
                {
                    size_t copySize =
                        std::min(bmcVersion->size(), bmcVerStrSize - 1);
                    bmcVersion->copy(bmcVerStr, copySize);
                    return 0;
                }
            }
        }
    }
    return 1;
}
} // namespace crashdump

int cd_snprintf_s(char *str, size_t len, const char *format, ...)
{
    int ret;
    va_list args;
    va_start(args, format);
    ret = vsnprintf_s(str, len, format, args);
    va_end(args);
    return ret;
}
