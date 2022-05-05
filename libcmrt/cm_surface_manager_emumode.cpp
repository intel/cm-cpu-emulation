/*========================== begin_copyright_notice ============================

Copyright (C) 2017 Intel Corporation

SPDX-License-Identifier: MIT

============================= end_copyright_notice ===========================*/

#include "cm_include.h"

#include "cm_buffer_emumode.h"
#include "cm_surface_manager_emumode.h"
#include "cm_surface_emumode.h"

#include "cm_buffer_emumode.h"
#include "cm_surface_2d_emumode.h"
#include "cm_surface_3d_emumode.h"
#include "cm.h"
#include "genx_dataport.h"

int32_t CmSurfaceManagerEmu::Create(CmSurfaceManagerEmu *&pManager, CM_HAL_MAX_VALUES HalMaxValues, CM_HAL_MAX_VALUES_EX HalMaxValuesEx)
{
    int32_t result = CM_SUCCESS;

    pManager = new CmSurfaceManagerEmu();

    if( pManager )
    {
        result = pManager->Initialize( HalMaxValues , HalMaxValuesEx );

        if( result != CM_SUCCESS )
        {
            CmSurfaceManagerEmu::Destroy( pManager );
        }

    }
    else
    {
        GFX_EMU_ASSERT( 0 );
        result = CM_OUT_OF_HOST_MEMORY;
    }

    return result;
}

int32_t CmSurfaceManagerEmu::CreateBuffer(uint32_t width, CmBufferEmu* & pSurface,
        void *&pSysMem, bool noRegisterBuffer )
{
    pSurface = nullptr;
    CmSurfaceFormatID surfFormat = R8G8B8A8_UINT;

    uint32_t index = 0;

#if 0
    index = m_SurfaceArray.GetFreeIndex();

    if( index >= this->m_maxSurfaceCount )
    {
        GFX_EMU_ASSERT( 0 );
        return CM_EXCEED_SURFACE_AMOUNT;
    }
#endif

    this->findFreeIndex(0, index);

    if( index >= this->m_maxSurfaceCount )
    {
        GFX_EMU_ASSERT( 0 );
        return CM_EXCEED_SURFACE_AMOUNT;
    }

    if( m_bufferCount >= m_maxBufferCount )
    {
        GFX_EMU_ASSERT( 0 );
        return CM_EXCEED_SURFACE_AMOUNT;
    }

    int32_t result = CmBufferEmu::Create( index, index, width, surfFormat, true,
            pSurface,pSysMem, noRegisterBuffer, this );

    if( result != CM_SUCCESS )
    {
        GFX_EMU_ASSERT( 0 );
        return result;
    }
    else
    {
        m_SurfaceArray.SetElement(index, pSurface);
    }

    m_bufferCount ++;

    return CM_SUCCESS;
}

int32_t CmSurfaceManagerEmu::getBytesPerPixel(CM_SURFACE_FORMAT format, uint32_t *additional_increment)
{
    uint32_t sizePerPixel = 0;
    if(
#if defined(CM_DX11) || defined(__GNUC__)
        ( format != CM_SURFACE_FORMAT_R32_UINT )   &&
        ( format != CM_SURFACE_FORMAT_R32_SINT )   &&
        ( format != CM_SURFACE_FORMAT_R8G8_UNORM ) &&
        ( format != CM_SURFACE_FORMAT_R16_UINT )   &&
        ( format != CM_SURFACE_FORMAT_R16_SINT )   &&
#elif defined(CM_DX9)
        (format != CM_SURFACE_FORMAT_L16) &&
        (format != CM_SURFACE_FORMAT_IRW0) &&
        (format != CM_SURFACE_FORMAT_IRW1) &&
        (format != CM_SURFACE_FORMAT_IRW2) &&
        (format != CM_SURFACE_FORMAT_IRW3) &&
#endif
        ( format != CM_SURFACE_FORMAT_X8R8G8B8 ) &&
        ( format != CM_SURFACE_FORMAT_A8R8G8B8 ) &&
        ( format != CM_SURFACE_FORMAT_R32G32B32A32F ) &&
        ( format != CM_SURFACE_FORMAT_NV12 ) &&
#if defined(_WIN32)
        ( format != CM_SURFACE_FORMAT_P010 ) &&
        ( format != CM_SURFACE_FORMAT_P016 ) &&
#endif
        ( format != CM_SURFACE_FORMAT_YUY2 ) &&
        ( format != CM_SURFACE_FORMAT_UYVY ) &&
        ( format != CM_SURFACE_FORMAT_A8 )&&
        ( format != CM_SURFACE_FORMAT_P8) &&
        ( format != CM_SURFACE_FORMAT_R32F) &&
        ( format != CM_SURFACE_FORMAT_V8U8) &&
        ( format != CM_SURFACE_FORMAT_R10G10B10A2 ) &&
        ( format != CM_SURFACE_FORMAT_A16B16G16R16 ) )
    {
        GFX_EMU_ASSERT( 0 );
        return 0;
    }

    switch( format )
    {
    case CM_SURFACE_FORMAT_R32G32B32A32F:
        sizePerPixel = 16;
        break;
    case CM_SURFACE_FORMAT_A16B16G16R16:
        sizePerPixel = 8;
        break;
    case CM_SURFACE_FORMAT_X8R8G8B8:
    case CM_SURFACE_FORMAT_A8R8G8B8:
    case CM_SURFACE_FORMAT_R32F:
    case CM_SURFACE_FORMAT_R10G10B10A2:
#if defined(CM_DX11) || defined(__GNUC__)
    case CM_SURFACE_FORMAT_R32_UINT:
    case CM_SURFACE_FORMAT_R32_SINT:
#endif
        sizePerPixel = 4;
        break;
    case CM_SURFACE_FORMAT_YUY2:
    case CM_SURFACE_FORMAT_UYVY:
    case CM_SURFACE_FORMAT_V8U8:
#ifdef CM_DX11
    case CM_SURFACE_FORMAT_R8G8_UNORM:
    case CM_SURFACE_FORMAT_R16_UINT:
    case CM_SURFACE_FORMAT_R16_SINT:
#elif defined(CM_DX9)
    case CM_SURFACE_FORMAT_L16:
    case CM_SURFACE_FORMAT_IRW0:
    case CM_SURFACE_FORMAT_IRW1:
    case CM_SURFACE_FORMAT_IRW2:
    case CM_SURFACE_FORMAT_IRW3:
#endif
        sizePerPixel = 2;
        break;
#if defined(_WIN32)
    case CM_SURFACE_FORMAT_P010:
    case CM_SURFACE_FORMAT_P016:
#endif
        sizePerPixel = 2;
        *additional_increment = 1;
        break;
    case CM_SURFACE_FORMAT_A8:
    case CM_SURFACE_FORMAT_P8:
        sizePerPixel = 1;
        break;
    case CM_SURFACE_FORMAT_NV12:
        sizePerPixel = 1;
        *additional_increment=1;
        break;

    default:
        GFX_EMU_ERROR_MESSAGE("Fail to get surface description!");
        GFX_EMU_ASSERT( 0 );
        return -1;
    }

    return sizePerPixel;
}

