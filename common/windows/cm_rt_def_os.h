/*========================== begin_copyright_notice ============================

Copyright (C) 2017 Intel Corporation

SPDX-License-Identifier: MIT

============================= end_copyright_notice ===========================*/

//!
//! \file      cm_rt_def_os.h
//! \brief     Contains windows-specific Definitions for CM
//!

#ifndef __CM_RT_DEF_OS_H__
#define __CM_RT_DEF_OS_H__

#include "windows.h"
#include "winbase.h"
#include "stdio.h"
#include "typeinfo"

// For CM_* APIs
#include <cfgmgr32.h>
#pragma comment(lib, "cfgmgr32.lib")

// For GUID_DEVCLASS_*
#include <devguid.h>
#include <string>
#include <algorithm>

//Using CM_DX9 by default
#if (!defined(CM_DX11))
#ifndef CM_DX9
#define CM_DX9
#endif
#endif

#define CM_ATTRIBUTE(attribute) __declspec(attribute)
#define CM_TYPE_NAME(type)  typeid(type).name()

#ifndef CM_NOINLINE
        #define CM_NOINLINE
#endif

inline void * CM_ALIGNED_MALLOC(size_t size, size_t alignment)
{
    return _aligned_malloc(size, alignment);
}

inline void CM_ALIGNED_FREE(void * memory)
{
    _aligned_free(memory);
}

//multi-thread API:
#define THREAD_HANDLE HANDLE
inline void CM_THREAD_CREATE(THREAD_HANDLE *handle, void * start_routine, void * arg)
{
    handle[0] = CreateThread(NULL,0,(LPTHREAD_START_ROUTINE)start_routine, (LPVOID )arg, 0, NULL);
}
inline void CM_THREAD_EXIT(void * retval)
{
    ExitThread(0);
    *(int *)retval = 0; // useless statement, just for removing C4100 warning.
}

inline int CM_THREAD_JOIN(THREAD_HANDLE *handle_array, int thread_cnt)
{
    DWORD ret = WaitForMultipleObjects( thread_cnt, handle_array, true, INFINITE );
    return ret;
}

