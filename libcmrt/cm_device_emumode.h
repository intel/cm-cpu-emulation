/*========================== begin_copyright_notice ============================

Copyright (C) 2017 Intel Corporation

SPDX-License-Identifier: MIT

============================= end_copyright_notice ===========================*/

#pragma once

#include <vector>

#include "cm_device_base.h"
#include "cm_array.h"
#include "cm_def.h"
#include "cm_priv_def.h"

class CmSurfaceManager;
class CmSurfaceManagerEmu;
class CmSurface;
class CmBuffer;
class CmBufferEmu;
class CmSurface2D;
class CmSurface2DEmu;
class CmSurface3D;
class CmQueue;
class CmQueueEmu;
class CmProgram;
class CmProgramEmu;
class CmKernel;
class CmKernelEmu;
class CmThreadSpace;
class CmDevice;
class CmTask;
class CmThreadGroupSpace;

//!
//! CM Device
//!
class CmDeviceEmu : public CmDevice
{
public:
#ifdef CM_DX9
    static int32_t Create( IDirect3DDeviceManager9* pD3DDeviceMgr, CmDeviceEmu* &pDevice );
#elif defined CM_DX11
    static int32_t Create( ID3D11Device* pD3DDeviceMgr, CmDeviceEmu* &pDevice );
#elif defined __GNUC__
    static int32_t Create( CmDeviceEmu* &pDevice );
#endif
    static int32_t Destroy( CmDeviceEmu* &pDevice );

    CM_RT_API int32_t Acquire();
    CM_RT_API int32_t SafeRelease();

#ifdef CM_DX9
    CM_RT_API int32_t GetD3DDeviceManager( IDirect3DDeviceManager9* & pDeviceManager );
#elif defined CM_DX11
    CM_RT_API int32_t GetD3D11Device(ID3D11Device* &pD3D11Device);
#endif

    CM_RT_API int32_t CreateBuffer(uint32_t size, CmBuffer* & pSurface ) override;
    CM_RT_API int32_t CreateSurface2D(uint32_t width, uint32_t height, CM_SURFACE_FORMAT format, CmSurface2D* & pSurface ) override;
    CM_RT_API int32_t CreateSurface3D(uint32_t width, uint32_t height, uint32_t depth, CM_SURFACE_FORMAT format, CmSurface3D* & pSurface ) override;
#if defined(_WIN32)
    CM_RT_API int32_t CreateSurface2D( CM_IDIRECT3DSURFACE* pD3DSurf, CmSurface2D* & pSurface );
    CM_RT_API int32_t CreateSurface2D( CM_IDIRECT3DSURFACE** ppD3DSurf, const uint32_t surfaceCount, CmSurface2D**  ppSurface );
#else
    CM_RT_API int32_t CreateSurface2D( VASurfaceID iVASurface, CmSurface2D* & pSurface ) override;
    CM_RT_API int32_t CreateSurface2D( VASurfaceID* iVASurface, const uint32_t surfaceCount, CmSurface2D** pSurface ) override;
#endif
    CM_RT_API int32_t DestroySurface( CmBuffer* & pSurface) override;
    CM_RT_API int32_t DestroySurface( CmSurface2D* & pSurface) override;
    CM_RT_API int32_t DestroySurface( CmSurface3D* & pSurface) override;

    CM_RT_API int32_t CreateQueue( CmQueue* & pQueue) override;
    CM_RT_API int32_t LoadProgram(void* pCommonISACode, const uint32_t size, CmProgram*& pProgram, const char* options = nullptr ) override;
    CM_RT_API int32_t CreateKernel( CmProgram* pProgram, const char* kernelName, const void * fncPnt, CmKernel* & pKernel, const char* options = nullptr) override;
    CM_RT_API int32_t CreateKernel( CmProgram* pProgram, const char* kernelName, CmKernel* & pKernel, const char* options = nullptr) override;
    CM_RT_API int32_t CreateTask(CmTask *& pKernelArray) override;

