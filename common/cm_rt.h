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

//!
//! \file      cm_rt.h
//! \brief     Contains all exposed APIs and Definitions for CM
//!

#ifndef __CM_RT_H__
#define __CM_RT_H__

//**********************************************************************
// Version
//**********************************************************************
#include "cm_version_defs.h"


//**********************************************************************
// external headers
//**********************************************************************
#ifndef __SYCL_EXPLICIT_SIMD_PLUGIN__
#include <stdint.h>
#endif // __SYCL_EXPLICIT_SIMD_PLUGIN__

//**********************************************************************
// Macros
//**********************************************************************
#ifdef __cplusplus
#define EXTERN_C     extern "C"
#else
#define EXTERN_C
#endif
#define CM_RT_API
#define CM_KERNEL_FUNCTION(...) CM_KERNEL_FUNCTION2(__VA_ARGS__)

#define _NAME_MERGE_(x, y)                      x ## y
#define _NAME_LABEL_(name, id)                  _NAME_MERGE_(name, id)
#define __CODEGEN_UNIQUE(name)                  _NAME_LABEL_(name, __LINE__)
#define BITFIELD_RANGE( startbit, endbit )     ((endbit)-(startbit)+1)
#define BITFIELD_BIT(bit)                        1

#define CM_MIN_SURF_WIDTH   1
#define CM_MIN_SURF_HEIGHT  1
#define CM_MIN_SURF_DEPTH   2

#define CM_MAX_1D_SURF_WIDTH    0x80000000 // 2^31 2 GB

#define CM_MAX_2D_SURF_WIDTH    16384
#define CM_MAX_2D_SURF_HEIGHT   16384

#define CM_MAX_3D_SURF_WIDTH    2048
#define CM_MAX_3D_SURF_HEIGHT   2048
#define CM_MAX_3D_SURF_DEPTH    2048

#define CM_MAX_OPTION_SIZE_IN_BYTE          512
#define CM_MAX_KERNEL_NAME_SIZE_IN_BYTE     256
#define CM_MAX_ISA_FILE_NAME_SIZE_IN_BYTE   256

#define CM_MAX_THREADSPACE_WIDTH_FOR_MW        511
#define CM_MAX_THREADSPACE_HEIGHT_FOR_MW       511
#define CM_MAX_THREADSPACE_WIDTH_FOR_MO        512
#define CM_MAX_THREADSPACE_HEIGHT_FOR_MO       512
#define CM_MAX_THREADSPACE_WIDTH_SKLUP_FOR_MW  2047
#define CM_MAX_THREADSPACE_HEIGHT_SKLUP_FOR_MW 2047

#define CM_NO_EVENT  ((CmEvent *)(-1))  //NO Event

// Cm Device Create Option
#define CM_DEVICE_CREATE_OPTION_SCRATCH_SPACE_DISABLE       1
#define CM_DEVICE_CREATE_OPTION_TDR_DISABLE                 64

#define CM_DEVICE_CONFIG_DISABLE_TASKFLUSHEDSEMAPHORE_OFFSET  6
#define CM_DEVICE_CONFIG_DISABLE_TASKFLUSHEDSEMAPHORE_MASK   (1<<CM_DEVICE_CONFIG_DISABLE_TASKFLUSHEDSEMAPHORE_OFFSET)
#define CM_DEVICE_CREATE_OPTION_TASKFLUSHEDSEMAPHORE_DISABLE  1   //to disable the semaphore for task flushed
#define CM_DEVICE_CONFIG_MIDTHREADPREEMPTION_OFFSET           22
#define CM_DEVICE_CONFIG_MIDTHREADPREEMPTION_DISENABLE         (1 << CM_DEVICE_CONFIG_MIDTHREADPREEMPTION_OFFSET)
#define CM_DEVICE_CONFIG_KERNEL_DEBUG_OFFSET                  23
#define CM_DEVICE_CONFIG_KERNEL_DEBUG_ENABLE               (1 << CM_DEVICE_CONFIG_KERNEL_DEBUG_OFFSET)
#define CM_DEVICE_CONFIG_GPUCOPY_OFFSET                    29
#define CM_DEVICE_CONFIG_GPUCOPY_DISABLE                   (1 << CM_DEVICE_CONFIG_GPUCOPY_OFFSET)
#define CM_DEVICE_CONFIG_FAST_PATH_OFFSET                  30
#define CM_DEVICE_CONFIG_FAST_PATH_ENABLE                  (1 << CM_DEVICE_CONFIG_FAST_PATH_OFFSET)

#define CM_DEVICE_CREATE_OPTION_DEFAULT                    CM_DEVICE_CONFIG_FAST_PATH_ENABLE

#define CM_MAX_DEPENDENCY_COUNT         8
#define CM_NUM_DWORD_FOR_MW_PARAM       16

