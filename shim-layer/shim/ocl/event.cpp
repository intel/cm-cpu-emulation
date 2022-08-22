/*========================== begin_copyright_notice ============================

Copyright (C) 2021 Intel Corporation

SPDX-License-Identifier: MIT

============================= end_copyright_notice ===========================*/

#include "event.h"
#include "runtime.h"

CL_API_ENTRY cl_int CL_API_CALL SHIM_CALL(clWaitForEvents)(
    cl_uint num_events, const cl_event *event_list) CL_API_SUFFIX__VERSION_1_0 {
  for (int i = 0; i < num_events; i++) {
    shim::IntrusivePtr<shim::cl::Event> event(
        static_cast<shim::cl::Event *>(event_list[i]));

    if (!event->fake_ && event->event_->WaitForTaskFinished() != CL_SUCCESS) {
      return CL_EXEC_STATUS_ERROR_FOR_EVENTS_IN_WAIT_LIST;
    }
  }

  return CL_SUCCESS;
}

CL_API_ENTRY cl_int CL_API_CALL SHIM_CALL(clRetainEvent)(cl_event event)
    CL_API_SUFFIX__VERSION_1_0 {
  shim::IntrusivePtrAddRef(static_cast<shim::cl::Event *>(event));
  return CL_SUCCESS;
}

CL_API_ENTRY cl_int CL_API_CALL SHIM_CALL(clReleaseEvent)(cl_event event)
    CL_API_SUFFIX__VERSION_1_0 {
  shim::IntrusivePtr<shim::cl::Event> k(static_cast<shim::cl::Event *>(event),
                                        false);
  return CL_SUCCESS;
}

CL_API_ENTRY cl_int CL_API_CALL SHIM_CALL(clGetEventProfilingInfo)(
    cl_event event, cl_profiling_info param_name, size_t param_value_size,
    void *param_value,
    size_t *param_value_size_ret) CL_API_SUFFIX__VERSION_1_0 {
  shim::IntrusivePtr<shim::cl::Event> e(static_cast<shim::cl::Event *>(event));

  // Profiling is not implemented in EMU
  switch (param_name) {
  case CL_PROFILING_COMMAND_QUEUED:
    return shim::cl::SetResult<cl_ulong>(0, param_value_size, param_value,
                                         param_value_size_ret);
  case CL_PROFILING_COMMAND_SUBMIT:
    return shim::cl::SetResult<cl_ulong>(100, param_value_size, param_value,
                                         param_value_size_ret);
  case CL_PROFILING_COMMAND_START:
    return shim::cl::SetResult<cl_ulong>(200, param_value_size, param_value,
                                         param_value_size_ret);
  case CL_PROFILING_COMMAND_END:
    return shim::cl::SetResult<cl_ulong>(300, param_value_size, param_value,
                                         param_value_size_ret);
  case CL_PROFILING_COMMAND_COMPLETE:
    return shim::cl::SetResult<cl_ulong>(400, param_value_size, param_value,
                                         param_value_size_ret);
  }

  return CL_INVALID_VALUE;
}

extern "C" {
SHIM_EXPORT(clWaitForEvents);
SHIM_EXPORT(clRetainEvent);
SHIM_EXPORT(clReleaseEvent);
SHIM_EXPORT(clGetEventProfilingInfo);
}
