/*===================== begin_copyright_notice ==================================

 Copyright (c) 2020, Intel Corporation


 Permission is hereby granted, free of charge, to any person obtaining a
 copy of this software and associated documentation files (the "Software"),
 to deal in the Software without restriction, including without limitation
 the rights to use, copy, modify, merge, publish, distribute, sublicense,
 and/or sell copies of the Software, and to permit persons to whom the
 Software is furnished to do so, subject to the following conditions:

 The above copyright notice and this permission notice shall be included
 in all copies or substantial portions of the Software.

 THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
 OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
 THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR
 OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
 ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
 OTHER DEALINGS IN THE SOFTWARE.
======================= end_copyright_notice ==================================*/

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
)
{
    return CM_SUCCESS;
}
