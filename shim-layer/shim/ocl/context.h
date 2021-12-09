
/*========================== begin_copyright_notice ============================

Copyright (C) 2021 Intel Corporation

SPDX-License-Identifier: MIT

============================= end_copyright_notice ===========================*/

#ifndef CM_EMU_SHIM_OCL_CONTEXT_H
#define CM_EMU_SHIM_OCL_CONTEXT_H

#include "device.h"
#include "intrusive_pointer.h"

extern "C" {
struct _cl_context {
  cl_icd_dispatch *dispatch;
};

extern CL_API_ENTRY cl_context CL_API_CALL
SHIM_CALL(clCreateContext)(const cl_context_properties * properties,
                           cl_uint              num_devices,
                           const cl_device_id * devices,
                           void (CL_CALLBACK * pfn_notify)(const char * errinfo,
                                                           const void * private_info,
                                                           size_t       cb,
                                                           void *       user_data),
                           void *               user_data,
                           cl_int *             errcode_ret) CL_API_SUFFIX__VERSION_1_0;

extern CL_API_ENTRY cl_context CL_API_CALL
SHIM_CALL(clCreateContextFromType)(const cl_context_properties * properties,
                                   cl_device_type      device_type,
                                   void (CL_CALLBACK * pfn_notify)(const char * errinfo,
                                                                   const void * private_info,
                                                                   size_t       cb,
                                                                   void *       user_data),
                                   void *              user_data,
                                   cl_int *            errcode_ret) CL_API_SUFFIX__VERSION_1_0;

extern CL_API_ENTRY cl_int CL_API_CALL
SHIM_CALL(clRetainContext)(cl_context context) CL_API_SUFFIX__VERSION_1_0;

extern CL_API_ENTRY cl_int CL_API_CALL
SHIM_CALL(clReleaseContext)(cl_context context) CL_API_SUFFIX__VERSION_1_0;

extern CL_API_ENTRY cl_int CL_API_CALL
SHIM_CALL(clGetContextInfo)(cl_context         context,
                            cl_context_info    param_name,
                            size_t             param_value_size,
                            void *             param_value,
                            size_t *           param_value_size_ret) CL_API_SUFFIX__VERSION_1_0;

extern CL_API_ENTRY cl_int CL_API_CALL
SHIM_CALL(clSetContextDestructorCallback)(cl_context         context,
                                          void (CL_CALLBACK* pfn_notify)(cl_context context,
                                                                         void* user_data),
                                          void*              user_data) CL_API_SUFFIX__VERSION_3_0;

}

namespace shim {
namespace cl {

struct Context : public _cl_context, public IntrusiveRefCounter<Context> {
  Context(Device &dev) : dev_(dev), pfn_notify(nullptr), user_data(nullptr) {
    this->dispatch = dev_.dispatch;
  }

  ~Context() {
    if (pfn_notify) {
      pfn_notify(this, user_data);
    }
  }

  Device &dev_;

  void (CL_CALLBACK* pfn_notify)(cl_context context, void* user_data);
  void *user_data;
};

} // namespace cl
} // namespace shim

#endif // CM_EMU_SHIM_OCL_CONTEXT_H
