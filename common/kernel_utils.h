/*========================== begin_copyright_notice ============================

Copyright (C) 2020 Intel Corporation

SPDX-License-Identifier: MIT

============================= end_copyright_notice ===========================*/

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
    ~ProgramManager() {
        for (auto p : m_programs)
        {
            FreeProgramInternal(p);
        }
    }

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
