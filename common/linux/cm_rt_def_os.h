/*========================== begin_copyright_notice ============================

Copyright (C) 2017 Intel Corporation

SPDX-License-Identifier: MIT

============================= end_copyright_notice ===========================*/

//!
//! \file      cm_rt_def_os.h
//! \brief     Contains linux-specific Definitions for CM
//!

#ifndef __CM_RT_DEF_OS_H__
#define __CM_RT_DEF_OS_H__

#include <time.h>
#include <va_stub.h>
#include <malloc.h>

#define _tmain main

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <math.h>
#include <string.h>
#include <sys/time.h>
#include <pthread.h>
#include <x86intrin.h>
#include <iostream>

#define EXTERN_C extern "C"

#ifndef CM_NOINLINE
    #define CM_NOINLINE __attribute__((noinline))
#endif

typedef int BOOL;
typedef char byte;
typedef unsigned char BYTE;
typedef unsigned int UINT32;
typedef UINT32 DWORD;
typedef int INT;
typedef unsigned int UINT;
typedef signed char INT8;
typedef unsigned char UINT8;
typedef signed short INT16;
typedef unsigned short UINT16;
typedef signed int INT32;
typedef signed long long INT64;
typedef unsigned long long UINT64;

typedef enum _VACMTEXTUREADDRESS {
    VACMTADDRESS_WRAP            = 1,
    VACMTADDRESS_MIRROR          = 2,
    VACMTADDRESS_CLAMP           = 3,
    VACMTADDRESS_BORDER          = 4,
    VACMTADDRESS_MIRRORONCE      = 5,

    VACMTADDRESS_FORCE_DWORD     = 0x7fffffff
} VACMTEXTUREADDRESS;

typedef enum _VACMTEXTUREFILTERTYPE {
    VACMTEXF_NONE            = 0,
    VACMTEXF_POINT           = 1,
    VACMTEXF_LINEAR          = 2,
    VACMTEXF_ANISOTROPIC     = 3,
    VACMTEXF_FLATCUBIC       = 4,
    VACMTEXF_GAUSSIANCUBIC   = 5,
    VACMTEXF_PYRAMIDALQUAD   = 6,
    VACMTEXF_GAUSSIANQUAD    = 7,
    VACMTEXF_CONVOLUTIONMONO = 8,    // Convolution filter for monochrome textures
    VACMTEXF_FORCE_DWORD     = 0x7fffffff
} VACMTEXTUREFILTERTYPE;

#define CM_ATTRIBUTE(attribute) __attribute__((attribute))

typedef enum _VA_CM_FORMAT {

    VA_CM_FMT_UNKNOWN              =   0,

    VA_CM_FMT_A8R8G8B8             =  21,
    VA_CM_FMT_X8R8G8B8             =  22,
    VA_CM_FMT_A8                   =  28,
    VA_CM_FMT_A2B10G10R10          =  31,
    VA_CM_FMT_A8B8G8R8             =  32,
    VA_CM_FMT_R16G16UN             =  35,
    VA_CM_FMT_A16B16G16R16         =  36,
    VA_CM_FMT_A8P8                 =  40,
    VA_CM_FMT_P8                   =  41,
    VA_CM_FMT_R32U                 =  42,
    VA_CM_FMT_R8G8UN               =  49,
    VA_CM_FMT_L8                   =  50,
    VA_CM_FMT_A8L8                 =  51,
    VA_CM_FMT_R16UN                =  56,
    VA_CM_FMT_R16U                 =  57,
    VA_CM_FMT_V8U8                 =  60,
    VA_CM_FMT_R8UN                 =  61,
    VA_CM_FMT_R8U                  =  62,
    VA_CM_FMT_R32S                 =  71,
    VA_CM_FMT_D16                  =  80,
    VA_CM_FMT_L16                  =  81,
    VA_CM_FMT_R16F                 = 111,
    VA_CM_FMT_IA44                 = 112,
    VA_CM_FMT_A16B16G16R16F        = 113,
    VA_CM_FMT_R32F                 = 114,
    VA_CM_FMT_R32G32B32A32F        = 115,
    VA_CM_FMT_I420                 =  VA_FOURCC('I','4','2','0'),
    VA_CM_FMT_P216                 =  VA_FOURCC('P','2','1','6'),
    VA_CM_FMT_400P                 =  VA_FOURCC('4','0','0','P'),
    VA_CM_FMT_Y8UN                 =  VA_FOURCC('Y','8','U','N'),
    VA_CM_FMT_NV12                 =  VA_FOURCC_NV12,
    VA_CM_FMT_UYVY                 =  VA_FOURCC_UYVY,
    VA_CM_FMT_YUY2                 =  VA_FOURCC_YUY2,
    VA_CM_FMT_444P                 =  VA_FOURCC_444P,
    VA_CM_FMT_411P                 =  VA_FOURCC_411P,
    VA_CM_FMT_422H                 =  VA_FOURCC_422H,
    VA_CM_FMT_422V                 =  VA_FOURCC_422V,
    VA_CM_FMT_411R                 =  VA_FOURCC_411R,
    VA_CM_FMT_RGBP                 =  VA_FOURCC_RGBP,
    VA_CM_FMT_BGRP                 =  VA_FOURCC_BGRP,
    VA_CM_FMT_IMC3                 =  VA_FOURCC_IMC3,
    VA_CM_FMT_YV12                 =  VA_FOURCC_YV12,
    VA_CM_FMT_P010                 =  VA_FOURCC_P010,
    VA_CM_FMT_P016                 =  VA_FOURCC_P016,
    VA_CM_FMT_P208                 =  VA_FOURCC_P208,
    VA_CM_FMT_AYUV                 =  VA_FOURCC_AYUV,
    VA_CM_FMT_AI44                 =  VA_FOURCC_AI44,

    VA_CM_FMT_MAX                  =  0xFFFFFFFF
} VA_CM_FORMAT;

