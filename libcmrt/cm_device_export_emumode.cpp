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

#include "cm_include.h"
#include "cm_device_emumode.h"
#include "cm_statistics.h"

//!
//! \brief      Creates CM Device for emulation mode in linux
//! \details    Creation of more than one CmDevice for concurrent
//!                use is not supported. Trying to create a second one without first destroying
//!                a previously created CmDevice will return CM_FAILURE. The CM API version
//!                supported by the runtime library will be returned in parameter version.
//!                The third parameter can input a given VADisplay created by user using
//!                LibVA’s vaInitialize().
//! \param      [out] pD
//!             Reference to the pointer to the CmDevice to be created
//! \param      [out] version
//!             Reference to CM API version supported by the runtime library
//! \param        [in] va_dpy
//!                Reference to a given VADisplay from vaInitialize if NOT nullptr
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

//!
//! \brief      Destroys CM Device for emulation mode
//! \details    Also destroys surfaces, kernels, samplers and the queue
//!                that were created using this device instance that have not explicitly been
//!                destroyed by calling respective destroy functions.
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

