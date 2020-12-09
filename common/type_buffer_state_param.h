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

#ifndef GUARD_common_type_buffer_state_param_h
#define GUARD_common_type_buffer_state_param_h

struct CM_BUFFER_STATE_PARAM {
    CM_RT_API CM_BUFFER_STATE_PARAM();
    CM_RT_API CM_BUFFER_STATE_PARAM(const CM_BUFFER_STATE_PARAM &another);
    const CM_BUFFER_STATE_PARAM& operator=(
        const CM_BUFFER_STATE_PARAM &another);

    uint32_t size;  //! Designated size of the buffer, if it is 0, set it as the original width.

    uint32_t base_address_offset;  //! Offset should be 16-aligned.

    CM_SURFACE_MEM_OBJ_CTRL mocs;  //! Memory object control settings for surfaces.
                                   //! If not set (all zeros), then the aliases mocs setting is the
                                   //! same as the original one.
};
#endif // GUARD_common_type_buffer_state_param_h
