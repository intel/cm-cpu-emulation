/*========================== begin_copyright_notice ============================

Copyright (C) 2022 Intel Corporation

SPDX-License-Identifier: MIT

============================= end_copyright_notice ===========================*/

#ifndef CM_EMU_SHIM_ZE_MODULE_H
#define CM_EMU_SHIM_ZE_MODULE_H

#include <unordered_map>

#include "ze.h"

#include "context.h"

namespace shim {
namespace ze {
struct Module : public IntrusiveRefCounter<Module> {
  Module(IntrusivePtr<Context> ctx, IntrusivePtr<CmProgramEmu> module)
      : ctx_(ctx), module_(module) {}

  IntrusivePtr<Context> ctx_;
  IntrusivePtr<CmProgramEmu> module_;
};
} // namespace ze
} // namespace shim

extern "C" {
ZE_APIEXPORT ze_result_t ZE_APICALL SHIM_CALL(zeModuleCreate)(
    ze_context_handle_t hContext, ze_device_handle_t hDevice,
    const ze_module_desc_t *desc, ze_module_handle_t *phModule,
    ze_module_build_log_handle_t *phBuildLog);
ZE_APIEXPORT ze_result_t ZE_APICALL
    SHIM_CALL(zeModuleDestroy)(ze_module_handle_t hModule);
ZE_APIEXPORT ze_result_t ZE_APICALL SHIM_CALL(zeModuleDynamicLink)(
    uint32_t numModules, ze_module_handle_t *phModules,
    ze_module_build_log_handle_t *phLinkLog);
ZE_APIEXPORT ze_result_t ZE_APICALL SHIM_CALL(zeModuleBuildLogDestroy)(
    ze_module_build_log_handle_t hModuleBuildLog);
ZE_APIEXPORT ze_result_t ZE_APICALL SHIM_CALL(zeModuleBuildLogGetString)(
    ze_module_build_log_handle_t hModuleBuildLog, size_t *pSize,
    char *pBuildLog);
ZE_APIEXPORT ze_result_t ZE_APICALL SHIM_CALL(zeModuleGetNativeBinary)(
    ze_module_handle_t hModule, size_t *pSize, uint8_t *pModuleNativeBinary);
ZE_APIEXPORT ze_result_t ZE_APICALL SHIM_CALL(zeModuleGetGlobalPointer)(
    ze_module_handle_t hModule, const char *pGlobalName, size_t *pSize,
    void **pptr);
ZE_APIEXPORT ze_result_t ZE_APICALL SHIM_CALL(zeModuleGetKernelNames)(
    ze_module_handle_t hModule, uint32_t *pCount, const char **pNames);
ZE_APIEXPORT ze_result_t ZE_APICALL SHIM_CALL(zeModuleGetProperties)(
    ze_module_handle_t hModule, ze_module_properties_t *pModuleProperties);
ZE_APIEXPORT ze_result_t ZE_APICALL SHIM_CALL(zeKernelCreate)(
    ze_module_handle_t hModule, const ze_kernel_desc_t *desc,
    ze_kernel_handle_t *phKernel);
ZE_APIEXPORT ze_result_t ZE_APICALL
    SHIM_CALL(zeKernelDestroy)(ze_kernel_handle_t hKernel);
ZE_APIEXPORT ze_result_t ZE_APICALL SHIM_CALL(zeModuleGetFunctionPointer)(
    ze_module_handle_t hModule, const char *pFunctionName, void **pfnFunction);
ZE_APIEXPORT ze_result_t ZE_APICALL SHIM_CALL(zeKernelSetGroupSize)(
    ze_kernel_handle_t hKernel, uint32_t groupSizeX, uint32_t groupSizeY,
    uint32_t groupSizeZ);
ZE_APIEXPORT ze_result_t ZE_APICALL SHIM_CALL(zeKernelSuggestGroupSize)(
    ze_kernel_handle_t hKernel, uint32_t globalSizeX, uint32_t globalSizeY,
    uint32_t globalSizeZ, uint32_t *groupSizeX, uint32_t *groupSizeY,
    uint32_t *groupSizeZ);
ZE_APIEXPORT ze_result_t ZE_APICALL SHIM_CALL(
    zeKernelSuggestMaxCooperativeGroupCount)(ze_kernel_handle_t hKernel,
                                             uint32_t *totalGroupCount);
ZE_APIEXPORT ze_result_t ZE_APICALL SHIM_CALL(zeKernelSetArgumentValue)(
    ze_kernel_handle_t hKernel, uint32_t argIndex, size_t argSize,
    const void *pArgValue);
ZE_APIEXPORT ze_result_t ZE_APICALL SHIM_CALL(zeKernelSetIndirectAccess)(
    ze_kernel_handle_t hKernel, ze_kernel_indirect_access_flags_t flags);
ZE_APIEXPORT ze_result_t ZE_APICALL SHIM_CALL(zeKernelGetIndirectAccess)(
    ze_kernel_handle_t hKernel, ze_kernel_indirect_access_flags_t *pFlags);
ZE_APIEXPORT ze_result_t ZE_APICALL SHIM_CALL(zeKernelGetSourceAttributes)(
    ze_kernel_handle_t hKernel, uint32_t *pSize, char **pString);
ZE_APIEXPORT ze_result_t ZE_APICALL SHIM_CALL(zeKernelSetCacheConfig)(
    ze_kernel_handle_t hKernel, ze_cache_config_flags_t flags);
ZE_APIEXPORT ze_result_t ZE_APICALL SHIM_CALL(zeKernelGetProperties)(
    ze_kernel_handle_t hKernel, ze_kernel_properties_t *pKernelProperties);
ZE_APIEXPORT ze_result_t ZE_APICALL SHIM_CALL(zeKernelGetName)(
    ze_kernel_handle_t hKernel, size_t *pSize, char *pName);
} // extern "C"
#endif // CM_EMU_SHIM_ZE_MODULE_H
