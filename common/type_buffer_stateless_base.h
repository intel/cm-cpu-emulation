/*========================== begin_copyright_notice ============================

Copyright (C) 2020 Intel Corporation

SPDX-License-Identifier: MIT

============================= end_copyright_notice ===========================*/

#ifndef GUARD_common_type_buffer_stateless_base_h
#define GUARD_common_type_buffer_stateless_base_h
//! \brief      CmBufferStateless Class to manage 1D surface created from vedio or
//!             system memory.
//! \details    CmBufferStateless represents a 1D surface in vedio memory or system
//!             space that is stateless-accessed by GPU. The stateless buffer can be
//!             created from vedio memory, which can be only accessed by GPU. It also
//!             can be created from system memory, which is local shared memor
//!             between GPU and CPU and can be access by both GPU and CPU. GPU can
//!             access the memory by passing the graphics address to CM kernel
//!             function(genx_main) as an argument for kernel to use. CPU can access the
//!             system address dirtectly.
class CmBufferStateless
{
public:
    //!
    //! \brief      Get the staring address in graphics memory space.
    //! \param      [out] gfxAddr
    //!             Staring address in graphics memory space.
    //! \returns    CM_SUCCESS.
    //!
    CM_RT_API virtual int32_t GetGfxAddress(uint64_t &gfxAddr) = 0;

    //!
    //! \brief      Get the starting address in system memory space.
    //! \param      [out] sysAddr
    //!             Starting address in system memory space.
    //! \returns    CM_SUCCESS.
    //!
    CM_RT_API virtual int32_t GetSysAddress(void *&sysAddr) = 0;

    //!
    //! \brief      Copies data in stateless buffer to system memory using CPU.
    //! \details    The size of data copied is the size of data in buffer.This
    //!             API is a blocking function, i.e. the function will not
    //!             return until the copy operation is completed. Buffer
    //!             reading will not happen until the status of the dependent
    //!             event becomes CM_STATUS_FINISHED. It is application's
    //!             responsibility to make sure no other task enqueued after
    //!             the task corresponding to the dependent task but before
    //!             ReadSurface. If sysMemSize is given, it will be checked
    //!             against the size needed for the buffer. If the sysMemSize
    //!             is less than the CmBufferStateless size, copy only happens for
    //!             sysMemSize bytes, not all data in CmBufferStateless.
    //! \param      [out] sysMem
    //!             Pointer to the system memory receiving buffer data.
    //! \param      [in] event
    //!             Pointer to the dependent event used for sychronization.
    //! \param      [in] sysMemSize
    //!             Size of the system memory in byte.
    //! \retval     CM_SUCCESS if copy is successful.
    //! \retval     CM_INVALID_ARG_VALUE if sysMem is nullptr.
    //! \retval     CM_LOCK_SURFACE_FAIL if surface locking is failed.
    //! \retval     CM_FAILURE otherwise.
    //!
    CM_RT_API virtual int32_t
    ReadSurface(unsigned char *sysMem,
                CmEvent *event,
                uint64_t sysMemSize = 0xFFFFFFFFFFFFFFFFULL) = 0;

    //!
    //! \brief      Copies system memory content to stateless buffer usingg CPU.
    //! \details    The size of data copied is the size of data in the
    //!             buffer. This is a blocking function, i.e. the function will
    //!             not return until the copy operation is completed. Buffer
    //!             writing will not happen until the status of the dependent
    //!             event becomes CM_STATUS_FINISHED. The dependent event for
    //!             WriteSurface is usually NULL. If sysMemSize is given, it
    //!             will be checked against the size needed for the buffer.
    //!             If the sysMemSize is less than the CmBufferStateless size,
    //!             copy only happens for sysMemSize bytes, not all data in
    //!             CmBufferStateless.
    //! \param      [in] sysMem
    //!             Pointer to the system memory storing the buffer data.
    //! \param      [in] event
    //!             Pointer to the dependent event used for sychronization.
    //! \param      [in] sysMemSize
    //!             Size of the system memory in byte.
    //! \retval     CM_SUCCESS if copy is successful.
    //! \retval     CM_INVALID_ARG_VALUE if sysMem pointer is nullptr.
    //! \retval     CM_LOCK_SURFACE_FAIL if surface locking is failed.
    //! \retval     CM_FAILURE otherwise.
    //!
    CM_RT_API virtual int32_t
    WriteSurface(const unsigned char *sysMem,
                 CmEvent *event,
                 uint64_t sysMemSize = 0xFFFFFFFFFFFFFFFFULL) = 0;

    virtual ~CmBufferStateless () = default;
};
#endif // GUARD_common_type_buffer_stateless_base_h
