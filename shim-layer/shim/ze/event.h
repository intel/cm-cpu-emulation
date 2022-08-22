/*========================== begin_copyright_notice ============================

Copyright (C) 2022 Intel Corporation

SPDX-License-Identifier: MIT

============================= end_copyright_notice ===========================*/

#ifndef CM_EMU_SHIM_ZE_EVENT_H
#define CM_EMU_SHIM_ZE_EVENT_H

#include <vector>

#include "ze.h"

#include "context.h"

#include "intrusive_pointer.h"

namespace shim {
namespace ze {

struct Event;

class EventPool : public IntrusiveRefCounter<EventPool> {
public:
  EventPool(IntrusivePtr<Context> ctx, size_t count)
      : ctx_(ctx), pool_(count, nullptr) {}

  IntrusivePtr<Context> ctx_;
  std::vector<IntrusivePtr<Event>> pool_;
};

struct Event : public IntrusiveRefCounter<Event> {
  Event(IntrusivePtr<EventPool> pool, size_t index)
      : pool_(pool), index_(index) {}

  size_t index_;
  IntrusivePtr<EventPool> pool_;

  IntrusivePtr<CmEventEmu> event_ = nullptr;

  bool signalled_ = false;
};

} // namespace ze
} // namespace shim

extern "C" {
ZE_APIEXPORT ze_result_t ZE_APICALL SHIM_CALL(zeEventPoolCreate)(
    ze_context_handle_t hContext, const ze_event_pool_desc_t *desc,
    uint32_t numDevices, ze_device_handle_t *phDevices,
    ze_event_pool_handle_t *phEventPool);
ZE_APIEXPORT ze_result_t ZE_APICALL
    SHIM_CALL(zeEventPoolDestroy)(ze_event_pool_handle_t hEventPool);
ZE_APIEXPORT ze_result_t ZE_APICALL SHIM_CALL(zeEventCreate)(
    ze_event_pool_handle_t hEventPool, const ze_event_desc_t *desc,
    ze_event_handle_t *phEvent);
ZE_APIEXPORT
    ze_result_t ZE_APICALL SHIM_CALL(zeEventDestroy)(ze_event_handle_t hEvent);
ZE_APIEXPORT ze_result_t ZE_APICALL SHIM_CALL(zeEventPoolGetIpcHandle)(
    ze_event_pool_handle_t hEventPool, ze_ipc_event_pool_handle_t *phIpc);
ZE_APIEXPORT ze_result_t ZE_APICALL SHIM_CALL(zeEventPoolOpenIpcHandle)(
    ze_context_handle_t hContext, ze_ipc_event_pool_handle_t hIpc,
    ze_event_pool_handle_t *phEventPool);
ZE_APIEXPORT ze_result_t ZE_APICALL
    SHIM_CALL(zeEventPoolCloseIpcHandle)(ze_event_pool_handle_t hEventPool);
ZE_APIEXPORT ze_result_t ZE_APICALL
    SHIM_CALL(zeEventHostSignal)(ze_event_handle_t hEvent);
ZE_APIEXPORT ze_result_t ZE_APICALL SHIM_CALL(zeEventHostSynchronize)(
    ze_event_handle_t hEvent, uint64_t timeout);
ZE_APIEXPORT ze_result_t ZE_APICALL
    SHIM_CALL(zeEventQueryStatus)(ze_event_handle_t hEvent);
ZE_APIEXPORT ze_result_t ZE_APICALL
    SHIM_CALL(zeEventHostReset)(ze_event_handle_t hEvent);
ZE_APIEXPORT ze_result_t ZE_APICALL SHIM_CALL(zeEventQueryKernelTimestamp)(
    ze_event_handle_t hEvent, ze_kernel_timestamp_result_t *dstptr);
} // extern "C"

#endif // CM_EMU_SHIM_ZE_EVENT_H
