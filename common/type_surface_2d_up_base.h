/*========================== begin_copyright_notice ============================

Copyright (C) 2020 Intel Corporation

SPDX-License-Identifier: MIT

============================= end_copyright_notice ===========================*/

#ifndef GUARD_common_type_surface_2d_up_base_h
#define GUARD_common_type_surface_2d_up_base_h

//! \brief      CmSurface2DUP Class to manage 2D surface created upon user
//!             provided system memory.
//! \details    Each CmSurface2DUP object is associated with a SurfaceIndex
//!             object containing a unique index value the surface is mapped to
//!             when created by the CmDevice. The CmDevice keeps the mapping b/w
//!             index and CmSurface2DUP. The SurfaceIndex is passed to CM kernel
//!             function (genx_main) as argument to indicate the surface. CPU can
//!             access the system memory through the memory point. GPU can access
//!             the 2D surface through SurfaceIndex in kernel. It is application's
//!             responsibility to make sure the accesses from both sides are not
//!             overlapped.
class CmSurface2DUP
{
public:
    //!
    //! \brief      This function returns the SurfaceIndex object associated
    //!             with the surface.
    //! \param      [out] pIndex
    //!             Reference to the pointer to SurfaceIndex.
    //! \returns    CM_SUCCESS.
    //!
    CM_RT_API virtual int32_t GetIndex( SurfaceIndex*& pIndex ) = 0;

    //!
    //! \brief      Set surface property for interlace usage.
    //! \details    A surface can be set as top filed, bottom filed for
    //!             interlace usage.
    //! \param      [in] frameType
    //!             It specifies type for surface, it has three
    //!             types:CM_FRAME,CM_TOP_FIELD, CM_BOTTOM_FIELD.
    //! \note       By default, the surface property is CM_FRAME type which means
    //!             its content is progressive data, not interleave data.
    //! \returns    CM_SUCCESS.
    //!
    CM_RT_API virtual int32_t SetProperty(CM_FRAME_TYPE frameType) = 0;

    virtual ~CmSurface2DUP () = default;
};
#endif // GUARD_common_type_surface_2d_up_base_h
