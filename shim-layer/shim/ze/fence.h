/*========================== begin_copyright_notice ============================

Copyright (C) 2022 Intel Corporation

SPDX-License-Identifier: MIT

============================= end_copyright_notice ===========================*/

#ifndef CM_EMU_SHIM_ZE_FENCE_H
#define CM_EMU_SHIM_ZE_FENCE_H

#include "queue.h"

namespace shim {
namespace ze {
struct Fence : public IntrusiveRefCounter<Fence> {
  Fence(IntrusivePtr<Queue> queue, bool signalled) : queue_(queue), signalled_(signalled) {}

  IntrusivePtr<Queue> queue_;
  bool signalled_;
};
} // namespace ze
} // namespace shim

extern "C" {
ZE_APIEXPORT ze_result_t ZE_APICALL SHIM_CALL(zeFenceCreate)(
    ze_command_queue_handle_t hCommandQueue, const ze_fence_desc_t *desc,
    ze_fence_handle_t *phFence);
ZE_APIEXPORT ze_result_t ZE_APICALL SHIM_CALL(zeFenceDestroy)(
    ze_fence_handle_t hFence);
ZE_APIEXPORT ze_result_t ZE_APICALL SHIM_CALL(zeFenceHostSynchronize)(
    ze_fence_handle_t hFence, uint64_t timeout);
ZE_APIEXPORT ze_result_t ZE_APICALL SHIM_CALL(zeFenceQueryStatus)(
    ze_fence_handle_t hFence);
ZE_APIEXPORT ze_result_t ZE_APICALL SHIM_CALL(zeFenceReset)(
    ze_fence_handle_t hFence);
} // extern "C"
#endif // CM_EMU_SHIM_ZE_FENCE_H
