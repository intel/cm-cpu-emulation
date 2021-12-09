/*========================== begin_copyright_notice ============================

Copyright (C) 2017 Intel Corporation

SPDX-License-Identifier: MIT

============================= end_copyright_notice ===========================*/

#ifndef CMRTLIB_WINDOWS_SHARE_CM_DEVICE_BASE_H_
#define CMRTLIB_WINDOWS_SHARE_CM_DEVICE_BASE_H_

#ifndef CMRT_UMD_API
#include "cm_device_def.h"
#endif
#ifdef CM_DX11
#include "d3d11.h"
#else
#include "d3d9.h"
#include <dxva2api.h>
#endif

#include "type_device_base.h"

#endif  // #ifndef CMRTLIB_WINDOWS_SHARE_CM_DEVICE_BASE_H_
