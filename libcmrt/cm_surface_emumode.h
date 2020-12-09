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

#include "genx_dataport.h"

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

protected:
    CmSurfaceEmu( bool isCmCreated, CmSurfaceManagerEmu* surfaceManager);
    virtual ~CmSurfaceEmu( void );
    virtual int32_t Initialize( uint32_t index );

    SurfaceIndex* m_pIndex;
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

};
