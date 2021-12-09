/*========================== begin_copyright_notice ============================

Copyright (C) 2020 Intel Corporation

SPDX-License-Identifier: MIT

============================= end_copyright_notice ===========================*/

#define NOMINMAX

#include <iterator>
#include <regex>
#include <limits>

#include "kernel_utils.h"
#include "shim_support.h"

#include "emu_utils.h"

using namespace std::string_literals;

// compiler-dependent functionality
namespace compiler
{
#if defined(_WIN32)
    // visual studio
    constexpr char KERNEL_SIG_REGEX[] = "^[^<]+<[^(]+\\(([^)]+)\\).*$";
    constexpr char SURFACE_INDEX_TYPE[] = "class SurfaceIndex";
    constexpr char FLOAT_TYPE[] = "float";
    constexpr char DOUBLE_TYPE[] = "double";
    constexpr char LONG_DOUBLE_TYPE[] = "long double";
#else /* _WIN32 */
    // clang and GCC
    constexpr char KERNEL_SIG_REGEX[] = "^[^=]+=[^(]+\\(([^)]+)\\).*$";
    constexpr char SURFACE_INDEX_TYPE[] = "SurfaceIndex";
    constexpr char FLOAT_TYPE[] = "float";
    constexpr char DOUBLE_TYPE[] = "double";
    constexpr char LONG_DOUBLE_TYPE[] = "long double";
#endif /* _WIN32 */
}

constexpr int MAX_KERNELS_PER_PROGRAM = std::numeric_limits<int>::max();

CmArgumentType CmArgumentTypeFromString(const std::string& s)
{
    static const std::unordered_map<std::string, CmArgumentType> mapping = {
        {compiler::SURFACE_INDEX_TYPE, CmArgumentType::SurfaceIndex},
        {compiler::FLOAT_TYPE, CmArgumentType::Fp},
        {compiler::DOUBLE_TYPE, CmArgumentType::Fp},
        {compiler::LONG_DOUBLE_TYPE, CmArgumentType::Fp},
    };

    auto t = mapping.find(s);
    if (t != mapping.end())
    {
        return t->second;
    }
    else
    {
        return CmArgumentType::Scalar;
    }
}

Kernel2SigMap EnumerateKernels(os::SharedLibHandle dll)
{
    static const std::string KERNEL_BASENAME = "__kernel_"s;

    Kernel2SigMap result;
    for (int i = 0; i < MAX_KERNELS_PER_PROGRAM; ++i)
    {
        std::string probed_name = KERNEL_BASENAME + std::to_string(i);
        shim::KernelDescriptor *desc = reinterpret_cast<shim::KernelDescriptor*>(os::GetSharedSymbolAddress(dll, probed_name.c_str()));
        if (desc)
        {
            KernelInfo info = {desc->signature, desc->func};
            result.emplace(desc->name, info);
        }
        else
            break;
    }
    return result;
}

CmArgTypeVector ParseKernelSignature(const std::string& signature)
{
    std::regex params_regex(compiler::KERNEL_SIG_REGEX);
    std::smatch match;
    CmArgTypeVector result;
    if (std::regex_match(signature, match, params_regex))
    {
        // comma separated list of parameter types
        std::string params = match[1].str();
        std::regex separator_regex("\\s*,\\s*");
        std::for_each(std::sregex_token_iterator(params.begin(), params.end(), separator_regex, -1),
            std::sregex_token_iterator(), [&result](auto& token)
        {
            CmArgumentType t = CmArgumentTypeFromString(token.str());
            result.push_back(t);
        });
    }
    return result;
}

bool ProgramInfo::isValid()
{
    return os::IsLibHandleValid(handle);
}

ProgramInfo::KernelData::KernelData(void* ep, CmArgTypeVector&& arguments):
    entry_point(ep),
    args(arguments)
{
}

ProgramInfo::KernelData::KernelData(void* ep, const CmArgTypeVector& arguments):
    entry_point(ep),
    args(arguments)
{
}

ProgramManager& ProgramManager::instance()
{
    static ProgramManager inst;
    return inst;
}

bool ProgramManager::IsProgramValid(ProgramHandle program)
{
    return m_programs.find(program) != m_programs.end();
}

ProgramInfo ProgramManager::AddProgram(const unsigned char* const bytes, size_t size)
{
    std::string libname = os::CreateTempFile(bytes, size);
    if (libname.empty())
    {
        return ProgramInfo();
    }
    os::SharedLibHandle h = os::LoadSharedLib(libname);
    if (!os::IsLibHandleValid(h))
    {
        os::DeleteFile(libname);
        return ProgramInfo();
    }
    m_programs.insert(h);

    ProgramInfo pd;
    pd.handle = h;

    auto kernels2sigs = EnumerateKernels(h);
    for (auto& [name, info]: kernels2sigs)
    {
        CmArgTypeVector args = ParseKernelSignature(info.signature);
        pd.kernels.try_emplace(name, info.func, std::move(args));
    }

    return pd;
}

bool ProgramManager::FreeProgram(ProgramHandle program)
{
    bool success = FreeProgramInternal(program);
    size_t num_erased = m_programs.erase(program);
    return success && (num_erased != 0);
}

bool ProgramManager::FreeProgramInternal(ProgramHandle program)
{
    auto it = m_programs.find(program);
    if (it == m_programs.end())
    {
        return false;
    }
    std::string libname = os::GetSharedLibLocation(program);
    os::FreeSharedLib(program);
    os::DeleteFile(libname);
    return true;
}

ProgramManager::ProgramManager()
{
}
