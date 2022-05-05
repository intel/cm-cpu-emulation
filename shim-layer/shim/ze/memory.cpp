/*========================== begin_copyright_notice ============================

Copyright (C) 2022 Intel Corporation

SPDX-License-Identifier: MIT

============================= end_copyright_notice ===========================*/

#include <new>

#include "memory.h"
#include "context.h"

void *shim::ze::MemoryManager::Alloc(IntrusivePtr<CmDeviceEmu> dev, size_t size, size_t alignment) {
  try {
    const std::lock_guard<std::mutex> lock(mutex_);
    const auto align = static_cast<std::align_val_t>(alignment);

    std::unique_ptr<void, std::function<void(void*)>> ptr(::operator new(size, align),
                                                          [this, size, align](void *p) {
                                                            ::operator delete(p, size, align);
                                                          });

    CmBufferUP *buffer = nullptr;
    if (auto r = dev->CreateBufferUP(size, ptr.get(), buffer); r != CM_SUCCESS) {
      return nullptr;
    }

    BufferAllocT buf(BufferPtrT(buffer, [dev](CmBufferUP *p) { dev->DestroyBufferUP(p); }),
                     size, alignment);
    buffers_.emplace(ptr.get(), std::move(buf));

    return ptr.release();
  } catch (std::bad_alloc &e) {
    return nullptr;
  }
}

void shim::ze::MemoryManager::Free(void *ptr) {
  const std::lock_guard<std::mutex> lock(mutex_);

  auto it = buffers_.find(ptr);
  if (it == std::end(buffers_)) {
    return;
  }

  const auto align = static_cast<std::align_val_t>(it->second.alignment_);

  ::operator delete(ptr, it->second.size_, align);
  buffers_.erase(it);
}

SurfaceIndex *shim::ze::MemoryManager::GetIndex(void *ptr) {
  const std::lock_guard<std::mutex> lock(mutex_);
  auto it = buffers_.find(ptr);

  if (it == std::end(buffers_)) {
    return nullptr;
  }

  SurfaceIndex *index = nullptr;
  auto r = it->second.buffer_->GetIndex(index);

  if (r != CM_SUCCESS) {
    return nullptr;
  }

  return index;
}

