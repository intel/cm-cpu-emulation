/*========================== begin_copyright_notice ============================

Copyright (C) 2021 Intel Corporation

SPDX-License-Identifier: MIT

============================= end_copyright_notice ===========================*/

#include "context.h"
#include "runtime.h"

extern "C" {

CL_API_ENTRY cl_context CL_API_CALL
SHIM_CALL(clCreateContext)(const cl_context_properties * properties,
                           cl_uint              num_devices,
                           const cl_device_id * devices,
                           void (CL_CALLBACK * pfn_notify)(const char * errinfo,
                                                           const void * private_info,
                                                           size_t       cb,
                                                           void *       user_data),
                           void *               user_data,
                           cl_int *             errcode_ret) CL_API_SUFFIX__VERSION_1_0 {
  auto &rt = shim::cl::Runtime::Instance();
  ERRCODE(CL_SUCCESS);

  shim::IntrusivePtr<shim::cl::Context> ctx(nullptr);

  if (devices && (num_devices < 1 || devices[0] != &rt.device)){
    ERRCODE(CL_INVALID_DEVICE);
  } else {
    ctx.reset(new(std::nothrow) shim::cl::Context(rt.device));

    if (!ctx) {
      ERRCODE(CL_OUT_OF_HOST_MEMORY);
    }
  }

  if (pfn_notify) {
    const char *errinfo = ctx ? "ok" : "fail";
    pfn_notify(errinfo, nullptr, 0, user_data);
  }

  IntrusivePtrAddRef(ctx.get());
  return ctx.get();
}

CL_API_ENTRY cl_context CL_API_CALL
SHIM_CALL(clCreateContextFromType)(const cl_context_properties * properties,
                                   cl_device_type      device_type,
                                   void (CL_CALLBACK * pfn_notify)(const char * errinfo,
                                                                   const void * private_info,
                                                                   size_t       cb,
                                                                   void *       user_data),
                                   void *              user_data,
                                   cl_int *            errcode_ret) CL_API_SUFFIX__VERSION_1_0 {
  cl_device_id dev = nullptr;
  cl_uint num_devs = 0;
  if (auto status = SHIM_CALL(clGetDeviceIDs)(nullptr, device_type, 1, &dev, &num_devs); status != CL_SUCCESS) {
    ERRCODE(status);
    return nullptr;
  }

  return SHIM_CALL(clCreateContext)(properties, 1, &dev, pfn_notify, user_data, errcode_ret);
}

extern CL_API_ENTRY cl_int CL_API_CALL
SHIM_CALL(clRetainContext)(cl_context context) CL_API_SUFFIX__VERSION_1_0 {
  shim::IntrusivePtrAddRef(static_cast<shim::cl::Context*>(context));
  return CL_SUCCESS;
}

CL_API_ENTRY cl_int CL_API_CALL
SHIM_CALL(clReleaseContext)(cl_context context) CL_API_SUFFIX__VERSION_1_0 {
  shim::IntrusivePtr<shim::cl::Context> ctx(static_cast<shim::cl::Context*>(context), false);
  return CL_SUCCESS;
}

CL_API_ENTRY cl_int CL_API_CALL
SHIM_CALL(clGetContextInfo)(cl_context         context,
                            cl_context_info    param_name,
                            size_t             param_value_size,
                            void *             param_value,
                            size_t *           param_value_size_ret) CL_API_SUFFIX__VERSION_1_0 {
  using shim::cl::SetResult;
  shim::IntrusivePtr<shim::cl::Context> ctx(static_cast<shim::cl::Context*>(context));

  switch (param_name) {
    case CL_CONTEXT_REFERENCE_COUNT:
      return SetResult<cl_uint>(ctx->UseCount() - 1, param_value_size,
                                param_value, param_value_size_ret);
    case CL_CONTEXT_NUM_DEVICES:
      return SetResult<cl_uint>(1, param_value_size,
                                param_value, param_value_size_ret);
    case CL_CONTEXT_DEVICES:
      return SetResult<cl_device_id>(&ctx->dev_, param_value_size,
                                     param_value, param_value_size_ret);
    case CL_CONTEXT_PROPERTIES:
      return SetResult<cl_context_properties>(0, param_value_size, param_value,
                                              param_value_size_ret);
  }

  return CL_INVALID_VALUE;
}

CL_API_ENTRY cl_int CL_API_CALL
SHIM_CALL(clSetContextDestructorCallback)(cl_context         context,
                                          void (CL_CALLBACK* pfn_notify)(cl_context context,
                                                                         void* user_data),
                                          void*              user_data) CL_API_SUFFIX__VERSION_3_0 {
  shim::IntrusivePtr<shim::cl::Context> ctx(static_cast<shim::cl::Context*>(context));

  ctx->pfn_notify = pfn_notify;
  ctx->user_data = user_data;
  return CL_SUCCESS;
}

SHIM_EXPORT(clCreateContext);
SHIM_EXPORT(clCreateContextFromType);
SHIM_EXPORT(clRetainContext);
SHIM_EXPORT(clReleaseContext);
SHIM_EXPORT(clGetContextInfo);
SHIM_EXPORT(clSetContextDestructorCallback);
}
