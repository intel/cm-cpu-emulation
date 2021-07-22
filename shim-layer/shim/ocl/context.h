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
