/*========================== begin_copyright_notice ============================

Copyright (C) 2022 Intel Corporation

SPDX-License-Identifier: MIT

============================= end_copyright_notice ===========================*/

#ifndef CM_EMU_SHIM_ZE_CONTEXT_H
#define CM_EMU_SHIM_ZE_CONTEXT_H

#include "device.h"
#include "memory.h"

#include "intrusive_pointer.h"

namespace shim {
namespace ze {
class Context : public IntrusiveRefCounter<Context> {
 public:
  Context(IntrusivePtr<CmDeviceEmu> dev) : dev_(dev) {}
  Context(const Context &) = delete;
  Context(Context &&) = delete;
  Context &operator =(const Context &) = delete;
  Context &operator =(Context &&) = delete;
  ~Context() = default;

  IntrusivePtr<CmDeviceEmu> dev_;
  MemoryManager mm_;
};
} // namespace ze
} // namespace shim

extern "C" {
ZE_APIEXPORT ze_result_t ZE_APICALL SHIM_CALL(zeContextCreate)(
    ze_driver_handle_t hDriver,
    const ze_context_desc_t *desc,
    ze_context_handle_t *phContext);
ZE_APIEXPORT ze_result_t ZE_APICALL SHIM_CALL(zeContextDestroy)(
    ze_context_handle_t hContext);
ZE_APIEXPORT ze_result_t ZE_APICALL SHIM_CALL(zeContextGetStatus)(
    ze_context_handle_t hContext);
ZE_APIEXPORT ze_result_t ZE_APICALL SHIM_CALL(zeContextSystemBarrier)(
    ze_context_handle_t hContext, ze_device_handle_t hDevice);
ZE_APIEXPORT ze_result_t ZE_APICALL SHIM_CALL(zeContextMakeMemoryResident)(
    ze_context_handle_t hContext, ze_device_handle_t hDevice, void *ptr,
    size_t size);
ZE_APIEXPORT ze_result_t ZE_APICALL SHIM_CALL(zeContextEvictMemory)(
    ze_context_handle_t hContext, ze_device_handle_t hDevice, void *ptr,
    size_t size);
ZE_APIEXPORT ze_result_t ZE_APICALL SHIM_CALL(zeContextMakeImageResident)(
    ze_context_handle_t hContext, ze_device_handle_t hDevice,
    ze_image_handle_t hImage);
ZE_APIEXPORT ze_result_t ZE_APICALL SHIM_CALL(zeContextEvictImage)(
    ze_context_handle_t hContext, ze_device_handle_t hDevice,
    ze_image_handle_t hImage);
} // extern "C"
#endif // CM_EMU_SHIM_ZE_CONTEXT_H
