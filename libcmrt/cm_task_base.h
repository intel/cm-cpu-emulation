/*===================== begin_copyright_notice ==================================

 Copyright (c) 2021, Intel Corporation


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

#ifndef CMRTLIB____SHARE_CM_TASK_BASE_H_
#define CMRTLIB____SHARE_CM_TASK_BASE_H_

#include "cm_include.h"

class CmKernel;
class SurfaceIndex;

#include "type_power_option.h"

#define CM_TURBO_BOOST_DISABLE 0
#define CM_TURBO_BOOST_ENABLE  1
#define CM_TURBO_BOOST_DEFAULT CM_TURBO_BOOST_ENABLE

// currently maximum multiple GPGPU walker number = 2
#define MAXIMUM_GPGPUWALKER_NUM 2

// Thread execution arbitration mode for skl+
#define THREAD_EXECUTION_ARBITRATION_MODE_AGE_BASED 0
#define THREAD_EXECUTION_ARBITRATION_MODE_ROUND_ROBIN 1
#define THREAD_EXECUTION_ARBITRATION_MODE_ROUND_ROBIN_AFTER_DEPENDENCY 2

#include "type_task_config.h"
#include "type_kernel_sync_config.h"
#include "type_task_config.h"
#include "type_kernel_exec_mode.h"
#include "type_exec_config.h"
#include "type_cond_end_operator_code.h"
#include "type_cond_end_param.h"
#include "type_task_base.h"

#endif  // #ifndef CMRTLIB____SHARE_CM_TASK_BASE_H_
