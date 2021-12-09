/*========================== begin_copyright_notice ============================

Copyright (C) 2017 Intel Corporation

SPDX-License-Identifier: MIT

============================= end_copyright_notice ===========================*/

#pragma once

#include <string>
#include <vector>
#include <algorithm>
#include <iterator>

namespace GfxEmu {
namespace DbgSymb {
struct SymbDesc {
    std::string name {};
    std::string typeName {};
    void* addr {nullptr};
    bool isFloat {false};
    bool isClass {false};
    bool isPointer {false};
    bool isVectorOrMatrix {false};
    std::string paramInitFunctionLinkageName;
    void *paramInitFunctionAddr {nullptr};
    size_t size {0};
    bool isType() { return !size; }
};

using ParamDesc = SymbDesc;

struct FunctionDesc : SymbDesc {
    std::vector<ParamDesc> params;
    std::string linkageName;
    FunctionDesc () = default;
    FunctionDesc (const FunctionDesc& o) : SymbDesc (o) {
        std::copy(o.params.begin(), o.params.end(), std::back_inserter(params));
        linkageName = o.linkageName;
    }
};

struct ArgVisitorParams {
    DbgSymb::FunctionDesc *kernelDescPtr {nullptr};
    std::string funcName;
    void *funcAddr {nullptr};
    std::string moduleFileName;
    void *moduleBaseAddr {nullptr};
    bool isFound {false};
    int curParamIdx {0};
};

}; // namespace DbgSymb
}; // namespace GfxEmu
