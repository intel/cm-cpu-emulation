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

#include "emu_log.h"
#ifdef GFX_EMU_DEBUG_SYMBOLS_ACCESS_ENABLED
#include "emu_dbgsymb.h"
#endif
#include "emu_utils.h"

#include <unordered_map>
#include <map>
#include <atomic>
#include <mutex>
#include <iostream>
#include <sstream>
#include <string>

#include <dlfcn.h>
#include <cxxabi.h>
#include <execinfo.h>
//#include <unistd.h>
//#include <stdio.h>

#include <csignal>

#include "emu_cfg.h"
#include "emu_utils.h"

namespace GfxEmu {
namespace Log {

void setLogFile(const std::string& file) {
    if(LogFile_ != stdout)
        std::fclose(LogFile_);

    LogFile_ = std::fopen(file.c_str(), GfxEmu::CfgCache::LogFileMode.c_str());
    if(!LogFile_)
        GFX_EMU_FAIL_WITH_MESSAGE("Couldn't set log file to %s, error: %s",
            file.c_str(), GfxEmu::Utils::lastErrorStr ().c_str());
}

void adviceToEnable(Flags::Type flag, const std::string& msg) {
    if(!Flags::isEnabled(flag)) {
        GFX_EMU_MESSAGE(Flags::fCfg | flag | Flags::fSticky,
            std::string{"Enable %s logging channel "} + msg,
                Flags::toStr(flag));
    }
}

namespace Flags {

// --> static data wrappers to ensure initialization.
auto StaticData_thread_local_enabledFlags = []()->auto& {
    thread_local static std::map <Type,bool> map_;
    return map_;
};

auto StaticData_msgToFlagMap = []()-> const auto& {
    static const std::vector<std::pair<const char*,Flags::Type>> msgToFlag = {
#define GFX_EMU_DEBUG_MINIMAL_LEVEL(...)
#define GFX_EMU_DEBUG_LEVEL(msg,constant,num) { msg, Flags:: constant },
#define GFX_EMU_DEBUG_FLAG(msg,constant,shift,enabled) { msg, Flags:: constant },
#include "emu_log_flags.h"
#undef GFX_EMU_DEBUG_MINIMAL_LEVEL
#undef GFX_EMU_DEBUG_LEVEL
#undef GFX_EMU_DEBUG_FLAG
    };
    return msgToFlag;
};
// <-- static data wrappers to ensure initialization.

GFX_EMU_API bool isEnabled (Type f) {
    auto& cache = StaticData_thread_local_enabledFlags();
    try { return cache.at(f); }
    catch (...) {
        cache[f] =
               (f & (fSticky | fCritical)) || ( // Sticky and Critical messages are always displayed.
                   (f & GfxEmu::CfgCache::LogChannels) &&
                   (!(f & fLevelsMask) ||
                        (
                            (f & fLevelsMask) >= GfxEmu::CfgCache::MinimalLevel
                        )
                    )
               );
        return cache[f];
    }
}

GFX_EMU_API Type regexToFlags (const std::string& rxStr, const bool setLevel) {
    StaticData_thread_local_enabledFlags ().clear ();

    Type flags = 0, curLevelSeen = 0, prevLevelSeen = 0;

    const auto rx = std::regex{rxStr, std::regex::ECMAScript | std::regex::icase};

    for (const auto& msg2flag: StaticData_msgToFlagMap ()) {
        if(std::regex_search(msg2flag.first,rx)) {
            if (setLevel && (curLevelSeen = (msg2flag.second & fLevelsMask))) {
                if(prevLevelSeen && curLevelSeen >= prevLevelSeen)
                    continue;
                flags &= ~prevLevelSeen;
                prevLevelSeen = curLevelSeen;
            }
            flags |= msg2flag.second;
        }
    }

    if(setLevel)
        flags &= fLevelsMask;
    else
        flags &= ~fLevelsMask;

    return flags;
}

GFX_EMU_API const char* toStr(const GfxEmu::Log::Flags::Type flags) {
    if(!flags) return "";

    static_assert(std::is_same_v<GfxEmu::Log::Flags::Type,uint64_t>);
#define GFX_EMU_DEBUG_MINIMAL_LEVEL(...)
#define GFX_EMU_DEBUG_LEVEL(msg,constant,num) { Flags:: constant , msg },
#define GFX_EMU_DEBUG_FLAG(msg,constant,shift,enabled) { Flags:: constant , msg },
        static std::map<Flags::Type, std::string>
            flagsToStr_ {
#include "emu_log_flags.h"
       };
#undef GFX_EMU_DEBUG_MINIMAL_LEVEL
#undef GFX_EMU_DEBUG_LEVEL
#undef GFX_EMU_DEBUG_FLAG
    static std::unordered_map<
        GfxEmu::Log::Flags::Type,
        std::string
    > cache_ = {{0, ""}};

    try { return cache_.at(flags).c_str(); }
    catch (...) {
        static std::mutex cacheLock;
        std::lock_guard guard {cacheLock};
        const auto it = cache_.find(flags);
        if(it != cache_.end ()) return it->second.c_str ();
        auto& out = cache_[flags];
        for(const auto& fl: flagsToStr_) {
            if(flags & fl.first)
                out += out.size () ?
                    ( fl.first == fSticky ?
                        "*" :
                        std::string(",") + fl.second
                    ) :
                    fl.second;
        }
        return out.c_str ();
    }
}

};

#define SIG_AND_NAME(v) {v, #v}
const static std::unordered_map<int,const char*> signals = {
        SIG_AND_NAME(SIGABRT),
        SIG_AND_NAME(SIGSEGV),
        SIG_AND_NAME(SIGBUS),
        SIG_AND_NAME(SIGILL),
        SIG_AND_NAME(SIGFPE)
    };
#undef SIG_AND_NAME

static std::atomic_flag backtraceOnAbortsFlag;

void SigHandler_(int signum, siginfo_t* si, void* unused )
{
    if(backtraceOnAbortsFlag.test_and_set ()) return; // only first failed thread, or the below shall hang.

    GfxEmu::ErrorMessage("--------------------------------------------------------------------------\n");
    GfxEmu::FailWithMessage<true>(
        signum,
        "Received signal %u %s. Printing backtrace and terminating.\n\n",
        signum,
        signals.at(signum)
    );
}

struct HandleAborts_ {
    HandleAborts_ ()
    {
        struct sigaction sa;
        sa.sa_flags = SA_SIGINFO;
        sa.sa_sigaction = SigHandler_;
        sigemptyset( &sa.sa_mask );

        for (const auto& signal: signals) {
            sigaction(signal.first, &sa, NULL);
        }
    }
};

inline static HandleAborts_ ha;

void printBacktrace() {

#ifndef GFX_EMU_DEBUG_ENABLED
    return;
#endif

    GFX_EMU_MESSAGE("------------------- begin backtrace -------------------\n");

#ifdef GFX_EMU_DEBUG_SYMBOLS_ACCESS_ENABLED

    void *callstack[128];
    const int nMaxFrames = sizeof(callstack) / sizeof(callstack[0]);
    char buf[1024];
    int nFrames = backtrace(callstack, nMaxFrames);
    char **symbols = backtrace_symbols(callstack, nFrames);

    if(!symbols) return;

    std::stringstream traceStream;
    for (int i = 0; i < nFrames; i++) {
        Dl_info info;

        if (dladdr(callstack[i], &info) && info.dli_sname) {
            char *demangled = nullptr;

            int status = -1;
            if (info.dli_sname[0] == '_')
                demangled = abi::__cxa_demangle(info.dli_sname, NULL, 0, &status);

            snprintf(buf, sizeof(buf),
                "%d: %s + %zd ",
                i,
                status == 0 ? demangled :
                              info.dli_sname == 0 ?
                              symbols[i] : info.dli_sname,
                (char *)callstack[i] - (char *)info.dli_saddr
            );

            free(demangled);

            traceStream << buf;
            if(symbols) traceStream << symbols[i] << "\n";
        } else {
            snprintf(buf, sizeof(buf), "%d %s\n",
                     i, symbols[i]);
            traceStream << buf;
        }

        traceStream << "\n";
    }

    free(symbols);

    if (nFrames == nMaxFrames)
        traceStream << "[truncated]\n";

    GFX_EMU_MESSAGE(traceStream.str());
    GFX_EMU_MESSAGE("------------------- end backtrace -------------------\n");
#endif // GFX_EMU_DEBUG_SYMBOLS_ACCESS_ENABLED
}
};
};