template<typename T>
inline const char * CM_TYPE_NAME_UNMANGLED();

template<> inline const char * CM_TYPE_NAME_UNMANGLED<char>() { return "char"; }
template<> inline const char * CM_TYPE_NAME_UNMANGLED<signed char>() { return "signed char"; }
template<> inline const char * CM_TYPE_NAME_UNMANGLED<unsigned char>() { return "unsigned char"; }
template<> inline const char * CM_TYPE_NAME_UNMANGLED<short>() { return "short"; }
template<> inline const char * CM_TYPE_NAME_UNMANGLED<unsigned short>() { return "unsigned short"; }
template<> inline const char * CM_TYPE_NAME_UNMANGLED<int>() { return "int"; }
template<> inline const char * CM_TYPE_NAME_UNMANGLED<unsigned int>() { return "unsigned int"; }
template<> inline const char * CM_TYPE_NAME_UNMANGLED<long>() { return "long"; }
template<> inline const char * CM_TYPE_NAME_UNMANGLED<unsigned long>() { return "unsigned long"; }
template<> inline const char * CM_TYPE_NAME_UNMANGLED<float>() { return "float"; }
template<> inline const char * CM_TYPE_NAME_UNMANGLED<double>() { return "double"; }

#define CM_TYPE_NAME(type)   CM_TYPE_NAME_UNMANGLED<type>()

inline void * CM_ALIGNED_MALLOC(size_t size, size_t alignment)
{
  return memalign(alignment, size);
}

inline void CM_ALIGNED_FREE(void * memory)
{
  free(memory);
}

//multi-thread API:
#define THREAD_HANDLE pthread_t
inline void CM_THREAD_CREATE(THREAD_HANDLE *handle, void * start_routine, void * arg)
{
    int err = 0;
    err = pthread_create(handle , NULL, (void * (*)(void *))start_routine, arg);
    if (err) {
        printf(" cm create thread failed! \n");
        exit(-1);
    }
}
inline void CM_THREAD_EXIT(void * retval)
{
    pthread_exit(retval);
}

inline int CM_THREAD_JOIN(THREAD_HANDLE *handle_array, int thread_cnt)
{
    void *tret;
    for(int i = 0; i < thread_cnt; i++)
    {
        pthread_join( handle_array[i], &tret);
    }
    return 0;
}

#define CM_SURFACE_FORMAT                       VA_CM_FORMAT

#define CM_SURFACE_FORMAT_UNKNOWN               VA_CM_FMT_UNKNOWN
#define CM_SURFACE_FORMAT_A8R8G8B8              VA_CM_FMT_A8R8G8B8
#define CM_SURFACE_FORMAT_X8R8G8B8              VA_CM_FMT_X8R8G8B8
#define CM_SURFACE_FORMAT_A8B8G8R8              VA_CM_FMT_A8B8G8R8
#define CM_SURFACE_FORMAT_A8                    VA_CM_FMT_A8
#define CM_SURFACE_FORMAT_P8                    VA_CM_FMT_P8
#define CM_SURFACE_FORMAT_R32F                  VA_CM_FMT_R32F
#define CM_SURFACE_FORMAT_NV12                  VA_CM_FMT_NV12
#define CM_SURFACE_FORMAT_UYVY                  VA_CM_FMT_UYVY
#define CM_SURFACE_FORMAT_YUY2                  VA_CM_FMT_YUY2
#define CM_SURFACE_FORMAT_V8U8                  VA_CM_FMT_V8U8

