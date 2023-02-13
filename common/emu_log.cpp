/*========================== begin_copyright_notice ============================

Copyright (C) 2021 Intel Corporation

SPDX-License-Identifier: MIT

============================= end_copyright_notice ===========================*/

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

#ifndef _WIN32

#include <dlfcn.h>
#include <cxxabi.h>
#include <execinfo.h>

#endif

#include <csignal>

#include "emu_cfg.h"
#include "emu_utils.h"

namespace GfxEmu {
namespace Log {

    MessagePrefixGuard::MessagePrefixGuard(const std::string& p) {
        prevPrefix.push (curPrefix);
        curPrefix = prevPrefix.top() + p;
    }

    MessagePrefixGuard::~MessagePrefixGuard() {
        curPrefix = prevPrefix.top ();
        prevPrefix.pop ();
    }

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
        GFX_EMU_MESSAGE(Flags::fCfg | flag,
            std::string{"Enable %s logging channel "} + msg,
                Flags::toStr(flag));
    }
}

namespace Flags {

std::map<Type,bool>& StaticData_thread_local_enabledFlags () {
    thread_local static auto& enabledFlags = *(new std::map <Type,bool>);
    return enabledFlags;
};

const std::vector<std::pair<const char*,Flags::Type>>& StaticData_msgToFlagMap () {
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
           (f & GfxEmu::CfgCache::LogChannels) &&
           (!(f & fLevelsMask) ||
                (
                    (f & fLevelsMask) >= GfxEmu::CfgCache::MinimalLevel
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
        static const auto& flagsToStr_ = *(new std::map<Flags::Type, std::string>
           {
#include "emu_log_flags.h"
           }
        );
#undef GFX_EMU_DEBUG_MINIMAL_LEVEL
#undef GFX_EMU_DEBUG_LEVEL
#undef GFX_EMU_DEBUG_FLAG

    thread_local static auto& cache_ = *(new std::unordered_map<
        GfxEmu::Log::Flags::Type,
        std::string
    > ({{0, ""}}));

    try { return cache_.at(flags).c_str(); }
    catch (...) {
        const auto it = cache_.find(flags);
        if(it != cache_.end ()) return it->second.c_str ();
        auto& out = cache_[flags];
        for(const auto& fl: flagsToStr_) {
            if(flags & fl.first)
                out += out.size () ?
                    std::string(",") + fl.second :
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
#ifndef _WIN32
        SIG_AND_NAME(SIGBUS),
#endif
        SIG_AND_NAME(SIGILL),
        SIG_AND_NAME(SIGFPE)
    };
#undef SIG_AND_NAME

static std::atomic_flag backtraceOnAbortsFlag;

#ifndef _WIN32
void SigHandler_(int signum, siginfo_t* si, void* unused )
#else
void SigHandler_(int signum)
#endif
{
    if(backtraceOnAbortsFlag.test_and_set ()) return; // only first failed thread, or the below shall hang.
    GFX_EMU_ERROR_MESSAGE(
        "--------------------------------------------------------------------------\n"
        "Received signal %u %s. Terminating.\n\n",
        signum,
        signals.at(signum)
    );
    GfxEmu::Utils::terminate(signum,true);
}

struct HandleAborts_ {
    HandleAborts_ ()
    {
        if(!GfxEmu::Cfg::CatchTerminatingSignals ())
            return;

#ifndef _WIN32
        struct sigaction sa;
        sa.sa_flags = SA_SIGINFO;
        sa.sa_sigaction = SigHandler_;
        sigemptyset( &sa.sa_mask );
#endif

        for (const auto& signal: signals) {
#ifdef _WIN32
            ::signal(signal.first, SigHandler_);
#else
            sigaction(signal.first, &sa, NULL);
#endif
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
#if defined(_WIN32)
    const ULONG framesToSkip = 0;
    const ULONG framesToCapture = 128;
    void* backTrace[framesToCapture] {};
    ULONG backTraceHash = 0;

    const USHORT nFrame = CaptureStackBackTrace(
        framesToSkip,
        framesToCapture,
        backTrace,
        &backTraceHash
    );

    for(auto iFrame = 0; iFrame < nFrame; iFrame++)
        PrintMessage("[%3d] = %s\n",
            iFrame,
            GfxEmu::DbgSymb::obj().addrToSymbDesc(backTrace[iFrame]).name.c_str ()
        );

    PrintMessage("backTraceHash = %08x\n", backTraceHash);
#else

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
                     (size_t)((char *)callstack[i] - (char *)info.dli_saddr)
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
#endif
#endif // GFX_EMU_DEBUG_SYMBOLS_ACCESS_ENABLED
}
};
};
