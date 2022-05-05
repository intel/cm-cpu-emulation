/*========================== begin_copyright_notice ============================

Copyright (C) 2017 Intel Corporation

SPDX-License-Identifier: MIT

============================= end_copyright_notice ===========================*/

#include "cm_include.h"
#include "cm_buffer_base.h"
#include "cm_buffer_emumode.h"
#include "cm_surface_manager_emumode.h"
#include "cm_event_base.h"
#include "cm.h"
#include "cm_mem_fast_copy.h"

#ifdef __GNUC__
extern void
CM_unregister_buffer_emu(SurfaceIndex buf_id, bool copy);

extern void
CM_register_buffer_emu(int buf_id, CmBufferType bclass, void *src, uint width) ;
#endif

CmBufferEmu::CmBufferEmu(uint32_t width,
                         CmSurfaceFormatID surfFormat,
                         bool isCmCreated,
                         CmSurfaceManagerEmu* surfaceManager):
                         CmSurfaceEmu(isCmCreated, surfaceManager),
						 m_gfxAddress(0),
                         m_sysAddress(nullptr)
{
    m_surfFormat = surfFormat;
    m_width = width;

    m_baseAddressOffset = 0;
    m_newSize = width;
}

CmBufferEmu::~CmBufferEmu(){
    SurfaceIndex* pIndex = nullptr;
    this->GetIndex(pIndex);
    if (nullptr == pIndex)
    {
        GFX_EMU_ERROR_MESSAGE("Surface index not found.\n");
        fflush(stderr);
        exit(1);
    }
    CM_unregister_buffer_emu(*pIndex,false);
    if(this->m_buffer != nullptr && this->alloc_dummy)
    {
#if defined(_WIN32)
        _aligned_free(this->m_buffer);
#else
        free(this->m_buffer);
#endif
    }
}

int32_t CmBufferEmu::Initialize( uint32_t index, uint32_t arrayIndex, void *&sysMem )
{
    m_arrayIndex=arrayIndex;
    if(sysMem == nullptr)
    {
#if defined(_WIN32)
        m_buffer = _aligned_malloc(m_width, 16); // oword aligned
#else
        posix_memalign(&m_buffer, 16, m_width); // oword aligned
#endif
        if (m_buffer == nullptr) {
            GFX_EMU_ERROR_MESSAGE("Out of memory (%d) - 1dEmu\n", m_width);
            fflush(stderr);
            exit(1);
        }
        CmSafeMemSet( m_buffer, 0, m_width );
        this->alloc_dummy = true;
        sysMem = m_buffer; // write buffer address back to sysMem
    }else
    {
        m_buffer = sysMem;
        this->alloc_dummy = false;
    }
    return CmSurfaceEmu::Initialize( index );

}

int32_t CmBufferEmu::Create(uint32_t index, uint32_t arrayIndex, uint32_t width,
        CmSurfaceFormatID surfFormat, bool isCmCreated,
        CmBufferEmu *&pSurface, void *&sysMem, bool noRegisterBuffer,
        CmSurfaceManagerEmu* surfaceManager)
{
    int32_t result = CM_SUCCESS;

    pSurface = new CmBufferEmu(width, surfFormat, isCmCreated, surfaceManager);
    if( pSurface )
    {
        pSurface->Initialize(index,arrayIndex,sysMem);
    }
    else
    {
        GFX_EMU_ASSERT( 0 );
        return CM_OUT_OF_HOST_MEMORY;
    }
    // noRegisterBuffer is true for an SVM buffer. We don't want to register
    // an SVM buffer, to avoid DoCopyAll() copying the spurious registered
    // buffer data back into the system memory after running the kernels.
    if (!noRegisterBuffer) {
            CM_register_buffer_emu(index, GEN4_INPUT_OUTPUT_BUFFER, pSurface->getBuffer(), width);
            if (pSurface->CheckStatus(index) == CM_FAILURE)
                return CM_OUT_OF_HOST_MEMORY;
    }
    return result;
}

