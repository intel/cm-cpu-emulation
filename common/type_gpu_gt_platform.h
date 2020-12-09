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

#ifndef GUARD_common_type_gpu_gt_platform_h
#define GUARD_common_type_gpu_gt_platform_h

enum GPU_GT_PLATFORM
{
    PLATFORM_INTEL_GT_UNKNOWN  = 0,
    PLATFORM_INTEL_GT1         = 1,
    PLATFORM_INTEL_GT2         = 2,
    PLATFORM_INTEL_GT3         = 3,
    PLATFORM_INTEL_GT4         = 4,
    PLATFORM_INTEL_GTA         = 8,
    PLATFORM_INTEL_GTC         = 9,
    PLATFORM_INTEL_GT1_5       = 10,
    PLATFORM_INTEL_GTX         = 11
};

#endif // GUARD_common_type_gpu_gt_platform_h
