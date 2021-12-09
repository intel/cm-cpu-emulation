/*========================== begin_copyright_notice ============================

Copyright (C) 2017 Intel Corporation

SPDX-License-Identifier: MIT

============================= end_copyright_notice ===========================*/

#include "cm_include.h"
#include "cm_device_emumode.h"
#include "cm_statistics.h"

#ifdef CM_DX9
//!
//! \brief      Creates CM Device for emulation mode [DX9]
//! \details    Creates a CmDevice from scratch or creates a CmDevice based on the input
//!				IDirect3DDeviceManager9 interface. If CmDevice is created from scratch, an
//!				internal IDirect3DDeviceManager9 interface will be created by the runtime.
//!				Creation of more than one CmDevice for concurrent use is supported.
//!				The CM API version supported by the runtime library will be
//!				returned in parameter version.
//! \param      [out] pD
//!             Reference to the pointer to the CmDevice to be created
//! \param      [out] version
//!             Reference to CM API version supported by the runtime library
//! \param		[in] pD3DDeviceMgr
//!				Pointer to the IDirect3DDeviceManager9 interface whose default value is NULL.
//!				If it is not null, the IDirect3DDeviceManager9 must contain a D3D Device inside.
//! \retval     CM_SUCCESS if device successfully created
//! \retval     CM_OUT_OF_HOST_MEMORY if out of system memory
//! \retval     CM_FAILURE otherwise
//!
extern "C"
CM_RT_API int32_t CreateCmDevice(CmDevice* &pD, uint32_t& version, IDirect3DDeviceManager9* pD3DDeviceMgr = nullptr)
{
    CmDeviceEmu* p=NULL;
    int32_t result = CmDeviceEmu::Create(pD3DDeviceMgr, p);
    pD = static_cast< CmDevice* >(p);
    if( result == CM_SUCCESS )
    {
        version = CURRENT_CM_VERSION;
    }
    else
    {
        version = 0;
    }
#ifdef GFX_EMU_DEBUG_ENABLED
    CmStatistics::Create();
#endif

    return result;
}

#elif defined(CM_DX11)
//!
//! \brief      Creates CM Device for emulation mode [DX11]
//! \details    Creates a CmDevice from scratch or creates a CmDevice based on the input
//!				ID3D11Device interface. If CmDevice is created from scratch (the input
//!				ID3D11Device interface is nullptr), an internal ID3D11Device and
//!				ID3D11DeviceContext interface will be created by the runtime.
//!				If CmDevice is created based on the input ID3D11Device interface, the
//!				ID3D11Device must be passed in. Creation of more than one CmDevice for
//!				concurrent use is supported. The CM API version supported by the runtime
//!				library will be returned in parameter version.
//! \param      [out] pD
//!             Reference to the pointer to the CmDevice to be created
//! \param      [out] version
//!             Reference to CM API version supported by the runtime library
//! \param		[in] pD3DDeviceMgr
//!				Pointer to the ID3D11Device interface whose default value is NULL. If it is not
//!				null, the ID3D11Device must contain a D3D Device inside.
//! \retval     CM_SUCCESS if the CmDevice is successfully created
//! \retval     CM_OUT_OF_HOST_MEMORY if out of system memory
//! \retval     CM_FAILURE otherwise
//!
extern "C"
CM_RT_API int32_t CreateCmDevice(CmDevice* &pD, uint32_t& version, ID3D11Device* pD3DDeviceMgr = nullptr)
{
    CmDeviceEmu* p=NULL;
    int32_t result = CmDeviceEmu::Create(pD3DDeviceMgr, p);
    pD = static_cast< CmDevice* >(p);
    if( result == CM_SUCCESS )
    {
        version = CURRENT_CM_VERSION;
    }
    else
    {
        version = 0;
    }
#ifdef GFX_EMU_DEBUG_ENABLED
    CmStatistics::Create();
#endif

    return result;
}

#elif defined(__GNUC__)

//!
//! \brief      Creates CM Device for emulation mode in linux
//! \details    Creation of more than one CmDevice for concurrent
//!				use is not supported. Trying to create a second one without first destroying
//!				a previously created CmDevice will return CM_FAILURE. The CM API version
//!				supported by the runtime library will be returned in parameter version.
//!				The third parameter can input a given VADisplay created by user using
//!				LibVA's vaInitialize().
//! \param      [out] pD
//!             Reference to the pointer to the CmDevice to be created
//! \param      [out] version
//!             Reference to CM API version supported by the runtime library
//! \param		[in] va_dpy
//!				Reference to a given VADisplay from vaInitialize if NOT nullptr
//! \retval     CM_SUCCESS if device successfully created
//! \retval     CM_OUT_OF_HOST_MEMORY if out of system memory
//! \retval     CM_FAILURE otherwise
//!
extern "C"
CM_RT_API int32_t CreateCmDevice(CmDevice* &pD, uint32_t& version, VADisplay va_dpy = nullptr)
{
    CmDeviceEmu* p=NULL;

    int32_t result = CmDeviceEmu::Create(p);
    pD = static_cast< CmDevice* >(p);
    if( result == CM_SUCCESS )
    {
        version = CURRENT_CM_VERSION;
    }
    else
    {
        version = 0;
    }
#ifdef GFX_EMU_DEBUG_ENABLED
        CmStatistics::Create();
#endif

    return result;
}

#endif

//!
//! \brief      Destroys CM Device for emulation mode
//! \details    Also destroys surfaces, kernels, samplers and the queue
//!                that were created using this device instance that have not explicitly been
//!                destroyed by calling respective destroy functions.
//!                The internal IDirect3DDeviceManager9 interface will be destroyed if CreateCmDevice was called
//!                with input pD3DDeviceMgr as NULL. The internal IDirect3DDeviceManager9 interface
//!                will NOT be destroyed if CreateCmDevice was called with a valid input
//!                pD3DDeviceMgr.
//! \param      [out] pD
//!             Reference to the pointer to the CmDevice to be destroyed
//! \retval     CM_SUCCESS if the CmDevice is successfully destroyed
//! \retval     CM_FAILURE otherwise
//!
extern "C"
CM_RT_API int32_t DestroyCmDevice(CmDevice* &pD)
{
    CmDeviceEmu* p = dynamic_cast<CmDeviceEmu*>(pD);
    int32_t result = CmDeviceEmu::Destroy(p);
    if(result == CM_SUCCESS)
    {
        pD = nullptr;
    }
#ifdef GFX_EMU_DEBUG_ENABLED
    CmStatistics::Destroy();
#endif
    return result;
}