int32_t CmSurfaceManagerEmu::CreateSurface2D(uint32_t width,
                                             uint32_t height,
                                             CM_SURFACE_FORMAT format,
                                             CmSurface2DEmu* & pCmSurface2D,
                                             void* pSysMem)
{
    pCmSurface2D = nullptr;

    uint32_t sizePerPixel = 0;
    uint32_t additional_increment=0;

    // The index should be the same as the index of this surface in surface registration
    // table in CM device in driver
    // Warning: need to watch surface reistration function change in driver
    // This loop can be avoided if surface registration function can return a index for
    // runtime to use here
    uint32_t index = 0;
#if 0
    index = m_SurfaceArray.GetFreeIndex();
    if( index >= this->m_maxSurfaceCount )
    {
        GFX_EMU_ASSERT( 0 );
        return CM_EXCEED_SURFACE_AMOUNT;
    }
#endif
    sizePerPixel = this->getBytesPerPixel(format, &additional_increment);
    if(sizePerPixel <=0)
    {
        return CM_SURFACE_FORMAT_NOT_SUPPORTED;
    }

    this->findFreeIndex(additional_increment, index);

    if( index +additional_increment >= this->m_maxSurfaceCount )
    {
        GFX_EMU_ASSERT( 0 );
        return CM_EXCEED_SURFACE_AMOUNT;
    }

    if( m_2DSurfaceCount >= m_max2DSurfaceCount )
    {
        GFX_EMU_ASSERT( 0 );
        return CM_EXCEED_SURFACE_AMOUNT;
    }

    int32_t result = Surface2DSanityCheck(width, height, format);
    if (result != CM_SUCCESS)
    {
        GFX_EMU_ASSERT(0);
        return result;
    }

    CmSurfaceFormatID surfFormat = ConvertOsFmtToSurfFmt(format);

    // true indicates pD3DSurf is created by CM
    result = CmSurface2DEmu::Create( index,sizePerPixel, width, height,format, surfFormat, true, pCmSurface2D, pSysMem, false, this );

    //to handle the requirements of the nv12 and imc1/2/3/4 formats
    m_bufferID+=additional_increment;
    if( result != CM_SUCCESS )
    {
        GFX_EMU_ASSERT( 0 );
        return result;
    }
    else
    {
        m_SurfaceArray.SetElement( index , pCmSurface2D);
        for(unsigned int i=1; i<=additional_increment;i++)
        {
            m_SurfaceArray.SetElement( index+i , m_pSurfaceDummy);
        }
    }

    m_2DSurfaceCount ++;

    return CM_SUCCESS;
}

