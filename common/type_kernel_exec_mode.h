/*========================== begin_copyright_notice ============================

Copyright (C) 2020 Intel Corporation

SPDX-License-Identifier: MIT

============================= end_copyright_notice ===========================*/

#ifndef GUARD_common_type_kernel_exec_mode_h
#define GUARD_common_type_kernel_exec_mode_h
typedef enum _CM_KERNEL_EXEC_MODE {
    CM_KERNEL_EXECUTION_MODE_MONOPOLIZED =  0,
    CM_KERNEL_EXECUTION_MODE_CONCURRENT,
} CM_KERNEL_EXEC_MODE;

#endif // GUARD_common_type_kernel_exec_mode_h
