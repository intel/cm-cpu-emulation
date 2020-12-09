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

#include "cm_event_base.h"
#include "cm_def.h"
#include "cm_debug.h"

class CmEvent;

class CmEventEmu : public CmEvent
{
public:

    static int32_t Create( uint32_t index, CmEventEmu* & pEvent );
    static int32_t Destroy( CmEventEmu* & pEvent );

    CM_RT_API int32_t GetStatus( CM_STATUS& status) ;
    CM_RT_API int32_t GetExecutionTime(unsigned long long& time);
    CM_RT_API int32_t WaitForTaskFinished(uint32_t dwTimeOutMs);

    int Acquire();
    int SafeRelease();

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
