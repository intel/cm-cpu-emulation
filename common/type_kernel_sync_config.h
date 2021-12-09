/*========================== begin_copyright_notice ============================

Copyright (C) 2020 Intel Corporation

SPDX-License-Identifier: MIT

============================= end_copyright_notice ===========================*/

#ifndef GUARD_common_type_kernel_sync_config_h
#define GUARD_common_type_kernel_sync_config_h
struct CM_KERNEL_SYNC_CONFIG {
    bool     dataCacheFlush   : 1; // true: cache will be flushed;
    uint32_t reserved         : 31;
};
#endif // GUARD_common_type_kernel_sync_config_h
