/*========================== begin_copyright_notice ============================

Copyright (C) 2017 Intel Corporation

SPDX-License-Identifier: MIT

============================= end_copyright_notice ===========================*/

#ifndef CMRTLIB____SHARE_CM_PRIV_DEF_H_
#define CMRTLIB____SHARE_CM_PRIV_DEF_H_

#define CM_ALIGN_CEIL(_a, _alignment) (((_a) + ((_alignment)-1)) & (~((_alignment)-1)))

// 2047x2047 (max TS size) * 256 bytes(color bits)  per walker, platform GEN11+(gen11+)
#define CM_THREADSPACE_MAX_COLOR_COUNT_GEN11_PLUS  256
#define CM_MAX_USER_THREADS_PER_MEDIA_WALKER_GEN11_PLUS (CM_MAX_THREADSPACE_WIDTH_SKLUP_FOR_MW * CM_MAX_THREADSPACE_HEIGHT_SKLUP_FOR_MW * CM_THREADSPACE_MAX_COLOR_COUNT_GEN11_PLUS)

#include "emu_platform.h"

#include "type_gpu_platform.h"
#include "type_gpu_gt_platform.h"

#define CM_INIT_PROGRAM_COUNT 16
#define CM_INIT_KERNEL_COUNT 64
#define CM_INIT_EVENT_COUNT 128

// max number of arguments in kernel signature
// can be either per kernel arg or per thread arg
#define CM_MAX_ARG_COUNT 255

// 512x512 (max TS size) * 64 bytes (sizeof(MO cmd) + 10DW) = 16 MB (max allocated for BB)
#define CM_MAX_USER_THREADS_PER_TASK 262144

// 512x512 (max TS size) * 64 bytes (sizeof(MO cmd) + 10DW) = 16 MB (max allocated for BB)
#define CM_MAX_USER_THREADS_PER_TASK_NO_THREADARG 262144

#define MAX_THREAD_SPACE_WIDTH_PERGROUP 64
#define MAX_THREAD_SPACE_HEIGHT_PERGROUP 64
#define MAX_THREAD_SPACE_WIDTH_PERGROUP_GEN12LP 64
#define MAX_THREAD_SPACE_HEIGHT_PERGROUP_GEN12LP 64
#define MAX_THREAD_SPACE_WIDTH_PERGROUP_XEHP_SDV 128
#define MAX_THREAD_SPACE_HEIGHT_PERGROUP_XEHP_SDV 128
#define CM_MAX_TASKS 4

#define CM_MAX_KERNELS_PER_TASK 64
#define CM_MAX_ENQUEUES_SINGLE_GSF 10
#define CM_MAX_MULFRM_KERNELS (CM_MAX_KERNELS_PER_TASK * CM_MAX_ENQUEUES_SINGLE_GSF)

#define CM_MAX_SAMPLER_8X8_TABLE_SIZE 2
#define CM_MAX_BUFFER_TABLE_SIZE 256
#define CM_MAX_2DSURFACE_TABLE_SIZE 256
#define CM_MAX_3DSURFACE_TABLE_SIZE 256
#define CM_MAX_2DSURFACEUP_TABLE_SIZE 512
#define CM_MAX_ARG_BYTE_PER_KERNEL 2016
#define CM_MAX_HW_THREADS 60
#define CM_KERNEL_BINARY_BLOCK_SIZE 65536
#define CM_MAX_KERNEL_BINARY_SIZE 262144

#define GT_RESERVED_INDEX_START 250
#define GT_RESERVED_INDEX_START_GEN9_PLUS 240

#define CM_GLOBAL_SURFACE_INDEX_START 243
#define CM_GLOBAL_SURFACE_INDEX_START_GEN9_PLUS 1

#define CM_NULL_SURFACE_BINDING_INDEX 0  // Reserve 0 for NULL surface

#define CM_NULL_SURFACE 0xFFFF

#define CM_MULFRM_INVALID_INDEX -1

#define CM_RT_CONFIG_NAME "CM_RT_CONFIG_NAME"
#define CM_RT_PRE_ARG "CM_RT_PRE_ARG"
#define CM_RT_POST_ARG "CM_RT_POST_ARG"
#define CM_RT_TILE_COUNT "CM_RT_TILE_COUNT"

#define CM_MAX_THREADSPACE_WIDTH_FOR_MW 511
#define CM_MAX_THREADSPACE_HEIGHT_FOR_MW 511
#define CM_THREADSPACE_MAX_COLOR_COUNT 16
// 511x511 (max TS size) * 16 bytes(color bits) per walker, platform gen7,gen7_5,gen8.
#define CM_MAX_USER_THREADS_PER_MEDIA_WALKER_PRE_SKL (CM_MAX_THREADSPACE_WIDTH_FOR_MW * CM_MAX_THREADSPACE_HEIGHT_FOR_MW * CM_THREADSPACE_MAX_COLOR_COUNT)

#define CM_MAX_THREADSPACE_WIDTH_SKLUP_FOR_MW  2047
#define CM_MAX_THREADSPACE_HEIGHT_SKLUP_FOR_MW 2047
#define CM_THREADSPACE_MAX_COLOR_COUNT_GEN11_PLUS 256
// 2047x2047 (max TS size) * 16 bytes(color bits) per walker, platform gen9 gen10.
#define CM_MAX_USER_THREADS_PER_MEDIA_WALKER_SKL_PLUS (CM_MAX_THREADSPACE_WIDTH_SKLUP_FOR_MW * CM_MAX_THREADSPACE_HEIGHT_SKLUP_FOR_MW * CM_THREADSPACE_MAX_COLOR_COUNT)
// 2047x2047 (max TS size) * 256 bytes(color bits) per walker, platform gen11+.
#define CM_MAX_USER_THREADS_PER_MEDIA_WALKER_GEN11_PLUS (CM_MAX_THREADSPACE_WIDTH_SKLUP_FOR_MW * CM_MAX_THREADSPACE_HEIGHT_SKLUP_FOR_MW * CM_THREADSPACE_MAX_COLOR_COUNT_GEN11_PLUS)

// to define frame type for interlace frame support
#include "type_frame.h"

// Need to consistant with compiler
enum CM_ARG_KIND
{
    // compiler-defined kind
    ARG_KIND_GENERAL = 0x0,
    //ARG_KIND_SURFACE = 0x2, compiler value for surface
    // runtime classify further surface to 1D/2D/3D
    ARG_KIND_SURFACE_2D = 0x2,
    ARG_KIND_SURFACE_1D = 0x3,
    ARG_KIND_SURFACE_3D = 0x4,
    ARG_KIND_SURFACE_2D_UP = 0x7,
    ARG_KIND_SURFACE = 0xb,
    ARG_KIND_STATE_BUFFER = 0x11,
    //Implicit arguments, supported starting from vISA3.3
    ARG_KIND_GENERAL_DEPVEC = 0x20,    //bit 7~3 == 4
    ARG_KIND_SURFACE_2D_SCOREBOARD = 0x2A,  //bit 7~3 == 5
    ARG_KIND_IMPLICT_LOCALSIZE = 0x2B,
    ARG_KIND_IMPLICT_GROUPSIZE = 0x2C,
    ARG_KIND_IMPLICIT_LOCALID = 0x2D
};

#endif  // #ifndef CMRTLIB____SHARE_CM_PRIV_DEF_H_
