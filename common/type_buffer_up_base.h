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

#ifndef GUARD_common_type_buffer_up_base_h
#define GUARD_common_type_buffer_up_base_h

//!
//! \brief      CmBufferUP class to manage 1D resource in user provided system memory.
//! \details    CmBufferUP represents a 1D surface in system memory.It is
//!             created upon the UP(User Provided) memory. The UP memory
//!             must be page(4K Bytes) aligned. CPU can access the memory
//!             as usual.Each CmBufferUP object is associated with a SurfaceIndex
//!             object containing a unique index value the surface is mapped to
//!             when created by the CmDevice.The CmDevice keeps the mapping b/w
//!             index and CmBufferUP. The SurfaceIndex is passed to CM kernel
//!             function(genx_main) as argument to indicate the surface. It is
//!             application’s responsibility to make sure the accesses from CPU
//!             and GPU are not overlapped.
//!
class CmBufferUP
{
public:
    //!
    //! \brief      This function returns the SurfaceIndex object associated
    //!             with this CmBufferUp object.
    //! \param      [out] pIndex
    //!             Reference to the pointer to SurfaceIndex.
    //! \returns    CM_SUCCESS.
    //!
    CM_RT_API virtual int32_t GetIndex(SurfaceIndex *&pIndex) = 0;

    //!
    //! \brief      Selects one of the pre-defined memory object control
    //!             settings for this buffer.
    //! \param      [in] option
    //!             Option of the pre-defined memory object control setting.
    //! \retval     CM_SUCCESS if the memory object control is set successfully.
    //! \retval     CM_FAILURE otherwise.
    //! \note       This API is only supported for Gen9 and plus platform.
    //!
    CM_RT_API virtual int32_t
    SelectMemoryObjectControlSetting(MEMORY_OBJECT_CONTROL option) = 0;

    virtual ~CmBufferUP () = default;
};
#endif // GUARD_common_type_buffer_up_base_h
