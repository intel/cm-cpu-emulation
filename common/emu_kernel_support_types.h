/*========================== begin_copyright_notice ============================

Copyright (C) 2017 Intel Corporation

SPDX-License-Identifier: MIT

============================= end_copyright_notice ===========================*/

#pragma once

#include <string>
#include <type_traits>
#ifdef _WIN32
    #define NOMINMAX
    #include <windows.h>
#endif

#include "emu_dbgsymb_types.h"
#include "emu_api_export.h"

namespace GfxEmu {
namespace KernelSupport {

using DbgSymb::FunctionDesc;

using VoidFuncPtr = void(*)();

static void
    *kGlobalKernelSearch      = nullptr,
    *kMainProgramKernelSearch = reinterpret_cast<void*>(1),
    *kModuleAddrInvalid       = reinterpret_cast<void*>(~0ull)
;

struct ProgramModule {

public:
    using ModuleHandle =
#ifdef _WIN32
        HMODULE
#else
        void*
#endif
    ;

private:
    std::string moduleFileName {""};
    void *moduleAddr {kGlobalKernelSearch};
    void *moduleEndAddr {nullptr};
    bool isOwning_ = false;
    bool isCreator = false;
    ModuleHandle moduleHandle;
    void initModuleHandle ();
    //void *srcBlobAddr {nullptr};

public:
    void setModuleAddr(void* addr) { moduleAddr = addr;};
    void* getModuleAddr() const { return moduleAddr;};
    GFX_EMU_API ModuleHandle getModuleHandle() const noexcept { return moduleHandle; }
    void setThisLoadedProgramInfo ();
    bool setModuleFileName (std::string fileName, bool setLoadedModuleInfo = true);
    GFX_EMU_API const std::string& getModuleFileName () const;
    GFX_EMU_API std::string getModuleBaseName () const;
    GFX_EMU_API std::string getModuleStemName () const;
    GFX_EMU_API std::string toStr () const;
    GFX_EMU_API bool isOwning() const;
    GFX_EMU_API bool isGlobalKernelSearch () const;
    GFX_EMU_API bool isMainProgramKernelSearch () const;
    GFX_EMU_API bool isSharedLib () const;
    GFX_EMU_API operator bool () const;
    //GFX_EMU_API bool contains(void *addr) const { return addr >= moduleAddr && addr < moduleEndAddr; };
    bool replace(ProgramModule&&);
    bool load(std::string fileName = "");

    GFX_EMU_API ~ProgramModule();
    GFX_EMU_API ProgramModule(ProgramModule&& other);
    GFX_EMU_API ProgramModule& operator =(ProgramModule&& other);

    ProgramModule& operator =(ProgramModule&) = delete;
    ProgramModule(ProgramModule&) = delete;

    ProgramModule();
    ProgramModule(void*,size_t);
    ProgramModule(std::string);

private:
    void terminateOwning ();
    void moveFrom(ProgramModule&&);
};

static_assert(!std::is_copy_constructible_v<ProgramModule>);
static_assert(!std::is_copy_assignable_v<ProgramModule>);
static_assert(std::is_move_constructible_v<ProgramModule>);
static_assert(std::is_move_assignable_v<ProgramModule>);

};
};
