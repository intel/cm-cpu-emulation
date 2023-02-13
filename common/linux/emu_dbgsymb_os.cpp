/*========================== begin_copyright_notice ============================

Copyright (C) 2021 Intel Corporation

SPDX-License-Identifier: MIT

============================= end_copyright_notice ===========================*/

#ifdef GFX_EMU_DEBUG_SYMBOLS_ACCESS_ENABLED

#include <cstring>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <regex>
#include <vector>
#include <cxxabi.h>

#include <link.h>

#include <dwarf.h>
#include <elfutils/version.h>
#include <elfutils/libdw.h>
#include <elfutils/libdwfl.h>

#define DWARF_GET_UNITS_REQ_MAJOR 0
#define DWARF_GET_UNITS_REQ_MINOR 171

#if _ELFUTILS_PREREQ( DWARF_GET_UNITS_REQ_MAJOR, DWARF_GET_UNITS_REQ_MINOR)
    #define ELFUTILS_HAS_DWARF_GET_UNITS
#endif

//#include "emu_dbgsymb_types.h"
#include "emu_log.h"
#include "emu_dbgsymb.h"
#include "emu_dbgsymb_os.h"
#include "emu_utils.h"
#include "emu_kernel_support_types.h"

namespace GfxEmu {
namespace DbgSymb {

using namespace GfxEmu::Log::Flags;
std::vector<std::string> DbgSymbOsImpl::fundTplTypeNameVariants {{
    "short",
    "long",
    "long long",
}};

void DbgSymbOsImpl::fundamentalTypeNameVariantsInit () {
    const auto limit = fundTplTypeNameVariants.size ();
    const auto dbSize = limit*3 + limit;
    fundTplTypeNameVariants.reserve(dbSize);
    for(bool forGccProducer: {true, false})
        fundTplTypeNameRepls[forGccProducer].reserve(dbSize);

    int i = 0;
    for (const auto type: fundTplTypeNameVariants) {
        if (i > limit) break;
        for (auto s: {"", " int"}) {
            fundTplTypeNameVariants.push_back("unsigned " + type + s);
            fundTplTypeNameVariants.push_back(type + " unsigned" + s);
        }
        i++;
    }

    std::sort(fundTplTypeNameVariants.begin(), fundTplTypeNameVariants.end(),
        [](const auto& a, const auto& b){ return a.compare(b);});
    std::sort(fundTplTypeNameVariants.begin(), fundTplTypeNameVariants.end(),
        [](const auto& a, const auto& b){ return a.length() > b.length();});

    for(bool forGccProducer: {true, false})
    for(auto& v: fundTplTypeNameVariants) {
        std::string repl = "";
        bool usFirst = forGccProducer ? false : true;
        bool addInt = forGccProducer ? true : false;

        const bool
            isUns = v.find("unsigned") != std::string::npos,
            isLL  = v.find("long long") != std::string::npos,
            isL   = !isLL && v.find("long") != std::string::npos,
            isS   = v.find("short") != std::string::npos,
            isI   = !(isLL || isL || isS) && v.find("int") != std::string::npos
        ;

        repl =
            std::string(isUns && usFirst ? "unsigned " : "") +
            std::string(isLL ? "long long" : isL ? "long" : isS ? "short" : "") +
            std::string(isUns && !usFirst ? " unsigned" : "") +
            std::string(isI || addInt ? " int" : "")
        ;

        if(forGccProducer) // i.e. do only once.
            fundTplTypeNameVariantsRx.emplace_back(std::string{v} + "(?=\\S*[,>])", std::regex::ECMAScript);
        fundTplTypeNameRepls[forGccProducer].emplace_back(repl);
    }

#if 0
    for (const auto& v: fundTplTypeNameVariants)
        std::cout << v << "\n";

    std::cout << "\n================\n";
    for(bool forGccProducer: {true, false})
    for (const auto& v: fundTplTypeNameRepls[forGccProducer])
        std::cout << v << "\n";
#endif
}

DbgSymbOsImpl::DbgSymbOsImpl () {
    fundamentalTypeNameVariantsInit ();
}

DbgSymbOsImpl::~DbgSymbOsImpl () {
}

std::string DbgSymbOsImpl::translateTemplatedKernelName (const char *kernelName, bool isGcc) {

    std::string out {kernelName};

    for (int i = 0; i < fundTplTypeNameVariants.size (); i++) {
        const auto& variantRx = fundTplTypeNameVariantsRx[i];
        const auto& replacement = fundTplTypeNameRepls[isGcc][i];

        out = std::regex_replace (out, variantRx, replacement, std::regex_constants::match_any);
    }

    if (out.compare(kernelName)) {
        GFX_EMU_DEBUG_MESSAGE(fDbgSymb | fDetail, "kernel name changed for lookup: %s -> %s\n", kernelName, out.c_str());
    }

    return out;
}

SymbDesc DbgSymbOsImpl::addrToSymbDesc (void *addr) {
    Dl_info info;
    SymbDesc out;
    if(dladdr(addr, &info) && info.dli_sname) {
        std::string name;
        if (info.dli_sname[0] == '_') {
            char *demangled = nullptr;
            int status = -1;
            demangled = abi::__cxa_demangle(info.dli_sname, NULL, 0, &status);
            if(status == 0) {
                if(char* p = std::strchr(demangled, '('))
                    *p = 0;
                name = demangled;
            }
        } else name = info.dli_sname;

        //const auto name = linkageNameToName(info.dli_sname);
        GFX_EMU_DEBUG_MESSAGE(fDbgSymb | fDetail,
            "Symbol found by address %p: %s %s", addr, info.dli_sname, name.c_str ());
        out.name = name;
        out.addr = info.dli_saddr;
    }
    return out;
}

using DwarfCallbackT = int(*)(Dwarf_Die*,void*);

static inline int dwarfCallback_fillFunctionDesc (
    Dwarf_Die *die,
    void *params_
) {
    auto params = reinterpret_cast<ArgVisitorParams*> (params_);
    //auto funcAddr = reinterpret_cast<Dwarf_Addr> (params->funcAddr);
    const char *funcName = params->funcName.c_str ();

    auto name = dwarf_diename(die);

    if (strcmp(name, funcName) != 0) {
        return DWARF_CB_OK; // Look further...
    }

    Dwarf_Addr dwarf_epc {0};
    dwarf_entrypc(die, &dwarf_epc);

    const auto hasChildren = dwarf_haschildren(die);

    Dwarf_Attribute linkageNameAttr;
    if(dwarf_attr_integrate(die,DW_AT_linkage_name,&linkageNameAttr)) {
         params->kernelDescPtr->linkageName = dwarf_formstring (&linkageNameAttr);
         GFX_EMU_DEBUG_MESSAGE(fDbgSymb | fDetail, "function linkage name: %s\n",
            params->kernelDescPtr->linkageName.c_str ());
    } else
        params->kernelDescPtr->linkageName= funcName;

    if (dwarf_hasattr_integrate(die,DW_AT_declaration)) {
        GFX_EMU_DEBUG_MESSAGE(fDbgSymb | fDetail, "is declaration\n");
    }

    GFX_EMU_DEBUG_MESSAGE(fDbgSymb | fDetail, "function name: %s\n", name);
    GFX_EMU_DEBUG_MESSAGE(fDbgSymb | fDetail, "die has children: %u\n", hasChildren);
    GFX_EMU_DEBUG_MESSAGE(fDbgSymb | fDetail, "dwarf entry pc: %p\n", dwarf_epc);

    if (!hasChildren) {
        params->isFound = true;
        return DWARF_CB_ABORT;
    }

    Dwarf_Die paramSearchDie;
    dwarf_child(die, &paramSearchDie);

    GFX_EMU_DEBUG_MESSAGE(fDbgSymb | fDetail, "------ params ------\n");

    // Loop over function's child nodes in search of DW_TAG_formal_parameter.
    GFX_EMU_MESSAGE_SCOPE_PREFIX(std::string{"kernel "} + name + " parameter ");
    int paramIdx = 0;
    while (true) {
        GFX_EMU_MESSAGE_SCOPE_PREFIX(std::to_string(paramIdx) + ": ");

        if (dwarf_tag(&paramSearchDie) == DW_TAG_template_type_parameter ||
            dwarf_tag(&paramSearchDie) == DW_TAG_template_value_parameter) {
            GFX_EMU_DEBUG_MESSAGE(fDbgSymb | fExtraDetail, "skipping template parameter DIE.\n");
            if (dwarf_siblingof(&paramSearchDie, &paramSearchDie) != 0)
                break;
            continue;
        }

        if (dwarf_tag(&paramSearchDie) != DW_TAG_formal_parameter) {
            GFX_EMU_DEBUG_MESSAGE(fDbgSymb | fExtraDetail, "got non-formal-parameter DIE, search complete.\n");
            break;
        }

        GFX_EMU_DEBUG_MESSAGE(fDbgSymb | fDetail, "name: %s\n",
             dwarf_diename(&paramSearchDie));

        if (!dwarf_hasattr(&paramSearchDie,DW_AT_type)) {
            GFX_EMU_ERROR_MESSAGE(fDbgSymb, "no param type attribute found!\n");
            break;
        }

        Dwarf_Attribute attr;
        if(!dwarf_attr_integrate(&paramSearchDie,DW_AT_type,&attr)) {
            GFX_EMU_ERROR_MESSAGE(fDbgSymb, "failed to get param type attribute value.\n");
            break;
        }

        Dwarf_Die param_type_die;
        auto typeDiePtr = dwarf_formref_die(&attr,&param_type_die);
        if (!typeDiePtr) {
            GFX_EMU_ERROR_MESSAGE(fDbgSymb, "failed to get param type die.\n");
            break;
        }

        params->kernelDescPtr->params.resize(paramIdx + 1);
        //params->kernelDescPtr->params.push_back({});

        params->kernelDescPtr->params.at(paramIdx).name = dwarf_diename(&paramSearchDie);

        while(typeDiePtr) {

            int peel_res = dwarf_peel_type(typeDiePtr, typeDiePtr);

            if (peel_res != 0) {
                GFX_EMU_ERROR_MESSAGE(fDbgSymb, "failed to peel type further, not enough debug info.\n");
                break;
            }

            const auto tag = dwarf_tag(typeDiePtr);
            if (tag == DW_TAG_pointer_type ||
                tag == DW_TAG_reference_type) {
                GFX_EMU_DEBUG_MESSAGE(fDbgSymb | fDetail, "is a pointer or reference.\n");
                params->kernelDescPtr->params.at(paramIdx).isPointer = true;
                if (!dwarf_attr_integrate(typeDiePtr,DW_AT_type,&attr)) {
                    GFX_EMU_ERROR_MESSAGE(fDbgSymb, "failed to get param type attribute.\n");
                    break;
                }
                typeDiePtr = dwarf_formref_die(&attr,typeDiePtr);
                continue;
            }

            const char *typeName = dwarf_diename(typeDiePtr);
            params->kernelDescPtr->params.at(paramIdx).typeName = typeName;
            GFX_EMU_DEBUG_MESSAGE(fDbgSymb | fDetail, "type: %s.\n", typeName);

            if (dwarf_tag(typeDiePtr) == DW_TAG_class_type) {
                params->kernelDescPtr->params.at(paramIdx).isClass = true;
                GFX_EMU_DEBUG_MESSAGE(fDbgSymb | fExtraDetail, "is a class type.\n");
            }

            {
                if(!dwarf_attr_integrate(typeDiePtr,DW_AT_byte_size,&attr)) {
                    GFX_EMU_WARNING_MESSAGE(fDbgSymb, "failed to get param size.\n");
                }

                dwarf_formudata(&attr, &(params->kernelDescPtr->params.at(paramIdx).size));
            }

            if (
                strcmp(typeName, "float") == 0 ||
                strcmp(typeName, "double") == 0
            ) {
                params->kernelDescPtr->params.at(paramIdx).isFloat = true;
            }

            for(const auto& specialType: {"vector", "matrix"}) {
                if(!std::regex_search(typeName,std::regex{specialType,std::regex::icase}))
                    continue;

                const char *paramInitFunctionName = "emuKernelParamInit__";
                params->kernelDescPtr->params.at(paramIdx).isVectorOrMatrix = true;

                Dwarf_Die tmp;
                dwarf_child(typeDiePtr, &tmp);

                bool lookForParamInitFunction = true;
                while(lookForParamInitFunction) {
                    const auto tag = dwarf_tag(&tmp);
                    if (tag == DW_TAG_subprogram &&
                        std::string{dwarf_diename(&tmp)} == paramInitFunctionName) {
                            lookForParamInitFunction = false;
                            if(dwarf_attr_integrate(&tmp,DW_AT_linkage_name,&linkageNameAttr)) {
                                 params->kernelDescPtr->params.at(paramIdx).paramInitFunctionLinkageName = dwarf_formstring (&linkageNameAttr);
                                 GFX_EMU_DEBUG_MESSAGE(fDbgSymb | fDetail,
                                    "param init method linkage name for argument %u: %s\n",
                                    paramIdx,
                                    params->kernelDescPtr->params.at(paramIdx).paramInitFunctionLinkageName.c_str ());
                            }
                            break;
                    }

                    if (dwarf_siblingof(&tmp, &tmp) != 0)
                        break;
                }

                if(lookForParamInitFunction) {
                    GFX_EMU_WARNING_MESSAGE(fDbgSymb, "can't find linkage name "
                        "of param init method %s for argument %u.\n",
                            paramInitFunctionName, paramIdx);
                } else {
                    params->kernelDescPtr->params.at(paramIdx).paramInitFunctionAddr =
                        GfxEmu::Utils::symbolNameToAddr(
                            params->moduleFileName.c_str (),
                            params->kernelDescPtr->params.at(paramIdx).paramInitFunctionLinkageName.c_str (),
                            params->kernelDescPtr->params.at(paramIdx).paramInitFunctionLinkageName.c_str ()
                        );

                    if(params->kernelDescPtr->params.at(paramIdx).paramInitFunctionAddr == nullptr) {
                        GFX_EMU_WARNING_MESSAGE(fDbgSymb, "can't find address "
                            "of param init method %s (%s) for argument %u.\n",
                                paramInitFunctionName,
                                params->kernelDescPtr->params.at(paramIdx).paramInitFunctionLinkageName.c_str (),
                                paramIdx);
                    }
                }
            }

            paramIdx++;
            params->isFound = true;
            break;
        }

        if (dwarf_siblingof(&paramSearchDie, &paramSearchDie) != 0)
            break;
    }

    return DWARF_CB_ABORT;
}

bool checkDwarfError () {
    const auto e = dwarf_errno ();
    if (e) {
        GFX_EMU_ERROR_MESSAGE("Errno: %u %s", e, dwarf_errmsg (e));
        return false;
    }
    return true;
};

struct ProgramNames_callback_data {
    std::vector<std::string> programNames;
    std::vector<std::string> filter;
};

int getAndFilterProgramNames_callback(struct dl_phdr_info *info, size_t size, void *data_) {
    auto& data = *reinterpret_cast<ProgramNames_callback_data*>(data_);

    std::string curLibName = info->dlpi_name;
    if (curLibName == "") curLibName = GfxEmu::Utils::getMainProgramName ();

    if(GfxEmu::Utils::isNotAKernelProgram(curLibName.c_str())) {
        // GFX_EMU_MESSAGE("Skipping %s \n", curLibName.c_str());
        return false;
    }

    bool matches = !data.filter.size ();
    if(!matches)
        for (const auto& f: data.filter)
            if (curLibName.find(f) != std::string::npos) {
                matches = true;
                break;
            }

    if (!matches) {
        GFX_EMU_DEBUG_MESSAGE(fDbgSymb | fExtraDetail,
            "library %s didn't pass filter of %s.\n",
            curLibName.c_str (), data.filter.size () ? data.filter[0].c_str () : "<no filter>");
        return 0;
    }

    if (curLibName != "") {
        GFX_EMU_DEBUG_MESSAGE(fDbgSymb | fDetail,
            "shall inspect debug symbols in %s.\n", curLibName.c_str ());
        data.programNames.push_back(curLibName);
    }

    return 0;
}

GfxEmu::DbgSymb::FunctionDesc
DbgSymbOsImpl::getFunctionDesc (
    const char *kernelName,
    const void *addr,
    const GfxEmu::KernelSupport::ProgramModule& programModule
) {
    GfxEmu::DbgSymb::FunctionDesc kernelDesc;
    kernelDesc.name = kernelName;
    kernelDesc.addr = (void*)addr;

    const auto& programName = programModule.getModuleFileName ();

    auto find_kernel_data_ = [&](
            const char* kernelName,
            Dwarf_Die& cudie,
            bool& isFoundFlag)
    {

        if (isFoundFlag) return;

        ArgVisitorParams params;
        params.funcName = kernelName;
        params.kernelDescPtr = &kernelDesc;
        params.moduleFileName = programName.c_str ();

        // For templated kernels we translate arbitrary fundamental types names to
        // those expected to be found in debug data to find kernel data.
        if (strchr(kernelName,'<')) {
            Dwarf_Attribute producerAttr;
            if(dwarf_attr_integrate(&cudie,DW_AT_producer,&producerAttr)) {
                const auto producer = dwarf_formstring (&producerAttr);
                // GFX_EMU_DEBUG_MESSAGE(fDbgSymb, "Debug data producer: %s", producer);
                const auto isGccProducer = strncmp(producer, "GNU", 3) == 0;
                params.funcName =
                    translateTemplatedKernelName(kernelName, isGccProducer);
            }
        } else
            params.funcName = kernelName;

        dwarf_getfuncs (&cudie, dwarfCallback_fillFunctionDesc, &params, 0);
        isFoundFlag |= params.isFound;
    };

    // -> For a list of program names for data lookup.
    ProgramNames_callback_data programNames_callback_data;

    if (programName != "") {
        GFX_EMU_DEBUG_MESSAGE(fDbgSymb | fInfo,
            "lookup for function %s in program module %s\n",
                kernelName, programName.c_str ());
        struct stat buffer;
        if (stat(programName.c_str(), &buffer) == 0) {
            programNames_callback_data.programNames.push_back(programName);
            programNames_callback_data.filter.push_back(programName);
        }
    } else
        programNames_callback_data.programNames.push_back(GfxEmu::Utils::getMainProgramName ());

    dl_iterate_phdr(getAndFilterProgramNames_callback, &programNames_callback_data);
    // <-

    bool isFound = false;
    for(const auto& curProgramName: programNames_callback_data.programNames) {
        GFX_EMU_DEBUG_MESSAGE(fDbgSymb | fDetail,
            "looking in %s\n", curProgramName.c_str ());

        if (isFound) break;

        Dwarf_CU* cu_ptr {NULL};
        Dwarf_Die cudie; //, subdie;

        const auto programFd = open(curProgramName.c_str(), O_RDONLY);
        if (programFd < 0) {
            GFX_EMU_WARNING_MESSAGE("failed to open %s.\n", curProgramName.c_str());
            close(programFd);
            continue;
        }

        dwarfSession = dwarf_begin (programFd, DWARF_C_READ);
        if (dwarf_errno ()) { // NB: dwarf error code enums are not installed, can't differentiate.
            GFX_EMU_WARNING_MESSAGE(fDbgSymb,
                "failed to start dwarf debug session for %s\n",
                curProgramName.c_str ());
            continue;
        }

#ifdef ELFUTILS_HAS_DWARF_GET_UNITS
        while (!isFound &&
                dwarf_get_units (
                    dwarfSession,
                    cu_ptr,
                    &cu_ptr,
                    NULL, NULL, &cudie, NULL // &subdie
                ) == 0) {
#else
        GFX_EMU_DEBUG_MESSAGE(fDbgSymb | fExtraDetail,
            "Currently used libdw (v. %u) has no dwarf_get_units(...) (provided in version >= %u.%u). "
            "Using older CU iteration method.\n",
                _ELFUTILS_VERSION, DWARF_GET_UNITS_REQ_MAJOR, DWARF_GET_UNITS_REQ_MINOR);

        size_t offset = 0, headerSize, prevOffset;
        while (!isFound && dwarf_nextcu(
            dwarfSession, offset, &offset, &headerSize, 0, 0, 0) == 0) {

            if (!dwarf_offdie(dwarfSession, prevOffset + headerSize, &cudie))
                break;

            prevOffset = offset;
#endif
            GFX_EMU_DEBUG_MESSAGE(fDbgSymb | fDetail, "compilation unit die: %p\n", cudie.addr);
            find_kernel_data_(kernelName, cudie, isFound);

            if(isFound && (
                // NB: for non-mangled kernel names we could find it earlier.
                !kernelDesc.addr || kernelDesc.linkageName != kernelDesc.name)
            ) {
                kernelDesc.addr = GfxEmu::Utils::symbolNameToAddr(
                    curProgramName.c_str (),
                    kernelDesc.linkageName,
                    kernelDesc.name);
            }
        }

        dwarf_end(dwarfSession);
        close(programFd);
    }

    if (!isFound) {
        GFX_EMU_WARNING_MESSAGE(fDbgSymb, "can't lookup data for the specified kernel name %s.\n",
            kernelName);
    }

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

GfxEmu::DbgSymb::SymbDesc DbgSymbIface::addrToSymbDesc(void *addr) {
    return impl->addrToSymbDesc(addr);
}

}; // namespace DbgSymb
}; // namespace GfxEmu

#endif // GFX_EMU_DEBUG_SYMBOLS_ACCESS_ENABLED
