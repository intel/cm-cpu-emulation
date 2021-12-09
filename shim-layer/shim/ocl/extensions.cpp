/*========================== begin_copyright_notice ============================

Copyright (C) 2021 Intel Corporation

SPDX-License-Identifier: MIT

============================= end_copyright_notice ===========================*/

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
