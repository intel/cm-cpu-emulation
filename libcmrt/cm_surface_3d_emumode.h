/*========================== begin_copyright_notice ============================

Copyright (C) 2017 Intel Corporation

SPDX-License-Identifier: MIT

============================= end_copyright_notice ===========================*/

#pragma once

#include "cm_memory_object_control.h"
#include "cm_surface_3d_base.h"
#include "cm_surface_emumode.h"
#include "emu_log.h"

class CmSurface3D;
class CmSurfaceEmu;

//!
//! CM 3D surface
//!
class CmSurface3DEmu :public CmSurfaceEmu, public CmSurface3D
{
public:
    static int32_t Create( uint32_t index, uint32_t arrayIndex, uint32_t width, uint32_t height, uint32_t depth, CM_SURFACE_FORMAT osApiSurfaceFormat, CmSurfaceFormatID surfFormat , bool isCmCreated, CmSurface3DEmu* &pSurface );
    CM_RT_API int32_t GetIndex(SurfaceIndex*& pIndex);
    CM_RT_API int32_t ReadSurface( unsigned char* pSysMem, CmEvent* pEvent, uint64_t sysMemSize = 0xFFFFFFFFFFFFFFFFULL );
    CM_RT_API int32_t WriteSurface( const unsigned char* pSysMem, CmEvent* pEvent, uint64_t sysMemSize = 0xFFFFFFFFFFFFFFFFULL );
    CM_RT_API int32_t InitSurface(const uint32_t initValue, CmEvent* pEvent);
    CM_RT_API int32_t SelectMemoryObjectControlSetting(MEMORY_OBJECT_CONTROL mem_ctrl) { return CmNotImplemented(__PRETTY_FUNCTION__); }

    int32_t GetArrayIndex( uint32_t& arrayIndex );
    int32_t SetArrayIndex( uint32_t arrayIndex );
#if defined(_WIN32)
    int32_t GetD3DSurface(CM_IDIRECT3DSURFACE*& pD3DSurface);
    int32_t SetD3DSurface(CM_IDIRECT3DSURFACE *pD3DSurf);
#endif
    uint32_t GetWidth(){return m_width;}
    uint32_t GetHeight() const {return m_height;}
    uint32_t GetDepth(){return m_depth;}
    void GetD3DFormat(CM_SURFACE_FORMAT &D3DFormat){ D3DFormat =  m_osApiSurfaceFormat;}
    void GetSurfaceFormat(CmSurfaceFormatID &surfFormat){ surfFormat =  m_surfFormat;}

protected:
    CmSurface3DEmu( uint32_t width, uint32_t height, uint32_t depth, CM_SURFACE_FORMAT osApiSurfaceFmt, CmSurfaceFormatID surfFormat, bool isCmCreated );
    ~CmSurface3DEmu( void );

    int32_t Initialize( uint32_t index, uint32_t arrayIndex );

    uint32_t m_arrayIndex;
#if defined(_WIN32)
    CM_IDIRECT3DSURFACE *m_pD3DSurf;
#endif
};
