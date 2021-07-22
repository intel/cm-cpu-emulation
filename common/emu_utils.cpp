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

#include "emu_utils.h"
#include "emu_log.h"

#include <iostream>
#include <string>
#include <codecvt>

    #include <dlfcn.h>
    #include <unistd.h>
    #include <errno.h>
    #include <string.h>

namespace GfxEmu {
namespace Utils {

using namespace GfxEmu::Log::Flags;

void* symbolNameToAddr (const char *moduleName, const std::string& linkageName, const std::string& name) {
    auto dlSession = dlopen(moduleName, RTLD_NOW | RTLD_GLOBAL);

    if(!dlSession) {
        GFX_EMU_WARNING_MESSAGE("dlopen returned error: %s\n", dlerror());
        return nullptr;
    }

    if (linkageName.size() == 0) {
        GFX_EMU_DEBUG_MESSAGE(fDbgSymb, "symbolNameToAddr: couldn't get linkage name for %s\n",
            name.c_str ());
        return nullptr;
    }

    GFX_EMU_DEBUG_MESSAGE(fDbgSymb | fExtraDetail,
        "symbolNameToAddr: lookup for symbol name %s, linkage name: %s\n",
            name.c_str(), linkageName.c_str());

    const auto addr = dlsym(dlSession, linkageName.c_str());
    if (!addr) {
        GFX_EMU_WARNING_MESSAGE(fDbgSymb | fCritical, "symbolNameToAddr: \n"
            "\tunable to lookup symbol %s (linkage name: %s) with dlsym().\n"
            "\tmay recompile kernel source with -rdynamic or -Wl,--export-dynamic for this to work.\n\n",
                name.c_str (),
                linkageName.c_str()
        );
    }
    else {
        GFX_EMU_DEBUG_MESSAGE(fDbgSymb | fExtraDetail, "symbolNameToAddr: kernel %s address is found at %p\n",
            name.c_str (),
            reinterpret_cast<uint64_t>(addr));
    }

    dlclose(dlSession);
    return addr;
}

// Create a string with last error message
std::string lastErrorStr()
{
    return strerror(errno);
}

bool stringToBool(const std::string& v) {
    static std::regex rx {"^\\s*[1-9]|true|yes", std::regex::icase};
    return std::regex_search(v, rx);
}

bool isNotAKernelProgram(const char *moduleFileName) {
    static std::regex systemLibsFilter {
        "^(linux-vdso|/(usr|lib))|libcm|cmrt_emu|libshim"
        , std::regex::ECMAScript | std::regex::icase};
    if(std::regex_search(moduleFileName, systemLibsFilter)) {
        //GfxEmu::DebugMessage<fDbgSymb | fExtraDetail>(
        //    "skipping non-kernel program %s\n", moduleName);
        return true;
    }
    return false;
}

bool deleteFile (const char *file) {
    bool status = false;
    status = unlink(file) == 0;

    if(!status)
        GFX_EMU_ERROR_MESSAGE("failed to delete file %s: %s\n",
            file, lastErrorStr ().c_str ());

    return status;
}

uint64_t getCurrentProcessId () {
    return ::getpid();
}

};
};
