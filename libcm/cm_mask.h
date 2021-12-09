/*========================== begin_copyright_notice ============================

Copyright (C) 2020 Intel Corporation

SPDX-License-Identifier: MIT

============================= end_copyright_notice ===========================*/

#ifndef CM_CM_MASK_H
#define CM_CM_MASK_H

#include "cm_traits.h"
#include "libcm_common.h"
#include "cm_intrin.h"

using MaskIntT = uint;
using MaskVecElemT = ushort;
template<unsigned size>
using MaskVecT = vector<MaskVecElemT, size>;

#endif // CM_CM_MASK_H
