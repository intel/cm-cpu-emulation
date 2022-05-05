/*========================== begin_copyright_notice ============================

Copyright (C) 2019 Intel Corporation

SPDX-License-Identifier: MIT

============================= end_copyright_notice ===========================*/

#ifndef GUARD_common_type_surface_2d_stateless_base_h
#define GUARD_common_type_surface_2d_stateless_base_h

class CmEvent;
//! \brief      CmSurface2DStateless represents a 2D surface in video memory.
//! \details    It is a 2D surface created in vedeo memory, which can be stateless
//!             accessed by GPU.Each CmSurface2DStateless object is associated with a
//!             SurfaceIndex object containing a unique index value the surface is mapped
//!             to when created by the CmDevice. GPU can access the memory by passing
//!             the graphics address to CM kernel function(genx_main) as an argument
//!             for kernel to use. And also, application needs to pass width, height and
//!             pitch as to CM kernel function.
class CmSurface2DStateless
{
public:
    //!
    //! \brief      Get the staring address in graphics memory space.
    //! \param      [out] gfxAddr
    //!             Starting address in graphics memory space.
    //! \returns    CM_SUCCESS.
    //!
    CM_RT_API virtual int32_t GetGfxAddress(uint64_t &gfxAddr) = 0;

    //!
    //! \brief   Copies data in this CmSurface2DStateless to system memory.
    //! \details Copied data size is the same as surface data size.
    //!          This is a blocking function, i.e. the function will not return
    //!          until the copy operation is completed.
    //!          Copying will not happen until the status of the dependent event
    //!          becomes CM_STATUS_FINISHED.
    //!          It's the application's responsibility to make sure no other
    //!          task enqueued between the task corresponding to the event and
    //!          this fuction call.
    //!          If sysMemSize is given, it will be checked against the size of
    //!          the surface data.
    //! \param [out] pSysMem
    //!        Pointer to the system memory receiving surface data.
    //! \param [in] pEvent
    //!        Pointer to the dependent event used for sychronization.
    //! \param [in] sysMemSize
    //!        Size of the system memory.
    //! \retval CM_SUCCESS if this function succeeds.
    //! \retval CM_INVALID_ARG_VALUE if sysMemSize is given but less than what
    //!         is needed.
    //! \retval CM_LOCK_SURFACE_FAIL if surface locking fails.
    //! \retval CM_FAILURE otherwise.
    //!
    CM_RT_API virtual int32_t
    ReadSurface(unsigned char *pSysMem,
                CmEvent *pEvent,
                uint64_t sysMemSize = 0xFFFFFFFFFFFFFFFFULL) = 0;

    //!
    //! \brief Copies data in system memory to this CmSurface2DStateless.
    //! \details Copied data size is the same as the surface data size.
    //!          This is a blocking function, i.e. the function will not return
    //!          until the copy operation is completed.
    //!          Copying will not happen until the status of the dependent event
    //!          becomes CM_STATUS_FINISHED.
    //!          If sysMemSize is given, it will be checked against the size of
    //!          the surface data.
    //! \param [in] pSysMem
    //!        Pointer to the system memory storing surface data.
    //! \param [in] pEvent
    //!        Pointer to the dependent event used for sychronization.
    //! \param [in] sysMemSize
    //!        Size of the system memory.
    //! \retval CM_SUCCESS if copy is successful.
    //! \retval CM_INVALID_ARG_VALUE if sysMemSize is given but less than what
    //!         is needed.
    //! \retval CM_LOCK_SURFACE_FAIL if surface locking fails.
    //! \retval CM_FAILURE otherwise.
    //!
    CM_RT_API virtual int32_t
    WriteSurface(const unsigned char *pSysMem,
                 CmEvent *pEvent,
                 uint64_t sysMemSize = 0xFFFFFFFFFFFFFFFFULL) = 0;

    virtual ~CmSurface2DStateless () = default;
};
#endif // GUARD_common_type_surface_2d_stateless_base_h
