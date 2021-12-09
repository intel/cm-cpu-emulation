/*========================== begin_copyright_notice ============================

Copyright (C) 2020 Intel Corporation

SPDX-License-Identifier: MIT

============================= end_copyright_notice ===========================*/

#ifndef GUARD_common_type_exec_config_h
#define GUARD_common_type_exec_config_h
struct CM_EXECUTION_CONFIG {
    CM_KERNEL_EXEC_MODE kernelExecutionMode = CM_KERNEL_EXECUTION_MODE_MONOPOLIZED;
    int                 concurrentPolicy    = 0; // Reserve for future extension.
};

#endif // GUARD_common_type_exec_config_h
