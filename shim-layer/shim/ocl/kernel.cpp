/*========================== begin_copyright_notice ============================

Copyright (C) 2021 Intel Corporation

SPDX-License-Identifier: MIT

============================= end_copyright_notice ===========================*/

#include <cassert>

#include "kernel.h"
#include "memory.h"

#include "emu_kernel_support.h"
#include "kernel_utils.h"

#include "cm_program_emumode.h"

CL_API_ENTRY cl_kernel CL_API_CALL
SHIM_CALL(clCreateKernel)(cl_program      program,
                          const char *    kernel_name,
                          cl_int *        errcode_ret) CL_API_SUFFIX__VERSION_1_0 {
  shim::IntrusivePtr<shim::cl::Program> prog(static_cast<shim::cl::Program*>(program));

  if (!prog->prog_) {
    ERRCODE(CL_INVALID_PROGRAM_EXECUTABLE);
    return nullptr;
  }

  if (!kernel_name) {
    ERRCODE(CL_INVALID_VALUE);
    return nullptr;
  }

  CmKernel *k = nullptr;
  if (prog->ctx_->dev_.device->CreateKernel(prog->prog_.get(), kernel_name, k) != CM_SUCCESS) {
    ERRCODE(CL_INVALID_KERNEL_NAME);
    return nullptr;
  }

  shim::IntrusivePtr<shim::cl::Kernel> kernel = new(std::nothrow) shim::cl::Kernel(prog, k);

  if (!kernel) {
    ERRCODE(CL_OUT_OF_HOST_MEMORY);
    return nullptr;
  }

  ERRCODE(CL_SUCCESS);

  shim::IntrusivePtrAddRef(kernel.get());
  return kernel.get();
}

CL_API_ENTRY cl_int CL_API_CALL
SHIM_CALL(clCreateKernelsInProgram)(cl_program     program,
                                    cl_uint        num_kernels,
                                    cl_kernel *    kernels,
                                    cl_uint *      num_kernels_ret) CL_API_SUFFIX__VERSION_1_0 {
  return CL_OUT_OF_RESOURCES;
}

CL_API_ENTRY cl_kernel CL_API_CALL
SHIM_CALL(clCloneKernel)(cl_kernel     source_kernel,
                         cl_int*       errcode_ret) CL_API_SUFFIX__VERSION_2_1 {
  ERRCODE(CL_OUT_OF_RESOURCES);
  return nullptr;
}

CL_API_ENTRY cl_int CL_API_CALL
SHIM_CALL(clRetainKernel)(cl_kernel    kernel) CL_API_SUFFIX__VERSION_1_0 {
  shim::IntrusivePtrAddRef(static_cast<shim::cl::Kernel*>(kernel));
  return CL_SUCCESS;
}

CL_API_ENTRY cl_int CL_API_CALL
SHIM_CALL(clReleaseKernel)(cl_kernel   kernel) CL_API_SUFFIX__VERSION_1_0 {
  shim::IntrusivePtr<shim::cl::Kernel> k(static_cast<shim::cl::Kernel*>(kernel), false);
  return CL_SUCCESS;
}

CL_API_ENTRY cl_int CL_API_CALL
SHIM_CALL(clSetKernelArg)(cl_kernel    kernel,
                          cl_uint      arg_index,
                          size_t       arg_size,
                          const void * arg_value) CL_API_SUFFIX__VERSION_1_0 {
  shim::IntrusivePtr<shim::cl::Kernel> kern(static_cast<shim::cl::Kernel*>(kernel));

  auto &buffers_ = kern->prog_->ctx_->dev_.buffers_;

  if (cl_mem m = *static_cast<const cl_mem*>(arg_value); buffers_.find(m) != buffers_.end()) {
    shim::IntrusivePtr<shim::cl::Memory> buf(static_cast<shim::cl::Memory*>(m));
    return std::visit([kern, arg_index](auto *buf) {
                        SurfaceIndex *idx = nullptr;
                        if (buf->GetIndex(idx) != CM_SUCCESS) {
                          return CL_INVALID_ARG_VALUE;
                        }

                        if (kern->kern_->SetKernelArg(arg_index, sizeof(SurfaceIndex), idx) != CM_SUCCESS) {
                          return CL_INVALID_ARG_VALUE;
                        }

                        return CL_SUCCESS;
                      }, buf->buffer_);
  } else if (kern->kern_->SetKernelArg(arg_index, arg_size, arg_value) != CM_SUCCESS) {
    return CL_INVALID_ARG_VALUE;
  }

  return CL_SUCCESS;
}

