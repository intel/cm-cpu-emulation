/*========================== begin_copyright_notice ============================

Copyright (C) 2020 Intel Corporation

SPDX-License-Identifier: MIT

============================= end_copyright_notice ===========================*/

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
//!             application's responsibility to make sure the accesses from CPU
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

    virtual ~CmBufferUP () = default;
};
#endif // GUARD_common_type_buffer_up_base_h
