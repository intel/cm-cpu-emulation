/*========================== begin_copyright_notice ============================

Copyright (C) 2017 Intel Corporation

SPDX-License-Identifier: MIT

============================= end_copyright_notice ===========================*/

//!
//! Declaration of types and data stuctures used by implementations of CmDevice on various operating systems.
//!

#ifndef CMRTLIB_AGNOSTIC_SHARE_CM_DEVICE_DEF_H_
#define CMRTLIB_AGNOSTIC_SHARE_CM_DEVICE_DEF_H_

#include <cstdint>
#include "cm_queue_base.h"
#include "cm_def.h"

#include "type_device_cap_name.h"

typedef enum _CM_SUB_MAJOR_STEPPING {
  A = 0,
  B = 1,
  C = 2,
  D = 3,
  E = 4,
  F = 5,
  G = 6,
  H = 7,
  I = 8,
  J = 9,
  K = 10,
  L = 11,
  M = 12,
  N = 13,
  O = 14,
  P = 15,
  Q = 16,
  R = 17,
  S = 18,
  T = 19,
  U = 20,
  V = 21,
  W = 22,
  X = 23,
  Y = 24,
  Z = 25
} CM_SUB_MAJOR_STEPPING;

typedef int32_t  (__cdecl *ReleaseSurfaceCallback)(void *cmDevice, void *surface);

struct CmDeviceCreationParam
{
    uint32_t createOption;        // [in]  Dev create option
    ReleaseSurfaceCallback releaseSurfaceFunc;  // [in]  Function Pointer to free surface
    void *deviceHandleInUmd;      // [out] pointer to handle in driver
    uint32_t version;             // [out] the Cm version
    uint32_t driverStoreEnabled;  // [out] DriverStoreEnable flag
    int32_t returnValue;          // [out] the return value from CMRT@UMD
};

#define CM_MAX_SPILL_SIZE_IN_BYTE_PER_HW_THREAD 9216 // 9K

#define CM_MAX_SURFACE3D_FORMAT_COUNT 3

#define CM_RT_PLATFORM "CM_RT_PLATFORM"
#define CM_RT_SKU "CM_RT_SKU"
#define CM_RT_MAX_THREADS "CM_RT_MAX_THREADS"
#define CM_RT_AUBLOAD_OPTION "CM_RT_AUBLOAD_OPTION"

#define CM_RT_MULTIPLE_FRAMES "CM_RT_MULTIPLE_FRAMES"
#define CM_RT_STEPPING "CM_RT_STEPPING"

#define CM_DEVICE_CONFIG_FAST_PATH_OFFSET 30
#define CM_DEVICE_CONFIG_FAST_PATH_ENABLE (1 << CM_DEVICE_CONFIG_FAST_PATH_OFFSET)
// enable the fast path by default from cmrtlib
#define CM_DEVICE_CREATE_OPTION_DEFAULT   CM_DEVICE_CONFIG_FAST_PATH_ENABLE
#define IGFX_UNKNOWN_CORE 0

#include "type_platform_info.h"

enum CM_QUERY_TYPE
{
    CM_QUERY_VERSION,
    CM_QUERY_REG_HANDLE,
    CM_QUERY_MAX_VALUES,
    CM_QUERY_GPU,
    CM_QUERY_GT,
    CM_QUERY_MIN_RENDER_FREQ,
    CM_QUERY_MAX_RENDER_FREQ,
    CM_QUERY_STEP,
    CM_QUERY_GPU_FREQ,
    CM_QUERY_MAX_VALUES_EX,
    CM_QUERY_SURFACE2D_FORMAT_COUNT,
    CM_QUERY_SURFACE2D_FORMATS,
    CM_QUERY_PLATFORM_INFO
};

struct CM_QUERY_CAPS
{
    CM_QUERY_TYPE type;
    union
    {
        int32_t version;
        HANDLE hRegistration;
        CM_HAL_MAX_VALUES maxValues;
        CM_HAL_MAX_VALUES_EX maxValuesEx;
        uint32_t genCore;
        uint32_t genGT;
        uint32_t minRenderFreq;
        uint32_t maxRenderFreq;
        uint32_t genStepId;
        uint32_t gpuCurrentFreq;
        uint32_t surf2DCount;
        uint32_t *surf2DFormats;
        CM_PLATFORM_INFO platformInfo;
    };
};

// Dummy param for execute
struct CM_PARAMS
{
    uint32_t placeHolder;
};

enum CM_FUNCTION_ID
{
    CM_FN_RT_ULT                       = 0x900, // (This function code is only used to run ults for CM_RT@UMD)

