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

#include "shim.h"

#define ERRCODE(x)                                                             \
  if (errcode_ret)                                                             \
  *errcode_ret = x

#endif // CM_EMU_SHIM_OCL_OCL_H
