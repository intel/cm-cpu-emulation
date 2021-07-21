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

#include "cm_include.h"
#include "cm_event_emumode.h"

int32_t CmEventEmu::Create( uint32_t index, CmEventEmu* & pEvent )
{
    int32_t result = CM_SUCCESS;
    pEvent = new CmEventEmu( index );
    if( pEvent )
    {
        pEvent->Acquire();
    }
    else
    {
        GFX_EMU_ASSERT( 0 );
        result = CM_OUT_OF_HOST_MEMORY;
    }
    return result;

}

int32_t CmEventEmu::Destroy( CmEventEmu* &pEvent )
{
    pEvent->SafeRelease();
    pEvent = nullptr;
    return CM_SUCCESS;
}

CmEventEmu::CmEventEmu( uint32_t index ): m_Index( index ), m_RefCount(0)
{

}

CmEventEmu::~CmEventEmu( void )
{

}

//!
//! Query the status of the task associated with the event
//! An event is generated when a task ( one kernel or multiples kernels running concurrently )
//! is enqueued.
//! This is a non-blocking call.
//! INPUT:
//!     The reference to status. For now only two status, CM_STATUS_QUEUED and CM_STATUS_FINISHED, are supported
//! OUTPUT:
//!     CM_SUCCESS if the status is successfully returned;
//!     CM_FAILURE if not.
//!
CM_RT_API int32_t CmEventEmu::GetStatus( CM_STATUS& status)
{
    status = CM_STATUS_FINISHED;
    return CM_SUCCESS;
}

//!
//! Wait for the task completed associated with the event
//! An internal event is generated when a task ( one kernel or multiples kernels running concurrently )
//! is enqueued.
//! INPUT:
//!     Timout in Milliseconds
//! OUTPUT:
//!     CM_SUCCESS if the status is successfully returned;
//!     CM_FAILURE if not.
//!
CM_RT_API int32_t CmEventEmu::WaitForTaskFinished(uint32_t dwTimeOutMs)
{
    CM_STATUS status;
    GetStatus(status);
    while( status != CM_STATUS_FINISHED)
    {
        GetStatus(status);
    }
    return CM_SUCCESS;
}

//!
//! Query the execution time of a task( one kernel or multiples kernels running concurrently )
//! in the unit of nanoseconds.
//! The execution time is from the time when the task starts to execution in GPU to the time
//! when the task finished execution
//! This is a non-blocking call.
//! INPUT:
//!     Reference to time
//! OUTPUT:
//!     CM_SUCCESS if the execution time is successfully returned
//!     CM_FAILURE if not, e.g. the task hasn't finished
//!
CM_RT_API int32_t CmEventEmu::GetExecutionTime(unsigned long long& time)
{
    time = 100;
    return CM_SUCCESS;
}

//! This is a non-blocking call.
//! INPUT:
//!     Reference to tick number
//! OUTPUT:
//!     CM_SUCCESS if the execution time is successfully returned
//!     CM_FAILURE if not, e.g. the task hasn't finished
//!
CM_RT_API int32_t CmEventEmu::GetExecutionTickTime(unsigned long long& tick)
{
    tick = 100;
    return CM_SUCCESS;
}

int32_t CmEventEmu::GetIndex( uint32_t & index )
{
    index = m_Index;
    return CM_SUCCESS;
}

//*-----------------------------------------------------------------------------
//| Purpose:    Increase Reference count
//| Returns:    Result of the operation.
//*-----------------------------------------------------------------------------
int CmEventEmu::Acquire()
{
    ++m_RefCount;
    return m_RefCount;
}

//*-----------------------------------------------------------------------------
//| Purpose:    De of Cm Event
//| Returns:    Result of the operation.
//*-----------------------------------------------------------------------------
int CmEventEmu::SafeRelease()
{
    --m_RefCount;
    if (m_RefCount == 0)
    {
        delete this;
        return 0;
    }
    else
    {
        return m_RefCount;
    }
}
