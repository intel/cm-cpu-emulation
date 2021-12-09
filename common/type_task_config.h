/*========================== begin_copyright_notice ============================

Copyright (C) 2020 Intel Corporation

SPDX-License-Identifier: MIT

============================= end_copyright_notice ===========================*/

#ifndef GUARD_common_type_task_config_h
#define GUARD_common_type_task_config_h

struct CM_TASK_CONFIG {
    // Enable or disable turbo-boost.
    bool turboBoostFlag: 1;

    // Whether threads will be dispatched in sets to fused EUs.
    bool fusedEuDispatchFlag: 1;

};

using _CM_TASK_CONFIG_EX = CM_TASK_CONFIG;

#endif // GUARD_common_type_task_config_h
