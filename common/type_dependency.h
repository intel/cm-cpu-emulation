/*========================== begin_copyright_notice ============================

Copyright (C) 2020 Intel Corporation

SPDX-License-Identifier: MIT

============================= end_copyright_notice ===========================*/

#ifndef GUARD_common_type_dependency_h
#define GUARD_common_type_dependency_h
typedef struct _CM_DEPENDENCY {
    uint32_t    count;
    int32_t     deltaX[CM_MAX_DEPENDENCY_COUNT];
    int32_t     deltaY[CM_MAX_DEPENDENCY_COUNT];
} CM_DEPENDENCY;
#endif // GUARD_common_type_dependency_h
