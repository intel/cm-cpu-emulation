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

#include "queue.h"

#include "event.h"
#include "kernel.h"
#include "memory.h"

CL_API_ENTRY cl_command_queue CL_API_CALL
SHIM_CALL(clCreateCommandQueue)(cl_context                     context,
                                cl_device_id                   device,
                                cl_command_queue_properties    properties,
                                cl_int *                       errcode_ret) {
  return SHIM_CALL(clCreateCommandQueueWithProperties)(context, device, nullptr, errcode_ret);
}

CL_API_ENTRY cl_command_queue CL_API_CALL
SHIM_CALL(clCreateCommandQueueWithProperties)(cl_context               context,
                                              cl_device_id             device,
                                              const cl_queue_properties *    properties,
                                              cl_int *                 errcode_ret) CL_API_SUFFIX__VERSION_2_0 {
  shim::IntrusivePtr<shim::cl::Context> ctx(static_cast<shim::cl::Context*>(context));

  ERRCODE(CL_SUCCESS);

  if (!ctx) {
    ERRCODE(CL_INVALID_CONTEXT);
    return nullptr;
  }

  if (device != &ctx->dev_) {
    ERRCODE(CL_INVALID_DEVICE);
    return nullptr;
  }

  shim::IntrusivePtr<shim::cl::Queue> queue = new(std::nothrow) shim::cl::Queue(ctx);

  if (!queue) {
    ERRCODE(CL_OUT_OF_HOST_MEMORY);
    return nullptr;
  }

  IntrusivePtrAddRef(queue.get());
  return queue.get();
}

CL_API_ENTRY cl_int CL_API_CALL
SHIM_CALL(clRetainCommandQueue)(cl_command_queue command_queue) CL_API_SUFFIX__VERSION_1_0 {
  shim::IntrusivePtrAddRef(static_cast<shim::cl::Queue*>(command_queue));
  return CL_SUCCESS;
}

CL_API_ENTRY cl_int CL_API_CALL
SHIM_CALL(clReleaseCommandQueue)(cl_command_queue command_queue) CL_API_SUFFIX__VERSION_1_0 {
  shim::IntrusivePtr<shim::cl::Queue> queue(static_cast<shim::cl::Queue*>(command_queue), false);
  return CL_SUCCESS;
}

CL_API_ENTRY cl_int CL_API_CALL
SHIM_CALL(clGetCommandQueueInfo)(cl_command_queue      command_queue,
                                 cl_command_queue_info param_name,
                                 size_t                param_value_size,
                                 void *                param_value,
                                 size_t *              param_value_size_ret) CL_API_SUFFIX__VERSION_1_0 {
  return CL_INVALID_VALUE;
}

CL_API_ENTRY cl_int CL_API_CALL
SHIM_CALL(clSetCommandQueueProperty)(cl_command_queue              command_queue,
                                     cl_command_queue_properties   properties,
                                     cl_bool                       enable,
                                     cl_command_queue_properties * old_properties) {
  return CL_INVALID_VALUE;
}

CL_API_ENTRY cl_int CL_API_CALL
SHIM_CALL(clFlush)(cl_command_queue command_queue) CL_API_SUFFIX__VERSION_1_0 {
  return CL_SUCCESS;
}

CL_API_ENTRY cl_int CL_API_CALL
SHIM_CALL(clFinish)(cl_command_queue command_queue) CL_API_SUFFIX__VERSION_1_0 {
  return CL_SUCCESS;
}

CL_API_ENTRY cl_int CL_API_CALL
SHIM_CALL(clEnqueueReadBuffer)(cl_command_queue    command_queue,
                               cl_mem              buffer,
                               cl_bool             blocking_read,
                               size_t              offset,
                               size_t              size,
                               void *              ptr,
                               cl_uint             num_events_in_wait_list,
                               const cl_event *    event_wait_list,
                               cl_event *          event) CL_API_SUFFIX__VERSION_1_0 {
  shim::IntrusivePtr<shim::cl::Queue> queue(static_cast<shim::cl::Queue*>(command_queue));
  shim::IntrusivePtr<shim::cl::Memory> memory(static_cast<shim::cl::Memory*>(buffer));

  if (auto status = SHIM_CALL(clWaitForEvents)(num_events_in_wait_list, event_wait_list);
      status != CL_SUCCESS) {
    return status;
  }

  if (offset != 0) {
    return CL_INVALID_VALUE;
  }

  if (auto **buf = std::get_if<CmBuffer*>(&memory->buffer_);
      !buf || (*buf)->ReadSurface(static_cast<unsigned char*>(ptr), nullptr, size) != CL_SUCCESS) {
    return CL_INVALID_MEM_OBJECT;
  }

  if (event) {
    shim::IntrusivePtr<shim::cl::Event> evt = new(std::nothrow) shim::cl::Event(queue);
    shim::IntrusivePtrAddRef(evt.get());
    *event = evt.get();
  }

  return CL_SUCCESS;
}

