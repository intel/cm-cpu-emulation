/*===================== begin_copyright_notice ==================================

 Copyright (c) 2021, Intel Corporation


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
