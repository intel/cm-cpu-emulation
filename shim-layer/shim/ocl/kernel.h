/*========================== begin_copyright_notice ============================

Copyright (C) 2021 Intel Corporation

SPDX-License-Identifier: MIT

============================= end_copyright_notice ===========================*/

#ifndef CM_EMU_SHIM_OCL_KERNEL_H
#define CM_EMU_SHIM_OCL_KERNEL_H

#include "ocl.h"
#include "program.h"

extern "C" {
struct _cl_kernel {
  cl_icd_dispatch *dispatch;
};

extern CL_API_ENTRY cl_kernel CL_API_CALL
SHIM_CALL(clCreateKernel)(cl_program      program,
                          const char *    kernel_name,
                          cl_int *        errcode_ret) CL_API_SUFFIX__VERSION_1_0;

extern CL_API_ENTRY cl_int CL_API_CALL
SHIM_CALL(clCreateKernelsInProgram)(cl_program     program,
                                    cl_uint        num_kernels,
                                    cl_kernel *    kernels,
                                    cl_uint *      num_kernels_ret) CL_API_SUFFIX__VERSION_1_0;

extern CL_API_ENTRY cl_kernel CL_API_CALL
SHIM_CALL(clCloneKernel)(cl_kernel     source_kernel,
                         cl_int*       errcode_ret) CL_API_SUFFIX__VERSION_2_1;

extern CL_API_ENTRY cl_int CL_API_CALL
SHIM_CALL(clRetainKernel)(cl_kernel    kernel) CL_API_SUFFIX__VERSION_1_0;

extern CL_API_ENTRY cl_int CL_API_CALL
SHIM_CALL(clReleaseKernel)(cl_kernel   kernel) CL_API_SUFFIX__VERSION_1_0;

extern CL_API_ENTRY cl_int CL_API_CALL
SHIM_CALL(clSetKernelArg)(cl_kernel    kernel,
                          cl_uint      arg_index,
                          size_t       arg_size,
                          const void * arg_value) CL_API_SUFFIX__VERSION_1_0;

extern CL_API_ENTRY cl_int CL_API_CALL
SHIM_CALL(clSetKernelArgSVMPointer)(cl_kernel    kernel,
                                    cl_uint      arg_index,
                                    const void * arg_value) CL_API_SUFFIX__VERSION_2_0;

extern CL_API_ENTRY cl_int CL_API_CALL
SHIM_CALL(clSetKernelExecInfo)(cl_kernel            kernel,
                               cl_kernel_exec_info  param_name,
                               size_t               param_value_size,
                               const void *         param_value) CL_API_SUFFIX__VERSION_2_0;

extern CL_API_ENTRY cl_int CL_API_CALL
SHIM_CALL(clGetKernelInfo)(cl_kernel       kernel,
                           cl_kernel_info  param_name,
                           size_t          param_value_size,
                           void *          param_value,
                           size_t *        param_value_size_ret) CL_API_SUFFIX__VERSION_1_0;

extern CL_API_ENTRY cl_int CL_API_CALL
SHIM_CALL(clGetKernelArgInfo)(cl_kernel       kernel,
                              cl_uint         arg_indx,
                              cl_kernel_arg_info  param_name,
                              size_t          param_value_size,
                              void *          param_value,
                              size_t *        param_value_size_ret) CL_API_SUFFIX__VERSION_1_2;

extern CL_API_ENTRY cl_int CL_API_CALL
SHIM_CALL(clGetKernelWorkGroupInfo)(cl_kernel                  kernel,
                                    cl_device_id               device,
                                    cl_kernel_work_group_info  param_name,
                                    size_t                     param_value_size,
                                    void *                     param_value,
                                    size_t *                   param_value_size_ret) CL_API_SUFFIX__VERSION_1_0;

extern CL_API_ENTRY cl_int CL_API_CALL
SHIM_CALL(clGetKernelSubGroupInfo)(cl_kernel                   kernel,
                                   cl_device_id                device,
                                   cl_kernel_sub_group_info    param_name,
                                   size_t                      input_value_size,
                                   const void*                 input_value,
                                   size_t                      param_value_size,
                                   void*                       param_value,
                                   size_t*                     param_value_size_ret) CL_API_SUFFIX__VERSION_2_1;

}

namespace shim {
namespace cl {

struct Kernel : public _cl_kernel, public IntrusiveRefCounter<Kernel> {
  Kernel(IntrusivePtr<Program> prog, CmKernel *kern) :
      prog_(prog), kern_(kern, [this](CmKernel *p) { prog_->ctx_->dev_.device->DestroyKernel(p); }) {
    dispatch = prog_->dispatch;
  }

  std::shared_ptr<CmKernel> kern_;
  IntrusivePtr<Program> prog_;
};

} // namespace cl
} // namespace shim

#endif // CM_EMU_SHIM_OCL_KERNEL_H
