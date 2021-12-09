/*========================== begin_copyright_notice ============================

Copyright (C) 2017 Intel Corporation

SPDX-License-Identifier: MIT

============================= end_copyright_notice ===========================*/

#pragma once
#include <vector>
#include <map>
#include "cm_buffer_base.h"
#include "cm_surface_emumode.h"

class CmBuffer;
class CmSurfaceEmu;

//!
//! CM 1D surface
//!
class CmBufferEmu :
    public CmSurfaceEmu,
    public CmBuffer,
    public CmBufferUP,
    public CmBufferSVM
{
public:
    static int32_t Create( uint32_t index, uint32_t arrayIndex, uint32_t width, CmSurfaceFormatID surfFormat , bool isCmCreated, CmBufferEmu* &pSurface, void *&sysMem, bool noRegisterBuffer = false, CmSurfaceManagerEmu* surfaceManager = nullptr);
    CM_RT_API int32_t ReadSurface( unsigned char* pSysMem, CmEvent* pEvent, uint64_t sysMemSize = 0xFFFFFFFFFFFFFFFFULL );
    CM_RT_API int32_t WriteSurface( const unsigned char* pSysMem, CmEvent* pEvent, uint64_t sysMemSize = 0xFFFFFFFFFFFFFFFFULL );
    CM_RT_API int32_t GetIndex( SurfaceIndex*& pIndex );
    CM_RT_API int32_t GetHandle( uint32_t& handle) {
        return CmNotImplemented(__PRETTY_FUNCTION__); }
    CM_RT_API int32_t GetAddress( void  *&pAddr) {
        return  CmNotImplemented(__PRETTY_FUNCTION__); }

    int32_t SetSysAddress(void *address);
    CM_RT_API int32_t GetSysAddress(void *&pAddr);

    int32_t GetArrayIndex( uint32_t& arrayIndex );
    int32_t SetArrayIndex( uint32_t arrayIndex );
    int32_t DoCopy();
    int32_t DoGPUCopy(
bool doD3DCopy=true
);
    uint32_t GetWidth(){return m_width;}
    uint32_t GetHeight() const {return 1;}
    uint32_t GetDepth(){return 1;}
    void SetDeviceTileID(int32_t deviceTileID) { m_deviceTileID = deviceTileID; }
    CM_RT_API int32_t InitSurface(const uint32_t initValue, CmEvent* pEvent);
    CM_RT_API int32_t SelectMemoryObjectControlSetting(MEMORY_OBJECT_CONTROL mem_ctrl) {
            return CmNotImplemented(__PRETTY_FUNCTION__);
    }
    CM_RT_API int32_t SetSurfaceStateParam(SurfaceIndex *surface_index,
                                           const CM_BUFFER_STATE_PARAM *state_param);
    int32_t CreateBufferAlias(SurfaceIndex* & aliasIndex);

    std::vector<SurfaceIndex *> GetAliasIndices() { return m_aliasIndices;}

    int32_t GPUCopyForBufferAlias();
protected:
    int32_t Initialize( uint32_t index, uint32_t arrayIndex, void *&sysMem );
    CmBufferEmu( uint32_t width, CmSurfaceFormatID surfFormat, bool isCmCreated, CmSurfaceManagerEmu* surfaceManager);
    ~CmBufferEmu( void );

    uint32_t m_arrayIndex;

    std::vector<SurfaceIndex *> m_aliasIndices;

    uint32_t m_baseAddressOffset;

    uint32_t m_newSize;

    std::map<uint32_t, CM_BUFFER_STATE_PARAM> m_aliasBufferStates;

    uint64_t m_gfxAddress;
    void *m_sysAddress;
    int m_deviceTileID; // if it is >= 0, it is multiTile
};
