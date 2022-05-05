/*========================== begin_copyright_notice ============================

Copyright (C) 2017 Intel Corporation

SPDX-License-Identifier: MIT

============================= end_copyright_notice ===========================*/

#pragma once

#include <vector>
#include "cm_array.h"
#include "cm_def.h"
#include "cm_surface_emumode.h"
#include "cm_surface_2d_emumode.h"
#include "cm_buffer_emumode.h"
#include <set>

class CmSurfaceEmu;
class CmBufferEmu;
class CmSurface2D;
class CmSurface2DEmu;
class CmSurface3D;
class CmSurface3DEmu;
class SurfaceIndex;

struct CompareByGfxAddress
{
    bool operator()(CmSurfaceEmu *left, CmSurfaceEmu *right) const
    {
        uint64_t lAddr = 0;
        uint64_t rAddr = 0;

        if (left->GetStatelessSurfaceType() == CM_STATELESS_BUFFER)
        {
            CmBufferEmu *bufferStatelessLeft = static_cast<CmBufferEmu *>(left);
            bufferStatelessLeft->GetGfxAddress(lAddr);
        }
        else if (left->GetStatelessSurfaceType() == CM_STATELESS_SURFACE_2D)
        {
            CmSurface2DEmu *surf2DStatelessLeft = static_cast<CmSurface2DEmu *>(left);
            surf2DStatelessLeft->GetGfxAddress(lAddr);
        }

        if (right->GetStatelessSurfaceType() == CM_STATELESS_BUFFER)
        {
            CmBufferEmu *bufferStatelessRight = static_cast<CmBufferEmu *>(right);
            bufferStatelessRight->GetGfxAddress(rAddr);
        }
        else if (right->GetStatelessSurfaceType() == CM_STATELESS_SURFACE_2D)
        {
            CmSurface2DEmu *surf2DStatelessRight = static_cast<CmSurface2DEmu *>(right);
            surf2DStatelessRight->GetGfxAddress(rAddr);
        }
        return (lAddr < rAddr);
    }
};

class CmSurfaceManagerEmu
{
public:
    static int32_t Create( CmSurfaceManagerEmu *&pManager, CM_HAL_MAX_VALUES HalMaxValues, CM_HAL_MAX_VALUES_EX HalMaxValuesEx );
    static int32_t Destroy( CmSurfaceManagerEmu* &pManager );

    int32_t CreateBuffer(uint32_t width, CmBufferEmu* & pSurface, void *&pSysMem, bool noRegisterBuffer = false);
    int32_t CreateSurface2D(uint32_t width, uint32_t height, CM_SURFACE_FORMAT format, CmSurface2DEmu* & pSurface,void* pSysMem = nullptr );
    int32_t CreateSurface2DUP(uint32_t width, uint32_t height, CM_SURFACE_FORMAT format, CmSurface2DEmu* & pSurface,void* pSysMem = nullptr );
    #if defined(_WIN32)
    int32_t CreateSurface2D(CM_IDIRECT3DSURFACE* pD3DSurf, CmSurface2DEmu* &pSurface);
    #endif
    int32_t CreateSurface3D(uint32_t width, uint32_t height, uint32_t depth, CM_SURFACE_FORMAT format, CmSurface3DEmu* & pSurface );
    int32_t MoveSurface( uint32_t src_index, uint32_t dst_index);
    void FindSurfaceArea(uint32_t index, int & startpos, int & endpos);
    int32_t DestroySurface( CmBufferEmu* & pSurface );
    int32_t DestroySurface( CmSurface2DEmu* & pSurface );
    int32_t DestroySurface( CmSurface3DEmu* & pSurface );
    int32_t DestroySurface2DUP( CmSurface2DEmu* & pSurface2D );

    int32_t GetSurface( const uint32_t index, CmSurfaceEmu* & pSurface );
    int32_t DoCopyAll( );
    int32_t DoGPUCopySelect();
    int32_t findFreeIndex(uint32_t additionalIncrement, uint32_t &index);

    int getBytesPerPixel(CM_SURFACE_FORMAT format, uint32_t *additional_increment);

    int32_t Surface2DSanityCheck(uint32_t width, uint32_t height, CM_SURFACE_FORMAT format);
    CmSurfaceFormatID ConvertOsFmtToSurfFmt(CM_SURFACE_FORMAT format);

    void SetSurfaceArrayElement(uint32_t index, CmSurfaceEmu* surface)
    {
        m_SurfaceArray.SetElement(index, surface);
    }

    void SetSurfaceArrayDummy(uint32_t index)
    {
        m_SurfaceArray.SetElement(index, m_pSurfaceDummy);
    }

    void AddToAliasIndexTable(uint32_t index)
    {
        m_aliasIndexTable.push_back(index);
    }

    void RemoveFromAliasIndexTable(uint32_t index)
    {
        std::vector<uint32_t>::iterator iter = std::find(m_aliasIndexTable.begin(), m_aliasIndexTable.end(), index);
        m_aliasIndexTable.erase(iter);
    }

    int32_t CreateBufferStateless(size_t size, uint32_t option, void *memAddress, CmBufferEmu *&pBufferStateless);

    int32_t CreateSurface2DStateless(uint32_t width, uint32_t height, uint32_t &pitch, CmSurface2DEmu *&pSurface2D);

	std::set<CmSurfaceEmu *, CompareByGfxAddress> &
        GetStatelessSurfaceArray() { return m_statelessSurfaceArray; }

protected:

    CmSurfaceManagerEmu();
    ~CmSurfaceManagerEmu( void );
    int32_t Initialize( CM_HAL_MAX_VALUES HalMaxValues, CM_HAL_MAX_VALUES_EX HalMaxValuesEx );

	/* Emulator not support GPU memory map, ...GfxMem and ...SysMem must be the same */
    int32_t CreateBufferStatelessBasedGfxMem(size_t size, CmBufferEmu *&pSurface);
    int32_t CreateBufferStatelessBasedSysMem(size_t size, void *memAddress, CmBufferEmu *&pSurface) { return CmNotImplemented(__PRETTY_FUNCTION__); }

    CmDynamicArray m_SurfaceArray;
    uint32_t m_maxSurfaceCount;
    uint32_t m_bufferID;

    CmSurface2DEmu* m_pSurfaceDummy;

    uint32_t m_maxBufferCount;
    uint32_t m_bufferCount;

    uint32_t m_max2DSurfaceCount;
    uint32_t m_2DSurfaceCount;

    uint32_t m_max3DSurfaceCount;
    uint32_t m_3DSurfaceCount;

    uint32_t m_max2DUPSurfaceCount;
    uint32_t m_2DUPSurfaceCount;

    std::vector<uint32_t> m_aliasIndexTable;

	std::set<CmSurfaceEmu *, CompareByGfxAddress> m_statelessSurfaceArray;
};
