/*===================== begin_copyright_notice ==================================

 Copyright (c) 2021, Intel Corporation


 Permission is hereby granted, free of charge, to any person obtaining a
 copy of this software and associated documentation files (the "Software"),
 to deal in the Software without restriction, including without limitation
 the rights to use, copy, modify, merge, publish, distribute, sublicense,
 and/or sell copies of the Software, and to permit persons to whom the
 Software is furnished to do so, subject to the following conditions:

 The above copyright notice and this permission notice shall be included
 in all copies or substantial portions of the Software.

 THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
 OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
 THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR
 OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
 ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
 OTHER DEALINGS IN THE SOFTWARE.
======================= end_copyright_notice ==================================*/

#include <thread>
#include <sstream>

#include "emu_cfg.h"
#include "emu_api_export.h"
#include "emu_log.h"
#include "emu_platform.h"

namespace GfxEmu {

namespace Log {
namespace Flags {
GFX_EMU_API_IMPORT Type regexToFlags (const std::string& rxStr, const bool setLevel);
};
};

GFX_EMU_API Cfg_& Cfg() {
    static Cfg_ cfg;
    return cfg;
};

const int64_t Cfg_::Param::kIntValueInvalid = std::numeric_limits<int64_t>::max ( );
const double Cfg_::Param::kFpValueInvalid = std::numeric_limits<double>::max ( );

//------------------------------------------

GfxEmu::Log::Flags::Type updateLogCfg(Cfg_::Param& p, const bool setLevel) {

    if(!p.isSettingDefaults()) {
        int64_t tmpFlags = 0; //p.getDefaultInt();

        std::stringstream in {GfxEmu::Utils::removeSpace(p.getStr ())};
        std::string submatch;

        while(std::getline(in,submatch,',')) {
            bool inverse = false;
            if(submatch[0] == '~') {
                inverse = true;
                submatch.erase(0,1);
            }

            if(!std::regex_search(
                submatch,
                std::regex{"\\S"}
            )) continue;

            const auto submatchFlags = GfxEmu::Log::Flags::regexToFlags(submatch, setLevel);

            if(inverse)
                tmpFlags &= ~submatchFlags;
            else
                tmpFlags |= submatchFlags;

            //GFX_EMU_MESSAGE("submatch result: %s inverse?:%u submatchFlags:%llx tmpFlags:%llx\n",
            //    submatch.c_str(), inverse, submatchFlags, tmpFlags);
        }

        p.setSubValue(tmpFlags);
    }

    p.setSubValue(GfxEmu::Log::Flags::toStr(p.getInt ()));
    return p.getInt ();
}

//------------------------------------------

auto getCfgParamsRegistry = []()->auto& {
    static std::vector<const Cfg_::Param*> g_cfgParams;
    return g_cfgParams;
};

void Cfg_::Param::addToRegistry() {
    getCfgParamsRegistry ().emplace_back(this);
}

GFX_EMU_API
Cfg_::Cfg_() :
    LogFile {
        "log file",
        "a file to where log messages shall be redirected",
        {"EMU_LOG_FILE", ""},
        "",
        [](Param& p){
            if(!p.isSettingDefaults ()) {
                if(std::regex_search(
                    p.getStr (),
                    std::regex{"\\w"}
                )) {
                    GfxEmu::Log::setLogFile(p.getStr ());
                } else
                    GFX_EMU_FAIL_WITH_MESSAGE(fCfg, "bad log file name %s containing no word charachters.\n",
                        p.getStr ().c_str());
            }
            return true;
        }
    }
    ,LogChannels {
        "log channels",
        "enable/disable logging topic channels",
        {"EMU_LOG_CHANNELS",""},
        GfxEmu::CfgCache::LogChannels.load(),
        [](Param& p){
            GfxEmu::CfgCache::LogChannels.store(
                GfxEmu::updateLogCfg(p, false)
            );
            return true;
        }
    }
    ,LogLevel {
        "log level",
        "set minimum message importance level",
        {"EMU_LOG_LEVEL",""},
        GfxEmu::CfgCache::MinimalLevel.load(),
        [](Param& p){
            GfxEmu::CfgCache::MinimalLevel.store(
                GfxEmu::updateLogCfg(p, true)
            );
            return true;
        }
    }
    ,Platform {
        "HW platform",
        "",
        {"CM_RT_PLATFORM",""},
        "SKL",
        [](Param& p) {
            p.set(GfxEmu::Utils::toLower(p.getStr ()));
            const auto it = GfxEmu::Platform::StaticData_nameToInt ().find(GfxEmu::Utils::toUpper(p.getStr ()));
            if(it == GfxEmu::Platform::StaticData_nameToInt ().end ()) {
                p.setSubValue(GfxEmu::Platform::UNDEFINED);
                return false;
            }
            p.setSubValue(it->second);
            return true;
        },
        "specified platform is unknown"
    }
    ,Sku {
        "SKU name",
        "",
        {"CM_RT_SKU",""},
        "undefined",
        [](Param& p) {
            p.set(GfxEmu::Utils::toLower(p.getStr ()));
            const auto it = GfxEmu::Platform::Sku::StaticData_nameToInt ().find(GfxEmu::Utils::toUpper(p.getStr ()));
            if(it == GfxEmu::Platform::Sku::StaticData_nameToInt ().end ()) {
                p.setSubValue(GfxEmu::Platform::Sku::UNDEFINED);
                return p.isSettingDefaults () ? true : false;
            }
            p.setSubValue(it->second);
            return true;
        },
        "specified sku unknown"
    }
    ,ParallelThreads {
        "parallel work-items (kernel threads) limit",

        "Controls how many work-items shall be allowed to be in running state at the same time"

        "In multi-thread work-items scheduling mode "
        "the value of 1 is a special case in which kernel threads shall be scheduled in predefined sequential order."

        , {"CM_RT_PARALLEL_THREADS","--emu-parallel-threads"},
        std::thread::hardware_concurrency(),
        [](Param& p) {
            if(p.getInt () > std::thread::hardware_concurrency())
                GFX_EMU_MESSAGE(fCfg,
                    "notice: there are less HW cores available (%u) than requested to run parallel OS threads (%u). "
                    "This is not advised as will lead to a poorer performance.\n",
                        std::thread::hardware_concurrency(), p.getInt ());
            return p.getInt() > 0;
        },
        "parallel threads number must be > 0"
    }
    ,ResidentGroups {
        "resident work-groups limit",
        "how many work-group resources shall be allocated for parallel/concurrent execution",
        {"CM_RT_RESIDENT_GROUPS","--emu-resident-groups"},
        1,
        [](Param& p) {return p.getInt() > 0;},
        "resident groups number must be > 0"
    }
    {
        //GfxEmu::Log::printBacktrace ();
        printSummary ();
    };

Cfg_::~Cfg_ () {
    //printSummary ();
}

GFX_EMU_API const std::vector<const Cfg_::Param*>& Cfg_::getParamsRegistry () const {
    return getCfgParamsRegistry ();
}

GFX_EMU_API void Cfg_::printSummary () {
    GFX_EMU_MESSAGE(fCfg, "--- configuration params summary --- \n")
    for(const auto& p: getParamsRegistry ())
        GFX_EMU_MESSAGE(fCfg, "all params > %s %s\n",
            p->getName ().c_str (), p->getDbgStr ().c_str ());
    GFX_EMU_MESSAGE(fCfg, "--- user-defined --- \n")
    for(const auto& p: getParamsRegistry ())
        if(p->isUserDefined ())
            GFX_EMU_MESSAGE(fCfg, "user-defined > %s %s\n",
                p->getName ().c_str (), p->getDbgStr ().c_str ());
    GFX_EMU_MESSAGE(fCfg, "------------------------------------ \n")
}

}; // namespace GfxEmu