    //CM_RT_API int32_t GetCaps(CM_DEVICE_CAP_NAME capName, size_t& capValueSize, void* pCapValue );

    CM_RT_API int32_t CreateThreadSpace( uint32_t width, uint32_t height, CmThreadSpace* & pTS ) override;
    CM_RT_API int32_t CreateThreadGroupSpace( uint32_t thrdSpaceWidth, uint32_t thrdSpaceHeight, uint32_t grpSpaceWidth, uint32_t grpSpaceHeight, CmThreadGroupSpace*& pTGS ) override;

    int32_t GetSurfaceManagerEmu( CmSurfaceManagerEmu* & pSurfaceMgr );

    CM_RT_API int32_t DestroyKernel( CmKernel*& pKernel) override;
    CM_RT_API int32_t DestroyProgram( CmProgram*& pProgram ) override;
    CM_RT_API int32_t DestroyThreadSpace( CmThreadSpace* & pTS ) override;
    CM_RT_API int32_t DestroyTask(CmTask *& pKernelArray) override;
    CM_RT_API int32_t DestroyThreadGroupSpace(CmThreadGroupSpace*& pTGS) override;

    CM_RT_API int32_t GetCaps(CM_DEVICE_CAP_NAME capName, size_t& capValueSize, void* pCapValue ) override;
    CM_RT_API int32_t CreateBufferUP(uint32_t size, void* pSystMem, CmBufferUP* & pSurface ) override;
    CM_RT_API int32_t DestroyBufferUP( CmBufferUP* & pSurface) override;

    CM_RT_API int32_t GetSurface2DInfo(uint32_t width, uint32_t height, CM_SURFACE_FORMAT format, uint32_t & pitch, uint32_t & physicalSize ) override;
    CM_RT_API int32_t CreateSurface2DUP(uint32_t width, uint32_t height, CM_SURFACE_FORMAT format, void* pSysMem, CmSurface2DUP* & pSurface ) override;
    CM_RT_API int32_t DestroySurface2DUP( CmSurface2DUP* & pSurface) override;

    CM_RT_API int32_t SetCaps(CM_DEVICE_CAP_NAME capsName, size_t capValueSize, void* pCapValue) override;
    CM_RT_API int32_t GetVISAVersion(uint32_t& majorVersion, uint32_t& minorVersion) override { return CmNotImplemented(__PRETTY_FUNCTION__); }

    int32_t GetHalMaxValues( CM_HAL_MAX_VALUES* & pHalMaxValues );
    int32_t GetSurfManager( CmSurfaceManagerEmu* &m_pSurfaceMgr);
    int32_t DoCopyAll();
    int32_t DoGPUCopySelect();
    int32_t GetGenPlatform(GfxEmu::Platform::Id& platform );
    bool IsValidSurfaceIndex(uint32_t surfBTI);

    CM_RT_API int32_t InitPrintBuffer(size_t size = CM_DEFAULT_PRINT_BUFFER_SIZE ) override {return CM_SUCCESS; };
    CM_RT_API int32_t FlushPrintBuffer() override { return CM_SUCCESS; };

#ifdef CM_DX11
    CM_RT_API int32_t CreateSurface2DSubresource( ID3D11Texture2D* pD3D11Texture2D, uint32_t subresourceCount, CmSurface2D** ppSurfaces, uint32_t& createdSurfaceCount, uint32_t option ) {return CmNotImplemented(__PRETTY_FUNCTION__);};
    CM_RT_API int32_t CreateSurface2DbySubresourceIndex( ID3D11Texture2D* pD3D11Texture2D, uint32_t FirstArraySlice, uint32_t FirstMipSlice, CmSurface2D* &pSurface){return CmNotImplemented(__PRETTY_FUNCTION__);};
#endif

