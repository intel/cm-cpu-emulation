/*========================== begin_copyright_notice ============================

Copyright (C) 2017 Intel Corporation

SPDX-License-Identifier: MIT

============================= end_copyright_notice ===========================*/

#ifndef CMRTLIB____SHARE_CM_BUFFER_BASE_H_
#define CMRTLIB____SHARE_CM_BUFFER_BASE_H_

#include "cm_surface_alias_table.h"

class SurfaceIndex;
class CmEvent;

#define CM_MAX_NUM_BUFFER_ALIASES 10  // maximum number of aliases for one Buffer. Arbitrary - can be increased

#include "type_buffer_svm_base.h"
#include "type_buffer_up_base.h"
#include "type_buffer_base.h"
#include "type_buffer_stateless_base.h"

#endif  // #ifndef CMRTLIB____SHARE_CM_BUFFER_BASE_H_
