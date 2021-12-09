/*========================== begin_copyright_notice ============================

Copyright (C) 2020 Intel Corporation

SPDX-License-Identifier: MIT

============================= end_copyright_notice ===========================*/

#ifndef GUARD_common_type_frame_h
#define GUARD_common_type_frame_h

enum CM_FRAME_TYPE
{
    CM_FRAME,     // singe frame, not interlaced
    CM_TOP_FIELD,
    CM_BOTTOM_FIELD,
    MAX_FRAME_TYPE
} ;

#endif // GUARD_common_type_frame_h
