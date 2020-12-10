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

#ifndef GUARD_common_type_surface_3d_base_h
#define GUARD_common_type_surface_3d_base_h

class CmEvent;
class SurfaceIndex;

//! \brief      CmSurface3D class to manage 3D surface in video memory.
//! \details    CmSurface3D represents a 3D surface in video memory. Each
//!             CmSurface3D object is associated with a SurfaceIndex object
//!             containing a unique index value the surface is mapped to when
//!             created by the CmDevice. The CmDevice keeps the mapping b/w
//!             index and CmSurface3D. The SurfaceIndex is passed to CM kernel
//!             function as argument to indicate the surface.
class CmSurface3D
{
public:
    //! \brief      This function returns the SurfaceIndex object associated
    //!             with the surface.
    //! \param      [out] pIndex
    //!             Reference to the pointer to the SurfaceIndex.
    //! \returns    Always returns CM_SUCCESS.
    CM_RT_API virtual int32_t GetIndex( SurfaceIndex*& pIndex ) = 0;

    //! \brief      Copy the surface content to system memory.
    //! \details    The size of data copied is the size of data in surface. It is
    //!             the caller's responsibility to allocate the system memory.
    //!             This is a blocking function, i.e. the function will not
    //!             return until the copy operation is completed. Surface
    //!             reading will not happen until the status of the dependent
    //!             event becomes CM_STATUS_FINISHED. It is application's
    //!             responsibility to make sure no other task enqueued after
    //!             the task corresponding to the dependent task but before
    //!             ReadSurface. If sysMemSize is given, it will be checked
    //!             against the size needed for the surface.
    //! \param      [in] pSysMem
    //!             pointer to the target system memory.
    //! \param      [in] pEvent
    //!             pointer to the dependent event. See also CmEvent.
    //! \param      [in] sysMemSize
    //!             size of the system memory. Default value is -1 which means
    //!             no memory size checking.
    //! \retval     CM_SUCCESS if the copy is successful.
    //! \retval     CM_INVALID_ARG_VALUE if pSysMem is nullptr or the sysMemSize
    //!             is less than total size of CmSurface3D.
    //! \retval     CM_LOCK_SURFACE_FAIL if surface locking is failed.
    //! \retval     CM_EXCEED_MAX_TIMEOUT if the dependent task times out.
    //! \retval     CM_FAILURE otherwise.
    CM_RT_API virtual int32_t ReadSurface( unsigned char* pSysMem, CmEvent* pEvent, uint64_t sysMemSize = 0xFFFFFFFFFFFFFFFFULL ) = 0;

    //! \brief      Copy system memory content to surface.
    //! \details    The size of data copied is the size of data in surface. It is the
    //!             caller's responsibility to allocate the system memory. This
    //!             is a blocking function, i.e. the function will not return
    //!             until the copy operation is completed. Surface writing will
    //!             not happen until the status of the dependent event becomes
    //!             CM_STATUS_FINISHED. The dependent event for WriteSurface is
    //!             usually NULL. If sysMemSize is given, it will be checked
    //!             against the size needed for the surface.
    //! \param      [in] pSysMem
    //!             pointer to the source system memory.
    //! \param      [in] pEvent
    //!             pointer to the dependent event. See also CmEvent.
    //! \param      [in] sysMemSize
    //!             size of the system memory. The default value is -1, which
    //!             means no memory size checking.
    //! \retval     CM_SUCCESS if the copy is successful.
    //! \retval     CM_INVALID_ARG_VALUE if pSysMem is nullptr or the sysMemSize
    //!             is less than total size of CmSurface3D.
    //! \retval     CM_LOCK_SURFACE_FAIL if surface locking is failed.
    //! \retval     CM_EXCEED_MAX_TIMEOUT if the dependent task times out.
    //! \retval     CM_FAILURE otherwise.
    CM_RT_API virtual int32_t WriteSurface( const unsigned char* pSysMem, CmEvent* pEvent, uint64_t sysMemSize = 0xFFFFFFFFFFFFFFFFULL ) = 0;

    //! \brief      Set memory in surface to initValue.
    //! \details    The size of data initialized is the size of data in the surface. This
    //!             is a blocking function, i.e. the function will not return
    //!             until the memset operation is completed. Surface
    //!             initialization will not happen until the status of the
    //!             dependent event becomes CM_STATUS_FINISHED. The dependent
    //!             event for InitSurface is usually NULL.
    //! \param      [in] initValue
    //!             4-byte value used to initilize the memory.
    //! \param      [in] pEvent
    //!             pointer to the dependent event. See also CmEvent.
    //! \retval     CM_SUCCESS if the copy is successful.
    //! \retval     CM_LOCK_SURFACE_FAIL if surface locking is failed.
    //! \retval     CM_FAILURE otherwise.
    CM_RT_API virtual int32_t InitSurface(const uint32_t initValue, CmEvent* pEvent) = 0;

    //! \brief      Selects one of the pre-defined memory object control
    //!             settings for this surface.
    //! \param      [in] option
    //!             Option of the pre-defined memory object control setting.
    //!             See also MEMORY_OBJECT_CONTROL.
    //! \retval     CM_SUCCESS if the copy is successful.
    //! \retval     CM_FAILURE otherwise.
    //! \note       This API is platform related, and only for SKL and plus platforms.
    CM_RT_API virtual int32_t
    SelectMemoryObjectControlSetting(MEMORY_OBJECT_CONTROL option) = 0;

    virtual ~CmSurface3D () = default;
};
#endif // GUARD_common_type_surface_3d_base_h
