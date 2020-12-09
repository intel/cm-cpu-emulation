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

#ifndef GUARD_common_type_buffer_base_h
#define GUARD_common_type_buffer_base_h

//!
//! \brief      CmBuffer class to manage 1D surface in video memory.
//! \details    CmBuffer represents a 1D surface in video memory. Each CmBuffer
//!             object is associated with a SurfaceIndex object containing a
//!             unique index value the surface is mapped to when created by the
//!             CmDevice. The CmDevice keeps the mapping b/w index and CmBuffer.
//!             The SurfaceIndex is passed to CM kernel function (genx_main) as
//!             argument to indicate the surface.
//!
class CmBuffer
{
public:
    //!
    //! \brief      This function returns the SurfaceIndex object associated
    //!             with this CmBuffer object.
    //! \param      [out] pIndex
    //!             Reference to the pointer to SurfaceIndex.
    //! \returns    CM_SUCCESS.
    //!
    CM_RT_API virtual int32_t GetIndex(SurfaceIndex *&pIndex) = 0;

    //!
    //! \brief      Copies data in this buffer to system memory using CPU.
    //! \details    The size of data copied is the size of data in buffer.This
    //!             API is a blocking function, i.e. the function will not
    //!             return until the copy operation is completed. Buffer
    //!             reading will not happen until the status of the dependent
    //!             event becomes CM_STATUS_FINISHED. It is application's
    //!             responsibility to make sure no other task enqueued after
    //!             the task corresponding to the dependent task but before
    //!             ReadSurface. If sysMemSize is given, it will be checked
    //!             against the size needed for the buffer. If the sysMemSize
    //!             is less than the CmBuffer size, copy only happens for sysMemSize
    //!             bytes, not all data in CmBuffer.
    //! \param      [out] pSysMem
    //!             Pointer to the system memory receiving buffer data.
    //! \param      [in] pEvent
    //!             Pointer to the dependent event used for sychronization.
    //! \param      [in] sysMemSize
    //!             Size of the system memory in byte.
    //! \retval     CM_SUCCESS if copy is successful.
    //! \retval     CM_INVALID_ARG_VALUE if sysMem is nullptr.
    //! \retval     CM_LOCK_SURFACE_FAIL if surface locking is failed.
    //! \retval     CM_FAILURE otherwise.
    //!
    CM_RT_API virtual int32_t ReadSurface(unsigned char *pSysMem,
                                          CmEvent *pEvent,
                                          uint64_t sysMemSize = 0xFFFFFFFFFFFFFFFFULL) = 0;

    //!
    //! \brief      Copies system memory content to this buffer usingg CPU.
    //! \details    The size of data copied is the size of data in the
    //!             buffer. This is a blocking function, i.e. the function will
    //!             not return until the copy operation is completed. Buffer
    //!             writing will not happen until the status of the dependent
    //!             event becomes CM_STATUS_FINISHED. The dependent event for
    //!             WriteSurface is usually NULL. If sysMemSize is given, it
    //!             will be checked against the size needed for the buffer.
    //!             If the sysMemSize is less than the CmBuffer size, copy only
    //!             happens for sysMemSize bytes, not all data in CmBuffer.
    //! \param      [in] pSysMem
    //!             Pointer to the system memory storing the buffer data.
    //! \param      [in] pEvent
    //!             Pointer to the dependent event used for sychronization.
    //! \param      [in] sysMemSize
    //!             Size of the system memory in byte.
    //! \retval     CM_SUCCESS if copy is successful.
    //! \retval     CM_INVALID_ARG_VALUE if sysMem pointer is nullptr.
    //! \retval     CM_LOCK_SURFACE_FAIL if surface locking is failed.
    //! \retval     CM_FAILURE otherwise.
    //!
    CM_RT_API virtual int32_t WriteSurface(const unsigned char *pSysMem,
                                           CmEvent *pEvent,
                                           uint64_t sysMemSize = 0xFFFFFFFFFFFFFFFFULL) = 0;

    //!
    //! \brief      Sets memory in this buffer to uint32_t value using CPU.
    //! \details    The size of data initialized is the size of data in the
    //!             buffer. This is a blocking function, i.e. the function will
    //!             not return until the set operation is completed. Buffer
    //!             initialization will not happen until the status of the
    //!             dependent event becomes CM_STATUS_FINISHED. The dependent
    //!             event for InitSurface is usually NULL.
    //! \param      [in] initValue
    //!             uint32_t value used to initialize to.
    //! \param      [in] pEvent
    //!             Pointer to the dependent event used for sychronization.
    //! \retval     CM_SUCCESS if copy is successful.
    //! \retval     CM_LOCK_SURFACE_FAIL if surface locking is failed.
    //! \retval     CM_FAILURE otherwise.
    //!
    CM_RT_API virtual int32_t InitSurface(const uint32_t initValue,
                                          CmEvent *pEvent) = 0;

    //!
    //! \brief      Selects one of the pre-defined memory object control
    //!             settings for this buffer.
    //! \param      [in] optiom
    //!             Option of the pre-defined memory object control setting.
    //! \retval     CM_SUCCESS if the memory object control is set successfully.
    //! \retval     CM_FAILURE otherwise.
    //! \note       This API is only supported for Gen9 and plus platform.
    //!
    CM_RT_API virtual int32_t
    SelectMemoryObjectControlSetting(MEMORY_OBJECT_CONTROL option) = 0;

    //!
    //! \brief      Sets the surface state of this buffer.
    //! \details    Set the new size, offset, and mocs to the surface index,
    //!             so they will take effect during surface state setting.
    //!             Usually, the surface index is an alias surface index created
    //!             by calling CmDevice::CreateBufferAlias. This function can be used to
    //!             reinterpret the size, offset, and mocs of an existing CmBuffer.
    //! \param      [in] surface_index
    //!             Pointer to surface index of this buffer.
    //! \param      [in] state_param
    //!             The surface state parameter of this buffer. It contains
    //!             size, base address offset and memory object control setting. The
    //!             offset must be 16-aligned
    //!             otherwise it will cause a GPU hang.
    //! \retval     CM_SUCCESS if the surface state is set successfully.
    //! \retval     CM_INVALID_ARG_VALUE if the surface state parameter is
    //!             invalid.
    //! \retval     CM_NULL_POINTER if any internal used pointer is nullptr.
    //! \retval     CM_FAILURE otherwise.
    //!
    CM_RT_API virtual int32_t SetSurfaceStateParam(SurfaceIndex *surface_index,
                                                   const CM_BUFFER_STATE_PARAM *state_param) = 0;

    virtual ~CmBuffer () = default;
};
#endif // GUARD_common_type_buffer_base_h
