/*========================== begin_copyright_notice ============================

Copyright (C) 2022 Intel Corporation

SPDX-License-Identifier: MIT

============================= end_copyright_notice ===========================*/

#include "utils.h"
#include "cmdlist.h"

ze_result_t shim::ze::Queue::Execute(const shim::ze::Command &cmd) {
  for (auto &event : cmd.wait_) {
    ze_event_handle_t h = reinterpret_cast<ze_event_handle_t>(event.get());
    if (auto r = SHIM_CALL(zeEventHostSynchronize)(h, ~0ull); r != ZE_RESULT_SUCCESS) {
      return ZE_RESULT_ERROR_DEVICE_LOST;
    }
  }

  switch (cmd.type_) {
    case shim::ze::Command::Type::Barrier:
      break;
    case shim::ze::Command::Type::EventReset:
      if (auto r = SHIM_CALL(zeEventHostReset)(reinterpret_cast<ze_event_handle_t>(cmd.events_[0].get()));
          r != ZE_RESULT_SUCCESS) {
        return ZE_RESULT_ERROR_DEVICE_LOST;
      }
      break;
    case shim::ze::Command::Type::EventTimestamp: {
      for (size_t i = 0; i < cmd.events_.size(); i++) {
        ze_kernel_timestamp_result_t *dst = nullptr;

        if (cmd.timestamps_.offsets) {
          dst = reinterpret_cast<ze_kernel_timestamp_result_t*>(
              static_cast<char*>(cmd.timestamps_.dst) + cmd.timestamps_.offsets[i]);
        } else {
          dst = &reinterpret_cast<ze_kernel_timestamp_result_t*>(cmd.timestamps_.dst)[i];
        }

        auto r = SHIM_CALL(zeEventQueryKernelTimestamp)(
            reinterpret_cast<ze_event_handle_t>(cmd.events_[i].get()),
            dst);

        if (r != ZE_RESULT_SUCCESS) {
          return ZE_RESULT_ERROR_DEVICE_LOST;
        }
      }
      break;
    }
    case shim::ze::Command::Type::BufferCopy:
      // All the allocations are present by host pointers. According to
      // LevelZero spec, the regions shall not be overlapped.
      std::memcpy(cmd.copy_.dst, cmd.copy_.src, cmd.copy_.size);
      break;
    case shim::ze::Command::Type::BufferFill:
      std::memset(cmd.fill_.dst, cmd.fill_.pattern, cmd.copy_.size);
      break;
    case shim::ze::Command::Type::ImageCopyFromMem: {
      auto r = std::visit([src=cmd.copy_.src](auto &surf) {
                            return surf->WriteSurface(reinterpret_cast<const uint8_t *>(src), nullptr);
                          }, cmd.image_->surface_);
      if (r != CM_SUCCESS) {
        return ZE_RESULT_ERROR_DEVICE_LOST;
      }

      break;
    }
    case shim::ze::Command::Type::ImageCopyToMem: {
      auto r = std::visit([dst=cmd.copy_.dst](auto &surf) {
                            return surf->ReadSurface(reinterpret_cast<uint8_t *>(dst), nullptr);
                          }, cmd.image_->surface_);
      if (r != CM_SUCCESS) {
        return ZE_RESULT_ERROR_DEVICE_LOST;
      }

      break;
    }
    case shim::ze::Command::Type::Kernel: {
      CmTask *task = nullptr;
      CmThreadGroupSpace *tgs = nullptr;
      auto _ = shim::finally([&]() {
                               if (task != nullptr) {
                                 dev_->DestroyTask(task);
                               }
                               if (tgs != nullptr) {
                                 dev_->DestroyThreadGroupSpace(tgs);
                               }
                             });

      if (auto r = dev_->CreateTask(task); r != CM_SUCCESS) {
        return ZE_RESULT_ERROR_DEVICE_LOST;
      }

      auto kernel = cmd.kernel_;
      auto &threads = kernel->group_size_[kernel->kernel_.get()];

      if (auto r = dev_->CreateThreadGroupSpace(threads.groupSizeX, threads.groupSizeY,
                                                cmd.group_count_.groupCountX,
                                                cmd.group_count_.groupCountY,
                                                tgs);
          r != CM_SUCCESS) {
        return ZE_RESULT_ERROR_DEVICE_LOST;
      }

      kernel->kernel_->AssociateThreadGroupSpace(tgs);
      task->AddKernel(cmd.kernel_->kernel_.get());

      CmEvent *event = nullptr;
      if (auto r = queue_->EnqueueWithGroup(task, event); r != CM_SUCCESS) {
        return ZE_RESULT_ERROR_DEVICE_LOST;
      }

      if (cmd.signal_) {
        cmd.signal_->event_ = dynamic_cast<CmEventEmu*>(event);
        event_ = cmd.signal_;
      } else if (auto r = event->WaitForTaskFinished(~0u); r != CM_SUCCESS) {
        return ZE_RESULT_ERROR_DEVICE_LOST;
      }

      return ZE_RESULT_SUCCESS;
    }
    default:
      return ZE_RESULT_ERROR_UNSUPPORTED_FEATURE;
  }

  if (cmd.signal_) {
    cmd.signal_->signalled_ = true;
  }

  event_ = nullptr;

  return ZE_RESULT_SUCCESS;
}

