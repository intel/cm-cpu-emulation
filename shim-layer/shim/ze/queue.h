/*========================== begin_copyright_notice ============================

Copyright (C) 2022 Intel Corporation

SPDX-License-Identifier: MIT

============================= end_copyright_notice ===========================*/

#ifndef CM_EMU_SHIM_ZE_QUEUE_H
#define CM_EMU_SHIM_ZE_QUEUE_H

#include <memory>

#include "context.h"
#include "event.h"
#include "intrusive_pointer.h"

namespace shim {
namespace ze {
class Command;

class Queue : public IntrusiveRefCounter<Queue> {
private:
  struct Deleter {
    void operator()(CmQueueEmu *p) {
      if (p) {
        CmQueueEmu::Destroy(p);
      }
    }
  };

public:
  Queue(IntrusivePtr<Context> ctx, IntrusivePtr<CmDeviceEmu> dev)
      : ctx_(ctx), dev_(dev), queue_(nullptr, {}) {
    CmQueueEmu *q = nullptr;

    if (auto status = CmQueueEmu::Create(dev_.get(), q); status != CM_SUCCESS) {
      throw Error(::GetCmErrorString(status));
    }

    queue_.reset(q);
  }

  ze_result_t Execute(const Command &cmd);

  IntrusivePtr<Context> ctx_;
  IntrusivePtr<CmDeviceEmu> dev_;
  std::unique_ptr<CmQueueEmu, Deleter> queue_;
  IntrusivePtr<Event> event_;
};
} // namespace ze
} // namespace shim

extern "C" {
ZE_APIEXPORT ze_result_t ZE_APICALL SHIM_CALL(zeCommandQueueCreate)(
    ze_context_handle_t hContext, ze_device_handle_t hDevice,
    const ze_command_queue_desc_t *desc,
    ze_command_queue_handle_t *phCommandQueue);

ZE_APIEXPORT ze_result_t ZE_APICALL
    SHIM_CALL(zeCommandQueueDestroy)(ze_command_queue_handle_t hCommandQueue);

ZE_APIEXPORT ze_result_t ZE_APICALL SHIM_CALL(
    zeCommandQueueExecuteCommandLists)(ze_command_queue_handle_t hCommandQueue,
                                       uint32_t numCommandLists,
                                       ze_command_list_handle_t *phCommandLists,
                                       ze_fence_handle_t hFence);

ZE_APIEXPORT ze_result_t ZE_APICALL SHIM_CALL(zeCommandQueueSynchronize)(
    ze_command_queue_handle_t hCommandQueue, uint64_t timeout);
}
#endif // CM_EMU_SHIM_ZE_QUEUE_H
