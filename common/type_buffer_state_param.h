/*========================== begin_copyright_notice ============================

Copyright (C) 2020 Intel Corporation

SPDX-License-Identifier: MIT

============================= end_copyright_notice ===========================*/

#ifndef GUARD_common_type_buffer_state_param_h
#define GUARD_common_type_buffer_state_param_h
// Parameters used to set the surface state of the buffer
struct CM_BUFFER_STATE_PARAM
{
    uint32_t uiSize;
    uint32_t uiBaseAddressOffset;
    CM_SURFACE_MEM_OBJ_CTRL mocs;
};
#endif // GUARD_common_type_buffer_state_param_h
