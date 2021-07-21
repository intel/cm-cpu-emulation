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

#include "runtime.h"

extern "C" {

CL_API_ENTRY CL_API_PREFIX__VERSION_1_1_DEPRECATED void * CL_API_CALL
SHIM_CALL(clGetExtensionFunctionAddress)(const char * func_name) {
  using namespace std::literals;

  if (!func_name) {
    return nullptr;
  }

  // cl_icd_khr
  if (func_name == "clIcdGetPlatformIDsKHR"sv) {
    return reinterpret_cast<void*>(SHIM_CALL(clIcdGetPlatformIDsKHR));
  }
  if (func_name == "clGetPlatformInfo"sv) {
    return reinterpret_cast<void*>(SHIM_CALL(clGetPlatformInfo));
  }

  // Non-standard CM EMU extensions
  // if (func_name == "SetDefaultResidentGroupAndParallelThreadNum"sv) {
  //   return reinterpret_cast<void*>(&SetDefaultResidentGroupAndParallelThreadNum);
  // }
  // if (func_name == "SetResidentGroupAndParallelThreadNumForQueue"sv) {
  //   return reinterpret_cast<void*>(&SetResidentGroupAndParallelThreadNumForQueue);
  // }

  return nullptr;
}

CL_API_ENTRY void * CL_API_CALL
SHIM_CALL(clGetExtensionFunctionAddressForPlatform)(cl_platform_id platform,
                                                    const char *   func_name) CL_API_SUFFIX__VERSION_1_2 {
  auto &rt = shim::cl::Runtime::Instance();

  if (platform != &rt.platform) {
    return nullptr;
  }

  return SHIM_CALL(clGetExtensionFunctionAddress)(func_name);
}

SHIM_EXPORT(clGetExtensionFunctionAddressForPlatform);
SHIM_EXPORT(clGetExtensionFunctionAddress);
}
