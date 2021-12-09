/*========================== begin_copyright_notice ============================

Copyright (C) 2020 Intel Corporation

SPDX-License-Identifier: MIT

============================= end_copyright_notice ===========================*/

#ifndef GUARD_common_type_cond_end_param_h
#define GUARD_common_type_cond_end_param_h
struct CM_CONDITIONAL_END_PARAM {
    uint32_t opValue;
    CM_CONDITIONAL_END_OPERATOR_CODE  opCode;
    bool  opMask;
    bool  opLevel;
};
#endif // GUARD_common_type_cond_end_param_h