    CM_FN_CREATECMDEVICE                   = 0x1000,
    CM_FN_DESTROYCMDEVICE                  = 0x1001,

    CM_FN_CMDEVICE_CREATEBUFFER            = 0x1100,
    CM_FN_CMDEVICE_DESTROYBUFFER           = 0x1101,
    CM_FN_CMDEVICE_CREATEBUFFERUP          = 0x1102,
    CM_FN_CMDEVICE_DESTROYBUFFERUP         = 0x1103,
    CM_FN_CMDEVICE_CREATESURFACE2D         = 0x1104,
    CM_FN_CMDEVICE_DESTROYSURFACE2D        = 0x1105,
    CM_FN_CMDEVICE_CREATESURFACE2DUP       = 0x1106,
    CM_FN_CMDEVICE_DESTROYSURFACE2DUP      = 0x1107,
    CM_FN_CMDEVICE_GETSURFACE2DINFO        = 0x1108,
    CM_FN_CMDEVICE_CREATESURFACE3D         = 0x1109,
    CM_FN_CMDEVICE_DESTROYSURFACE3D        = 0x110A,
    CM_FN_CMDEVICE_CREATEQUEUE             = 0x110B,
    CM_FN_CMDEVICE_LOADPROGRAM             = 0x110C,
    CM_FN_CMDEVICE_DESTROYPROGRAM          = 0x110D,
    CM_FN_CMDEVICE_CREATEKERNEL            = 0x110E,
    CM_FN_CMDEVICE_DESTROYKERNEL           = 0x110F,
    CM_FN_CMDEVICE_CREATETASK              = 0x1110,
    CM_FN_CMDEVICE_DESTROYTASK             = 0x1111,
    CM_FN_CMDEVICE_GETCAPS                 = 0x1112,
    CM_FN_CMDEVICE_SETCAPS                 = 0x1113,
    CM_FN_CMDEVICE_CREATETHREADSPACE       = 0x1114,
    CM_FN_CMDEVICE_DESTROYTHREADSPACE      = 0x1115,
    CM_FN_CMDEVICE_CREATETHREADGROUPSPACE  = 0x1116,
    CM_FN_CMDEVICE_DESTROYTHREADGROUPSPACE = 0x1117,
    CM_FN_CMDEVICE_SETL3CONFIG             = 0x1118,
    CM_FN_CMDEVICE_SETSUGGESTEDL3CONFIG    = 0x1119,
    CM_FN_CMDEVICE_INIT_PRINT_BUFFER       = 0x112C,
    CM_FN_CMDEVICE_CREATEBUFFERSVM          = 0x1131,
    CM_FN_CMDEVICE_DESTROYBUFFERSVM         = 0x1132,
    CM_FN_CMDEVICE_CLONEKERNEL             = 0x1137,
    CM_FN_CMDEVICE_CREATESURFACE2D_ALIAS   = 0x1138,
    CM_FN_CMDEVICE_CREATESURFACE2D_EX      = 0x113C,
    CM_FN_CMDEVICE_CREATEBUFFER_ALIAS      = 0x113D,
    CM_FN_CMDEVICE_GETVISAVERSION          = 0x1140,
    CM_FN_CMDEVICE_CREATEQUEUEEX           = 0x1141,
    CM_FN_CMDEVICE_FLUSH_PRINT_BUFFER      = 0x1142,
    CM_FN_CMQUEUE_ENQUEUE                  = 0x1500,
    CM_FN_CMQUEUE_DESTROYEVENT             = 0x1501,
    CM_FN_CMQUEUE_ENQUEUECOPY              = 0x1502,
    CM_FN_CMQUEUE_ENQUEUEWITHGROUP         = 0x1504,
    CM_FN_CMQUEUE_ENQUEUESURF2DINIT        = 0x1505,
    CM_FN_CMQUEUE_ENQUEUECOPY_V2V          = 0x1506,
    CM_FN_CMQUEUE_ENQUEUECOPY_L2L          = 0x1507,
    CM_FN_CMQUEUE_ENQUEUEWITHHINTS         = 0x1509,
    CM_FN_CMQUEUE_ENQUEUEFAST              = 0x150a,
    CM_FN_CMQUEUE_DESTROYEVENTFAST         = 0x150b,
    CM_FN_CMQUEUE_ENQUEUEWITHGROUPFAST     = 0x150c,
    CM_FN_CMQUEUE_ENQUEUECOPY_BUFFER       = 0x150d,

};

#endif  // #ifndef CMRTLIB_AGNOSTIC_SHARE_CM_DEVICE_DEF_H_
