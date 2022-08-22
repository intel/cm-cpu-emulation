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
SHIM_CALL(clCreateKernel)(cl_program program, const char *kernel_name,
                          cl_int *errcode_ret) CL_API_SUFFIX__VERSION_1_0 {
  shim::IntrusivePtr<shim::cl::Program> prog(
      static_cast<shim::cl::Program *>(program));

  if (!prog->prog_) {
    ERRCODE(CL_INVALID_PROGRAM_EXECUTABLE);
    return nullptr;
  }

  if (!kernel_name) {
    ERRCODE(CL_INVALID_VALUE);
    return nullptr;
  }

  CmKernel *k = nullptr;
  if (prog->ctx_->dev_.device->CreateKernel(prog->prog_.get(), kernel_name,
                                            k) != CM_SUCCESS) {
    ERRCODE(CL_INVALID_KERNEL_NAME);
    return nullptr;
  }

  shim::IntrusivePtr<shim::cl::Kernel> kernel =
      new (std::nothrow) shim::cl::Kernel(prog, k);

  if (!kernel) {
    ERRCODE(CL_OUT_OF_HOST_MEMORY);
    return nullptr;
  }

  ERRCODE(CL_SUCCESS);

  shim::IntrusivePtrAddRef(kernel.get());
  return kernel.get();
}

CL_API_ENTRY cl_int CL_API_CALL SHIM_CALL(clCreateKernelsInProgram)(
    cl_program program, cl_uint num_kernels, cl_kernel *kernels,
    cl_uint *num_kernels_ret) CL_API_SUFFIX__VERSION_1_0 {
  return CL_OUT_OF_RESOURCES;
}

CL_API_ENTRY cl_kernel CL_API_CALL SHIM_CALL(clCloneKernel)(
    cl_kernel source_kernel, cl_int *errcode_ret) CL_API_SUFFIX__VERSION_2_1 {
  ERRCODE(CL_OUT_OF_RESOURCES);
  return nullptr;
}

CL_API_ENTRY cl_int CL_API_CALL SHIM_CALL(clRetainKernel)(cl_kernel kernel)
    CL_API_SUFFIX__VERSION_1_0 {
  shim::IntrusivePtrAddRef(static_cast<shim::cl::Kernel *>(kernel));
  return CL_SUCCESS;
}

CL_API_ENTRY cl_int CL_API_CALL SHIM_CALL(clReleaseKernel)(cl_kernel kernel)
    CL_API_SUFFIX__VERSION_1_0 {
  shim::IntrusivePtr<shim::cl::Kernel> k(
      static_cast<shim::cl::Kernel *>(kernel), false);
  return CL_SUCCESS;
}

static bool isMemObjectValid(shim::cl::Device &dev, shim::cl::Memory *mem) {
  return dev.buffers_.find(mem) != dev.buffers_.end();
}