#define CM_SURFACE_FORMAT_R8_UINT               VA_CM_FMT_R8U
#define CM_SURFACE_FORMAT_R16_UINT              VA_CM_FMT_R16U
#define CM_SURFACE_FORMAT_R16_SINT              VA_CM_FMT_A8L8
#define CM_SURFACE_FORMAT_D16                   VA_CM_FMT_D16
#define CM_SURFACE_FORMAT_L16                   VA_CM_FMT_L16
#define CM_SURFACE_FORMAT_A16B16G16R16          VA_CM_FMT_A16B16G16R16
#define CM_SURFACE_FORMAT_R10G10B10A2           VA_CM_FMT_A2B10G10R10
#define CM_SURFACE_FORMAT_A16B16G16R16F         VA_CM_FMT_A16B16G16R16F
#define CM_SURFACE_FORMAT_R32G32B32A32F         VA_CM_FMT_R32G32B32A32F

#define CM_SURFACE_FORMAT_444P                  VA_CM_FMT_444P
#define CM_SURFACE_FORMAT_422H                  VA_CM_FMT_422H
#define CM_SURFACE_FORMAT_422V                  VA_CM_FMT_422V
#define CM_SURFACE_FORMAT_411P                  VA_CM_FMT_411P
#define CM_SURFACE_FORMAT_411R                  VA_CM_FMT_411R
#define CM_SURFACE_FORMAT_RGBP                  VA_CM_FMT_RGBP
#define CM_SURFACE_FORMAT_BGRP                  VA_CM_FMT_BGRP
#define CM_SURFACE_FORMAT_IMC3                  VA_CM_FMT_IMC3
#define CM_SURFACE_FORMAT_YV12                  VA_CM_FMT_YV12
#define CM_SURFACE_FORMAT_P010                  VA_CM_FMT_P010
#define CM_SURFACE_FORMAT_P016                  VA_CM_FMT_P016
#define CM_SURFACE_FORMAT_P208                  VA_CM_FMT_P208
#define CM_SURFACE_FORMAT_AYUV                  VA_CM_FMT_AYUV

#define CM_SURFACE_FORMAT_IA44                  VA_CM_FMT_IA44
#define CM_SURFACE_FORMAT_AI44                  VA_CM_FMT_AI44
#define CM_SURFACE_FORMAT_I420                  VA_CM_FMT_I420
#define CM_SURFACE_FORMAT_P216                  VA_CM_FMT_P216
#define CM_SURFACE_FORMAT_400P                  VA_CM_FMT_400P
#define CM_SURFACE_FORMAT_R16_FLOAT             VA_CM_FMT_R16F
#define CM_SURFACE_FORMAT_Y8_UNORM              VA_CM_FMT_Y8UN
#define CM_SURFACE_FORMAT_A8P8                  VA_CM_FMT_A8P8
#define CM_SURFACE_FORMAT_R32_SINT              VA_CM_FMT_R32S
#define CM_SURFACE_FORMAT_R32_UINT              VA_CM_FMT_R32U
#define CM_SURFACE_FORMAT_R8G8_UNORM            VA_CM_FMT_R8G8UN
#define CM_SURFACE_FORMAT_R8_UNORM              VA_CM_FMT_R8UN
#define CM_SURFACE_FORMAT_R16G16_UNORM          VA_CM_FMT_R16G16UN
#define CM_SURFACE_FORMAT_R16_UNORM             VA_CM_FMT_R16UN

#include "type_large_integer.h"

//Performance
EXTERN_C INT QueryPerformanceFrequency(LARGE_INTEGER *lpFrequency);
EXTERN_C INT QueryPerformanceCounter(LARGE_INTEGER *lpPerformanceCount);
    #define CM_KERNEL_FUNCTION2(...) #__VA_ARGS__, (void *)(void (__cdecl *) (void))__VA_ARGS__

    #define _NAME(...) #__VA_ARGS__, (void (__cdecl *)(void))__VA_ARGS__

#endif //__CM_RT_DEF_OS_H__