int32_t CmSurfaceManagerEmu::CreateSurface2DUP(uint32_t width, uint32_t height, CM_SURFACE_FORMAT format, CmSurface2DEmu* & pCmSurface2D, void* pSysMem)
{
    pCmSurface2D = nullptr;

    uint32_t sizePerPixel = 0;
    uint32_t additional_increment=0;

    // The index should be the same as the index of this surface in surface registration
    // table in CM device in driver
    // Warning: need to watch surface reistration function change in driver
    // This loop can be avoided if surface registration function can return a index for
    // runtime to use here
    uint32_t index = 0;
    sizePerPixel = this->getBytesPerPixel(format, &additional_increment);
    if(sizePerPixel <=0)
    {
        return CM_SURFACE_FORMAT_NOT_SUPPORTED;
    }

    int32_t result = Surface2DSanityCheck(width, height, format);
    if (result != CM_SUCCESS)
    {
        GFX_EMU_ASSERT(0);
        return result;
    }

    if (nullptr == pSysMem)
    {
        GFX_EMU_ASSERT(0);
        return CM_INVALID_ARG_VALUE;
    }

    this->findFreeIndex(additional_increment, index);

    if( index +additional_increment >= this->m_maxSurfaceCount )
    {
        GFX_EMU_ASSERT( 0 );
        return CM_EXCEED_SURFACE_AMOUNT;
    }

    if( m_2DUPSurfaceCount >= m_max2DUPSurfaceCount )
    {
        GFX_EMU_ASSERT( 0 );
        return CM_EXCEED_SURFACE_AMOUNT;
    }

    CmSurfaceFormatID surfFormat = ConvertOsFmtToSurfFmt(format);

    // true indicates pD3DSurf is created by CM
    result = CmSurface2DEmu::Create( index,sizePerPixel, width, height,format, surfFormat, true, pCmSurface2D, pSysMem, false, this );

    //to handle the requirements of the nv12 and imc1/2/3/4 formats
    m_bufferID+=additional_increment;
    if( result != CM_SUCCESS )
    {
        GFX_EMU_ASSERT( 0 );
        return result;
    }
    else
    {
        m_SurfaceArray.SetElement( index , pCmSurface2D);
        for(unsigned int i=1; i<=additional_increment;i++)
        {
            m_SurfaceArray.SetElement( index+i , m_pSurfaceDummy);
        }
    }

    m_2DUPSurfaceCount ++;

    return CM_SUCCESS;
}

int32_t CmSurfaceManagerEmu::CreateSurface3D(uint32_t width, uint32_t height, uint32_t depth, CM_SURFACE_FORMAT format, CmSurface3DEmu* & pCmSurface3D )
{
    //only these two are supported for 3D
    if( ( format != CM_SURFACE_FORMAT_X8R8G8B8 ) &&
        ( format != CM_SURFACE_FORMAT_R32G32B32A32F ) &&
        ( format != CM_SURFACE_FORMAT_A8R8G8B8 ) )
    {
        GFX_EMU_ASSERT( 0 );
        return CM_SURFACE_FORMAT_NOT_SUPPORTED;
    }

    pCmSurface3D = nullptr;

    uint32_t sizePerPixel = 0;
    uint32_t additional_increment=0;

    // The index should be the same as the index of this surface in surface registration
    // table in CM device in driver
    // Warning: need to watch surface reistration function change in driver
    // This loop can be avoided if surface registration function can return a index for
    // runtime to use here
    uint32_t index = 0;
#if 0
    index = m_SurfaceArray.GetFreeIndex();
    if( index >= this->m_maxSurfaceCount )
    {
        GFX_EMU_ASSERT( 0 );
        return CM_EXCEED_SURFACE_AMOUNT;
    }
#endif
    sizePerPixel = this->getBytesPerPixel(format, &additional_increment);
    if(sizePerPixel <=0)
    {
        return CM_SURFACE_FORMAT_NOT_SUPPORTED;
    }

    this->findFreeIndex(additional_increment, index);

    if( index +additional_increment >= this->m_maxSurfaceCount )
    {
        GFX_EMU_ASSERT( 0 );
        return CM_EXCEED_SURFACE_AMOUNT;
    }

    if( m_3DSurfaceCount >= m_max3DSurfaceCount )
    {
        GFX_EMU_ASSERT( 0 );
        return CM_EXCEED_SURFACE_AMOUNT;
    }

    CmSurfaceFormatID surfFormat = ConvertOsFmtToSurfFmt(format);

    /*
        Vol5c Shared Functions 1.11.4.1.1 SURFACE_STATE
        width: SURFTYPE_3D:  width of surface ?1 (x/u dimension) [0,2047]
        height: SURFTYPE_3D:  height of surface ?1 (y/v dimension) [0,2047]
        depth: SURFTYPE_3D:  depth of surface ?1 (z/r dimension) [0,2047]
    */

    if( ( width < CM_MIN_SURF_WIDTH ) || ( height < CM_MIN_SURF_HEIGHT ) ||
        ( width > CM_MAX_3D_SURF_WIDTH ) ||
        ( height > CM_MAX_3D_SURF_HEIGHT ) ||
        ( depth > CM_MAX_3D_SURF_DEPTH ))
    {
        GFX_EMU_ASSERT(0);
        return CM_INVALID_ARG_VALUE;
    }

    // true indicates pD3DSurf is created by CM
    int32_t result = CmSurface3DEmu::Create( index,index, width*sizePerPixel, height,depth, format, surfFormat, true, pCmSurface3D );

    if( result != CM_SUCCESS )
    {
        GFX_EMU_ASSERT( 0 );
        return result;
    }
    else
    {
        m_SurfaceArray.SetElement( index , pCmSurface3D);
    }

    m_3DSurfaceCount ++;

    return CM_SUCCESS;
}

