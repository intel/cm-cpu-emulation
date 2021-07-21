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

#ifndef CM_EMU_SHIM_OCL_RUNTIME_H
#define CM_EMU_SHIM_OCL_RUNTIME_H

#include <algorithm>
#include <array>
#include <iostream>
#include <memory>
#include <string_view>
#include <variant>

#define CL_TARGET_OPENCL_VERSION 300
#include <CL/cl.h>
#include <CL/cl_ext.h>
#include <CL/cl_icd.h>

#include <cm_rt.h>

#include "intrusive_pointer.h"
#include "platform.h"
#include "device.h"

namespace shim {
namespace cl {

template <class... Ts> struct overloaded : Ts... { using Ts::operator()...; };
template <class... Ts> overloaded(Ts...)->overloaded<Ts...>;

struct Error : public std::runtime_error {
  Error(const std::string &msg) : std::runtime_error(msg) {}
};

class Runtime {
 public:
  static Runtime &Instance();

  Platform platform;
  Device device;

 private:
  Runtime();
};

template <typename T>
cl_int SetResult(T &&value, size_t buffer_size, void *buffer, size_t *buffer_size_ret) {
  if (!buffer && !buffer_size_ret) {
    return CL_INVALID_VALUE;
  }

  if (buffer && buffer_size < sizeof(value)) {
    return CL_INVALID_VALUE;
  }

  if (buffer) {
    *static_cast<T*>(buffer) = value;
  }

  if (buffer_size_ret) {
    *buffer_size_ret = sizeof(value);
  }

  return CL_SUCCESS;
}

template <typename T, size_t N>
cl_int SetResult(const std::array<T, N> &value, size_t buffer_size,
                 void *buffer, size_t *buffer_size_ret) {
  if (!buffer && !buffer_size_ret) {
    return CL_INVALID_VALUE;
  }
  if (buffer && buffer_size < sizeof(value)) {
    return CL_INVALID_VALUE;
  }

  if (buffer) {
    std::copy(std::begin(value), std::end(value), static_cast<T*>(buffer));
  }

  if (buffer_size_ret) {
    *buffer_size_ret = sizeof(T) * N;
  }

  return CL_SUCCESS;
}

cl_int SetResult(std::string_view value, size_t buffer_size,
                 void *buffer, size_t *buffer_size_ret);

} // namespace cl
} // namespace shim

#endif // CM_EMU_SHIM_OCL_RUNTIME_H
