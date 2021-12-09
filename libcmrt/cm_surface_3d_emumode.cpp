/*========================== begin_copyright_notice ============================

Copyright (C) 2017 Intel Corporation

SPDX-License-Identifier: MIT

============================= end_copyright_notice ===========================*/

#include "cm_include.h"
#include "cm_surface_3d_emumode.h"
#include "cm.h"
#include "cm_mem_fast_copy.h"
#include "cm_mem.h"

#ifdef __GNUC__
extern void
    CM_register_buffer_emu(int buf_id, CmBufferType bclass, void *src, uint width, uint height,
                       CmSurfaceFormatID surfFormat, uint depth, uint pitch);
extern void
    CM_unregister_buffer_emu(SurfaceIndex buf_id, bool copy);
#endif

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

#if defined(_WIN32)
int32_t CmSurface3DEmu::SetD3DSurface(CM_IDIRECT3DSURFACE *pD3DSurf)
{
#ifdef CM_DX9
    int32_t result = CM_SUCCESS;
    D3DSURFACE_DESC desc;
    HRESULT hRes = pD3DSurf->GetDesc( &desc );
    this->m_pD3DSurf = pD3DSurf;
    D3DLOCKED_RECT rect;
    hRes = m_pD3DSurf->LockRect( &rect, nullptr, D3DLOCK_READONLY );
    if( hRes != D3D_OK )
    {
        GFX_EMU_ERROR_MESSAGE("Fail to lock a surface!");
        GFX_EMU_ASSERT( 0 );
        return CM_FAILURE;
    }

    uint32_t pitch = rect.Pitch;

    uint8_t *pSurf = ( uint8_t *)rect.pBits;
    uint8_t *pDst = ( uint8_t *)this->m_buffer;
    for (uint32_t i=0; i < this->m_height; i++)
    {
        CmFastMemCopyFromWC(pDst, pSurf, this->m_width, GetCpuInstructionLevel());
        pSurf += pitch;
        pDst += this->m_width;
    }

    hRes = m_pD3DSurf->UnlockRect();
    if( hRes != D3D_OK )
    {
        GFX_EMU_ERROR_MESSAGE("Fail to unlock a surface!");
        GFX_EMU_ASSERT( 0 );
        return CM_FAILURE;
    }

    return result;
#elif defined CM_DX11
    if(this->m_pD3DSurf != nullptr)
    {
        ID3D11Device *pD3D11Device = nullptr;
        ID3D11DeviceContext* pD3D11DeviceContext = nullptr;
        pD3D11Device->GetImmediateContext(&pD3D11DeviceContext);
        D3D11_MAPPED_SUBRESOURCE MappedResource;
        HRESULT hRes = pD3D11DeviceContext->Map((ID3D11Resource*)m_pD3DSurf, 0, D3D11_MAP_READ_WRITE, 0, &MappedResource);
        if( hRes != D3D_OK )
        {
            GFX_EMU_ERROR_MESSAGE("Fail to map a surface!");
            GFX_EMU_ASSERT( 0 );
            return CM_LOCK_SURFACE_FAIL;
        }
        uint8_t *pSurf = ( uint8_t *)MappedResource.pData;
        uint8_t *pDst = ( uint8_t *)this->m_buffer;
        CmFastMemCopyFromWC(pSurf, pDst, this->m_width * this->m_height, GetCpuInstructionLevel());
        pD3D11DeviceContext->Unmap((ID3D11Resource*)m_pD3DSurf, 0);
    }
    return CM_SUCCESS;
#endif
}
#endif

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
        GFX_EMU_ASSERT( 0 );
        result = CM_OUT_OF_HOST_MEMORY;
    }

    return result;

}

CM_RT_API int32_t CmSurface3DEmu::WriteSurface( const unsigned char* pSysMem, CmEvent* pEvent, uint64_t sysMemSize )
{
    if(pSysMem == nullptr)
    {
        GFX_EMU_ASSERT( 0 );
        return CM_INVALID_ARG_VALUE;
    }

    if( sysMemSize < this->m_width * this->m_height * this->m_depth )
    {
        GFX_EMU_ASSERT( 0 );
        return CM_INVALID_ARG_VALUE;
    }

    CmFastMemCopyFromWC(this->m_buffer, pSysMem, this->m_height*this->m_width*m_depth, GetCpuInstructionLevel());

    return CM_SUCCESS;
}

CM_RT_API int32_t CmSurface3DEmu::ReadSurface( unsigned char* pSysMem , CmEvent* pEvent, uint64_t sysMemSize )
{
    if(pSysMem == nullptr)
    {
        GFX_EMU_ASSERT( 0 );
        return CM_INVALID_ARG_VALUE;
    }

    if( sysMemSize < this->m_width * this->m_height * this->m_depth )
    {
        GFX_EMU_ASSERT( 0 );
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

#if defined(_WIN32)
int32_t CmSurface3DEmu::GetD3DSurface(CM_IDIRECT3DSURFACE* &pD3DSurface)
{
    if(!this->m_IsCmCreated)
        pD3DSurface =  this->m_pD3DSurf;
    else
    {
        pD3DSurface = nullptr;
    }
    return CM_SUCCESS;
}
#endif

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
