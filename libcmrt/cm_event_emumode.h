/*========================== begin_copyright_notice ============================

Copyright (C) 2017 Intel Corporation

SPDX-License-Identifier: MIT

============================= end_copyright_notice ===========================*/

#pragma once

#include "cm_event_base.h"
#include "cm_def.h"
#include "emu_log.h"

class CmEvent;

class CmEventEmu : public CmEvent
{
public:

    static int32_t Create( uint32_t index, CmEventEmu* & pEvent );
    static int32_t Destroy( CmEventEmu* & pEvent );

    CM_RT_API int32_t GetStatus( CM_STATUS& status) ;
    CM_RT_API int32_t GetExecutionTime(unsigned long long& time);
    CM_RT_API int32_t WaitForTaskFinished(uint32_t dwTimeOutMs);

    CM_RT_API int Acquire();
    CM_RT_API int SafeRelease();

    //GT-PIN
    CM_RT_API int32_t GetSurfaceDetails(uint32_t kernIndex, uint32_t surfBTI,CM_SURFACE_DETAILS& outDetails ) { return CmNotImplemented(__PRETTY_FUNCTION__); }

    int32_t GetIndex( uint32_t & index );
    CM_RT_API int32_t GetProfilingInfo(CM_EVENT_PROFILING_INFO infoType, size_t paramSize, void  *pInputValue, void  *pValue) { return CmNotImplemented(__PRETTY_FUNCTION__); };
	CM_RT_API int32_t GetExecutionTickTime(unsigned long long& tick);
protected:
    CmEventEmu( uint32_t index  );
    ~CmEventEmu( void );

    uint32_t m_Index;
    int m_RefCount;

};
