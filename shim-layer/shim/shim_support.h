/*===================== begin_copyright_notice ==================================

 Copyright (c) 2020, Intel Corporation


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

#pragma once

#include "os_defines.h"

namespace shim
{

template<class T>
constexpr const char*
get_func_signature()
{
    return __PRETTY_FUNCTION__;
}

struct KernelDescriptor
{
  const char *name;
  const char *signature;
  void *func;
};

}

#define _UNIQUE_VARNAME(base, idx) base ## idx
#define UNIQUE_VARNAME(base, idx) _UNIQUE_VARNAME(base,idx)

#define EXPORT_SIGNATURE(kernel)                                          \
  extern "C"                                                              \
  SHIM_API_EXPORT                                                         \
  const shim::KernelDescriptor UNIQUE_VARNAME(__kernel_, __COUNTER__) = { \
    #kernel,                                                              \
    shim::get_func_signature<decltype(kernel)>(),                         \
    reinterpret_cast<void*>(&kernel)                                      \
  };

#define TEMPLATE_STRING(name, ...) #name "<" #__VA_ARGS__ ">"
#define EXPORT_TEMPLATE_SIGNATURE(kernel, ...)                            \
  extern "C"                                                              \
  SHIM_API_EXPORT                                                         \
  const shim::KernelDescriptor UNIQUE_VARNAME(__kernel_, __COUNTER__) = { \
    TEMPLATE_STRING(kernel, __VA_ARGS__),                                 \
    shim::get_func_signature<decltype(kernel<__VA_ARGS__>)>(),            \
    reinterpret_cast<void*>(&kernel<__VA_ARGS__>)                         \
  }

#ifdef _GENX_MAIN_
#undef _GENX_MAIN_
#endif /* _GENX_MAIN_ */
#define _GENX_MAIN_ SHIM_API_EXPORT
