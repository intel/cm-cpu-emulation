/*========================== begin_copyright_notice ============================

Copyright (C) 2020 Intel Corporation

SPDX-License-Identifier: MIT

============================= end_copyright_notice ===========================*/

#ifndef GUARD_common_type_event_profiling_info_h
#define GUARD_common_type_event_profiling_info_h
enum CM_EVENT_PROFILING_INFO
{
    CM_EVENT_PROFILING_HWSTART,
    CM_EVENT_PROFILING_HWEND,
    CM_EVENT_PROFILING_SUBMIT,
    CM_EVENT_PROFILING_COMPLETE,
    CM_EVENT_PROFILING_ENQUEUE,
    CM_EVENT_PROFILING_KERNELCOUNT,
    CM_EVENT_PROFILING_KERNELNAMES,
    CM_EVENT_PROFILING_THREADSPACE,
    CM_EVENT_PROFILING_CALLBACK
};
#endif // GUARD_common_type_event_profiling_info_h