#if defined(_WIN32)
#ifdef CM_DX9
int32_t CmSurfaceManagerEmu::CreateSurface2D(IDirect3DSurface9 *pD3DSurf, CmSurface2DEmu *&pSurface)
{
    int status = CM_FAILURE;
    int width = 0;
    int height = 0;
    CM_SURFACE_FORMAT format;
    D3DSURFACE_DESC desc;
    HRESULT hRes = pD3DSurf->GetDesc( &desc );
    if( hRes != D3D_OK )
    {
        GFX_EMU_ERROR_MESSAGE("Fail to get surface description!");
        GFX_EMU_ASSERT( 0 );
        return CM_FAILURE;
    }

    width = desc.Width;
    height = desc.Height;
    format = desc.Format;
    status = this->CreateSurface2D(width, height, format, pSurface, false);
    if(status == CM_SUCCESS)
    {
        status = pSurface->SetD3DSurface(pD3DSurf);
    }

    return status;
}
#elif CM_DX11
int32_t CmSurfaceManagerEmu::CreateSurface2D(ID3D11Texture2D *pD3D11Texture, CmSurface2DEmu *&pSurface)
{
    int status = CM_FAILURE;
    int width = 0;
    int height = 0;
    CM_SURFACE_FORMAT format;
    D3D11_TEXTURE2D_DESC desc;
    pD3D11Texture->GetDesc( &desc );

    width = desc.Width;
    height = desc.Height;
    format = desc.Format;
    status = this->CreateSurface2D(width, height, format, pSurface, false);
    if(status == CM_SUCCESS)
    {
        status = pSurface->SetD3DSurface(pD3D11Texture);
    }

    return status;
}
#endif
#endif  //__GNUC__

int32_t CmSurfaceManagerEmu::Initialize( CM_HAL_MAX_VALUES HalMaxValues, CM_HAL_MAX_VALUES_EX HalMaxValuesEx )
{
    uint32_t maxSurfaceCount = HalMaxValues.maxBufferTableSize + HalMaxValues.max2DSurfaceTableSize + HalMaxValues.max3DSurfaceTableSize + HalMaxValuesEx.max2DUPSurfaceTableSize;
    uint32_t maxVirtualSurfaceCount = HalMaxValuesEx.maxSampler8x8TableSize + HalMaxValues.maxSamplerTableSize;
    m_maxSurfaceCount = maxSurfaceCount + maxVirtualSurfaceCount;
    m_maxBufferCount = HalMaxValues.maxBufferTableSize;
    m_max2DSurfaceCount = HalMaxValues.max2DSurfaceTableSize;
    m_max3DSurfaceCount = HalMaxValues.max3DSurfaceTableSize;
    m_max2DUPSurfaceCount = HalMaxValuesEx.max2DUPSurfaceTableSize;

    for(uint32_t i = 0; i<m_maxSurfaceCount; i++)
    {
        this->m_SurfaceArray.SetElement(i,nullptr);
    }
    return CM_SUCCESS;
}
int32_t CmSurfaceManagerEmu::Destroy(CmSurfaceManagerEmu* &pManager)
{
    CmSafeRelease( pManager );

    return CM_SUCCESS;
}
int32_t CmSurfaceManagerEmu::DestroySurface( CmBufferEmu* & pSurface1D )
{
    uint32_t index = 0;
    pSurface1D->GetArrayIndex( index );

    for (auto surface : m_statelessSurfaceArray)
    {
        if (surface == m_SurfaceArray.GetElement(index))
        {
            m_statelessSurfaceArray.erase(surface);
            break;
        }
    }

    GFX_EMU_ASSERT( m_SurfaceArray.GetElement( index ) == pSurface1D );
    m_SurfaceArray.SetElement( index , nullptr );

    std::vector<SurfaceIndex *> aliasIndices = pSurface1D->GetAliasIndices();

    for (std::vector<SurfaceIndex*>::iterator iter = aliasIndices.begin();
        iter != aliasIndices.end();
        ++iter)
    {
        m_SurfaceArray.SetElement((*iter)->get_data(), nullptr);
        RemoveFromAliasIndexTable((*iter)->get_data());
        CM_unregister_buffer_emu(*(*iter), false);
        delete *iter;
    }

    CmSurfaceEmu* pSurface = pSurface1D;
    CmSurfaceEmu::Destroy( pSurface ) ;

    m_bufferCount --;

    return CM_SUCCESS;
}
int32_t CmSurfaceManagerEmu::DestroySurface( CmSurface2DEmu* & pSurface2D )
{
    uint32_t index = 0;
    pSurface2D->GetArrayIndex( index );

    for (auto surface : m_statelessSurfaceArray)
    {
        if (surface == m_SurfaceArray.GetElement(index))
        {
            m_statelessSurfaceArray.erase(surface);
            break;
        }
    }

    GFX_EMU_ASSERT( m_SurfaceArray.GetElement( index ) == pSurface2D );
    m_SurfaceArray.SetElement( index , nullptr);

    if(m_SurfaceArray.GetElement( index+1 ) == m_pSurfaceDummy)
        m_SurfaceArray.SetElement( index+1 , nullptr);

    if(m_SurfaceArray.GetElement( index+2 ) == m_pSurfaceDummy)
        m_SurfaceArray.SetElement( index+2 , nullptr);

    std::vector<SurfaceIndex *> aliasIndices = pSurface2D->GetAliasIndices();

    for (std::vector<SurfaceIndex*>::iterator iter = aliasIndices.begin();
        iter != aliasIndices.end();
        ++iter)
    {
        m_SurfaceArray.SetElement((*iter)->get_data(), nullptr);
        RemoveFromAliasIndexTable((*iter)->get_data());
        CM_unregister_buffer_emu(*(*iter), false);
        if (m_SurfaceArray.GetElement((*iter)->get_data() + 1) == m_pSurfaceDummy)
        {
            SurfaceIndex* pIndexTemp = new SurfaceIndex((*iter)->get_data() + 1);
            CM_unregister_buffer_emu(*pIndexTemp, false);
            delete pIndexTemp;
            m_SurfaceArray.SetElement(index + 1, nullptr);
        }

        if (m_SurfaceArray.GetElement((*iter)->get_data() + 2) == m_pSurfaceDummy)
        {
            SurfaceIndex* pIndexTemp = new SurfaceIndex((*iter)->get_data() + 2);
            CM_unregister_buffer_emu(*pIndexTemp, false);
            delete pIndexTemp;
            m_SurfaceArray.SetElement(index + 2, nullptr);
        }
        delete *iter;
    }

    CmSurfaceEmu* pSurface = pSurface2D;
    CmSurfaceEmu::Destroy( pSurface ) ;

    m_2DSurfaceCount --;

    return CM_SUCCESS;
}
int32_t CmSurfaceManagerEmu::DestroySurface( CmSurface3DEmu* & pSurface3D )
{
    uint32_t index = 0;
    pSurface3D->GetArrayIndex( index );

    GFX_EMU_ASSERT( m_SurfaceArray.GetElement( index ) == pSurface3D );
    m_SurfaceArray.SetElement( index , nullptr);

    CmSurfaceEmu* pSurface = pSurface3D;
    CmSurfaceEmu::Destroy( pSurface ) ;

    m_3DSurfaceCount --;

    return CM_SUCCESS;
}

