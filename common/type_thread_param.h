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

#ifndef GUARD_common_type_thread_param_h
#define GUARD_common_type_thread_param_h

typedef struct _CM_THREAD_PARAM {
    CM_COORDINATE   scoreboardCoordinates;     //[X, Y] terms of the scoreboard values of the current thread.
    uint8_t         scoreboardColor;           // dependency color of the current thread.
    uint8_t         sliceDestinationSelect;    //select determines the slice of the current thread must be sent to.
    uint8_t         subSliceDestinationSelect;    //select determines the sub-slice of the current thread must be sent to.
} CM_THREAD_PARAM, *PCM_THREAD_PARAM;
#endif // GUARD_common_type_thread_param_h
