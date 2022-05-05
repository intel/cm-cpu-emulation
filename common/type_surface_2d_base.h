/*========================== begin_copyright_notice ============================

Copyright (C) 2017 Intel Corporation

SPDX-License-Identifier: MIT

============================= end_copyright_notice ===========================*/

#ifndef GUARD_common_type_surface_2d_base_h
#define GUARD_common_type_surface_2d_base_h
//! \brief      CmSurface2D represents a 2D surface in video memory.
//! \details    CmSurface2D represents a 2D surface in video memory. It is a
//!             wrapper around
//!             a D3D 2D surface in Windows or a
//!             VA 2D surface in Linux. Each CmSurface2D object is associated with a
//!             SurfaceIndex object containing a unique index value the
//!             surface is mapped to when created by the CmDevice.The CmDevice
//!             keeps the mapping b/w index and CmSurface2D. The SurfaceIndex
//!             is passed to CM kernel function(genx_main) as argument to
//!             indicate the surface.
class CmSurface2D
{
public:
    //!
    //! \brief Retrieves surface index of this CmSurface2D.
    //! \param [out] pIndex
    //!        Reference to an SurfaceIndex pointer.
    //!        It will point to the internal SurfaceIndex.
    //! \retval CM_SUECCESS.
    //!
    CM_RT_API virtual int32_t GetIndex(SurfaceIndex* &pIndex) = 0;

#ifdef CM_DX9
    //!
    //! \brief Retrieves internal D3D9 surface.
    //! \details In HW mode, the D3D surface underlying this CmSurface2D is
    //!          returned in pD3DSurface.
    //! \param [out] pD3DSurface
    //!        Reference to a D3DSurface pointer.
    //! \retval CM_SUECCESS.
    //!
    CM_RT_API virtual int32_t GetD3DSurface(IDirect3DSurface9* &pD3DSurface) = 0;
#elif defined(CM_DX11)
    //!
    //! \brief Retrieves internal D3D11 surface.
    //! \details In HW mode, the D3D surface underlying this CmSurface2D is
	//! \details In HW mode, the D3D surface underlying this CmSurface2D is
	//!          returned in pD3DSurface.
	//! \param [out] pD3D11Texture2D
    //!        Reference to a D3DSurface pointer.
    //! \retval CM_SUECCESS.
    //!
    CM_RT_API virtual int32_t GetD3DSurface(ID3D11Texture2D* &pD3D11Texture2D) = 0;
#endif

    //!
    //! \brief Copies data in this CmSurface2D to system memory.
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
    //! \brief Copies data in system memory to this CmSurface2D.
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

    //!
    //! \brief Copies data in this CmSurface2D to system memory with system
    //!        memory stride.
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
    //! \param [in] stride
    //!        System memory stride in bytes.
    //!        It equals actual surface width in bytes plus extra padding bytes.
    //! \param [in] sysMemSize
    //!        Size of the system memory.
    //! \retval CM_SUCCESS if this function succeeds.
    //! \retval CM_INVALID_ARG_VALUE if sysMemSize is given but less than what
    //!         is needed.
    //! \retval CM_LOCK_SURFACE_FAIL if surface locking fails.
    //! \retval CM_FAILURE otherwise.
    //!
    CM_RT_API virtual int32_t
    ReadSurfaceStride(unsigned char *pSysMem,
                      CmEvent *pEvent,
                      const unsigned int stride,
                      uint64_t sysMemSize = 0xFFFFFFFFFFFFFFFFULL) = 0;

    //!
    //! \brief Copies data in system memory to this CmSurface2D with system
    //!        memory stride.
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
    //!        Pointer to the dependent CmEvent used for sychronization.
    //! \param [in] stride
    //!        System memory stride in bytes.
    //!        It equals actual surface width in bytes plus extra padding bytes.
    //! \param [in] sysMemSize
    //!        Size of the system memory.
    //! \retval CM_SUCCESS if copy is successful.
    //! \retval CM_INVALID_ARG_VALUE if sysMemSize is given but less than what
    //!         is needed.
    //! \retval CM_LOCK_SURFACE_FAIL if surface locking fails.
    //! \retval CM_FAILURE otherwise.
    //!
    CM_RT_API virtual int32_t
    WriteSurfaceStride(const unsigned char *pSysMem,
                       CmEvent *pEvent,
                       const unsigned int stride,
                       uint64_t sysMemSize = 0xFFFFFFFFFFFFFFFFULL) = 0;