void CmSurfaceManagerEmu::FindSurfaceArea(uint32_t index, int & leftstep, int & rightstep)
{
    leftstep = 0;
    rightstep = 0;
    CmSurfaceEmu * pSurfaceEmu = (CmSurfaceEmu *)m_SurfaceArray.GetElement(leftstep+index);
    while(pSurfaceEmu == m_pSurfaceDummy && index+leftstep > 0)
    {
        leftstep--;
        pSurfaceEmu = (CmSurfaceEmu *)m_SurfaceArray.GetElement(leftstep+index);
    }

    pSurfaceEmu = (CmSurfaceEmu *)m_SurfaceArray.GetElement(rightstep+index+1);
    while(pSurfaceEmu == m_pSurfaceDummy && index+rightstep+1 < m_maxSurfaceCount-1)
    {
        rightstep++;
        pSurfaceEmu = (CmSurfaceEmu *)m_SurfaceArray.GetElement(rightstep+index+1);
    }
}

int32_t CmSurfaceManagerEmu::MoveSurface( uint32_t src_index, uint32_t dst_index)
{
    int src_left_step = 0;
    int src_right_step = 0;
    int dst_left_step = 0;
    int dst_right_step = 0;
    int additional_increment = 0;
    FindSurfaceArea(src_index, src_left_step, src_right_step);
    FindSurfaceArea(dst_index, dst_left_step, dst_right_step);

    //left-aligned in case dst_index==0(which generated by find_free_index function)
    src_index += src_left_step;
    dst_index += dst_left_step;
    src_right_step -= src_left_step;
    dst_right_step -= dst_left_step;
    additional_increment = src_right_step > dst_right_step?src_right_step:dst_left_step;

    for( int i=0;i<=additional_increment;i++)
    {
        CmSurfaceEmu *pCurSurface = nullptr;
        pCurSurface= (CmSurfaceEmu *)m_SurfaceArray.GetElement( dst_index+i );
        if(pCurSurface != nullptr)
        {
            return CM_FAILURE;
        }
    }

    for( int i=0;i<=additional_increment;i++)
    {
        //change in m_SurfaceArray
        CmSurfaceEmu *pCurSurface = nullptr;
        pCurSurface= (CmSurfaceEmu *)m_SurfaceArray.GetElement( src_index+i );
        m_SurfaceArray.SetElement(dst_index+i,pCurSurface);
        m_SurfaceArray.SetElement(src_index+i,nullptr);

        //change in Surface itself
        SurfaceIndex * pdst_surf_index= new SurfaceIndex(dst_index+i);
        pCurSurface->SetIndex(pdst_surf_index);
        pCurSurface->SetArrayIndex(dst_index+i);

        //change in iobuffers
        SurfaceIndex * psrc_surf_index= new SurfaceIndex(src_index+i);
        CmBufferDescField type=GEN4_FIELD_SURFACE_ID;
        CM_modify_buffer_emu(*psrc_surf_index, type, dst_index+i);
    }

    return CM_SUCCESS;
}

