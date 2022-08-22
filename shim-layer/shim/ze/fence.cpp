/*========================== begin_copyright_notice ============================

Copyright (C) 2022 Intel Corporation

SPDX-License-Identifier: MIT

============================= end_copyright_notice ===========================*/

#include "fence.h"

extern "C" {
SHIM_EXPORT(zeFenceCreate);
SHIM_EXPORT(zeFenceDestroy);
SHIM_EXPORT(zeFenceHostSynchronize);
SHIM_EXPORT(zeFenceQueryStatus);
SHIM_EXPORT(zeFenceReset);
} // extern "C"

ZE_APIEXPORT ze_result_t ZE_APICALL SHIM_CALL(zeFenceCreate)(
    ze_command_queue_handle_t hCommandQueue, const ze_fence_desc_t *desc,
    ze_fence_handle_t *phFence) {
  if (hCommandQueue == nullptr) {
    return ZE_RESULT_ERROR_INVALID_NULL_HANDLE;
  }

  if (desc == nullptr || phFence == nullptr) {
    return ZE_RESULT_ERROR_INVALID_NULL_POINTER;
  }

  if (desc->flags > 1) {
    return ZE_RESULT_ERROR_INVALID_ENUMERATION;
  }

  shim::IntrusivePtr<shim::ze::Queue> queue(
      reinterpret_cast<shim::ze::Queue *>(hCommandQueue));

  try {
    auto fence = shim::MakeIntrusive<shim::ze::Fence>(
        queue, desc->flags & ZE_FENCE_FLAG_SIGNALED);
    shim::IntrusivePtrAddRef(fence.get());
    *phFence = reinterpret_cast<ze_fence_handle_t>(fence.get());
  } catch (std::bad_alloc &e) {
    return ZE_RESULT_ERROR_OUT_OF_HOST_MEMORY;
  }

  return ZE_RESULT_SUCCESS;
}

ZE_APIEXPORT ze_result_t ZE_APICALL
SHIM_CALL(zeFenceDestroy)(ze_fence_handle_t hFence) {
  if (hFence == nullptr) {
    return ZE_RESULT_ERROR_INVALID_NULL_HANDLE;
  }

  shim::IntrusivePtr<shim::ze::Fence> fence(
      reinterpret_cast<shim::ze::Fence *>(hFence), false);

  return ZE_RESULT_SUCCESS;
}

ZE_APIEXPORT ze_result_t ZE_APICALL
SHIM_CALL(zeFenceHostSynchronize)(ze_fence_handle_t hFence, uint64_t timeout) {
  if (hFence == nullptr) {
    return ZE_RESULT_ERROR_INVALID_NULL_HANDLE;
  }

  shim::IntrusivePtr<shim::ze::Fence> fence =
      reinterpret_cast<shim::ze::Fence *>(hFence);

  if (fence->signalled_) {
    return ZE_RESULT_SUCCESS;
  }

  if (fence->queue_->event_) {
    auto e = reinterpret_cast<ze_event_handle_t>(fence->queue_->event_.get());
    auto r = SHIM_CALL(zeEventHostSynchronize)(e, timeout);

    if (r != ZE_RESULT_SUCCESS) {
      return r;
    }
  }

  fence->signalled_ = true;
  return ZE_RESULT_SUCCESS;
}

ZE_APIEXPORT ze_result_t ZE_APICALL
SHIM_CALL(zeFenceQueryStatus)(ze_fence_handle_t hFence) {
  if (hFence == nullptr) {
    return ZE_RESULT_ERROR_INVALID_NULL_HANDLE;
  }

  shim::IntrusivePtr<shim::ze::Fence> fence =
      reinterpret_cast<shim::ze::Fence *>(hFence);

  if (fence->signalled_) {
    return ZE_RESULT_SUCCESS;
  }

  if (fence->queue_->event_) {
    auto e = reinterpret_cast<ze_event_handle_t>(fence->queue_->event_.get());
    auto r = SHIM_CALL(zeEventQueryStatus)(e);

    if (r != ZE_RESULT_SUCCESS) {
      return r;
    }
  }

  fence->signalled_ = true;
  return ZE_RESULT_SUCCESS;
}

ZE_APIEXPORT ze_result_t ZE_APICALL
SHIM_CALL(zeFenceReset)(ze_fence_handle_t hFence) {
  if (hFence == nullptr) {
    return ZE_RESULT_ERROR_INVALID_NULL_HANDLE;
  }

  shim::IntrusivePtr<shim::ze::Fence> fence =
      reinterpret_cast<shim::ze::Fence *>(hFence);

  fence->signalled_ = false;
  return ZE_RESULT_SUCCESS;
}
