/*========================== begin_copyright_notice ============================

Copyright (C) 2020 Intel Corporation

SPDX-License-Identifier: MIT

============================= end_copyright_notice ===========================*/

#ifndef GUARD_common_type_walking_pattern_h
#define GUARD_common_type_walking_pattern_h
typedef enum _CM_WALKING_PATTERN {
    CM_WALK_DEFAULT         = 0,
    CM_WALK_WAVEFRONT       = 1,
    CM_WALK_WAVEFRONT26     = 2,
    CM_WALK_VERTICAL        = 3,
    CM_WALK_HORIZONTAL      = 4,
    CM_WALK_WAVEFRONT26X    = 5,
    CM_WALK_WAVEFRONT26ZIG  = 6,
    CM_WALK_WAVEFRONT45D    = 7,
    CM_WALK_WAVEFRONT45XD_2 = 8
} CM_WALKING_PATTERN;
#endif // GUARD_common_type_walking_pattern_h
