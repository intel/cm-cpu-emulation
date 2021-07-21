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

// os-dependent functionality
namespace os
{

using SharedLibHandle = void*;

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
