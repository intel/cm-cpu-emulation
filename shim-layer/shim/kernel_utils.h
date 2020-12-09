/*===================== begin_copyright_notice ==================================

 Copyright (c) 2020, Intel Corporation


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

#include <vector>
#include <string>
#include <unordered_set>
#include <unordered_map>

#include "os_utils.h"

enum class CmArgumentType
{
    SurfaceIndex,
    Scalar,
    Invalid
};

struct KernelInfo
{
    std::string signature;
    void *func;
};

using Kernel2SigMap = std::unordered_map<std::string, KernelInfo>;
using CmArgTypeVector = std::vector<CmArgumentType>;
using ProgramHandle = os::SharedLibHandle;

struct ProgramInfo
{
    struct KernelData
    {
        CmArgTypeVector args;
        void* entry_point;

        KernelData(void* ep, const CmArgTypeVector& arguments);
        KernelData(void* ep, CmArgTypeVector&& arguments);

        KernelData(const KernelData&) = delete;
        KernelData(KernelData&&) = delete;
        KernelData() = delete;
    };
    using KernelMap = std::unordered_map<std::string, KernelData>;

    ProgramHandle handle = ProgramHandle();
    KernelMap kernels;

    bool isValid();
};

class ProgramManager
{
public:
    ProgramInfo AddProgram(const unsigned char* const bytes, size_t size);
    bool IsProgramValid(ProgramHandle program);
    bool FreeProgram(ProgramHandle program);

    static ProgramManager& instance();

private:
    bool FreeProgramInternal(ProgramHandle program);
    ProgramManager();
    ~ProgramManager();

private:
    std::unordered_set<ProgramHandle> m_programs;

public:
    ProgramManager(const ProgramManager&) = delete;
    ProgramManager(ProgramManager&&) = delete;
    ProgramManager& operator=(const ProgramManager&) = delete;
};

extern CmArgumentType CmArgumentTypeFromString(const std::string& s);
extern Kernel2SigMap EnumerateKernels(os::SharedLibHandle dll);
extern CmArgTypeVector ParseKernelSignature(const std::string &signature);
