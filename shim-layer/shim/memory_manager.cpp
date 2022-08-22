/*========================== begin_copyright_notice ============================

Copyright (C) 2022 Intel Corporation

SPDX-License-Identifier: MIT

============================= end_copyright_notice ===========================*/

#include "memory_manager.h"

#include <algorithm>

void *shim::MemoryManager::Alloc(IntrusivePtr<CmDeviceEmu> dev, size_t size,
                                 size_t alignment) {
  constexpr size_t minimal_alingment = 64;
  try {
    const std::lock_guard<std::mutex> lock(mutex_);
    const auto align =
        static_cast<std::align_val_t>(std::max(alignment, minimal_alingment));

    std::unique_ptr<void, std::function<void(void *)>> ptr(
        ::operator new(size, align),
        [this, size, align](void *p) { ::operator delete(p, size, align); });

    CmBufferUP *buffer = nullptr;
    if (auto r = dev->CreateBufferUP(size, ptr.get(), buffer);
        r != CM_SUCCESS) {
      return nullptr;
    }

    BufferAllocT buf(
        BufferPtrT(buffer, [dev](CmBufferUP *p) { dev->DestroyBufferUP(p); }),
        size, alignment);
    buffers_.emplace(ptr.get(), std::move(buf));

    return ptr.release();
  } catch (std::bad_alloc &e) {
    return nullptr;
  }
}

void shim::MemoryManager::Free(void *ptr) {
  const std::lock_guard<std::mutex> lock(mutex_);

  auto it = buffers_.find(ptr);
  if (it == std::end(buffers_)) {
    return;
  }

  const auto align = static_cast<std::align_val_t>(it->second.alignment_);

  ::operator delete(ptr, it->second.size_, align);
  buffers_.erase(it);
}

SurfaceIndex *shim::MemoryManager::GetIndex(void *ptr) {
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
