/*========================== begin_copyright_notice ============================

Copyright (C) 2020 Intel Corporation

SPDX-License-Identifier: MIT

============================= end_copyright_notice ===========================*/

#ifndef GUARD_common_type_return_code_h
#define GUARD_common_type_return_code_h

enum CM_RETURN_CODE
{
#define CM_INTERNAL_ERROR(ENUM) CM_INTERNAL_ERROR_##ENUM
    CM_SUCCESS                                  = 0,
    /*
     * RANGE -1 ~ -9999 FOR ERROR CODE
     */
    CM_FAILURE                                  = -1,
    CM_NOT_IMPLEMENTED                          = -2,
    CM_SURFACE_ALLOCATION_FAILURE               = -3,
    CM_OUT_OF_HOST_MEMORY                       = -4,
    CM_SURFACE_FORMAT_NOT_SUPPORTED             = -5,
    CM_EXCEED_SURFACE_AMOUNT                    = -6,
    CM_EXCEED_KERNEL_ARG_AMOUNT                 = -7,
    CM_EXCEED_KERNEL_ARG_SIZE_IN_BYTE           = -8,
    CM_INVALID_ARG_INDEX                        = -9,
    CM_INVALID_ARG_VALUE                        = -10,
    CM_INVALID_ARG_SIZE                         = -11,
    CM_INVALID_THREAD_INDEX                     = -12,
    CM_INVALID_WIDTH                            = -13,
    CM_INVALID_HEIGHT                           = -14,
    CM_INVALID_DEPTH                            = -15,
    CM_INVALID_COMMON_ISA                       = -16,
    CM_OPEN_VIDEO_DEVICE_FAILURE                = -17,
    CM_VIDEO_DEVICE_LOCKED                      = -18,  // Video device is already locked.
    CM_LOCK_VIDEO_DEVICE_FAILURE                = -19,  // Video device is not locked but can't be locked.
    CM_EXCEED_SAMPLER_AMOUNT                    = -20,
    CM_EXCEED_MAX_KERNEL_PER_ENQUEUE            = -21,
    CM_EXCEED_MAX_KERNEL_SIZE_IN_BYTE           = -22,
    CM_EXCEED_MAX_THREAD_AMOUNT_PER_ENQUEUE     = -23,
    CM_INTERNAL_ERROR(24)                       = -24,
    CM_INVALID_THREAD_SPACE                     = -25,
    CM_EXCEED_MAX_TIMEOUT                       = -26,
    CM_JITDLL_LOAD_FAILURE                      = -27,
    CM_JIT_COMPILE_FAILURE                      = -28,
    CM_INTERNAL_ERROR(29)                       = -29,
    CM_INVALID_THREAD_GROUP_SPACE               = -30,
    CM_THREAD_ARG_NOT_ALLOWED                   = -31,
    CM_INVALID_GLOBAL_BUFFER_INDEX              = -32,
    CM_INVALID_BUFFER_HANDLER                   = -33,
    CM_EXCEED_MAX_SLM_SIZE                      = -34,
    CM_JITDLL_OLDER_THAN_ISA                    = -35,
    CM_INVALID_HARDWARE_THREAD_NUMBER           = -36,
    CM_INTERNAL_ERROR(37)                       = -37,
    CM_INVALIDE_L3_CONFIGURATION                = -38,
    CM_INVALID_TEXTURE2D_USAGE                  = -39,
    CM_INTEL_GFX_NOTFOUND                       = -40,
    CM_GPUCOPY_INVALID_SYSMEM                   = -41,
    CM_GPUCOPY_INVALID_WIDTH                    = -42,
    CM_GPUCOPY_INVALID_STRIDE                   = -43,
    CM_EVENT_DRIVEN_FAILURE                     = -44,
    CM_LOCK_SURFACE_FAIL                        = -45, // Lock surface failed
    CM_INVALID_GENX_BINARY                      = -46,
    CM_FEATURE_NOT_SUPPORTED_IN_DRIVER          = -47, // driver out-of-sync
    CM_QUERY_DLL_VERSION_FAILURE                = -48, //Fail in getting DLL file version
    CM_KERNELPAYLOAD_PERTHREADARG_MUTEX_FAIL    = -49,
    CM_KERNELPAYLOAD_PERKERNELARG_MUTEX_FAIL    = -50,
    CM_KERNELPAYLOAD_SETTING_FAILURE            = -51,
    CM_KERNELPAYLOAD_SURFACE_INVALID_BTINDEX    = -52,
    CM_NOT_SET_KERNEL_ARGUMENT                  = -53,
    CM_GPUCOPY_INVALID_SURFACES                 = -54,
    CM_GPUCOPY_INVALID_SIZE                     = -55,
    CM_GPUCOPY_OUT_OF_RESOURCE                  = -56,
    CM_INVALID_VIDEO_DEVICE                     = -57,
    CM_SURFACE_DELAY_DESTROY                    = -58,
    CM_INTERNAL_ERROR(59)                       = -59,
    CM_INTERNAL_ERROR(60)                       = -60,
    CM_FEATURE_NOT_SUPPORTED_BY_HARDWARE        = -61,
    CM_RESOURCE_USAGE_NOT_SUPPORT_READWRITE     = -62,
    CM_MULTIPLE_MIPLEVELS_NOT_SUPPORTED         = -63,
    CM_INVALID_UMD_CONTEXT                      = -64,
    CM_INVALID_LIBVA_SURFACE                    = -65,
    CM_INVALID_LIBVA_INITIALIZE                 = -66,
    CM_KERNEL_THREADSPACE_NOT_SET               = -67,
    CM_INVALID_KERNEL_THREADSPACE               = -68,
    CM_KERNEL_THREADSPACE_THREADS_NOT_ASSOCIATED= -69,
    CM_KERNEL_THREADSPACE_INTEGRITY_FAILED      = -70,
    CM_INVALID_USERPROVIDED_GENBINARY           = -71,
    CM_INVALID_PRIVATE_DATA                     = -72,
    CM_INVALID_MOS_RESOURCE_HANDLE              = -73,
    CM_SURFACE_CACHED                           = -74,
    CM_SURFACE_IN_USE                           = -75,
    CM_INVALID_GPUCOPY_KERNEL                   = -76,
    CM_INVALID_DEPENDENCY_WITH_WALKING_PATTERN  = -77,
    CM_INVALID_MEDIA_WALKING_PATTERN            = -78,
    CM_FAILED_TO_ALLOCATE_SVM_BUFFER            = -79,
    CM_EXCEED_MAX_POWER_OPTION_FOR_PLATFORM     = -80,
    CM_INVALID_KERNEL_THREADGROUPSPACE          = -81,
    CM_INVALID_KERNEL_SPILL_CODE                = -82,
    CM_UMD_DRIVER_NOT_SUPPORTED                 = -83,
    CM_INVALID_GPU_FREQUENCY_VALUE              = -84,
    CM_SYSTEM_MEMORY_NOT_4KPAGE_ALIGNED         = -85,
    CM_KERNEL_ARG_SETTING_FAILED                = -86,
    CM_NO_AVAILABLE_SURFACE                     = -87,
    CM_VA_SURFACE_NOT_SUPPORTED                 = -88,
    CM_TOO_MUCH_THREADS                         = -89,
    CM_NULL_POINTER                             = -90,
    CM_EXCEED_MAX_NUM_2D_ALIASES                = -91,
    CM_INVALID_PARAM_SIZE                       = -92,
    CM_GT_UNSUPPORTED                           = -93,
    CM_INTERNAL_ERROR(94)                       = -94,
    CM_PLATFORM_UNSUPPORTED_FOR_API             = -95,
    CM_TASK_MEDIA_RESET                         = -96,
    CM_KERNELPAYLOAD_SAMPLER_INVALID_BTINDEX    = -97,
    CM_EXCEED_MAX_NUM_BUFFER_ALIASES            = -98,
    CM_SYSTEM_MEMORY_NOT_4PIXELS_ALIGNED        = -99,
    CM_FAILED_TO_CREATE_CURBE_SURFACE           = -100,
    CM_INVALID_CAP_NAME                         = -101,
    CM_INVALID_PARAM_FOR_CREATE_QUEUE_EX        = -102,
    CM_INVALID_CREATE_OPTION_FOR_BUFFER_STATELESS = -103,
    CM_INVALID_KERNEL_ARG_POINTER                 = -104,
    CM_LOAD_LIBRARY_FAILED                        = -105,
    CM_NO_SUPPORTED_ADAPTER                       = -106,

    /*
     * RANGE -10000 ~ -19999 FOR INTERNAL ERROR CODE
     */
    CM_INTERNAL_ERROR_CODE_OFFSET               = -10000,

    /*
     * RANGE <=-20000 AREAD FOR MOST STATUS CONVERSION
     */
    CM_MOS_STATUS_CONVERTED_CODE_OFFSET         = -20000
#undef CM_INTERNAL_ERROR
};
#endif // GUARD_common_type_return_code_h
