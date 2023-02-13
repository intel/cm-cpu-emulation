/*========================== begin_copyright_notice ============================

Copyright (C) 2021 Intel Corporation

SPDX-License-Identifier: MIT

============================= end_copyright_notice ===========================*/

#include "program.h"

#include <fstream>
#include <iterator>
#include <sstream>
#include <streambuf>

#include "intrusive_pointer.h"
#include "os_utils.h"
#include "runtime.h"

CL_API_ENTRY cl_program CL_API_CALL SHIM_CALL(clCreateProgramWithSource)(
    cl_context context, cl_uint count, const char **strings,
    const size_t *lengths, cl_int *errcode_ret) CL_API_SUFFIX__VERSION_1_0 {
  shim::IntrusivePtr<shim::cl::Context> ctx(
      static_cast<shim::cl::Context *>(context));

  ERRCODE(CL_SUCCESS);

  if (!count || !strings) {
    ERRCODE(CL_INVALID_VALUE);
    return nullptr;
  }

  std::vector<std::string> srcs;
  try {
    std::transform(strings, strings + count, std::back_inserter(srcs),
                   [](const char *src) {
                     if (!src) {
                       throw CL_INVALID_VALUE;
                     }
                     return std::string(src);
                   });
  } catch (cl_int &e) {
    ERRCODE(e);
    return nullptr;
  }

  shim::IntrusivePtr<shim::cl::Program> prog =
      new (std::nothrow) shim::cl::Program(ctx, std::move(srcs));

  if (!prog) {
    ERRCODE(CL_OUT_OF_HOST_MEMORY);
    return nullptr;
  }

  IntrusivePtrAddRef(prog.get());
  return prog.get();
}

CL_API_ENTRY cl_program CL_API_CALL SHIM_CALL(clCreateProgramWithBinary)(
    cl_context context, cl_uint num_devices, const cl_device_id *device_list,
    const size_t *lengths, const unsigned char **binaries,
    cl_int *binary_status, cl_int *errcode_ret) CL_API_SUFFIX__VERSION_1_0 {
  shim::IntrusivePtr<shim::cl::Context> ctx(
      static_cast<shim::cl::Context *>(context));

  ERRCODE(CL_SUCCESS);
  if (binary_status)
    binary_status[0] = CL_INVALID_VALUE;

  if (num_devices != 1 || !device_list) {
    ERRCODE(CL_INVALID_VALUE);
    return nullptr;
  }

  if (device_list[0] != &ctx->dev_) {
    ERRCODE(CL_INVALID_DEVICE);
    return nullptr;
  }

  if (!lengths || !binaries || !lengths[0] || !binaries[0]) {
    ERRCODE(CL_INVALID_VALUE);
    return nullptr;
  }

  CmProgramEmu *p = nullptr;
  if (CmProgramEmu::Create(ctx->dev_.device.get(), p,
                           const_cast<unsigned char *>(binaries[0]),
                           lengths[0])) {
    ERRCODE(CL_INVALID_BINARY);
    return nullptr;
  }

  shim::IntrusivePtr<CmProgramEmu> prg(p, false);
  shim::IntrusivePtr<shim::cl::Program> prog =
      new (std::nothrow) shim::cl::Program(ctx, prg);

  if (!prog) {
    ERRCODE(CL_OUT_OF_HOST_MEMORY);
  }

  IntrusivePtrAddRef(prog.get());

  if (binary_status)
    binary_status[0] = CL_SUCCESS;

  return prog.get();
}

CL_API_ENTRY cl_program CL_API_CALL
SHIM_CALL(clCreateProgramWithBuiltInKernels)(
    cl_context context, cl_uint num_devices, const cl_device_id *device_list,
    const char *kernel_names, cl_int *errcode_ret) CL_API_SUFFIX__VERSION_1_2 {
  ERRCODE(CL_INVALID_VALUE);
  return nullptr;
}

CL_API_ENTRY cl_program CL_API_CALL SHIM_CALL(clCreateProgramWithIL)(
    cl_context context, const void *il, size_t length,
    cl_int *errcode_ret) CL_API_SUFFIX__VERSION_2_1 {
  ERRCODE(CL_INVALID_OPERATION);
  return nullptr;
}

CL_API_ENTRY cl_int CL_API_CALL SHIM_CALL(clRetainProgram)(cl_program program)
    CL_API_SUFFIX__VERSION_1_0 {
  shim::IntrusivePtr<shim::cl::Program> prog(
      static_cast<shim::cl::Program *>(program));
  IntrusivePtrAddRef(prog.get());
  return CL_SUCCESS;
}

CL_API_ENTRY cl_int CL_API_CALL SHIM_CALL(clReleaseProgram)(cl_program program)
    CL_API_SUFFIX__VERSION_1_0 {
  shim::IntrusivePtr<shim::cl::Program> prog(
      static_cast<shim::cl::Program *>(program), false);
  return CL_SUCCESS;
}

