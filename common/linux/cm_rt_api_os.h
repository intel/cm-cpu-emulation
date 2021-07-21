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
