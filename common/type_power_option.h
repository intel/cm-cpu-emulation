/*========================== begin_copyright_notice ============================

Copyright (C) 2020 Intel Corporation

SPDX-License-Identifier: MIT

============================= end_copyright_notice ===========================*/

#ifndef GUARD_common_type_power_option_h
#define GUARD_common_type_power_option_h

typedef struct _CM_POWER_OPTION {
    uint16_t nSlice;     // set number of slice to use: 0(default number), 1, 2...
    uint16_t nSubSlice;  // set number of subslice to use: 0(default number), 1, 2...
    uint16_t nEU;        // set number of EU to use: 0(default number), 1, 2...
} CM_POWER_OPTION, *PCM_POWER_OPTION;
#endif // GUARD_common_type_power_option_h
