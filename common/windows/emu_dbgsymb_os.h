/*========================== begin_copyright_notice ============================

Copyright (C) 2021 Intel Corporation

SPDX-License-Identifier: MIT

============================= end_copyright_notice ===========================*/

#ifndef EMU_SYMBOLS_OS_WIN_H
#define EMU_SYMBOLS_OS_WIN_H

#ifdef GFX_EMU_DEBUG_SYMBOLS_ACCESS_ENABLED

#include <windows.h>
#pragma comment( lib, "dbghelp.lib" )
// Defined in CMake #define _NO_CVCONST_H
#include <dbghelp.h>

#include <string>
#include <vector>

#include "emu_dbgsymb.h"
#include "emu_dbgsymb_types.h"
#include "emu_kernel_support_types.h"

namespace GfxEmu {
namespace DbgSymb {

struct DbgSymbOsImpl {
friend struct DbgSymbIface;
private:
    HANDLE sessionHandle;
    bool gotSymbols = false;
protected:
    DbgSymbOsImpl ();
public:
    ~DbgSymbOsImpl ();
    bool setLoadedProgramModuleInfo(void *addr, const std::string& programName, GfxEmu::KernelSupport::ProgramModule& desc);
    std::string linkageNameToName (const char *linkageName);
    std::string nameToLinkageName (const char *name);
    void* nameToAddr (const char *name);
    SymbDesc addrToSymbDesc (void *addr);
    GfxEmu::DbgSymb::FunctionDesc getFunctionDesc (
        const char *kernelName,
        const void *addr,
        const GfxEmu::KernelSupport::ProgramModule& programModule
    );
};

}; // namespace DbgSymb
}; // namespace GfxEmu

#endif // GFX_EMU_DEBUG_SYMBOLS_ACCESS_ENABLED
#endif // EMU_SYMBOLS_OS_WIN_H
