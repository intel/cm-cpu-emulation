/*========================== begin_copyright_notice ============================

Copyright (C) 2022 Intel Corporation

SPDX-License-Identifier: MIT

============================= end_copyright_notice ===========================*/

#include "sampler.h"

extern "C" {
SHIM_EXPORT(zeSamplerCreate);
SHIM_EXPORT(zeSamplerDestroy);
} // extern "C"

ZE_APIEXPORT ze_result_t ZE_APICALL SHIM_CALL(zeSamplerCreate)(
    ze_context_handle_t hContext, ze_device_handle_t hDevice,
    const ze_sampler_desc_t *desc, ze_sampler_handle_t *phSampler) {
  GFX_EMU_WARNING_MESSAGE(fShim, "sampler support is not implemented\n");
  return ZE_RESULT_ERROR_UNSUPPORTED_FEATURE;
}

ZE_APIEXPORT ze_result_t ZE_APICALL SHIM_CALL(zeSamplerDestroy)(
    ze_sampler_handle_t hSampler) {
  GFX_EMU_WARNING_MESSAGE(fShim, "sampler support is not implemented\n");
  return ZE_RESULT_ERROR_UNSUPPORTED_FEATURE;
}
