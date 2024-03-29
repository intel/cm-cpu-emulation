/*========================== begin_copyright_notice ============================

Copyright (C) 2017 Intel Corporation

SPDX-License-Identifier: MIT

============================= end_copyright_notice ===========================*/

#ifndef GUARD_common_type_event_base_h
#define GUARD_common_type_event_base_h

#define CM_MAX_TIMEOUT 2 // Time in seconds before kernel should timeout
#define CM_MAX_TIMEOUT_MS CM_MAX_TIMEOUT*1000 // Time in milliseconds before kernel should timeout
//! \brief   A CmEvent object is genereated after a task is enqueued.
//! \details A CmEvent object is generated after a task(one kernel or multiples
//!          kernels running concurrently) is enqueued for execution.This
//!          event object can subsequently be queried to determine the status
//!          of execution and other execution related information.
class CmEvent
{
public:
    //!
    //! \brief      Query the status of the task associated with the event.
    //! \details    An event is generated when a task (one kernel or multiples
    //!             kernels running concurrently) is enqueued. This is a
    //!             non-blocking call.
    //! \param      [out] status
    //!             The reference to the execution status. Four status values,
    //!             CM_STATUS_QUEUED, CM_STATUS_FLUSHED, CM_STATUS_STARTED, and
    //!             CM_STATUS_FINISHED are supported. CM_STATUS_FLUSHED means
    //!             that the task is already sent to driver/hardware.
    //!             CM_STATUS_STARTED means hardware starts to execute the task.
    //!             CM_STATUS_FINISHED means hardware has finished the
    //!             execution of all kernels in the task. In Emulation
    //!             mode since the Enqueue operation is a blocking call, this
    //!             function always returns CM_STATUS_FINISHED.
    //! \retval     CM_SUCCESS if the status is successfully returned.
    //! \retval     CM_FAILURE if otherwise.
    //!
    CM_RT_API virtual int32_t GetStatus( CM_STATUS& status) = 0 ;

    //!
    //! \brief      Query the execution time of a task (one kernel or multiples
    //!             kernels running concurrently) in the unit of nanoseconds.
    //! \details    The execution time is measured from the time the task
    //!             started execution in GPU to the time when the task finished
    //!             execution.We recommend pair this API with
    //!             CmEvent::WaitForTaskFinished in practice when you try to
    //!             get GPU HW execution time. This is a non-blocking call.
    //!             In Emulation
    //!             mode this function always returns 100.
    //! \param      [out] time
    //!             Reference to time.
    //! \retval     CM_SUCCESS if the execution time is successfully returned.
    //! \retval     CM_FAILURE if not, e.g. the task hasn't finished.
    //!
    CM_RT_API virtual int32_t GetExecutionTime(unsigned long long& time) = 0;

    //!
    //! \brief      Wait for the task associated with the event finishing
    //!             execution on GPU (task status became CM_STATUS_FINISHED).
    //! \details    It applies event driven synchronization between GPU and CPU
    //!             to reduce CPU usage and power consumption in the waiting:
    //!             current process goes to sleep and waits for the
    //!             notification from OS until task finishes. We recommend use
    //!             this API followed by CmEvent::GetExecutionTime when you try
    //!             to get GPU HW execution time.
    //! \param      [in] timeOutMs
    //!             Timeout in milliseconds for the waiting, 2000 milliseconds
    //!             by-default.
    //! \retval     CM_SUCCESS if successfully wait and get notification from
    //!             OS.
    //! \retval     CM_NOT_IMPLEMENTED if DDI version is less than 2.4.
    //! \retval     CM_EVENT_DRIVEN_FAILURE otherwise.
    //! \note       This API is unnecessary to be added before the ReadSurface
    //!             method of class Surface2D/3D etc. We already apply
    //!             event-driven synchronization optimization inside
    //!             ReadSurface if the dependent event is given.
    //!
    CM_RT_API virtual int32_t WaitForTaskFinished(uint32_t timeOutMs = CM_MAX_TIMEOUT_MS) = 0;

    //!
    //! \brief      Gets surface details for GT-Pin.
    //! \param      [in] kernIndex
    //!             Index of the kernel which owns the surface.
    //! \param      [in] surfBTI
    //!             Index in Binding table for queried surface.
    //! \param      [out] outDetails
    //!             Detail info about this surface, which includes width,
    //!             height, depth, format, pitch, slice Pitch, surface base
    //!             address, tiled type, vertical offset, and horizontal offset.
    //! \retval     CM_SUCCESS if query is successfully finished.
    //! \retval     CM_NOT_IMPLEMENTED if DDI version is less than 2.3.
    //! \retval     CM_INVALID_ARG_VALUE if input parameter is invalid.
    //! \retval     CM_FAILURE otherwise.
    //!
    CM_RT_API virtual int32_t GetSurfaceDetails( uint32_t kernIndex, uint32_t surfBTI,CM_SURFACE_DETAILS& outDetails )=0;

    //!
    //! \brief      This function can be used to get more profiling
    //!             information for vTune.
    //! \details    It can provided 9 profiling values, for profiling
    //!             information,including
    //!             CM_EVENT_PROFILING_HWSTART,CM_EVENT_PROFILING_HWEND,
    //!             CM_EVENT_PROFILING_SUBMIT,CM_EVENT_PROFILING_COMPLETE,
    //!             CM_EVENT_PROFILING_ENQUEUE,CM_EVENT_PROFILING_KERNELCOUNT,
    //!             CM_EVENT_PROFILING_KERNELNAMES,
    //!             CM_EVENT_PROFILING_THREADSPACE,
    //!             CM_EVENT_PROFILING_CALLBACK.
    //! \param      [in] infoType
    //!             One value of CM_EVENT_PROFILING_INFO, specify which
    //!             information to get.
    //! \param      [in] paramSize
    //!             Size of the parameter.
    //! \param      [in] inputValue
    //!             Pointer pointing to memory where to get kernel index or
    //!             callback function.
    //! \param      [out] value
    //!             Pointer pointing to memory where the cap information should
    //!             be returned.
    //! \retval     CM_SUCCESS if get information successfully.
    //! \retval     CM_FAILURE otherwise.
    //!
    CM_RT_API virtual int32_t GetProfilingInfo(CM_EVENT_PROFILING_INFO infoType, size_t paramSize, void  *inputValue, void  *value) = 0;

    //!
    //! \brief      Query the raw tick time of a task(one kernel or multiples
    //!             kernels running concurrently).
    //! \details    We recommend pair this API with
    //!             CmEvent::WaitForTaskFinished in practice when you try to
    //!             get HW execution tick time.
    //! \param      [out] tick
    //!             Reference to time.
    //! \retval     CM_SUCCESS if the raw tick time is successfully returned.
    //! \retval     CM_FAILURE if not, e.g. the task hasn't finished.
    //!
    CM_RT_API virtual int32_t GetExecutionTickTime(unsigned long long& tick) = 0;

    virtual ~CmEvent () = default;
};
#endif // GUARD_common_type_event_base_h
