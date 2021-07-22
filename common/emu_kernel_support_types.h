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

#pragma once

#include <string>
#include <type_traits>

#include "emu_dbgsymb_types.h"
#include "emu_api_export.h"

namespace GfxEmu {
namespace KernelSupport {

using VoidFuncPtr = void(*)();

static void
    *kGlobalKernelSearch      = nullptr,
    *kMainProgramKernelSearch = reinterpret_cast<void*>(1),
    *kModuleAddrInvalid       = reinterpret_cast<void*>(~0ull)
;

struct ProgramModule {

public:
    using ModuleHandle =
        void*
    ;

private:
    std::string moduleFileName {""};
    void *moduleAddr {kGlobalKernelSearch};
    void *moduleEndAddr {nullptr};
    bool isOwning_ = false;
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
    bool unloadIfOwning ();
    void moveFrom(ProgramModule&&);
};

static_assert(!std::is_copy_constructible_v<ProgramModule>);
static_assert(!std::is_copy_assignable_v<ProgramModule>);
static_assert(std::is_move_constructible_v<ProgramModule>);
static_assert(std::is_move_assignable_v<ProgramModule>);

};
};

