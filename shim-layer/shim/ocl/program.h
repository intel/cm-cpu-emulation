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

#ifndef CM_EMU_SHIM_OCL_PROGRAM_H
#define CM_EMU_SHIM_OCL_PROGRAM_H

#include <algorithm>
#include <iterator>
#include <fstream>
#include <sstream>
#include <vector>

#include "context.h"

extern "C" {
struct _cl_program {
  cl_icd_dispatch *dispatch;
};

extern CL_API_ENTRY cl_program CL_API_CALL
SHIM_CALL(clCreateProgramWithSource)(cl_context        context,
                                     cl_uint           count,
                                     const char **     strings,
                                     const size_t *    lengths,
                                     cl_int *          errcode_ret) CL_API_SUFFIX__VERSION_1_0;

extern CL_API_ENTRY cl_program CL_API_CALL
SHIM_CALL(clCreateProgramWithBinary)(cl_context                     context,
                                     cl_uint                        num_devices,
                                     const cl_device_id *           device_list,
                                     const size_t *                 lengths,
                                     const unsigned char **         binaries,
                                     cl_int *                       binary_status,
                                     cl_int *                       errcode_ret) CL_API_SUFFIX__VERSION_1_0;

extern CL_API_ENTRY cl_program CL_API_CALL
SHIM_CALL(clCreateProgramWithBuiltInKernels)(cl_context            context,
                                             cl_uint               num_devices,
                                             const cl_device_id *  device_list,
                                             const char *          kernel_names,
                                             cl_int *              errcode_ret) CL_API_SUFFIX__VERSION_1_2;

extern CL_API_ENTRY cl_program CL_API_CALL
SHIM_CALL(clCreateProgramWithIL)(cl_context    context,
                                 const void*    il,
                                 size_t         length,
                                 cl_int*        errcode_ret) CL_API_SUFFIX__VERSION_2_1;

extern CL_API_ENTRY cl_int CL_API_CALL
SHIM_CALL(clRetainProgram)(cl_program program) CL_API_SUFFIX__VERSION_1_0;

extern CL_API_ENTRY cl_int CL_API_CALL
SHIM_CALL(clReleaseProgram)(cl_program program) CL_API_SUFFIX__VERSION_1_0;

extern CL_API_ENTRY cl_int CL_API_CALL
SHIM_CALL(clBuildProgram)(cl_program           program,
                          cl_uint              num_devices,
                          const cl_device_id * device_list,
                          const char *         options,
                          void (CL_CALLBACK *  pfn_notify)(cl_program program,
                                                           void * user_data),
                          void *               user_data) CL_API_SUFFIX__VERSION_1_0;

extern CL_API_ENTRY cl_int CL_API_CALL
SHIM_CALL(clCompileProgram)(cl_program           program,
                            cl_uint              num_devices,
                            const cl_device_id * device_list,
                            const char *         options,
                            cl_uint              num_input_headers,
                            const cl_program *   input_headers,
                            const char **        header_include_names,
                            void (CL_CALLBACK *  pfn_notify)(cl_program program,
                                                             void * user_data),
                            void *               user_data) CL_API_SUFFIX__VERSION_1_2;

extern CL_API_ENTRY cl_program CL_API_CALL
SHIM_CALL(clLinkProgram)(cl_context           context,
                         cl_uint              num_devices,
                         const cl_device_id * device_list,
                         const char *         options,
                         cl_uint              num_input_programs,
                         const cl_program *   input_programs,
                         void (CL_CALLBACK *  pfn_notify)(cl_program program,
                                                          void * user_data),
                         void *               user_data,
                         cl_int *             errcode_ret) CL_API_SUFFIX__VERSION_1_2;

extern CL_API_ENTRY CL_API_PREFIX__VERSION_2_2_DEPRECATED cl_int CL_API_CALL
SHIM_CALL(clSetProgramReleaseCallback)(cl_program          program,
                                       void (CL_CALLBACK * pfn_notify)(cl_program program,
                                                                       void * user_data),
                                       void *              user_data) CL_API_SUFFIX__VERSION_2_2_DEPRECATED;

extern CL_API_ENTRY cl_int CL_API_CALL
SHIM_CALL(clSetProgramSpecializationConstant)(cl_program  program,
                                              cl_uint     spec_id,
                                              size_t      spec_size,
                                              const void* spec_value) CL_API_SUFFIX__VERSION_2_2;

extern CL_API_ENTRY cl_int CL_API_CALL
SHIM_CALL(clUnloadPlatformCompiler)(cl_platform_id platform) CL_API_SUFFIX__VERSION_1_2;

extern CL_API_ENTRY cl_int CL_API_CALL
SHIM_CALL(clGetProgramInfo)(cl_program         program,
                            cl_program_info    param_name,
                            size_t             param_value_size,
                            void *             param_value,
                            size_t *           param_value_size_ret) CL_API_SUFFIX__VERSION_1_0;

extern CL_API_ENTRY cl_int CL_API_CALL
SHIM_CALL(clGetProgramBuildInfo)(cl_program            program,
                                 cl_device_id          device,
                                 cl_program_build_info param_name,
                                 size_t                param_value_size,
                                 void *                param_value,
                                 size_t *              param_value_size_ret) CL_API_SUFFIX__VERSION_1_0;

CL_API_ENTRY CL_API_PREFIX__VERSION_1_1_DEPRECATED cl_int CL_API_CALL
SHIM_CALL(clUnloadCompiler)(void) CL_API_SUFFIX__VERSION_1_1_DEPRECATED;

}

namespace shim {
namespace cl {

struct Program : public _cl_program, public IntrusiveRefCounter<Program>  {
  enum class Type {
    kInvalid,
    kSource,
    kObject,
    kBinary,
  };

  Program(IntrusivePtr<Context> ctx) : ctx_(ctx), type_(Type::kInvalid) {
    dispatch = ctx_->dispatch;
  }

  Program(IntrusivePtr<Context> ctx, std::vector<std::string> &&srcs) :
      ctx_(ctx), type_(Type::kSource), sources_(srcs) {
    dispatch = ctx_->dispatch;
  }

  Program(IntrusivePtr<Context> ctx, CmProgram *prog) :
      ctx_(ctx), type_(Type::kBinary),
      prog_(prog, [ctx](CmProgram *p) { ctx->dev_.device->DestroyProgram(p); }) {
    dispatch = ctx_->dispatch;
  }

  IntrusivePtr<Context> ctx_;

  Type type_;

  // kSource
  std::vector<std::string> sources_;

  // kObject
  std::string object_;

  // kBinary
  std::shared_ptr<CmProgram> prog_;
};

} // namespace cl
} // namespace shim

#endif // CM_EMU_SHIM_OCL_PROGRAM_H
