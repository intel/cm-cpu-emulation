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
#include "cm_surface_3d_emumode.h"
#include "cm.h"
#include "cm_mem_fast_copy.h"

extern void
    CM_register_buffer_emu(int buf_id, CmBufferType bclass, void *src, uint width, uint height,
                       CmSurfaceFormatID surfFormat, uint depth, uint pitch);
extern void
    CM_unregister_buffer_emu(SurfaceIndex buf_id, bool copy);

CmSurface3DEmu::CmSurface3DEmu( uint32_t width, uint32_t height, uint32_t depth, CM_SURFACE_FORMAT osApiSurfaceFmt,CmSurfaceFormatID surfFormat , bool isCmCreated ):
    CmSurfaceEmu(isCmCreated, nullptr)
{
    m_width = width;
    m_height = height;
    m_depth = depth;
    m_osApiSurfaceFormat = osApiSurfaceFmt;
    m_surfFormat = surfFormat;
}

int32_t CmSurface3DEmu::Initialize( uint32_t index, uint32_t arrayIndex )
{
    this->m_arrayIndex=arrayIndex;
    printf("%d x %d x %d\n",m_width,m_height,m_depth);
    m_buffer = malloc(m_width * m_height*m_depth);
    printf("buffer: %p\n",m_buffer);
    CmSafeMemSet( m_buffer, 0, m_width * m_height*m_depth );
    this->alloc_dummy = true;

    CM_register_buffer_emu(index, GEN4_INPUT_OUTPUT_BUFFER, m_buffer, m_width, m_height, this->m_surfFormat, m_depth, 0);
    if(CheckStatus(index)==CM_FAILURE)
            return CM_OUT_OF_HOST_MEMORY;

    return CmSurfaceEmu::Initialize( index );

}

int32_t CmSurface3DEmu::Create( uint32_t index, uint32_t arrayIndex, uint32_t width, uint32_t height, uint32_t depth, CM_SURFACE_FORMAT osApiSurfaceFmt, CmSurfaceFormatID surfFormat , bool isCmCreated, CmSurface3DEmu* &pSurface )
{
    int32_t result = CM_SUCCESS;

    pSurface = new CmSurface3DEmu(width, height, depth, osApiSurfaceFmt, surfFormat, isCmCreated );
    if( pSurface )
    {
        result = pSurface->Initialize(index, arrayIndex);
    }
    else
    {
        CmAssert( 0 );
        result = CM_OUT_OF_HOST_MEMORY;
    }

    return result;

}

CM_RT_API int32_t CmSurface3DEmu::WriteSurface( const unsigned char* pSysMem, CmEvent* pEvent, uint64_t sysMemSize )
{
    if(pSysMem == nullptr)
    {
        CmAssert( 0 );
        return CM_INVALID_ARG_VALUE;
    }

    if( sysMemSize < this->m_width * this->m_height * this->m_depth )
    {
        CmAssert( 0 );
        return CM_INVALID_ARG_VALUE;
    }

    CmFastMemCopyFromWC(this->m_buffer, pSysMem, this->m_height*this->m_width*m_depth, GetCpuInstructionLevel());

    return CM_SUCCESS;
}

CM_RT_API int32_t CmSurface3DEmu::ReadSurface( unsigned char* pSysMem , CmEvent* pEvent, uint64_t sysMemSize )
{
    if(pSysMem == nullptr)
    {
        CmAssert( 0 );
        return CM_INVALID_ARG_VALUE;
    }

    if( sysMemSize < this->m_width * this->m_height * this->m_depth )
    {
        CmAssert( 0 );
        return CM_INVALID_ARG_VALUE;
    }

    CmFastMemCopyFromWC(pSysMem, this->m_buffer, this->m_height*this->m_width*m_depth, GetCpuInstructionLevel());

    return CM_SUCCESS;
}

int32_t CmSurface3DEmu::GetArrayIndex( uint32_t& arrayIndex )
{
    arrayIndex = m_arrayIndex;
    return CM_SUCCESS;
}

int32_t CmSurface3DEmu::SetArrayIndex( uint32_t arrayIndex )
{
    m_arrayIndex = arrayIndex;
    return CM_SUCCESS;
}

CM_RT_API int32_t CmSurface3DEmu::GetIndex( SurfaceIndex*& pIndex )
{
    pIndex = m_pIndex;
    return CM_SUCCESS;
}

CmSurface3DEmu::~CmSurface3DEmu(){
    SurfaceIndex* pIndex;
    this->GetIndex(pIndex);
    CM_unregister_buffer_emu(*pIndex,false);
    if(this->m_buffer != nullptr && this->alloc_dummy)
    {
        free(this->m_buffer);
    }
}

CM_RT_API int32_t CmSurface3DEmu::InitSurface(const uint32_t initValue, CmEvent* pEvent)
{
    CmDwordMemSet( m_buffer, initValue, m_height * m_width * m_depth );
    return CM_SUCCESS;
}
