/*========================== begin_copyright_notice ============================

Copyright (C) 2017 Intel Corporation

SPDX-License-Identifier: MIT

============================= end_copyright_notice ===========================*/

#include "cm_include.h"

#include "cm_program_emumode.h"
#include <iostream>
#include <fstream>
#include <string.h>

#include "emu_kernel_support.h"
#include "emu_log.h"

using namespace std;

int32_t CmProgramEmu::Create(
    CmDeviceEmu* pCmDev,
    CmProgramEmu*& pProgram,
    void *programAddr,
    size_t size)
{
    int32_t result = CM_SUCCESS;

    pProgram = new CmProgramEmu( pCmDev,  programAddr, size );

    if (!pProgram->GetProgramModule ()) {
        GFX_EMU_WARNING_MESSAGE_AT("CmProgramEmu::Create: can't setup program for address %p.\n", programAddr);
    }

    if( pProgram )
    {
        pProgram->Acquire();
    }
    else
    {
        GFX_EMU_ASSERT( 0 );
        result = CM_OUT_OF_HOST_MEMORY;
    }
    return result;

}

int32_t CmProgramEmu::Destroy( CmProgramEmu* &pProgram )
{
    int refCount = pProgram->SafeRelease();
    if (refCount == 0)
    {
        pProgram = nullptr;
    }
    return CM_SUCCESS;
}

CmProgramEmu::CmProgramEmu( CmDeviceEmu* pCmDev, void *programAddr, size_t size):
    m_pCmDev( pCmDev ),
    m_programModule ( GfxEmu::KernelSupport::setupProgram(programAddr, size) )
{
    m_refCount = 0;
}

CmProgramEmu::~CmProgramEmu( void )
{
}

//*-----------------------------------------------------------------------------
//| Purpose:    Acquire: Increase refcount
//| Returns:    Result of the operation.
//*-----------------------------------------------------------------------------
int CmProgramEmu::Acquire(void)
{
    m_refCount++;
    return m_refCount;
}

//*-----------------------------------------------------------------------------
//| Purpose:    SafeRelease:
//| Returns:    Result of the operation.
//*-----------------------------------------------------------------------------
int CmProgramEmu::SafeRelease(void)
{
    --m_refCount;
    if (m_refCount == 0)
    {
        delete this;
        return 0;
    }
    return m_refCount;
}