    //!
    //! \brief Sets surface data to a unified value.
    //! \details This is a blocking function, i.e. the function will not return
    //!          until the operation is completed.
    //!          Initialization will not happen until the status of the
    //!          dependent event becomes CM_STATUS_FINISHED.
    //! \param [in] initValue
    //!        The value for initialization.
    //! \param [in] pEvent
    //!        Pointer to the dependent event used for sychronization.
    //! \retval CM_SUCCESS if initialization is successful.
    //! \retval CM_LOCK_SURFACE_FAIL if surface locking fails.
    //! \retval CM_FAILURE otherwise.
    //!
    CM_RT_API virtual int32_t InitSurface(const unsigned long initValue,
                                      CmEvent *pEvent) = 0;

#ifdef CM_DX11
    //!
    //! \brief Queries the sub-resource index for this CmSurface2D.
    //! \note This function works only if DX11 is enabled.
    //! \param [out] FirstArraySlice
    //!        Index of array slice.
    //! \param [out] FirstMipSlice
    //!        Index of mip slice. It is always 0 for now since we only a single
    //!        mip level is supported.
    //! \retval CM_SUCCESS.
    //!
    CM_RT_API virtual int32_t
    QuerySubresourceIndex(unsigned int &FirstArraySlice,
                          unsigned int &FirstMipSlice) = 0;
#endif

#ifndef _WIN32
    //!
    //! \brief Retrieves libva surface ID.
    //! \note This function is a Linux-only API.
    //! \param [out] iVASurface
    //!        Reference to a VASurfaceID receiving libva surface ID.
    //! \retval CM_SUCCESS.
    //!
    CM_RT_API virtual int32_t GetVaSurfaceID(VASurfaceID &iVASurface) = 0;
#endif
    //!
    //! \brief Hybrid memory copy from this CmSurface2D to system memory with
    //!        system memory strides.
    //! \details Copied data size is the same as surface data size.
    //!          This is a blocking function, i.e. the function will not return
    //!          until the copy operation is completed.
    //!          Copying will not happen until the status of the dependent event
    //!          becomes CM_STATUS_FINISHED.
    //!          If sysMemSize is given, it will be checked against the size of
    //!          the surface data.
    //! \param [out] pSysMem
    //!        Pointer to the system memory receiving surface data.
    //! \param [in] pEvent
    //!        Pointer to the dependent event used for sychronization.
    //! \param [in] iWidthStride
    //!        Horizontal stride of system memory in bytes.
    //! \param [in] iHeightStride
    //!        Vertical stride of system memory in rows.
    //! \param [in] sysMemSize
    //!        Size of the system memory.
    //! \param [in] uiOption
    //!        Option to disable/enable hybrid memory copy.
    //! \retval CM_SUCCESS if this function succeeds.
    //! \retval CM_INVALID_ARG_VALUE if sysMemSize is given but less than what
    //!         is needed.
    //! \retval CM_LOCK_SURFACE_FAIL if surface locking fails.
    //! \retval CM_FAILURE otherwise.
    //!
    CM_RT_API virtual int32_t
    ReadSurfaceHybridStrides(unsigned char *pSysMem,
                             CmEvent *pEvent,
                             const unsigned int iWidthStride,
                             const unsigned int iHeightStride,
                             uint64_t sysMemSize = 0xFFFFFFFFFFFFFFFFULL,
                             unsigned int uiOption = 0) = 0;

    //!
    //! \brief Hybrid memory copy from system memory to this CmSurface2D with
    //!        system memory stride.
    //! \details Copied data size is the same as the surface data size.
    //!          This is a blocking function, i.e. the function will not return
    //!          until the copy
    //!          operation is completed.
    //!          Copying will not happen until the status of the dependent event
    //!          becomes CM_STATUS_FINISHED.
    //!          If sysMemSize is given, it will be checked against the size of
    //!          the surface data.
    //! \param [in] pSysMem
    //!        Pointer to the system memory storing surface data.
    //! \param [in] pEvent
    //!        Pointer to the dependent event used for sychronization.
    //! \param [in] iWidthStride
    //!        Horizontal stride of system memory in bytes.
    //! \param [in] iHeightStride
    //!        Vertical stride of system memory in rows.
    //! \param [in] sysMemSize
    //!        Size of the system memory.
    //! \param [in] uiOption
    //!        Option to disable/enable hybrid memory copy.
    //! \retval CM_SUCCESS if copy is successful.
    //! \retval CM_INVALID_ARG_VALUE if sysMemSize is given but less than what
    //!         is needed.
    //! \retval CM_LOCK_SURFACE_FAIL if surface locking fails.
    //! \retval CM_FAILURE otherwise.
    //!
    CM_RT_API virtual int32_t
    WriteSurfaceHybridStrides(const unsigned char *pSysMem,
                              CmEvent *pEvent,
                              const uint32_t iWidthStride,
                              const uint32_t iHeightStride,
                              uint64_t sysMemSize = 0xFFFFFFFFFFFFFFFFULL,
                              unsigned int uiOption = 0) = 0;

    //!
    //! \brief Sets frame type of this CmSurface2D.
    //! \details By default this CmSurface2D is a whole frame.
    //! \param [in] frameType
    //!        A value in enumeration CM_FRAME_TYPE, frame type of this
    //!        CmSurface2D. It should be a whole frame or a field in an
    //!        interlaced frame.
    //! \retval CM_SUCCESS.
    //!
    CM_RT_API virtual int32_t SetProperty(CM_FRAME_TYPE frameType) = 0;

    //!
    //! \brief Sets surface state parameters for an alias of this CmSurface2D.
    //! \details If pSurfIndex is nullptr, default state of this CmSurface2D
    //!          is changed.
    //! \param [in] pSurfIndex
    //!        Pointer to the surface index of an alias of this CmSurface2D. A new
    //!        surface state is created for this alias or the existing state is updated.
    //! \param [in] pSSParam
    //!        Pointer to a new state parameter.
    //! \retval CM_INVALID_ARG_VALUE if any parameter is invalid.
    //! \retval CM_SUCCESS if successful.
    //!
    CM_RT_API virtual int32_t
    SetSurfaceStateParam(SurfaceIndex *surface_index,
                         const CM_SURFACE2D_STATE_PARAM *state_param) = 0;

    virtual ~CmSurface2D () = default;
};
#endif // GUARD_common_type_surface_2d_base_h