int32_t CmSurfaceManagerEmu::DestroySurface2DUP( CmSurface2DEmu* & pSurface2D )
{
    uint32_t index = 0;
    pSurface2D->GetArrayIndex( index );

    GFX_EMU_ASSERT( m_SurfaceArray.GetElement( index ) == pSurface2D );
    m_SurfaceArray.SetElement( index , nullptr);

    if(m_SurfaceArray.GetElement( index+1 ) == m_pSurfaceDummy)
        m_SurfaceArray.SetElement( index+1 , nullptr);

    if(m_SurfaceArray.GetElement( index+2 ) == m_pSurfaceDummy)
        m_SurfaceArray.SetElement( index+2 , nullptr);

    CmSurfaceEmu* pSurface = pSurface2D;
    CmSurfaceEmu::Destroy( pSurface ) ;

    m_2DUPSurfaceCount --;

    return CM_SUCCESS;
}

CmSurfaceManagerEmu::CmSurfaceManagerEmu()
	:m_SurfaceArray(512),
	m_bufferID(0),
	m_maxBufferCount(0),
	m_bufferCount(0),
	m_max2DSurfaceCount(0),
	m_2DSurfaceCount(0),
	m_max2DUPSurfaceCount(0),
	m_2DUPSurfaceCount(0),
	m_max3DSurfaceCount(0),
	m_3DSurfaceCount(0)
{
    void *dummy = nullptr;
    CmSurface2DEmu::Create( -1,-1, 0, 0, CM_SURFACE_FORMAT_A8R8G8B8, R8G8B8A8_UNORM, true, this->m_pSurfaceDummy,dummy, true, this);
}

CmSurfaceManagerEmu::~CmSurfaceManagerEmu()
{
    for( uint32_t i = 0; i < 512; i ++)
    {
        CmSurfaceEmu* pSurface = (CmSurfaceEmu *)m_SurfaceArray.GetElement( i );
        if( pSurface && m_pSurfaceDummy !=  pSurface )
        {
            CmSurface2DEmu* pSurf2D = dynamic_cast< CmSurface2DEmu* >( pSurface );
            if( pSurf2D )
            {
                DestroySurface( pSurf2D );
            }
            else
            {
                CmBufferEmu* pSurf1D = dynamic_cast< CmBufferEmu* >( pSurface );
                if( pSurf1D )
                {
                    DestroySurface( pSurf1D );
                }
                else
                {
                    CmSurface3DEmu* pSurf3D = dynamic_cast< CmSurface3DEmu* >( pSurface );
                    if( pSurf3D )
                    {
                        DestroySurface( pSurf3D );
                    }else
                    {
#if defined(_WIN32)
#endif /* _WIN32 */
                    }
                }
            }

        }
    }

    CmSurfaceEmu* pSurface = m_pSurfaceDummy;
    CmSurfaceEmu::Destroy( pSurface ) ;

    m_SurfaceArray.Delete();
}

int32_t CmSurfaceManagerEmu::findFreeIndex(uint32_t additional_increment, uint32_t &index)
{
    bool skip_surface_index0 = getenv("SKIP_SURFACE_INDEX0");
    for(uint32_t i = skip_surface_index0 ? 1 : 0; i + additional_increment < this->m_maxSurfaceCount; i++)
    {
        switch(additional_increment)
        {
        case 0:
            {
                if(m_SurfaceArray.GetElement(i) == nullptr)
                {
                    index = i;
                    return CM_SUCCESS;
                }
                break;
            }
        case 1:
            {
                if(m_SurfaceArray.GetElement(i) == nullptr&&
                    m_SurfaceArray.GetElement(i+1) == nullptr)
                {
                    index = i;
                    return CM_SUCCESS;
                }
                break;
            }
        case 2:
            {
                if(m_SurfaceArray.GetElement(i) == nullptr&&
                    m_SurfaceArray.GetElement(i+1) == nullptr&&
                    m_SurfaceArray.GetElement(i+2) == nullptr)
                {
                    index = i;
                    return CM_SUCCESS;
                }
                break;
            }
        default:
            {
                bool found=true;
                for(uint32_t j=0; j <= additional_increment+1; j++)
                {
                    if(m_SurfaceArray.GetElement(i+j) != nullptr)
                    {
                        found = false;
                        i=i+j;
                        break;
                    }
                }
                if(found)
                {
                    index = i;
                    return CM_SUCCESS;
                }
            }
        }
    }
    return CM_FAILURE;
}

int32_t CmSurfaceManagerEmu::DoCopyAll( )
{

    for(uint32_t i=0;i<m_SurfaceArray.GetSize();i++)
    {
        CmSurfaceEmu * surfTemp = (CmSurfaceEmu *)m_SurfaceArray.GetElement(i);
        if(surfTemp == nullptr)
            continue;
        surfTemp->DoCopy();
    }
    return CM_SUCCESS;
}

