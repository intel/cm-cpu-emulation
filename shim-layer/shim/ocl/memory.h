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

#ifndef CM_EMU_SHIM_OCL_MEMORY_H
#define CM_EMU_SHIM_OCL_MEMORY_H

#include <variant>

#include "context.h"
#include "intrusive_pointer.h"
#include "utils.h"

extern "C" {
struct _cl_mem {
  cl_icd_dispatch *dispatch;
};

CL_API_ENTRY cl_mem CL_API_CALL
SHIM_CALL(clCreateBuffer)(cl_context   context,
                          cl_mem_flags flags,
                          size_t       size,
                          void *       host_ptr,
                          cl_int *     errcode_ret) CL_API_SUFFIX__VERSION_1_0;

CL_API_ENTRY cl_mem CL_API_CALL
SHIM_CALL(clCreateSubBuffer)(cl_mem                   buffer,
                             cl_mem_flags             flags,
                             cl_buffer_create_type    buffer_create_type,
                             const void *             buffer_create_info,
                             cl_int *                 errcode_ret) CL_API_SUFFIX__VERSION_1_1;

CL_API_ENTRY cl_mem CL_API_CALL
SHIM_CALL(clCreateImage)(cl_context              context,
                         cl_mem_flags            flags,
                         const cl_image_format * image_format,
                         const cl_image_desc *   image_desc,
                         void *                  host_ptr,
                         cl_int *                errcode_ret) CL_API_SUFFIX__VERSION_1_2;

CL_API_ENTRY cl_mem CL_API_CALL
SHIM_CALL(clCreateBufferWithProperties)(cl_context                context,
                                        const cl_mem_properties * properties,
                                        cl_mem_flags              flags,
                                        size_t                    size,
                                        void *                    host_ptr,
                                        cl_int *                  errcode_ret) CL_API_SUFFIX__VERSION_3_0;

CL_API_ENTRY cl_mem CL_API_CALL
SHIM_CALL(clCreateImageWithProperties)(cl_context                context,
                                       const cl_mem_properties * properties,
                                       cl_mem_flags              flags,
                                       const cl_image_format *   image_format,
                                       const cl_image_desc *     image_desc,
                                       void *                    host_ptr,
                                       cl_int *                  errcode_ret) CL_API_SUFFIX__VERSION_3_0;

CL_API_ENTRY cl_int CL_API_CALL
SHIM_CALL(clRetainMemObject)(cl_mem memobj) CL_API_SUFFIX__VERSION_1_0;

CL_API_ENTRY cl_int CL_API_CALL
SHIM_CALL(clReleaseMemObject)(cl_mem memobj) CL_API_SUFFIX__VERSION_1_0;

CL_API_ENTRY cl_int CL_API_CALL
SHIM_CALL(clGetImageInfo)(cl_mem           image,
                          cl_image_info    param_name,
                          size_t           param_value_size,
                          void *           param_value,
                          size_t *         param_value_size_ret) CL_API_SUFFIX__VERSION_1_0;

CL_API_ENTRY void * CL_API_CALL
SHIM_CALL(clSVMAlloc)(cl_context       context,
                      cl_svm_mem_flags flags,
                      size_t           size,
                      cl_uint          alignment) CL_API_SUFFIX__VERSION_2_0;

CL_API_ENTRY void CL_API_CALL
SHIM_CALL(clSVMFree)(cl_context        context,
                     void *            svm_pointer) CL_API_SUFFIX__VERSION_2_0;
}

namespace shim {
namespace cl {

struct Memory : public _cl_mem, public IntrusiveRefCounter<Memory> {
  std::variant<CmBuffer*, CmSurface2D*, CmSurface3D*> buffer_;
  IntrusivePtr<Context> ctx_;

  template <typename T>
  Memory(IntrusivePtr<Context> ctx, T *buf) : ctx_(ctx), buffer_(buf) {
    dispatch = ctx_->dispatch;
  }

  ~Memory() {
    std::visit([this](auto *p) { ctx_->dev_.device->DestroySurface(p); }, buffer_);
  }
};

} // namespace cl
} // namespace shim

#endif // CM_EMU_SHIM_OCL_MEMORY_H