extern "C" {
SHIM_EXPORT(zeMemAllocShared);
SHIM_EXPORT(zeMemAllocDevice);
SHIM_EXPORT(zeMemAllocHost);
SHIM_EXPORT(zeMemFree);
SHIM_EXPORT(zeMemGetAllocProperties);
SHIM_EXPORT(zeMemGetAddressRange);
SHIM_EXPORT(zeMemGetIpcHandle);
SHIM_EXPORT(zeMemOpenIpcHandle);
SHIM_EXPORT(zeMemCloseIpcHandle);

ZE_APIEXPORT ze_result_t ZE_APICALL SHIM_CALL(zeMemAllocShared)(
    ze_context_handle_t hContext, const ze_device_mem_alloc_desc_t *device_desc,
    const ze_host_mem_alloc_desc_t *host_desc, size_t size, size_t alignment,
    ze_device_handle_t hDevice, void **pptr) {
  if (hContext == nullptr) {
    return ZE_RESULT_ERROR_INVALID_NULL_HANDLE;
  }

  if (device_desc == nullptr || host_desc == nullptr || pptr == nullptr) {
    return ZE_RESULT_ERROR_INVALID_NULL_POINTER;
  }

  if (device_desc->flags > 0x3 || host_desc->flags > 0x7) {
    return ZE_RESULT_ERROR_INVALID_ENUMERATION;
  }

  if (size == 0) {
    return ZE_RESULT_ERROR_UNSUPPORTED_SIZE;
  }

  if ((alignment & (alignment - 1)) != 0) {
    return ZE_RESULT_ERROR_UNSUPPORTED_ALIGNMENT;
  }

  shim::IntrusivePtr<shim::ze::Context> ctx(reinterpret_cast<shim::ze::Context*>(hContext));
  shim::IntrusivePtr<CmDeviceEmu> dev(reinterpret_cast<CmDeviceEmu*>(hDevice));

  if (!dev) {
    dev = ctx->dev_;
  }

  auto &mm = ctx->mm_;
  *pptr = mm.Alloc(dev, size, alignment);

  if (*pptr == nullptr) {
    return ZE_RESULT_ERROR_OUT_OF_HOST_MEMORY;
  }

  return ZE_RESULT_SUCCESS;
}

ZE_APIEXPORT ze_result_t ZE_APICALL SHIM_CALL(zeMemAllocDevice)(
    ze_context_handle_t hContext, const ze_device_mem_alloc_desc_t *device_desc,
    size_t size, size_t alignment, ze_device_handle_t hDevice, void **pptr) {
  if (hDevice == nullptr) {
    return ZE_RESULT_ERROR_INVALID_NULL_HANDLE;
  }

  ze_host_mem_alloc_desc_t host_desc = {
    ZE_STRUCTURE_TYPE_HOST_MEM_ALLOC_DESC,
    nullptr, 0
  };

  return SHIM_CALL(zeMemAllocShared)(hContext, device_desc, &host_desc, size,
                                     alignment, hDevice, pptr);
}

ZE_APIEXPORT ze_result_t ZE_APICALL SHIM_CALL(zeMemAllocHost)(
    ze_context_handle_t hContext, const ze_host_mem_alloc_desc_t *host_desc,
    size_t size, size_t alignment, void **pptr) {
  ze_device_mem_alloc_desc_t device_desc = {
    ZE_STRUCTURE_TYPE_HOST_MEM_ALLOC_DESC,
    nullptr, 0, 0
  };

  return SHIM_CALL(zeMemAllocShared)(hContext, &device_desc, host_desc, size,
                                     alignment, nullptr, pptr);
}

ZE_APIEXPORT ze_result_t ZE_APICALL SHIM_CALL(zeMemFree)(
    ze_context_handle_t hContext, void *ptr) {
  if (hContext == nullptr) {
    return ZE_RESULT_ERROR_INVALID_NULL_HANDLE;
  }

  if (ptr == nullptr) {
    return ZE_RESULT_ERROR_INVALID_NULL_POINTER;
  }

  shim::IntrusivePtr<shim::ze::Context> ctx(reinterpret_cast<shim::ze::Context*>(hContext));

  auto &mm = ctx->mm_;
  mm.Free(ptr);

  return ZE_RESULT_SUCCESS;
}

ZE_APIEXPORT ze_result_t ZE_APICALL SHIM_CALL(zeMemGetAllocProperties)(
    ze_context_handle_t hContext, const void *ptr,
    ze_memory_allocation_properties_t *pMemAllocProperties,
    ze_device_handle_t *phDevice) {
  if (hContext == nullptr) {
    return ZE_RESULT_ERROR_INVALID_NULL_HANDLE;
  }

  if (ptr == nullptr || pMemAllocProperties == nullptr) {
    return ZE_RESULT_ERROR_INVALID_NULL_POINTER;
  }

  shim::IntrusivePtr<shim::ze::Context> ctx(reinterpret_cast<shim::ze::Context*>(hContext));
  auto &mm = ctx->mm_;

  SurfaceIndex *surf = mm.GetIndex(const_cast<void*>(ptr));

  if (surf == nullptr) {
    pMemAllocProperties->type = ZE_MEMORY_TYPE_UNKNOWN;
  } else {
    pMemAllocProperties->type = ZE_MEMORY_TYPE_SHARED;
    pMemAllocProperties->id = reinterpret_cast<uint64_t>(surf);
    pMemAllocProperties->pageSize = 0x1000;

    if (phDevice != nullptr) {
      *phDevice = reinterpret_cast<ze_device_handle_t>(ctx->dev_.get());
    }
  }

  return ZE_RESULT_SUCCESS;
}

ZE_APIEXPORT ze_result_t ZE_APICALL SHIM_CALL(zeMemGetAddressRange)(
    ze_context_handle_t hContext, const void *ptr, void **pBase, size_t *pSize) {
  if (hContext == nullptr) {
    return ZE_RESULT_ERROR_INVALID_NULL_HANDLE;
  }

  if (ptr == nullptr) {
    return ZE_RESULT_ERROR_INVALID_NULL_POINTER;
  }

  shim::IntrusivePtr<shim::ze::Context> ctx(reinterpret_cast<shim::ze::Context*>(hContext));
  auto &mm = ctx->mm_;

  auto it = std::find_if(std::begin(mm), std::end(mm),
                         [&](const auto &node) {
                           uintptr_t addr = reinterpret_cast<uintptr_t>(ptr);
                           uintptr_t base = reinterpret_cast<uintptr_t>(node.first);
                           uintptr_t limit = base + node.second.size_;
                           return (base <= addr) && (addr < limit);
                         });

  void *base = nullptr;
  size_t size = 0;

  if (it != std::end(mm)) {
    base = it->first;
    size = it->second.size_;
  }

  if (pBase != nullptr) {
    *pBase = base;
  }

  if (pSize != nullptr) {
    *pSize = size;
  }

  return ZE_RESULT_SUCCESS;
}

ZE_APIEXPORT ze_result_t ZE_APICALL SHIM_CALL(zeMemGetIpcHandle)(
    ze_context_handle_t hContext, const void *ptr, ze_ipc_mem_handle_t *pIpcHandle) {
  if (hContext == nullptr) {
    return ZE_RESULT_ERROR_INVALID_NULL_HANDLE;
  }

  if (ptr == nullptr || pIpcHandle == nullptr) {
    return ZE_RESULT_ERROR_INVALID_NULL_POINTER;
  }

  return ZE_RESULT_ERROR_UNSUPPORTED_FEATURE;
}

ZE_APIEXPORT ze_result_t ZE_APICALL SHIM_CALL(zeMemOpenIpcHandle)(
    ze_context_handle_t hContext, ze_device_handle_t hDevice,
    ze_ipc_mem_handle_t handle, ze_ipc_memory_flags_t flags, void **pptr) {
  if (hContext == nullptr || hDevice == nullptr) {
    return ZE_RESULT_ERROR_INVALID_NULL_HANDLE;
  }

  if (pptr == nullptr) {
    return ZE_RESULT_ERROR_INVALID_NULL_POINTER;
  }

  if (flags > 0x1) {
    return ZE_RESULT_ERROR_INVALID_ENUMERATION;
  }

  return ZE_RESULT_ERROR_UNSUPPORTED_FEATURE;
}

ZE_APIEXPORT ze_result_t ZE_APICALL SHIM_CALL(zeMemCloseIpcHandle)(
    ze_context_handle_t hContext, const void *ptr) {
  if (hContext == nullptr) {
    return ZE_RESULT_ERROR_INVALID_NULL_HANDLE;
  }

  if (ptr == nullptr) {
    return ZE_RESULT_ERROR_INVALID_NULL_POINTER;
  }

  return ZE_RESULT_ERROR_UNSUPPORTED_FEATURE;
}
} // extern "C"
