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

#ifndef CM_EMU_SHIM_OCL_DEVICE_H
#define CM_EMU_SHIM_OCL_DEVICE_H

#include <memory>
#include <unordered_set>

#include "ocl.h"

extern "C" {
struct _cl_device_id {
  cl_icd_dispatch *dispatch;
};

CL_API_ENTRY cl_int CL_API_CALL
SHIM_CALL(clGetDeviceIDs)(cl_platform_id   platform,
                          cl_device_type   device_type,
                          cl_uint          num_entries,
                          cl_device_id *   devices,
                          cl_uint *        num_devices) CL_API_SUFFIX__VERSION_1_0;

CL_API_ENTRY cl_int CL_API_CALL
SHIM_CALL(clGetDeviceInfo)(cl_device_id    device,
                           cl_device_info  param_name,
                           size_t          param_value_size,
                           void *          param_value,
                           size_t *        param_value_size_ret) CL_API_SUFFIX__VERSION_1_0;

CL_API_ENTRY cl_int CL_API_CALL
SHIM_CALL(clRetainDevice)(cl_device_id device) CL_API_SUFFIX__VERSION_1_2;

CL_API_ENTRY cl_int CL_API_CALL
SHIM_CALL(clReleaseDevice)(cl_device_id device) CL_API_SUFFIX__VERSION_1_2;
}

namespace shim {
namespace cl {

struct Device : public _cl_device_id {
  Device(cl_icd_dispatch *dispatch);

  int Id() const;
  std::string_view Name() const;
  std::string CompilerCommand() const;
  std::string LinkerCommand() const;

  std::shared_ptr<CmDevice> device;
  std::unordered_set<cl_mem> buffers_;
};

} // namespace cl
} // namespace shim

#endif // CM_EMU_SHIM_OCL_DEVICE_H
