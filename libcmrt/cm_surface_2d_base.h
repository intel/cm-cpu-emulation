/*========================== begin_copyright_notice ============================

Copyright (C) 2017 Intel Corporation

SPDX-License-Identifier: MIT

============================= end_copyright_notice ===========================*/

#ifndef CMRTLIB____SHARE_CM_SURFACE_2D_BASE_H_
#define CMRTLIB____SHARE_CM_SURFACE_2D_BASE_H_

#include <cstdint>
#include "cm_def.h"
#include "cm_surface_alias_table.h"

class SurfaceIndex;
class CmEvent;
struct CM_SURFACE2D_STATE_PARAM;

#define CM_MAX_NUM_2D_ALIASES       10                                  // maximum number of aliases for one surface. Arbitrary - can be increased

#include "type_surface_2d_base.h"

#endif  // #ifndef CMRTLIB____SHARE_CM_SURFACE_2D_BASE_H_
