/*========================== begin_copyright_notice ============================

Copyright (C) 2020 Intel Corporation

SPDX-License-Identifier: MIT

============================= end_copyright_notice ===========================*/

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
    PLATFORM_INTEL_DG1         = 20, //DG1
    PLATFORM_INTEL_DG2         = 22, //DG2
};

#define PLATFORM_INTEL_PVC 18
#define PLATFORM_INTEL_XEHP_SDV 14

#endif // GUARD_common_type_gpu_platform_h
