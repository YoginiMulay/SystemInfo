/**
* If not stated otherwise in this file or this component's LICENSE
* file the following copyright and licenses apply:
*
* Copyright 2020 RDK Management
*
* Licensed under the Apache License, Version 2.0 (the "License");
* you may not use this file except in compliance with the License.
* You may obtain a copy of the License at
*
* http://www.apache.org/licenses/LICENSE-2.0
*
* Unless required by applicable law or agreed to in writing, software
* distributed under the License is distributed on an "AS IS" BASIS,
* WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
* See the License for the specific language governing permissions and
* limitations under the License.
**/

#include "SystemInfo.h"

#include "UtilsIarm.h"
#include "UtilsJsonRpc.h"
#include "UtilsString.h"
#include "UtilscRunScript.h"
#include "UtilsgetRFCConfig.h"

using namespace std;

#define API_VERSION_NUMBER_MAJOR 1
#define API_VERSION_NUMBER_MINOR 0
#define API_VERSION_NUMBER_PATCH 0

/* SystemInfo Based Macros & Structures */
#define IARM_BUS_SYS_INFO_NAME "SYSTEM_INFO"
#define IARM_BUS_SYSTEMINFO_API_isAvailable "isSystemInfoAvailable"


#define registerMethod(...) for (uint8_t i = 1; GetHandler(i); i++) GetHandler(i)->Register<JsonObject, JsonObject>(__VA_ARGS__)

namespace WPEFramework
{
    namespace {
        static Plugin::Metadata<Plugin::SystemInfo> metadata(
            // Version (Major, Minor, Patch)
            API_VERSION_NUMBER_MAJOR, API_VERSION_NUMBER_MINOR, API_VERSION_NUMBER_PATCH,
            // Preconditions
            {},
            // Terminations
            {},
            // Controls
            {}
        );
    }

    namespace Plugin
    {
        SERVICE_REGISTRATION(SystemInfo, API_VERSION_NUMBER_MAJOR, API_VERSION_NUMBER_MINOR, API_VERSION_NUMBER_PATCH);
        SystemInfo* SystemInfo::_instance = nullptr;

        SystemInfo::SystemInfo()
        : PluginHost::JSONRPC()
        {
            SystemInfo::_instance = this;

            CreateHandler({2});

            // Quirk
            registerMethod("getQuirks", &SystemInfo::getQuirks, this);

            // SystemInfo_API_Version_1
            registerMethod("isSystemInfoAvailable", &SystemInfo::isSystemInfoAvailable, this);
        }

        SystemInfo::~SystemInfo()
        {
        }

        const string SystemInfo::Initialize(PluginHost::IShell*  service )
        {
            string msg;
            if (!Utils::IARM::init())
            {
                msg = "IARM bus is not available";
                LOGERR("IARM bus is not available. Failed to activate SystemInfo Plugin");
            }
            else
            {
                msg = "SystemInfo Initialization successful";
            }
            return msg;
        }

        void SystemInfo::Deinitialize(PluginHost::IShell* /* service */)
        {
            Unregister("getQuirks");
            Unregister("isSystemInfoAvailable");

            SystemInfo::_instance = nullptr;
        }

        string SystemInfo::Information() const
        {
             return(string());
        }

        // Wrapper methods
        uint32_t SystemInfo::getQuirks(const JsonObject& parameters, JsonObject& response)
        {
            JsonArray array;
            array.Add("RDK-20093");
            response["quirks"] = array;
            returnResponse(true);
        }

        uint32_t SystemInfo::isSystemInfoAvailable (const JsonObject& parameters, JsonObject& response)
        {
            bool result = false;
      
            IARM_BUS_NetSrvMgr_Iface_EventData_t param = {0};
            strncpy(param.setInterface, interface.c_str(), INTERFACE_SIZE);
            if (IARM_RESULT_SUCCESS == IARM_Bus_Call (IARM_BUS_SYS_INFO_NAME, IARM_BUS_SYSTEMINFO_API_isAvailable, (void*)&param, sizeof(param)))
            {
                LOGINFO("%s :: Enabled = %d ",__FUNCTION__,param.isInterfaceEnabled);
                response["enabled"] = param.isInterfaceEnabled;
                result = true;
            }
            else
                LOGWARN ("Call to %s for %s failed", IARM_BUS_SYS_INFO_NAME, IARM_BUS_SYSTEMINFO_API_isAvailable);

            returnResponse(result);
        }
    } // namespace Plugin
} // namespace WPEFramework
