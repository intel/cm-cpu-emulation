/*========================== begin_copyright_notice ============================

Copyright (C) 2017 Intel Corporation

SPDX-License-Identifier: MIT

============================= end_copyright_notice ===========================*/

#include "cm_include.h"
#include "cm_surface_2d_emumode.h"
#include "cm_surface_manager_emumode.h"
#include "cm.h"
#include "cm_mem_fast_copy.h"

#ifdef __GNUC__
extern void
CM_unregister_buffer_emu(SurfaceIndex buf_id, bool copy);
extern void
CM_register_buffer_emu(SurfaceIndex buf_id, CmBufferType bclass, void *src, uint width, uint height,
                       CmSurfaceFormatID surfFormat, uint depth, uint pitch);

#endif

CmSurface2DEmu::CmSurface2DEmu( uint32_t width, uint32_t height,CM_SURFACE_FORMAT osApiSurfaceFmt,
                               CmSurfaceFormatID surfFormat , bool isCmCreated, uint32_t sizePerPixel,
                               CmSurfaceManagerEmu* surfaceManager):
    CmSurfaceEmu(isCmCreated, surfaceManager),
    m_pD3DSurf(nullptr),
    m_sizeperPixel(sizePerPixel)
{
    m_width = width;
    m_height = height;
    m_osApiSurfaceFormat = osApiSurfaceFmt;
    m_surfFormat = surfFormat;

    m_XOffset = 0;
    m_YOffset = 0;
    m_newWidth = width;
    m_newHeight = height;
    m_newFormat = osApiSurfaceFmt;
}

int32_t CmSurface2DEmu::Initialize( uint32_t index, void* &pSysMem, bool dummySurf )
{
    m_dummySurf=dummySurf;

    m_OriginalWidth = m_width*m_sizeperPixel;
    m_OriginalHeight = m_height;
    uint32_t temp = 0;
    switch( this->m_osApiSurfaceFormat )
    {
    case CM_SURFACE_FORMAT_NV12:
#if 0
        temp=(int)ceil((double)m_width/64);
        m_width=temp*64*m_sizeperPixel;
#endif
        //set UV plane's height to roundup(m_height/2) for supporting NV12 format with odd height(DX9 only)
        m_height += (m_height + 1)/2;
        break;
#if defined(_WIN32)
    case CM_SURFACE_FORMAT_P016:
    case CM_SURFACE_FORMAT_P010:
         m_width *= m_sizeperPixel;
         m_height += m_height/2;
         break;
#endif
    default:
        m_width *= m_sizeperPixel;
    }
    m_newWidth = m_width;
    this->m_arrayIndex=index;
    //Dummy surface no need to register it or waste space allocating internal buffer.
    if(dummySurf)
    {
        alloc_dummy = false;
        return CmSurfaceEmu::Initialize( index );
    }
    if(pSysMem != nullptr)
    {
        m_buffer = pSysMem;
        this->alloc_dummy = false;
    }else
    {
        m_buffer = malloc(m_width * m_height);
        if(m_buffer == nullptr)
        {
            GFX_EMU_ASSERT( 0 );
            return CM_OUT_OF_HOST_MEMORY;
        }
        CmSafeMemSet( m_buffer, 0, m_width * m_height );
        this->alloc_dummy = true;
        pSysMem = m_buffer;
    }
    int32_t status = this->RegisterSurface(index);
    if(status != CM_SUCCESS)
        return status;

    return CmSurfaceEmu::Initialize( index );

}

