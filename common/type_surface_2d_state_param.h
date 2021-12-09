/*========================== begin_copyright_notice ============================

Copyright (C) 2020 Intel Corporation

SPDX-License-Identifier: MIT

============================= end_copyright_notice ===========================*/

#ifndef GUARD_common_type_surface_2d_state_param_h
#define GUARD_common_type_surface_2d_state_param_h
struct CM_SURFACE2D_STATE_PARAM {
  unsigned int format;
  unsigned int width;
  unsigned int height;
  unsigned int depth;
  unsigned int pitch;
  unsigned short memory_object_control;
  unsigned int surface_x_offset;
  unsigned int surface_y_offset;
  unsigned int reserved[4];  // for future usage
};
#endif // GUARD_common_type_surface_2d_state_param_h
