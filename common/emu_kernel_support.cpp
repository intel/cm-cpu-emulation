/*========================== begin_copyright_notice ============================

Copyright (C) 2017 Intel Corporation

SPDX-License-Identifier: MIT

============================= end_copyright_notice ===========================*/

//#define TESTING_SWITCH_GLOBAL_KSEARCH_TO_MAIN_KSEARCH_
#define LEGACY_ENABLE_USER_SUPPLIED_KERNEL_ADDRESS_
#define LEGACY_ENABLE_USER_SUPPLIED_KERNEL_ADDRESS_OVERRIDE_GLOBAL_SEARCH_

#include <unordered_map>
#include <cstring>
#include <string>

#include "emu_utils.h"
#include "emu_log.h"
#ifdef GFX_EMU_DEBUG_SYMBOLS_ACCESS_ENABLED
#include "emu_dbgsymb.h"
#endif
#include "emu_kernel_support.h"

#include <kernel_utils.h>

namespace GfxEmu {
namespace KernelSupport {

using namespace GfxEmu::Log::Flags;

namespace details {

static std::unordered_map<
    std::string,
    std::unordered_map<
        std::string,
        DbgSymb::FunctionDesc
    >>
kernelsByProgram;

static std::unordered_map<VoidFuncPtr,DbgSymb::FunctionDesc*>
addrToKernel;

bool fallback_getKernelDataFromShim(GfxEmu::DbgSymb::FunctionDesc& k, const ProgramModule& programModule);

const DbgSymb::FunctionDesc& getKernelDesc_ (
        std::string kernelName,
        const ProgramModule& programModule,
        VoidFuncPtr addr
    )
{
    GFX_EMU_DEBUG_MESSAGE(fKernelSupport | fDetail, "-------------------\n");
    GFX_EMU_DEBUG_MESSAGE(fKernelSupport | fDetail,
        "looking up info about kernel addr:%p name:%s.\n",
            addr, kernelName.c_str());
    GFX_EMU_DEBUG_MESSAGE(fKernelSupport | fDetail, "-------------------\n");

    GFX_EMU_DEBUG_MESSAGE(fKernelSupport | fDetail,
        "associated program desc dump: %s\n", programModule.toStr ().c_str ()
    );

    ProgramModule programModuleOverride;
#ifdef LEGACY_ENABLE_USER_SUPPLIED_KERNEL_ADDRESS_
    // If kernel address provided explicitly see if need to override the associated program.
    // Inform a user.
    if (addr) {
        const auto& k = addrToKernel.find(addr);
        if(k != addrToKernel.end ())
            return *(k->second);

        GFX_EMU_DEBUG_MESSAGE(fKernelSupport | fDetail,
            "explicit kernel address provided. "
            "Checking for overriding program descriptor for kernel %s, addr %p.\n",
            kernelName.c_str(), addr);

#ifndef LEGACY_ENABLE_USER_SUPPLIED_KERNEL_ADDRESS_OVERRIDE_GLOBAL_SEARCH_
        if (programModule.isGlobalKernelSearch ()) {
            GFX_EMU_DEBUG_MESSAGE_AT(fKernelSupport | fDetail, "supplied program descriptor is a global kernel search,"
                " will not override with a more restrictive one."
                " Define LEGACY_ENABLE_USER_SUPPLIED_KERNEL_ADDRESS_OVERRIDE_GLOBAL_SEARCH_ to always override.\n");
        }
        else
#endif
        {
            programModuleOverride = std::move(GfxEmu::KernelSupport::setupProgram((void*)addr));
            if(programModuleOverride.getModuleAddr () != programModule.getModuleAddr ()) {
                GFX_EMU_MESSAGE(fKernelSupport | fInfo, "explicit address for kernel %s provided: %p. "
                    "Overriding program descriptor. "
                    "Old module base: %p, new: %p\n",
                    kernelName.c_str(), addr, programModule.getModuleAddr (), programModuleOverride.getModuleAddr ());
                //programModule.replace(programModuleOverride);
            }
        }
    }
#endif

    #ifdef TESTING_SWITCH_GLOBAL_KSEARCH_TO_MAIN_KSEARCH_
        // For testing only: switch from global to main program kernel search.
        if(programModule.isGlobalKernelSearch () &&
           kernelName != "")
        {
            GFX_EMU_MESSAGE(
                "\n[ATTENTION] switching from global to main program kernel search mode enabled!"
                " Switching!\n\n");
            //programModule.replace(
                programModuleOverride = GfxEmu::KernelSupport::setupProgram(
                    GfxEmu::KernelSupport::kMainProgramKernelSearch
                )
            //);
        } else
            GFX_EMU_FAIL_WITH_MESSAGE("can't get kernel descriptor: neither program module address nor "
                "kernel address nor kernel name were passed.\n");
    #endif

    // If no kernel name provided, try lookup by address.
    if(kernelName == "")
    {
        if(addr == nullptr) {
            GFX_EMU_FAIL_WITH_MESSAGE("can't find any data for a kernel "
                "when no name and address provided\n");
        }
#ifdef GFX_EMU_DEBUG_SYMBOLS_ACCESS_ENABLED
        const auto symbDesc = DbgSymb::obj().addrToSymbDesc(
            reinterpret_cast<void*>(addr)
        );

        kernelName = symbDesc.name;
#else
        GFX_EMU_WARNING_MESSAGE("no kernel information query method is supported in current build.\n");
#endif
    }

    const auto& kernelModuleApplied =
        !programModuleOverride.isGlobalKernelSearch () ?
            programModuleOverride :
            programModule
    ;

    //--
    auto& kernels = kernelsByProgram[kernelModuleApplied.getModuleStemName ()];

    {
        const auto& k = kernels.find(kernelName);
        if(k != kernels.end ())
            return k->second;
    }

    auto& k = kernels[kernelName];
    k.name = kernelName;

#ifdef GFX_EMU_DEBUG_SYMBOLS_ACCESS_ENABLED
    k = DbgSymb::obj().getFunctionDesc(
        kernelName.c_str (),
        (void*)addr,
        kernelModuleApplied
    );

    if(!k.addr)
        fallback_getKernelDataFromShim(k, kernelModuleApplied);
#else
    GFX_EMU_WARNING_MESSAGE(fKernelSupport, "kernel data query via debug data is not supported in current build.\n");

    k.addr = GfxEmu::Utils::symbolNameToAddr(
        kernelModuleApplied.getModuleFileName ().c_str (),
        kernelName.c_str (),
        kernelName.c_str ()
    );

    const auto shimKernelInfoFound = fallback_getKernelDataFromShim(k, kernelModuleApplied);
    if(!shimKernelInfoFound)
        GFX_EMU_WARNING_MESSAGE(fKernelSupport, "kernel %s arguments data is not found.\n",
            kernelName.c_str ());
#endif

    if(!k.addr && addr) {
        GFX_EMU_MESSAGE(fKernelSupport | fDbgSymb, "setting kernel %s address to user-defined value of %p\n",
            kernelName.c_str (), addr);
        k.addr = reinterpret_cast<void*>(addr);
    }

    if (k.addr)
        addrToKernel[reinterpret_cast<void(*)()>(k.addr)] = &k;

    return k;
}

bool fallback_getKernelDataFromShim(GfxEmu::DbgSymb::FunctionDesc& k, const ProgramModule& programModule) {

    bool shimKernelInfoFound = false;

    // Use shim layer method (signature parsing) to extrapolate kernel argument types.
    const auto kernels2sigs = EnumerateKernels(
            programModule.getModuleHandle ()
        );

    for (auto& [name, info]: kernels2sigs) {

        if(k.name == name) {
            shimKernelInfoFound = true;

            if(!k.addr) {
                GFX_EMU_MESSAGE(fKernelSupport,
                    "getting kernel %s address from shim layer.\n",
                        k.name.c_str ());
                k.addr = info.func;
            }

            if(!k.params.size ()) {
                GFX_EMU_MESSAGE(fKernelSupport,
                    "getting kernel %s parameters data via shim signature parse.\n",
                        k.name.c_str ());

                for (const auto& shimArgType: ParseKernelSignature(info.signature)) {
                    GfxEmu::DbgSymb::ParamDesc argDesc;
                    switch(shimArgType) {
                        case CmArgumentType::Fp:
                            argDesc.isFloat = true;
                            break;
                        case CmArgumentType::SurfaceIndex:
                            argDesc.isClass = true;
                            break;
                        case CmArgumentType::Invalid:
                            GFX_EMU_FAIL_WITH_MESSAGE(fShim | fKernelSupport, "argument type is not"
                                " determined for kernel %s argument %u", k.name.c_str (), k.params.size ());
                        default: break;
                    }
                    k.params.push_back(argDesc);
                }
            }
        }
    }

    return shimKernelInfoFound;
}

void invalidateProgramModuleAssociatedData(const ProgramModule& desc) {
    GFX_EMU_DEBUG_MESSAGE(fKernelSupport | fExtraDetail,
            "invalidating kernel descriptions caches associated with %s\n", desc.toStr().c_str ()
        );
    const auto& key = desc.getModuleStemName ();
    for (auto& desc: kernelsByProgram[key]) {
        addrToKernel.erase(reinterpret_cast<void(*)()>(desc.second.addr));
    }
    kernelsByProgram.erase(key);
}

}; // namespace details

const GfxEmu::DbgSymb::FunctionDesc& getKernelDesc (
    std::string kernelName,
    VoidFuncPtr kernelAddr
) {
    return details::getKernelDesc_(kernelName, setupProgram(0), kernelAddr);
}

const GfxEmu::DbgSymb::FunctionDesc& getKernelDesc (
    std::string kernelName,
    const ProgramModule& programModule,
    VoidFuncPtr kernelAddr
) {
    return details::getKernelDesc_(kernelName, programModule, kernelAddr);
}

ProgramModule setupProgram (const std::string& programName) {
    return ProgramModule {programName};
}

ProgramModule setupProgram (void* addr, size_t size) {
    return ProgramModule {addr, size};
}

}; // namespace KernelSupport
}; // namespace GfxEmu
