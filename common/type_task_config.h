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

    // Whether single dispatch CCS mode is enabled.
    bool singleSliceDispatchFlag: 1;

    // Whether large-GRF mode is enabled.
    bool enableLargeGrf: 1;

    uint32_t statelessCachePolicy : 3;

    // Specify the EU thread sheduling policy:
    //    For PVC: 0-default, 1-Oldest Fist, 2-Round Robin, 3-Stall based Round Robin
    //    For SKL+ except for PVC: 0-Age Based, 1-Round Robin, 2-Round Robin After Dependency
    uint32_t euThreadSchedulingPolicy : 2;

    uint32_t reserved_bits : 23;
    uint32_t reserved0;
    uint32_t reserved1;
    uint32_t reserved2;
};

using _CM_TASK_CONFIG_EX = CM_TASK_CONFIG;

#endif // GUARD_common_type_task_config_h
