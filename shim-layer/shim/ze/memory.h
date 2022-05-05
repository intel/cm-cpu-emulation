/*========================== begin_copyright_notice ============================

Copyright (C) 2022 Intel Corporation

SPDX-License-Identifier: MIT

============================= end_copyright_notice ===========================*/

#ifndef CM_EMU_SHIM_ZE_MEMORY_H
#define CM_EMU_SHIM_ZE_MEMORY_H

#include <functional>
#include <mutex>
#include <unordered_map>

#include "ze.h"
#include "device.h"
#include "intrusive_pointer.h"

namespace shim {
namespace ze {
class MemoryManager {
 public:
  MemoryManager() = default;

  void *Alloc(IntrusivePtr<CmDeviceEmu> dev, size_t size, size_t alignment);
  void Free(void *ptr);

  SurfaceIndex *GetIndex(void *ptr);

  auto begin() { return buffers_.begin(); }
  auto end() { return buffers_.end(); }

 private:
  std::mutex mutex_;

  using BufferPtrT = std::unique_ptr<CmBufferUP, std::function<void(CmBufferUP*)>>;

  struct BufferAllocT {
    BufferPtrT buffer_;
    size_t size_;
    size_t alignment_;

    BufferAllocT(BufferPtrT &&buffer, size_t size, size_t alignment)
        : buffer_(std::move(buffer)), size_(size), alignment_(alignment) {}
  };

  std::unordered_map<void *, BufferAllocT> buffers_;
};

} // namespace ze
} // namespace shim

extern "C" {
ZE_APIEXPORT ze_result_t ZE_APICALL SHIM_CALL(zeMemAllocShared)(
    ze_context_handle_t hContext, const ze_device_mem_alloc_desc_t *device_desc,
    const ze_host_mem_alloc_desc_t *host_desc, size_t size, size_t alignment,
    ze_device_handle_t hDevice, void **pptr);
ZE_APIEXPORT ze_result_t ZE_APICALL SHIM_CALL(zeMemAllocDevice)(
    ze_context_handle_t hContext, const ze_device_mem_alloc_desc_t *device_desc,
    size_t size, size_t alignment, ze_device_handle_t hDevice, void **pptr);
ZE_APIEXPORT ze_result_t ZE_APICALL SHIM_CALL(zeMemAllocHost)(
    ze_context_handle_t hContext, const ze_host_mem_alloc_desc_t *host_desc,
    size_t size, size_t alignment, void **pptr);
ZE_APIEXPORT ze_result_t ZE_APICALL SHIM_CALL(zeMemFree)(
    ze_context_handle_t hContext, void *ptr);
ZE_APIEXPORT ze_result_t ZE_APICALL SHIM_CALL(zeMemGetAllocProperties)(
    ze_context_handle_t hContext, const void *ptr,
    ze_memory_allocation_properties_t *pMemAllocProperties,
    ze_device_handle_t *phDevice);
ZE_APIEXPORT ze_result_t ZE_APICALL SHIM_CALL(zeMemGetAddressRange)(
    ze_context_handle_t hContext, const void *ptr, void **pBase, size_t *pSize);
ZE_APIEXPORT ze_result_t ZE_APICALL SHIM_CALL(zeMemGetIpcHandle)(
    ze_context_handle_t hContext, const void *ptr, ze_ipc_mem_handle_t *pIpcHandle);
ZE_APIEXPORT ze_result_t ZE_APICALL SHIM_CALL(zeMemOpenIpcHandle)(
    ze_context_handle_t hContext, ze_device_handle_t hDevice,
    ze_ipc_mem_handle_t handle, ze_ipc_memory_flags_t flags, void **pptr);
ZE_APIEXPORT ze_result_t ZE_APICALL SHIM_CALL(zeMemCloseIpcHandle)(
    ze_context_handle_t hContext, const void *ptr);
}
#endif // CM_EMU_SHIM_ZE_MEMORY_H
