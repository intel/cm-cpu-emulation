/*========================== begin_copyright_notice ============================

Copyright (C) 2019 Intel Corporation

SPDX-License-Identifier: MIT

============================= end_copyright_notice ===========================*/

#pragma once

#include <string>

#if defined(_WIN32)
#include <windows.h>
#endif /* _WIN32 */

// os-dependent functionality
namespace os
{

#if defined(_WIN32)
using SharedLibHandle = HMODULE;
#else /* _WIN32 */
using SharedLibHandle = void*;
#endif /* _WIN32 */

bool IsLibHandleValid(SharedLibHandle h);
SharedLibHandle LoadSharedLib(const std::string& name);
bool FreeSharedLib(SharedLibHandle h);
void *GetSharedSymbolAddress(SharedLibHandle h, const std::string& name);
std::string GetSharedLibLocation(SharedLibHandle h);
/// Create a temporary file and write "bytes" to it. Return filename.
std::string CreateTempFile(const unsigned char* const bytes, size_t num_bytes);
bool DeleteFile(const std::string& name);
std::string GetEnvVarValue(const std::string& name);
}
