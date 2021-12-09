/*========================== begin_copyright_notice ============================

Copyright (C) 2020 Intel Corporation

SPDX-License-Identifier: MIT

============================= end_copyright_notice ===========================*/

#ifndef GUARD_common_type_message_sequence_h
#define GUARD_common_type_message_sequence_h
typedef enum _CM_MESSAGE_SEQUENCE_
{
    CM_MS_1x1   = 0,
    CM_MS_16x1  = 1,
    CM_MS_16x4  = 2,
    CM_MS_32x1  = 3,
    CM_MS_32x4  = 4,
    CM_MS_64x1  = 5,
    CM_MS_64x4  = 6
}CM_MESSAGE_SEQUENCE;
#endif // GUARD_common_type_message_sequence_h
