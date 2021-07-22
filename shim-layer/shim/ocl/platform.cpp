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

#include <array>
#include <cassert>

#include "platform.h"
#include "runtime.h"

extern "C" {
CL_API_ENTRY cl_int CL_API_CALL
SHIM_CALL(clGetPlatformIDs)(cl_uint          num_entries,
                            cl_platform_id * platforms,
                            cl_uint *        num_platforms) CL_API_SUFFIX__VERSION_1_0 {
  return SHIM_CALL(clIcdGetPlatformIDsKHR)(num_entries, platforms, num_platforms);
}

CL_API_ENTRY cl_int CL_API_CALL
SHIM_CALL(clGetPlatformInfo)(cl_platform_id   platform,
                             cl_platform_info param_name,
                             size_t           param_value_size,
                             void *           param_value,
                             size_t *         param_value_size_ret) CL_API_SUFFIX__VERSION_1_0 {
  using namespace std::literals;
  using shim::cl::SetResult;

  auto &rt = shim::cl::Runtime::Instance();

  if (platform && platform != &rt.platform) {
    return CL_INVALID_PLATFORM;
  }

  switch (param_name) {
    case CL_PLATFORM_PROFILE:
      return SetResult("FULL_PROFILE"sv, param_value_size,
                       param_value, param_value_size_ret);
    case CL_PLATFORM_VERSION:
      return SetResult("OpenCL 3.0 C-for-Metal CPU emulation"sv,
                       param_value_size, param_value, param_value_size_ret);
    case CL_PLATFORM_NUMERIC_VERSION:
      return SetResult(cl_version(300), param_value_size, param_value, param_value_size_ret);
    case CL_PLATFORM_NAME:
      return SetResult("Intel(R) OpenCL C-for-Metal CPU emulation"sv,
                       param_value_size, param_value, param_value_size_ret);
    case CL_PLATFORM_VENDOR:
      return SetResult("Intel(R) Corporation"sv, param_value_size,
                       param_value, param_value_size_ret);
    case CL_PLATFORM_EXTENSIONS:
      return SetResult("cl_khr_icd cl_khr_fp16 cl_khr_fp64"sv, param_value_size,
                       param_value, param_value_size_ret);
    case CL_PLATFORM_EXTENSIONS_WITH_VERSION: {
      static const std::array<cl_name_version, 3> extensions = {{

        { 300, "cl_khr_icd" },
        { 300, "cl_khr_fp16" },
        { 300, "cl_khr_fp64" },
      }};

      return SetResult(extensions, param_value_size, param_value, param_value_size_ret);
    }
    case CL_PLATFORM_HOST_TIMER_RESOLUTION:
      return SetResult(cl_ulong(0), param_value_size, param_value, param_value_size_ret);
    case CL_PLATFORM_ICD_SUFFIX_KHR:
      return SetResult("CMEMU"sv, param_value_size,
                       param_value, param_value_size_ret);
  }

  return CL_INVALID_VALUE;
}

CL_API_ENTRY cl_int CL_API_CALL
SHIM_CALL(clIcdGetPlatformIDsKHR)(cl_uint          num_entries,
                                  cl_platform_id * platforms,
                                  cl_uint *        num_platforms) {
  auto &rt = shim::cl::Runtime::Instance();

  if ((platforms && num_entries < 1) || (!platforms && !num_platforms)){
    return CL_INVALID_VALUE;
  }

  if (platforms) {
    *platforms = &rt.platform;
  }
  if (num_platforms) {
    *num_platforms = 1;
  }

  return CL_SUCCESS;
}

SHIM_EXPORT(clGetPlatformIDs);
SHIM_EXPORT(clGetPlatformInfo);
SHIM_EXPORT(clIcdGetPlatformIDsKHR);
}
