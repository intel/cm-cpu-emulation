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

#ifndef GUARD_common_type_device_cap_name_h
#define GUARD_common_type_device_cap_name_h

typedef enum _CM_DEVICE_CAP_NAME {
  CAP_KERNEL_COUNT_PER_TASK,
  CAP_KERNEL_BINARY_SIZE,
  CAP_SAMPLER_COUNT ,
  CAP_SAMPLER_COUNT_PER_KERNEL,
  CAP_BUFFER_COUNT ,
  CAP_SURFACE2D_COUNT,
  CAP_SURFACE3D_COUNT,
  CAP_SURFACE_COUNT_PER_KERNEL,
  CAP_ARG_COUNT_PER_KERNEL,
  CAP_ARG_SIZE_PER_KERNEL ,
  CAP_USER_DEFINED_THREAD_COUNT_PER_TASK,
  CAP_HW_THREAD_COUNT,
  CAP_SURFACE2D_FORMAT_COUNT,
  CAP_SURFACE2D_FORMATS,
  CAP_SURFACE3D_FORMAT_COUNT,
  CAP_SURFACE3D_FORMATS,
  CAP_GPU_PLATFORM,
  CAP_GT_PLATFORM,
  CAP_MIN_FREQUENCY,
  CAP_MAX_FREQUENCY,
  CAP_L3_CONFIG,
  CAP_GPU_CURRENT_FREQUENCY,
  CAP_USER_DEFINED_THREAD_COUNT_PER_TASK_NO_THREAD_ARG,
  CAP_USER_DEFINED_THREAD_COUNT_PER_MEDIA_WALKER,
  CAP_USER_DEFINED_THREAD_COUNT_PER_THREAD_GROUP,
  CAP_SURFACE2DUP_COUNT,
  CAP_PLATFORM_INFO,
  CAP_MAX_BUFFER_SIZE,
  CAP_MAX_SUBDEV_COUNT //for app to retrieve the total count of sub devices
} CM_DEVICE_CAP_NAME;

#endif // GUARD_common_type_device_cap_name_h