CL_API_ENTRY cl_int CL_API_CALL SHIM_CALL(clBuildProgram)(
    cl_program program, cl_uint num_devices, const cl_device_id *device_list,
    const char *options,
    void(CL_CALLBACK *pfn_notify)(cl_program program, void *user_data),
    void *user_data) CL_API_SUFFIX__VERSION_1_0 {
  shim::IntrusivePtr<shim::cl::Program> prog(
      static_cast<shim::cl::Program *>(program));

  if (prog->prog_) {
    if (pfn_notify) {
      pfn_notify(program, user_data);
    }
    return CL_SUCCESS;
  }

  if (auto status = SHIM_CALL(clCompileProgram)(
          program, num_devices, device_list, options, 0, nullptr, nullptr,
          nullptr, nullptr);
      status != CL_SUCCESS) {
    if (status == CL_COMPILE_PROGRAM_FAILURE) {
      return CL_BUILD_PROGRAM_FAILURE;
    }
    return status;
  }

  cl_int status = CL_SUCCESS;

  cl_device_id devlist[] = {&prog->ctx_->dev_};
  cl_program proglist[] = {program};

  shim::IntrusivePtr<shim::cl::Program> linked(
      static_cast<shim::cl::Program *>(
          SHIM_CALL(clLinkProgram)(prog->ctx_.get(), 1, devlist, nullptr, 1,
                                   proglist, pfn_notify, user_data, &status)),
      false);

  if (status == CL_LINK_PROGRAM_FAILURE) {
    return CL_BUILD_PROGRAM_FAILURE;
  }
  if (status != CL_SUCCESS) {
    return status;
  }

  prog->prog_ = linked->prog_;

  return CL_SUCCESS;
}

CL_API_ENTRY cl_int CL_API_CALL SHIM_CALL(clCompileProgram)(
    cl_program program, cl_uint num_devices, const cl_device_id *device_list,
    const char *options, cl_uint num_input_headers,
    const cl_program *input_headers, const char **header_include_names,
    void(CL_CALLBACK *pfn_notify)(cl_program program, void *user_data),
    void *user_data) CL_API_SUFFIX__VERSION_1_2 {
#if defined(_WIN32)
  // Online compilation is not implemented on Windows
  return CL_COMPILER_NOT_AVAILABLE;
#else  // defined(_WIN32)
  using namespace std::literals;
  shim::IntrusivePtr<shim::cl::Program> prog(
      static_cast<shim::cl::Program *>(program));

  if (num_devices != 1 || !device_list) {
    return CL_INVALID_VALUE;
  }

  auto &dev = prog->ctx_->dev_;
  if (device_list[0] != &dev) {
    return CL_INVALID_DEVICE;
  }

  if (prog->type_ != shim::cl::Program::Type::kSource) {
    return CL_INVALID_PROGRAM;
  }

  if (num_input_headers != 0) {
    return CL_INVALID_VALUE;
  }

  std::string compiler = dev.CompilerCommand();

  if (!options) {
    return CL_COMPILE_PROGRAM_FAILURE;
  }

  std::cerr << "options: " << options << std::endl;

  std::string user_flags(options);
  if (user_flags.find("-cmc") == 0) {
    user_flags.erase(0, 4);
  } else {
    return CL_COMPILE_PROGRAM_FAILURE;
  }

  // filtering out CMC specific options: -Qxcm* -mCM* -fcm* -vc*
  for (const auto& o : { "-Qxcm", "-mCM", "-fcm", "-vc" }) {
    const auto found = user_flags.find(o);
    if (found != std::string::npos) {
      user_flags.erase(found, user_flags.find(' ', found));
    }
  }

  unsigned char dummy = '\0';
  // Create a temporary file for source code
  auto srcfile = os::CreateTempFile(&dummy, sizeof(dummy));

  // Create a temporary file for a compiled object
  auto objfile = os::CreateTempFile(&dummy, sizeof(dummy));

  compiler += " "sv;
  compiler += user_flags;
  compiler += " -o ";
  compiler += objfile;
  compiler += " ";
  compiler += srcfile;

  std::cerr << compiler << std::endl;

  {
    std::ofstream out(srcfile);

    std::copy(std::begin(prog->sources_), std::end(prog->sources_),
              std::ostream_iterator<std::string>(out, "\n"));
  }

  if (std::system(compiler.c_str()) != 0) {
    return CL_COMPILE_PROGRAM_FAILURE;
  }

  prog->object_ = objfile;

  if (pfn_notify) {
    pfn_notify(program, user_data);
  }

  return CL_SUCCESS;
#endif // defined(_WIN32)
}

