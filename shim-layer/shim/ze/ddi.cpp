/*========================== begin_copyright_notice ============================

Copyright (C) 2022 Intel Corporation

SPDX-License-Identifier: MIT

============================= end_copyright_notice ===========================*/

#include <level_zero/ze_ddi.h>

#include "cmdlist.h"
#include "context.h"
#include "device.h"
#include "driver.h"
#include "event.h"
#include "fence.h"
#include "image.h"
#include "module.h"
#include "queue.h"
#include "sampler.h"

#define DDIX(type, func) pDdiTable->pfn##func = SHIM_CALL(ze##type##func)

namespace {
ze_result_t CheckVersion(ze_api_version_t version) {
  auto &driver = shim::ze::Driver::Instance();
  auto major = ZE_MAJOR_VERSION(driver.version);
  auto minor = ZE_MINOR_VERSION(driver.version);

  if (ZE_MAJOR_VERSION(version) != major) {
    return ZE_RESULT_ERROR_UNSUPPORTED_VERSION;
  }

  if (ZE_MINOR_VERSION(version) < minor) {
    return ZE_RESULT_ERROR_UNSUPPORTED_VERSION;
  }

  return ZE_RESULT_SUCCESS;
}
} // namespace

extern "C" {
ZE_DLLEXPORT ze_result_t ZE_APICALL zeGetGlobalProcAddrTable(
    ze_api_version_t version, ze_global_dditable_t *pDdiTable) {
  if (pDdiTable == nullptr) {
    return ZE_RESULT_ERROR_INVALID_NULL_POINTER;
  }

  if (auto r = CheckVersion(version); r != ZE_RESULT_SUCCESS) {
    return r;
  }

  pDdiTable->pfnInit = SHIM_CALL(zeInit);

  return ZE_RESULT_SUCCESS;
}

ZE_DLLEXPORT ze_result_t ZE_APICALL zeGetDriverProcAddrTable(
    ze_api_version_t version, ze_driver_dditable_t *pDdiTable) {
  if (pDdiTable == nullptr) {
    return ZE_RESULT_ERROR_INVALID_NULL_POINTER;
  }

  if (auto r = CheckVersion(version); r != ZE_RESULT_SUCCESS) {
    return r;
  }

#define DDI(func) DDIX(Driver, func)
  DDI(Get);
  DDI(GetApiVersion);
  DDI(GetProperties);
  DDI(GetIpcProperties);
  DDI(GetExtensionProperties);
#undef DDI

  return ZE_RESULT_SUCCESS;
}

ZE_DLLEXPORT ze_result_t ZE_APICALL zeGetDeviceProcAddrTable(
    ze_api_version_t version, ze_device_dditable_t *pDdiTable) {
  if (pDdiTable == nullptr) {
    return ZE_RESULT_ERROR_INVALID_NULL_POINTER;
  }

  if (auto r = CheckVersion(version); r != ZE_RESULT_SUCCESS) {
    return r;
  }

#define DDI(func) DDIX(Device, func)
  DDI(Get);
  DDI(GetSubDevices);
  DDI(GetProperties);
  DDI(GetComputeProperties);
  DDI(GetModuleProperties);
  DDI(GetCommandQueueGroupProperties);
  DDI(GetMemoryProperties);
  DDI(GetMemoryAccessProperties);
  DDI(GetCacheProperties);
  DDI(GetImageProperties);
  DDI(GetExternalMemoryProperties);
  DDI(GetP2PProperties);
  DDI(CanAccessPeer);
  DDI(GetStatus);
#undef DDI

  return ZE_RESULT_SUCCESS;
}

ZE_DLLEXPORT ze_result_t ZE_APICALL zeGetContextProcAddrTable(
    ze_api_version_t version, ze_context_dditable_t *pDdiTable) {
  if (pDdiTable == nullptr) {
    return ZE_RESULT_ERROR_INVALID_NULL_POINTER;
  }

  if (auto r = CheckVersion(version); r != ZE_RESULT_SUCCESS) {
    return r;
  }

#define DDI(func) DDIX(Context, func)
  DDI(Create);
  DDI(Destroy);
  DDI(GetStatus);
  DDI(SystemBarrier);
  DDI(MakeMemoryResident);
  DDI(EvictMemory);
  DDI(MakeImageResident);
  DDI(EvictImage);
#undef DDI

  return ZE_RESULT_SUCCESS;
}

ZE_DLLEXPORT ze_result_t ZE_APICALL zeGetCommandQueueProcAddrTable(
    ze_api_version_t version, ze_command_queue_dditable_t *pDdiTable) {
  if (pDdiTable == nullptr) {
    return ZE_RESULT_ERROR_INVALID_NULL_POINTER;
  }

  if (auto r = CheckVersion(version); r != ZE_RESULT_SUCCESS) {
    return r;
  }

#define DDI(func) DDIX(CommandQueue, func)
  DDI(Create);
  DDI(Destroy);
  DDI(ExecuteCommandLists);
  DDI(Synchronize);
#undef DDI

  return ZE_RESULT_SUCCESS;
}

ZE_DLLEXPORT ze_result_t ZE_APICALL zeGetCommandListProcAddrTable(
    ze_api_version_t version, ze_command_list_dditable_t *pDdiTable) {
  if (pDdiTable == nullptr) {
    return ZE_RESULT_ERROR_INVALID_NULL_POINTER;
  }

  if (auto r = CheckVersion(version); r != ZE_RESULT_SUCCESS) {
    return r;
  }

#define DDI(func) DDIX(CommandList, func)
  DDI(Create);
  DDI(CreateImmediate);
  DDI(Destroy);
  DDI(Close);
  DDI(Reset);
  DDI(AppendWriteGlobalTimestamp);
  DDI(AppendBarrier);
  DDI(AppendMemoryRangesBarrier);
  DDI(AppendMemoryCopy);
  DDI(AppendMemoryFill);
  DDI(AppendMemoryCopyRegion);
  DDI(AppendMemoryCopyFromContext);
  DDI(AppendImageCopy);
  DDI(AppendImageCopyRegion);
  DDI(AppendImageCopyToMemory);
  DDI(AppendImageCopyFromMemory);
  DDI(AppendMemoryPrefetch);
  DDI(AppendMemAdvise);
  DDI(AppendSignalEvent);
  DDI(AppendWaitOnEvents);
  DDI(AppendQueryKernelTimestamps);
  DDI(AppendLaunchKernel);
  DDI(AppendLaunchCooperativeKernel);
  DDI(AppendLaunchKernelIndirect);
  DDI(AppendLaunchMultipleKernelsIndirect);
#undef DDI

  return ZE_RESULT_SUCCESS;
}

ZE_DLLEXPORT ze_result_t ZE_APICALL zeGetEventProcAddrTable(
    ze_api_version_t version, ze_event_dditable_t *pDdiTable) {
  if (pDdiTable == nullptr) {
    return ZE_RESULT_ERROR_INVALID_NULL_POINTER;
  }

  if (auto r = CheckVersion(version); r != ZE_RESULT_SUCCESS) {
    return r;
  }

#define DDI(func) DDIX(Event, func)
  DDI(Create);
  DDI(Destroy);
  DDI(HostSignal);
  DDI(HostSynchronize);
  DDI(QueryStatus);
  DDI(HostReset);
  DDI(QueryKernelTimestamp);
#undef DDI

  return ZE_RESULT_SUCCESS;
}

ZE_DLLEXPORT ze_result_t ZE_APICALL zeGetEventPoolProcAddrTable(
    ze_api_version_t version, ze_event_pool_dditable_t *pDdiTable) {
  if (pDdiTable == nullptr) {
    return ZE_RESULT_ERROR_INVALID_NULL_POINTER;
  }

  if (auto r = CheckVersion(version); r != ZE_RESULT_SUCCESS) {
    return r;
  }

#define DDI(func) DDIX(EventPool, func)
  DDI(Create);
  DDI(Destroy);
  DDI(GetIpcHandle);
  DDI(OpenIpcHandle);
  DDI(CloseIpcHandle);
#undef DDI

  return ZE_RESULT_SUCCESS;
}

ZE_DLLEXPORT ze_result_t ZE_APICALL zeGetFenceProcAddrTable(
    ze_api_version_t version, ze_fence_dditable_t *pDdiTable) {
  if (pDdiTable == nullptr) {
    return ZE_RESULT_ERROR_INVALID_NULL_POINTER;
  }

  if (auto r = CheckVersion(version); r != ZE_RESULT_SUCCESS) {
    return r;
  }

#define DDI(func) DDIX(Fence, func)
  DDI(Create);
  DDI(Destroy);
  DDI(HostSynchronize);
  DDI(QueryStatus);
  DDI(Reset);
#undef DDI

  return ZE_RESULT_SUCCESS;
}

ZE_DLLEXPORT ze_result_t ZE_APICALL zeGetImageProcAddrTable(
    ze_api_version_t version, ze_image_dditable_t *pDdiTable) {
  if (pDdiTable == nullptr) {
    return ZE_RESULT_ERROR_INVALID_NULL_POINTER;
  }

  if (auto r = CheckVersion(version); r != ZE_RESULT_SUCCESS) {
    return r;
  }

#define DDI(func) DDIX(Image, func)
  DDI(GetProperties);
  DDI(Create);
  DDI(Destroy);
#undef DDI

  return ZE_RESULT_SUCCESS;
}

ZE_DLLEXPORT ze_result_t ZE_APICALL zeGetKernelProcAddrTable(
    ze_api_version_t version, ze_kernel_dditable_t *pDdiTable) {
  if (pDdiTable == nullptr) {
    return ZE_RESULT_ERROR_INVALID_NULL_POINTER;
  }

  if (auto r = CheckVersion(version); r != ZE_RESULT_SUCCESS) {
    return r;
  }

#define DDI(func) DDIX(Kernel, func)
  DDI(Create);
  DDI(Destroy);
  DDI(SetCacheConfig);
  DDI(SetGroupSize);
  DDI(SuggestGroupSize);
  DDI(SuggestMaxCooperativeGroupCount);
  DDI(SetArgumentValue);
  DDI(SetIndirectAccess);
  DDI(GetIndirectAccess);
  DDI(GetSourceAttributes);
  DDI(GetProperties);
  DDI(GetName);
#undef DDI

  return ZE_RESULT_SUCCESS;
}

ZE_DLLEXPORT ze_result_t ZE_APICALL
zeGetMemProcAddrTable(ze_api_version_t version, ze_mem_dditable_t *pDdiTable) {
  if (pDdiTable == nullptr) {
    return ZE_RESULT_ERROR_INVALID_NULL_POINTER;
  }

  if (auto r = CheckVersion(version); r != ZE_RESULT_SUCCESS) {
    return r;
  }

#define DDI(func) DDIX(Mem, func)
  DDI(AllocShared);
  DDI(AllocDevice);
  DDI(AllocHost);
  DDI(Free);
  DDI(GetAllocProperties);
  DDI(GetAddressRange);
  DDI(GetIpcHandle);
  DDI(OpenIpcHandle);
  DDI(CloseIpcHandle);
#undef DDI

  return ZE_RESULT_SUCCESS;
}

ZE_DLLEXPORT ze_result_t ZE_APICALL zeGetModuleProcAddrTable(
    ze_api_version_t version, ze_module_dditable_t *pDdiTable) {
  if (pDdiTable == nullptr) {
    return ZE_RESULT_ERROR_INVALID_NULL_POINTER;
  }

  if (auto r = CheckVersion(version); r != ZE_RESULT_SUCCESS) {
    return r;
  }

#define DDI(func) DDIX(Module, func)
  DDI(Create);
  DDI(Destroy);
  DDI(DynamicLink);
  DDI(GetNativeBinary);
  DDI(GetGlobalPointer);
  DDI(GetKernelNames);
  DDI(GetProperties);
  DDI(GetFunctionPointer);
#undef DDI

  return ZE_RESULT_SUCCESS;
}

ZE_DLLEXPORT ze_result_t ZE_APICALL zeGetModuleBuildLogProcAddrTable(
    ze_api_version_t version, ze_module_build_log_dditable_t *pDdiTable) {
  if (pDdiTable == nullptr) {
    return ZE_RESULT_ERROR_INVALID_NULL_POINTER;
  }

  if (auto r = CheckVersion(version); r != ZE_RESULT_SUCCESS) {
    return r;
  }

#define DDI(func) DDIX(ModuleBuildLog, func)
  DDI(Destroy);
  DDI(GetString);
#undef DDI

  return ZE_RESULT_SUCCESS;
}

ZE_DLLEXPORT ze_result_t ZE_APICALL zeGetSamplerProcAddrTable(
    ze_api_version_t version, ze_sampler_dditable_t *pDdiTable) {
  if (pDdiTable == nullptr) {
    return ZE_RESULT_ERROR_INVALID_NULL_POINTER;
  }

  if (auto r = CheckVersion(version); r != ZE_RESULT_SUCCESS) {
    return r;
  }

#define DDI(func) DDIX(Sampler, func)
  DDI(Create);
  DDI(Destroy);
#undef DDI

  return ZE_RESULT_SUCCESS;
}
} // extern "C"
