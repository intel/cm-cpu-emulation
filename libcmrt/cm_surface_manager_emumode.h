/*===================== begin_copyright_notice ==================================

 Copyright (c) 2021, Intel Corporation


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

#pragma once

#include <vector>
#include "cm_array.h"
#include "cm_def.h"
#include "cm_memory_object_control.h"
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

class CmSurfaceManagerEmu
{
public:
    static int32_t Create( CmSurfaceManagerEmu *&pManager, CM_HAL_MAX_VALUES HalMaxValues, CM_HAL_MAX_VALUES_EX HalMaxValuesEx );
    static int32_t Destroy( CmSurfaceManagerEmu* &pManager );

    int32_t CreateBuffer(uint32_t width, CmBufferEmu* & pSurface, void *&pSysMem, bool noRegisterBuffer = false);
    int32_t CreateSurface2D(uint32_t width, uint32_t height, CM_SURFACE_FORMAT format, CmSurface2DEmu* & pSurface,void* pSysMem = nullptr );
    int32_t CreateSurface2DUP(uint32_t width, uint32_t height, CM_SURFACE_FORMAT format, CmSurface2DEmu* & pSurface,void* pSysMem = nullptr );
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

protected:

    CmSurfaceManagerEmu();
    ~CmSurfaceManagerEmu( void );
    int32_t Initialize( CM_HAL_MAX_VALUES HalMaxValues, CM_HAL_MAX_VALUES_EX HalMaxValuesEx );

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

};