int32_t CmSurface2DEmu::RegisterSurface(uint32_t index)
{
    SurfaceIndex *pind1 = new SurfaceIndex(index);
    SurfaceIndex *pind2 = nullptr;
    SurfaceIndex *pind3 = nullptr;
    switch( this->m_osApiSurfaceFormat )
    {
    case CM_SURFACE_FORMAT_NV12:
#if defined(_WIN32)
    case CM_SURFACE_FORMAT_P016:
    case CM_SURFACE_FORMAT_P010:
#endif
        {
#if 0
        CM_register_buffer_emu(*pind1, GEN4_INPUT_OUTPUT_BUFFER, m_buffer, m_width, m_height*2/3, this->m_surfFormat);
        if(CheckStatus(pind1->get_data())==CM_FAILURE)
            return CM_OUT_OF_HOST_MEMORY;
        pind2 = new SurfaceIndex(index+GENX_SURFACE_UV_PLANE);
        CM_register_buffer_emu(index+GENX_SURFACE_UV_PLANE, GEN4_INPUT_OUTPUT_BUFFER, (char *)m_buffer+m_width*m_height*2/3, m_width, m_height-m_height*2/3, this->m_surfFormat);
        if(CheckStatus(pind2->get_data())==CM_FAILURE)
            return CM_OUT_OF_HOST_MEMORY;
        delete pind2;
#else
        cm_list<CmEmulSys::iobuffer>::iterator buff_iter = nullptr;
        CM_register_buffer_emu(*pind1, GEN4_INPUT_OUTPUT_BUFFER, m_buffer, m_width, m_height, this->m_surfFormat, 1, 0);
        if (CheckStatus(pind1->get_data()) == CM_FAILURE)
        {
            delete pind1;
            return CM_OUT_OF_HOST_MEMORY;
        }

        buff_iter = CmEmulSys::search_buffer(pind1->get_data());
        buff_iter->height = m_height*2/3;
        if (buff_iter->p_volatile == nullptr)
        {
            delete pind1;
            return CM_FAILURE;
        }

        // Let's consider the pre/post execution copy as legacy
        // and comment it out for now.
        //CmFastMemCopyFromWC((char *)buff_iter->p_volatile + m_width * buff_iter->height, (char *)m_buffer + m_width * buff_iter->height,
        //    this->m_width, GetCpuInstructionLevel());

        //UV plane
        CmEmulSys::iobuffer new_buff;

        new_buff.id = index+GENX_SURFACE_UV_PLANE;
        new_buff.bclass = GEN4_INPUT_OUTPUT_BUFFER;
        new_buff.pixelFormat = R16_UNORM;
        new_buff.p = (char *)m_buffer + m_width * buff_iter->height;
        new_buff.width = m_width;
        new_buff.height = m_height - buff_iter->height;
        new_buff.depth = 1;
        new_buff.pitch = m_width;

        new_buff.p_volatile = (char *) buff_iter->p_volatile + m_width * buff_iter->height;
        CmEmulSys::iobuffers.add(new_buff);

#endif
        m_nBuffUsed=2;
        break;
        }
    default:
        if(this->m_osApiSurfaceFormat == CM_SURFACE_FORMAT_YUY2)
        {
            m_surfFormat = YCRCB_NORMAL;
        }else if (m_osApiSurfaceFormat == CM_SURFACE_FORMAT_UYVY)
        {
            m_surfFormat = YCRCB_SWAPY;
        }

        CM_register_buffer_emu(*pind1, GEN4_INPUT_OUTPUT_BUFFER, m_buffer, m_width, m_height, this->m_surfFormat, 1, 0);
        if (CheckStatus(pind1->get_data()) == CM_FAILURE)
        {
            delete pind1;
            return CM_OUT_OF_HOST_MEMORY;
        }
        m_nBuffUsed=1;
        break;
    }
    delete pind1;
    return CM_SUCCESS;
}

#if defined(_WIN32)
#ifdef CM_DX9
int32_t CmSurface2DEmu::SetD3DSurface(IDirect3DSurface9 *pD3DSurf)
{
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
    this->m_SMUPSurface=true;
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
}
#elif defined CM_DX11
int32_t CmSurface2DEmu::SetD3DSurface(ID3D11Texture2D *pD3DSurf)
{
    if (this->m_pD3DSurf == nullptr)
    {
        GFX_EMU_ASSERT(0);
        return CM_NULL_POINTER;
    }
    ID3D11Device *pD3D11Device = nullptr;
    ID3D11DeviceContext* pD3D11DeviceContext = nullptr;
    pD3D11Device->GetImmediateContext(&pD3D11DeviceContext);
    D3D11_MAPPED_SUBRESOURCE MappedResource;
    HRESULT hRes = pD3D11DeviceContext->Map((ID3D11Resource*)m_pD3DSurf, 0, D3D11_MAP_READ, 0, &MappedResource);
    if( hRes != D3D_OK )
    {
        GFX_EMU_ERROR_MESSAGE("Fail to map a surface!");
        GFX_EMU_ASSERT( 0 );
        return CM_LOCK_SURFACE_FAIL;
    }
    uint8_t *pSurf = ( uint8_t *)MappedResource.pData;
    uint8_t *pDst = ( uint8_t *)this->m_buffer;
    CmFastMemCopyFromWC(pDst, pSurf, this->m_width * this->m_height, GetCpuInstructionLevel());
    pD3D11DeviceContext->Unmap((ID3D11Resource*)m_pD3DSurf, 0);
    return CM_SUCCESS;

}
#endif
#endif

int32_t CmSurface2DEmu::Create( uint32_t index, uint32_t sizePerPixel, uint32_t width, uint32_t height, CM_SURFACE_FORMAT osApiSurfaceFmt, CmSurfaceFormatID surfFormat , bool isCmCreated, CmSurface2DEmu* &pSurface, void* &pSysMem, bool dummySurf, CmSurfaceManagerEmu* surfaceManager)
{
    int32_t result = CM_SUCCESS;

    pSurface = new CmSurface2DEmu(width, height, osApiSurfaceFmt, surfFormat, isCmCreated, sizePerPixel, surfaceManager);
    if( pSurface )
    {
        result = pSurface->Initialize(index,pSysMem, dummySurf);
    }
    else
    {
        GFX_EMU_ASSERT( 0 );
        result = CM_OUT_OF_HOST_MEMORY;
    }

    return result;
}

