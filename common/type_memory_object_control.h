/*========================== begin_copyright_notice ============================

Copyright (C) 2020 Intel Corporation

SPDX-License-Identifier: MIT

============================= end_copyright_notice ===========================*/

#ifndef GUARD_common_type_memory_object_control_h
#define GUARD_common_type_memory_object_control_h
enum MEMORY_OBJECT_CONTROL
{
    MEMORY_OBJECT_CONTROL_SKL_DEFAULT = 0,
    MEMORY_OBJECT_CONTROL_SKL_NO_L3,
    MEMORY_OBJECT_CONTROL_SKL_NO_LLC_ELLC,
    MEMORY_OBJECT_CONTROL_SKL_NO_LLC,
    MEMORY_OBJECT_CONTROL_SKL_NO_ELLC,
    MEMORY_OBJECT_CONTROL_SKL_NO_LLC_L3,
    MEMORY_OBJECT_CONTROL_SKL_NO_ELLC_L3,
    MEMORY_OBJECT_CONTROL_SKL_NO_CACHE,
    MEMORY_OBJECT_CONTROL_SKL_COUNT,

    MEMORY_OBJECT_CONTROL_XE_LP_DEFAULT = 0,
    MEMORY_OBJECT_CONTROL_XE_LP_L1_ENABLED,
    MEMORY_OBJECT_CONTROL_XE_LP_COUNT,

    MEMORY_OBJECT_CONTROL_UNKNOWN = 0xff
};

#endif // GUARD_common_type_memory_object_control_h
