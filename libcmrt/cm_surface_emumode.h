/*========================== begin_copyright_notice ============================

Copyright (C) 2017 Intel Corporation

SPDX-License-Identifier: MIT

============================= end_copyright_notice ===========================*/

#pragma once

#if defined(_WIN32)
typedef enum _CmSurfaceFormatID CmSurfaceFormatID;
#else
#include "genx_dataport.h"
#endif

#include "cm_def.h"
#include "cm_priv_def.h"

class SurfaceIndex;

class CmSurface2DEmu;
class CmSurface3DEmu;
class CmSurfaceManagerEmu;

//!
//! CM surface, abstraction of CmBuffer, CmSurface2D, and CmSurface3D.
//!
class CmSurfaceEmu
{
public:
    static int32_t Destroy( CmSurfaceEmu* &pSurface );

    CM_RT_API int32_t GetIndex( SurfaceIndex*& pIndex );
    int32_t SetIndex( SurfaceIndex* pIndex );
    bool IsCmCreated( void ){ return m_IsCmCreated; }
    virtual int32_t CheckStatus(int buf_id);
    virtual int32_t DoCopy();
    virtual int32_t DoGPUCopy(
bool doD3DCopy=true
);
    //emu mode
    void * getBuffer();
    void setISSmUpSurface(){m_SMUPSurface = true;}
    bool getISSmUpSurface(){return m_SMUPSurface;}
    virtual uint32_t GetWidth() = 0;
    virtual uint32_t GetHeight() const = 0;
    virtual uint32_t GetDepth() = 0;
    virtual int32_t GetArrayIndex( uint32_t& arrayIndex )=0;
    virtual int32_t SetArrayIndex( uint32_t arrayIndex )=0;

    CM_STATELESS_SURFACE_TYPE GetStatelessSurfaceType() {
        return m_stateless_surface_type;
    }

    void SetStatelessSurfaceType(CM_STATELESS_SURFACE_TYPE type)
    {
        m_stateless_surface_type = type;
    }

protected:
    CmSurfaceEmu( bool isCmCreated, CmSurfaceManagerEmu* surfaceManager);
    virtual ~CmSurfaceEmu( void );
    virtual int32_t Initialize( uint32_t index );

    SurfaceIndex* m_pIndex;
    // true for 1D and 2D if the internel D3D surface is CM created; false if it is MSDK created
    // always true for 3D
    bool m_IsCmCreated;

    //emu mode
    void *m_buffer;
    bool alloc_dummy; //set to true when surface is created,
                        //when write happens if this is true
                        //memory is freed
    uint32_t m_width;
    uint32_t m_height;
    uint32_t m_depth;
    CmSurfaceFormatID m_surfFormat;
    CM_SURFACE_FORMAT m_osApiSurfaceFormat;
    bool m_SMUPSurface;

    uint32_t m_OriginalWidth;
    uint32_t m_OriginalHeight;

    CmSurfaceManagerEmu* m_surfaceManager;

    CM_STATELESS_SURFACE_TYPE m_stateless_surface_type;
};
