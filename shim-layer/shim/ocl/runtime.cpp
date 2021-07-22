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

#include "runtime.h"

#include "context.h"
#include "device.h"
#include "event.h"
#include "kernel.h"
#include "memory.h"
#include "platform.h"
#include "program.h"
#include "queue.h"

#include <cstring>

namespace shim {
namespace cl {

#define DUMMY(x) nullptr
#define ENTRY(x) SHIM_CALL(x)
static cl_icd_dispatch dispatch_ = {
  ENTRY(clGetPlatformIDs),
  ENTRY(clGetPlatformInfo),
  ENTRY(clGetDeviceIDs),
  ENTRY(clGetDeviceInfo),
  ENTRY(clCreateContext),
  ENTRY(clCreateContextFromType),
  ENTRY(clRetainContext),
  ENTRY(clReleaseContext),
  ENTRY(clGetContextInfo),
  ENTRY(clCreateCommandQueue),
  ENTRY(clRetainCommandQueue),
  ENTRY(clReleaseCommandQueue),
  ENTRY(clGetCommandQueueInfo),
  ENTRY(clSetCommandQueueProperty),
  ENTRY(clCreateBuffer),
  DUMMY(clCreateImage2D),
  DUMMY(clCreateImage3D),
  ENTRY(clRetainMemObject),
  ENTRY(clReleaseMemObject),
  DUMMY(clGetSupportedImageFormats),
  DUMMY(clGetMemObjectInfo),
  DUMMY(clGetImageInfo),
  DUMMY(clCreateSampler),
  DUMMY(clRetainSampler),
  DUMMY(clReleaseSampler),
  DUMMY(clGetSamplerInfo),
  ENTRY(clCreateProgramWithSource),
  ENTRY(clCreateProgramWithBinary),
  ENTRY(clRetainProgram),
  ENTRY(clReleaseProgram),
  ENTRY(clBuildProgram),
  ENTRY(clUnloadCompiler),
  ENTRY(clGetProgramInfo),
  ENTRY(clGetProgramBuildInfo),
  ENTRY(clCreateKernel),
  ENTRY(clCreateKernelsInProgram),
  ENTRY(clRetainKernel),
  ENTRY(clReleaseKernel),
  ENTRY(clSetKernelArg),
  ENTRY(clGetKernelInfo),
  ENTRY(clGetKernelWorkGroupInfo),
  ENTRY(clWaitForEvents),
  DUMMY(clGetEventInfo),
  ENTRY(clRetainEvent),
  ENTRY(clReleaseEvent),
  ENTRY(clGetEventProfilingInfo),
  ENTRY(clFlush),
  ENTRY(clFinish),
  ENTRY(clEnqueueReadBuffer),
  ENTRY(clEnqueueWriteBuffer),
  DUMMY(clEnqueueCopyBuffer),
  ENTRY(clEnqueueReadImage),
  ENTRY(clEnqueueWriteImage),
  DUMMY(clEnqueueCopyImage),
  DUMMY(clEnqueueCopyImageToBuffer),
  DUMMY(clEnqueueCopyBufferToImage),
  DUMMY(clEnqueueMapBuffer),
  DUMMY(clEnqueueMapImage),
  DUMMY(clEnqueueUnmapMemObject),
  ENTRY(clEnqueueNDRangeKernel),
  DUMMY(clEnqueueTask),
  DUMMY(clEnqueueNativeKernel),
  DUMMY(clEnqueueMarker),
  DUMMY(clEnqueueWaitForEvents),
  DUMMY(clEnqueueBarrier),
  DUMMY(clGetExtensionFunctionAddress),
  DUMMY(clCreateFromGLBuffer),
  DUMMY(clCreateFromGLTexture2D),
  DUMMY(clCreateFromGLTexture3D),
  DUMMY(clCreateFromGLRenderbuffer),
  DUMMY(clGetGLObjectInfo),
  DUMMY(clGetGLTextureInfo),
  DUMMY(clEnqueueAcquireGLObjects),
  DUMMY(clEnqueueReleaseGLObjects),
  DUMMY(clGetGLContextInfoKHR),

  /* cl_khr_d3d10_sharing */
  DUMMY(clGetDeviceIDsFromD3D10KHR),
  DUMMY(clCreateFromD3D10BufferKHR),
  DUMMY(clCreateFromD3D10Texture2DKHR),
  DUMMY(clCreateFromD3D10Texture3DKHR),
  DUMMY(clEnqueueAcquireD3D10ObjectsKHR),
  DUMMY(clEnqueueReleaseD3D10ObjectsKHR),

  /* OpenCL 1.1 */
  DUMMY(clSetEventCallback),
  DUMMY(clCreateSubBuffer),
  DUMMY(clSetMemObjectDestructorCallback),
  DUMMY(clCreateUserEvent),
  DUMMY(clSetUserEventStatus),
  DUMMY(clEnqueueReadBufferRect),
  DUMMY(clEnqueueWriteBufferRect),
  DUMMY(clEnqueueCopyBufferRect),

  /* cl_ext_device_fission */
  DUMMY(clCreateSubDevicesEXT),
  DUMMY(clRetainDeviceEXT),
  DUMMY(clReleaseDeviceEXT),

  /* cl_khr_gl_event */
  DUMMY(clCreateEventFromGLsyncKHR),

  /* OpenCL 1.2 */
  DUMMY(clCreateSubDevices),
  ENTRY(clRetainDevice),
  ENTRY(clReleaseDevice),
  ENTRY(clCreateImage),
  ENTRY(clCreateProgramWithBuiltInKernels),
  ENTRY(clCompileProgram),
  ENTRY(clLinkProgram),
  ENTRY(clUnloadPlatformCompiler),
  ENTRY(clGetKernelArgInfo),
  DUMMY(clEnqueueFillBuffer),
  DUMMY(clEnqueueFillImage),
  DUMMY(clEnqueueMigrateMemObjects),
  DUMMY(clEnqueueMarkerWithWaitList),
  DUMMY(clEnqueueBarrierWithWaitList),
  DUMMY(clGetExtensionFunctionAddressForPlatform),
  DUMMY(clCreateFromGLTexture),

  /* cl_khr_d3d11_sharing */
  DUMMY(clGetDeviceIDsFromD3D11KHR),
  DUMMY(clCreateFromD3D11BufferKHR),
  DUMMY(clCreateFromD3D11Texture2DKHR),
  DUMMY(clCreateFromD3D11Texture3DKHR),
  DUMMY(clCreateFromDX9MediaSurfaceKHR),
  DUMMY(clEnqueueAcquireD3D11ObjectsKHR),
  DUMMY(clEnqueueReleaseD3D11ObjectsKHR),

  /* cl_khr_dx9_media_sharing */
  DUMMY(clGetDeviceIDsFromDX9MediaAdapterKHR),
  DUMMY(clEnqueueAcquireDX9MediaSurfacesKHR),
  DUMMY(clEnqueueReleaseDX9MediaSurfacesKHR),

  /* cl_khr_egl_image */
  DUMMY(clCreateFromEGLImageKHR),
  DUMMY(clEnqueueAcquireEGLObjectsKHR),
  DUMMY(clEnqueueReleaseEGLObjectsKHR),

  /* cl_khr_egl_event */
  DUMMY(clCreateEventFromEGLSyncKHR),

  /* OpenCL 2.0 */
  ENTRY(clCreateCommandQueueWithProperties),
  DUMMY(clCreatePipe),
  DUMMY(clGetPipeInfo),
  ENTRY(clSVMAlloc),
  ENTRY(clSVMFree),
  DUMMY(clEnqueueSVMFree),
  DUMMY(clEnqueueSVMMemcpy),
  DUMMY(clEnqueueSVMMemFill),
  ENTRY(clEnqueueSVMMap),
  ENTRY(clEnqueueSVMUnmap),
  DUMMY(clCreateSamplerWithProperties),
  ENTRY(clSetKernelArgSVMPointer),
  DUMMY(clSetKernelExecInfo),

  /* cl_khr_sub_groups */
  DUMMY(clGetKernelSubGroupInfoKHR),

  /* OpenCL 2.1 */
  ENTRY(clCloneKernel),
  ENTRY(clCreateProgramWithIL),
  DUMMY(clEnqueueSVMMigrateMem),
  DUMMY(clGetDeviceAndHostTimer),
  DUMMY(clGetHostTimer),
  ENTRY(clGetKernelSubGroupInfo),
  DUMMY(clSetDefaultDeviceCommandQueue),

  /* OpenCL 2.2 */
  DUMMY(clSetProgramReleaseCallback),
  DUMMY(clSetProgramSpecializationConstant),

  /* OpenCL 3.0 */
  ENTRY(clCreateBufferWithProperties),
  ENTRY(clCreateImageWithProperties),
  ENTRY(clSetContextDestructorCallback),
};

Runtime::Runtime() : platform(&dispatch_), device(&dispatch_) {
}

Runtime &Runtime::Instance() {
  static Runtime rt;
  return rt;
}

cl_int SetResult(std::string_view value, size_t buffer_size,
                 void *buffer, size_t *buffer_size_ret) {
  if (!buffer && !buffer_size_ret) {
    return CL_INVALID_VALUE;
  }
  if (buffer && buffer_size <= value.size()) {
    return CL_INVALID_BUFFER_SIZE;
  }

  if (buffer) {
    char *b = static_cast<char*>(buffer);
    std::strncpy(b, value.data(), buffer_size - 1);
    b[buffer_size - 1] = '\0';
  }

  if (buffer_size_ret) {
    *buffer_size_ret = value.size() + 1;
  }

  return CL_SUCCESS;
}

} // namespace cl
} // namespace shim