CL_API_ENTRY cl_int CL_API_CALL
SHIM_CALL(clEnqueueWriteBuffer)(cl_command_queue    command_queue,
                                cl_mem              buffer,
                                cl_bool             blocking_read,
                                size_t              offset,
                                size_t              size,
                                const void *        ptr,
                                cl_uint             num_events_in_wait_list,
                                const cl_event *    event_wait_list,
                                cl_event *          event) CL_API_SUFFIX__VERSION_1_0 {
  shim::IntrusivePtr<shim::cl::Queue> queue(static_cast<shim::cl::Queue*>(command_queue));
  shim::IntrusivePtr<shim::cl::Memory> memory(static_cast<shim::cl::Memory*>(buffer));

  if (auto status = SHIM_CALL(clWaitForEvents)(num_events_in_wait_list, event_wait_list);
      status != CL_SUCCESS) {
    return status;
  }

  if (offset != 0) {
    return CL_INVALID_VALUE;
  }

  if (auto **buf = std::get_if<CmBuffer*>(&memory->buffer_);
      !buf || (*buf)->WriteSurface(static_cast<const unsigned char*>(ptr), nullptr, size) != CL_SUCCESS) {
    return CL_INVALID_MEM_OBJECT;
  }

  if (event) {
    shim::IntrusivePtr<shim::cl::Event> evt = new(std::nothrow) shim::cl::Event(queue);
    shim::IntrusivePtrAddRef(evt.get());
    *event = evt.get();
  }

  return CL_SUCCESS;
}

CL_API_ENTRY cl_int CL_API_CALL
SHIM_CALL(clEnqueueNDRangeKernel)(cl_command_queue command_queue,
                                  cl_kernel        kernel,
                                  cl_uint          work_dim,
                                  const size_t *   global_work_offset,
                                  const size_t *   global_work_size,
                                  const size_t *   local_work_size,
                                  cl_uint          num_events_in_wait_list,
                                  const cl_event * event_wait_list,
                                  cl_event *       event) CL_API_SUFFIX__VERSION_1_0 {
  shim::IntrusivePtr<shim::cl::Queue> queue(static_cast<shim::cl::Queue*>(command_queue));
  shim::IntrusivePtr<shim::cl::Kernel> kern(static_cast<shim::cl::Kernel*>(kernel));

  if (work_dim > 3 || (work_dim == 3 && (global_work_size[2] > 1 || local_work_size[2] > 1))) {
    return CL_OUT_OF_RESOURCES;
  }

  if (auto status = SHIM_CALL(clWaitForEvents)(num_events_in_wait_list, event_wait_list);
      status != CL_SUCCESS) {
    return status;
  }

  size_t groups[2] = {
    global_work_size[0] / local_work_size[0],
    work_dim > 1 ? global_work_size[1] / local_work_size[1] : 1,
  };

  CmThreadGroupSpace *thread_group_space = nullptr;
  if (queue->ctx_->dev_.device->CreateThreadGroupSpace(
          local_work_size[0], work_dim > 1 ? local_work_size[1] : 1,
          groups[0], groups[1], thread_group_space) != CM_SUCCESS) {
    return CL_OUT_OF_RESOURCES;
  }

  CmTask *task = nullptr;
  if (queue->ctx_->dev_.device->CreateTask(task) != CM_SUCCESS) {
    queue->ctx_->dev_.device->DestroyThreadGroupSpace(thread_group_space);
    return CL_OUT_OF_RESOURCES;
  }

  if (task->AddKernel(kern->kern_.get()) != CM_SUCCESS) {
    queue->ctx_->dev_.device->DestroyThreadGroupSpace(thread_group_space);
    queue->ctx_->dev_.device->DestroyTask(task);
    return CL_OUT_OF_RESOURCES;
  }

  CmEvent *e = nullptr;
  if (queue->queue_->EnqueueWithGroup(task, e, thread_group_space)) {
    queue->ctx_->dev_.device->DestroyThreadGroupSpace(thread_group_space);
    queue->ctx_->dev_.device->DestroyTask(task);
    return CL_OUT_OF_RESOURCES;
  }

  if (event) {
    shim::IntrusivePtr<shim::cl::Event> evt = new(std::nothrow) shim::cl::Event(queue, e, thread_group_space, task);
    if (!evt) {
      e->WaitForTaskFinished();
    } else {
      IntrusivePtrAddRef(evt.get());
      *event = evt.get();
    }
  } else {
    e->WaitForTaskFinished();
  }

  return CL_SUCCESS;
}

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
                              cl_event *           event) CL_API_SUFFIX__VERSION_1_0 {
  shim::IntrusivePtr<shim::cl::Queue> queue(static_cast<shim::cl::Queue*>(command_queue));
  shim::IntrusivePtr<shim::cl::Memory> memory(static_cast<shim::cl::Memory*>(image));

  if (auto status = SHIM_CALL(clWaitForEvents)(num_events_in_wait_list, event_wait_list);
      status != CL_SUCCESS) {
    return status;
  }

  if (row_pitch || slice_pitch) {
    return CL_OUT_OF_RESOURCES;
  }

  if (auto **buf = std::get_if<CmSurface2D*>(&memory->buffer_);
      !buf || (*buf)->ReadSurface(static_cast<unsigned char*>(ptr), nullptr) != CL_SUCCESS) {
    return CL_INVALID_MEM_OBJECT;
  }

  if (event) {
    shim::IntrusivePtr<shim::cl::Event> evt = new(std::nothrow) shim::cl::Event(queue);
    shim::IntrusivePtrAddRef(evt.get());
    *event = evt.get();
  }

  return CL_SUCCESS;
}

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
                               cl_event *          event) CL_API_SUFFIX__VERSION_1_0 {
  shim::IntrusivePtr<shim::cl::Queue> queue(static_cast<shim::cl::Queue*>(command_queue));
  shim::IntrusivePtr<shim::cl::Memory> memory(static_cast<shim::cl::Memory*>(image));

  if (auto status = SHIM_CALL(clWaitForEvents)(num_events_in_wait_list, event_wait_list);
      status != CL_SUCCESS) {
    return status;
  }

  if (input_row_pitch || input_slice_pitch) {
    return CL_OUT_OF_RESOURCES;
  }

  if (auto **buf = std::get_if<CmSurface2D*>(&memory->buffer_);
      !buf || (*buf)->WriteSurface(static_cast<const unsigned char*>(ptr), nullptr) != CL_SUCCESS) {
    return CL_INVALID_MEM_OBJECT;
  }

  if (event) {
    shim::IntrusivePtr<shim::cl::Event> evt = new(std::nothrow) shim::cl::Event(queue);
    shim::IntrusivePtrAddRef(evt.get());
    *event = evt.get();
  }

  return CL_SUCCESS;
}

