/*========================== begin_copyright_notice ============================

Copyright (C) 2022 Intel Corporation

SPDX-License-Identifier: MIT

============================= end_copyright_notice ===========================*/

#include "event.h"

extern "C" {
SHIM_EXPORT(zeEventPoolCreate);
SHIM_EXPORT(zeEventPoolDestroy);
SHIM_EXPORT(zeEventCreate);
SHIM_EXPORT(zeEventDestroy);
SHIM_EXPORT(zeEventPoolGetIpcHandle);
SHIM_EXPORT(zeEventPoolOpenIpcHandle);
SHIM_EXPORT(zeEventPoolCloseIpcHandle);
SHIM_EXPORT(zeEventHostSignal);
SHIM_EXPORT(zeEventHostSynchronize);
SHIM_EXPORT(zeEventQueryStatus);
SHIM_EXPORT(zeEventHostReset);
SHIM_EXPORT(zeEventQueryKernelTimestamp);
} // extern "C"

ZE_APIEXPORT ze_result_t ZE_APICALL SHIM_CALL(zeEventPoolCreate)(
    ze_context_handle_t hContext, const ze_event_pool_desc_t *desc,
    uint32_t numDevices, ze_device_handle_t *phDevices,
    ze_event_pool_handle_t *phEventPool) {
  if (hContext == nullptr) {
    return ZE_RESULT_ERROR_INVALID_NULL_HANDLE;
  }

  if (desc == nullptr || phEventPool == nullptr) {
    return ZE_RESULT_ERROR_INVALID_NULL_POINTER;
  }

  if (desc->flags > 0x7) {
    return ZE_RESULT_ERROR_INVALID_ENUMERATION;
  }

  if (desc->count == 0 || (numDevices > 0 && phDevices == nullptr)) {
    return ZE_RESULT_ERROR_INVALID_SIZE;
  }

  shim::IntrusivePtr<shim::ze::Context> ctx =
      reinterpret_cast<shim::ze::Context *>(hContext);

  try {
    auto pool = shim::MakeIntrusive<shim::ze::EventPool>(ctx, desc->count);

    shim::IntrusivePtrAddRef(pool.get());
    *phEventPool = reinterpret_cast<ze_event_pool_handle_t>(pool.get());
  } catch (std::bad_alloc &e) {
    return ZE_RESULT_ERROR_OUT_OF_HOST_MEMORY;
  }

  return ZE_RESULT_SUCCESS;
}

ZE_APIEXPORT ze_result_t ZE_APICALL
SHIM_CALL(zeEventPoolDestroy)(ze_event_pool_handle_t hEventPool) {
  if (hEventPool == nullptr) {
    return ZE_RESULT_ERROR_INVALID_NULL_HANDLE;
  }

  shim::IntrusivePtr<shim::ze::EventPool> pool(
      reinterpret_cast<shim::ze::EventPool *>(hEventPool), false);

  return ZE_RESULT_SUCCESS;
}

ZE_APIEXPORT ze_result_t ZE_APICALL SHIM_CALL(zeEventCreate)(
    ze_event_pool_handle_t hEventPool, const ze_event_desc_t *desc,
    ze_event_handle_t *phEvent) {
  if (hEventPool == nullptr) {
    return ZE_RESULT_ERROR_INVALID_NULL_HANDLE;
  }

  if (desc == nullptr || phEvent == nullptr) {
    return ZE_RESULT_ERROR_INVALID_NULL_POINTER;
  }

  if (desc->signal > 0x7 || desc->wait > 0x7) {
    return ZE_RESULT_ERROR_INVALID_ENUMERATION;
  }

  shim::IntrusivePtr<shim::ze::EventPool> pool(
      reinterpret_cast<shim::ze::EventPool *>(hEventPool));

  if (desc->index >= pool->pool_.size()) {
    return ZE_RESULT_ERROR_INVALID_ARGUMENT;
  }

  try {
    auto &event = pool->pool_[desc->index];

    if (!event) {
      event = shim::MakeIntrusive<shim::ze::Event>(pool, desc->index);
    }

    shim::IntrusivePtrAddRef(event.get());
    *phEvent = reinterpret_cast<ze_event_handle_t>(event.get());
  } catch (std::bad_alloc &e) {
    return ZE_RESULT_ERROR_OUT_OF_HOST_MEMORY;
  }

  return ZE_RESULT_SUCCESS;
}

ZE_APIEXPORT ze_result_t ZE_APICALL
SHIM_CALL(zeEventDestroy)(ze_event_handle_t hEvent) {
  if (hEvent == nullptr) {
    return ZE_RESULT_ERROR_INVALID_NULL_HANDLE;
  }

  shim::IntrusivePtr<shim::ze::Event> event(
      reinterpret_cast<shim::ze::Event *>(hEvent), false);

  // Event is used only by pool and current thread
  if (event->UseCount() == 2) {
    event->pool_->pool_[event->index_] = nullptr;
  }

  return ZE_RESULT_SUCCESS;
}

