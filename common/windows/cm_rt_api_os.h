/*========================== begin_copyright_notice ============================

Copyright (C) 2017 Intel Corporation

SPDX-License-Identifier: MIT

============================= end_copyright_notice ===========================*/

//!
//! \file      cm_rt_api_os.h
//! \brief     Contains Windows Specific APIs and Definitions for CM
//!

#ifndef __CM_RT_API_OS_H__
#define __CM_RT_API_OS_H__

#include "type_surface_2d_base.h"

#include "type_device_base.h"

#ifdef CM_DX9
    EXTERN_C CM_RT_API INT CreateCmDevice(CmDevice* &pD, UINT& version, IDirect3DDeviceManager9* pD3DDeviceMgr = nullptr );
    EXTERN_C CM_RT_API INT CreateCmDeviceEx(CmDevice* &pD, UINT & version, IDirect3DDeviceManager9* pD3DDeviceMgr, UINT  DevCreateOption = CM_DEVICE_CREATE_OPTION_DEFAULT);

#elif defined(CM_DX11)
typedef enum _AdapterInfoType
{
    Description,                    //    WCHAR Description[ 128 ];
    VendorId,                       //    UINT VendorId;
    DeviceId,                       //    UINT DeviceId;
    SubSysId,                       //    UINT SubSysId;
    Revision,                       //    UINT Revision;
    DedicatedVideoMemory,           //    SIZE_T DedicatedVideoMemory;
    DedicatedSystemMemory,          //    SIZE_T DedicatedSystemMemory;
    SharedSystemMemory,             //    SIZE_T SharedSystemMemory;
    AdapterLuid,                    //    LUID AdapterLuid;
    isDiscreteGPU,                  //    bool
    MaxThread,                      //    UINT
    EuNumber,                       //    UINT
    TileNumber,                     //    UINT
    Reserved                        //    UINT
} AdapterInfoType;

EXTERN_C CM_RT_API INT GetCmSupportedAdapters(uint32_t& count);
EXTERN_C CM_RT_API INT QueryCmAdapterInfo(UINT AdapterIndex, AdapterInfoType infoName, void *info, uint32_t infoSize, uint32_t *OutInfoSize);
EXTERN_C CM_RT_API INT CreateCmDeviceFromAdapter(CmDevice* &pCmDev, UINT& version, UINT AdapterIndex, uint32_t DevCreateOption = CM_DEVICE_CREATE_OPTION_DEFAULT);
EXTERN_C CM_RT_API INT CreateCmDevice(CmDevice* &pCmDev, UINT& version, ID3D11Device* pD3D11Device = nullptr);
EXTERN_C CM_RT_API INT CreateCmDeviceEx(CmDevice* &pCmDev, UINT& version, ID3D11Device* pD3D11Device, UINT DevCreateOption = CM_DEVICE_CREATE_OPTION_DEFAULT);
#endif

// vtune helpers API
EXTERN_C CM_RT_API UINT CMRT_GetKernelCount(CmEvent *event);
EXTERN_C CM_RT_API INT CMRT_GetKernelName(CmEvent *event, UINT index, char **KernelName);
EXTERN_C CM_RT_API INT CMRT_GetKernelThreadSpace(CmEvent *event, UINT index, UINT *localWidth, UINT *localHeight, UINT *globalWidth, UINT *globalHeight);
EXTERN_C CM_RT_API INT CMRT_GetSubmitTime(CmEvent *event, LARGE_INTEGER *time);
EXTERN_C CM_RT_API INT CMRT_GetHWStartTime(CmEvent *event, LARGE_INTEGER *time);
EXTERN_C CM_RT_API INT CMRT_GetHWEndTime(CmEvent *event, LARGE_INTEGER *time);
EXTERN_C CM_RT_API INT CMRT_GetCompleteTime(CmEvent *event, LARGE_INTEGER *time);
EXTERN_C CM_RT_API INT CMRT_SetEventCallback(CmEvent *event, callback_function function, void *user_data);
EXTERN_C CM_RT_API INT CMRT_GetEnqueueTime( CmEvent *event, LARGE_INTEGER *time );

#endif //__CM_RT_API_OS_H__
