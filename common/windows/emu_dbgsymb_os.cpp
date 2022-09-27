/*========================== begin_copyright_notice ============================

Copyright (C) 2021 Intel Corporation

SPDX-License-Identifier: MIT

============================= end_copyright_notice ===========================*/

#ifdef GFX_EMU_DEBUG_SYMBOLS_ACCESS_ENABLED

#include <string>
#include <vector>
#include <sstream>
#include <mutex>
#include <filesystem>
namespace fs = std::filesystem;

#include "emu_utils.h"
#include "emu_log.h"
#include "emu_dbgsymb.h"
#include "emu_dbgsymb_os.h"

namespace GfxEmu {
namespace DbgSymb {

using namespace GfxEmu::Log::Flags;

#define SYMBOL_INFO_BUFFER_AND_PTR(bufferName,ptrName) \
    char alignas(sizeof(max_align_t)) bufferName [sizeof(SYMBOL_INFO) + MAX_SYM_NAME * sizeof(TCHAR)];\
    auto ptrName = reinterpret_cast<PSYMBOL_INFO>(bufferName);\
    ptrName->SizeOfStruct = sizeof(SYMBOL_INFO);\
    ptrName->MaxNameLen = MAX_SYM_NAME;

//#define SYMBOL_INFO64_BUFFER_AND_PTR(bufferName,ptrName) \
//    char alignas(sizeof(max_align_t)) bufferName [sizeof(IMAGEHLP_SYMBOL64) + MAX_SYM_NAME * sizeof(CHAR)];\
//    auto ptrName = reinterpret_cast<PIMAGEHLP_SYMBOL>(bufferName);\
//    ptrName->SizeOfStruct = sizeof(IMAGEHLP_SYMBOL64);\
//    ptrName->MaxNameLength = MAX_SYM_NAME;

auto getTypeInfo = [](
        auto sessionHandle,
        auto& out,
        auto ModBase,
        auto id,
        auto query,
        const char* queryCStr)
{
    if(!::SymGetTypeInfo(
        sessionHandle,
        ModBase,
        id,
        query,
        &out)
    ) {
        GFX_EMU_DEBUG_MESSAGE(fDbgSymb | fDetail, "failed to query for %u %s: %s\n",
            query, queryCStr,
            GfxEmu::Utils::lastErrorStr ().c_str());
        return false;
    } else {
        std::stringstream ss; ss << out;
        GFX_EMU_DEBUG_MESSAGE(fDbgSymb | fDetail, "query %s: %s\n",
            queryCStr, ss.str().c_str());
    }
    return true;
};

#define GET_TYPE_INFO(out_type,out,base,id,query) \
    out_type out {}; auto out ## Res = getTypeInfo(sessionHandle,out,base,id,query,#query)

enum BasicType
{
    btNoType = 0,
    btVoid = 1,
    btChar = 2,
    btWChar = 3,
    btInt = 6,
    btUInt = 7,
    btFloat = 8,
    btBCD = 9,
    btBool = 10,
    btLong = 13,
    btULong = 14,
    btCurrency = 25,
    btDate = 26,
    btVariant = 27,
    btComplex = 28,
    btBit = 29,
    btBSTR = 30,
    btHresult = 31
};

#define ENTRY(v) #v,
const static std::vector<const char*> kSymTagNames {
    ENTRY(SymTagNull)
    ENTRY(SymTagExe)
    ENTRY(SymTagCompiland)
    ENTRY(SymTagCompilandDetails)
    ENTRY(SymTagCompilandEnv)
    ENTRY(SymTagFunction)
    ENTRY(SymTagBlock)
    ENTRY(SymTagData)
    ENTRY(SymTagAnnotation)
    ENTRY(SymTagLabel)
    ENTRY(SymTagPublicSymbol)
    ENTRY(SymTagUDT)
    ENTRY(SymTagEnum)
    ENTRY(SymTagFunctionType)
    ENTRY(SymTagPointerType)
    ENTRY(SymTagArrayType)
    ENTRY(SymTagBaseType)
    ENTRY(SymTagTypedef)
    ENTRY(SymTagBaseClass)
    ENTRY(SymTagFriend)
    ENTRY(SymTagFunctionArgType)
    ENTRY(SymTagFuncDebugStart)
    ENTRY(SymTagFuncDebugEnd)
    ENTRY(SymTagUsingNamespace)
    ENTRY(SymTagVTableShape)
    ENTRY(SymTagVTable)
    ENTRY(SymTagCustom)
    ENTRY(SymTagThunk)
    ENTRY(SymTagCustomType)
    ENTRY(SymTagManagedType)
    ENTRY(SymTagDimension)
    ENTRY(SymTagCallSite)
    ENTRY(SymTagInlineSite)
    ENTRY(SymTagBaseInterface)
    ENTRY(SymTagVectorType)
    ENTRY(SymTagMatrixType)
    ENTRY(SymTagHLSLType)
    ENTRY(SymTagCaller)
    ENTRY(SymTagCallee)
    ENTRY(SymTagExport)
    ENTRY(SymTagHeapAllocationSite)
    ENTRY(SymTagCoffGroup)
    ENTRY(SymTagMax)
    #undef ENTRY
    ""
};

static BOOL CALLBACK findVectorOrMatrixPramInitFunction(
    SYMBOL_INFO* pSymbol,
    ULONG SymbolSize,
    PVOID UserData )
{
    auto& arg = *reinterpret_cast<GfxEmu::DbgSymb::ParamDesc*> (UserData);
    arg.isVectorOrMatrix = true;
    arg.paramInitFunctionLinkageName = pSymbol->Name;
    arg.paramInitFunctionAddr = reinterpret_cast<void*> (pSymbol->Address);
    return true;
}

static BOOL CALLBACK fillParameterNames(
    SYMBOL_INFO* pSymbol,
    ULONG SymbolSize,
    PVOID UserData )
{
    auto& userData = *reinterpret_cast<GfxEmu::DbgSymb::ArgVisitorParams*> (UserData);

    if (!(pSymbol->Flags & SYMFLAG_PARAMETER))
        return true;

    auto tag = (enum SymTagEnum)0;
    if(!::SymGetTypeInfo(
            GetCurrentProcess (),
            pSymbol->ModBase,
            pSymbol->TypeIndex,
            TI_GET_SYMTAG,
            &tag)
    ) {
        GFX_EMU_DEBUG_MESSAGE(fDbgSymb | fDetail, "Failed to get symbol tag: %s",
            GfxEmu::Utils::lastErrorStr ().c_str());
        return false;
    }

    userData.kernelDescPtr->params.at(userData.curParamIdx).name = pSymbol->Name;
    userData.curParamIdx++;

    GFX_EMU_DEBUG_MESSAGE(fDbgSymb | fDetail, "got parameter symbol %s %u mod:%x,%x func:%x tag: %u\n",
        pSymbol->Name,
        pSymbol->TypeIndex,
        pSymbol->ModBase,
        pSymbol->ModBase,
        userData.funcAddr,
        tag
    );

    return true;
}

DbgSymbOsImpl::DbgSymbOsImpl () {
    const auto ver = ImagehlpApiVersion();
    GFX_EMU_MESSAGE(fDbgSymb, "DbgHelp API ver: %u.%u rev. %u\n",
        ver->MajorVersion,
        ver->MinorVersion,
        ver->Revision
    );

    sessionHandle = GetCurrentProcess();

    ::SymSetOptions(
        SymGetOptions() |
        SYMOPT_DEBUG |
        SYMOPT_DEFERRED_LOADS |
        SYMOPT_UNDNAME
    );

    if(!::SymInitialize (
            sessionHandle,
            NULL,
            false))
    {
        GFX_EMU_WARNING_MESSAGE(fDbgSymb, "Failed to SymInitialize: %s \n",
            GfxEmu::Utils::lastErrorStr ().c_str());
        return;
    }
}

DbgSymbOsImpl::~DbgSymbOsImpl () {
    if(!::SymCleanup(sessionHandle))
        GFX_EMU_FAIL_WITH_MESSAGE("SymCleanup() failed. Error code: %s \n",
            GfxEmu::Utils::lastErrorStr ().c_str());
}

std::string DbgSymbOsImpl::linkageNameToName (const char *linkageName) {
    std::string name;
    name.resize (1024, '\0');
    if(int count = ::UnDecorateSymbolName(linkageName, name.data(), 1024,
        UNDNAME_COMPLETE
    )) {
        name.resize(count);
    } else name.resize(0);
    return name;
}

std::string DbgSymbOsImpl::nameToLinkageName (const char *name) {
    GFX_EMU_WARNING_MESSAGE(fDbgSymb, "symbol name to linkage name is not implemented due to: unused.")
    return name;
}

void* DbgSymbOsImpl::nameToAddr (const char *name) {
    SYMBOL_INFO_BUFFER_AND_PTR(buffer,pSymbol);
    if (!::SymFromName(sessionHandle, name, pSymbol))
        GFX_EMU_FAIL_WITH_MESSAGE("SymFromName returned error : %s\n",
            GfxEmu::Utils::lastErrorStr ().c_str());

    auto addr = reinterpret_cast<void*>(pSymbol->Address);

    GFX_EMU_DEBUG_MESSAGE(fDbgSymb | fDetail, "nameToAddr: symbol %s is found at %p\n",
        name, addr);

    return addr;
}

SymbDesc DbgSymbOsImpl::addrToSymbDesc (void *addr) {
    SymbDesc d;
    DWORD64 dwDisplacement = 0;
    auto dwAddress = reinterpret_cast<DWORD64> (addr);

    SYMBOL_INFO_BUFFER_AND_PTR(s1buf,pSymbol);

    if (!::SymFromAddr(sessionHandle, dwAddress, &dwDisplacement, pSymbol)) {
        GFX_EMU_WARNING_MESSAGE(fDbgSymb, "SymFromAddr returned error : %s\n",
            GfxEmu::Utils::lastErrorStr ().c_str ());
    } else {
        d.name = linkageNameToName(pSymbol->Name);
        d.addr = reinterpret_cast<void*> (pSymbol->Address);
        GFX_EMU_DEBUG_MESSAGE(fDbgSymb | fDetail, "addrToSymbDesc: symbol Name is %s [ %s ], address is %p vs %p\n",
             d.name.c_str (), pSymbol->Name, d.addr, addr);
    }

    return d;
}

bool GetChildren (
    HANDLE   hProcess,
    DWORD64  ModuleBase,
    ULONG    Index,
    ULONG*   pChildren,
    DWORD&   NumChildren,
    DWORD    MaxChildren
) {
    if(!pChildren || !MaxChildren)
        return false;

    DWORD childCount = 0;
    if(!SymGetTypeInfo( hProcess, ModuleBase, Index, TI_GET_CHILDRENCOUNT, &childCount )) {
        DWORD ErrCode = GetLastError();
        return false;
    }

    if(childCount == 0) {
        NumChildren = 0;
        return true;
    }

    int FindChildrenSize = sizeof(TI_FINDCHILDREN_PARAMS) + childCount*sizeof(ULONG);
    TI_FINDCHILDREN_PARAMS* pFC = (TI_FINDCHILDREN_PARAMS*)_alloca( FindChildrenSize );
    memset( pFC, 0, FindChildrenSize );
    pFC->Count = childCount;

    if(!SymGetTypeInfo(hProcess, ModuleBase, Index, TI_FINDCHILDREN, pFC)) {
        return false;
    }

    NumChildren = std::min(childCount,MaxChildren);
    for(DWORD i = 0; i < NumChildren; i++) {
        pChildren[i] = pFC->ChildId[i];
    }

    return true;
}

//-------------------------------------------------
struct setLoadedProgramModuleInfoCallback_data {
    GfxEmu::KernelSupport::ProgramModule* descPtr {nullptr};
    std::string queryModuleName;
    DWORD64 queryModuleAddr {0};
    bool found {false};
};

static BOOL CALLBACK getVectorOfModules(
    PCSTR ModuleName,
    DWORD64 ModuleBase,
    ULONG ModuleSize,
    PVOID UserContext
) {
    if (GfxEmu::Utils::isNotAKernelProgram(ModuleName))
        return true;

    auto& data = *(std::vector<std::pair<std::string,uint64_t>>*)UserContext;
    data.emplace_back(ModuleName,ModuleBase);
    return true;
}

//PENUMLOADED_MODULES_CALLBACK64 setLoadedProgramModuleInfoCallback;
static BOOL CALLBACK setLoadedProgramModuleInfoCallback(
    PCSTR ModuleName,
    DWORD64 ModuleBase,
    ULONG ModuleSize,
    PVOID UserContext
) {
    if (GfxEmu::Utils::isNotAKernelProgram(ModuleName))
        return true;

    //GFX_EMU_DEBUG_MESSAGE(fDbgSymb | fExtraDetail, "checking loaded module name: %s\n", ModuleName);
    auto& data = *reinterpret_cast<setLoadedProgramModuleInfoCallback_data*>(UserContext);

    if (data.found)
        return true;

    GFX_EMU_DEBUG_MESSAGE(fDbgSymb | fDetail,
            "checking module %s, base: %p (query name: %s, query addr: %p).\n",
            ModuleName, ModuleBase, data.queryModuleName.c_str(), data.queryModuleAddr);

    if ((ModuleName && data.queryModuleName != "" && (
         data.queryModuleName.find(ModuleName) != std::string::npos ||
         std::string{ModuleName}.find(data.queryModuleName) != std::string::npos )
        ) || (
            (data.queryModuleAddr > ModuleBase) && (data.queryModuleAddr < (ModuleBase + ModuleSize))
        ))
    {
        data.descPtr->setModuleAddr(reinterpret_cast<void*>(ModuleBase));
        //data.descPtr->setModuleEndAddr(reinterpret_cast<void*>(ModuleBase + ModuleSize));
        data.descPtr->setModuleFileName(ModuleName, false);
        GFX_EMU_DEBUG_MESSAGE(fDbgSymb | fDetail,
            "found  module %s.\n",
            data.descPtr->getModuleFileName ().c_str());
        data.found = true;
        return true;
    }

    return true;
}

bool DbgSymbOsImpl::setLoadedProgramModuleInfo(
    void *addr,
    const std::string& programName,
    GfxEmu::KernelSupport::ProgramModule& desc
)
{
    GFX_EMU_ASSERT(addr || programName.size ());

    setLoadedProgramModuleInfoCallback_data data;
    data.queryModuleName = programName;
    data.queryModuleAddr = reinterpret_cast<DWORD64>(addr);
    data.descPtr = &desc;

    ::EnumerateLoadedModules64(
        sessionHandle,
        setLoadedProgramModuleInfoCallback,
        &data);

    if(!::SymRefreshModuleList(sessionHandle))
        GFX_EMU_FAIL_WITH_MESSAGE("can't refresh program module list.");

    return true;
}

GfxEmu::DbgSymb::FunctionDesc
DbgSymbOsImpl::getFunctionDesc (
    const char *kernelName,
    const void *addr,
    const GfxEmu::KernelSupport::ProgramModule& programModule
) {
    static std::mutex mtx;
    std::lock_guard lock (mtx);

    GfxEmu::DbgSymb::FunctionDesc kernelDesc;
    kernelDesc.addr = (void*)addr;

    auto& params = kernelDesc.params;

    if(programModule.isGlobalKernelSearch ())
        if(!::SymRefreshModuleList(sessionHandle))
            GFX_EMU_FAIL_WITH_MESSAGE("Can't refresh module list.");

    auto moduleBaseAddr = reinterpret_cast<uint64_t>(programModule.getModuleAddr ()); // pSymbol->ModBase
    std::vector<std::pair<std::string,uint64_t>> modules;

    if(programModule.isGlobalKernelSearch ())
        ::EnumerateLoadedModules64(
            sessionHandle,
            getVectorOfModules,
            &modules);
    else
        modules.emplace_back(programModule.getModuleStemName (), moduleBaseAddr);

    SYMBOL_INFO_BUFFER_AND_PTR(s1buf,pSymbol);
    auto tag = (enum SymTagEnum)0;
    bool found = false;

    std::string lookupKernelNameNoModule = kernelName;

    {
        static const auto rxes = {
            std::regex{",[[:space:]]+"},
            std::regex{"long[[:space:]]+long"},
            std::regex{"([,<[:space:]])half(?!_)"}
        };

        static const auto replacement = {
            ",",
            "__int64",
            "$1half_float::half"
        };

        for(int i = 0; i < rxes.size (); i++)
            lookupKernelNameNoModule = std::regex_replace (
                lookupKernelNameNoModule, *(rxes.begin() + i),
                    *(replacement.begin () + i));
    }

    for(auto module: modules) {

        const auto lookupKernelName =
            GfxEmu::KernelSupport::ProgramModule{module.first}.getModuleStemName() + "!" +
            lookupKernelNameNoModule;

        GFX_EMU_MESSAGE(fDbgSymb | fInfo, "looking for debug info for the kernel %s, "
            "lookup kernel name %s\n", kernelName, lookupKernelName.c_str ());

        if (!::SymFromName(sessionHandle, lookupKernelName.c_str(), pSymbol)) {
            GFX_EMU_WARNING_MESSAGE(fDbgSymb, "can't find debug info for the kernel %s, "
                "lookup kernel name %s\n", kernelName, lookupKernelName.c_str ());
            continue;
        }

        GFX_EMU_DEBUG_MESSAGE(fDbgSymb | fDetail, "kernel name from symbol data: %s\n", pSymbol->Name);
        GFX_EMU_DEBUG_MESSAGE(fDbgSymb | fDetail, "kernel address from symbol data: 0x%x\n", pSymbol->Address);

        if(!::SymGetTypeInfo(
            sessionHandle,
            module.second,
            pSymbol->TypeIndex,
            TI_GET_SYMTAG,
            &tag)
        ) {
            GFX_EMU_DEBUG_MESSAGE(fDbgSymb | fDetail,"Failed to get symbol tag for kernel %s: %s",
                lookupKernelName.c_str(),
                GfxEmu::Utils::lastErrorStr ().c_str());
            continue;
        } else {
            found = true;
            moduleBaseAddr = module.second;
            break;
        }
    }

    if(!found) return {};

    if (tag == SymTagEnum::SymTagFunctionType) {
        GFX_EMU_DEBUG_MESSAGE(fDbgSymb | fDetail, "found function %s, type index: %u mod:%x, tag: %u\n",
            pSymbol->Name,
            pSymbol->TypeIndex,
            moduleBaseAddr,
            tag
        );
    } else {
        GFX_EMU_WARNING_MESSAGE(fDbgSymb, "symbol's tag is not of a function type.\n");
        return {};
    }

    if(pSymbol->Address == 0) {
        GFX_EMU_WARNING_MESSAGE(fKernelSupport | fDbgSymb | fDetail,
            "WIN API gives no address for the function %s. User-provided address is %p\n",
                kernelName, addr);
    } else
        kernelDesc.addr = reinterpret_cast<void*>(pSymbol->Address);

    if(addr && kernelDesc.addr && addr != kernelDesc.addr)
        GFX_EMU_WARNING_MESSAGE(fKernelSupport | fDbgSymb | fExtraDetail,
            "function %s address provided by the user %p "
            " is not equal to reported by the WIN API: %p\n",
                kernelName, addr, kernelDesc.addr);

    GFX_EMU_MESSAGE_SCOPE_PREFIX("@params: ");

    constexpr auto childIndexesMax = 255;
    ULONG childIndexes[childIndexesMax];
    DWORD childIndexesCount = 0;

    if(!GetChildren (
        sessionHandle,
        moduleBaseAddr,
        pSymbol->TypeIndex,
        childIndexes,
        childIndexesCount,
        childIndexesMax
    ))
    {
        return {};
    }

    GFX_EMU_DEBUG_MESSAGE(fDbgSymb | fDetail, "params found: %u\n", childIndexesCount);

    for (DWORD i = 0; i < childIndexesCount; i++) {
        GFX_EMU_DEBUG_MESSAGE(fDbgSymb | fDetail, "--------------------------\n");
        const auto index = childIndexes[i];
        params.resize(i + 1);

        GET_TYPE_INFO(DWORD,type,moduleBaseAddr,index, TI_GET_TYPE);
        GET_TYPE_INFO(DWORD,typeId,moduleBaseAddr,index, TI_GET_TYPEID);
        GET_TYPE_INFO(enum SymTagEnum,symTag,moduleBaseAddr,typeId,TI_GET_SYMTAG);

        GFX_EMU_DEBUG_MESSAGE(fDbgSymb | fDetail, "tag: %s\n", kSymTagNames[symTag]);

        GET_TYPE_INFO(ULONG64,length,moduleBaseAddr,typeId, TI_GET_LENGTH);
        params[i].size = length;
        GFX_EMU_DEBUG_MESSAGE(fDbgSymb | fDetail, "length: %u\n", length);

        switch (symTag) {
            case SymTagUDT: {
                GET_TYPE_INFO(WCHAR*,wname,moduleBaseAddr,typeId,TI_GET_SYMNAME);
                if (wnameRes) {
                    params[i].typeName = GfxEmu::Utils::wstringToString(wname);
                    GFX_EMU_DEBUG_MESSAGE(fDbgSymb | fDetail, "name: %s\n", params[i].typeName.c_str ());
                }
                params[i].isClass = true;
                GFX_EMU_DEBUG_MESSAGE(fDbgSymb | fExtraDetail, "is a class type.\n");

                ::SymEnumSymbols(
                    sessionHandle,
                    moduleBaseAddr,
                    (std::string("*") + params[i].typeName + "*emuKernelParamInit__*").c_str (),
                    findVectorOrMatrixPramInitFunction,
                    &(params[i]));
            break;}
            case SymTagPointerType: {
                GFX_EMU_DEBUG_MESSAGE(fDbgSymb | fDetail, "is a pointer.\n");
                params[i].isPointer = true;
            break;}
            case SymTagBaseType: {
                GET_TYPE_INFO(DWORD,baseType,moduleBaseAddr,typeId,TI_GET_BASETYPE);
                if (baseType == BasicType::btFloat) {
                    GFX_EMU_DEBUG_MESSAGE(fDbgSymb | fDetail, "is fp.\n");
                    params[i].isFloat = true;
                }
            break;}
            default: {
                GET_TYPE_INFO(DWORD,typeId2,moduleBaseAddr,typeId,TI_GET_TYPEID);
                if (typeId2Res) {
                    GET_TYPE_INFO(enum SymTagEnum,symTag,moduleBaseAddr,typeId2,TI_GET_SYMTAG);
                    GFX_EMU_DEBUG_MESSAGE(fDbgSymb | fDetail, "tag2: %u\n", symTag);
                }
            break;}
        };

        GET_TYPE_INFO(DWORD,symIdx,moduleBaseAddr,index,TI_GET_SYMINDEX);
    }

    IMAGEHLP_STACK_FRAME sf;
    sf.InstructionOffset = pSymbol->Address;

    if (!::SymSetContext(
            sessionHandle,
            &sf,
            0) && ::GetLastError() != ERROR_SUCCESS)
    {
        GFX_EMU_FAIL_WITH_MESSAGE("failed to SymSetContext: %s.", GfxEmu::Utils::lastErrorStr ().c_str());
    }

    GfxEmu::DbgSymb::ArgVisitorParams data;
    data.kernelDescPtr = &kernelDesc;
    data.funcAddr = reinterpret_cast<void*>(pSymbol->Address);
    ::SymEnumSymbols(
        sessionHandle,
        0,
        0,
        fillParameterNames,
        &data);

    return kernelDesc;
}

DbgSymbIface::DbgSymbIface () {
    impl = new DbgSymbOsImpl;
}

DbgSymbIface::~DbgSymbIface () {
    delete impl;
}

GfxEmu::DbgSymb::FunctionDesc DbgSymbIface::getFunctionDesc (
    const char *kernelName,
    const void *addr,
    const GfxEmu::KernelSupport::ProgramModule& programModule
) {
    return impl->getFunctionDesc(kernelName, addr, programModule);
}

bool DbgSymbIface::setLoadedProgramModuleInfo(void *addr, const std::string& programName, GfxEmu::KernelSupport::ProgramModule& desc) {
    return impl->setLoadedProgramModuleInfo(addr, programName, desc);
}

GfxEmu::DbgSymb::SymbDesc DbgSymbIface::addrToSymbDesc(void *addr) {
    return impl->addrToSymbDesc(addr);
}

}; // namespace DbgSymb
}; // namespace GfxEmu

#undef GET_TYPE_INFO

#endif // GFX_EMU_DEBUG_SYMBOLS_ACCESS_ENABLED
