/*========================== begin_copyright_notice ============================

Copyright (C) 2017 Intel Corporation

SPDX-License-Identifier: MIT

============================= end_copyright_notice ===========================*/

//!
//! \file      cm_rt_api_os.h
//! \brief     Contains Linux Specific APIs and Definitions for CM
//!

#ifndef __CM_RT_API_OS_H__
#define __CM_RT_API_OS_H__

#include "type_surface_2d_base.h"
#include "type_device_base.h"

EXTERN_C CM_RT_API INT CreateCmDevice(CmDevice* &device, UINT& version, VADisplay vaDisplay = nullptr);
EXTERN_C CM_RT_API INT CreateCmDeviceEx(CmDevice* &device, UINT& version, VADisplay vaDisplay, UINT DevCreateOption = CM_DEVICE_CREATE_OPTION_DEFAULT);

#endif //__CM_RT_API_OS_H__