    CM_RT_API int32_t CreateBufferSVM( uint32_t size, void* & pSystMem, uint32_t access_flag, CmBufferSVM* & pSurface ) override;
    CM_RT_API int32_t DestroyBufferSVM( CmBufferSVM* & pSurface) override;

#ifdef __GNUC__
    CM_RT_API int32_t GetVaDpy(VADisplay* & pva_dpy) override { return CmNotImplemented(__PRETTY_FUNCTION__);};
    CM_RT_API int32_t CreateVaSurface2D( uint32_t width, uint32_t height, CM_SURFACE_FORMAT format, VASurfaceID & iVASurface, CmSurface2D* & pSurface) override{ return CmNotImplemented(__PRETTY_FUNCTION__);};
#endif

    CM_RT_API int32_t CloneKernel( CmKernel * &pKernelDest, CmKernel *pKernelSrc ) override;

    CM_RT_API int32_t CreateSurface2DAlias(CmSurface2D* p2DSurface, SurfaceIndex* &aliasSurfaceIndex) override;
    CM_RT_API int32_t FlushPrintBufferIntoFile(const char *filename) override { return CmNotImplemented(__PRETTY_FUNCTION__); }
    CM_RT_API int32_t CreateThreadGroupSpaceEx(uint32_t thrdSpaceWidth, uint32_t thrdSpaceHeight, uint32_t thrdSpaceDepth, uint32_t grpSpaceWidth, uint32_t grpSpaceHeight, uint32_t grpSpaceDepth, CmThreadGroupSpace*& pTGS) override;

    CM_RT_API int32_t CreateBufferAlias(CmBuffer *pBuffer, SurfaceIndex* &pAliasIndex) override;

    CM_RT_API int32_t CreateQueueEx(CmQueue *&pQueue, CM_QUEUE_CREATE_OPTION QueueCreateOption = CM_DEFAULT_QUEUE_CREATE_OPTION) override;
    CM_RT_API int32_t CreateBufferEx(uint32_t size, CmBuffer* & pSurface,uint32_t id ) override;
    CM_RT_API int32_t DispatchTask() override { return CM_SUCCESS; };

    CM_RT_API int32_t CreateBufferStateless(size_t size, uint32_t option, void *memAddress, CmBufferStateless *&pSurface) override;
    CM_RT_API int32_t DestroyBufferStateless(CmBufferStateless *&pSurface) override;
    CM_RT_API int32_t
        CreateSurface2DStateless(uint32_t width,
                                 uint32_t height,
                                 uint32_t &pitch,
                                 CmSurface2DStateless *&pSurface) override;
    CM_RT_API int32_t
        DestroySurface2DStateless(CmSurface2DStateless *&pSurface) override;

protected:
#ifdef CM_DX9
    int32_t Initialize( IDirect3DDeviceManager9* pD3DDeviceMgr );
#elif defined CM_DX11
    int32_t Initialize( ID3D11Device* pD3DDeviceMgr );
#elif defined __GNUC__
    int32_t Initialize( );
#endif

    std::vector<CmQueueEmu*> m_queueArray;

    CmQueueEmu* m_pQueue;
    CmSurfaceManagerEmu* m_pSurfaceMgr;

#ifdef CM_DX9
    IDirect3DDeviceManager9* m_pD3DDeviceMgr;
#elif defined CM_DX11
    ID3D11Device* m_pD3DDeviceMgr;
#endif

    CmDynamicArray m_ProgramArray;
    uint32_t m_ProgramCount;
    CmDynamicArray m_KernelArray;
    uint32_t m_KernelCount;
    int32_t  m_tileCount;

    CmDeviceEmu( );
    ~CmDeviceEmu( ) override;

    int32_t DestroyQueue( CmQueue* & pQueue );

    // synchronization objects
    CSync m_CriticalSection_Program;
    CSync m_CriticalSection_Kernel;
    CSync m_CriticalSection_Surface;
    CSync m_CriticalSection_DeviceRefCount;

    CM_HAL_MAX_VALUES m_HalMaxValues;
    CM_HAL_MAX_VALUES_EX m_HalMaxValuesEx;
    uint32_t m_refcount;

};