#define    CM_CHROMA_SITING_NONE           0,
#define    CM_CHROMA_SITING_HORZ_LEFT      1 << 0
#define    CM_CHROMA_SITING_HORZ_CENTER    1 << 1
#define    CM_CHROMA_SITING_HORZ_RIGHT     1 << 2
#define    CM_CHROMA_SITING_VERT_TOP       1 << 4
#define    CM_CHROMA_SITING_VERT_CENTER    1 << 5
#define    CM_CHROMA_SITING_VERT_BOTTOM    1 << 6

#define CM_NULL_SURFACE                     0xFFFF

#define  CM_FUSED_EU_DISABLE                 0
#define  CM_FUSED_EU_ENABLE                  1
#define  CM_FUSED_EU_DEFAULT                 CM_FUSED_EU_DISABLE

#define  CM_TURBO_BOOST_DISABLE               0
#define  CM_TURBO_BOOST_ENABLE                1
#define  CM_TURBO_BOOST_DEFAULT              CM_TURBO_BOOST_ENABLE

#define CM_CALLBACK __cdecl

// SVM buffer access flags definition
#define CM_SVM_ACCESS_FLAG_COARSE_GRAINED    (0)                                 //Coarse-grained SVM buffer, IA/GT cache coherency disabled
#define CM_SVM_ACCESS_FLAG_FINE_GRAINED      (1 << 0)                            //Fine-grained SVM buffer, IA/GT cache coherency enabled
#define CM_SVM_ACCESS_FLAG_ATOMICS           (1 << 1)                            //Crosse IA/GT atomics supported SVM buffer, need CM_SVM_ACCESS_FLAG_FINE_GRAINED flag is set as well
#define CM_SVM_ACCESS_FLAG_DEFAULT           CM_SVM_ACCESS_FLAG_COARSE_GRAINED   //default is coarse-grained SVM buffer


//**********************************************************************
// OS-specific includings and types
//**********************************************************************
#include "cm_rt_def_os.h"

//**********************************************************************
// Enumerations
//**********************************************************************
#include "type_return_code.h"
#include "type_status.h"
#include "type_pixel_type.h"
#include "type_gpu_platform.h"
#include "type_gpu_gt_platform.h"
#include "type_device_cap_name.h"
#include "type_fastcopy_option.h"
#include "type_dependency_pattern.h"
#include "type_walking_pattern.h"
#include "type_26zi_dispatch_pattern.h"
#include "type_mw_group_select.h"
/**************** L3/Cache ***************/
#include "type_memory_object_control.h"
#include "type_memory_type.h"
#include "type_l3_suggest_config.h"
#include "type_surface_address_control_mode.h"
#include "type_message_sequence.h"
#include "type_min_max_filter_ctrl.h"
#include "type_event_profiling_info.h"
// to define frame type for interlace frame support
#include "type_frame.h"

//**********************************************************************
// Structures
//**********************************************************************
#include "type_platform_info.h"
#include "type_walking_patterns.h"
#include "type_dependency.h"
#include "type_coord.h"
#include "type_thread_param.h"
#include "type_l3_config_register_values.h"

#include "type_surface_details.h"
#include "type_power_option.h"
#include "type_task_config.h"
#include "type_kernel_exec_mode.h"
#include "type_exec_config.h"
// parameters used to set the surface state of the buffer
#include "type_surface_mem_obj_ctrl.h"
#include "type_buffer_state_param.h"
#include "type_surface_2d_state_param.h"
#include "type_queue_create_option.h"
#include "type_cond_end_operator_code.h"
#include "type_cond_end_param.h"
#include "type_kernel_sync_config.h"

//**********************************************************************
// Classes
//**********************************************************************
#include "cm_index_types.h"

#include "type_event_base.h"
#include "type_kernel_base.h"
#include "type_task_base.h"
#include "type_buffer_base.h"
#include "type_buffer_up_base.h"
#include "type_buffer_svm_base.h"
#include "type_surface_2d_up_base.h"
#include "type_surface_3d_base.h"
#include "type_thread_space_base.h"
#include "type_queue_base.h"


//**********************************************************************
// Function pointer types
//**********************************************************************
typedef void (CM_CALLBACK *callback_function)(CmEvent*, void *);
typedef void (*IMG_WALKER_FUNTYPE)(void* img, void* arg);

//**********************************************************************
// OS-specific APIs and classes
//**********************************************************************
#include "cm_rt_api_os.h"

//**********************************************************************
// Functions declaration
//**********************************************************************
EXTERN_C CM_RT_API INT DestroyCmDevice(CmDevice* &device);
EXTERN_C CM_RT_API INT CMRT_Enqueue(CmQueue* queue, CmTask* task, CmEvent** event, const CmThreadSpace* threadSpace = nullptr);
EXTERN_C CM_RT_API const char* GetCmErrorString(int errCode);


#endif //__CM_RT_H__
