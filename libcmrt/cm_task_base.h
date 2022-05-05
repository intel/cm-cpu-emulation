/*========================== begin_copyright_notice ============================

Copyright (C) 2017 Intel Corporation

SPDX-License-Identifier: MIT

============================= end_copyright_notice ===========================*/

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

// EU thread scheduling mode override for PVC
#define EU_THREAD_SCHEDULING_MODE_OVERRIDE_DEFAULT 0
#define EU_THREAD_SCHEDULING_MODE_OVERRIDE_OLEST_FIRST 1
#define EU_THREAD_SCHEDULING_MODE_OVERRIDE_ROUND_ROBIN 2
#define EU_THREAD_SCHEDULING_MODE_OVERRIDE_STALL_ROUND_ROBIN 3

// Thread execution arbitration mode for skl+ except for PVC
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
