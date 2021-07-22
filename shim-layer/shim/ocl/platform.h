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
