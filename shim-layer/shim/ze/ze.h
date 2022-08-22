/*========================== begin_copyright_notice ============================

Copyright (C) 2022 Intel Corporation

SPDX-License-Identifier: MIT

============================= end_copyright_notice ===========================*/

#ifndef CM_EMU_SHIM_ZE_ZE_H
#define CM_EMU_SHIM_ZE_ZE_H

#include <level_zero/ze_api.h>

#include "shim.h"

namespace shim {
namespace ze {
struct Error : public std::runtime_error {
  Error(const std::string &msg) : std::runtime_error(msg) {}
};
} // namespace ze
} // namespace shim

#endif // CM_EMU_SHIM_ZE_ZE_H
