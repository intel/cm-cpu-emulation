/*========================== begin_copyright_notice ============================

Copyright (C) 2021 Intel Corporation

SPDX-License-Identifier: MIT

============================= end_copyright_notice ===========================*/

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

namespace Cfg {
const int64_t Param::kIntValueInvalid = std::numeric_limits<int64_t>::max ( );
const double Param::kFpValueInvalid = std::numeric_limits<double>::max ( );

//------------------------------------------

GfxEmu::Log::Flags::Type updateLogCfg(Cfg::Param& p, const bool setLevel) {

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
    static std::vector<const Cfg::Param*> g_cfgParams;
    return g_cfgParams;
};

void Param::addToRegistry() {
    getCfgParamsRegistry ().emplace_back(this);
}

constexpr bool kFibersEnabled =
    false;

#define CFG_PARAM(n,...)\
    GFX_EMU_API GfxEmu::Cfg::Param& n () { static auto& p_ = *(new GfxEmu::Cfg::Param {__VA_ARGS__}); return p_; }
#include <emu_cfg_params.h>
#undef CFG_PARAM

void* init () {
#define CFG_PARAM(n,...) n ();
#include <emu_cfg_params.h>
#undef CFG_PARAM
    //GfxEmu::Log::printBacktrace ();
    printSummary ();
    return nullptr;
}

GFX_EMU_API const std::vector<const Cfg::Param*>& getParamsRegistry () {
    return getCfgParamsRegistry ();
}

GFX_EMU_API void printSummary () {
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

}; // namespace Cfg
}; // namespace GfxEmu
