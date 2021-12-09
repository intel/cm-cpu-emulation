/*========================== begin_copyright_notice ============================

Copyright (C) 2017 Intel Corporation

SPDX-License-Identifier: MIT

============================= end_copyright_notice ===========================*/

#include "cm_include.h"
#include "cm_surface_emumode.h"
#include "cm.h"
#include "cm_mem.h"

int32_t CmSurfaceEmu::Destroy( CmSurfaceEmu* &pSurface )
{
    CmSafeRelease( pSurface );
    return CM_SUCCESS;
}

CmSurfaceEmu::CmSurfaceEmu( bool isCmCreated,
                            CmSurfaceManagerEmu* surfaceManager):
    m_IsCmCreated( isCmCreated ),
    m_OriginalWidth( 0 ),
    m_OriginalHeight( 0 ),
    m_surfaceManager(surfaceManager)
{
    m_SMUPSurface=false;
}

CmSurfaceEmu::~CmSurfaceEmu( void )
{
    delete m_pIndex;
}

int32_t CmSurfaceEmu::Initialize( uint32_t index )
{
    // using CM compiler data structure
    m_pIndex = new SurfaceIndex( index );
    if( m_pIndex )
    {
        return CM_SUCCESS;
    }
    else
    {
        return CM_OUT_OF_HOST_MEMORY;
    }

}

//!
//! Each CmSurface(1D/2D/3D) is assigned an index when it is created by the CmDevice.
//! CmDevice keep a mapping b/w index and CmSurface.
//! The index is passed to CM kernel function (genx_main) as argument to indicate surface.
//! INPUT:
//!     Reference to index
//! OUTPUT:
//!     CM_SUCCESS if index is successfully returned
//!
CM_RT_API int32_t CmSurfaceEmu::GetIndex( SurfaceIndex*& pIndex )
{
    pIndex = m_pIndex;
    return CM_SUCCESS;
}

int32_t CmSurfaceEmu::SetIndex( SurfaceIndex* pIndex )
{
    m_pIndex = pIndex;
    return CM_SUCCESS;
}

void * CmSurfaceEmu::getBuffer()
{
    return this->m_buffer;
}
int32_t CmSurfaceEmu::CheckStatus(int buf_id)
{
    cm_list<CmEmulSys::iobuffer>::iterator buff_iter;
    buff_iter = CmEmulSys::search_buffer(buf_id);
    if(buff_iter->p_volatile == nullptr)
    {
        return CM_FAILURE;
    }
    else
    {
        return CM_SUCCESS;
    }
}
int32_t CmSurfaceEmu::DoCopy()
{
    return CM_SUCCESS;
}
int32_t CmSurfaceEmu::DoGPUCopy(
bool doD3DCopy
)
{
    return CM_SUCCESS;
}
