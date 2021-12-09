/*========================== begin_copyright_notice ============================

Copyright (C) 2017 Intel Corporation

SPDX-License-Identifier: MIT

============================= end_copyright_notice ===========================*/

#ifndef CMRTLIB_AGNOSTIC_SHARE_CM_MEMORY_OBJECT_CONTROL_H_
#define CMRTLIB_AGNOSTIC_SHARE_CM_MEMORY_OBJECT_CONTROL_H_

#include <ostream>

#include "cm_include.h"
#include "cm_priv_def.h"
#include "type_memory_object_control.h"
#include "type_memory_type.h"
#include "type_surface_mem_obj_ctrl.h"

enum SurfaceTypeInGsf
{
   GRITS_SURFACE,
   GRITS_MEDIABUFFER
};

enum MEMORY_OBJECT_CONTROL_SUFACE_TYPE {
  MEMORY_OBJECT_CONTROL_SUFACE_TYPE_SURFACE,
  MEMORY_OBJECT_CONTROL_SUFACE_TYPE_MEDIABUFFER,
  MEMORY_OBJECT_CONTROL_SUFACE_TYPE_MEMORYBUFFER
};

#endif  // #ifndef CMRTLIB_AGNOSTIC_SHARE_CM_MEMORY_OBJECT_CONTROL_H_
