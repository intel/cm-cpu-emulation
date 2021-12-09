/*========================== begin_copyright_notice ============================

Copyright (C) 2020 Intel Corporation

SPDX-License-Identifier: MIT

============================= end_copyright_notice ===========================*/

#ifndef CM_UTILS_H
#define CM_UTILS_H

#include <algorithm>
#include <cctype>
#include <cstdlib>
#include <string>
#include <type_traits>
#include <regex>
#include <atomic>

#ifdef _WIN32
#define NOMINMAX
#include <windows.h>
#endif

#include "emu_api_export.h"

namespace GfxEmu {
namespace Utils {

[[noreturn]] GFX_EMU_API void terminate(int code, bool printBacktrace = false);

// --- debug and linkage data ---

GFX_EMU_API void* symbolNameToAddr (const char *moduleName, const std::string& linkageName, const std::string& name = "");

// --- atomics ---

template<class T>
inline void atomicUpdateMax(std::atomic<T>& max, const T& v) noexcept {
    auto prev = max.load();
    while(prev < v && !max.compare_exchange_weak(prev, v))
        ;
}

// --- strings ---
template<class T>
inline std::vector<T> extractFromStr(const std::string& str, std::regex rx) {
    std::stringstream ss;
    std::vector<T> out;
    T tmp;
    auto it = std::sregex_token_iterator(str.begin (), str.end (), rx);
    while(it != std::sregex_token_iterator()) {
        std::stringstream{*it} >> tmp;
        if(tmp) out.push_back(tmp);
        ++it;
    }
    return out;
}

bool stringToBool(const std::string& v);

inline std::string toLower(std::string str) {
    std::transform(str.begin(), str.end(), str.begin(),
        [](unsigned char c){ return std::tolower(c); });
    return str;
}

inline std::string toUpper(std::string str) {
    std::transform(str.begin(), str.end(), str.begin(),
        [](unsigned char c){ return std::toupper(c); });
    return str;
}

inline std::string removeSpace(std::string str) {
    for(const auto c: {' ', '\t'})
        str.erase(std::remove(str.begin(), str.end(), c), str.end());
    return str;
}

// -- system-specific: error, process name, etc. ---

inline void debugBreak () {
#ifdef _WIN32
    __debugbreak();
#else
    __builtin_trap();
#endif
}

std::string lastErrorStr();

#ifdef _WIN32
std::string wstringToString(std::wstring ws);
std::string getCurrentProcessBaseName ();
#endif

inline std::string getMainProgramName () {
#ifdef _WIN32
    return getCurrentProcessBaseName ();
#else
    return "/proc/self/exe";
#endif
}

bool deleteFile (const char*);

uint64_t getCurrentProcessId ();

inline std::string getEnvStrNoSynch(const char* name) {
    if (name)
        if (const char* val = std::getenv(name))
            return val;
    return "";
}

// --- misc ---

bool isNotAKernelProgram(const char *moduleName);

using VoidFuncPtr = void(*)();

// static_assert(std::is_convertible<VoidFuncPtr,void*>::value);
// This will FAIL, however POSIX's dlsym specification requires void* and a function pointer
// to be interconvertible [https://pubs.opengroup.org/onlinepubs/9699919799/functions/dlsym.html]
// At least we can ensure alignment and size match
static_assert(sizeof(VoidFuncPtr) == sizeof(void*));
static_assert(std::alignment_of<VoidFuncPtr>::value == std::alignment_of<void*>::value);

}; // namespace Utils
}; // namespace GfxEmu

#endif
