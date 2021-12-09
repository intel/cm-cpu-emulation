/*========================== begin_copyright_notice ============================

Copyright (C) 2017 Intel Corporation

SPDX-License-Identifier: MIT

============================= end_copyright_notice ===========================*/

#include <mutex>

#include "genx_threading.h"

namespace CmEmulSys {

std::recursive_mutex dataportMtx;
CM_API void init_tm() {}
CM_API void finalize_tm() {}
void enter_dataport_cs() { dataportMtx.lock (); }
void leave_dataport_cs() { dataportMtx.unlock (); }

};
