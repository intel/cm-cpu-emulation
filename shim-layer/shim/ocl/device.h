/*========================== begin_copyright_notice ============================

Copyright (C) 2021 Intel Corporation

SPDX-License-Identifier: MIT

============================= end_copyright_notice ===========================*/

#ifndef CM_EMU_SHIM_OCL_DEVICE_H
#define CM_EMU_SHIM_OCL_DEVICE_H

#include <memory>
#include <unordered_set>

#include "ocl.h"

#include "intrusive_pointer.h"

extern "C" {
struct _cl_device_id {
  cl_icd_dispatch *dispatch;
};

CL_API_ENTRY cl_int CL_API_CALL SHIM_CALL(clGetDeviceIDs)(
    cl_platform_id platform, cl_device_type device_type, cl_uint num_entries,
    cl_device_id *devices, cl_uint *num_devices) CL_API_SUFFIX__VERSION_1_0;

CL_API_ENTRY cl_int CL_API_CALL SHIM_CALL(clGetDeviceInfo)(
    cl_device_id device, cl_device_info param_name, size_t param_value_size,
    void *param_value, size_t *param_value_size_ret) CL_API_SUFFIX__VERSION_1_0;

CL_API_ENTRY cl_int CL_API_CALL SHIM_CALL(clRetainDevice)(cl_device_id device)
    CL_API_SUFFIX__VERSION_1_2;

CL_API_ENTRY cl_int CL_API_CALL SHIM_CALL(clReleaseDevice)(cl_device_id device)
    CL_API_SUFFIX__VERSION_1_2;
}

namespace shim {
namespace cl {

struct Device : public _cl_device_id {
  Device(cl_icd_dispatch *dispatch);

  int Id() const;
  std::string_view Name() const;
  std::string CompilerCommand() const;
  std::string LinkerCommand() const;

  IntrusivePtr<CmDeviceEmu> device;
  std::unordered_set<cl_mem> buffers_;
};

} // namespace cl
} // namespace shim

#endif // CM_EMU_SHIM_OCL_DEVICE_H
