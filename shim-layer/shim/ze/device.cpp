/*========================== begin_copyright_notice ============================

Copyright (C) 2022 Intel Corporation

SPDX-License-Identifier: MIT

============================= end_copyright_notice ===========================*/

#include <memory>

#include "emu_cfg.h"

#include "device.h"
#include "driver.h"

#include "intrusive_pointer.h"

namespace {
shim::IntrusivePtr<CmDeviceEmu> GetDevice() {
  CmDevice *dev = nullptr;
  unsigned version = 0;

  shim::IntrusivePtr<CmDeviceEmu> device = nullptr;

  if (auto r = ::CreateCmDevice(dev, version);
      r == CM_SUCCESS && version >= CM_1_0) {
    device.reset(dynamic_cast<CmDeviceEmu*>(dev), false);
  }

  return device;
}
} // namespace

extern "C" {
SHIM_EXPORT(zeDeviceGet);
SHIM_EXPORT(zeDeviceGetSubDevices);
SHIM_EXPORT(zeDeviceGetProperties);
SHIM_EXPORT(zeDeviceGetComputeProperties);
SHIM_EXPORT(zeDeviceGetModuleProperties);
SHIM_EXPORT(zeDeviceGetCommandQueueGroupProperties);
SHIM_EXPORT(zeDeviceGetMemoryProperties);
SHIM_EXPORT(zeDeviceGetMemoryAccessProperties);
SHIM_EXPORT(zeDeviceGetCacheProperties);
SHIM_EXPORT(zeDeviceGetImageProperties);
SHIM_EXPORT(zeDeviceGetExternalMemoryProperties);
SHIM_EXPORT(zeDeviceGetP2PProperties);
SHIM_EXPORT(zeDeviceCanAccessPeer);
SHIM_EXPORT(zeDeviceGetStatus);

ZE_APIEXPORT ze_result_t ZE_APICALL SHIM_CALL(zeDeviceGet)(
    ze_driver_handle_t hDriver, uint32_t *pCount, ze_device_handle_t *phDevices) {
  static shim::IntrusivePtr<CmDeviceEmu> dev = GetDevice();

  if (hDriver == nullptr) {
    return ZE_RESULT_ERROR_INVALID_NULL_HANDLE;
  }

  if (pCount == nullptr) {
    return ZE_RESULT_ERROR_INVALID_NULL_POINTER;
  }

  if (!dev) {
    return ZE_RESULT_ERROR_DEVICE_LOST;
  }

  if (*pCount == 0) {
    *pCount = 1;
  }

  if (phDevices != nullptr) {
    shim::IntrusivePtrAddRef(dev.get());
    *phDevices = reinterpret_cast<ze_device_handle_t>(dev.get());
  }

  return ZE_RESULT_SUCCESS;
}

ZE_APIEXPORT ze_result_t ZE_APICALL SHIM_CALL(zeDeviceGetSubDevices)(
    ze_device_handle_t hDevice,
    uint32_t *pCount,
    ze_device_handle_t *phSubdevices) {
  if (hDevice == nullptr) {
    return ZE_RESULT_ERROR_INVALID_NULL_HANDLE;
  }

  if (pCount == nullptr) {
    return ZE_RESULT_ERROR_INVALID_NULL_POINTER;
  }

  shim::IntrusivePtr<CmDeviceEmu> dev(reinterpret_cast<CmDeviceEmu*>(hDevice));
  if (*pCount == 0) {
    *pCount = 0;
  }

  return ZE_RESULT_SUCCESS;
}

ZE_APIEXPORT ze_result_t ZE_APICALL SHIM_CALL(zeDeviceGetProperties)(
    ze_device_handle_t hDevice,
    ze_device_properties_t *pDeviceProperties) {
  if (hDevice == nullptr) {
    return ZE_RESULT_ERROR_INVALID_NULL_HANDLE;
  }

  if (pDeviceProperties == nullptr) {
    return ZE_RESULT_ERROR_INVALID_NULL_POINTER;
  }

  shim::IntrusivePtr<CmDeviceEmu> dev(reinterpret_cast<CmDeviceEmu*>(hDevice));

  const auto platform = GfxEmu::Cfg::Platform().getInt<GfxEmu::Platform::Id>();
  const auto sku = GfxEmu::Cfg::Sku().getInt<GfxEmu::Platform::Sku::Id>();
  const auto &cfg = GfxEmu::Cfg::getPlatformConfig(platform);

  pDeviceProperties->type = ZE_DEVICE_TYPE_GPU;
  pDeviceProperties->vendorId = 0x8086;
  pDeviceProperties->deviceId = 0;
  pDeviceProperties->flags = 0;

  pDeviceProperties->subdeviceId = 0;

  pDeviceProperties->coreClockRate = 1000000;
  pDeviceProperties->maxMemAllocSize = 0;

  pDeviceProperties->maxHardwareContexts = 1;
  pDeviceProperties->maxCommandQueuePriority = 1;

  pDeviceProperties->numThreadsPerEU = cfg.getThreadsPerEu(sku);
  pDeviceProperties->physicalEUSimdWidth = cfg.hwSimd;
  pDeviceProperties->numEUsPerSubslice = cfg.getEuPerSubslice(sku);
  pDeviceProperties->numSubslicesPerSlice = 1;
  pDeviceProperties->numSlices = 1;

  pDeviceProperties->timerResolution = 1000;
  pDeviceProperties->timestampValidBits = 60;
  pDeviceProperties->kernelTimestampValidBits = 60;

  pDeviceProperties->uuid = {0xff};

  std::strncpy(pDeviceProperties->name, cfg.name.c_str(), ZE_MAX_DEVICE_NAME);

  return ZE_RESULT_SUCCESS;
}

ZE_APIEXPORT ze_result_t ZE_APICALL SHIM_CALL(zeDeviceGetComputeProperties)(
    ze_device_handle_t hDevice,
    ze_device_compute_properties_t *pComputeProperties) {
  if (hDevice == nullptr) {
    return ZE_RESULT_ERROR_INVALID_NULL_HANDLE;
  }

  if (pComputeProperties == nullptr) {
    return ZE_RESULT_ERROR_INVALID_NULL_POINTER;
  }

  shim::IntrusivePtr<CmDeviceEmu> dev(reinterpret_cast<CmDeviceEmu*>(hDevice));

  const auto platform = GfxEmu::Cfg::Platform().getInt<GfxEmu::Platform::Id>();
  const auto sku = GfxEmu::Cfg::Sku().getInt<GfxEmu::Platform::Sku::Id>();
  const auto &cfg = GfxEmu::Cfg::getPlatformConfig(platform);
  const unsigned group_size = 256;
  const unsigned group_count = 0xffffffffu;

  pComputeProperties->maxTotalGroupSize = group_size;
  pComputeProperties->maxGroupSizeX = group_size;
  pComputeProperties->maxGroupSizeY = group_size;
  pComputeProperties->maxGroupSizeZ = group_size;

  pComputeProperties->maxGroupCountX = group_count;
  pComputeProperties->maxGroupCountY = group_count;
  pComputeProperties->maxGroupCountZ = group_count;
  pComputeProperties->maxSharedLocalMemory = 1u << 16;

  size_t i = pComputeProperties->numSubGroupSizes = 0;

  for (auto simd = cfg.hwSimd; simd <= 32; simd *= 2) {
    pComputeProperties->subGroupSizes[i++] = simd;
  }

  return ZE_RESULT_SUCCESS;
}

ZE_APIEXPORT ze_result_t ZE_APICALL SHIM_CALL(zeDeviceGetModuleProperties)(
    ze_device_handle_t hDevice,
    ze_device_module_properties_t *pModuleProperties) {
  if (hDevice == nullptr) {
    return ZE_RESULT_ERROR_INVALID_NULL_HANDLE;
  }

  if (pModuleProperties == nullptr) {
    return ZE_RESULT_ERROR_INVALID_NULL_POINTER;
  }

  shim::IntrusivePtr<CmDeviceEmu> dev(reinterpret_cast<CmDeviceEmu*>(hDevice));

  const auto platform = GfxEmu::Cfg::Platform().getInt<GfxEmu::Platform::Id>();
  const auto sku = GfxEmu::Cfg::Sku().getInt<GfxEmu::Platform::Sku::Id>();
  const auto &cfg = GfxEmu::Cfg::getPlatformConfig(platform);

  // SPIR-V will be never supported by EMU
  pModuleProperties->spirvVersionSupported = 0;

  pModuleProperties->flags = ZE_DEVICE_MODULE_FLAG_FP16
                           | ZE_DEVICE_MODULE_FLAG_INT64_ATOMICS;

  if (cfg.flags & GfxEmu::Cfg::PlatformFlags::Dp4a) {
    pModuleProperties->flags |= ZE_DEVICE_MODULE_FLAG_DP4A;
  }

  constexpr uint32_t fpflags = ZE_DEVICE_FP_FLAG_DENORM
                             | ZE_DEVICE_FP_FLAG_INF_NAN
                             | ZE_DEVICE_FP_FLAG_ROUND_TO_NEAREST
                             | ZE_DEVICE_FP_FLAG_ROUND_TO_ZERO
                             | ZE_DEVICE_FP_FLAG_ROUND_TO_INF
                             | ZE_DEVICE_FP_FLAG_FMA;

  pModuleProperties->fp16flags = fpflags;
  pModuleProperties->fp32flags = fpflags;

  if (cfg.flags & GfxEmu::Cfg::PlatformFlags::IeeeDivSqrt) {
    pModuleProperties->fp32flags |= ZE_DEVICE_FP_FLAG_ROUNDED_DIVIDE_SQRT;
  }

  if (cfg.flags & GfxEmu::Cfg::PlatformFlags::Fp64) {
    pModuleProperties->flags |= ZE_DEVICE_MODULE_FLAG_FP64;
    pModuleProperties->fp64flags = fpflags;

    if (cfg.flags & GfxEmu::Cfg::PlatformFlags::IeeeDivSqrt) {
      pModuleProperties->fp64flags |= ZE_DEVICE_FP_FLAG_ROUNDED_DIVIDE_SQRT;
    }
  }
  pModuleProperties->maxArgumentsSize = 1024;
  pModuleProperties->printfBufferSize = 4 << 20;
  pModuleProperties->nativeKernelSupported = {{0xff}};

  return ZE_RESULT_SUCCESS;
}

ZE_APIEXPORT ze_result_t ZE_APICALL SHIM_CALL(zeDeviceGetCommandQueueGroupProperties)(
    ze_device_handle_t hDevice, uint32_t *pCount,
    ze_command_queue_group_properties_t *pCommandQueueGroupProperties) {
  if (hDevice == nullptr) {
    return ZE_RESULT_ERROR_INVALID_NULL_HANDLE;
  }

  if (pCount == nullptr) {
    return ZE_RESULT_ERROR_INVALID_NULL_POINTER;
  }

  if (*pCount == 0) {
    *pCount = 1;
  }

  if (pCommandQueueGroupProperties != nullptr) {
    pCommandQueueGroupProperties[0].flags = ZE_COMMAND_QUEUE_GROUP_PROPERTY_FLAG_COMPUTE
        | ZE_COMMAND_QUEUE_GROUP_PROPERTY_FLAG_COPY
        | ZE_COMMAND_QUEUE_GROUP_PROPERTY_FLAG_METRICS;
    pCommandQueueGroupProperties[0].maxMemoryFillPatternSize = 1;
    pCommandQueueGroupProperties[0].numQueues = 1;
  }

  return ZE_RESULT_SUCCESS;
}

ZE_APIEXPORT ze_result_t ZE_APICALL SHIM_CALL(zeDeviceGetMemoryProperties)(
    ze_device_handle_t hDevice, uint32_t *pCount,
    ze_device_memory_properties_t *pMemProperties) {
  if (hDevice == nullptr) {
    return ZE_RESULT_ERROR_INVALID_NULL_HANDLE;
  }

  if (pCount == nullptr) {
    return ZE_RESULT_ERROR_INVALID_NULL_POINTER;
  }

  if (*pCount == 0) {
    *pCount = 1;
  }

  if (pMemProperties != nullptr) {
    pMemProperties[0].flags = 0;
    pMemProperties[0].maxClockRate = 1 << 20;
    pMemProperties[0].maxBusWidth = 64;
    pMemProperties[0].totalSize = 1ull << 48;
    std::strncpy(pMemProperties[0].name, "CM EMU host memory", ZE_MAX_DEVICE_NAME);
  }

  return ZE_RESULT_SUCCESS;
}

ZE_APIEXPORT ze_result_t ZE_APICALL SHIM_CALL(zeDeviceGetMemoryAccessProperties)(
    ze_device_handle_t hDevice,
    ze_device_memory_access_properties_t *pMemAccessProperties) {
  if (hDevice == nullptr) {
    return ZE_RESULT_ERROR_INVALID_NULL_HANDLE;
  }

  if (pMemAccessProperties == nullptr) {
    return ZE_RESULT_ERROR_INVALID_NULL_POINTER;
  }

  auto caps = ZE_MEMORY_ACCESS_CAP_FLAG_RW
      | ZE_MEMORY_ACCESS_CAP_FLAG_ATOMIC
      | ZE_MEMORY_ACCESS_CAP_FLAG_CONCURRENT
      | ZE_MEMORY_ACCESS_CAP_FLAG_CONCURRENT_ATOMIC;

  pMemAccessProperties->hostAllocCapabilities = caps;
  pMemAccessProperties->deviceAllocCapabilities = caps;
  pMemAccessProperties->sharedSingleDeviceAllocCapabilities = caps;
  pMemAccessProperties->sharedCrossDeviceAllocCapabilities = 0;
  pMemAccessProperties->sharedSystemAllocCapabilities = ZE_MEMORY_ACCESS_CAP_FLAG_RW;

  return ZE_RESULT_SUCCESS;
}

ZE_APIEXPORT ze_result_t ZE_APICALL SHIM_CALL(zeDeviceGetCacheProperties)(
    ze_device_handle_t hDevice, uint32_t *pCount,
    ze_device_cache_properties_t *pCacheProperties) {
  if (hDevice == nullptr) {
    return ZE_RESULT_ERROR_INVALID_NULL_HANDLE;
  }

  if (pCount == nullptr) {
    return ZE_RESULT_ERROR_INVALID_NULL_POINTER;
  }

  *pCount = 0;

  return ZE_RESULT_SUCCESS;
}

ZE_APIEXPORT ze_result_t ZE_APICALL SHIM_CALL(zeDeviceGetImageProperties)(
    ze_device_handle_t hDevice,
    ze_device_image_properties_t *pImageProperties) {
  if (hDevice == nullptr) {
    return ZE_RESULT_ERROR_INVALID_NULL_HANDLE;
  }

  if (pImageProperties == nullptr) {
    return ZE_RESULT_ERROR_INVALID_NULL_POINTER;
  }

  return ZE_RESULT_ERROR_UNSUPPORTED_FEATURE;
}

ZE_APIEXPORT ze_result_t ZE_APICALL SHIM_CALL(zeDeviceGetExternalMemoryProperties)(
    ze_device_handle_t hDevice,
    ze_device_external_memory_properties_t *pExternalMemoryProperties) {
  if (hDevice == nullptr) {
    return ZE_RESULT_ERROR_INVALID_NULL_HANDLE;
  }

  if (pExternalMemoryProperties == nullptr) {
    return ZE_RESULT_ERROR_INVALID_NULL_POINTER;
  }

  pExternalMemoryProperties->memoryAllocationImportTypes = 0;
  pExternalMemoryProperties->memoryAllocationExportTypes = 0;
  pExternalMemoryProperties->imageImportTypes = 0;
  pExternalMemoryProperties->imageExportTypes = 0;

  return ZE_RESULT_SUCCESS;
}

ZE_APIEXPORT ze_result_t ZE_APICALL SHIM_CALL(zeDeviceGetP2PProperties)(
    ze_device_handle_t hDevice,
    ze_device_handle_t hPeerDevice,
    ze_device_p2p_properties_t *pP2PProperties) {
  if (hDevice == nullptr || hPeerDevice == nullptr) {
    return ZE_RESULT_ERROR_INVALID_NULL_HANDLE;
  }

  if (pP2PProperties == nullptr) {
    return ZE_RESULT_ERROR_INVALID_NULL_POINTER;
  }

  if (hDevice == hPeerDevice) {
    pP2PProperties->flags = ZE_DEVICE_P2P_PROPERTY_FLAG_ACCESS;
  } else {
    pP2PProperties->flags = 0;
  }

  return ZE_RESULT_SUCCESS;
}

ZE_APIEXPORT ze_result_t ZE_APICALL SHIM_CALL(zeDeviceCanAccessPeer)(
    ze_device_handle_t hDevice,
    ze_device_handle_t hPeerDevice,
    ze_bool_t *value) {
  if (hDevice == nullptr || hPeerDevice == nullptr) {
    return ZE_RESULT_ERROR_INVALID_NULL_HANDLE;
  }

  if (value == nullptr) {
    return ZE_RESULT_ERROR_INVALID_NULL_POINTER;
  }

  if (hDevice == hPeerDevice) {
    *value = true;
  } else {
    *value = false;
  }

  return ZE_RESULT_SUCCESS;
}

ZE_APIEXPORT ze_result_t ZE_APICALL SHIM_CALL(zeDeviceGetStatus)(
    ze_device_handle_t hDevice) {
  if (hDevice == nullptr) {
    return ZE_RESULT_ERROR_INVALID_NULL_HANDLE;
  }

  return ZE_RESULT_SUCCESS;
}
}