CM_RT_API int32_t CmBufferEmu::ReadSurface(unsigned char *pSysMem, CmEvent* pEvent, uint64_t sysMemSize )
{
    int index=0;

    if(pSysMem == nullptr)
    {
        GFX_EMU_ASSERT( 0 );
        return CM_INVALID_ARG_VALUE;
    }

    if( sysMemSize < this->m_width )
    {
        GFX_EMU_ASSERT( 0 );
        return CM_INVALID_ARG_VALUE;
    }
    /*
        to avoid this bug:
        surf1->write(mem1)
        surf1->read(mem2)
        use mem1 <-- problem because otherwise mem1 will be overwritten
    */
    CmFastMemCopyFromWC(pSysMem, this->m_buffer, this->m_width, GetCpuInstructionLevel());

    return CM_SUCCESS;
}

CM_RT_API int32_t CmBufferEmu::WriteSurface(const unsigned char *pSysMem, CmEvent* pEvent, uint64_t sysMemSize )
{
    if(pSysMem == nullptr)
    {
        GFX_EMU_ASSERT( 0 );
        return CM_INVALID_ARG_VALUE;
    }

    if( sysMemSize < this->m_width )
    {
        GFX_EMU_ASSERT( 0 );
        return CM_INVALID_ARG_VALUE;
    }

    CmSafeMemCopy( this->m_buffer, pSysMem, this->m_width );

    return DoGPUCopy();
}

CM_RT_API int32_t CmBufferEmu::GetIndex(SurfaceIndex *&pIndex)
{
    pIndex = m_pIndex;
    return CM_SUCCESS;
}

int32_t CmBufferEmu::GetArrayIndex( uint32_t& arrayIndex )
{
    arrayIndex = m_arrayIndex;
    return CM_SUCCESS;
}

int32_t CmBufferEmu::SetArrayIndex( uint32_t arrayIndex )
{
    m_arrayIndex = arrayIndex;
    return CM_SUCCESS;
}

int32_t CmBufferEmu::DoCopy()
{
    cm_list<CmEmulSys::iobuffer>::iterator buff_iter;
    buff_iter = CmEmulSys::search_buffer(m_pIndex->get_data());
    if(buff_iter->p_volatile == nullptr)
        return CM_FAILURE;
    // Let's consider the pre/post execution copy as legacy
    // and comment it out for now.
    //CmFastMemCopyFromWC((int8_t*)m_buffer+m_baseAddressOffset, buff_iter->p_volatile, m_newSize, GetCpuInstructionLevel());
    return CM_SUCCESS;
}

int32_t CmBufferEmu::DoGPUCopy(
bool doD3DCopy
)
{
    cm_list<CmEmulSys::iobuffer>::iterator buff_iter;
    buff_iter = CmEmulSys::search_buffer(m_pIndex->get_data());
    if(buff_iter->p_volatile == nullptr)
        return CM_FAILURE;
    // Let's consider the pre/post execution copy as legacy
    // and comment it out for now.
    //CmFastMemCopyFromWC(buff_iter->p_volatile, (int8_t*)m_buffer + m_baseAddressOffset, m_newSize, GetCpuInstructionLevel());

    GPUCopyForBufferAlias();

    return CM_SUCCESS;
}

CM_RT_API int32_t CmBufferEmu::InitSurface(const uint32_t initValue, CmEvent* pEvent)
{
    CmDwordMemSet( m_buffer, initValue, m_width );
    return DoGPUCopy();
}

