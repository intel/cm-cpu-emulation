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

#ifndef GUARD_common_type_memory_object_control_h
#define GUARD_common_type_memory_object_control_h

enum MEMORY_OBJECT_CONTROL
{
    MEMORY_OBJECT_CONTROL_SKL_DEFAULT = 0,
    MEMORY_OBJECT_CONTROL_SKL_NO_L3,
    MEMORY_OBJECT_CONTROL_SKL_NO_LLC_ELLC,
    MEMORY_OBJECT_CONTROL_SKL_NO_LLC,
    MEMORY_OBJECT_CONTROL_SKL_NO_ELLC,
    MEMORY_OBJECT_CONTROL_SKL_NO_LLC_L3,
    MEMORY_OBJECT_CONTROL_SKL_NO_ELLC_L3,
    MEMORY_OBJECT_CONTROL_SKL_NO_CACHE,
    MEMORY_OBJECT_CONTROL_SKL_COUNT,

    MEMORY_OBJECT_CONTROL_TGL_DEFAULT = 0,
    MEMORY_OBJECT_CONTROL_TGL_L1_ENABLED,
    MEMORY_OBJECT_CONTROL_TGL_COUNT,

    MEMORY_OBJECT_CONTROL_UNKNOWN = 0xff
};

#endif // GUARD_common_type_memory_object_control_h
