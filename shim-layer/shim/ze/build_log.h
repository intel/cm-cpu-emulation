/*========================== begin_copyright_notice ============================

Copyright (C) 2022 Intel Corporation

SPDX-License-Identifier: MIT

============================= end_copyright_notice ===========================*/

#ifndef CM_EMU_SHIM_ZE_BUILD_LOG_H
#define CM_EMU_SHIM_ZE_BUILD_LOG_H

#include "intrusive_pointer.h"

namespace shim::ze {
struct BuildLog : public IntrusiveRefCounter<BuildLog> {
  std::string str;
};
} // namespace shim::ze

#endif // CM_EMU_SHIM_ZE_BUILD_LOG_H
