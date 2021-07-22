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

#ifndef CM_UTILS_H
#define CM_UTILS_H

#include <algorithm>
#include <cctype>
#include <cstdlib>
#include <string>
#include <type_traits>
#include <regex>
#include <atomic>

namespace GfxEmu {
namespace Utils {

// --- debug and linkage data ---

void* symbolNameToAddr (const char *moduleName, const std::string& linkageName, const std::string& name = "");

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
    __builtin_trap();
}

std::string lastErrorStr();

inline std::string getMainProgramName () {
    return "/proc/self/exe";
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
