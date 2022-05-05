/*========================== begin_copyright_notice ============================

Copyright (C) 2020 Intel Corporation

SPDX-License-Identifier: MIT

============================= end_copyright_notice ===========================*/

#ifndef GUARD_common_type_queue_create_option_h
#define GUARD_common_type_queue_create_option_h

#include "type_queue_type.h"
#include "type_queue_priority.h"
#include "type_queue_sseu_usage_hint_type.h"

struct CM_QUEUE_CREATE_OPTION
{
    CM_QUEUE_TYPE                 QueueType               : 3;
    bool                          RAMode                  : 1;
    CM_QUEUE_PRIORITY             QueuePriority           : 3;
    bool                          UserGPUContext          : 1; // Is the user-provided GPU Context already created externally
    unsigned int                  GPUContext              : 8; // user-provided GPU Context ordinal
    CM_QUEUE_SSEU_USAGE_HINT_TYPE SseuUsageHint           : 3;

      unsigned int                reserved                : 1;

    unsigned int                  TileMask                : 8; // for XEHP_SDV, tile count is <= 4

};

//// clang-format off
inline constexpr CM_QUEUE_CREATE_OPTION CM_DEFAULT_QUEUE_CREATE_OPTION = {
        CM_QUEUE_TYPE_RENDER,
        false,
        CM_QUEUE_PRIORITY_NORMAL,
        false,
        0,
        CM_QUEUE_SSEU_USAGE_HINT_DEFAULT,
        0,
        0,
};
//// clang-format on

#endif // GUARD_common_type_queue_create_option_h
