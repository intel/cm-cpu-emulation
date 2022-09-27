/*========================== begin_copyright_notice ============================

Copyright (C) 2017 Intel Corporation

SPDX-License-Identifier: MIT

============================= end_copyright_notice ===========================*/

#ifndef EMU_SYMBOLS_H
#define EMU_SYMBOLS_H

#include "emu_dbgsymb_types.h"
#include "emu_kernel_support_types.h"

#ifdef GFX_EMU_DEBUG_SYMBOLS_ACCESS_ENABLED
namespace GfxEmu {
namespace DbgSymb {
using VoidFuncPtr = void(*)();

struct DbgSymbIface {
friend DbgSymbIface& obj();
public:
    GfxEmu::DbgSymb::FunctionDesc getFunctionDesc(const char*, const void *, const GfxEmu::KernelSupport::ProgramModule&);
    //GfxEmu::DbgSymb::SymbDesc addrToSymbDesc(VoidFuncPtr);
    GfxEmu::DbgSymb::SymbDesc addrToSymbDesc(void*);
#ifdef _WIN32
    bool setLoadedProgramModuleInfo(void *addr, const std::string& programName, GfxEmu::KernelSupport::ProgramModule& desc);
#endif
    ~DbgSymbIface ();
protected:
    DbgSymbIface ();
private:
    struct DbgSymbOsImpl* impl;
};

DbgSymbIface& obj();

}; // namespace DbgSymb
}; // namespace GfxEmu
#endif // GFX_EMU_DEBUG_SYMBOLS_ACCESS_ENABLED

#endif // EMU_SYMBOLS_H
