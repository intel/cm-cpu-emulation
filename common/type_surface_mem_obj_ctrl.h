/*========================== begin_copyright_notice ============================

Copyright (C) 2020 Intel Corporation

SPDX-License-Identifier: MIT

============================= end_copyright_notice ===========================*/

#ifndef GUARD_common_type_surface_mem_obj_ctrl_h
#define GUARD_common_type_surface_mem_obj_ctrl_h

struct CM_SURFACE_MEM_OBJ_CTRL {
  CM_RT_API CM_SURFACE_MEM_OBJ_CTRL();
  CM_RT_API CM_SURFACE_MEM_OBJ_CTRL(const CM_SURFACE_MEM_OBJ_CTRL &another);
  CM_RT_API CM_SURFACE_MEM_OBJ_CTRL& operator=(
      const CM_SURFACE_MEM_OBJ_CTRL& another);
  MEMORY_OBJECT_CONTROL mem_ctrl;
  MEMORY_TYPE mem_type;
  int age;
};

#endif // GUARD_common_type_surface_mem_obj_ctrl_h
