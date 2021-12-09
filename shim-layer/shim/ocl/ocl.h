/*========================== begin_copyright_notice ============================

Copyright (C) 2017 Intel Corporation

SPDX-License-Identifier: MIT

============================= end_copyright_notice ===========================*/

#ifndef CM_EMU_SHIM_OCL_OCL_H
#define CM_EMU_SHIM_OCL_OCL_H

#define CL_TARGET_OPENCL_VERSION 300
#include <CL/cl.h>
#include <CL/cl_ext.h>
#include <CL/cl_icd.h>

#include <cm_rt.h>

#if defined(_WIN32)
// Alias does not work in MSVC, using def file
# define ALIAS_X(name, aliasname)
#else // defined(_WIN32)
# define ALIAS_X(name, aliasname) \
  extern __typeof (name) aliasname __attribute__ ((alias (#name)))
#endif // defined(_WIN32)

#define ALIAS(name, aliasname) ALIAS_X(name, aliasname)

#define SHIM_CALL(x) shim_ ## x

#define SHIM_EXPORT(x) ALIAS(SHIM_CALL(x), x)

#define ERRCODE(x) if (errcode_ret) *errcode_ret = x

#endif // CM_EMU_SHIM_OCL_OCL_H