//DoGPUCopySelect
int32_t CmSurfaceManagerEmu::DoGPUCopySelect( )
{

    for(uint32_t i=0;i<m_SurfaceArray.GetSize();i++)
    {
        CmSurfaceEmu * surfTemp = (CmSurfaceEmu *)m_SurfaceArray.GetElement(i);
        if(surfTemp == nullptr || !surfTemp->getISSmUpSurface())
            continue;
        surfTemp->DoGPUCopy();
    }
    return CM_SUCCESS;
}

int32_t CmSurfaceManagerEmu::GetSurface( const uint32_t index, CmSurfaceEmu* & pSurface )
{
    pSurface = (CmSurfaceEmu *)m_SurfaceArray.GetElement(index);
    return CM_SUCCESS;
}

int32_t CmSurfaceManagerEmu::Surface2DSanityCheck(uint32_t width, uint32_t height, CM_SURFACE_FORMAT format)
{
    if ((width < CM_MIN_SURF_WIDTH) || (width > CM_MAX_2D_SURF_WIDTH))
    {
        GFX_EMU_ASSERT(0);
        return CM_INVALID_WIDTH;
    }

    if ((height < CM_MIN_SURF_HEIGHT) || (height > CM_MAX_2D_SURF_HEIGHT))
    {
        GFX_EMU_ASSERT(0);
        return CM_INVALID_HEIGHT;
    }

    switch (format)
    {
    case CM_SURFACE_FORMAT_X8R8G8B8:
    case CM_SURFACE_FORMAT_A8R8G8B8:
    case CM_SURFACE_FORMAT_R32G32B32A32F:
    case CM_SURFACE_FORMAT_R32F:
    case CM_SURFACE_FORMAT_A16B16G16R16:
    case CM_SURFACE_FORMAT_R10G10B10A2:
    case CM_SURFACE_FORMAT_A8:
    case CM_SURFACE_FORMAT_P8:
    case CM_SURFACE_FORMAT_V8U8:
#if defined(CM_DX9)
    case CM_SURFACE_FORMAT_IRW0:
    case CM_SURFACE_FORMAT_IRW1:
    case CM_SURFACE_FORMAT_IRW2:
    case CM_SURFACE_FORMAT_IRW3:
    case CM_SURFACE_FORMAT_L16:
#elif defined(CM_DX11) || defined(__GNUC__)
    case CM_SURFACE_FORMAT_R16_UINT:
    case CM_SURFACE_FORMAT_R16_SINT:
    case CM_SURFACE_FORMAT_R32_SINT:
    case CM_SURFACE_FORMAT_R32_UINT:
    case CM_SURFACE_FORMAT_R8G8_UNORM:
#endif
        break;
    case CM_SURFACE_FORMAT_UYVY:
    case CM_SURFACE_FORMAT_YUY2:
        if (width & 0x1)
        {
            GFX_EMU_ASSERT(0);
            return CM_INVALID_WIDTH;
        }
        break;
#if defined(_WIN32)
    case CM_SURFACE_FORMAT_P016:
    case CM_SURFACE_FORMAT_P010:
#endif
        if (width & 0x1)
        {
            GFX_EMU_ASSERT(0);
            return CM_INVALID_WIDTH;
        }
        if (height & 0x1)
        {
            GFX_EMU_ASSERT(0);
            return CM_INVALID_HEIGHT;
        }
        break;

    case CM_SURFACE_FORMAT_NV12:
        if (width & 0x1)
        {
            GFX_EMU_ASSERT(0);
            return CM_INVALID_WIDTH;
        }
#if defined(CM_DX11) || defined(__GNUC__)
        if (height & 0x1)
        {
            GFX_EMU_ASSERT(0);
            return CM_INVALID_HEIGHT;
        }
#endif
        break;

    default:
        GFX_EMU_ASSERT(0);
        return CM_SURFACE_FORMAT_NOT_SUPPORTED;
    }

    return CM_SUCCESS;
}

CmSurfaceFormatID CmSurfaceManagerEmu::ConvertOsFmtToSurfFmt(CM_SURFACE_FORMAT format)
{
    switch (format)
    {
    case CM_SURFACE_FORMAT_X8R8G8B8:                return B8G8R8X8_UNORM_SRGB;
    case CM_SURFACE_FORMAT_A8R8G8B8:                return B8G8R8A8_UNORM;
    case CM_SURFACE_FORMAT_R32G32B32A32F:           return R32G32B32A32_FLOAT;
    case CM_SURFACE_FORMAT_R32F:                    return R32_FLOAT;
    case CM_SURFACE_FORMAT_A16B16G16R16:            return R16G16B16A16_UNORM;
    case CM_SURFACE_FORMAT_R10G10B10A2:             return R10G10B10A2_UNORM;
    case CM_SURFACE_FORMAT_A8:                      return A8_UNORM;
    case CM_SURFACE_FORMAT_P8:                      return R8_UNORM;
    case CM_SURFACE_FORMAT_V8U8:                    return R8G8_SNORM;
    case CM_SURFACE_FORMAT_YUY2:                    return YCRCB_NORMAL;
    case CM_SURFACE_FORMAT_NV12:                    return R8_UNORM;
    case CM_SURFACE_FORMAT_P016:                    return R16_UNORM;
    case CM_SURFACE_FORMAT_P010:                    return R16_UNORM;
#if defined(CM_DX9)
    case CM_SURFACE_FORMAT_IRW0:                    return R16_UNORM;
    case CM_SURFACE_FORMAT_IRW1:                    return R16_UNORM;
    case CM_SURFACE_FORMAT_IRW2:                    return R16_UNORM;
    case CM_SURFACE_FORMAT_IRW3:                    return R16_UNORM;
    case CM_SURFACE_FORMAT_L16:                     return L16_UNORM;
    case CM_SURFACE_FORMAT_UYVY:                    return YCRCB_SWAPY;
#elif defined(CM_DX11) || defined(__GNUC__)
    case CM_SURFACE_FORMAT_R16_UINT:                return R16_UINT;
    case CM_SURFACE_FORMAT_R16_SINT:                return R16_SINT;
    case CM_SURFACE_FORMAT_R32_SINT:                return R32_SINT;
    case CM_SURFACE_FORMAT_R32_UINT:                return R32_UINT;
    case CM_SURFACE_FORMAT_R8G8_UNORM:              return R8G8_UNORM;
    case CM_SURFACE_FORMAT_UYVY:                    return YCRCB_SWAPUVY;
#endif
    default:
        GFX_EMU_ASSERT(0);
        return INVALID_SURF_FORMAT;
    }
}