CL_API_ENTRY cl_program CL_API_CALL SHIM_CALL(clLinkProgram)(
    cl_context context, cl_uint num_devices, const cl_device_id *device_list,
    const char *options, cl_uint num_input_programs,
    const cl_program *input_programs,
    void(CL_CALLBACK *pfn_notify)(cl_program program, void *user_data),
    void *user_data, cl_int *errcode_ret) CL_API_SUFFIX__VERSION_1_2 {
#if defined(_WIN32)
  // Online compilation is not implemented on Windows
  ERRCODE(CL_LINKER_NOT_AVAILABLE);
  return nullptr;
#else  // defined(_WIN32)
  shim::IntrusivePtr<shim::cl::Context> ctx(
      static_cast<shim::cl::Context *>(context));
  ERRCODE(CL_SUCCESS);

  if (num_devices != 1 || !device_list) {
    ERRCODE(CL_INVALID_VALUE);
    return nullptr;
  }

  if (!num_input_programs) {
    ERRCODE(CL_INVALID_OPERATION);
    return nullptr;
  }

  std::string linker = ctx->dev_.LinkerCommand();
  if (options) {
    linker += options;
  }

  std::ostringstream files;
  std::transform(input_programs, input_programs + num_input_programs,
                 std::ostream_iterator<std::string>(files, " "),
                 [&files](cl_program p) {
                   shim::IntrusivePtr<shim::cl::Program> prog(
                       static_cast<shim::cl::Program *>(p));
                   return prog->object_;
                 });

  unsigned char dummy = '\0';
  auto exefile = os::CreateTempFile(&dummy, sizeof(dummy));

  linker += " ";
  linker += files.str();
  linker += " -o ";
  linker += exefile;

  std::cerr << linker << std::endl;

  if (std::system(linker.c_str()) != 0) {
    ERRCODE(CL_LINK_PROGRAM_FAILURE);
    return nullptr;
  }

  std::ifstream in(exefile, std::ios::binary);
  in.exceptions(std::ios::failbit | std::ios::badbit);

  std::vector<unsigned char> blob;

  in.seekg(0, std::ios::end);
  blob.reserve(in.tellg());
  in.seekg(0, std::ios::beg);

  blob.assign(std::istreambuf_iterator<char>(in),
              std::istreambuf_iterator<char>());

  CmProgramEmu *p = nullptr;
  if (CmProgramEmu::Create(ctx->dev_.device.get(), p, blob.data(),
                           blob.size())) {
    ERRCODE(CL_INVALID_BINARY);
    return nullptr;
  }

  shim::IntrusivePtr<CmProgramEmu> prg(p, false);
  shim::IntrusivePtr<shim::cl::Program> prog =
      new (std::nothrow) shim::cl::Program(ctx, prg);

  if (!prog) {
    ERRCODE(CL_OUT_OF_HOST_MEMORY);
    return nullptr;
  }

  IntrusivePtrAddRef(prog.get());

  if (pfn_notify) {
    pfn_notify(prog.get(), user_data);
  }

  return prog.get();
#endif // defined(_WIN32)
}

CL_API_ENTRY CL_API_PREFIX__VERSION_2_2_DEPRECATED cl_int CL_API_CALL
SHIM_CALL(clSetProgramReleaseCallback)(
    cl_program program,
    void(CL_CALLBACK *pfn_notify)(cl_program program, void *user_data),
    void *user_data) {
  return CL_INVALID_OPERATION;
}

CL_API_ENTRY cl_int CL_API_CALL SHIM_CALL(clSetProgramSpecializationConstant)(
    cl_program program, cl_uint spec_id, size_t spec_size,
    const void *spec_value) CL_API_SUFFIX__VERSION_2_2 {
  return CL_INVALID_OPERATION;
}

CL_API_ENTRY cl_int CL_API_CALL SHIM_CALL(clUnloadPlatformCompiler)(
    cl_platform_id platform) CL_API_SUFFIX__VERSION_1_2 {
  return CL_SUCCESS;
}

