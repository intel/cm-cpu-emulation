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

#include "cm_include.h"

#include "cm_program_emumode.h"
#include <iostream>
#include <fstream>
#include <string.h>

using namespace std;

int32_t CmProgramEmu::Create( CmDeviceEmu* pCmDev, CmProgramEmu*& pProgram )
{
    int32_t result = CM_SUCCESS;
    pProgram = new CmProgramEmu( pCmDev );
    if( pProgram )
    {
        pProgram->Acquire();
    }
    else
    {
        CmAssert( 0 );
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

CmProgramEmu::CmProgramEmu( CmDeviceEmu* pCmDev ):
    m_pCmDev( pCmDev )
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
