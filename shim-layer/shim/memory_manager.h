/*========================== begin_copyright_notice ============================

Copyright (C) 2022 Intel Corporation

SPDX-License-Identifier: MIT

============================= end_copyright_notice ===========================*/

#ifndef CM_EMU_SHIM_MEMORY_MANAGER_H
#define CM_EMU_SHIM_MEMORY_MANAGER_H

#include <functional>
#include <mutex>
#include <unordered_map>

#include "shim.h"

#include "intrusive_pointer.h"

namespace shim {
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

  using BufferPtrT =
      std::unique_ptr<CmBufferUP, std::function<void(CmBufferUP *)>>;

  struct BufferAllocT {
    BufferPtrT buffer_;
    size_t size_;
    size_t alignment_;

    BufferAllocT(BufferPtrT &&buffer, size_t size, size_t alignment)
        : buffer_(std::move(buffer)), size_(size), alignment_(alignment) {}
  };

  std::unordered_map<void *, BufferAllocT> buffers_;
};
} // namespace shim

#endif // CM_EMU_SHIM_MEMORY_MANAGER_H
