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

#include "memory.h"

#include <map>

static CM_SURFACE_FORMAT ImageFormatToCmFormat(const cl_image_format * format) {
  using ULongPair = std::pair<unsigned long, unsigned long>;
  using FmtMap = std::map<ULongPair, CM_SURFACE_FORMAT>;
  static const FmtMap cl2cm = {
    {{CL_UNORM_INT8, CL_RGBA}, CM_SURFACE_FORMAT_A8R8G8B8},
    {{CL_UNORM_INT8, CL_ARGB}, CM_SURFACE_FORMAT_A8R8G8B8},
  };

  if (auto result = cl2cm.find({ format->image_channel_data_type, format->image_channel_order });
      result != cl2cm.end()) {
    return result->second;
  }

  return CM_SURFACE_FORMAT_A8R8G8B8;
}

extern "C" {

CL_API_ENTRY cl_mem CL_API_CALL
SHIM_CALL(clCreateBuffer)(cl_context   context,
                          cl_mem_flags flags,
                          size_t       size,
                          void *       host_ptr,
                          cl_int *     errcode_ret) CL_API_SUFFIX__VERSION_1_0 {
  shim::IntrusivePtr<shim::cl::Context> ctx(static_cast<shim::cl::Context*>(context));

  ERRCODE(CL_SUCCESS);

  CmBuffer *buf = nullptr;
  if (ctx->dev_.device->CreateBuffer(size, buf) != CM_SUCCESS) {
    ERRCODE(CL_OUT_OF_RESOURCES);
    return nullptr;
  }

  shim::IntrusivePtr<shim::cl::Memory> mem = new(std::nothrow) shim::cl::Memory(ctx, buf);

  if (!mem) {
    ERRCODE(CL_OUT_OF_RESOURCES);
    ctx->dev_.device->DestroySurface(buf);
    return nullptr;
  }

  if (host_ptr && buf->WriteSurface(static_cast<unsigned char*>(host_ptr), nullptr, size) != CM_SUCCESS) {
    ERRCODE(CL_INVALID_HOST_PTR);
    return nullptr;
  }

  ctx->dev_.buffers_.insert(mem.get());

  IntrusivePtrAddRef(mem.get());
  return mem.get();
}

CL_API_ENTRY cl_mem CL_API_CALL
SHIM_CALL(clCreateSubBuffer)(cl_mem                   buffer,
                             cl_mem_flags             flags,
                             cl_buffer_create_type    buffer_create_type,
                             const void *             buffer_create_info,
                             cl_int *                 errcode_ret) CL_API_SUFFIX__VERSION_1_1 {
  ERRCODE(CL_OUT_OF_RESOURCES);
  return nullptr;
}

CL_API_ENTRY cl_mem CL_API_CALL
SHIM_CALL(clCreateImage)(cl_context              context,
                         cl_mem_flags            flags,
                         const cl_image_format * image_format,
                         const cl_image_desc *   image_desc,
                         void *                  host_ptr,
                         cl_int *                errcode_ret) CL_API_SUFFIX__VERSION_1_2 {
  shim::IntrusivePtr<shim::cl::Context> ctx(static_cast<shim::cl::Context*>(context));

  ERRCODE(CL_SUCCESS);

  if (!image_format || !image_desc || image_desc->image_type != CL_MEM_OBJECT_IMAGE2D) {
    ERRCODE(CL_INVALID_IMAGE_DESCRIPTOR);
    return nullptr;
  }

  CmSurface2D *surface = nullptr;
  if (ctx->dev_.device->CreateSurface2D(image_desc->image_width, image_desc->image_height,
                                        ImageFormatToCmFormat(image_format), surface) != CM_SUCCESS) {
    ERRCODE(CL_OUT_OF_RESOURCES);
    return nullptr;
  }

  if (host_ptr) {
    if (surface->WriteSurface(reinterpret_cast<const unsigned char *>(host_ptr), nullptr) != CM_SUCCESS) {
      ERRCODE(CL_OUT_OF_RESOURCES);
      return nullptr;
    }
  }

  shim::IntrusivePtr<shim::cl::Memory> mem = new(std::nothrow) shim::cl::Memory(ctx, surface);

  if (!mem) {
    ERRCODE(CL_OUT_OF_HOST_MEMORY);
    return nullptr;
  }

  ctx->dev_.buffers_.insert(mem.get());

  shim::IntrusivePtrAddRef(mem.get());
  return mem.get();
}

CL_API_ENTRY cl_mem CL_API_CALL
SHIM_CALL(clCreateBufferWithProperties)(cl_context                context,
                                        const cl_mem_properties * properties,
                                        cl_mem_flags              flags,
                                        size_t                    size,
                                        void *                    host_ptr,
                                        cl_int *                  errcode_ret) CL_API_SUFFIX__VERSION_3_0 {
  return SHIM_CALL(clCreateBuffer)(context, flags, size, host_ptr, errcode_ret);
}

CL_API_ENTRY cl_mem CL_API_CALL
SHIM_CALL(clCreateImageWithProperties)(cl_context                context,
                                       const cl_mem_properties * properties,
                                       cl_mem_flags              flags,
                                       const cl_image_format *   image_format,
                                       const cl_image_desc *     image_desc,
                                       void *                    host_ptr,
                                       cl_int *                  errcode_ret) CL_API_SUFFIX__VERSION_3_0 {
  return SHIM_CALL(clCreateImage)(context, flags, image_format, image_desc, host_ptr, errcode_ret);
}

CL_API_ENTRY cl_int CL_API_CALL
SHIM_CALL(clRetainMemObject)(cl_mem memobj) CL_API_SUFFIX__VERSION_1_0 {
  shim::IntrusivePtrAddRef(static_cast<shim::cl::Memory*>(memobj));
  return CL_SUCCESS;
}

CL_API_ENTRY cl_int CL_API_CALL
SHIM_CALL(clReleaseMemObject)(cl_mem memobj) CL_API_SUFFIX__VERSION_1_0 {
  shim::IntrusivePtr<shim::cl::Memory> mem(static_cast<shim::cl::Memory*>(memobj), false);

  if (mem->UseCount() == 1) {
    mem->ctx_->dev_.buffers_.erase(mem.get());
  }

  return CL_SUCCESS;
}

CL_API_ENTRY void * CL_API_CALL
SHIM_CALL(clSVMAlloc)(cl_context       context,
                      cl_svm_mem_flags flags,
                      size_t           size,
                      cl_uint          alignment) CL_API_SUFFIX__VERSION_2_0 {
  return std::malloc(size);
}

CL_API_ENTRY void CL_API_CALL
SHIM_CALL(clSVMFree)(cl_context        context,
                     void *            svm_pointer) CL_API_SUFFIX__VERSION_2_0 {
  std::free(svm_pointer);
}

SHIM_EXPORT(clCreateBuffer);
SHIM_EXPORT(clCreateSubBuffer);
SHIM_EXPORT(clCreateImage);
SHIM_EXPORT(clCreateBufferWithProperties);
SHIM_EXPORT(clCreateImageWithProperties);
SHIM_EXPORT(clRetainMemObject);
SHIM_EXPORT(clReleaseMemObject);
SHIM_EXPORT(clSVMAlloc);
SHIM_EXPORT(clSVMFree);
}