ZE_APIEXPORT ze_result_t ZE_APICALL SHIM_CALL(zeEventPoolGetIpcHandle)(
    ze_event_pool_handle_t hEventPool, ze_ipc_event_pool_handle_t *phIpc) {
  return ZE_RESULT_ERROR_UNSUPPORTED_FEATURE;
}

ZE_APIEXPORT ze_result_t ZE_APICALL SHIM_CALL(zeEventPoolOpenIpcHandle)(
    ze_context_handle_t hContext, ze_ipc_event_pool_handle_t hIpc,
    ze_event_pool_handle_t *phEventPool) {
  return ZE_RESULT_ERROR_UNSUPPORTED_FEATURE;
}

ZE_APIEXPORT ze_result_t ZE_APICALL
SHIM_CALL(zeEventPoolCloseIpcHandle)(ze_event_pool_handle_t hEventPool) {
  return ZE_RESULT_ERROR_UNSUPPORTED_FEATURE;
}

ZE_APIEXPORT ze_result_t ZE_APICALL
SHIM_CALL(zeEventHostSignal)(ze_event_handle_t hEvent) {
  if (hEvent == nullptr) {
    return ZE_RESULT_ERROR_INVALID_NULL_HANDLE;
  }

  shim::IntrusivePtr<shim::ze::Event> event(
      reinterpret_cast<shim::ze::Event *>(hEvent));
  event->signalled_ = true;

  return ZE_RESULT_SUCCESS;
}

ZE_APIEXPORT ze_result_t ZE_APICALL
SHIM_CALL(zeEventHostSynchronize)(ze_event_handle_t hEvent, uint64_t timeout) {
  if (hEvent == nullptr) {
    return ZE_RESULT_ERROR_INVALID_NULL_HANDLE;
  }

  shim::IntrusivePtr<shim::ze::Event> event(
      reinterpret_cast<shim::ze::Event *>(hEvent));

  if (event->signalled_) {
    return ZE_RESULT_SUCCESS;
  } else if (event->event_) {
    uint64_t timeout_ms = timeout / 1000000 + 1;
    if (timeout == std::numeric_limits<uint64_t>::max() ||
        timeout_ms > std::numeric_limits<uint32_t>::max()) {
      timeout_ms = std::numeric_limits<uint32_t>::max();
    }
    event->event_->WaitForTaskFinished(timeout_ms);

    CM_STATUS status;
    if (auto r = event->event_->GetStatus(status); r != CM_SUCCESS) {
      return ZE_RESULT_ERROR_DEVICE_LOST;
    }

    if (status == CM_STATUS_FINISHED) {
      event->signalled_ = true;
      return ZE_RESULT_SUCCESS;
    }
  }

  return ZE_RESULT_NOT_READY;
}

ZE_APIEXPORT ze_result_t ZE_APICALL
SHIM_CALL(zeEventQueryStatus)(ze_event_handle_t hEvent) {
  if (hEvent == nullptr) {
    return ZE_RESULT_ERROR_INVALID_NULL_HANDLE;
  }

  shim::IntrusivePtr<shim::ze::Event> event(
      reinterpret_cast<shim::ze::Event *>(hEvent));

  if (event->signalled_) {
    return ZE_RESULT_SUCCESS;
  } else if (event->event_) {
    CM_STATUS status;
    if (auto r = event->event_->GetStatus(status); r != CM_SUCCESS) {
      return ZE_RESULT_ERROR_DEVICE_LOST;
    }

    if (status == CM_STATUS_FINISHED) {
      return ZE_RESULT_SUCCESS;
    }
  }

  return ZE_RESULT_NOT_READY;
}

ZE_APIEXPORT ze_result_t ZE_APICALL
SHIM_CALL(zeEventHostReset)(ze_event_handle_t hEvent) {
  if (hEvent == nullptr) {
    return ZE_RESULT_ERROR_INVALID_NULL_HANDLE;
  }

  shim::IntrusivePtr<shim::ze::Event> event(
      reinterpret_cast<shim::ze::Event *>(hEvent));
  event->signalled_ = false;
  event->event_ = nullptr;

  return ZE_RESULT_SUCCESS;
}

ZE_APIEXPORT ze_result_t ZE_APICALL SHIM_CALL(zeEventQueryKernelTimestamp)(
    ze_event_handle_t hEvent, ze_kernel_timestamp_result_t *dstptr) {
  if (dstptr == nullptr) {
    return ZE_RESULT_ERROR_INVALID_NULL_POINTER;
  }

  if (auto r = SHIM_CALL(zeEventQueryStatus)(hEvent)) {
    return r;
  }
  dstptr->global.kernelStart = 0;
  dstptr->global.kernelEnd = 100;
  dstptr->context.kernelStart = 0;
  dstptr->context.kernelEnd = 100;

  return ZE_RESULT_SUCCESS;
}
