/*========================== begin_copyright_notice ============================

Copyright (C) 2022 Intel Corporation

SPDX-License-Identifier: MIT

============================= end_copyright_notice ===========================*/

#include <cassert>

#include "device.h"
#include "kernel.h"
#include "module.h"

#include "build_log.h"
#include "context.h"
#include "memory.h"

#include "utils.h"

thread_local std::unordered_map<CmKernelEmu *, shim::ze::Kernel::GroupSize>
    shim::ze::Kernel::group_size_;

extern "C" {
SHIM_EXPORT(zeModuleCreate);
SHIM_EXPORT(zeModuleDestroy);
SHIM_EXPORT(zeModuleDynamicLink);
SHIM_EXPORT(zeModuleBuildLogDestroy);
SHIM_EXPORT(zeModuleBuildLogGetString);
SHIM_EXPORT(zeModuleGetNativeBinary);
SHIM_EXPORT(zeModuleGetGlobalPointer);
SHIM_EXPORT(zeModuleGetKernelNames);
SHIM_EXPORT(zeModuleGetProperties);
SHIM_EXPORT(zeKernelCreate);
SHIM_EXPORT(zeKernelDestroy);
SHIM_EXPORT(zeModuleGetFunctionPointer);
SHIM_EXPORT(zeKernelSetGroupSize);
SHIM_EXPORT(zeKernelSuggestGroupSize);
SHIM_EXPORT(zeKernelSuggestMaxCooperativeGroupCount);
SHIM_EXPORT(zeKernelSetArgumentValue);
SHIM_EXPORT(zeKernelSetIndirectAccess);
SHIM_EXPORT(zeKernelGetIndirectAccess);
SHIM_EXPORT(zeKernelGetSourceAttributes);
SHIM_EXPORT(zeKernelSetCacheConfig);
SHIM_EXPORT(zeKernelGetProperties);
SHIM_EXPORT(zeKernelGetName);

ZE_APIEXPORT ze_result_t ZE_APICALL SHIM_CALL(zeModuleCreate)(
    ze_context_handle_t hContext, ze_device_handle_t hDevice,
    const ze_module_desc_t *desc, ze_module_handle_t *phModule,
    ze_module_build_log_handle_t *phBuildLog) {
  if (hContext == nullptr || hDevice == nullptr) {
    return ZE_RESULT_ERROR_INVALID_NULL_HANDLE;
  }

  shim::IntrusivePtr<shim::ze::Context> ctx(
      reinterpret_cast<shim::ze::Context *>(hContext));
  shim::IntrusivePtr<CmDeviceEmu> dev(reinterpret_cast<CmDeviceEmu *>(hDevice));

  if (desc == nullptr || desc->pInputModule == nullptr || phModule == nullptr) {
    return ZE_RESULT_ERROR_INVALID_NULL_POINTER;
  }

  if (desc->format > ZE_MODULE_FORMAT_NATIVE) {
    return ZE_RESULT_ERROR_INVALID_ENUMERATION;
  }

  if (desc->inputSize == 0) {
    return ZE_RESULT_ERROR_INVALID_SIZE;
  }

  try {
    auto log = shim::MakeIntrusive<shim::ze::BuildLog>();

    if (phBuildLog) {
      shim::IntrusivePtrAddRef(log.get());
      *phBuildLog = reinterpret_cast<ze_module_build_log_handle_t>(log.get());
    }

    if (desc->format == ZE_MODULE_FORMAT_IL_SPIRV) {
      log->str = "SPIRV modules are unsupported";
      return ZE_RESULT_ERROR_MODULE_BUILD_FAILURE;
    }

    CmProgramEmu *prog = nullptr;
    auto r = CmProgramEmu::Create(dev.get(), prog,
                                  const_cast<uint8_t *>(desc->pInputModule),
                                  desc->inputSize);
    switch (r) {
    case CM_SUCCESS:
      break;
    case CM_OUT_OF_HOST_MEMORY:
      return ZE_RESULT_ERROR_OUT_OF_HOST_MEMORY;
    default:
      return ZE_RESULT_ERROR_INVALID_NATIVE_BINARY;
    }

    auto program = shim::IntrusivePtr<CmProgramEmu>(prog, false);
    auto module = shim::MakeIntrusive<shim::ze::Module>(ctx, program);

    shim::IntrusivePtrAddRef(module.get());
    *phModule = reinterpret_cast<ze_module_handle_t>(module.get());
  } catch (std::bad_alloc &e) {
    return ZE_RESULT_ERROR_OUT_OF_HOST_MEMORY;
  }

  return ZE_RESULT_SUCCESS;
}

ZE_APIEXPORT ze_result_t ZE_APICALL
SHIM_CALL(zeModuleDestroy)(ze_module_handle_t hModule) {
  if (hModule == nullptr) {
    return ZE_RESULT_ERROR_INVALID_NULL_HANDLE;
  }

  shim::IntrusivePtr<shim::ze::Module> module(
      reinterpret_cast<shim::ze::Module *>(hModule), false);

  return ZE_RESULT_SUCCESS;
}

ZE_APIEXPORT ze_result_t ZE_APICALL SHIM_CALL(zeModuleDynamicLink)(
    uint32_t numModules, ze_module_handle_t *phModules,
    ze_module_build_log_handle_t *phLinkLog) {
  if (phModules == nullptr) {
    return ZE_RESULT_ERROR_INVALID_NULL_POINTER;
  }

  try {
    auto log = shim::MakeIntrusive<shim::ze::BuildLog>();

    if (phLinkLog) {
      shim::IntrusivePtrAddRef(log.get());
      *phLinkLog = reinterpret_cast<ze_module_build_log_handle_t>(log.get());
    }

    log->str = "Dynamic linking is unsupported";
    return ZE_RESULT_ERROR_MODULE_LINK_FAILURE;
  } catch (std::bad_alloc &e) {
    return ZE_RESULT_ERROR_OUT_OF_HOST_MEMORY;
  }

  return ZE_RESULT_SUCCESS;
}

ZE_APIEXPORT ze_result_t ZE_APICALL SHIM_CALL(zeModuleBuildLogDestroy)(
    ze_module_build_log_handle_t hModuleBuildLog) {
  if (hModuleBuildLog == nullptr) {
    return ZE_RESULT_ERROR_INVALID_NULL_HANDLE;
  }

  shim::IntrusivePtr<shim::ze::BuildLog> log(
      reinterpret_cast<shim::ze::BuildLog *>(hModuleBuildLog), false);

  return ZE_RESULT_SUCCESS;
}

ZE_APIEXPORT ze_result_t ZE_APICALL SHIM_CALL(zeModuleBuildLogGetString)(
    ze_module_build_log_handle_t hModuleBuildLog, size_t *pSize,
    char *pBuildLog) {
  if (hModuleBuildLog == nullptr) {
    return ZE_RESULT_ERROR_INVALID_NULL_HANDLE;
  }

  if (pSize == nullptr) {
    return ZE_RESULT_ERROR_INVALID_NULL_POINTER;
  }

  shim::IntrusivePtr<shim::ze::BuildLog> log(
      reinterpret_cast<shim::ze::BuildLog *>(hModuleBuildLog));

  if (*pSize == 0) {
    *pSize = log->str.length() + 1;
  }

  if (pBuildLog != nullptr) {
    auto n = std::min(log->str.length(), *pSize - 1);
    std::copy_n(log->str.begin(), n, pBuildLog);
    pBuildLog[n] = '\0';
  }

  return ZE_RESULT_SUCCESS;
}

ZE_APIEXPORT ze_result_t ZE_APICALL SHIM_CALL(zeModuleGetNativeBinary)(
    ze_module_handle_t hModule, size_t *pSize, uint8_t *pModuleNativeBinary) {
  if (hModule == nullptr) {
    return ZE_RESULT_ERROR_INVALID_NULL_HANDLE;
  }

  if (pSize == nullptr) {
    return ZE_RESULT_ERROR_INVALID_NULL_POINTER;
  }

  shim::IntrusivePtr<shim::ze::Module> module(
      reinterpret_cast<shim::ze::Module *>(hModule));

  // Cannot get a binary in current implementation
  return ZE_RESULT_ERROR_UNSUPPORTED_FEATURE;
}

ZE_APIEXPORT ze_result_t ZE_APICALL SHIM_CALL(zeModuleGetGlobalPointer)(
    ze_module_handle_t hModule, const char *pGlobalName, size_t *pSize,
    void **pptr) {
  if (hModule == nullptr) {
    return ZE_RESULT_ERROR_INVALID_NULL_HANDLE;
  }

  if (pGlobalName == nullptr) {
    return ZE_RESULT_ERROR_INVALID_NULL_POINTER;
  }

  shim::IntrusivePtr<shim::ze::Module> module(
      reinterpret_cast<shim::ze::Module *>(hModule));

  // CM does not support global variables so far
  return ZE_RESULT_ERROR_INVALID_GLOBAL_NAME;
}

ZE_APIEXPORT ze_result_t ZE_APICALL SHIM_CALL(zeModuleGetKernelNames)(
    ze_module_handle_t hModule, uint32_t *pCount, const char **pNames) {
  if (hModule == nullptr) {
    return ZE_RESULT_ERROR_INVALID_NULL_HANDLE;
  }

  if (pCount == nullptr) {
    return ZE_RESULT_ERROR_INVALID_NULL_POINTER;
  }

  shim::IntrusivePtr<shim::ze::Module> module(
      reinterpret_cast<shim::ze::Module *>(hModule));

  // CM EMU does not support kernels enumeration
  return ZE_RESULT_ERROR_UNSUPPORTED_FEATURE;
}

ZE_APIEXPORT ze_result_t ZE_APICALL SHIM_CALL(zeModuleGetProperties)(
    ze_module_handle_t hModule, ze_module_properties_t *pModuleProperties) {
  if (hModule == nullptr) {
    return ZE_RESULT_ERROR_INVALID_NULL_HANDLE;
  }

  if (pModuleProperties == nullptr) {
    return ZE_RESULT_ERROR_INVALID_NULL_POINTER;
  }

  shim::IntrusivePtr<shim::ze::Module> module(
      reinterpret_cast<shim::ze::Module *>(hModule));

  // Dynamic Linking is not supported
  pModuleProperties->flags = 0;

  return ZE_RESULT_SUCCESS;
}

ZE_APIEXPORT ze_result_t ZE_APICALL SHIM_CALL(zeKernelCreate)(
    ze_module_handle_t hModule, const ze_kernel_desc_t *desc,
    ze_kernel_handle_t *phKernel) {
  if (hModule == nullptr) {
    return ZE_RESULT_ERROR_INVALID_NULL_HANDLE;
  }

  if (desc == nullptr || desc->pKernelName == nullptr || phKernel == nullptr) {
    return ZE_RESULT_ERROR_INVALID_NULL_POINTER;
  }

  if (desc->flags > 0x3) {
    return ZE_RESULT_ERROR_INVALID_ENUMERATION;
  }

  shim::IntrusivePtr<shim::ze::Module> module(
      reinterpret_cast<shim::ze::Module *>(hModule));
  auto dev = module->ctx_->dev_;

  CmKernel *k = nullptr;

  auto r =
      dev->CreateKernel(module->module_.get(), desc->pKernelName, k, nullptr);
  if (r != CM_SUCCESS) {
    return ZE_RESULT_ERROR_INVALID_KERNEL_NAME;
  }

  shim::IntrusivePtr<CmKernelEmu> kptr(dynamic_cast<CmKernelEmu *>(k), false);

  try {
    auto kernel = shim::MakeIntrusive<shim::ze::Kernel>(module->ctx_, kptr);
    shim::IntrusivePtrAddRef(kernel.get());
    *phKernel = reinterpret_cast<ze_kernel_handle_t>(kernel.get());
  } catch (std::bad_alloc &e) {
    return ZE_RESULT_ERROR_OUT_OF_HOST_MEMORY;
  }

  return ZE_RESULT_SUCCESS;
}

ZE_APIEXPORT ze_result_t ZE_APICALL
SHIM_CALL(zeKernelDestroy)(ze_kernel_handle_t hKernel) {
  if (hKernel == nullptr) {
    return ZE_RESULT_ERROR_INVALID_NULL_HANDLE;
  }

  shim::IntrusivePtr<shim::ze::Kernel> kernel(
      reinterpret_cast<shim::ze::Kernel *>(hKernel), false);

  if (kernel->UseCount() == 1) {
    shim::ze::Kernel::group_size_.erase(kernel->kernel_.get());
  }

  return ZE_RESULT_SUCCESS;
}

ZE_APIEXPORT ze_result_t ZE_APICALL SHIM_CALL(zeModuleGetFunctionPointer)(
    ze_module_handle_t hModule, const char *pFunctionName, void **pfnFunction) {
  if (hModule == nullptr) {
    return ZE_RESULT_ERROR_INVALID_NULL_HANDLE;
  }

  if (pFunctionName == nullptr || pfnFunction == nullptr) {
    return ZE_RESULT_ERROR_INVALID_NULL_POINTER;
  }

  shim::IntrusivePtr<shim::ze::Module> module(
      reinterpret_cast<shim::ze::Module *>(hModule));

  // This feature is not implemented
  return ZE_RESULT_ERROR_INVALID_FUNCTION_NAME;
}

ZE_APIEXPORT ze_result_t ZE_APICALL
SHIM_CALL(zeKernelSetGroupSize)(ze_kernel_handle_t hKernel, uint32_t groupSizeX,
                                uint32_t groupSizeY, uint32_t groupSizeZ) {
  if (hKernel == nullptr) {
    return ZE_RESULT_ERROR_INVALID_NULL_HANDLE;
  }

  shim::IntrusivePtr<shim::ze::Kernel> kernel(
      reinterpret_cast<shim::ze::Kernel *>(hKernel));
  shim::ze::Kernel::group_size_[kernel->kernel_.get()] = {
      groupSizeX, groupSizeY, groupSizeZ};

  return ZE_RESULT_SUCCESS;
}

ZE_APIEXPORT ze_result_t ZE_APICALL SHIM_CALL(zeKernelSuggestGroupSize)(
    ze_kernel_handle_t hKernel, uint32_t globalSizeX, uint32_t globalSizeY,
    uint32_t globalSizeZ, uint32_t *groupSizeX, uint32_t *groupSizeY,
    uint32_t *groupSizeZ) {
  if (hKernel == nullptr) {
    return ZE_RESULT_ERROR_INVALID_NULL_HANDLE;
  }

  shim::IntrusivePtr<shim::ze::Kernel> kernel(
      reinterpret_cast<shim::ze::Kernel *>(hKernel));

  // Suggestion is not supported
  return ZE_RESULT_ERROR_INVALID_GLOBAL_WIDTH_DIMENSION;
}

ZE_APIEXPORT ze_result_t ZE_APICALL
SHIM_CALL(zeKernelSuggestMaxCooperativeGroupCount)(ze_kernel_handle_t hKernel,
                                                   uint32_t *totalGroupCount) {
  if (hKernel == nullptr) {
    return ZE_RESULT_ERROR_INVALID_NULL_HANDLE;
  }

  shim::IntrusivePtr<shim::ze::Kernel> kernel(
      reinterpret_cast<shim::ze::Kernel *>(hKernel));

  // Suggestion is not supported
  return ZE_RESULT_ERROR_UNSUPPORTED_FEATURE;
}

ZE_APIEXPORT ze_result_t ZE_APICALL SHIM_CALL(zeKernelSetArgumentValue)(
    ze_kernel_handle_t hKernel, uint32_t argIndex, size_t argSize,
    const void *pArgValue) {
  if (hKernel == nullptr) {
    return ZE_RESULT_ERROR_INVALID_NULL_HANDLE;
  }

  shim::IntrusivePtr<shim::ze::Kernel> kernel(
      reinterpret_cast<shim::ze::Kernel *>(hKernel));

  auto fdesc = kernel->kernel_->GetFunctionDesc();

  if (argIndex >= fdesc.params.size()) {
    return ZE_RESULT_ERROR_INVALID_KERNEL_ARGUMENT_INDEX;
  }

  const auto &argdesc = fdesc.params[argIndex];

  if (argdesc.isClass) {
    if (pArgValue == nullptr) {
      return ZE_RESULT_ERROR_INVALID_ARGUMENT;
    }

    if (argdesc.typeName == "SurfaceIndex" || // Workaround for kernel<half>
        (argdesc.typeName == "" && argSize == sizeof(void *))) {
      if (sizeof(void *) != argSize) {
        return ZE_RESULT_ERROR_INVALID_KERNEL_ARGUMENT_SIZE;
      }

      void *ptr = nullptr;
      std::memcpy(&ptr, pArgValue, sizeof(ptr));

      if (ptr == nullptr) {
        return ZE_RESULT_ERROR_INVALID_ARGUMENT;
      }

      auto &mm = kernel->ctx_->mm_;

      if (auto *buffer = mm.GetIndex(ptr); buffer) {
        auto r = kernel->kernel_->SetKernelArg(argIndex, sizeof(SurfaceIndex),
                                               buffer);
        if (r != CM_SUCCESS) {
          return ZE_RESULT_ERROR_INVALID_KERNEL_ARGUMENT_SIZE;
        }
      } else { // Image
        shim::IntrusivePtr<shim::ze::Image> image(
            reinterpret_cast<shim::ze::Image *>(ptr));
        kernel->images_.push_back(image);

        SurfaceIndex *surf = image->GetIndex();

        auto r =
            kernel->kernel_->SetKernelArg(argIndex, sizeof(SurfaceIndex), surf);
        if (r != CM_SUCCESS) {
          return ZE_RESULT_ERROR_INVALID_KERNEL_ARGUMENT_SIZE;
        }
      }
    } else if (argdesc.typeName == "SamplerIndex") {
      GFX_EMU_WARNING_MESSAGE(fShim, "sampler support is not implemented\n");
      return ZE_RESULT_ERROR_UNSUPPORTED_FEATURE;
    } else {
      GFX_EMU_WARNING_MESSAGE(fShim, "unknown kernel argument #%u type: '%s'\n",
                              argIndex, argdesc.typeName.c_str());
      return ZE_RESULT_ERROR_INVALID_KERNEL_ARGUMENT_SIZE;
    }
  } else { // pointer, scalar, vector or matrix
    if (argdesc.size != 0 && argdesc.size != argSize) {
      return ZE_RESULT_ERROR_INVALID_KERNEL_ARGUMENT_SIZE;
    }

    auto r = kernel->kernel_->SetKernelArg(argIndex, argSize, pArgValue);

    if (r != CM_SUCCESS) {
      return ZE_RESULT_ERROR_INVALID_KERNEL_ARGUMENT_SIZE;
    }
  }

  return ZE_RESULT_SUCCESS;
}

ZE_APIEXPORT ze_result_t ZE_APICALL SHIM_CALL(zeKernelSetIndirectAccess)(
    ze_kernel_handle_t hKernel, ze_kernel_indirect_access_flags_t flags) {
  if (hKernel == nullptr) {
    return ZE_RESULT_ERROR_INVALID_NULL_HANDLE;
  }

  if (flags > 0x7) {
    return ZE_RESULT_ERROR_INVALID_ENUMERATION;
  }

  shim::IntrusivePtr<shim::ze::Kernel> kernel(
      reinterpret_cast<shim::ze::Kernel *>(hKernel));

  kernel->indirect_access_flags_ = flags;

  return ZE_RESULT_SUCCESS;
}

ZE_APIEXPORT ze_result_t ZE_APICALL SHIM_CALL(zeKernelGetIndirectAccess)(
    ze_kernel_handle_t hKernel, ze_kernel_indirect_access_flags_t *pFlags) {
  if (hKernel == nullptr) {
    return ZE_RESULT_ERROR_INVALID_NULL_HANDLE;
  }

  if (pFlags == nullptr) {
    return ZE_RESULT_ERROR_INVALID_NULL_POINTER;
  }

  shim::IntrusivePtr<shim::ze::Kernel> kernel(
      reinterpret_cast<shim::ze::Kernel *>(hKernel));

  *pFlags = kernel->indirect_access_flags_;

  return ZE_RESULT_SUCCESS;
}

ZE_APIEXPORT ze_result_t ZE_APICALL SHIM_CALL(zeKernelGetSourceAttributes)(
    ze_kernel_handle_t hKernel, uint32_t *pSize, char **pString) {
  static const char empty[] = "";

  if (hKernel == nullptr) {
    return ZE_RESULT_ERROR_INVALID_NULL_HANDLE;
  }

  if (pSize == nullptr || pString == nullptr) {
    return ZE_RESULT_ERROR_INVALID_NULL_POINTER;
  }

  *pSize = sizeof(empty);
  *pString = const_cast<char *>(empty);

  return ZE_RESULT_SUCCESS;
}

ZE_APIEXPORT ze_result_t ZE_APICALL SHIM_CALL(zeKernelSetCacheConfig)(
    ze_kernel_handle_t hKernel, ze_cache_config_flags_t flags) {
  if (hKernel == nullptr) {
    return ZE_RESULT_ERROR_INVALID_NULL_HANDLE;
  }

  if (flags > 0x3) {
    return ZE_RESULT_ERROR_INVALID_ENUMERATION;
  }

  return ZE_RESULT_ERROR_UNSUPPORTED_FEATURE;
}

ZE_APIEXPORT ze_result_t ZE_APICALL SHIM_CALL(zeKernelGetProperties)(
    ze_kernel_handle_t hKernel, ze_kernel_properties_t *pKernelProperties) {
  if (hKernel == nullptr) {
    return ZE_RESULT_ERROR_INVALID_NULL_HANDLE;
  }

  if (pKernelProperties == nullptr) {
    return ZE_RESULT_ERROR_INVALID_NULL_POINTER;
  }

  shim::IntrusivePtr<shim::ze::Kernel> kernel(
      reinterpret_cast<shim::ze::Kernel *>(hKernel));
  auto fdesc = kernel->kernel_->GetFunctionDesc();

  pKernelProperties->numKernelArgs = fdesc.params.size();
  pKernelProperties->requiredGroupSizeX = 0;
  pKernelProperties->requiredGroupSizeY = 0;
  pKernelProperties->requiredGroupSizeZ = 0;
  pKernelProperties->requiredNumSubGroups = 0;
  pKernelProperties->requiredSubgroupSize = 0;
  pKernelProperties->maxSubgroupSize = 32;
  pKernelProperties->localMemSize =
      0; // We cannot detect SLM size before a kernel execution
  pKernelProperties->privateMemSize = 0;
  kernel->kernel_->QuerySpillSize(pKernelProperties->spillMemSize);
  pKernelProperties->uuid = {{0xff}, {0xff}};

  return ZE_RESULT_SUCCESS;
}

ZE_APIEXPORT ze_result_t ZE_APICALL SHIM_CALL(zeKernelGetName)(
    ze_kernel_handle_t hKernel, size_t *pSize, char *pName) {
  if (hKernel == nullptr) {
    return ZE_RESULT_ERROR_INVALID_NULL_HANDLE;
  }

  if (pSize == nullptr) {
    return ZE_RESULT_ERROR_INVALID_NULL_POINTER;
  }

  shim::IntrusivePtr<shim::ze::Kernel> kernel(
      reinterpret_cast<shim::ze::Kernel *>(hKernel));
  std::string name = kernel->kernel_->GetName();

  if (*pSize == 0) {
    *pSize = name.length() + 1;
  }

  if (pName != nullptr) {
    auto n = std::min(name.length(), *pSize - 1);
    std::copy_n(std::begin(name), n, pName);
    pName[n] = '\0';
  }

  return ZE_RESULT_SUCCESS;
}

} // extern "C"