#ifdef CM_DX9
    #include <d3d9.h>
    #include <dxva2api.h>

    #define CM_SURFACE_FORMAT                       D3DFORMAT

    #define CM_SURFACE_FORMAT_UNKNOWN               D3DFMT_UNKNOWN
    #define CM_SURFACE_FORMAT_A8R8G8B8              D3DFMT_A8R8G8B8
    #define CM_SURFACE_FORMAT_X8R8G8B8              D3DFMT_X8R8G8B8
    #define CM_SURFACE_FORMAT_A8B8G8R8              D3DFMT_A8B8G8R8
    #define CM_SURFACE_FORMAT_A8                    D3DFMT_A8
    #define CM_SURFACE_FORMAT_P8                    D3DFMT_P8
    #define CM_SURFACE_FORMAT_R32F                  D3DFMT_R32F
    #define CM_SURFACE_FORMAT_NV12                  (D3DFORMAT)MAKEFOURCC('N','V','1','2')
    #define CM_SURFACE_FORMAT_P016                  (D3DFORMAT)MAKEFOURCC('P','0','1','6')
    #define CM_SURFACE_FORMAT_P010                  (D3DFORMAT)MAKEFOURCC('P','0','1','0')
    #define CM_SURFACE_FORMAT_UYVY                  D3DFMT_UYVY
    #define CM_SURFACE_FORMAT_YUY2                  D3DFMT_YUY2
    #define CM_SURFACE_FORMAT_V8U8                  D3DFMT_V8U8
    #define CM_SURFACE_FORMAT_A8L8                  D3DFMT_A8L8
    #define CM_SURFACE_FORMAT_D16                   D3DFMT_D16
    #define CM_SURFACE_FORMAT_L16                   D3DFMT_L16
    #define CM_SURFACE_FORMAT_A16B16G16R16          D3DFMT_A16B16G16R16
    #define CM_SURFACE_FORMAT_A16B16G16R16F         D3DFMT_A16B16G16R16F
    #define CM_SURFACE_FORMAT_R10G10B10A2           D3DFMT_A2B10G10R10
    #define CM_SURFACE_FORMAT_IRW0                  (D3DFORMAT)MAKEFOURCC('I','R','W','0')
    #define CM_SURFACE_FORMAT_IRW1                  (D3DFORMAT)MAKEFOURCC('I','R','W','1')
    #define CM_SURFACE_FORMAT_IRW2                  (D3DFORMAT)MAKEFOURCC('I','R','W','2')
    #define CM_SURFACE_FORMAT_IRW3                  (D3DFORMAT)MAKEFOURCC('I','R','W','3')
    #define CM_SURFACE_FORMAT_R16_FLOAT             D3DFMT_R16F  //beryl
    #define CM_SURFACE_FORMAT_A8P8                  D3DFMT_A8P8
    #define CM_SURFACE_FORMAT_R32G32B32A32F         D3DFMT_A32B32G32R32F
    #define CM_SURFACE_FORMAT_I420                  (D3DFORMAT)MAKEFOURCC('I','4','2','0')
    #define CM_SURFACE_FORMAT_IMC3                  (D3DFORMAT)MAKEFOURCC('I','M','C','3')
    #define CM_SURFACE_FORMAT_IA44                  (D3DFORMAT)MAKEFOURCC('I','A','4','4')
    #define CM_SURFACE_FORMAT_AI44                  (D3DFORMAT)MAKEFOURCC('A','I','4','4')
    #define CM_SURFACE_FORMAT_P216                  (D3DFORMAT)MAKEFOURCC('P','2','1','6')
    #define CM_SURFACE_FORMAT_Y410                  (D3DFORMAT)MAKEFOURCC('Y','4','1','0')
    #define CM_SURFACE_FORMAT_Y416                  (D3DFORMAT)MAKEFOURCC('Y','4','1','6')
    #define CM_SURFACE_FORMAT_Y210                  (D3DFORMAT)MAKEFOURCC('Y','2','1','0')
    #define CM_SURFACE_FORMAT_Y216                  (D3DFORMAT)MAKEFOURCC('Y','2','1','6')
    #define CM_SURFACE_FORMAT_AYUV                  (D3DFORMAT)MAKEFOURCC('A','Y','U','V')
    #define CM_SURFACE_FORMAT_YV12                  (D3DFORMAT)MAKEFOURCC('Y','V','1','2')
    #define CM_SURFACE_FORMAT_400P                  (D3DFORMAT)MAKEFOURCC('4','0','0','P')
    #define CM_SURFACE_FORMAT_411P                  (D3DFORMAT)MAKEFOURCC('4','1','1','P')
    #define CM_SURFACE_FORMAT_411R                  (D3DFORMAT)MAKEFOURCC('4','1','1','R')
    #define CM_SURFACE_FORMAT_422H                  (D3DFORMAT)MAKEFOURCC('4','2','2','H')
    #define CM_SURFACE_FORMAT_422V                  (D3DFORMAT)MAKEFOURCC('4','2','2','V')
    #define CM_SURFACE_FORMAT_444P                  (D3DFORMAT)MAKEFOURCC('4','4','4','P')
    #define CM_SURFACE_FORMAT_RGBP                  (D3DFORMAT)MAKEFOURCC('R','G','B','P')
    #define CM_SURFACE_FORMAT_BGRP                  (D3DFORMAT)MAKEFOURCC('B','G','R','P')
    #define CM_SURFACE_FORMAT_Y8_UNORM              (D3DFORMAT)MAKEFOURCC('Y','8','U','N')
    #define CM_SURFACE_FORMAT_P208                  (D3DFORMAT)MAKEFOURCC('P','2','0','8')

    #define CM_TEXTURE_ADDRESS_TYPE                 D3DTEXTUREADDRESS
    #define CM_TEXTURE_ADDRESS_WRAP                 D3DTADDRESS_WRAP
    #define CM_TEXTURE_ADDRESS_MIRROR               D3DTADDRESS_MIRROR
    #define CM_TEXTURE_ADDRESS_CLAMP                D3DTADDRESS_CLAMP
    #define CM_TEXTURE_ADDRESS_BORDER               D3DTADDRESS_BORDER
    #define CM_TEXTURE_ADDRESS_MIRRORONCE           D3DTADDRESS_MIRRORONCE

    #define CM_TEXTURE_FILTER_TYPE                  D3DTEXTUREFILTERTYPE

    #define CM_TEXTURE_FILTER_TYPE_NONE             D3DTEXF_NONE
    #define CM_TEXTURE_FILTER_TYPE_POINT            D3DTEXF_POINT
    #define CM_TEXTURE_FILTER_TYPE_LINEAR           D3DTEXF_LINEAR
    #define CM_TEXTURE_FILTER_TYPE_ANISOTROPIC      D3DTEXF_ANISOTROPIC
    #define CM_TEXTURE_FILTER_TYPE_FLATCUBIC        D3DTEXF_FLATCUBIC
    #define CM_TEXTURE_FILTER_TYPE_GAUSSIANCUBIC    D3DTEXF_GAUSSIANCUBIC
    #define CM_TEXTURE_FILTER_TYPE_PYRAMIDALQUAD    D3DTEXF_PYRAMIDALQUAD
    #define CM_TEXTURE_FILTER_TYPE_GAUSSIANQUAD     D3DTEXF_GAUSSIANQUAD
    #define CM_TEXTURE_FILTER_TYPE_CONVOLUTIONMONO  D3DTEXF_CONVOLUTIONMONO
