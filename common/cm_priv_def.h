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

#ifndef CMRTLIB____SHARE_CM_PRIV_DEF_H_
#define CMRTLIB____SHARE_CM_PRIV_DEF_H_

#define CM_ALIGN_CEIL(_a, _alignment) (((_a) + ((_alignment)-1)) & (~((_alignment)-1)))

static const int skl_max_threads[5] = {0, 98, 161, 329, 497};
static const int bxt_max_threads[8] = {0, -1, -1, -1, -1, 108, 72, 144};
static const int kbl_max_threads[8] = { 0, -1, 161, 329 };
static const int icllp_max_threads[3] = {0, 224, 448};
static const int tgllp_max_threads[] = {0, 224, 672};
static const int xehp_sdv_max_threads[] = {0, 1024, 2048, 4096};

static const int skl_threads_per_eu[5] = {0, 7, 7, 7, 7};
static const int bxt_threads_per_eu[8] = {0, -1, -1, -1, -1, 6, 6, 6}; //gt1,gt2,gt3,gt4 not defined
static const int kbl_threads_per_eu[8] = { 0, -1, 7, 7}; //gt1,gt4 not defined
static const int icllp_threads_per_eu[3] = { 0, 7, 7};
static const int tgllp_threads_per_eu[] = {0, 7, 7};
static const int xehp_sdv_threads_per_eu[] = {0, 8, 8, 8};

static const int skl_eu_per_subslice[5] = {0, 8, 8, 8, 8};
static const int bxt_eu_per_subslice[8] = {0, -1, -1, -1, -1, 6, 6, 8}; //gt1,gt2,gt3,gt4 not defined
static const int kbl_eu_per_subslice[8] = { 0, -1, 8, 8 }; //gt1, gt4 not defined
static const int icllp_eu_per_subslice[3] = { 0, 8, 8};
static const int tgllp_eu_per_subslice[]  = {0, 16, 16};
static const int xehp_sdv_eu_per_subslice[]    = {0, 16, 16, 16};
static const int xehp_sdv_max_ccs_index[] = {0, 1, 2, 4};

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
