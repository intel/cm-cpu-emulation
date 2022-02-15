/*========================== begin_copyright_notice ============================

Copyright (C) 2017 Intel Corporation

SPDX-License-Identifier: MIT

============================= end_copyright_notice ===========================*/

/////////////////////////////////////////////////////////////////////
#include <iostream>
#include <sstream>

using namespace std;
#include "cm_include.h"
#include "cm_mem.h"
#include "emu_log.h"

/////////////////////////////////////////////////////////////////////
#if defined(_WIN32)
#include "Iphlpapi.h"
#endif
#include "cm_statistics.h"
#ifndef CMRT_VISA_EMU
#include "cm_task_emumode.h"
#endif

#include "cm_kernel_emumode.h"

//////////////////////////////////////////////////////////////////////////////////
#define STATISTIC_LOCATION_KEY_NAME "SOFTWARE\\Intel\\IGFX\\CM\\"
#define STATISTIC_VALUE_NAME        "enableStatistics"

////////////////////////////////////////////////////////////////////////
CmStatistics * CmStatistics::m_pTracker = nullptr;

///////////////////////////////////////////////////////////////////////
CmStatistics::CmStatistics(void)
            : m_lsRunnedKernel(nullptr)
{
}
///////////////////////////////////////////////////////////////
#if defined(_WIN32)
bool
CmStatistics::IsEnabled(void)
{
    HKEY  dmKey;
    int32_t result = RegOpenKeyExA(HKEY_LOCAL_MACHINE, STATISTIC_LOCATION_KEY_NAME, 0, KEY_READ | KEY_WOW64_32KEY, &dmKey);
    if(ERROR_SUCCESS != result)
    {
        result = RegOpenKeyExA(HKEY_LOCAL_MACHINE, STATISTIC_LOCATION_KEY_NAME, 0, KEY_READ |  KEY_WOW64_64KEY, &dmKey);
        if( ERROR_SUCCESS != result )
        {
            return false;
        }
    }

    {
        //do something with the key
        unsigned long type;
        unsigned int enabled    = 0;
        unsigned int  len       = sizeof(enabled);

        int32_t readSuccess = RegQueryValueExA(dmKey, STATISTIC_VALUE_NAME, nullptr, &type, (LPBYTE)(&enabled), (LPDWORD)(&len));

        RegCloseKey(dmKey);

        // Check for success AND that the data is a null terminated string
        if(ERROR_SUCCESS != readSuccess)
        {
            return false;
        }
        else
        {
            if( enabled == 1)
            {
                return true;
            }
            else
            {
                return false;
            }
        }
    }
}
#else  //__GNUC__
bool
CmStatistics::IsEnabled(void)
{
    return false; //Currently NOT support for CmStatistics in Linux
}
#endif
/////////////////////////////////////////////////////////////
CmStatistics *
CmStatistics::Create(void)
{
    //////////////////////////////////////////////////////
    std::stringstream out;
    time_t currentTime;
    time(&currentTime);
#if defined(_WIN32)
    tm tm_lcltime;
    localtime_s(&tm_lcltime, &currentTime);
    char charstr_time[32];
    asctime_s( charstr_time, 32, &tm_lcltime);
    out << "CmDevice Created: " << charstr_time << endl;
    GFX_EMU_MESSAGE(fStat, "%s\n", out.str ().c_str ());
#else
    //Currently NOT support for CmStatistics in Linux
#endif

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
///////////////////////////////////////////////
CmStatistics::~CmStatistics(void)
{
    //////////////////////////////////////////////////////
    std::stringstream out;
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
#if defined(_WIN32)
    tm tm_lcltime;
    localtime_s(&tm_lcltime, &currentTime);
    char charstr_time[32];
    asctime_s( charstr_time, 32, &tm_lcltime);
    out << "CmDevice Destroyed: " << charstr_time << endl;
    GFX_EMU_MESSAGE(fStat, "%s\n", out.str ().c_str ());
#else
    //Currently NOT support for CmStatistics in Linux
#endif
}
/////////////////////////////////////////////////////////
int
CmStatistics::Destroy(void)
{
    //////////////////////////////////////////////////////
    if( m_pTracker == nullptr )
    {
        return CM_SUCCESS;
    }

    delete m_pTracker;
    m_pTracker = nullptr;

    return CM_SUCCESS;
}

////////////////////////////////////////////////////
int
CmStatistics::TrackRunnedKernels( const CmTask * const pTask )
{
    ////////////////////////////////////////////////////////////
    if( pTask == nullptr )
    {
        return CM_FAILURE;
    }

    //////////////////////////////////////////////////
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
