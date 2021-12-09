/*========================== begin_copyright_notice ============================

Copyright (C) 2021 Intel Corporation

SPDX-License-Identifier: MIT

============================= end_copyright_notice ===========================*/

// See README_KERNEL_SUPPORT.md for API doc.

#ifndef EMU_KERNEL_SUPPORT_H
#define EMU_KERNEL_SUPPORT_H

#include "emu_kernel_support_types.h"
#include "emu_dbgsymb_types.h"
#include "emu_api_export.h"

#define GFX_EMU_KERNEL_SUPPORT_ENABLED
namespace GfxEmu {
namespace KernelSupport {

GFX_EMU_API const GfxEmu::DbgSymb::FunctionDesc& getKernelDesc (
    std::string kernelName,
    VoidFuncPtr kernelAddr = nullptr);

GFX_EMU_API const GfxEmu::DbgSymb::FunctionDesc& getKernelDesc (
    std::string kernelName,
    const ProgramModule& programModule,
    VoidFuncPtr kernelAddr = nullptr);

// if size == 0 programAddr must point to an address space of already loaded program module.
// if size != 0 shall try to create shared lib from an in-memory blob pointed by programAddr and load it.
GFX_EMU_API ProgramModule setupProgram (void *programAddr, size_t size = 0);

// Looks for an already loaded shared library or loads it for you.
GFX_EMU_API ProgramModule setupProgram (const std::string& programName);

//-----------------------------------------------------------------
namespace details {
GFX_EMU_API void invalidateProgramModuleAssociatedData(const ProgramModule& desc);
}

inline void compile_test__ () {
    getKernelDesc("myKernel");
    getKernelDesc("", nullptr);
    //getKernelDesc("myKernel",GfxEmu::KernelSupport::ProgramModule{} /*not allowed, construction context is restricted*/);
    getKernelDesc("myKernel", setupProgram(GfxEmu::KernelSupport::kGlobalKernelSearch));
    setupProgram("");
    setupProgram(nullptr);
    setupProgram(GfxEmu::KernelSupport::kMainProgramKernelSearch);
    setupProgram(GfxEmu::KernelSupport::kGlobalKernelSearch);
    setupProgram(nullptr, 0);
}

// GFX_EMU_API void init ();

}; // namespace KernelSupport
}; // namespace GfxEmu

#endif // EMU_KERNEL_SUPPORT_H
