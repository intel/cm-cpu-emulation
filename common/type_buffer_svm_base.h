/*========================== begin_copyright_notice ============================

Copyright (C) 2020 Intel Corporation

SPDX-License-Identifier: MIT

============================= end_copyright_notice ===========================*/

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
