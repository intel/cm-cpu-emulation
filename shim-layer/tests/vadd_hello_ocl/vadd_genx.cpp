/*========================== begin_copyright_notice ============================

Copyright (C) 2019 Intel Corporation

SPDX-License-Identifier: MIT

============================= end_copyright_notice ===========================*/

#include <cm/cm.h>

#include "shim_support.h"

extern "C" _GENX_MAIN_ void
vadd(SurfaceIndex ibuf0 [[type("buffer_t int")]],
    SurfaceIndex ibuf1 [[type("buffer_t int")]],
    SurfaceIndex obuf [[type("buffer_t int")]])
{
    unsigned tid = cm_group_id(0) * cm_local_size(0) + cm_local_id(0);

    vector<unsigned, 32> in0;
    vector<unsigned, 32> in1;
    read(ibuf0, tid * 32 * sizeof(unsigned), in0);
    read(ibuf1, tid * 32 * sizeof(unsigned), in1);
    vector<unsigned, 32> in2 = in0 + in1;
    write(obuf,  tid * 32 * sizeof(unsigned), in2);
}

EXPORT_SIGNATURE(vadd);