CM_RT_API int32_t CmSurface2DEmu::WriteSurface( const unsigned char* pSysMem, CmEvent* pEvent, uint64_t sysMemSize )
{
    int index=0;

    if(pSysMem == nullptr)
    {
        GFX_EMU_ASSERT( 0 );
        return CM_INVALID_ARG_VALUE;
    }

    if( sysMemSize < this->m_width * this->m_height )
    {
        GFX_EMU_ASSERT( 0 );
        return CM_INVALID_ARG_VALUE;
    }

    unsigned char* temp_buffer=(unsigned char*)m_buffer;
    if(m_width != m_OriginalWidth)
    {
        for (uint32_t i=0; i < this->m_height; i++)
        {
            CmSafeMemCopy( temp_buffer, pSysMem, this->m_OriginalWidth );
            temp_buffer += m_width;
            pSysMem += this->m_OriginalWidth;
        }
    }else
    CmSafeMemCopy( this->m_buffer, pSysMem, this->m_height*this->m_width );

#if defined(_WIN32)
#ifdef CM_DX9
    if(this->m_pD3DSurf != nullptr)
    {
        D3DSURFACE_DESC desc;
        HRESULT hRes = m_pD3DSurf->GetDesc( &desc );
        D3DLOCKED_RECT rect;
        hRes = m_pD3DSurf->LockRect( &rect, nullptr, D3DLOCK_DISCARD );
        if( hRes != D3D_OK )
        {
            GFX_EMU_ERROR_MESSAGE("Fail to lock a surface!");
            GFX_EMU_ASSERT( 0 );
            return CM_FAILURE;
        }
        this->m_SMUPSurface=true;
        uint32_t pitch = rect.Pitch;

        uint8_t *pSurf = ( uint8_t *)rect.pBits;
        uint8_t *pSrc = ( uint8_t *)this->m_buffer;
        for (uint32_t i=0; i < this->m_height; i++)
        {
            CmFastMemCopyFromWC(pSurf, pSrc, m_width, GetCpuInstructionLevel());
            pSurf += pitch;
            pSrc += this->m_width;
        }

        hRes = m_pD3DSurf->UnlockRect();
        if( hRes != D3D_OK )
        {
            GFX_EMU_ERROR_MESSAGE("Fail to unlock a surface!");
            GFX_EMU_ASSERT( 0 );
            return CM_FAILURE;
        }
    }
#elif defined CM_DX11
    if(this->m_pD3DSurf != nullptr)
    {
        ID3D11Device *pD3D11Device = nullptr;
        ID3D11DeviceContext* pD3D11DeviceContext = nullptr;
        pD3D11Device->GetImmediateContext(&pD3D11DeviceContext);
        D3D11_MAPPED_SUBRESOURCE MappedResource;
        HRESULT hRes = pD3D11DeviceContext->Map((ID3D11Resource*)m_pD3DSurf, 0, D3D11_MAP_READ, 0, &MappedResource);
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
#endif
#endif

    return DoGPUCopy(
        false
    );
}

CM_RT_API int32_t CmSurface2DEmu::ReadSurface( unsigned char* pSysMem , CmEvent* pEvent, uint64_t sysMemSize )
{
    int index=0;

    if(pSysMem == nullptr)
    {
        GFX_EMU_ASSERT( 0 );
        return CM_INVALID_ARG_VALUE;
    }

    if( sysMemSize < this->m_width * this->m_height )
    {
        GFX_EMU_ASSERT( 0 );
        return CM_INVALID_ARG_VALUE;
    }

    unsigned char* temp_buffer=(unsigned char*)m_buffer;
    if(m_width != m_OriginalWidth)
    {
        for (uint32_t i=0; i < this->m_height; i++)
        {
            CmSafeMemCopy( pSysMem, temp_buffer,  this->m_OriginalWidth );
            temp_buffer += m_width;
            pSysMem += this->m_OriginalWidth;
        }
    }else
        CmFastMemCopyFromWC(pSysMem, this->m_buffer, this->m_height*this->m_width, GetCpuInstructionLevel());

    return CM_SUCCESS;
}

int32_t CmSurface2DEmu::GetArrayIndex( uint32_t& arrayIndex )
{
    arrayIndex = m_arrayIndex;
    return CM_SUCCESS;
}

int32_t CmSurface2DEmu::SetArrayIndex( uint32_t arrayIndex )
{
    m_arrayIndex = arrayIndex;
    return CM_SUCCESS;
}

CM_RT_API int32_t CmSurface2DEmu::GetIndex( SurfaceIndex*& pIndex )
{
    pIndex = m_pIndex;
    return CM_SUCCESS;
}

#if defined(_WIN32)
CM_RT_API int32_t CmSurface2DEmu::GetD3DSurface(CM_IDIRECT3DSURFACE* &pD3DSurface)
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
CmSurface2DEmu::~CmSurface2DEmu(){
    int index=0;
    SurfaceIndex* pIndex;
    this->GetIndex(pIndex);
    index = pIndex->get_data();

    if(CM_SURFACE_FORMAT_NV12 == this->m_osApiSurfaceFormat
#if defined(_WIN32)
       || CM_SURFACE_FORMAT_P010 == this->m_osApiSurfaceFormat ||
       CM_SURFACE_FORMAT_P016 == this->m_osApiSurfaceFormat
#endif
       )
    {
        cm_list<CmEmulSys::iobuffer>::iterator buff_iter = CmEmulSys::search_buffer(index + GENX_SURFACE_UV_PLANE);
        CmEmulSys::iobuffers.remove(buff_iter);
        CM_unregister_buffer_emu(*pIndex, false);
    }else
    {
        for(int i=0;i<m_nBuffUsed&&!m_dummySurf;i++)
        {
            SurfaceIndex* pIndexTemp =  new SurfaceIndex(index+i);;
            CM_unregister_buffer_emu(*pIndexTemp, false);
            delete pIndexTemp;
        }
    }

    if(this->m_buffer != nullptr && this->alloc_dummy)
    {
        free(this->m_buffer);
    }
}

CM_RT_API int32_t CmSurface2DEmu::ReadSurfaceStride( unsigned char* pSysMem, CmEvent* pEvent, const uint32_t stride, uint64_t sysMemSize )
{
    int index=0;
    SurfaceIndex* pIndex;
    if(pSysMem == nullptr)
    {
        GFX_EMU_ASSERT( 0 );
        return CM_INVALID_ARG_VALUE;
    }

    if( sysMemSize < this->m_width * this->m_height )
    {
        GFX_EMU_ASSERT( 0 );
        return CM_INVALID_ARG_VALUE;
    }
    this->GetIndex(pIndex);
    index = pIndex->get_data();
#if 0
    for(int i=0;i<m_nBuffUsed;i++)
    {
        SurfaceIndex *pIndex = new SurfaceIndex(index+i);
        CM_unregister_buffer_emu(*pIndex);
        delete pIndex;
    }
#endif
    unsigned char* tempSrc = (unsigned char *)this->m_buffer;
    if(stride != this->m_width)
    {
        for(unsigned int i = 0; i< this->m_height; i++)
        {
            CmSafeMemCopy( pSysMem, tempSrc, this->m_width );
            tempSrc += this->m_width;
            pSysMem += stride; //assume user knows what they are doing

        }
    }else
    {
        //Data should already be in m_buffer.
        CmFastMemCopyFromWC(pSysMem, this->m_buffer, this->m_height*this->m_width, GetCpuInstructionLevel());
    }

    return CM_SUCCESS;
}

CM_RT_API int32_t CmSurface2DEmu::WriteSurfaceStride( const unsigned char* pSysMem, CmEvent* pEvent, const uint32_t stride, uint64_t sysMemSize )
{
    int index=0;

    if(pSysMem == nullptr)
    {
        GFX_EMU_ASSERT( 0 );
        return CM_INVALID_ARG_VALUE;
    }

    if( sysMemSize < this->m_width * this->m_height )
    {
        GFX_EMU_ASSERT( 0 );
        return CM_INVALID_ARG_VALUE;
    }

    unsigned char* tempDst = (unsigned char *)this->m_buffer;
    if(stride != this->m_width)
    {
        for(unsigned int i = 0; i< this->m_height; i++)
        {
            CmSafeMemCopy( tempDst,  pSysMem, this->m_width );
            tempDst += this->m_width;
            pSysMem += stride; //assume user knows what they are doing

        }
    }else
    {
        //Data should already be in m_buffer.
        CmSafeMemCopy( this->m_buffer, pSysMem, this->m_height*this->m_width );
    }

    return DoGPUCopy();
}

int32_t CmSurface2DEmu::DoCopy()
{
    if(m_dummySurf)
        return CM_SUCCESS;
    cm_list<CmEmulSys::iobuffer>::iterator buff_iter;
    cm_list<CmEmulSys::iobuffer>::iterator buff_iter2;
    cm_list<CmEmulSys::iobuffer>::iterator buff_iter3;

    buff_iter = CmEmulSys::search_buffer(m_pIndex->get_data());
    if(buff_iter->p_volatile == nullptr)
        return CM_FAILURE;

    switch(m_newFormat)
    {
        case CM_SURFACE_FORMAT_NV12:
        case CM_SURFACE_FORMAT_P010:
        case CM_SURFACE_FORMAT_P016:
            // Let's consider the pre/post execution copy as legacy
            // and comment it out for now.
            //for (uint32_t i = 0; i < m_newHeight; i++)
            //{
            //    CmFastMemCopyFromWC((char*)m_buffer + m_XOffset + m_YOffset* m_width + i*m_width,
            //                        (char *)buff_iter->p_volatile + i*m_newWidth,
            //                         m_newWidth,
            //                         GetCpuInstructionLevel());
            //}

            buff_iter2 = CmEmulSys::search_buffer(m_pIndex->get_data()+GENX_SURFACE_UV_PLANE);
            if(buff_iter2->p_volatile == nullptr)
                return CM_FAILURE;

            // Let's consider the pre/post execution copy as legacy
            // and comment it out for now.
            //for (uint32_t i = 0; i < m_newHeight/2; i++)
            //{
            //    CmFastMemCopyFromWC((char*)m_buffer + m_XOffset + m_YOffset* m_width + i*m_width + m_width*m_height*2/3,
            //                        (char *)buff_iter2->p_volatile + i*m_newWidth,
            //                        m_newWidth,
            //                        GetCpuInstructionLevel());
            //}
            break;
        default:
            //gets around issue when two surfaces share the same src surface or d3d surface, and only one is used in the kernel.
            //so value is not over written with stale data.
            // Let's consider the pre/post execution copy as legacy
            // and comment it out for now.
            //if(!memcmp(m_buffer, buff_iter->p_volatile, this->m_width*this->m_height))
            //{
                return CM_SUCCESS;
            //}

            // Let's consider the pre/post execution copy as legacy
            // and comment it out for now.
            //for (uint32_t i = 0; i < m_newHeight; i++)
            //{
            //    CmFastMemCopyFromWC((char*)m_buffer + m_XOffset + m_YOffset* m_width + i*m_width,
            //        (char *)buff_iter->p_volatile + i*m_newWidth,
            //        m_newWidth,
            //        GetCpuInstructionLevel());
            //}
            break;
    }

    //Incase surface was created with D3D surface
#if defined(_WIN32)
#ifdef CM_DX9
    if(this->m_pD3DSurf != nullptr)
    {
        D3DSURFACE_DESC desc;
        HRESULT hRes = m_pD3DSurf->GetDesc( &desc );
        if( hRes != D3D_OK )
        {
            GFX_EMU_ERROR_MESSAGE("Fail to get Descriptor!");
            GFX_EMU_ASSERT( 0 );
            return CM_FAILURE;
        }
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
            CmFastMemCopyFromWC(pSurf, pDst,  this->m_width, GetCpuInstructionLevel());
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
    }
#elif defined CM_DX11
    if(this->m_pD3DSurf != nullptr)
    {
        ID3D11Device *pD3D11Device = nullptr;
        ID3D11DeviceContext* pD3D11DeviceContext = nullptr;
        pD3D11Device->GetImmediateContext(&pD3D11DeviceContext);
        D3D11_MAPPED_SUBRESOURCE MappedResource;
        HRESULT hRes = pD3D11DeviceContext->Map((ID3D11Resource*)m_pD3DSurf, 0, D3D11_MAP_READ, 0, &MappedResource);
        if( hRes != D3D_OK )
        {
            GFX_EMU_ERROR_MESSAGE("Fail to map a surface!");
            GFX_EMU_ASSERT( 0 );
            return CM_LOCK_SURFACE_FAIL;
        }
        uint8_t *pSurf = ( uint8_t *)MappedResource.pData;
        uint8_t *pDst = ( uint8_t *)this->m_buffer;
        CmFastMemCopyFromWC( pSurf, pDst, this->m_width * this->m_height, GetCpuInstructionLevel());
        pD3D11DeviceContext->Unmap((ID3D11Resource*)m_pD3DSurf, 0);
    }
#endif
#endif
    return CM_SUCCESS;
}

int32_t CmSurface2DEmu::DoGPUCopy(
    bool doD3DCopy
)
{
    if(m_dummySurf)
        return CM_SUCCESS;
    cm_list<CmEmulSys::iobuffer>::iterator buff_iter;
    cm_list<CmEmulSys::iobuffer>::iterator buff_iter2;

    buff_iter = CmEmulSys::search_buffer(m_pIndex->get_data());
    if(buff_iter->p_volatile == nullptr)
        return CM_FAILURE;

#if defined(_WIN32)
#ifdef CM_DX9
    if(this->m_pD3DSurf != nullptr && doD3DCopy)
    {
        D3DSURFACE_DESC desc;
        HRESULT hRes = m_pD3DSurf->GetDesc( &desc );
        D3DLOCKED_RECT rect;
        hRes = m_pD3DSurf->LockRect( &rect, nullptr, D3DLOCK_READONLY );
        if( hRes != D3D_OK )
        {
            GFX_EMU_ERROR_MESSAGE("Fail to lock a surface!");
            GFX_EMU_ASSERT( 0 );
            return CM_FAILURE;
        }
        this->m_SMUPSurface=true;
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
    }
#elif CM_DX11
    if(this->m_pD3DSurf != nullptr && doD3DCopy)
    {
        ID3D11Device *pD3D11Device = nullptr;
        ID3D11DeviceContext* pD3D11DeviceContext = nullptr;
        pD3D11Device->GetImmediateContext(&pD3D11DeviceContext);
        D3D11_MAPPED_SUBRESOURCE MappedResource;
        HRESULT hRes = pD3D11DeviceContext->Map((ID3D11Resource*)m_pD3DSurf, 0, D3D11_MAP_READ, 0, &MappedResource);
        if( hRes != D3D_OK )
        {
            GFX_EMU_ERROR_MESSAGE("Fail to map a surface!");
            GFX_EMU_ASSERT( 0 );
            return CM_LOCK_SURFACE_FAIL;
        }
        uint8_t *pSurf = ( uint8_t *)MappedResource.pData;
        uint8_t *pDst = ( uint8_t *)this->m_buffer;
        CmFastMemCopyFromWC( pDst, pSurf, this->m_width * this->m_height, GetCpuInstructionLevel());
        pD3D11DeviceContext->Unmap((ID3D11Resource*)m_pD3DSurf, 0);
    }
#endif
#endif

    switch( m_newFormat)
    {
        case CM_SURFACE_FORMAT_NV12:
        case CM_SURFACE_FORMAT_P010:
        case CM_SURFACE_FORMAT_P016:
            // Let's consider the pre/post execution copy as legacy
            // and comment it out for now.
            //for (uint32_t i = 0; i < m_newHeight; i++)
            //{
            //    CmFastMemCopyFromWC((char *)buff_iter->p_volatile + i*m_newWidth,
            //                        (char*)m_buffer + m_XOffset + m_YOffset* m_width + i*m_width,
            //                        m_newWidth,
            //                        GetCpuInstructionLevel());
            //}

            buff_iter2 = CmEmulSys::search_buffer(m_pIndex->get_data() + GENX_SURFACE_UV_PLANE);
            if (buff_iter2->p_volatile == nullptr)
                return CM_FAILURE;

            // Let's consider the pre/post execution copy as legacy
            // and comment it out for now.
            //for (uint32_t i = 0; i < m_newHeight / 2; i++)
            //{
            //    CmFastMemCopyFromWC((char *)buff_iter2->p_volatile + i*m_newWidth,
            //                        (char*)m_buffer + m_XOffset + m_YOffset/2* m_width + i*m_width + m_width*m_height*2/3,
            //                         m_newWidth,
            //                         GetCpuInstructionLevel());
            //}
            break;
        default:
            // Let's consider the pre/post execution copy as legacy
            // and comment it out for now.
            //for (uint32_t i = 0; i < m_newHeight; i++)
            //{
            //    CmFastMemCopyFromWC((char *)buff_iter->p_volatile + i*m_newWidth,
            //        (char*)m_buffer + m_XOffset + m_YOffset* m_width + i*m_width,
            //        m_newWidth,
            //        GetCpuInstructionLevel());
            //}
            break;
    }

    GPUCopyForSurface2DAlias();

    return CM_SUCCESS;
}

int32_t CmSurface2DEmu::setInternalBuffer(void * buffer)
{
    if(this->alloc_dummy)
    {
        free(m_buffer);
        alloc_dummy = false;
    }
    this->m_buffer = buffer;
    return CM_SUCCESS;
}

CM_RT_API int32_t CmSurface2DEmu::InitSurface(const unsigned long initValue, CmEvent* pEvent)
{
    unsigned char* temp_buffer=(unsigned char*)m_buffer;
    if(m_width != m_OriginalWidth)
    {
        for (uint32_t i=0; i < m_height; i++)
        {
            CmDwordMemSet( temp_buffer, initValue, m_OriginalWidth );
            temp_buffer += m_width;
        }
    }
    else
    {
        CmDwordMemSet( m_buffer, initValue, m_height*m_width );
    }

#if defined(_WIN32)
#ifdef CM_DX9
    if(this->m_pD3DSurf != nullptr)
    {
        D3DSURFACE_DESC desc;
        HRESULT hRes = m_pD3DSurf->GetDesc( &desc );
        D3DLOCKED_RECT rect;
        hRes = m_pD3DSurf->LockRect( &rect, nullptr, D3DLOCK_DISCARD );
        if( hRes != D3D_OK )
        {
            GFX_EMU_ERROR_MESSAGE("Fail to lock a surface!");
            GFX_EMU_ASSERT( 0 );
            return CM_FAILURE;
        }
        this->m_SMUPSurface=true;
        uint32_t pitch = rect.Pitch;

        uint8_t *pSurf = ( uint8_t *)rect.pBits;
        uint8_t *pSrc = ( uint8_t *)m_buffer;
        for (uint32_t i=0; i < m_height; i++)
        {
            CmFastMemCopyFromWC(pSurf, pSrc, m_width, GetCpuInstructionLevel());
            pSurf += pitch;
            pSrc += m_width;
        }

        hRes = m_pD3DSurf->UnlockRect();
        if( hRes != D3D_OK )
        {
            GFX_EMU_ERROR_MESSAGE("Fail to unlock a surface!");
            GFX_EMU_ASSERT( 0 );
            return CM_FAILURE;
        }
    }
#elif defined CM_DX11
    //To Do: Fill this in when DX11 Write Surface is complete
#endif
#endif // _WIN32

    return DoGPUCopy(
false
    );
}

int32_t CmSurface2DEmu::CreateSurface2DAlias(SurfaceIndex* & aliasIndex)
{
    uint32_t newIndex = 0;
    uint32_t additionalIncrement = 0;
    if (m_aliasIndices.size() < CM_MAX_NUM_2D_ALIASES)
    {
        m_surfaceManager->getBytesPerPixel(m_osApiSurfaceFormat, &additionalIncrement);
        m_surfaceManager->findFreeIndex(additionalIncrement, newIndex);
        m_surfaceManager->SetSurfaceArrayElement(newIndex, this);
        for (uint i = 1; i < additionalIncrement; i++)
        {
            m_surfaceManager->SetSurfaceArrayDummy(newIndex+i);
        }
        aliasIndex = new SurfaceIndex(newIndex);
        m_aliasIndices.push_back(aliasIndex);
        m_surfaceManager->AddToAliasIndexTable(newIndex);
        return CM_SUCCESS;
    }
    else
    {
        return CM_EXCEED_MAX_NUM_BUFFER_ALIASES;
    }
}

int32_t CmSurface2DEmu::RegisterAliasSurface(SurfaceIndex* & aliasIndex,
                                             const CM_SURFACE2D_STATE_PARAM *surfStateParam)
{

    if (surfStateParam == nullptr)
    {
        GFX_EMU_ASSERT(0);
        return CM_NULL_POINTER;
    }

    uint32_t additionalIncrement = 0;
    uint32_t bytesPerPixel = 0;
    CM_SURFACE_FORMAT format = m_osApiSurfaceFormat;
    if (surfStateParam->format != 0)
    {
        format = (CM_SURFACE_FORMAT)surfStateParam->format;
    }

    CmSurfaceFormatID surfFormat = m_surfaceManager->ConvertOsFmtToSurfFmt(format);

    bytesPerPixel = m_surfaceManager->getBytesPerPixel(format, &additionalIncrement);

    SurfaceIndex *surfIndex = aliasIndex;
    if (surfIndex == nullptr)
    {
        surfIndex = m_pIndex;

        m_XOffset = surfStateParam->surface_x_offset;
        m_YOffset = surfStateParam->surface_y_offset;
        m_newWidth = surfStateParam->width*bytesPerPixel;
        m_newHeight = surfStateParam->width;
        m_newFormat = format;
    }
    else
    {
        CM_SURFACE2D_STATE_PARAM aliasSurfaceState;
        CmFastMemCopy(&aliasSurfaceState, surfStateParam, sizeof(CM_SURFACE2D_STATE_PARAM));
        aliasSurfaceState.format = format;
        aliasSurfaceState.width = surfStateParam->width*bytesPerPixel;
        std::pair<uint32_t, CM_SURFACE2D_STATE_PARAM> newElement(surfIndex->get_data(), aliasSurfaceState);
        m_aliasSurfaceStates.insert(newElement);
    }

    switch (format)
    {
        case CM_SURFACE_FORMAT_NV12:
        case CM_SURFACE_FORMAT_P016:
        case CM_SURFACE_FORMAT_P010:
        {
            cm_list<CmEmulSys::iobuffer>::iterator buff_iter = nullptr;
            CM_register_buffer_emu(*surfIndex, GEN4_INPUT_OUTPUT_BUFFER,
                                  (char*)m_buffer + surfStateParam->surface_x_offset + surfStateParam->surface_y_offset*m_width,
                                   surfStateParam->width*bytesPerPixel, surfStateParam->height, surfFormat, 1, 0);
            buff_iter = CmEmulSys::search_buffer(surfIndex->get_data());

            // Let's consider the pre/post execution copy as legacy
            // and comment it out for now.
            //for (uint i = 0; i < surfStateParam->height; i++)
            //{
            //    CmFastMemCopyFromWC((char *)buff_iter->p_volatile + i*surfStateParam->width*bytesPerPixel,
            //                        (char*)m_buffer + surfStateParam->surface_x_offset + surfStateParam->surface_y_offset*m_width + i*m_width,
            //                        surfStateParam->width*bytesPerPixel,
            //                        GetCpuInstructionLevel());
            //}
            buff_iter->p =  buff_iter->p_volatile;
            //UV plane

            SurfaceIndex *surfIndexUV = new SurfaceIndex(surfIndex->get_data() + GENX_SURFACE_UV_PLANE);

            CM_register_buffer_emu(*surfIndexUV, GEN4_INPUT_OUTPUT_BUFFER,
                (char*)m_buffer + surfStateParam->surface_x_offset + surfStateParam->surface_y_offset*m_width + m_width*m_height*2/3,
                surfStateParam->width*bytesPerPixel, surfStateParam->height/2, R16_UNORM, 1, 0);

            cm_list<CmEmulSys::iobuffer>::iterator buff_iterUV = CmEmulSys::search_buffer(surfIndexUV->get_data());

            // Let's consider the pre/post execution copy as legacy
            // and comment it out for now.
            //for (uint i = 0; i < surfStateParam->height/2; i++)
            //{
            //    CmFastMemCopyFromWC((char *)buff_iterUV->p_volatile + i*surfStateParam->width*bytesPerPixel,
            //        (char*)m_buffer + surfStateParam->surface_x_offset + surfStateParam->surface_y_offset/2*m_width + i*m_width + m_width*m_height * 2 / 3,
            //        surfStateParam->width*bytesPerPixel,
            //        GetCpuInstructionLevel());
            //}
            buff_iterUV->p = buff_iterUV->p_volatile;
            delete surfIndexUV;
            break;
        }
        default:
            cm_list<CmEmulSys::iobuffer>::iterator buff_iter = nullptr;
            CM_register_buffer_emu(*surfIndex, GEN4_INPUT_OUTPUT_BUFFER,
                (char*)m_buffer + surfStateParam->surface_x_offset + surfStateParam->surface_y_offset*surfStateParam->width*bytesPerPixel,
                surfStateParam->width*bytesPerPixel, surfStateParam->height, surfFormat, 1, 0);
            buff_iter = CmEmulSys::search_buffer(surfIndex->get_data());
            buff_iter->p = buff_iter->p_volatile;
            // Let's consider the pre/post execution copy as legacy
            // and comment it out for now.
            //for (uint i = 0; i < surfStateParam->height; i++)
            //{
            //    CmFastMemCopyFromWC((char *)buff_iter->p_volatile + i*surfStateParam->width*bytesPerPixel,
            //        (char*)m_buffer + surfStateParam->surface_x_offset + surfStateParam->surface_y_offset*m_width + i*m_width,
            //        surfStateParam->width*bytesPerPixel,
            //        GetCpuInstructionLevel());
            //}
            break;
        }

    return CM_SUCCESS;
}

CM_RT_API int32_t CmSurface2DEmu::SetSurfaceStateParam(SurfaceIndex *surfIndex,
                                const CM_SURFACE2D_STATE_PARAM *surfStateParam)
{
    if (surfStateParam == nullptr)
    {
        GFX_EMU_ASSERT(0);
        return CM_NULL_POINTER;
    }

    RegisterAliasSurface(surfIndex, surfStateParam);

    return CM_SUCCESS;
}

int32_t CmSurface2DEmu::GPUCopyForSurface2DAlias()
{
    cm_list<CmEmulSys::iobuffer>::iterator buff_iter;
    cm_list<CmEmulSys::iobuffer>::iterator buff_iter2;
    for (uint i = 0; i < m_aliasIndices.size(); i++)
    {
        std::map<uint32_t, CM_SURFACE2D_STATE_PARAM>::iterator iter
            = m_aliasSurfaceStates.find(m_aliasIndices[i]->get_data());
        if (m_aliasSurfaceStates.end() == iter)
        {
            return CM_SUCCESS;
        }
        else
        {
            buff_iter = CmEmulSys::search_buffer(m_aliasIndices[i]->get_data());
            switch (iter->second.format)
            {
                case CM_SURFACE_FORMAT_NV12:
                case CM_SURFACE_FORMAT_P010:
                case CM_SURFACE_FORMAT_P016:
                    // Let's consider the pre/post execution copy as legacy
                    // and comment it out for now.
                    //for (uint32_t i = 0; i < iter->second.height; i++)
                    //{
                    //    CmFastMemCopyFromWC((char *)buff_iter->p_volatile + i*iter->second.width,
                    //        (char*)m_buffer + iter->second.surface_x_offset + iter->second.surface_y_offset* m_width + i*m_width,
                    //        iter->second.width,
                    //        GetCpuInstructionLevel());
                    //}

                    buff_iter2 = CmEmulSys::search_buffer(m_aliasIndices[i]->get_data() + GENX_SURFACE_UV_PLANE);
                    if (buff_iter2->p_volatile == nullptr)
                        return CM_FAILURE;

                    // Let's consider the pre/post execution copy as legacy
                    // and comment it out for now.
                    //for (uint32_t i = 0; i < iter->second.height/2; i++)
                    //{
                    //    CmFastMemCopyFromWC((char *)buff_iter2->p_volatile + i*iter->second.width,
                    //        (char*)m_buffer + iter->second.surface_x_offset + iter->second.surface_y_offset/2 * m_width + i*m_width + m_width*m_height *2/3,
                    //        iter->second.width,
                    //        GetCpuInstructionLevel());
                    //}
                    break;
                default:
                    // Let's consider the pre/post execution copy as legacy
                    // and comment it out for now.
                    //for (uint32_t i = 0; i < iter->second.height; i++)
                    //{
                    //    CmFastMemCopyFromWC((char *)buff_iter->p_volatile + i*iter->second.width,
                    //        (char*)m_buffer + iter->second.surface_x_offset + iter->second.surface_y_offset* m_width + i*m_width,
                    //        iter->second.width,
                    //        GetCpuInstructionLevel());
                    //}
                    break;
            }

        }

    }
    return CM_SUCCESS;
}
