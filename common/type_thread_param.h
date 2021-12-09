/*========================== begin_copyright_notice ============================

Copyright (C) 2020 Intel Corporation

SPDX-License-Identifier: MIT

============================= end_copyright_notice ===========================*/

#ifndef GUARD_common_type_thread_param_h
#define GUARD_common_type_thread_param_h
typedef struct _CM_THREAD_PARAM {
    CM_COORDINATE   scoreboardCoordinates;     //[X, Y] terms of the scoreboard values of the current thread.
    uint8_t         scoreboardColor;           // dependency color of the current thread.
    uint8_t         sliceDestinationSelect;    //select determines the slice of the current thread must be sent to.
    uint8_t         subSliceDestinationSelect;    //select determines the sub-slice of the current thread must be sent to.
} CM_THREAD_PARAM, *PCM_THREAD_PARAM;
#endif // GUARD_common_type_thread_param_h
