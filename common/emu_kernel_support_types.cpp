/*========================== begin_copyright_notice ============================

Copyright (C) 2021 Intel Corporation

SPDX-License-Identifier: MIT

============================= end_copyright_notice ===========================*/

#include <fstream>
#include <sstream>

#if defined(_WIN32) || __clang__ || __GNUC__ > 7
    #include <filesystem>
    namespace fs = std::filesystem;
#else
    #include <experimental/filesystem>
    namespace fs = std::experimental::filesystem;
#endif

#ifdef _WIN32
    #include <windows.h>
#else
    #include <link.h>
    #include <dlfcn.h>
#endif

#include "emu_kernel_support.h"
#include "emu_log.h"
#ifdef GFX_EMU_DEBUG_SYMBOLS_ACCESS_ENABLED
#include "emu_dbgsymb.h"
#endif
#include "emu_utils.h"
#include "emu_cfg.h"

namespace GfxEmu {
namespace KernelSupport {

using namespace GfxEmu::Log::Flags;

//------------------------------------------------------------------------
namespace details {

#ifndef _WIN32
//-------------------------------------------------------------------
struct setLoadedProgramModuleInfo_callback_data{
    void *queryAddr {nullptr};
    std::string queryModuleName;
    GfxEmu::KernelSupport::ProgramModule* descPtr {nullptr};
    bool found {false};
};

int setLoadedProgramModuleInfo_callback(struct dl_phdr_info *info, size_t size, void *data_)
{
    auto& data = *reinterpret_cast<setLoadedProgramModuleInfo_callback_data*> (data_);
    if (data.found) return 0;

    if(GfxEmu::Utils::isNotAKernelProgram(info->dlpi_name)) {
        return 0;
    }

    std::string curModuleName = info->dlpi_name;
    if (curModuleName == "") curModuleName = "/proc/self/exe";

    GFX_EMU_DEBUG_MESSAGE(fKernelSupport | fExtraDetail,
        "inspecting segments in %s (%d segments)\n", curModuleName.c_str(),
        info->dlpi_phnum);

    bool found = false;

    GFX_EMU_ASSERT(data.queryAddr || data.queryModuleName.size ());

    if (data.queryModuleName.size () &&
        (curModuleName.find(data.queryModuleName) != std::string::npos ||
         data.queryModuleName.find(curModuleName) != std::string::npos)
        )
    {
        data.found = true;
    }
    else if (data.queryAddr) {
        for (int j = 0; j < info->dlpi_phnum; j++) {
            const auto lo = reinterpret_cast<void*>(info->dlpi_addr + info->dlpi_phdr[j].p_vaddr);
            const auto hi = reinterpret_cast<void*>((uint64_t)lo + info->dlpi_phdr[j].p_memsz);
            //data.descPtr->setModuleEndAddr(hi);
            if(data.queryAddr >= lo && data.queryAddr < hi) {
                GFX_EMU_DEBUG_MESSAGE(fKernelSupport | fExtraDetail,
                    "success: address %p found in header %2d: address=%10p, size %u. cur lib name: %s\n",
                    data.queryAddr, j, lo, info->dlpi_phdr[j].p_memsz, curModuleName.c_str());

                data.found = true;
                // we do not break; need to get moduleEndAddr.
            } else if (!data.found) {
                GFX_EMU_DEBUG_MESSAGE(fKernelSupport | fExtraDetail,
                    "checking if %p matches elf header %2d: address=%10p of size %u\n",
                    data.queryAddr, j, lo, info->dlpi_phdr[j].p_memsz);
            }
        }
    }

    if(data.found) {
        data.descPtr->setModuleFileName(curModuleName, false);
        data.descPtr->setModuleAddr(reinterpret_cast<void *>(info->dlpi_addr));
    } else {
        //data.descPtr->setModuleEndAddr(0);
        if (data.queryAddr) {
            GFX_EMU_DEBUG_MESSAGE(fKernelSupport | fExtraDetail,
                "address %p is not found in %s.\n", data.queryAddr, curModuleName.c_str ()
            );
        } else {
            GFX_EMU_DEBUG_MESSAGE(fKernelSupport | fExtraDetail,
                "module name %s is not found in %s.\n", data.queryModuleName.c_str (), curModuleName.c_str ()
            );
        }
    }

    return 0;
}
#endif

bool setLoadedProgramModuleInfo (void *addr, const std::string& moduleName, ProgramModule& desc) {
#ifdef _WIN32
    #ifdef GFX_EMU_DEBUG_SYMBOLS_ACCESS_ENABLED
        return GfxEmu::DbgSymb::obj().setLoadedProgramModuleInfo (addr, moduleName, desc);
    #else
        GFX_EMU_WARNING_MESSAGE("Getting module info is only supported if debug symbols access enabled.\n");
        return false;
    #endif
#else
    setLoadedProgramModuleInfo_callback_data cbData;
    cbData.queryAddr = addr;
    cbData.queryModuleName = moduleName;
    cbData.descPtr = &desc;
    dl_iterate_phdr(setLoadedProgramModuleInfo_callback, &cbData);
#endif

    if (desc.getModuleAddr () != nullptr)
        return true;

    return false;
}

}; // namespace details

//------------------------------------------------------------------------

std::string ProgramModule::toStr () const {
    std::stringstream ss;

    const bool isGks = isGlobalKernelSearch ();
    const bool isSpecial = isGks || isMainProgramKernelSearch ();

    ss << "[id:0x" << std::hex << this << ", " <<
       (isGks ? "GLOBAL KERNEL SEARCH" :
        isSpecial ? "MAIN PROGRAM KERNEL SEARCH, " : "");

    if(!isGks) {
        ss << "stem: " << getModuleStemName () <<
              ", file: " << getModuleFileName () <<
              ", base addr: " << moduleAddr;
        if(!isSpecial) {
            if(isOwning ()) ss << ", owning(handle=0x" << moduleHandle << ")";
            else ss << ", non-owning";
        }
    }

    ss << "]";

    return ss.str ();
}

void ProgramModule::initModuleHandle() {
#ifdef _WIN32
    if(!GetModuleHandleExA(
            GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT,
                NULL,&moduleHandle)) {
        GFX_EMU_FAIL_WITH_MESSAGE("can't init handle for main program: %s\n", GfxEmu::Utils::lastErrorStr ().c_str ());
    }
#else

    moduleHandle = dlopen(NULL, RTLD_NOW | RTLD_NOLOAD);
    if(!moduleHandle) {
        GFX_EMU_FAIL_WITH_MESSAGE("can't init handle for main program: %s\n", dlerror());
    }
#endif
    ;
}

ProgramModule::ProgramModule ()
{
    GFX_EMU_DEBUG_MESSAGE(fKernelSupport | fExtraDetail,
        "(+) construct %s\n", toStr ().c_str ());
    initModuleHandle();
}

ProgramModule::~ProgramModule () {
    GFX_EMU_DEBUG_MESSAGE(fKernelSupport | fExtraDetail,
        "(x) destruct %s\n", toStr ().c_str ());

    terminateOwning ();
}

ProgramModule::operator bool () const {
    return
        isOwning () || (
            moduleAddr != kModuleAddrInvalid
        );
};

void ProgramModule::moveFrom(ProgramModule&& other) {
    terminateOwning ();

    moduleHandle = std::move(other.moduleHandle);
    isOwning_ = other.isOwning_;
    other.isOwning_ = false; // make the other non-owning to avoid unloading on following destruction.

    moduleFileName = std::move(other.moduleFileName);
    moduleAddr = std::move(other.moduleAddr);
    other.moduleAddr = kModuleAddrInvalid;
    moduleEndAddr = std::move(other.moduleEndAddr);
}

ProgramModule::ProgramModule(ProgramModule&& other) {
    GFX_EMU_DEBUG_MESSAGE(fKernelSupport | fExtraDetail,
        "(<+) move construct %s <+ %s\n", toStr ().c_str (), other.toStr().c_str());
    moveFrom(std::move(other));
};

ProgramModule& ProgramModule::operator =(ProgramModule&& other) {
    GFX_EMU_DEBUG_MESSAGE(fKernelSupport | fExtraDetail,
        "(<=) move assign %s <= %s\n", toStr ().c_str (), other.toStr().c_str());
    moveFrom(std::move(other));
    return *this;
};

bool ProgramModule::setModuleFileName(std::string fileName, bool setLoadedModuleInfo) {
    fileName = fs::absolute(fileName).string ();
    if(!fileName.size())
        GFX_EMU_FAIL_WITH_MESSAGE("Setting empty program module name is not allowed.\n");

    if(isOwning ()) {
        if(fileName != getModuleFileName () &&
           fileName.find(getModuleBaseName ()) == std::string::npos /*clarification from relative to absolute path is allowed*/)
            GFX_EMU_FAIL_WITH_MESSAGE(fKernelSupport,
                "trying to set different file name (%s) for an owning program module: %s\n",
                    fileName.c_str(), toStr ().c_str());
        return true;
    }

    moduleFileName.assign(fileName);

    if (setLoadedModuleInfo)
        setThisLoadedProgramInfo ();

    return true;
}

const std::string& ProgramModule::getModuleFileName() const {
    return moduleFileName;
}

std::string ProgramModule::getModuleBaseName() const {
    try{ return fs::path(moduleFileName).filename ().string(); }
    catch (...) { GFX_EMU_WARNING_MESSAGE(fKernelSupport, "can't extract file basename from %s\n", moduleFileName.c_str());
        return moduleFileName; };
};

std::string ProgramModule::getModuleStemName() const {
    try{ return fs::path(moduleFileName).filename ().stem().string(); }
    catch (...) { GFX_EMU_WARNING_MESSAGE(fKernelSupport, "can't extract filename stem from %s\n", moduleFileName.c_str());
        return moduleFileName; };
};

bool ProgramModule::isOwning() const {
    return isOwning_;
};

void ProgramModule::terminateOwning () {

    if(!isOwning()) return;

    GFX_EMU_DEBUG_MESSAGE(fKernelSupport | fInfo,
        "unloading shared library for program module %s\n", toStr ().c_str ());

    details::invalidateProgramModuleAssociatedData(*this);

    if(
#ifdef _WIN32
        ::FreeLibrary(moduleHandle) == 0
#else
        dlclose(moduleHandle) != 0
#endif

    ) {

        const char* err =
#ifdef _WIN32
                GfxEmu::Utils::lastErrorStr ().c_str ()
#else
                dlerror()
#endif
        ;

        GFX_EMU_FAIL_WITH_MESSAGE(
            fKernelSupport,
            "failed to unload shared library for program module %s: %s\n",
                toStr ().c_str (), err);
    }

    isOwning_ = false;
    initModuleHandle();
    moduleAddr = kGlobalKernelSearch;
    moduleEndAddr = nullptr;

    if(isCreator && !GfxEmu::Cfg::RetainTmpFiles ().getInt ()) {
        if(std::remove(getModuleFileName().c_str()) != 0) {
            GFX_EMU_FAIL_WITH_MESSAGE(fKernelSupport, "failed to delete program module file %s\n",
                getModuleFileName().c_str());
        }
    }

    isCreator = false;

    return;
}

bool ProgramModule::isGlobalKernelSearch () const {
    return moduleFileName == "" && moduleAddr == GfxEmu::KernelSupport::kGlobalKernelSearch;
}

bool ProgramModule::isMainProgramKernelSearch () const {
    return
        moduleFileName.find(GfxEmu::Utils::getMainProgramName ()) != std::string::npos
    ;
}

bool ProgramModule::isSharedLib () const {
    return !isGlobalKernelSearch() && !isMainProgramKernelSearch();
}

bool ProgramModule::load(std::string fileName) {

    if (fileName != "") {
        fileName = fs::absolute(fileName).string ();

        if (getModuleFileName () != "" &&
            getModuleFileName () != fileName)
        {
            GFX_EMU_WARNING_MESSAGE(fKernelSupport, "trying to load program %s via "
                "program module already associated with a program: %s",
                    fileName.c_str (), toStr ().c_str ());

            return false;
        }
        else
            setModuleFileName(fileName, false);
    }

    GFX_EMU_DEBUG_MESSAGE(fKernelSupport | fInfo,
            "loading shared library %s\n", fileName.c_str ());

    if(isGlobalKernelSearch () || isMainProgramKernelSearch ()) {
        GFX_EMU_WARNING_MESSAGE(fKernelSupport, "trying to load non-shared-libary program module: %s\n",
            toStr ().c_str ());
        return true;
    }

    if(isOwning()) {
        GFX_EMU_WARNING_MESSAGE(fKernelSupport, "trying to load program %s via"
            " already owning program module: %s\n",
            getModuleFileName ().c_str(),
            toStr ().c_str ());
        return true;
    }

#ifdef _WIN32
    moduleHandle = LoadLibraryA(moduleFileName.c_str ());
    if (!moduleHandle) {
        GFX_EMU_FAIL_WITH_MESSAGE(fKernelSupport, "can't init handle for program %s: %s.\n",
            moduleFileName.c_str (),
            GfxEmu::Utils::lastErrorStr ().c_str ());
    }
#else
    moduleHandle = dlopen(moduleFileName.c_str (), RTLD_NOW | RTLD_GLOBAL);
    if (!moduleHandle) {
        GFX_EMU_FAIL_WITH_MESSAGE(fKernelSupport, "can't init handle for program %s: %s.\n",
            moduleFileName.c_str (),
            dlerror());
    }
#endif

    setThisLoadedProgramInfo ();

    isOwning_ = true;

    return true;
}

void ProgramModule::setThisLoadedProgramInfo () {
    if(!details::setLoadedProgramModuleInfo(nullptr, getModuleFileName(), *this)) {
        GFX_EMU_WARNING_MESSAGE(fKernelSupport, "can't fill desc for %s program.\n", moduleFileName.c_str());
    }
}

ProgramModule::ProgramModule(void *addr, size_t size) {
    initModuleHandle();
    if (size) {
#define GFX_EMU_MIN_KERNEL_LIB_BLOB_SIZE 50
        if (size < GFX_EMU_MIN_KERNEL_LIB_BLOB_SIZE) {
            GFX_EMU_MESSAGE(fKernelSupport | fInfo,
                "pointed to in-memory program blob that is too small (< %u). Won't create library file from"
                    " buffer %p of size %u.\n", GFX_EMU_MIN_KERNEL_LIB_BLOB_SIZE, addr, size);

            return;
        }

        std::stringstream fileNameSs;
        fileNameSs <<
            "_pmodule_pid" <<
            std::dec << GfxEmu::Utils::getCurrentProcessId () << "_blob" <<
            std::hex << reinterpret_cast<uint64_t>(addr) << ".emu";

        const auto fileName = fs::absolute(fileNameSs.str ()).string ();

        GFX_EMU_MESSAGE(fKernelSupport | fInfo,
            "pointed to in-memory program blob, creating auxiliary library file %s from buffer %p of size %u.\n",
            fileName.c_str(), addr, size);

        std::ofstream file(fileName, std::ios::binary | std::ios::trunc);

        if (!file.is_open ()) {
            GFX_EMU_FAIL_WITH_MESSAGE(fKernelSupport, "can't open file at %s.\n", fileName.c_str());
        }

        for (size_t i = 0; i < size; i++)
            file << *(reinterpret_cast<uint8_t*>(addr) + i);

        file.close ();

        fs::permissions(fileName,
                        fs::perms::owner_all | fs::perms::group_all
    #if defined(_WIN32) || __clang__ || __GNUC__ > 7
                        ,
                        fs::perm_options::add
    #endif
        );

        if(!load (fileName)) {
            GFX_EMU_FAIL_WITH_MESSAGE(fKernelSupport, "can't load %s program.\n", getModuleFileName ().c_str());
        }

        isCreator = true;

        return;
    }

    if (addr == GfxEmu::KernelSupport::kGlobalKernelSearch) {
        GFX_EMU_MESSAGE(fKernelSupport | fInfo, "setting up global kernel search meta-program module.\n");
        return;
    }

    if (addr == GfxEmu::KernelSupport::kMainProgramKernelSearch) {
        GFX_EMU_MESSAGE(fKernelSupport | fInfo, "setting main program module.\n");
        if(!setModuleFileName(GfxEmu::Utils::getMainProgramName ()))
            GFX_EMU_WARNING_MESSAGE(fKernelSupport, "failed to create main program module.");
    }

    if(!details::setLoadedProgramModuleInfo (addr, "", *this)) {
        GFX_EMU_WARNING_MESSAGE(fKernelSupport,
            "can't find loaded program info for addr: %p.\n", addr);
    }
}

ProgramModule::ProgramModule(std::string fileName) {
    initModuleHandle();
    fileName = fs::absolute(fileName).string ();
    if(!details::setLoadedProgramModuleInfo (0, fileName, *this)) {
        if (fileName == "") {
            GFX_EMU_WARNING_MESSAGE(fKernelSupport,
                "can't construct program module from empty program name.\n");
            return;
        }

        if(!load (fileName)) {
            GFX_EMU_WARNING_MESSAGE(fKernelSupport, "program load failed: %s.\n", fileName.c_str ());
        }
    }
}

};
};
