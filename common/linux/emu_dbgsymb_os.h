/*========================== begin_copyright_notice ============================

Copyright (C) 2021 Intel Corporation

SPDX-License-Identifier: MIT

============================= end_copyright_notice ===========================*/

#ifndef EMU_SYMBOLS_OS_LIN_H
#define EMU_SYMBOLS_OS_LIN_H

#include <regex>
#include <vector>

#include "emu_dbgsymb_types.h"

#ifdef GFX_EMU_DEBUG_SYMBOLS_ACCESS_ENABLED
#include <dwarf.h>

#include "emu_kernel_support_types.h"

namespace GfxEmu {
namespace DbgSymb {

struct DbgSymbOsImpl {
friend struct DbgSymbIface;
private:
    struct Dwarf* dwarfSession;
    int processHandle;
    bool gotSymbols = false;
    static std::vector<std::string> fundTplTypeNameVariants;
    std::vector<std::regex>  fundTplTypeNameVariantsRx;
    std::vector<std::string> fundTplTypeNameRepls[2]; // Two variants of replacement for Gcc and Clang dwarf producers.
    void fundamentalTypeNameVariantsInit ();
protected:
    DbgSymbOsImpl ();
public:
    ~DbgSymbOsImpl ();
    std::string translateTemplatedKernelName (const char *kernelName, bool isGcc);
    void* nameToAddr (const std::string& linkageName, const std::string& name);
    SymbDesc addrToSymbDesc (void *addr);
    GfxEmu::DbgSymb::FunctionDesc getFunctionDesc (
        const char *kernelName,
        const void *addr,
        const GfxEmu::KernelSupport::ProgramModule& programModule
    );
};

}; // namespace DbgSymb
}; // namespace GfxEmu
#endif
#endif
