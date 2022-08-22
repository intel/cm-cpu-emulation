
/*========================== begin_copyright_notice ============================

Copyright (C) 2021 Intel Corporation

SPDX-License-Identifier: MIT

============================= end_copyright_notice ===========================*/

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

CL_API_ENTRY cl_mem CL_API_CALL SHIM_CALL(clCreateBuffer)(
    cl_context context, cl_mem_flags flags, size_t size, void *host_ptr,
    cl_int *errcode_ret) CL_API_SUFFIX__VERSION_1_0;

CL_API_ENTRY cl_mem CL_API_CALL SHIM_CALL(clCreateSubBuffer)(
    cl_mem buffer, cl_mem_flags flags, cl_buffer_create_type buffer_create_type,
    const void *buffer_create_info,
    cl_int *errcode_ret) CL_API_SUFFIX__VERSION_1_1;

CL_API_ENTRY cl_mem CL_API_CALL SHIM_CALL(clCreateImage)(
    cl_context context, cl_mem_flags flags, const cl_image_format *image_format,
    const cl_image_desc *image_desc, void *host_ptr,
    cl_int *errcode_ret) CL_API_SUFFIX__VERSION_1_2;

CL_API_ENTRY cl_mem CL_API_CALL SHIM_CALL(clCreateBufferWithProperties)(
    cl_context context, const cl_mem_properties *properties, cl_mem_flags flags,
    size_t size, void *host_ptr,
    cl_int *errcode_ret) CL_API_SUFFIX__VERSION_3_0;

CL_API_ENTRY cl_mem CL_API_CALL SHIM_CALL(clCreateImageWithProperties)(
    cl_context context, const cl_mem_properties *properties, cl_mem_flags flags,
    const cl_image_format *image_format, const cl_image_desc *image_desc,
    void *host_ptr, cl_int *errcode_ret) CL_API_SUFFIX__VERSION_3_0;

CL_API_ENTRY cl_int CL_API_CALL SHIM_CALL(clRetainMemObject)(cl_mem memobj)
    CL_API_SUFFIX__VERSION_1_0;

CL_API_ENTRY cl_int CL_API_CALL SHIM_CALL(clReleaseMemObject)(cl_mem memobj)
    CL_API_SUFFIX__VERSION_1_0;

CL_API_ENTRY cl_int CL_API_CALL SHIM_CALL(clGetImageInfo)(
    cl_mem image, cl_image_info param_name, size_t param_value_size,
    void *param_value, size_t *param_value_size_ret) CL_API_SUFFIX__VERSION_1_0;

CL_API_ENTRY void *CL_API_CALL SHIM_CALL(clSVMAlloc)(
    cl_context context, cl_svm_mem_flags flags, size_t size,
    cl_uint alignment) CL_API_SUFFIX__VERSION_2_0;

CL_API_ENTRY void CL_API_CALL SHIM_CALL(clSVMFree)(
    cl_context context, void *svm_pointer) CL_API_SUFFIX__VERSION_2_0;
}

namespace shim {
namespace cl {

struct Memory : public _cl_mem, public IntrusiveRefCounter<Memory> {
  std::variant<void *, CmSurface2D *, CmSurface3D *> buffer_;
  IntrusivePtr<Context> ctx_;

  template <typename T>
  Memory(IntrusivePtr<Context> ctx, T *buf) : ctx_(ctx), buffer_(buf) {
    dispatch = ctx_->dispatch;
  }

  ~Memory() {
    std::visit(
        overloaded{
            [this](CmSurface2D *p) { ctx_->dev_.device->DestroySurface(p); },
            [this](CmSurface3D *p) { ctx_->dev_.device->DestroySurface(p); },
            [this](void *p) { ctx_->mm_.Free(p); }},
        buffer_);
  }
};

} // namespace cl
} // namespace shim

#endif // CM_EMU_SHIM_OCL_MEMORY_H
