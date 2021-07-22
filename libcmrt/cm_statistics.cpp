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

#include <fstream>
#include <iostream>
using namespace std;
#include "cm_include.h"
#include "cm_mem.h"

#include "cm_statistics.h"
#ifndef CMRT_VISA_EMU
#include "cm_task_emumode.h"
#endif

#include "cm_kernel_emumode.h"

#define STATISTIC_LOCATION_KEY_NAME "SOFTWARE\\Intel\\IGFX\\CM\\"
#define STATISTIC_VALUE_NAME        "enableStatistics"

CmStatistics * CmStatistics::m_pTracker = nullptr;

CmStatistics::CmStatistics(void)
            : m_lsRunnedKernel(nullptr)
{
}

bool
CmStatistics::IsEnabled(void)
{
    return false; //Currently NOT support for CmStatistics in Linux
}

CmStatistics *
CmStatistics::Create(void)
{

    ofstream    out("cmrt_dump.txt", ofstream::app);
    time_t currentTime;
    time(&currentTime);
    //Currently NOT support for CmStatistics in Linux

    if( m_pTracker == nullptr && IsEnabled() )
    {
        m_pTracker = new (std::nothrow) CmStatistics();
        if(m_pTracker == nullptr)
        {
            GFX_EMU_ASSERT( 0 );
            return nullptr;
        }
    }

    return m_pTracker;
}

CmStatistics::~CmStatistics(void)
{

    ofstream    out("cmrt_dump.txt", ofstream::app);
    out.width(30);
    out <<"KernelName";
    out.width(30);
    out <<"RunnedTimes"<<endl;
    CmRunnedKernels_t *p = m_lsRunnedKernel;
    for( ; p != nullptr;)
    {
        out.width(30);
        out<<p->m_strName;
        out.width(30);
        out<<p->m_nTimes<<endl;
        m_lsRunnedKernel = p;
        p = p->m_next;
        delete m_lsRunnedKernel;
        m_lsRunnedKernel = p;
    }
    out << endl;
    time_t currentTime;
    time(&currentTime);
    //Currently NOT support for CmStatistics in Linux
}

int
CmStatistics::Destroy(void)
{

    if( m_pTracker == nullptr )
    {
        return CM_SUCCESS;
    }

    delete m_pTracker;
    m_pTracker = nullptr;

    return CM_SUCCESS;
}

int
CmStatistics::TrackRunnedKernels( const CmTask * const pTask )
{

    if( pTask == nullptr )
    {
        return CM_FAILURE;
    }

    CmKernelArrayEmu *pTsk = (CmKernelArrayEmu *)pTask;
    int nKernels =  pTsk->GetKernelCount();

    for( int i = 0; i < nKernels; i++ )
    {
        CmKernelEmu *pKernel = dynamic_cast< CmKernelEmu * >(pTsk->GetKernelPointer(i));

        if( pKernel == nullptr )
        {
            return CM_FAILURE;
        }
        char *name = pKernel->GetName( );
        unsigned int len = (unsigned int)strnlen(name, CM_MAX_KERNEL_NAME_SIZE_IN_BYTE);
        if( len >= CM_MAX_KERNEL_NAME_SIZE_IN_BYTE )
        {
            GFX_EMU_ASSERT( 0 );
            return CM_FAILURE;
        }
        CmRunnedKernels_t *p = m_lsRunnedKernel;
        for( ; p != nullptr; p = p->m_next )
        {
            if( p->m_nLen == len && CmSafeMemCompare(name, p->m_strName, len) == 0 )
            {
                p->m_nTimes++;
                break;
            }
        }
        if( p == nullptr )
        {
            p = new (std::nothrow) CmRunnedKernels_t;
            if( p == nullptr )
            {
                GFX_EMU_ASSERT( 0 );
                return CM_OUT_OF_HOST_MEMORY;
            }
            CmSafeMemSet(p, 0, sizeof(CmRunnedKernels_t) );
            memcpy(p->m_strName, name, len);
            p->m_nLen   = len;
            p->m_nTimes = 1;
            p->m_next   = m_lsRunnedKernel;
            m_lsRunnedKernel = p;
        }
    }

    return CM_SUCCESS;
}
