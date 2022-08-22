/*========================== begin_copyright_notice ============================

Copyright (C) 2021 Intel Corporation

SPDX-License-Identifier: MIT

============================= end_copyright_notice ===========================*/

#ifndef CM_EMU_SHIM_OCL_EVENT_H
#define CM_EMU_SHIM_OCL_EVENT_H

#include <memory>
#include <unordered_set>

#include "ocl.h"
#include "queue.h"

extern "C" {
struct _cl_event {
  cl_icd_dispatch *dispatch;
};

CL_API_ENTRY cl_int CL_API_CALL SHIM_CALL(clWaitForEvents)(
    cl_uint num_events, const cl_event *event_list) CL_API_SUFFIX__VERSION_1_0;

CL_API_ENTRY cl_int CL_API_CALL SHIM_CALL(clRetainEvent)(cl_event event)
    CL_API_SUFFIX__VERSION_1_0;

CL_API_ENTRY cl_int CL_API_CALL SHIM_CALL(clReleaseEvent)(cl_event event)
    CL_API_SUFFIX__VERSION_1_0;

CL_API_ENTRY cl_int CL_API_CALL SHIM_CALL(clGetEventProfilingInfo)(
    cl_event event, cl_profiling_info param_name, size_t param_value_size,
    void *param_value, size_t *param_value_size_ret) CL_API_SUFFIX__VERSION_1_0;
}

namespace shim {
namespace cl {

struct Event : public _cl_event, public IntrusiveRefCounter<Event> {
  Event(IntrusivePtr<Queue> q, CmEvent *e, CmThreadGroupSpace *tgs,
        CmTask *task)
      : fake_(false),
        event_(e, [q](CmEvent *e) { q->queue_->DestroyEvent(e); }),
        tgs_(tgs,
             [dev = q->ctx_->dev_.device](CmThreadGroupSpace *tgs) {
               dev->DestroyThreadGroupSpace(tgs);
             }),
        task_(task, [dev = q->ctx_->dev_.device](CmTask *task) {
          dev->DestroyTask(task);
        }) {
    dispatch = q->dispatch;
  }

  Event(IntrusivePtr<Queue> q) : fake_(true) { dispatch = q->dispatch; }

  bool fake_;
  std::shared_ptr<CmEvent> event_;
  std::shared_ptr<CmThreadGroupSpace> tgs_;
  std::shared_ptr<CmTask> task_;
};

} // namespace cl
} // namespace shim

#endif // CM_EMU_SHIM_OCL_EVENT_H
