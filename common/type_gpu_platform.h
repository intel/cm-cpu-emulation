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

#ifndef GUARD_common_type_gpu_platform_h
#define GUARD_common_type_gpu_platform_h

enum GPU_PLATFORM
{
    PLATFORM_INTEL_UNKNOWN     = 0,  //not known
    PLATFORM_INTEL_BDW         = 4,  //Broadwell
    PLATFORM_INTEL_SKL         = 7,  //Skylake
    PLATFORM_INTEL_BXT         = 8,  //Broxton
    PLATFORM_INTEL_KBL         = 11, //Kabylake
    PLATFORM_INTEL_ICLLP       = 13, //Icelake LP
    PLATFORM_INTEL_TGLLP       = 15, //TigerLakeLP
};

#define PLATFORM_INTEL_XEHP_SDV 14

#endif // GUARD_common_type_gpu_platform_h
