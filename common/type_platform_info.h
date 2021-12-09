/*========================== begin_copyright_notice ============================

Copyright (C) 2020 Intel Corporation

SPDX-License-Identifier: MIT

============================= end_copyright_notice ===========================*/

#ifndef GUARD_common_type_platform_info_h
#define GUARD_common_type_platform_info_h
struct CM_PLATFORM_INFO
{
    uint32_t numSlices;
    uint32_t numSubSlices;
    uint32_t numEUsPerSubSlice;
    uint32_t numHWThreadsPerEU;
    uint32_t numMaxEUsPerPool;
};

#endif // GUARD_common_type_platform_info_h
