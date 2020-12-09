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

#pragma once

#include "cm_memory_object_control.h"
#include "cm_surface_3d_base.h"
#include "cm_surface_emumode.h"
#include "cm_debug.h"

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
    uint32_t GetWidth(){return m_width;}
    uint32_t GetHeight() const {return m_height;}
    uint32_t GetDepth(){return m_depth;}
    void GetSurfaceFormat(CmSurfaceFormatID &surfFormat){ surfFormat =  m_surfFormat;}

protected:
    CmSurface3DEmu( uint32_t width, uint32_t height, uint32_t depth, CM_SURFACE_FORMAT osApiSurfaceFmt, CmSurfaceFormatID surfFormat, bool isCmCreated );
    ~CmSurface3DEmu( void );

    int32_t Initialize( uint32_t index, uint32_t arrayIndex );

    uint32_t m_arrayIndex;
};
