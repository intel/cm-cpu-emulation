/*========================== begin_copyright_notice ============================

Copyright (C) 2020 Intel Corporation

SPDX-License-Identifier: MIT

============================= end_copyright_notice ===========================*/

#ifndef GUARD_common_type_memory_type_h
#define GUARD_common_type_memory_type_h

enum MEMORY_TYPE {
  CM_USE_PTE = 0,
  CM_UNCACHEABLE,
  CM_WRITE_THROUGH,
  CM_WRITE_BACK,

  MEMORY_TYPE_BDW_UC_WITH_FENCE = 0,
  MEMORY_TYPE_BDW_UC,
  MEMORY_TYPE_BDW_WT,
  MEMORY_TYPE_BDW_WB
};

#endif // GUARD_common_type_memory_type_h