extern "C" {
SHIM_EXPORT(zeCommandQueueCreate);
SHIM_EXPORT(zeCommandQueueDestroy);
SHIM_EXPORT(zeCommandQueueExecuteCommandLists);
SHIM_EXPORT(zeCommandQueueSynchronize);
} // extern "C"

ZE_APIEXPORT ze_result_t ZE_APICALL SHIM_CALL(zeCommandQueueCreate)(
    ze_context_handle_t hContext,
    ze_device_handle_t hDevice,
    const ze_command_queue_desc_t *desc,
    ze_command_queue_handle_t *phCommandQueue) {
  if (hContext == nullptr || hDevice == nullptr) {
    return ZE_RESULT_ERROR_INVALID_NULL_HANDLE;
  }

  if (desc->flags > 1 || desc->mode > ZE_COMMAND_QUEUE_MODE_ASYNCHRONOUS ||
      desc->priority > ZE_COMMAND_QUEUE_PRIORITY_PRIORITY_HIGH) {
    return ZE_RESULT_ERROR_INVALID_ENUMERATION;
  }

  shim::IntrusivePtr<shim::ze::Context> ctx(reinterpret_cast<shim::ze::Context*>(hContext));
  shim::IntrusivePtr<CmDeviceEmu> dev(reinterpret_cast<CmDeviceEmu*>(hDevice));

  try {
    auto queue = shim::MakeIntrusive<shim::ze::Queue>(ctx, dev);
    shim::IntrusivePtrAddRef(queue.get());
    *phCommandQueue = reinterpret_cast<ze_command_queue_handle_t>(queue.get());
  } catch (std::bad_alloc &e) {
    return ZE_RESULT_ERROR_OUT_OF_HOST_MEMORY;
  }

  return ZE_RESULT_SUCCESS;
}

ZE_APIEXPORT ze_result_t ZE_APICALL SHIM_CALL(zeCommandQueueDestroy)(
    ze_command_queue_handle_t hCommandQueue) {
  if (hCommandQueue == nullptr) {
    return ZE_RESULT_ERROR_INVALID_NULL_HANDLE;
  }

  shim::IntrusivePtr<shim::ze::Queue> queue(reinterpret_cast<shim::ze::Queue*>(hCommandQueue), false);

  return ZE_RESULT_SUCCESS;
}

ZE_APIEXPORT ze_result_t ZE_APICALL SHIM_CALL(zeCommandQueueExecuteCommandLists)(
    ze_command_queue_handle_t hCommandQueue,
    uint32_t numCommandLists,
    ze_command_list_handle_t *phCommandLists,
    ze_fence_handle_t hFence) {
  if (hCommandQueue == nullptr) {
    return ZE_RESULT_ERROR_INVALID_NULL_HANDLE;
  }

  if (phCommandLists == nullptr) {
    return ZE_RESULT_ERROR_INVALID_NULL_POINTER;
  }

  if (numCommandLists == 0) {
    return ZE_RESULT_ERROR_INVALID_SIZE;
  }

  shim::IntrusivePtr<shim::ze::Queue> queue(reinterpret_cast<shim::ze::Queue*>(hCommandQueue));

  for (size_t i = 0; i < numCommandLists; i++) {
    shim::IntrusivePtr<shim::ze::CommandList> cmdlist(reinterpret_cast<shim::ze::CommandList*>(phCommandLists[i]));

    for (const auto &cmd : *cmdlist) {
      if (auto r = queue->Execute(cmd); r != ZE_RESULT_SUCCESS) {
        return r;
      }
    }
  }

  return ZE_RESULT_SUCCESS;
}

ZE_APIEXPORT ze_result_t ZE_APICALL SHIM_CALL(zeCommandQueueSynchronize)(
    ze_command_queue_handle_t hCommandQueue,
    uint64_t timeout) {
  if (hCommandQueue == nullptr) {
    return ZE_RESULT_ERROR_INVALID_NULL_HANDLE;
  }

  shim::IntrusivePtr<shim::ze::Queue> queue(reinterpret_cast<shim::ze::Queue*>(hCommandQueue));

  if (queue->event_) {
    auto r = SHIM_CALL(zeEventHostSynchronize)(
        reinterpret_cast<ze_event_handle_t>(queue->event_.get()), timeout);

    if (r != ZE_RESULT_SUCCESS) {
      return r;
    }

    queue->event_ = nullptr;
  }

  return ZE_RESULT_SUCCESS;
}