int32_t CmBufferEmu::CreateBufferAlias(SurfaceIndex* & aliasIndex)
{
    uint32_t newIndex = 0;
    if (m_aliasIndices.size() < CM_MAX_NUM_BUFFER_ALIASES)
    {
        m_surfaceManager->findFreeIndex(0, newIndex);
        m_surfaceManager->SetSurfaceArrayElement(newIndex, this);
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

CM_RT_API int32_t CmBufferEmu::SetSurfaceStateParam(SurfaceIndex *surfIndex,
                                                   const CM_BUFFER_STATE_PARAM *bufferStateParam)
{
    if (bufferStateParam == nullptr)
    {
        GFX_EMU_ASSERT(0);
        return CM_NULL_POINTER;
    }

    uint32_t        newSize = 0;
    uint32_t        aliasIndex = 0;
    if (bufferStateParam->uiBaseAddressOffset + bufferStateParam->uiSize > m_width)
    {
        GFX_EMU_ERROR_MESSAGE("The offset exceeds the buffer size.");
        GFX_EMU_ASSERT(0);
        return CM_INVALID_ARG_VALUE;
    }
    if (bufferStateParam->uiBaseAddressOffset % 16) // the offset must be 16-aligned, otherwise it will cause a GPU hang
    {
        GFX_EMU_ERROR_MESSAGE("The offset must be 16-aligned, otherwise it will cause GPU hang.");
        GFX_EMU_ASSERT(0);
        return CM_INVALID_ARG_VALUE;
    }

    if (bufferStateParam->uiSize)
    {
        newSize = bufferStateParam->uiSize;
    }
    else
    {
        newSize = m_width - bufferStateParam->uiBaseAddressOffset;
    }

    if (surfIndex)
    {
        aliasIndex = surfIndex->get_data();
        CM_BUFFER_STATE_PARAM aliasBufferState;
        aliasBufferState.uiSize = newSize;
        aliasBufferState.uiBaseAddressOffset = bufferStateParam->uiBaseAddressOffset;
        std::pair<uint32_t, CM_BUFFER_STATE_PARAM> newElement(aliasIndex, aliasBufferState);
        m_aliasBufferStates.insert(newElement);
    }
    else
    {
        aliasIndex = m_pIndex->get_data();
        m_baseAddressOffset = bufferStateParam->uiBaseAddressOffset;
        m_newSize = newSize;
    }

    CM_register_buffer_emu(aliasIndex, GEN4_INPUT_OUTPUT_BUFFER, (int8_t*)m_buffer + bufferStateParam->uiBaseAddressOffset, newSize);

    cm_list<CmEmulSys::iobuffer>::iterator buff_iter;
    buff_iter = CmEmulSys::search_buffer(aliasIndex);
    if (buff_iter->p_volatile == nullptr)
        return CM_FAILURE;

    // Let's consider the pre/post execution copy as legacy
    // and comment it out for now.
    //CmFastMemCopyFromWC(buff_iter->p_volatile, (int8_t*)m_buffer + bufferStateParam->uiBaseAddressOffset, newSize, GetCpuInstructionLevel());

    return CM_SUCCESS;
}

int32_t CmBufferEmu::GPUCopyForBufferAlias()
{
    cm_list<CmEmulSys::iobuffer>::iterator buff_iter;
    for (uint i = 0; i < m_aliasIndices.size(); i++)
    {
        std::map<uint32_t, CM_BUFFER_STATE_PARAM>::iterator iter
            = m_aliasBufferStates.find(m_aliasIndices[i]->get_data());
        if (m_aliasBufferStates.end() == iter)
        {
            return CM_SUCCESS;
        }
        else
        {
            buff_iter = CmEmulSys::search_buffer(m_aliasIndices[i]->get_data());
            // Let's consider the pre/post execution copy as legacy
            // and comment it out for now.
            //CmFastMemCopyFromWC(buff_iter->p_volatile, (int8_t*)m_buffer + iter->second.uiBaseAddressOffset, iter->second.size, GetCpuInstructionLevel());
        }
    }
    return CM_SUCCESS;
}

CM_RT_API int32_t CmBufferEmu::GetGfxAddress(uint64_t &address)
{
    address = m_gfxAddress;
    return CM_SUCCESS;
}

int32_t CmBufferEmu::SetGfxAddress(uint64_t address)
{
    m_gfxAddress = address;
    return CM_SUCCESS;
}

CM_RT_API int32_t CmBufferEmu::GetSysAddress(void *&address)
{
    address = m_sysAddress;
    return CM_SUCCESS;
}

int32_t CmBufferEmu::SetSysAddress(void *address)
{
    m_sysAddress = address;
    return CM_SUCCESS;
}
