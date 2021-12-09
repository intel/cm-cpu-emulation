/*========================== begin_copyright_notice ============================

Copyright (C) 2017 Intel Corporation

SPDX-License-Identifier: MIT

============================= end_copyright_notice ===========================*/

#include "cm_include.h"
#include "cm_task_emumode.h"
#include "cm_device_emumode.h"
#include "cm_group_space_emumode.h"
#include "cm_mem.h"

#include "emu_cfg.h"

int32_t CmThreadGroupSpace::Create( CmDevice* pDevice, uint32_t thrdSpaceWidth, uint32_t thrdSpaceHeight, uint32_t grpSpaceWidth, uint32_t grpSpaceHeight, CmThreadGroupSpace* & pTGS)
{
    return Create(pDevice, thrdSpaceWidth, thrdSpaceHeight, 1, grpSpaceWidth, grpSpaceHeight, 1, pTGS);
}

int32_t CmThreadGroupSpace::Create(CmDevice *pDevice, uint32_t thrdSpaceWidth, uint32_t thrdSpaceHeight, uint32_t thrdSpaceDepth, uint32_t grpSpaceWidth, uint32_t grpSpaceHeight, uint32_t grpSpaceDepth, CmThreadGroupSpace *&pTGS)
{
    uint32_t max_thread_count_per_group = 0;
    size_t size = sizeof(int);
    pDevice->GetCaps(CAP_USER_DEFINED_THREAD_COUNT_PER_THREAD_GROUP, size, &max_thread_count_per_group);
    uint32_t max_thread_space_width_pergroup = 0;
    uint32_t max_thread_space_height_pergroup = 0;
    if (GfxEmu::Cfg::Platform ().getInt () >= GfxEmu::Platform::XEHP_SDV)
    {
        max_thread_space_width_pergroup  = MAX_THREAD_SPACE_WIDTH_PERGROUP_XEHP_SDV;
        max_thread_space_height_pergroup = MAX_THREAD_SPACE_HEIGHT_PERGROUP_XEHP_SDV;
    }

    if (GfxEmu::Cfg::Platform ().getInt () == GfxEmu::Platform::TGLLP)
    {
        max_thread_space_width_pergroup  = MAX_THREAD_SPACE_WIDTH_PERGROUP_GEN12LP;
        max_thread_space_height_pergroup = MAX_THREAD_SPACE_HEIGHT_PERGROUP_GEN12LP;
    }
    else
    {
        max_thread_space_width_pergroup  = MAX_THREAD_SPACE_WIDTH_PERGROUP;
        max_thread_space_height_pergroup = MAX_THREAD_SPACE_HEIGHT_PERGROUP;
    }

    if ((thrdSpaceWidth == 0) || (thrdSpaceHeight == 0) || (thrdSpaceDepth == 0)
        || (grpSpaceWidth == 0) || (grpSpaceHeight == 0) || (grpSpaceDepth == 0)
        || (thrdSpaceWidth > max_thread_space_width_pergroup)
        || (thrdSpaceHeight > max_thread_space_height_pergroup)
        || (thrdSpaceDepth * thrdSpaceHeight * thrdSpaceWidth > max_thread_count_per_group))
    {
        GFX_EMU_ERROR_MESSAGE("Exceed thread group size limitation!");
        GFX_EMU_ASSERT( 0 );
        return CM_INVALID_THREAD_GROUP_SPACE;
    }

    int32_t result = CM_SUCCESS;
    pTGS           = new CmThreadGroupSpace(pDevice, thrdSpaceWidth, thrdSpaceHeight, thrdSpaceDepth, grpSpaceWidth, grpSpaceHeight, grpSpaceDepth);
    if( pTGS )
    {
        result = pTGS->Initialize( );
        if( result != CM_SUCCESS )
        {
            CmThreadGroupSpace::Destroy( pTGS);
        }
    }
    else
    {
        GFX_EMU_ASSERT( 0 );
        result = CM_OUT_OF_HOST_MEMORY;
    }
    return result;
}

int32_t CmThreadGroupSpace::Destroy( CmThreadGroupSpace* &pTGS )
{
    CmSafeRelease( pTGS );
    return CM_SUCCESS;
}

int32_t CmThreadGroupSpace::GetThreadGroupSpaceSize(uint32_t & thrdSpaceWidth, uint32_t & thrdSpaceHeight, uint32_t & grpSpaceWidth, uint32_t & grpSpaceHeight)
{
    GFX_EMU_ASSERT(1 == m_threadSpaceDepth && 1 == m_groupSpaceDepth);
    thrdSpaceWidth = m_threadSpaceWidth;
    thrdSpaceHeight = m_threadSpaceHeight;
    grpSpaceWidth = m_groupSpaceWidth;
    grpSpaceHeight = m_groupSpaceHeight;

    return CM_SUCCESS;
}

int32_t CmThreadGroupSpace::GetThreadGroupSpaceSize(uint32_t &thrdSpaceWidth, uint32_t &thrdSpaceHeight, uint32_t &thrdSpaceDepth, uint32_t &grpSpaceWidth,
    uint32_t &grpSpaceHeight, uint32_t &grpSpaceDepth)
{
    thrdSpaceWidth = m_threadSpaceWidth;
    thrdSpaceHeight = m_threadSpaceHeight;
    thrdSpaceDepth = m_threadSpaceDepth;
    grpSpaceWidth = m_groupSpaceWidth;
    grpSpaceHeight = m_groupSpaceHeight;
    grpSpaceDepth = m_groupSpaceDepth;

    return CM_SUCCESS;
}

CmThreadGroupSpace::CmThreadGroupSpace(CmDevice *pCmDev, uint32_t thrdSpaceWidth, uint32_t thrdSpaceHeight, uint32_t thrdSpaceDepth, uint32_t grpSpaceWidth, uint32_t grpSpaceHeight, uint32_t grpSpaceDepth):
    m_pCmDev(pCmDev), m_threadSpaceWidth(thrdSpaceWidth), m_threadSpaceHeight(thrdSpaceHeight), m_threadSpaceDepth(thrdSpaceDepth),
    m_groupSpaceWidth(grpSpaceWidth), m_groupSpaceHeight(grpSpaceHeight), m_groupSpaceDepth(grpSpaceDepth)
{
}

CmThreadGroupSpace::~CmThreadGroupSpace( void )
{
}

int32_t CmThreadGroupSpace::Initialize( void )
{
    return CM_SUCCESS;
}