CL_API_ENTRY cl_int CL_API_CALL
SHIM_CALL(clEnqueueSVMMap)(cl_command_queue  command_queue,
                           cl_bool           blocking_map,
                           cl_map_flags      flags,
                           void *            svm_ptr,
                           size_t            size,
                           cl_uint           num_events_in_wait_list,
                           const cl_event *  event_wait_list,
                           cl_event *        event) CL_API_SUFFIX__VERSION_2_0 {
  if (auto status = SHIM_CALL(clWaitForEvents)(num_events_in_wait_list, event_wait_list);
      status != CL_SUCCESS) {
    return status;
  }

  if (event) {
    shim::IntrusivePtr<shim::cl::Queue> queue(static_cast<shim::cl::Queue*>(command_queue));
    shim::IntrusivePtr<shim::cl::Event> evt = new(std::nothrow) shim::cl::Event(queue);
    shim::IntrusivePtrAddRef(evt.get());
    *event = evt.get();
  }

  return CL_SUCCESS;
}

CL_API_ENTRY cl_int CL_API_CALL
SHIM_CALL(clEnqueueSVMUnmap)(cl_command_queue  command_queue,
                             void *            svm_ptr,
                             cl_uint           num_events_in_wait_list,
                             const cl_event *  event_wait_list,
                             cl_event *        event) CL_API_SUFFIX__VERSION_2_0 {
  if (auto status = SHIM_CALL(clWaitForEvents)(num_events_in_wait_list, event_wait_list);
      status != CL_SUCCESS) {
    return status;
  }

  if (event) {
    shim::IntrusivePtr<shim::cl::Queue> queue(static_cast<shim::cl::Queue*>(command_queue));
    shim::IntrusivePtr<shim::cl::Event> evt = new(std::nothrow) shim::cl::Event(queue);
    shim::IntrusivePtrAddRef(evt.get());
    *event = evt.get();
  }

  return CL_SUCCESS;
}

extern "C" {
SHIM_EXPORT(clCreateCommandQueue);
SHIM_EXPORT(clCreateCommandQueueWithProperties);
SHIM_EXPORT(clRetainCommandQueue);
SHIM_EXPORT(clReleaseCommandQueue);
SHIM_EXPORT(clGetCommandQueueInfo);
SHIM_EXPORT(clSetCommandQueueProperty);
SHIM_EXPORT(clFlush);
SHIM_EXPORT(clFinish);
SHIM_EXPORT(clEnqueueReadBuffer);
SHIM_EXPORT(clEnqueueWriteBuffer);
SHIM_EXPORT(clEnqueueNDRangeKernel);
SHIM_EXPORT(clEnqueueReadImage);
SHIM_EXPORT(clEnqueueWriteImage);
SHIM_EXPORT(clEnqueueSVMMap);
SHIM_EXPORT(clEnqueueSVMUnmap);
}
