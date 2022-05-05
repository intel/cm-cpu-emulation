/*========================== begin_copyright_notice ============================

Copyright (C) 2022 Intel Corporation

SPDX-License-Identifier: MIT

============================= end_copyright_notice ===========================*/

#include "context.h"
#include "driver.h"

extern "C" {
SHIM_EXPORT(zeContextCreate);
SHIM_EXPORT(zeContextDestroy);
SHIM_EXPORT(zeContextGetStatus);
SHIM_EXPORT(zeContextSystemBarrier);
SHIM_EXPORT(zeContextMakeMemoryResident);
SHIM_EXPORT(zeContextEvictMemory);
SHIM_EXPORT(zeContextMakeImageResident);
SHIM_EXPORT(zeContextEvictImage);
} // extern "C"

ZE_APIEXPORT ze_result_t ZE_APICALL SHIM_CALL(zeContextCreate)(
    ze_driver_handle_t hDriver,
    const ze_context_desc_t *desc,
    ze_context_handle_t *phContext) {
  auto &driver = shim::ze::Driver::Instance();

  if (hDriver == nullptr) {
    return ZE_RESULT_ERROR_INVALID_NULL_HANDLE;
  }

  if (desc == nullptr || phContext == nullptr) {
    return ZE_RESULT_ERROR_INVALID_NULL_POINTER;
  }

  if (desc->flags > 1) {
    return ZE_RESULT_ERROR_INVALID_ENUMERATION;
  }

  ze_device_handle_t hdev = nullptr;
  uint32_t count = 1;

  if (SHIM_CALL(zeDeviceGet)(hDriver, &count, &hdev) != ZE_RESULT_SUCCESS) {
    return ZE_RESULT_ERROR_DEVICE_LOST;
  }

  shim::IntrusivePtr<CmDeviceEmu> dev(reinterpret_cast<CmDeviceEmu*>(hdev));

  try {
    auto ctx = shim::MakeIntrusive<shim::ze::Context>(dev);
    shim::IntrusivePtrAddRef(ctx.get());
    *phContext = reinterpret_cast<ze_context_handle_t>(ctx.get());
  } catch (std::bad_alloc &e) {
    return ZE_RESULT_ERROR_OUT_OF_HOST_MEMORY;
  }

  return ZE_RESULT_SUCCESS;
}

ZE_APIEXPORT ze_result_t ZE_APICALL SHIM_CALL(zeContextDestroy)(
    ze_context_handle_t hContext) {
  if (hContext == nullptr) {
    return ZE_RESULT_ERROR_INVALID_NULL_HANDLE;
  }

  shim::IntrusivePtr<shim::ze::Context> ctx(reinterpret_cast<shim::ze::Context*>(hContext), false);

  return ZE_RESULT_SUCCESS;
}

ZE_APIEXPORT ze_result_t ZE_APICALL SHIM_CALL(zeContextGetStatus)(
    ze_context_handle_t hContext) {
  if (hContext == nullptr) {
    return ZE_RESULT_ERROR_INVALID_NULL_HANDLE;
  }

  shim::IntrusivePtr<shim::ze::Context> ctx(reinterpret_cast<shim::ze::Context*>(hContext));

  return ZE_RESULT_SUCCESS;
}

ZE_APIEXPORT ze_result_t ZE_APICALL SHIM_CALL(zeContextSystemBarrier)(
    ze_context_handle_t hContext, ze_device_handle_t hDevice) {
  if (hContext == nullptr || hDevice == nullptr) {
    return ZE_RESULT_ERROR_INVALID_NULL_HANDLE;
  }

  return ZE_RESULT_SUCCESS;
}

ZE_APIEXPORT ze_result_t ZE_APICALL SHIM_CALL(zeContextMakeMemoryResident)(
    ze_context_handle_t hContext, ze_device_handle_t hDevice, void *ptr,
    size_t size) {
  if (hContext == nullptr || hDevice == nullptr) {
    return ZE_RESULT_ERROR_INVALID_NULL_HANDLE;
  }

  if (ptr == nullptr) {
    return ZE_RESULT_ERROR_INVALID_NULL_POINTER;
  }

  return ZE_RESULT_SUCCESS;
}

ZE_APIEXPORT ze_result_t ZE_APICALL SHIM_CALL(zeContextEvictMemory)(
    ze_context_handle_t hContext, ze_device_handle_t hDevice, void *ptr,
    size_t size) {
  if (hContext == nullptr || hDevice == nullptr) {
    return ZE_RESULT_ERROR_INVALID_NULL_HANDLE;
  }

  if (ptr == nullptr) {
    return ZE_RESULT_ERROR_INVALID_NULL_POINTER;
  }

  return ZE_RESULT_SUCCESS;
}

ZE_APIEXPORT ze_result_t ZE_APICALL SHIM_CALL(zeContextMakeImageResident)(
    ze_context_handle_t hContext, ze_device_handle_t hDevice,
    ze_image_handle_t hImage) {
  if (hContext == nullptr || hDevice == nullptr || hImage) {
    return ZE_RESULT_ERROR_INVALID_NULL_HANDLE;
  }

  return ZE_RESULT_SUCCESS;
}

ZE_APIEXPORT ze_result_t ZE_APICALL SHIM_CALL(zeContextEvictImage)(
    ze_context_handle_t hContext, ze_device_handle_t hDevice,
    ze_image_handle_t hImage) {
  if (hContext == nullptr || hDevice == nullptr || hImage) {
    return ZE_RESULT_ERROR_INVALID_NULL_HANDLE;
  }

  return ZE_RESULT_SUCCESS;
}
