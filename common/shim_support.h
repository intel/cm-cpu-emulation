/*========================== begin_copyright_notice ============================

Copyright (C) 2019 Intel Corporation

SPDX-License-Identifier: MIT

============================= end_copyright_notice ===========================*/

#pragma once

#include "shim_os_defines.h"

namespace shim
{

template<class T>
constexpr const char*
get_func_signature()
{
#ifdef _WIN32
    return __FUNCSIG__;
#else
    return __PRETTY_FUNCTION__;
#endif
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
