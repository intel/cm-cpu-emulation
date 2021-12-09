/*========================== begin_copyright_notice ============================

Copyright (C) 2017 Intel Corporation

SPDX-License-Identifier: MIT

============================= end_copyright_notice ===========================*/

#ifndef CM_EMU_SHIM_OCL_PLATFORM_H
#define CM_EMU_SHIM_OCL_PLATFORM_H

#include "ocl.h"

extern "C" {
struct _cl_platform_id {
  cl_icd_dispatch *dispatch;
};

CL_API_ENTRY cl_int CL_API_CALL
SHIM_CALL(clGetPlatformIDs)(cl_uint          num_entries,
                            cl_platform_id * platforms,
                            cl_uint *        num_platforms) CL_API_SUFFIX__VERSION_1_0;

CL_API_ENTRY cl_int CL_API_CALL
SHIM_CALL(clGetPlatformInfo)(cl_platform_id   platform,
                             cl_platform_info param_name,
                             size_t           param_value_size,
                             void *           param_value,
                             size_t *         param_value_size_ret) CL_API_SUFFIX__VERSION_1_0;

CL_API_ENTRY cl_int CL_API_CALL
SHIM_CALL(clIcdGetPlatformIDsKHR)(cl_uint          num_entries,
                                  cl_platform_id * platforms,
                                  cl_uint *        num_platforms);
}

namespace shim {
namespace cl {

struct Platform : public _cl_platform_id {
  Platform(cl_icd_dispatch *dispatch) {
    this->dispatch = dispatch;
  }
};

} // namespace cl
} // namespace shim

#endif // CM_EMU_SHIM_OCL_PLATFORM_H
