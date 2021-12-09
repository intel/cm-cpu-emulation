/*========================== begin_copyright_notice ============================

Copyright (C) 2017 Intel Corporation

SPDX-License-Identifier: MIT

============================= end_copyright_notice ===========================*/

#ifndef GENX_THREADING_H
#define GENX_THREADING_H

#include <atomic>

#include "cm_common_macros.h"

namespace CmEmulSys {

CM_API void init_tm();
CM_API void finalize_tm();
void enter_dataport_cs();
void leave_dataport_cs();

}; //:: CmEmulSys

#endif /* GENX_THREADING */
