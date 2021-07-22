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

#include <vector>
#include <string>
#include <unordered_set>
#include <unordered_map>

#include "os_utils.h"

#include "emu_api_export.h"
#include "emu_utils.h"

enum class CmArgumentType
{
    SurfaceIndex,
    Fp,
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

    GFX_EMU_API bool isValid();
};

class ProgramManager
{
public:
    GFX_EMU_API ProgramInfo AddProgram(const unsigned char* const bytes, size_t size);
    GFX_EMU_API bool IsProgramValid(ProgramHandle program);
    GFX_EMU_API bool FreeProgram(ProgramHandle program);

    GFX_EMU_API static ProgramManager& instance();

private:
    GFX_EMU_API bool FreeProgramInternal(ProgramHandle program);
    ProgramManager();
    void Complete();
    ~ProgramManager() = default;

private:
    std::unordered_set<ProgramHandle> m_programs;

public:
    ProgramManager(const ProgramManager&) = delete;
    ProgramManager(ProgramManager&&) = delete;
    ProgramManager& operator=(const ProgramManager&) = delete;
};

static auto& g_shimProgramManagerInit_ = ProgramManager::instance();

GFX_EMU_API extern CmArgumentType CmArgumentTypeFromString(const std::string& s);
GFX_EMU_API extern Kernel2SigMap EnumerateKernels(os::SharedLibHandle dll);
GFX_EMU_API extern CmArgTypeVector ParseKernelSignature(const std::string &signature);
