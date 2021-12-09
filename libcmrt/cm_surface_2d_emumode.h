/*========================== begin_copyright_notice ============================

Copyright (C) 2017 Intel Corporation

SPDX-License-Identifier: MIT

============================= end_copyright_notice ===========================*/

#pragma once

#include "cm_surface_emumode.h"
#include "cm_memory_object_control.h"
#include "cm_surface_2d_base.h"
#include "cm_surface_2d_up_base.h"
#include "emu_log.h"

class CmSurface2D;
class CmSurface2DUP;
class CmSurfaceEmu;

    /*
        typedef enum _CmSurfacePlaneIndex_
{
    GENX_SURFACE_Y_PLANE     = 0,
    GENX_SURFACE_U_PLANE     = 1,
    GENX_SURFACE_UV_PLANE    = 1,
    GENX_SURFACE_V_PLANE     = 2
} CmSurfacePlaneIndex;
    */

typedef enum _CmSurfacePlaneIndex_ CmSurfacePlaneIndex;
//!
//! CM 2D surface
//!
class CmSurface2DEmu :
    public CmSurfaceEmu,
    public CmSurface2DUP,
    public CmSurface2D
{
public:
    static int32_t Create( uint32_t index, uint32_t sizePerPixel, uint32_t width, uint32_t height, CM_SURFACE_FORMAT osApiSurfaceFormat, CmSurfaceFormatID surfFormat , bool isCmCreated, CmSurface2DEmu* &pSurface, void* &pSysMem, bool dummySurf=false, CmSurfaceManagerEmu* surfaceManager= nullptr);
    CM_RT_API int32_t ReadSurface( unsigned char* pSysMem, CmEvent* pEvent, uint64_t sysMemSize = 0xFFFFFFFFFFFFFFFFULL );
    CM_RT_API int32_t WriteSurface( const unsigned char* pSysMem, CmEvent* pEvent, uint64_t sysMemSize = 0xFFFFFFFFFFFFFFFFULL );
    CM_RT_API int32_t ReadSurfaceStride( unsigned char* pSysMem, CmEvent* pEvent, const uint32_t stride, uint64_t sysMemSize = 0xFFFFFFFFFFFFFFFFULL );
    CM_RT_API int32_t WriteSurfaceStride( const unsigned char* pSysMem, CmEvent* pEvent, const uint32_t stride, uint64_t sysMemSize = 0xFFFFFFFFFFFFFFFFULL );
    CM_RT_API int32_t InitSurface(const unsigned long initValue, CmEvent* pEvent);

    CM_RT_API int32_t GetIndex( SurfaceIndex*& pIndex );
    CM_RT_API int32_t SetProperty(CM_FRAME_TYPE frameType) { return CmNotImplemented(__PRETTY_FUNCTION__); }

    CM_RT_API int32_t
    SetSurfaceStateParam(SurfaceIndex *surface_index,
                         const CM_SURFACE2D_STATE_PARAM *state_param);

    int32_t GetArrayIndex( uint32_t& arrayIndex );
    int32_t SetArrayIndex( uint32_t arrayIndex );
#if defined(_WIN32)
    CM_RT_API int32_t GetD3DSurface(CM_IDIRECT3DSURFACE*& pD3DSurface);
    int32_t SetD3DSurface(CM_IDIRECT3DSURFACE *pD3DSurf);
#endif
    uint32_t GetWidth(){return m_width;}
    uint32_t GetHeight() const {return m_height;}
    uint32_t GetSizeperPixel(){return m_sizeperPixel;}
    uint32_t GetDepth(){return 1;}
    void GetSurfaceFormat(CmSurfaceFormatID &surfFormat){ surfFormat =  m_surfFormat;}
    int32_t DoCopy();
    int32_t DoGPUCopy(
bool doD3DCopy=true
);
    int32_t setInternalBuffer(void * buffer);
    void GetD3DFormat(CM_SURFACE_FORMAT &D3DFormat){ D3DFormat =  m_osApiSurfaceFormat;}

#ifdef CM_DX11
    CM_RT_API int32_t QuerySubresourceIndex(uint32_t& FirstArraySlice, uint32_t& FirstMipSlice) {return CmNotImplemented(__PRETTY_FUNCTION__);};
#endif

    CM_RT_API int32_t ReadSurfaceHybridStrides( unsigned char* pSysMem, CmEvent* pEvent, const uint32_t iWidthStride, const uint32_t iHeightStride, uint64_t sysMemSize = 0xFFFFFFFFFFFFFFFFULL, uint32_t uiOption = 0 ) {return CmNotImplemented(__PRETTY_FUNCTION__);}
    CM_RT_API int32_t WriteSurfaceHybridStrides( const unsigned char* pSysMem, CmEvent* pEvent, const uint32_t iWidthStride, const uint32_t iHeightStride, uint64_t sysMemSize = 0xFFFFFFFFFFFFFFFFULL, uint32_t uiOption = 0 ) {return CmNotImplemented(__PRETTY_FUNCTION__);}

#ifndef _WIN32
    CM_RT_API  int32_t GetVaSurfaceID( VASurfaceID  &iVASurface) {return CmNotImplemented(__PRETTY_FUNCTION__);}
#endif

    CM_RT_API int32_t SelectMemoryObjectControlSetting(MEMORY_OBJECT_CONTROL mem_ctrl) { return CmNotImplemented(__PRETTY_FUNCTION__); }

    int32_t CreateSurface2DAlias(SurfaceIndex* & aliasIndex);

    int32_t RegisterAliasSurface(SurfaceIndex* & aliasIndex, const CM_SURFACE2D_STATE_PARAM *surfStateParam);

    std::vector<SurfaceIndex *> GetAliasIndices() { return m_aliasIndices; }

    int32_t GPUCopyForSurface2DAlias();

protected:
    CmSurface2DEmu( uint32_t width, uint32_t height, CM_SURFACE_FORMAT osApiSurfaceFmt, CmSurfaceFormatID surfFormat, bool isCmCreated, uint32_t sizePerPixel,CmSurfaceManagerEmu* surfaceManager);
    ~CmSurface2DEmu( void );

    int32_t Initialize( uint32_t index, void* &pSysMem, bool dummySurf=false );
    int32_t RegisterSurface(uint32_t index);

    int m_nBuffUsed;
    uint32_t m_arrayIndex;
#if defined(_WIN32)
    CM_IDIRECT3DSURFACE *m_pD3DSurf;
#else
    void *m_pD3DSurf;
#endif
    bool m_dummySurf;
    uint32_t m_sizeperPixel;

    //For alias surface2d
    std::vector<SurfaceIndex *> m_aliasIndices;
    std::map<uint32_t, CM_SURFACE2D_STATE_PARAM> m_aliasSurfaceStates;

    uint32_t m_XOffset;
    uint32_t m_YOffset;
    uint32_t m_newWidth;
    uint32_t m_newHeight;
    CM_SURFACE_FORMAT m_newFormat;

};
