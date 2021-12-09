/*========================== begin_copyright_notice ============================

Copyright (C) 2021 Intel Corporation

SPDX-License-Identifier: MIT

============================= end_copyright_notice ===========================*/

#ifndef CM_EMU_SHIM_OCL_QUEUE_H
#define CM_EMU_SHIM_OCL_QUEUE_H

#include "context.h"
#include "runtime.h"

extern "C" {
struct _cl_command_queue {
  cl_icd_dispatch *dispatch;
};

CL_API_ENTRY CL_API_PREFIX__VERSION_1_2_DEPRECATED cl_command_queue CL_API_CALL
SHIM_CALL(clCreateCommandQueue)(cl_context                     context,
                                cl_device_id                   device,
                                cl_command_queue_properties    properties,
                                cl_int *                       errcode_ret) CL_API_SUFFIX__VERSION_1_2_DEPRECATED;

CL_API_ENTRY cl_command_queue CL_API_CALL
SHIM_CALL(clCreateCommandQueueWithProperties)(cl_context               context,
                                              cl_device_id             device,
                                              const cl_queue_properties *    properties,
                                              cl_int *                 errcode_ret) CL_API_SUFFIX__VERSION_2_0;

CL_API_ENTRY cl_int CL_API_CALL
SHIM_CALL(clRetainCommandQueue)(cl_command_queue command_queue) CL_API_SUFFIX__VERSION_1_0;

CL_API_ENTRY cl_int CL_API_CALL
SHIM_CALL(clReleaseCommandQueue)(cl_command_queue command_queue) CL_API_SUFFIX__VERSION_1_0;

CL_API_ENTRY cl_int CL_API_CALL
SHIM_CALL(clGetCommandQueueInfo)(cl_command_queue      command_queue,
                                 cl_command_queue_info param_name,
                                 size_t                param_value_size,
                                 void *                param_value,
                                 size_t *              param_value_size_ret) CL_API_SUFFIX__VERSION_1_0;

CL_API_ENTRY cl_int CL_API_CALL
SHIM_CALL(clSetCommandQueueProperty)(cl_command_queue              command_queue,
                                     cl_command_queue_properties   properties,
                                     cl_bool                       enable,
                                     cl_command_queue_properties * old_properties) CL_API_SUFFIX__VERSION_1_0_DEPRECATED;

CL_API_ENTRY cl_int CL_API_CALL
SHIM_CALL(clFlush)(cl_command_queue command_queue) CL_API_SUFFIX__VERSION_1_0;

CL_API_ENTRY cl_int CL_API_CALL
SHIM_CALL(clFinish)(cl_command_queue command_queue) CL_API_SUFFIX__VERSION_1_0;

CL_API_ENTRY cl_int CL_API_CALL
SHIM_CALL(clEnqueueReadBuffer)(cl_command_queue    command_queue,
                               cl_mem              buffer,
                               cl_bool             blocking_read,
                               size_t              offset,
                               size_t              size,
                               void *              ptr,
                               cl_uint             num_events_in_wait_list,
                               const cl_event *    event_wait_list,
                               cl_event *          event) CL_API_SUFFIX__VERSION_1_0;

CL_API_ENTRY cl_int CL_API_CALL
SHIM_CALL(clEnqueueWriteBuffer)(cl_command_queue   command_queue,
                                cl_mem             buffer,
                                cl_bool            blocking_write,
                                size_t             offset,
                                size_t             size,
                                const void *       ptr,
                                cl_uint            num_events_in_wait_list,
                                const cl_event *   event_wait_list,
                                cl_event *         event) CL_API_SUFFIX__VERSION_1_0;

CL_API_ENTRY cl_int CL_API_CALL
SHIM_CALL(clEnqueueNDRangeKernel)(cl_command_queue command_queue,
                                  cl_kernel        kernel,
                                  cl_uint          work_dim,
                                  const size_t *   global_work_offset,
                                  const size_t *   global_work_size,
                                  const size_t *   local_work_size,
                                  cl_uint          num_events_in_wait_list,
                                  const cl_event * event_wait_list,
                                  cl_event *       event) CL_API_SUFFIX__VERSION_1_0;

CL_API_ENTRY cl_int CL_API_CALL
SHIM_CALL(clEnqueueReadImage)(cl_command_queue     command_queue,
                              cl_mem               image,
                              cl_bool              blocking_read,
                              const size_t *       origin,
                              const size_t *       region,
                              size_t               row_pitch,
                              size_t               slice_pitch,
                              void *               ptr,
                              cl_uint              num_events_in_wait_list,
                              const cl_event *     event_wait_list,
                              cl_event *           event) CL_API_SUFFIX__VERSION_1_0;

CL_API_ENTRY cl_int CL_API_CALL
SHIM_CALL(clEnqueueWriteImage)(cl_command_queue    command_queue,
                               cl_mem              image,
                               cl_bool             blocking_write,
                               const size_t *      origin,
                               const size_t *      region,
                               size_t              input_row_pitch,
                               size_t              input_slice_pitch,
                               const void *        ptr,
                               cl_uint             num_events_in_wait_list,
                               const cl_event *    event_wait_list,
                               cl_event *          event) CL_API_SUFFIX__VERSION_1_0;

CL_API_ENTRY cl_int CL_API_CALL
SHIM_CALL(clEnqueueSVMMap)(cl_command_queue  command_queue,
                           cl_bool           blocking_map,
                           cl_map_flags      flags,
                           void *            svm_ptr,
                           size_t            size,
                           cl_uint           num_events_in_wait_list,
                           const cl_event *  event_wait_list,
                           cl_event *        event) CL_API_SUFFIX__VERSION_2_0;

CL_API_ENTRY cl_int CL_API_CALL
SHIM_CALL(clEnqueueSVMUnmap)(cl_command_queue  command_queue,
                             void *            svm_ptr,
                             cl_uint           num_events_in_wait_list,
                             const cl_event *  event_wait_list,
                             cl_event *        event) CL_API_SUFFIX__VERSION_2_0;
}

namespace shim {
namespace cl {
struct Queue : public _cl_command_queue, public IntrusiveRefCounter<Queue> {
  Queue(IntrusivePtr<Context> ctx) : ctx_(ctx) {
    dispatch = ctx_->dispatch;
    if (auto status = ctx->dev_.device->CreateQueue(queue_)) {
      throw Error(::GetCmErrorString(status));
    }
  }

  IntrusivePtr<Context> ctx_;
  CmQueue *queue_;
};
} // namespace cl
} // namespace shim

#endif // CM_EMU_SHIM_OCL_QUEUE_H