#elif defined(CM_DX11)
    #include <d3d11.h>
    #ifndef MAKEFOURCC
    #define MAKEFOURCC(ch0, ch1, ch2, ch3)                              \
                ((DWORD)(BYTE)(ch0) | ((DWORD)(BYTE)(ch1) << 8) |       \
                ((DWORD)(BYTE)(ch2) << 16) | ((DWORD)(BYTE)(ch3) << 24 ))
    #endif
    #define CM_SURFACE_FORMAT                       DXGI_FORMAT

    #define CM_SURFACE_FORMAT_UNKNOWN               DXGI_FORMAT_UNKNOWN
    #define CM_SURFACE_FORMAT_A8R8G8B8              DXGI_FORMAT_B8G8R8A8_UNORM
    #define CM_SURFACE_FORMAT_X8R8G8B8              DXGI_FORMAT_B8G8R8X8_UNORM
    #define CM_SURFACE_FORMAT_A8B8G8R8              DXGI_FORMAT_R8G8B8A8_UNORM
    #define CM_SURFACE_FORMAT_A8                    DXGI_FORMAT_A8_UNORM
    #define CM_SURFACE_FORMAT_P8                    DXGI_FORMAT_P8
    #define CM_SURFACE_FORMAT_R32F                  DXGI_FORMAT_R32_FLOAT
    #define CM_SURFACE_FORMAT_NV12                  DXGI_FORMAT_NV12
    #define CM_SURFACE_FORMAT_P016                  DXGI_FORMAT_P016
    #define CM_SURFACE_FORMAT_P010                  DXGI_FORMAT_P010
    #define CM_SURFACE_FORMAT_UYVY                  DXGI_FORMAT_R8G8_B8G8_UNORM
    #define CM_SURFACE_FORMAT_YUY2                  DXGI_FORMAT_YUY2
    #define CM_SURFACE_FORMAT_V8U8                  DXGI_FORMAT_R8G8_SNORM
    #define CM_SURFACE_FORMAT_R8_UINT               DXGI_FORMAT_R8_UINT
    #define CM_SURFACE_FORMAT_R16_SINT              DXGI_FORMAT_R16_SINT
    #define CM_SURFACE_FORMAT_R16_UNORM             DXGI_FORMAT_R16_UNORM
    #define CM_SURFACE_FORMAT_R32_UINT              DXGI_FORMAT_R32_UINT
    #define CM_SURFACE_FORMAT_R32_SINT              DXGI_FORMAT_R32_SINT
    #define CM_SURFACE_FORMAT_R8G8_UNORM            DXGI_FORMAT_R8G8_UNORM
    #define CM_SURFACE_FORMAT_R8_UNORM              DXGI_FORMAT_R8_UNORM
    #define CM_SURFACE_FORMAT_R16_UINT              DXGI_FORMAT_R16_UINT
    #define CM_SURFACE_FORMAT_A16B16G16R16          DXGI_FORMAT_R16G16B16A16_UNORM
    #define CM_SURFACE_FORMAT_A16B16G16R16F         DXGI_FORMAT_R16G16B16A16_FLOAT
    #define CM_SURFACE_FORMAT_R10G10B10A2           DXGI_FORMAT_R10G10B10A2_UNORM
    #define CM_SURFACE_FORMAT_R16_TYPELESS          DXGI_FORMAT_R16_TYPELESS
    #define CM_SURFACE_FORMAT_AYUV                  DXGI_FORMAT_AYUV
    #define CM_SURFACE_FORMAT_R16G16_UNORM          DXGI_FORMAT_R16G16_UNORM
    #define CM_SURFACE_FORMAT_R16_FLOAT             DXGI_FORMAT_R16_FLOAT   //beryl
    #define CM_SURFACE_FORMAT_A8P8                  DXGI_FORMAT_A8P8
    #define CM_SURFACE_FORMAT_IA44                  DXGI_FORMAT_IA44
    #define CM_SURFACE_FORMAT_AI44                  DXGI_FORMAT_AI44
    #define CM_SURFACE_FORMAT_Y410                  DXGI_FORMAT_Y410
    #define CM_SURFACE_FORMAT_Y416                  DXGI_FORMAT_Y416
    #define CM_SURFACE_FORMAT_Y210                  DXGI_FORMAT_Y210
    #define CM_SURFACE_FORMAT_Y216                  DXGI_FORMAT_Y216
    #define CM_SURFACE_FORMAT_I420                  (DXGI_FORMAT)MAKEFOURCC('I','4','2','0')
    #define CM_SURFACE_FORMAT_IMC3                  (DXGI_FORMAT)MAKEFOURCC('I','M','C','3')
    #define CM_SURFACE_FORMAT_P216                  (DXGI_FORMAT)MAKEFOURCC('P','2','1','6')
    #define CM_SURFACE_FORMAT_YV12                  (DXGI_FORMAT)MAKEFOURCC('Y','V','1','2')
    #define CM_SURFACE_FORMAT_400P                  (DXGI_FORMAT)MAKEFOURCC('4','0','0','P')
    #define CM_SURFACE_FORMAT_411P                  (DXGI_FORMAT)MAKEFOURCC('4','1','1','P')
    #define CM_SURFACE_FORMAT_411R                  (DXGI_FORMAT)MAKEFOURCC('4','1','1','R')
    #define CM_SURFACE_FORMAT_422H                  (DXGI_FORMAT)MAKEFOURCC('4','2','2','H')
    #define CM_SURFACE_FORMAT_422V                  (DXGI_FORMAT)MAKEFOURCC('4','2','2','V')
    #define CM_SURFACE_FORMAT_444P                  (DXGI_FORMAT)MAKEFOURCC('4','4','4','P')
    #define CM_SURFACE_FORMAT_RGBP                  (DXGI_FORMAT)MAKEFOURCC('R','G','B','P')
    #define CM_SURFACE_FORMAT_BGRP                  (DXGI_FORMAT)MAKEFOURCC('B','G','R','P')
    #define CM_SURFACE_FORMAT_Y8_UNORM              (DXGI_FORMAT)MAKEFOURCC('Y','8','U','N')
    #define CM_SURFACE_FORMAT_D16                   DXGI_FORMAT_D16_UNORM
    #define CM_SURFACE_FORMAT_D32F                  DXGI_FORMAT_D32_FLOAT
    #define CM_SURFACE_FORMAT_D24_UNORM_S8_UINT     DXGI_FORMAT_D24_UNORM_S8_UINT
    #define CM_SURFACE_FORMAT_D32F_S8X24_UINT       DXGI_FORMAT_D32_FLOAT_S8X24_UINT
    #define CM_SURFACE_FORMAT_R16G16_SINT           DXGI_FORMAT_R16G16_SINT
    #define CM_SURFACE_FORMAT_R24G8_TYPELESS        DXGI_FORMAT_R24G8_TYPELESS
    #define CM_SURFACE_FORMAT_R32_TYPELESS          DXGI_FORMAT_R32_TYPELESS
    #define CM_SURFACE_FORMAT_R32G8X24_TYPELESS     DXGI_FORMAT_R32G8X24_TYPELESS
    #define CM_SURFACE_FORMAT_P208                  (DXGI_FORMAT)MAKEFOURCC('P','2','0','8')
    #define CM_SURFACE_FORMAT_R32G32B32A32F         DXGI_FORMAT_R32G32B32A32_FLOAT
    #define CM_TEXTURE_ADDRESS_TYPE                 D3D11_TEXTURE_ADDRESS_MODE

    #define CM_TEXTURE_ADDRESS_WRAP                 D3D11_TEXTURE_ADDRESS_WRAP
    #define CM_TEXTURE_ADDRESS_MIRROR               D3D11_TEXTURE_ADDRESS_MIRROR
    #define CM_TEXTURE_ADDRESS_CLAMP                D3D11_TEXTURE_ADDRESS_CLAMP
    #define CM_TEXTURE_ADDRESS_BORDER               D3D11_TEXTURE_ADDRESS_BORDER
    #define CM_TEXTURE_ADDRESS_MIRRORONCE           D3D11_TEXTURE_ADDRESS_MIRROR_ONCE

    #define CM_TEXTURE_FILTER_TYPE                  D3D11_FILTER_TYPE

    #define CM_TEXTURE_FILTER_TYPE_POINT            D3D11_FILTER_TYPE_POINT
    #define CM_TEXTURE_FILTER_TYPE_LINEAR           D3D11_FILTER_TYPE_LINEAR