CL_API_ENTRY cl_int CL_API_CALL
SHIM_CALL(clSetKernelArg)(cl_kernel kernel, cl_uint arg_index, size_t arg_size,
                          const void *arg_value) CL_API_SUFFIX__VERSION_1_0 {
  shim::IntrusivePtr<shim::cl::Kernel> kern(
      static_cast<shim::cl::Kernel *>(kernel));

  auto ctx = kern->prog_->ctx_;

  auto fdesc = kern->kern_->GetFunctionDesc();
  if (arg_index >= fdesc.params.size()) {
    return CL_INVALID_ARG_INDEX;
  }
  const auto &argdesc = fdesc.params[arg_index];

  if (argdesc.isClass) {
    if (arg_value == nullptr) {
      return CL_INVALID_ARG_VALUE;
    }

    if (argdesc.typeName == "SurfaceIndex" || // Workaround for kernel<half>
        (argdesc.typeName == "" && arg_size == sizeof(void *))) {
      if (sizeof(void *) != arg_size) {
        return CL_INVALID_ARG_SIZE;
      }

      void *ptr = nullptr;
      std::memcpy(&ptr, arg_value, sizeof(ptr));

      shim::cl::Memory *mem = static_cast<shim::cl::Memory *>(ptr);
      if (!isMemObjectValid(ctx->dev_, mem)) {
        return CL_INVALID_ARG_VALUE;
      }

      auto &mm = ctx->mm_;
      SurfaceIndex *bti = std::visit(
          shim::overloaded{[&mm](void *ptr) { return mm.GetIndex(ptr); },
                           [](CmSurface2D *img) {
                             SurfaceIndex *idx = nullptr;
                             img->GetIndex(idx);
                             return idx;
                           },
                           [](CmSurface3D *img) {
                             SurfaceIndex *idx = nullptr;
                             img->GetIndex(idx);
                             return idx;
                           }},
          mem->buffer_);

      auto r = kern->kern_->SetKernelArg(arg_index, sizeof(SurfaceIndex), bti);
      if (r != CM_SUCCESS) {
        return CL_INVALID_ARG_VALUE;
      }
    } else if (argdesc.typeName == "SamplerIndex") {
      GFX_EMU_WARNING_MESSAGE(fShim, "sampler support is not implemented\n");
      return CL_INVALID_ARG_VALUE;
    } else {
      GFX_EMU_WARNING_MESSAGE(fShim, "unknown kernel argument #%u type: '%s'\n",
                              arg_index, argdesc.typeName.c_str());
      return CL_INVALID_ARG_VALUE;
    }
  } else if (argdesc.isPointer) { // stateless buffer
    void *ptr = nullptr;
    std::memcpy(&ptr, arg_value, sizeof(ptr));

    shim::cl::Memory *mem = static_cast<shim::cl::Memory *>(ptr);
    if (!isMemObjectValid(ctx->dev_, mem)) {
      return CL_INVALID_ARG_VALUE;
    }

    void **pptr = std::get_if<void *>(&mem->buffer_);
    if (pptr == nullptr)
      return CL_INVALID_ARG_VALUE;

    auto r = kern->kern_->SetKernelArg(arg_index, sizeof(void *), pptr);
    if (r != CM_SUCCESS)
      return CL_INVALID_ARG_VALUE;
  } else { // scalar, vector or matrix
    if (argdesc.size != 0 && argdesc.size != arg_size) {
      return CL_INVALID_ARG_SIZE;
    }

    void *ptr = nullptr;
    const void *parg = arg_value;

    if (arg_size == sizeof(void *)) { // It may be svmptr_t
      void *ptr = nullptr;
      std::memcpy(&ptr, arg_value, sizeof(ptr));

      if (shim::cl::Memory *mem = static_cast<shim::cl::Memory *>(ptr);
          isMemObjectValid(ctx->dev_, mem)) {
        void **pptr = std::get_if<void *>(&mem->buffer_);
        if (pptr == nullptr)
          return CL_INVALID_ARG_VALUE;
        ptr = *pptr;
        parg = &ptr;
      }
    }

    auto r = kern->kern_->SetKernelArg(arg_index, arg_size, parg);
    if (r != CM_SUCCESS) {
      return CL_INVALID_ARG_SIZE;
    }
  }

  return CL_SUCCESS;
}

CL_API_ENTRY cl_int CL_API_CALL SHIM_CALL(clSetKernelArgSVMPointer)(
    cl_kernel kernel, cl_uint arg_index,
    const void *arg_value) CL_API_SUFFIX__VERSION_2_0 {
  return SHIM_CALL(clSetKernelArg)(kernel, arg_index, sizeof(void *),
                                   &arg_value);
}

CL_API_ENTRY cl_int CL_API_CALL SHIM_CALL(clSetKernelExecInfo)(
    cl_kernel kernel, cl_kernel_exec_info param_name, size_t param_value_size,
    const void *param_value) CL_API_SUFFIX__VERSION_2_0 {
  return CL_INVALID_VALUE;
}

CL_API_ENTRY cl_int CL_API_CALL SHIM_CALL(clGetKernelInfo)(
    cl_kernel kernel, cl_kernel_info param_name, size_t param_value_size,
    void *param_value,
    size_t *param_value_size_ret) CL_API_SUFFIX__VERSION_1_0 {
  return CL_INVALID_VALUE;
}

CL_API_ENTRY cl_int CL_API_CALL SHIM_CALL(clGetKernelArgInfo)(
    cl_kernel kernel, cl_uint arg_indx, cl_kernel_arg_info param_name,
    size_t param_value_size, void *param_value,
    size_t *param_value_size_ret) CL_API_SUFFIX__VERSION_1_2 {
  return CL_INVALID_VALUE;
}

CL_API_ENTRY cl_int CL_API_CALL SHIM_CALL(clGetKernelWorkGroupInfo)(
    cl_kernel kernel, cl_device_id device, cl_kernel_work_group_info param_name,
    size_t param_value_size, void *param_value,
    size_t *param_value_size_ret) CL_API_SUFFIX__VERSION_1_0 {
  return CL_INVALID_VALUE;
}

CL_API_ENTRY cl_int CL_API_CALL SHIM_CALL(clGetKernelSubGroupInfo)(
    cl_kernel kernel, cl_device_id device, cl_kernel_sub_group_info param_name,
    size_t input_value_size, const void *input_value, size_t param_value_size,
    void *param_value,
    size_t *param_value_size_ret) CL_API_SUFFIX__VERSION_2_1 {
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
