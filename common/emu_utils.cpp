/*========================== begin_copyright_notice ============================

Copyright (C) 2017 Intel Corporation

SPDX-License-Identifier: MIT

============================= end_copyright_notice ===========================*/

#include <iostream>
#include <string>
#include <codecvt>

#ifdef _WIN32
    #include <windows.h>
    #include <psapi.h>
    #include <libloaderapi.h>
#else
    #include <dlfcn.h>
    #include <unistd.h>
    #include <errno.h>
    #include <string.h>
#endif

#include "emu_utils.h"
#include "emu_log.h"
#include "emu_cfg.h"

namespace GfxEmu {
namespace Utils {

using namespace GfxEmu::Log::Flags;

void terminate(int code, bool printBacktrace) {
    if(printBacktrace || GfxEmu::Cfg::BacktraceOnTermination ()) {
        GfxEmu::Log::printBacktrace ();
    }
    GfxEmu::Log::flush ();
    _Exit(code);
}

void* symbolNameToAddr (const char *moduleName, const std::string& linkageName, const std::string& name) {
#ifdef _WIN32
    HMODULE moduleHandle;
    if(!GetModuleHandleExA(
            GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT,
                moduleName,&moduleHandle)) {
        GFX_EMU_WARNING_MESSAGE("GetModuleHandleExA failed: %s\n", lastErrorStr ().c_str ());
        return nullptr;
    }

    const auto addr = GetProcAddress(moduleHandle, linkageName.c_str());

    if(!addr)
        GFX_EMU_WARNING_MESSAGE("GetProcAddr failed: %s\n", lastErrorStr ().c_str ());

#else
    auto dlSession = dlopen(!strcmp(moduleName,"/proc/self/exe") ? 0 : moduleName, RTLD_NOW | RTLD_GLOBAL);

    if(!dlSession) {
        GFX_EMU_WARNING_MESSAGE("dlopen returned error: %s\n", dlerror());
        return nullptr;
    }

    if (linkageName.size() == 0) {
        GFX_EMU_DEBUG_MESSAGE(fDbgSymb, "symbolNameToAddr: couldn't get linkage name for %s\n",
            name.c_str ());
        return nullptr;
    }

    const auto addr = dlsym(dlSession, linkageName.c_str());
    if (!addr) {
        GFX_EMU_WARNING_MESSAGE(fDbgSymb | fCritical, "symbolNameToAddr: \n"
            "\tunable to lookup symbol %s (linkage name: %s) with dlsym().\n"
            "\tmay recompile with -rdynamic or -Wl,--export-dynamic for this to work.\n\n",
                name.c_str (),
                linkageName.c_str()
        );
    }

    dlclose(dlSession);
#endif

    GFX_EMU_DEBUG_MESSAGE(fDbgSymb | fDetail,
        "lookup in module %s for function %s -> address: %p\n",
            moduleName, linkageName.c_str (), addr);

    return addr;
}

// Create a string with last error message
std::string lastErrorStr()
{
#ifdef _WIN32
    DWORD error = GetLastError();
    if (error) {
        LPVOID lpMsgBuf;
        DWORD bufLen = FormatMessage(
            FORMAT_MESSAGE_ALLOCATE_BUFFER |
            FORMAT_MESSAGE_FROM_SYSTEM |
            FORMAT_MESSAGE_IGNORE_INSERTS,
            NULL,
            error,
            MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
            reinterpret_cast<LPTSTR>(&lpMsgBuf),
            0, NULL
        );

        if (bufLen) {
            auto lpMsgStr = reinterpret_cast<LPCSTR>(lpMsgBuf);
            std::string result(lpMsgStr, lpMsgStr+bufLen);
            LocalFree(lpMsgBuf);
            return result;
        }
    }

    return {};
#else
    return strerror(errno);
#endif
}

#ifdef _WIN32
std::string wstringToString(std::wstring ws) {
    return std::wstring_convert<std::codecvt_utf8<wchar_t>>{}.to_bytes( ws );
}

std::string getCurrentProcessBaseName () {
    char buf[1024];
    ::GetModuleBaseNameA(
        GetCurrentProcess (),
        NULL,
        buf, 1024);
    return buf;
}
#endif

bool stringToBool(const std::string& v) {
    static std::regex rx {"^\\s*[1-9]|true|yes", std::regex::icase};
    return std::regex_search(v, rx);
}

bool isNotAKernelProgram(const char *moduleFileName) {
    static std::regex systemLibsFilter {
#ifdef _WIN32
        "(shim.dll|shim_l0.dll|libcm.dll|igfx(11)?cmrt|system32|c:\\windows)"
#else
        "^(linux-vdso|/(usr|lib))|libcm|cmrt_emu|libshim"
#endif
        , std::regex::ECMAScript | std::regex::icase};
    if(std::regex_search(moduleFileName, systemLibsFilter)) {
        //GFX_EMU_DEBUG_MESSAGE(fDbgSymb | fExtraDetail,
        //    "skipping non-kernel program %s\n", moduleName);
        return true;
    }
    return false;
}

bool deleteFile (const char *file) {
    bool status = false;
#ifdef _WIN32
    status = ::DeleteFileA(file);
#else
    status = unlink(file) == 0;
#endif

    if(!status)
        GFX_EMU_ERROR_MESSAGE("failed to delete file %s: %s\n",
            file, lastErrorStr ().c_str ());

    return status;
}

uint64_t getCurrentProcessId () {
#ifdef _WIN32
    return ::GetCurrentProcessId();
#else
    return ::getpid();
#endif
}

};
};