#endif
    #define CM_KERNEL_FUNCTION2(...) #__VA_ARGS__, (void *)(void (__cdecl *) (void))__VA_ARGS__

    #define _NAME(...) #__VA_ARGS__, (void (__cdecl *)(void))__VA_ARGS__

#define OS_WIN_RS 10014310

/******************************************************************************\
Function:    GetWinVer
Description: Determines the version of windows os
Returns:     UINSIGNED INT 99 9 99999 ( MajorVersion x 1000 000 MinorVersion x 100 000 OSBuildNumber)
\******************************************************************************/
inline unsigned int GetWinVer( void )
{
    unsigned int Result = 0;
    LONG( WINAPI *pfnRtlGetVersion )( RTL_OSVERSIONINFOW* );
    ( FARPROC& )pfnRtlGetVersion = GetProcAddress( GetModuleHandleW( ( L"ntdll.dll" ) ), "RtlGetVersion" );
    if ( pfnRtlGetVersion )
    {
        RTL_OSVERSIONINFOW RTLOSVersionInfo = { 0 };
        RTLOSVersionInfo.dwOSVersionInfoSize = sizeof( RTLOSVersionInfo );
        // Get the OS version
        if ( pfnRtlGetVersion( &RTLOSVersionInfo ) == 0 )
        {
            Result = RTLOSVersionInfo.dwBuildNumber;
            Result += ( RTLOSVersionInfo.dwMinorVersion * 100000 );
            Result += ( RTLOSVersionInfo.dwMajorVersion * 1000000 );
        }
    }
    return Result;
}

