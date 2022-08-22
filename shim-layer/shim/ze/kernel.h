/*========================== begin_copyright_notice ============================

Copyright (C) 2022 Intel Corporation

SPDX-License-Identifier: MIT

============================= end_copyright_notice ===========================*/

#ifndef CM_EMU_SHIM_ZE_KERNEL_H
#define CM_EMU_SHIM_ZE_KERNEL_H

#include <unordered_map>
#include <vector>

#include "intrusive_pointer.h"

#include "context.h"
#include "image.h"

namespace shim {
namespace ze {
struct Kernel : public IntrusiveRefCounter<Kernel> {
  struct GroupSize {
    uint32_t groupSizeX;
    uint32_t groupSizeY;
    uint32_t groupSizeZ;
  };

  Kernel(IntrusivePtr<Context> ctx, IntrusivePtr<CmKernelEmu> kernel)
      : ctx_(ctx), kernel_(kernel) {}

  IntrusivePtr<Context> ctx_;
  IntrusivePtr<CmKernelEmu> kernel_;

  std::vector<IntrusivePtr<Image>> images_;

  ze_kernel_indirect_access_flags_t indirect_access_flags_ = 0;

  static thread_local std::unordered_map<CmKernelEmu *, GroupSize> group_size_;
};
} // namespace ze
} // namespace shim

#endif // CM_EMU_SHIM_ZE_KERNEL_H
