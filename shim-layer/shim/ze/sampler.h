/*========================== begin_copyright_notice ============================

Copyright (C) 2022 Intel Corporation

SPDX-License-Identifier: MIT

============================= end_copyright_notice ===========================*/

#ifndef CM_EMU_SHIM_ZE_SAMPLER_H
#define CM_EMU_SHIM_ZE_SAMPLER_H

#include "intrusive_pointer.h"
#include "ze.h"

namespace shim {
namespace ze {
struct Sampler : public IntrusiveRefCounter<Sampler> {};
} // namespace ze
} // namespace shim

extern "C" {
ZE_APIEXPORT ze_result_t ZE_APICALL SHIM_CALL(zeSamplerCreate)(
    ze_context_handle_t hContext, ze_device_handle_t hDevice,
    const ze_sampler_desc_t *desc, ze_sampler_handle_t *phSampler);
ZE_APIEXPORT ze_result_t ZE_APICALL
    SHIM_CALL(zeSamplerDestroy)(ze_sampler_handle_t hSampler);
} // extern "C"
#endif // CM_EMU_SHIM_ZE_SAMPLER_H
