/*========================== begin_copyright_notice ============================

Copyright (C) 2017 Intel Corporation

SPDX-License-Identifier: MIT

============================= end_copyright_notice ===========================*/

#ifndef CMRTLIB_WINDOWS_SHARE_CM_DEF_OS_H_
#define CMRTLIB_WINDOWS_SHARE_CM_DEF_OS_H_

#include <cstdint>
#include <cstddef>

#ifndef __PRETTY_FUNCTION__
#define __PRETTY_FUNCTION__ __FUNCSIG__
#endif

#ifdef CM_DX11
#include <d3d11.h>
#define CM_MAX_SURFACE2D_FORMAT_COUNT 57
// max resolution for surface 2D
#define CM_MAX_2D_SURF_WIDTH  16384
#define CM_MAX_2D_SURF_HEIGHT 16384

#elif defined(CM_DX9)
#include <d3d9.h>
#include <dxva2api.h>
#define CM_MAX_SURFACE2D_FORMAT_COUNT 47  //work around for surface type used in typed atomic message in SW scoreboard
// max resolution for surface 2D
#define CM_MAX_2D_SURF_WIDTH  32768
#define CM_MAX_2D_SURF_HEIGHT 16384

#endif  // #ifdef CM_DX11

#include "cm_rt_def_os.h"

////////////////////////////////////////////////////////////////////////////////////
//      Platform dependent macros (Start)
////////////////////////////////////////////////////////////////////////////////////
#define CM_STRCPY(dst, sizeInBytes, src)       strcpy_s(dst, sizeInBytes, src)
#define CM_STRNCPY(dst, sizeOfDst, src, count) strncpy_s(dst, sizeOfDst, src, count)
#define CM_STRCAT(dst, sizeOfDst, src)       strcat_s(dst, sizeOfDst, src)
#define CM_GETENV(dst, name) {size_t length; _dupenv_s(&dst, &length, name);}
#define CM_GETENV_FREE(dst)  { if (dst != NULL) free(dst); }
#define CM_FOPEN(pFile, filename, mode) fopen_s(&pFile, filename, mode)
////////////////////////////////////////////////////////////////////////////////////
//      Platform dependent macros (End)
////////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////////
//      Platform dependent definitions (Start)
////////////////////////////////////////////////////////////////////////////////////

#ifdef CM_DX9
#define CM_IDIRECT3DSURFACE                 IDirect3DSurface9
#elif defined(CM_DX11)  //For DX11
#define CM_IDIRECT3DSURFACE             ID3D11Texture2D

#ifndef D3D_OK
#define D3D_OK S_OK
#endif
#endif
////////////////////////////////////////////////////////////////////////////////////
//      Platform dependent definitions (End)
////////////////////////////////////////////////////////////////////////////////////

typedef enum _REGISTRATION_OP
{
    REG_IGNORE          = 0,
    REG_REGISTER        = 1,
    REG_UNREGISTER      = 2,
    REG_REGISTER_INDEX  = 3     // Register surface for Cm
} REGISTRATION_OP;

#ifdef CM_DX9
typedef struct tagDXVA2_SAMPLE_REG
{
    REGISTRATION_OP     iRegistration;
    IDirect3DSurface9   *pSurface;
} DXVA2_SAMPLE_REG;
#elif defined(CM_DX11)
typedef struct tagDXVA2_SAMPLE_REG
{
    REGISTRATION_OP     iRegistration;
    ID3D11Texture2D     *pD3D11Texture;
} DXVA2_SAMPLE_REG;
#endif

#ifdef CM_DX9
typedef IDirect3DSurface9   D3DSURFACE;
#elif defined(CM_DX11)
typedef ID3D11Texture2D     D3DSURFACE;

typedef struct _CmSupportedAdapterInfo
{
    IDXGIAdapter* CmSupportedAdapter = nullptr;
    DXGI_ADAPTER_DESC adapterDesc = {};
    bool bDiscreteGPU = false;
    UINT MaxThread = 0;
    UINT EuNumber = 0;
    UINT TileNumber = 0;
    UINT reserved[16];
} CmSupportedAdapterInfo;
#endif

typedef struct tagDXVA2_SURFACE_REGISTRATION
{
    HANDLE              hRegistration;
    DXVA2_SAMPLE_REG    *pRenderTarget;
    uint32_t            nSamples;
    DXVA2_SAMPLE_REG    *pSamples;
} DXVA2_SURFACE_REGISTRATION;

class CSync
{
public:
    CSync()
    {
        InitializeCriticalSection(&m_CriticalSection);
    }
    ~CSync() { DeleteCriticalSection(&m_CriticalSection); }
    void Acquire() { EnterCriticalSection(&m_CriticalSection); }
    void Release() { LeaveCriticalSection(&m_CriticalSection); }

private:
    CRITICAL_SECTION m_CriticalSection;
};

typedef struct _CM_CREATESURFACE2D_PARAM
{
    uint32_t    iWidth;                     // [in] width of 2D texture in pixel
    uint32_t    iHeight;                    // [in] height of 2D texture in pixel
    CM_SURFACE_FORMAT   Format;             // [in] DXGI format of 2D texture
    union
    {
        uint32_t index2DinLookupTable;       // [in] surface 2d's index in look up table, only used in dx9
        uint32_t uiVASurfaceID;              // [in] libva-surface 2d's index in media driver
    };
    D3DSURFACE* pD3DSurf;                   // [in] D3D9 : Pointer to D3D Surface
    void        *pCmSurface2DHandle;         // [out] pointer of CmSurface2D used in driver
    bool        bIsCmCreated;
    int32_t     iReturnValue;               // [out] the return value from driver
#ifdef CM_DX11
    uint32_t    uiFirstArraySlice;          // [in] the index of array slice
    uint32_t    uiMipSlice;                 // [in] the index of Mip slice
#endif
}CM_CREATESURFACE2D_PARAM, *PCM_CREATESURFACE2D_PARAM;

#ifndef CMRT_NOINLINE
#define CMRT_NOINLINE __declspec(noinline)
#endif

template <class T> void D3DSafeRelease(T **ppT)
{
    if (*ppT)
    {
        (*ppT)->Release();
        *ppT = nullptr;
    }
}

// returns the new reference count, intended to be used only for test purposes
template <class T> uint32_t D3DSafeAddRef(T *ppT)
{
    uint32_t iRefCnt = 0;
    if (ppT)
    {
        iRefCnt = ppT->AddRef();
    }
    return iRefCnt;
}

#endif  // #ifndef CMRTLIB_WINDOWS_SHARE_CM_DEF_OS_H_
