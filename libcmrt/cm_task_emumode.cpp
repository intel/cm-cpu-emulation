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
#include "cm_mem.h"
#include "cm_kernel_base.h"
#include "cm_task_emumode.h"

int32_t CmTask_RT::Create( uint32_t max_kernel_count, CmTask_RT* &pKernelArray)
{
    int32_t result = CM_SUCCESS;
    pKernelArray = new CmTask_RT( max_kernel_count );
    if( pKernelArray )
    {
        result = pKernelArray->Initialize();
        if( result != CM_SUCCESS )
        {
            CmTask_RT::Destroy( pKernelArray);
        }
    }
    else
    {
        CmAssert( 0 );
        result = CM_OUT_OF_HOST_MEMORY;
    }
    return result;

}

int32_t CmTask_RT::Destroy( CmTask_RT* &pKernelArray )
{
    CmSafeRelease(pKernelArray);

    return CM_SUCCESS;
}

CmTask_RT::CmTask_RT( uint32_t max_kernel_count ) :
    m_MaxKernelCount( max_kernel_count ), m_pKernelArray( nullptr ), m_KernelCount(0)
{
}

CmTask_RT::~CmTask_RT( void )
{
    CmSafeDeleteArray( m_pKernelArray );
}

int32_t CmTask_RT::Initialize( )
{
    m_pKernelArray = new CmKernel*[m_MaxKernelCount];

    if(m_pKernelArray)
    {
        CmSafeMemSet( m_pKernelArray, 0, sizeof(CmKernel*) * m_MaxKernelCount );
        return CM_SUCCESS;
    }
    else
    {
        CmAssert( 0 );
        return CM_OUT_OF_HOST_MEMORY;
    }
}

CM_RT_API int32_t CmTask_RT::AddKernel( CmKernel *pKernel )
{
    // already reached max kernel count
    if(m_MaxKernelCount <= m_KernelCount)
    {
        return CM_EXCEED_MAX_KERNEL_PER_ENQUEUE;
    }
    // passed in nullptr pointer
    if(pKernel == nullptr)
    {
        CmAssert( 0 );
        return CM_INVALID_ARG_VALUE;
    }

    m_pKernelArray[m_KernelCount] = pKernel;

    m_KernelCount++;

    return CM_SUCCESS;
}

CM_RT_API int32_t CmTask_RT::AddKernelWithConfig( CmKernel *pKernel, const CM_EXECUTION_CONFIG *config )
{
    return CM_SUCCESS;
}

CM_RT_API int32_t CmTask_RT::Reset( void )
{
    m_KernelCount = 0;
    if(m_pKernelArray)
    {
        CmSafeMemSet( m_pKernelArray, 0, sizeof(CmKernel*) * m_MaxKernelCount );
        return CM_SUCCESS;
    }
    else
    {
        CmAssert( 0 );
        return CM_FAILURE;
    }
}

CM_RT_API int32_t CmTask_RT::AddSync(void)
{
    return CM_SUCCESS;
}

uint32_t CmTask_RT::GetKernelCount()
{
    return m_KernelCount;
}

CmKernel* CmTask_RT::GetKernelPointer(uint32_t index)
{
    if(index >= m_KernelCount)
    {
        CmAssert(0);
        return nullptr;
    }
    return m_pKernelArray[index];
}