CL_API_ENTRY cl_int CL_API_CALL
SHIM_CALL(clSetKernelArgSVMPointer)(cl_kernel    kernel,
                                    cl_uint      arg_index,
                                    const void * arg_value) CL_API_SUFFIX__VERSION_2_0 {
  return SHIM_CALL(clSetKernelArg)(kernel, arg_index, sizeof(void*), &arg_value);
}

CL_API_ENTRY cl_int CL_API_CALL
SHIM_CALL(clSetKernelExecInfo)(cl_kernel            kernel,
                               cl_kernel_exec_info  param_name,
                               size_t               param_value_size,
                               const void *         param_value) CL_API_SUFFIX__VERSION_2_0 {
  return CL_INVALID_VALUE;
}

CL_API_ENTRY cl_int CL_API_CALL
SHIM_CALL(clGetKernelInfo)(cl_kernel       kernel,
                           cl_kernel_info  param_name,
                           size_t          param_value_size,
                           void *          param_value,
                           size_t *        param_value_size_ret) CL_API_SUFFIX__VERSION_1_0 {
  return CL_INVALID_VALUE;
}

CL_API_ENTRY cl_int CL_API_CALL
SHIM_CALL(clGetKernelArgInfo)(cl_kernel       kernel,
                              cl_uint         arg_indx,
                              cl_kernel_arg_info  param_name,
                              size_t          param_value_size,
                              void *          param_value,
                              size_t *        param_value_size_ret) CL_API_SUFFIX__VERSION_1_2 {
  return CL_INVALID_VALUE;
}

CL_API_ENTRY cl_int CL_API_CALL
SHIM_CALL(clGetKernelWorkGroupInfo)(cl_kernel                  kernel,
                                    cl_device_id               device,
                                    cl_kernel_work_group_info  param_name,
                                    size_t                     param_value_size,
                                    void *                     param_value,
                                    size_t *                   param_value_size_ret) CL_API_SUFFIX__VERSION_1_0 {
  return CL_INVALID_VALUE;
}

CL_API_ENTRY cl_int CL_API_CALL
SHIM_CALL(clGetKernelSubGroupInfo)(cl_kernel                   kernel,
                                   cl_device_id                device,
                                   cl_kernel_sub_group_info    param_name,
                                   size_t                      input_value_size,
                                   const void*                 input_value,
                                   size_t                      param_value_size,
                                   void*                       param_value,
                                   size_t*                     param_value_size_ret) CL_API_SUFFIX__VERSION_2_1 {
  return CL_INVALID_VALUE;
}

extern "C" {
SHIM_EXPORT(clCreateKernel);
SHIM_EXPORT(clCreateKernelsInProgram);
SHIM_EXPORT(clCloneKernel);
SHIM_EXPORT(clRetainKernel);
SHIM_EXPORT(clReleaseKernel);
SHIM_EXPORT(clSetKernelArg);
SHIM_EXPORT(clSetKernelArgSVMPointer);
SHIM_EXPORT(clSetKernelExecInfo);
SHIM_EXPORT(clGetKernelInfo);
SHIM_EXPORT(clGetKernelArgInfo);
SHIM_EXPORT(clGetKernelWorkGroupInfo);
SHIM_EXPORT(clGetKernelSubGroupInfo);
}
