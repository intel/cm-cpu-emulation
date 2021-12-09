/*========================== begin_copyright_notice ============================

Copyright (C) 2020 Intel Corporation

SPDX-License-Identifier: MIT

============================= end_copyright_notice ===========================*/

#ifndef GUARD_common_type_dependency_pattern_h
#define GUARD_common_type_dependency_pattern_h
typedef enum _CM_DEPENDENCY_PATTERN
{
    CM_NONE_DEPENDENCY          = 0,    //All threads run parallel, scanline dispatch
    CM_WAVEFRONT                = 1,
    CM_WAVEFRONT26              = 2,
    CM_VERTICAL_WAVE            = 3,
    CM_HORIZONTAL_WAVE          = 4,
    CM_WAVEFRONT26Z             = 5,
    CM_WAVEFRONT26X             = 6,
    CM_WAVEFRONT26ZIG           = 7,
    CM_WAVEFRONT26ZI            = 8
} CM_DEPENDENCY_PATTERN;

#endif // GUARD_common_type_dependency_pattern_h