CL_API_ENTRY cl_int CL_API_CALL SHIM_CALL(clGetProgramInfo)(
    cl_program program, cl_program_info param_name, size_t param_value_size,
    void *param_value,
    size_t *param_value_size_ret) CL_API_SUFFIX__VERSION_1_0 {
  using shim::cl::SetResult;
  shim::IntrusivePtr<shim::cl::Program> prog(
      static_cast<shim::cl::Program *>(program));

  switch (param_name) {
  case CL_PROGRAM_REFERENCE_COUNT:
    return SetResult<cl_uint>(prog->UseCount() - 1, param_value_size,
                              param_value, param_value_size_ret);
  case CL_PROGRAM_CONTEXT:
    return SetResult<cl_context>(prog->ctx_.get(), param_value_size,
                                 param_value, param_value_size_ret);
  case CL_PROGRAM_NUM_DEVICES:
    return SetResult<cl_uint>(1, param_value_size, param_value,
                              param_value_size_ret);
  case CL_PROGRAM_DEVICES:
    return SetResult<cl_device_id>(&(prog->ctx_->dev_), param_value_size,
                                   param_value, param_value_size_ret);
  case CL_PROGRAM_SOURCE: {
    std::stringstream out;
    std::copy(std::begin(prog->sources_), std::end(prog->sources_),
              std::ostream_iterator<std::string>(out, "\n"));
    std::string src = out.str();
    return SetResult(std::string_view(src), param_value_size, param_value,
                     param_value_size_ret);
  }
  case CL_PROGRAM_IL:
    if (param_value_size_ret)
      *param_value_size_ret = 0;
    return CL_SUCCESS;
  case CL_PROGRAM_BINARY_SIZES:
  case CL_PROGRAM_BINARIES:
  case CL_PROGRAM_NUM_KERNELS:
  case CL_PROGRAM_KERNEL_NAMES:
    return CL_INVALID_OPERATION;

  case CL_PROGRAM_SCOPE_GLOBAL_DTORS_PRESENT:
    return SetResult<cl_bool>(CL_FALSE, param_value_size, param_value,
                              param_value_size_ret);
  }
  return CL_INVALID_OPERATION;
}

CL_API_ENTRY cl_int CL_API_CALL SHIM_CALL(clGetProgramBuildInfo)(
    cl_program program, cl_device_id device, cl_program_build_info param_name,
    size_t param_value_size, void *param_value,
    size_t *param_value_size_ret) CL_API_SUFFIX__VERSION_1_0 {
  using namespace std::literals;
  using shim::cl::SetResult;

  shim::IntrusivePtr<shim::cl::Program> prog(
      static_cast<shim::cl::Program *>(program));

  if (device != &(prog->ctx_->dev_)) {
    return CL_INVALID_DEVICE;
  }

  switch (param_name) {
  case CL_PROGRAM_BUILD_STATUS:
    switch (prog->type_) {
    case shim::cl::Program::Type::kSource:
      return SetResult<cl_build_status>(CL_BUILD_NONE, param_value_size,
                                        param_value, param_value_size_ret);
    case shim::cl::Program::Type::kObject:
    case shim::cl::Program::Type::kBinary:
      return SetResult<cl_build_status>(CL_BUILD_SUCCESS, param_value_size,
                                        param_value, param_value_size_ret);
    }
  case CL_PROGRAM_BUILD_OPTIONS:
  case CL_PROGRAM_BUILD_LOG:
    return SetResult(""sv, param_value_size, param_value, param_value_size_ret);
  case CL_PROGRAM_BINARY_TYPE:
    switch (prog->type_) {
    case shim::cl::Program::Type::kSource:
      return SetResult<cl_program_binary_type>(CL_PROGRAM_BINARY_TYPE_NONE,
                                               param_value_size, param_value,
                                               param_value_size_ret);
    case shim::cl::Program::Type::kObject:
      return SetResult<cl_program_binary_type>(
          CL_PROGRAM_BINARY_TYPE_COMPILED_OBJECT, param_value_size, param_value,
          param_value_size_ret);
    case shim::cl::Program::Type::kBinary:
      return SetResult<cl_program_binary_type>(
          CL_PROGRAM_BINARY_TYPE_EXECUTABLE, param_value_size, param_value,
          param_value_size_ret);
    }
  case CL_PROGRAM_BUILD_GLOBAL_VARIABLE_TOTAL_SIZE:
    return SetResult<size_t>(0, param_value_size, param_value,
                             param_value_size_ret);
  }

  return CL_INVALID_OPERATION;
}

CL_API_ENTRY CL_API_PREFIX__VERSION_1_1_DEPRECATED cl_int CL_API_CALL
SHIM_CALL(clUnloadCompiler)(void) {
  return CL_SUCCESS;
}

extern "C" {
SHIM_EXPORT(clCreateProgramWithSource);
SHIM_EXPORT(clCreateProgramWithBinary);
SHIM_EXPORT(clCreateProgramWithBuiltInKernels);
SHIM_EXPORT(clCreateProgramWithIL);
SHIM_EXPORT(clRetainProgram);
SHIM_EXPORT(clReleaseProgram);
SHIM_EXPORT(clBuildProgram);
SHIM_EXPORT(clCompileProgram);
SHIM_EXPORT(clLinkProgram);
SHIM_EXPORT(clSetProgramReleaseCallback);
SHIM_EXPORT(clSetProgramSpecializationConstant);
SHIM_EXPORT(clUnloadPlatformCompiler);
SHIM_EXPORT(clGetProgramInfo);
SHIM_EXPORT(clGetProgramBuildInfo);
SHIM_EXPORT(clUnloadCompiler);
}