int32_t CmSurfaceManagerEmu::CreateBufferStateless(size_t size,
                                                   uint32_t option,
                                                   void *memAddress,
                                                   CmBufferEmu *&pBufferStateless)
{
    uint32_t status = CM_SUCCESS;

	switch(option)
	{
		case CM_BUFFER_STATELESS_CREATE_OPTION_GFX_MEM:
			status = CreateBufferStatelessBasedGfxMem(size, pBufferStateless);
			break;
		case CM_BUFFER_STATELESS_CREATE_OPTION_SYS_MEM:
			status = CreateBufferStatelessBasedSysMem(size, memAddress, pBufferStateless);
			break;
		default:
		    GFX_EMU_ASSERT(0);
			return CM_INVALID_CREATE_OPTION_FOR_BUFFER_STATELESS;
	}

    return status;
}

int32_t CmSurfaceManagerEmu::CreateBufferStatelessBasedGfxMem(size_t size,
                                                              CmBufferEmu *&pSurface)
{
    pSurface = nullptr;
	void *buffer = nullptr;

    uint32_t index = 0;
    index = m_SurfaceArray.GetFirstFreeIndex();

    if (index >= this->m_maxSurfaceCount)
    {
        GFX_EMU_ASSERT(0);
        return CM_EXCEED_SURFACE_AMOUNT;
    }

    if (m_bufferCount >= m_maxBufferCount)
    {
        GFX_EMU_ASSERT(0);
        return CM_EXCEED_SURFACE_AMOUNT;
    }

    int32_t result = CmBufferEmu::Create(index, index, size, R8G8B8A8_UINT, true, pSurface, buffer, false, this);

    if (result != CM_SUCCESS)
    {
        GFX_EMU_ASSERT(0);
        return result;
    }

    pSurface->SetGfxAddress(reinterpret_cast<uint64_t>(buffer));
    pSurface->SetStatelessSurfaceType(CM_STATELESS_BUFFER);

    m_statelessSurfaceArray.insert(pSurface);

    m_SurfaceArray.SetElement(index, pSurface);
    m_bufferCount++;

    return CM_SUCCESS;
}

int32_t CmSurfaceManagerEmu::CreateSurface2DStateless(uint32_t width,
                                                      uint32_t height,
                                                      uint32_t &pitch,
                                                      CmSurface2DEmu *&pSurface2D)
{
    pSurface2D = nullptr;
    void *buffer = nullptr;

    uint32_t index = 0;
    index = m_SurfaceArray.GetFirstFreeIndex();

    if (index >= this->m_maxSurfaceCount)
    {
        GFX_EMU_ASSERT(0);
        return CM_EXCEED_SURFACE_AMOUNT;
    }

    if (m_bufferCount >= m_maxBufferCount)
    {
        GFX_EMU_ASSERT(0);
        return CM_EXCEED_SURFACE_AMOUNT;
    }

    int32_t result = CmSurface2DEmu::Create(index, // index
                                            1,     // sizePerPixel
                                            width,
                                            height,
                                            CM_SURFACE_FORMAT_A8, // CM_SURFACE_FORMAT osApiSurfaceFmt
                                            A8_UNORM, // CmSurfaceFormatID surfFormat
                                            true, // isCreated
                                            pSurface2D, // pSurface
                                            buffer, // psysmem
                                            false, // dummysurf
                                            this); // surfacemanager

    if (result != CM_SUCCESS)
    {
        GFX_EMU_ASSERT(0);
        return result;
    }

    pSurface2D->SetGfxAddress(reinterpret_cast<uint64_t>(buffer));
    pSurface2D->SetStatelessSurfaceType(CM_STATELESS_SURFACE_2D);

    m_statelessSurfaceArray.insert(pSurface2D);

    m_SurfaceArray.SetElement(index, pSurface2D);

    m_2DSurfaceCount ++;
    pitch = width;

    return CM_SUCCESS;
}
