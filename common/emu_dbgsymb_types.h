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
    size_t size {0};
    bool isType() { return !size; }
};

using ArgDesc = SymbDesc;

struct FunctionDesc : SymbDesc {
    std::vector<ArgDesc> args;
    std::string linkageName;

    FunctionDesc () = default;
    FunctionDesc (const FunctionDesc& o) : SymbDesc (o) {
        std::copy(o.args.begin(), o.args.end(), std::back_inserter(args));
        linkageName = o.linkageName;

    }
};

struct ArgVisitorParams {
    DbgSymb::FunctionDesc *kernelDescPtr {nullptr};
    std::string funcName {};
    void* funcAddr {nullptr};
    bool isFound {false};
    bool setNameOnly {false};
    int curParamIdx {0};
};

}; // namespace DbgSymb
}; // namespace GfxEmu
