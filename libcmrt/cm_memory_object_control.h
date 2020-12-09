/*===================== begin_copyright_notice ==================================

 Copyright (c) 2020, Intel Corporation


 Permission is hereby granted, free of charge, to any person obtaining a
 copy of this software and associated documentation files (the "Software"),
 to deal in the Software without restriction, including without limitation
 the rights to use, copy, modify, merge, publish, distribute, sublicense,
 and/or sell copies of the Software, and to permit persons to whom the
 Software is furnished to do so, subject to the following conditions:

 The above copyright notice and this permission notice shall be included
 in all copies or substantial portions of the Software.

 THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
 OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
 THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR
 OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
 ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
 OTHER DEALINGS IN THE SOFTWARE.
======================= end_copyright_notice ==================================*/

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
