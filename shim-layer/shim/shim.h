/*========================== begin_copyright_notice ============================

Copyright (C) 2019-2022 Intel Corporation

SPDX-License-Identifier: MIT

============================= end_copyright_notice ===========================*/

#ifndef CM_EMU_SHIM_SHIM_H
#define CM_EMU_SHIM_SHIM_H

#include <cm_rt.h>

#include <cm_device_emumode.h>
#include <cm_event_emumode.h>
#include <cm_kernel_emumode.h>
#include <cm_program_emumode.h>
#include <cm_queue_emumode.h>

#if defined(_WIN32)
// Alias does not work in MSVC, using def file
#define ALIAS_X(name, aliasname)
#else // defined(_WIN32)
#define ALIAS_X(name, aliasname)                                               \
  extern __typeof(name) aliasname __attribute__((alias(#name)))
#endif // defined(_WIN32)

#define ALIAS(name, aliasname) ALIAS_X(name, aliasname)

#define SHIM_CALL(x) shim_##x

#define SHIM_EXPORT(x) ALIAS(SHIM_CALL(x), x)

namespace shim {
template <typename T>
using EnableIfEmuRefCounterT =
    std::void_t<decltype(std::declval<T *>()->SafeRelease())>;

template <typename T, typename = EnableIfEmuRefCounterT<T>>
inline void IntrusivePtrAddRef(T *ptr) {
  if (ptr) {
    ptr->Acquire();
  }
}

template <typename T, typename = EnableIfEmuRefCounterT<T>>
inline void IntrusivePtrRelease(T *ptr) {
  if (ptr) {
    ptr->SafeRelease();
  }
}
} // namespace shim

#endif // CM_EMU_SHIM_SHIM_H
