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

#ifndef GUARD_common_type_buffer_svm_base_h
#define GUARD_common_type_buffer_svm_base_h

//! \brief      CmBufferSVM Class to manage SVM (Shared Virtual Memory) resource.
//! \details    CmBufferSVM represents a 1D surface in SVM (Shared Virtual Memory)
//!             memory space that is shared between CPU and GPU. The SVM memory must
//!             be page(4K Bytes) aligned and runtime allocates SVM memory when
//!             CmDevice::CreateBufferSVM is called. CPU can access the memory by using
//!             the address returned by GetAddress. GPU can access the memory in two ways
//!             One way is to use the surfaceIndex returned by GetIndex, similar to all
//!             other surfaces and buffers. The other way is to pass the address to CM
//!             kernel function(genx_main) as an argument for kernel to use.
class CmBufferSVM
{
public:
    //!
    //! \brief      This function returns the SurfaceIndex object associated
    //!             with the SVM resource.
    //! \param      [out] pIndex
    //!             Reference to the pointer to SurfaceIndex.
    //! \returns    CM_SUCCESS.
    //!
    CM_RT_API virtual int32_t GetIndex(SurfaceIndex *&pIndex) = 0;

    //!
    //! \brief      Get the pointer of allocated SVM memory starting address.
    //! \param      [out] pAddr
    //!             return the allocated SVM memory starting address.
    //! \returns    CM_SUCCESS.
    //!
    CM_RT_API virtual int32_t GetAddress(void *&pAddr) = 0;

    virtual ~CmBufferSVM () = default;
};
#endif // GUARD_common_type_buffer_svm_base_h