inline void ConvertDoubleNullTermStringToArray(wchar_t *input_str,
                                               size_t input_str_len,
                                               std::wstring *&output_vec,
                                               int &device_num)
{
    output_vec = new std::wstring [input_str_len];
    std::wstring str;
    std::for_each(input_str, input_str +input_str_len-1, [&](wchar_t &w) {
    if (w != L'\0')
    {
        str += w;
    }
    else
    {
        output_vec[device_num]= str;
        device_num++;
        str = L"";
    }
    });
}

inline bool IsIntelDeviceInstanceID(const std::wstring &device_id)
{
    if (device_id.find(L"VEN_8086") != std::wstring::npos
        || device_id.find(L"ven_8086") != std::wstring::npos)
    {
    return true;
    }
    else
    {
    return false;
    }
}

inline bool GetDriverStorePath(WCHAR *pBuffer_driver)
{
    // Obtain a PnP handle to the Intel graphics adapter
    CONFIGRET    result = CR_SUCCESS;
    DWORD        dwWin32Err = ERROR_SUCCESS;
    std::wstring IntelDeviceID;

    ULONG     DeviceIDListSize = 0;
    PWSTR     DeviceIDList = nullptr;
    wchar_t     DisplayGUID[40];
    DEVINST     DeviceInst;

    StringFromGUID2(GUID_DEVCLASS_DISPLAY, DisplayGUID, sizeof(DisplayGUID));

    do
    {
    result = CM_Get_Device_ID_List_SizeW(&DeviceIDListSize, DisplayGUID,
                                             CM_GETIDLIST_FILTER_CLASS
                                             |CM_GETIDLIST_FILTER_PRESENT);
    if (result != CR_SUCCESS)
    {
        dwWin32Err = CM_MapCrToWin32Err(result, ERROR_FILE_NOT_FOUND);
        break;
    }

    DeviceIDList = reinterpret_cast<PWSTR>(HeapAlloc(GetProcessHeap(),
                                               HEAP_ZERO_MEMORY,
                                               DeviceIDListSize * sizeof(PWSTR)));
    if (DeviceIDList == nullptr)
    {
        result = CR_OUT_OF_MEMORY;
        break;
    }

    result = CM_Get_Device_ID_ListW(DisplayGUID, DeviceIDList, DeviceIDListSize,
                                                     CM_GETIDLIST_FILTER_CLASS
                                                     | CM_GETIDLIST_FILTER_PRESENT);

    } while (result == CR_BUFFER_SMALL);

    if (result != CR_SUCCESS)
    {
    dwWin32Err = CM_MapCrToWin32Err(result, ERROR_FILE_NOT_FOUND);
    HeapFree(GetProcessHeap(), 0, DeviceIDList);
    return false;
    }

    std::wstring* Devices = nullptr;
    int device_num = 0;
    ConvertDoubleNullTermStringToArray(DeviceIDList, DeviceIDListSize, Devices, device_num);

    HeapFree(GetProcessHeap(), 0, DeviceIDList);
    DeviceIDList = nullptr;

    std::for_each(Devices, Devices + device_num, [&](std::wstring &w)
    {
    if (IsIntelDeviceInstanceID(w))
    {
        IntelDeviceID = w;
    }

    });
    if (Devices)
    {
    delete []Devices;
    }

    //check if is intel device
    if (!IntelDeviceID.empty())
    {
    result = CM_Locate_DevNodeW(&DeviceInst,
                                    const_cast<wchar_t *>(IntelDeviceID.c_str()),
                                    CM_LOCATE_DEVNODE_NORMAL);
    if (result != CR_SUCCESS)
    {
        dwWin32Err = CM_MapCrToWin32Err(result, ERROR_FILE_NOT_FOUND);
        return false;
    }

    HKEY hKey_sw;
    result = CM_Open_DevNode_Key(DeviceInst, KEY_READ, 0,
                                                 RegDisposition_OpenExisting,
                                                 &hKey_sw, CM_REGISTRY_SOFTWARE);
    if (result != CR_SUCCESS)
    {
        dwWin32Err = CM_MapCrToWin32Err(result, ERROR_FILE_NOT_FOUND);
        return false;
    }

    DWORD dwBufferSize = sizeof(WCHAR) * MAX_PATH;
    ULONG nError;
    REGSAM  samDesired = KEY_READ;

    nError = RegQueryValueExW(hKey_sw, L"DriverStorePathForMDF", 0, NULL,
                                  (LPBYTE)pBuffer_driver, &dwBufferSize);
    if (ERROR_SUCCESS != nError)
    {
        return false;
    }

    RegCloseKey(hKey_sw);
    }
    else
    {
    return false;
    }

    return true;

}

