/*===================== begin_copyright_notice ==================================

 Copyright (c) 2020, Intel Corporation


 Permission is hereby granted, free of charge, to any person obtaining a
 copy of this software and associated documentation files (the "Software"),
 to deal in the Software without restriction, including without limitation
 the rights to use, copy, modify, merge, publish, distribute, sublicense,
 and/or sell copies of the Software, and to permit persons to whom the
 Software is furnished to do so, subject to the following conditions:

 The above copyright notice and this permission notice shall be included
 in all copies or substantial portions of the Software.

 THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
 OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
 THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR
 OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
 ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
 OTHER DEALINGS IN THE SOFTWARE.
======================= end_copyright_notice ==================================*/

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

};

//// clang-format off
inline constexpr CM_QUEUE_CREATE_OPTION CM_DEFAULT_QUEUE_CREATE_OPTION = {
        CM_QUEUE_TYPE_RENDER,
        false,
        CM_QUEUE_PRIORITY_NORMAL,
        false,
        0,
        CM_QUEUE_SSEU_USAGE_HINT_DEFAULT,
};
//// clang-format on

#endif // GUARD_common_type_queue_create_option_h
