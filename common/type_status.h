/*========================== begin_copyright_notice ============================

Copyright (C) 2020 Intel Corporation

SPDX-License-Identifier: MIT

============================= end_copyright_notice ===========================*/

#ifndef GUARD_common_type_status_h
#define GUARD_common_type_status_h
enum CM_STATUS
{
    CM_STATUS_QUEUED = 0,
    CM_STATUS_FLUSHED = 1,
    CM_STATUS_FINISHED = 2,
    CM_STATUS_STARTED = 3,
    CM_STATUS_RESET = 4
};
#endif // GUARD_common_type_status_h