inline bool LoadCMRTDLL()
{
    if ( GetWinVer() < OS_WIN_RS )  // only proceed for OS version RS1+
    {
        return false;
    }

    return false;

    WCHAR pBuffer[ MAX_PATH ];
    DWORD dwBufferSize = sizeof( pBuffer );
    REGSAM  samDesired = KEY_READ;

    if ( GetSystemWow64Directory( ( LPTSTR )pBuffer, MAX_PATH ) > 0 )  // 64-bit OS
    {
        samDesired |= KEY_WOW64_64KEY;
    }

    if (GetDriverStorePath(pBuffer) == true)
    {
#ifdef CM_DX11
        wcscat_s( pBuffer, MAX_PATH, L"\\igfx11cmrt" );
#else if CM_DX9
        wcscat_s( pBuffer, MAX_PATH, L"\\igfxcmrt" );
#endif

#ifndef _WIN64
        wcscat_s( pBuffer, MAX_PATH, L"32.dll" );
#else
        wcscat_s( pBuffer, MAX_PATH, L"64.dll" );
#endif

        HMODULE handle = LoadLibraryExW( pBuffer, NULL, 0 );
        if ( handle == NULL )
        {
            return false;
        }
        return true;
    }
    else
    {
        return false;
    }
}

static bool g_MDF_LoadCMRTDLL_Result = LoadCMRTDLL();

#endif //__CM_RT_DEF_OS_H__
