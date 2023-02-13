/*========================== begin_copyright_notice ============================

Copyright (C) 2021 Intel Corporation

SPDX-License-Identifier: MIT

============================= end_copyright_notice ===========================*/

#include <array>
#include <cassert>
#include <numeric>

#include "emu_cfg.h"

#include "platform.h"
#include "runtime.h"

shim::cl::Platform::Platform(cl_icd_dispatch *dispatch_) {
  dispatch = dispatch_;

  const auto platform = GfxEmu::Cfg::Platform().getInt<GfxEmu::Platform::Id>();
  const auto sku = GfxEmu::Cfg::Sku().getInt<GfxEmu::Platform::Sku::Id>();
  const auto &cfg = GfxEmu::Cfg::getPlatformConfig(platform);

  extensions.push_back(cl_name_version{CL_MAKE_VERSION_KHR(1, 0, 0),
                                       "cl_khr_extended_versioning"});
  extensions.push_back(
      cl_name_version{CL_MAKE_VERSION_KHR(1, 0, 0), "cl_khr_fp16"});

  if (cfg.flags & GfxEmu::Cfg::PlatformFlags::Fp64)
    extensions.push_back(
        cl_name_version{CL_MAKE_VERSION_KHR(1, 0, 0), "cl_khr_fp64"});

  extensions.push_back(
      cl_name_version{CL_MAKE_VERSION_KHR(1, 0, 0), "cl_khr_icd"});
  extensions.push_back(cl_name_version{CL_MAKE_VERSION_KHR(3, 0, 0),
                                       "cl_intel_required_subgroup_size"});

  extensions_str = std::accumulate(
      std::begin(extensions), std::end(extensions), std::string{},
      [](const auto &acc, const auto &ext) { return acc + ' ' + ext.name; });

  for (auto simd = cfg.hwSimd; simd <= 32; simd *= 2) {
    subgroup_sizes.push_back(simd);
  }
}

extern "C" {
CL_API_ENTRY cl_int CL_API_CALL
SHIM_CALL(clGetPlatformIDs)(cl_uint num_entries, cl_platform_id *platforms,
                            cl_uint *num_platforms) CL_API_SUFFIX__VERSION_1_0 {
  return SHIM_CALL(clIcdGetPlatformIDsKHR)(num_entries, platforms,
                                           num_platforms);
}

CL_API_ENTRY cl_int CL_API_CALL SHIM_CALL(clGetPlatformInfo)(
    cl_platform_id platform, cl_platform_info param_name,
    size_t param_value_size, void *param_value,
    size_t *param_value_size_ret) CL_API_SUFFIX__VERSION_1_0 {
  using namespace std::literals;
  using shim::cl::SetResult;

  auto &rt = shim::cl::Runtime::Instance();

  if (platform && platform != &rt.platform) {
    return CL_INVALID_PLATFORM;
  }

  switch (param_name) {
  case CL_PLATFORM_PROFILE:
    return SetResult("FULL_PROFILE"sv, param_value_size, param_value,
                     param_value_size_ret);
  case CL_PLATFORM_VERSION:
    return SetResult("OpenCL 3.0 C-for-Metal CPU emulation"sv, param_value_size,
                     param_value, param_value_size_ret);
  case CL_PLATFORM_NUMERIC_VERSION:
    return SetResult(CL_MAKE_VERSION_KHR(3, 0, 0), param_value_size,
                     param_value, param_value_size_ret);
  case CL_PLATFORM_NAME:
    return SetResult("Intel(R) OpenCL C-for-Metal CPU emulation"sv,
                     param_value_size, param_value, param_value_size_ret);
  case CL_PLATFORM_VENDOR:
    return SetResult("Intel(R) Corporation"sv, param_value_size, param_value,
                     param_value_size_ret);
  case CL_PLATFORM_EXTENSIONS:
    return SetResult(std::string_view(rt.platform.extensions_str),
                     param_value_size, param_value, param_value_size_ret);
  case CL_PLATFORM_EXTENSIONS_WITH_VERSION: {
    return SetResult<cl_name_version>(rt.platform.extensions, param_value_size,
                                      param_value, param_value_size_ret);
  }
  case CL_PLATFORM_HOST_TIMER_RESOLUTION:
    return SetResult(cl_ulong(0), param_value_size, param_value,
                     param_value_size_ret);
  case CL_PLATFORM_ICD_SUFFIX_KHR:
    return SetResult("CMEMU"sv, param_value_size, param_value,
                     param_value_size_ret);
  }

  return CL_INVALID_VALUE;
}

CL_API_ENTRY cl_int CL_API_CALL SHIM_CALL(clIcdGetPlatformIDsKHR)(
    cl_uint num_entries, cl_platform_id *platforms, cl_uint *num_platforms) {
  auto &rt = shim::cl::Runtime::Instance();

  if ((platforms && num_entries < 1) || (!platforms && !num_platforms)) {
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
