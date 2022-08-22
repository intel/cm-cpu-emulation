/*========================== begin_copyright_notice ============================

Copyright (C) 2022 Intel Corporation

SPDX-License-Identifier: MIT

============================= end_copyright_notice ===========================*/

#include "cmdlist.h"

ze_result_t shim::ze::CommandList::Append(const shim::ze::Command &cmd) {
  if (queue_) {
    return queue_->Execute(cmd);
  }

  list_.push_back(cmd);
  return ZE_RESULT_SUCCESS;
}

extern "C" {
SHIM_EXPORT(zeCommandListCreate);
SHIM_EXPORT(zeCommandListCreateImmediate);
SHIM_EXPORT(zeCommandListDestroy);
SHIM_EXPORT(zeCommandListClose);
SHIM_EXPORT(zeCommandListReset);
SHIM_EXPORT(zeCommandListAppendWriteGlobalTimestamp);
SHIM_EXPORT(zeCommandListAppendMemoryCopy);
SHIM_EXPORT(zeCommandListAppendMemoryFill);
SHIM_EXPORT(zeCommandListAppendMemoryCopyRegion);
SHIM_EXPORT(zeCommandListAppendMemoryCopyFromContext);
SHIM_EXPORT(zeCommandListAppendImageCopy);
SHIM_EXPORT(zeCommandListAppendImageCopyRegion);
SHIM_EXPORT(zeCommandListAppendImageCopyToMemory);
SHIM_EXPORT(zeCommandListAppendImageCopyFromMemory);
SHIM_EXPORT(zeCommandListAppendMemoryPrefetch);
SHIM_EXPORT(zeCommandListAppendMemAdvise);
SHIM_EXPORT(zeCommandListAppendBarrier);
SHIM_EXPORT(zeCommandListAppendMemoryRangesBarrier);
SHIM_EXPORT(zeCommandListAppendLaunchKernel);
SHIM_EXPORT(zeCommandListAppendLaunchCooperativeKernel);
SHIM_EXPORT(zeCommandListAppendLaunchKernelIndirect);
SHIM_EXPORT(zeCommandListAppendLaunchMultipleKernelsIndirect);
SHIM_EXPORT(zeCommandListAppendSignalEvent);
SHIM_EXPORT(zeCommandListAppendWaitOnEvents);
SHIM_EXPORT(zeCommandListAppendEventReset);
SHIM_EXPORT(zeCommandListAppendQueryKernelTimestamps);
} // extern "C"

ZE_APIEXPORT ze_result_t ZE_APICALL SHIM_CALL(zeCommandListCreate)(
    ze_context_handle_t hContext, ze_device_handle_t hDevice,
    const ze_command_list_desc_t *desc,
    ze_command_list_handle_t *phCommandList) {
  if (hContext == nullptr || hDevice == nullptr) {
    return ZE_RESULT_ERROR_INVALID_NULL_HANDLE;
  }

  if (desc == nullptr || phCommandList == nullptr) {
    return ZE_RESULT_ERROR_INVALID_NULL_POINTER;
  }

  if (desc->flags > 0x7) {
    return ZE_RESULT_ERROR_INVALID_ENUMERATION;
  }

  shim::IntrusivePtr<shim::ze::Context> ctx(
      reinterpret_cast<shim::ze::Context *>(hContext));
  shim::IntrusivePtr<CmDeviceEmu> dev(reinterpret_cast<CmDeviceEmu *>(hDevice));

  try {
    auto list = shim::MakeIntrusive<shim::ze::CommandList>(ctx);
    shim::IntrusivePtrAddRef(list.get());
    *phCommandList = reinterpret_cast<ze_command_list_handle_t>(list.get());
  } catch (std::bad_alloc &e) {
    return ZE_RESULT_ERROR_OUT_OF_HOST_MEMORY;
  }

  return ZE_RESULT_SUCCESS;
}

ZE_APIEXPORT ze_result_t ZE_APICALL SHIM_CALL(zeCommandListCreateImmediate)(
    ze_context_handle_t hContext, ze_device_handle_t hDevice,
    const ze_command_queue_desc_t *altdesc,
    ze_command_list_handle_t *phCommandList) {
  if (hContext == nullptr || hDevice == nullptr) {
    return ZE_RESULT_ERROR_INVALID_NULL_HANDLE;
  }

  if (altdesc == nullptr || phCommandList == nullptr) {
    return ZE_RESULT_ERROR_INVALID_NULL_POINTER;
  }

  if (altdesc->flags > 0x1 ||
      altdesc->mode > ZE_COMMAND_QUEUE_MODE_ASYNCHRONOUS ||
      altdesc->priority > ZE_COMMAND_QUEUE_PRIORITY_PRIORITY_HIGH) {
    return ZE_RESULT_ERROR_INVALID_ENUMERATION;
  }

  shim::IntrusivePtr<shim::ze::Context> ctx(
      reinterpret_cast<shim::ze::Context *>(hContext));
  shim::IntrusivePtr<CmDeviceEmu> dev(reinterpret_cast<CmDeviceEmu *>(hDevice));

  try {
    auto queue = shim::MakeIntrusive<shim::ze::Queue>(ctx, dev);
    auto list = shim::MakeIntrusive<shim::ze::CommandList>(ctx, queue);
    shim::IntrusivePtrAddRef(list.get());
    *phCommandList = reinterpret_cast<ze_command_list_handle_t>(list.get());
  } catch (std::bad_alloc &e) {
    return ZE_RESULT_ERROR_OUT_OF_HOST_MEMORY;
  }

  return ZE_RESULT_SUCCESS;
}

ZE_APIEXPORT ze_result_t ZE_APICALL
SHIM_CALL(zeCommandListDestroy)(ze_command_list_handle_t hCommandList) {
  if (hCommandList == nullptr) {
    return ZE_RESULT_ERROR_INVALID_NULL_HANDLE;
  }

  shim::IntrusivePtr<shim::ze::CommandList> list(
      reinterpret_cast<shim::ze::CommandList *>(hCommandList), false);

  return ZE_RESULT_SUCCESS;
}

ZE_APIEXPORT ze_result_t ZE_APICALL
SHIM_CALL(zeCommandListClose)(ze_command_list_handle_t hCommandList) {
  if (hCommandList == nullptr) {
    return ZE_RESULT_ERROR_INVALID_NULL_HANDLE;
  }

  return ZE_RESULT_SUCCESS;
}

ZE_APIEXPORT ze_result_t ZE_APICALL
SHIM_CALL(zeCommandListReset)(ze_command_list_handle_t hCommandList) {
  if (hCommandList == nullptr) {
    return ZE_RESULT_ERROR_INVALID_NULL_HANDLE;
  }

  shim::IntrusivePtr<shim::ze::CommandList> list =
      reinterpret_cast<shim::ze::CommandList *>(hCommandList);
  list->Reset();

  return ZE_RESULT_SUCCESS;
}

namespace {
std::vector<shim::IntrusivePtr<shim::ze::Event>>
GetEvents(size_t num, ze_event_handle_t *phEvents) {
  std::vector<shim::IntrusivePtr<shim::ze::Event>> events;

  std::transform(phEvents, phEvents + num, std::back_inserter(events),
                 [](ze_event_handle_t h) {
                   return reinterpret_cast<shim::ze::Event *>(h);
                 });

  return events;
}
} // namespace

ZE_APIEXPORT ze_result_t ZE_APICALL
SHIM_CALL(zeCommandListAppendWriteGlobalTimestamp)(
    ze_command_list_handle_t hCommandList, uint64_t *dstptr,
    ze_event_handle_t hSignalEvent, uint32_t numWaitEvents,
    ze_event_handle_t *phWaitEvents) {
  return ZE_RESULT_ERROR_UNSUPPORTED_FEATURE;
}

ZE_APIEXPORT ze_result_t ZE_APICALL SHIM_CALL(zeCommandListAppendMemoryCopy)(
    ze_command_list_handle_t hCommandList, void *dstptr, const void *srcptr,
    size_t size, ze_event_handle_t hSignalEvent, uint32_t numWaitEvents,
    ze_event_handle_t *phWaitEvents) {
  if (hCommandList == nullptr) {
    return ZE_RESULT_ERROR_INVALID_NULL_HANDLE;
  }

  if (dstptr == nullptr || srcptr == nullptr) {
    return ZE_RESULT_ERROR_INVALID_NULL_POINTER;
  }

  if (numWaitEvents > 0 && phWaitEvents == nullptr) {
    return ZE_RESULT_ERROR_INVALID_SIZE;
  }

  shim::IntrusivePtr<shim::ze::CommandList> list =
      reinterpret_cast<shim::ze::CommandList *>(hCommandList);

  try {
    shim::IntrusivePtr<shim::ze::Event> signal =
        reinterpret_cast<shim::ze::Event *>(hSignalEvent);
    auto wait = GetEvents(numWaitEvents, phWaitEvents);
    return list->Emplace(dstptr, srcptr, size, signal, std::move(wait));
  } catch (std::bad_alloc &e) {
    return ZE_RESULT_ERROR_OUT_OF_HOST_MEMORY;
  }
}

ZE_APIEXPORT ze_result_t ZE_APICALL SHIM_CALL(zeCommandListAppendMemoryFill)(
    ze_command_list_handle_t hCommandList, void *ptr, const void *pattern,
    size_t pattern_size, size_t size, ze_event_handle_t hSignalEvent,
    uint32_t numWaitEvents, ze_event_handle_t *phWaitEvents) {
  if (hCommandList == nullptr) {
    return ZE_RESULT_ERROR_INVALID_NULL_HANDLE;
  }

  if (ptr == nullptr || pattern == nullptr) {
    return ZE_RESULT_ERROR_INVALID_NULL_POINTER;
  }

  if (numWaitEvents > 0 && phWaitEvents == nullptr) {
    return ZE_RESULT_ERROR_INVALID_SIZE;
  }

  shim::IntrusivePtr<shim::ze::CommandList> list =
      reinterpret_cast<shim::ze::CommandList *>(hCommandList);

  try {
    shim::IntrusivePtr<shim::ze::Event> signal =
        reinterpret_cast<shim::ze::Event *>(hSignalEvent);
    auto wait = GetEvents(numWaitEvents, phWaitEvents);
    return list->Emplace(ptr, *static_cast<const uint8_t *>(pattern), size,
                         signal, std::move(wait));
  } catch (std::bad_alloc &e) {
    return ZE_RESULT_ERROR_OUT_OF_HOST_MEMORY;
  }
}

ZE_APIEXPORT ze_result_t ZE_APICALL
SHIM_CALL(zeCommandListAppendMemoryCopyRegion)(
    ze_command_list_handle_t hCommandList, void *dstptr,
    const ze_copy_region_t *dstRegion, uint32_t dstPitch,
    uint32_t dstSlicePitch, const void *srcptr,
    const ze_copy_region_t *srcRegion, uint32_t srcPitch,
    uint32_t srcSlicePitch, ze_event_handle_t hSignalEvent,
    uint32_t numWaitEvents, ze_event_handle_t *phWaitEvents) {
  return ZE_RESULT_ERROR_UNSUPPORTED_FEATURE;
}

ZE_APIEXPORT ze_result_t ZE_APICALL
SHIM_CALL(zeCommandListAppendMemoryCopyFromContext)(
    ze_command_list_handle_t hCommandList, void *dstptr,
    ze_context_handle_t hContextSrc, const void *srcptr, size_t size,
    ze_event_handle_t hSignalEvent, uint32_t numWaitEvents,
    ze_event_handle_t *phWaitEvents) {
  return ZE_RESULT_ERROR_UNSUPPORTED_FEATURE;
}

ZE_APIEXPORT ze_result_t ZE_APICALL SHIM_CALL(zeCommandListAppendImageCopy)(
    ze_command_list_handle_t hCommandList, ze_image_handle_t hDstImage,
    ze_image_handle_t hSrcImage, ze_event_handle_t hSignalEvent,
    uint32_t numWaitEvents, ze_event_handle_t *phWaitEvents) {
  return ZE_RESULT_ERROR_UNSUPPORTED_FEATURE;
}

ZE_APIEXPORT ze_result_t ZE_APICALL
SHIM_CALL(zeCommandListAppendImageCopyRegion)(
    ze_command_list_handle_t hCommandList, ze_image_handle_t hDstImage,
    ze_image_handle_t hSrcImage, const ze_image_region_t *pDstRegion,
    const ze_image_region_t *pSrcRegion, ze_event_handle_t hSignalEvent,
    uint32_t numWaitEvents, ze_event_handle_t *phWaitEvents) {
  return ZE_RESULT_ERROR_UNSUPPORTED_FEATURE;
}

ZE_APIEXPORT ze_result_t ZE_APICALL SHIM_CALL(
    zeCommandListAppendImageCopyToMemory)(ze_command_list_handle_t hCommandList,
                                          void *dstptr,
                                          ze_image_handle_t hSrcImage,
                                          const ze_image_region_t *pSrcRegion,
                                          ze_event_handle_t hSignalEvent,
                                          uint32_t numWaitEvents,
                                          ze_event_handle_t *phWaitEvents) {
  if (hCommandList == nullptr || hSrcImage == nullptr) {
    return ZE_RESULT_ERROR_INVALID_NULL_HANDLE;
  }

  if (dstptr == nullptr) {
    return ZE_RESULT_ERROR_INVALID_NULL_POINTER;
  }

  if (numWaitEvents > 0 && phWaitEvents == nullptr) {
    return ZE_RESULT_ERROR_INVALID_SIZE;
  }

  // EMU does not support region access
  if (pSrcRegion != nullptr) {
    return ZE_RESULT_ERROR_UNSUPPORTED_FEATURE;
  }

  shim::IntrusivePtr<shim::ze::CommandList> list =
      reinterpret_cast<shim::ze::CommandList *>(hCommandList);
  shim::IntrusivePtr<shim::ze::Image> image =
      reinterpret_cast<shim::ze::Image *>(hSrcImage);

  try {
    shim::IntrusivePtr<shim::ze::Event> signal =
        reinterpret_cast<shim::ze::Event *>(hSignalEvent);
    auto wait = GetEvents(numWaitEvents, phWaitEvents);
    return list->Emplace(dstptr, image, signal, std::move(wait));
  } catch (std::bad_alloc &e) {
    return ZE_RESULT_ERROR_OUT_OF_HOST_MEMORY;
  }
}

ZE_APIEXPORT ze_result_t ZE_APICALL
SHIM_CALL(zeCommandListAppendImageCopyFromMemory)(
    ze_command_list_handle_t hCommandList, ze_image_handle_t hDstImage,
    const void *srcptr, const ze_image_region_t *pDstRegion,
    ze_event_handle_t hSignalEvent, uint32_t numWaitEvents,
    ze_event_handle_t *phWaitEvents) {
  if (hCommandList == nullptr || hDstImage == nullptr) {
    return ZE_RESULT_ERROR_INVALID_NULL_HANDLE;
  }

  if (srcptr == nullptr) {
    return ZE_RESULT_ERROR_INVALID_NULL_POINTER;
  }

  if (numWaitEvents > 0 && phWaitEvents == nullptr) {
    return ZE_RESULT_ERROR_INVALID_SIZE;
  }

  // EMU does not support region access
  if (pDstRegion != nullptr) {
    return ZE_RESULT_ERROR_UNSUPPORTED_FEATURE;
  }

  shim::IntrusivePtr<shim::ze::CommandList> list =
      reinterpret_cast<shim::ze::CommandList *>(hCommandList);
  shim::IntrusivePtr<shim::ze::Image> image =
      reinterpret_cast<shim::ze::Image *>(hDstImage);

  try {
    shim::IntrusivePtr<shim::ze::Event> signal =
        reinterpret_cast<shim::ze::Event *>(hSignalEvent);
    auto wait = GetEvents(numWaitEvents, phWaitEvents);
    return list->Emplace(image, srcptr, signal, std::move(wait));
  } catch (std::bad_alloc &e) {
    return ZE_RESULT_ERROR_OUT_OF_HOST_MEMORY;
  }
}

ZE_APIEXPORT ze_result_t ZE_APICALL SHIM_CALL(
    zeCommandListAppendMemoryPrefetch)(ze_command_list_handle_t hCommandList,
                                       const void *ptr, size_t size) {
  return ZE_RESULT_ERROR_UNSUPPORTED_FEATURE;
}

ZE_APIEXPORT ze_result_t ZE_APICALL SHIM_CALL(zeCommandListAppendMemAdvise)(
    ze_command_list_handle_t hCommandList, ze_device_handle_t hDevice,
    const void *ptr, size_t size, ze_memory_advice_t advice) {
  return ZE_RESULT_ERROR_UNSUPPORTED_FEATURE;
}

ZE_APIEXPORT ze_result_t ZE_APICALL SHIM_CALL(zeCommandListAppendBarrier)(
    ze_command_list_handle_t hCommandList, ze_event_handle_t hSignalEvent,
    uint32_t numWaitEvents, ze_event_handle_t *phWaitEvents) {
  if (hCommandList == nullptr) {
    return ZE_RESULT_ERROR_INVALID_NULL_HANDLE;
  }

  if (numWaitEvents > 0 && phWaitEvents == nullptr) {
    return ZE_RESULT_ERROR_INVALID_SIZE;
  }

  shim::IntrusivePtr<shim::ze::CommandList> list =
      reinterpret_cast<shim::ze::CommandList *>(hCommandList);

  try {
    shim::IntrusivePtr<shim::ze::Event> signal =
        reinterpret_cast<shim::ze::Event *>(hSignalEvent);
    auto wait = GetEvents(numWaitEvents, phWaitEvents);
    return list->Emplace(signal, std::move(wait));
  } catch (std::bad_alloc &e) {
    return ZE_RESULT_ERROR_OUT_OF_HOST_MEMORY;
  }
}

ZE_APIEXPORT ze_result_t ZE_APICALL
SHIM_CALL(zeCommandListAppendMemoryRangesBarrier)(
    ze_command_list_handle_t hCommandList, uint32_t numRanges,
    const size_t *pRangeSizes, const void **pRanges,
    ze_event_handle_t hSignalEvent, uint32_t numWaitEvents,
    ze_event_handle_t *phWaitEvents) {
  if (pRangeSizes == nullptr || pRanges == nullptr) {
    return ZE_RESULT_ERROR_INVALID_NULL_POINTER;
  }

  return SHIM_CALL(zeCommandListAppendBarrier)(hCommandList, hSignalEvent,
                                               numWaitEvents, phWaitEvents);
}

ZE_APIEXPORT ze_result_t ZE_APICALL SHIM_CALL(zeCommandListAppendLaunchKernel)(
    ze_command_list_handle_t hCommandList, ze_kernel_handle_t hKernel,
    const ze_group_count_t *pLaunchFuncArgs, ze_event_handle_t hSignalEvent,
    uint32_t numWaitEvents, ze_event_handle_t *phWaitEvents) {
  if (hCommandList == nullptr || hKernel == nullptr) {
    return ZE_RESULT_ERROR_INVALID_NULL_HANDLE;
  }

  if (pLaunchFuncArgs == nullptr) {
    return ZE_RESULT_ERROR_INVALID_NULL_POINTER;
  }

  if (numWaitEvents > 0 && phWaitEvents == nullptr) {
    return ZE_RESULT_ERROR_INVALID_SIZE;
  }

  shim::IntrusivePtr<shim::ze::CommandList> list =
      reinterpret_cast<shim::ze::CommandList *>(hCommandList);
  shim::IntrusivePtr<shim::ze::Kernel> kernel =
      reinterpret_cast<shim::ze::Kernel *>(hKernel);

  // Group size is not set for the kernel
  if (kernel->group_size_.find(kernel->kernel_.get()) ==
      kernel->group_size_.end()) {
    return ZE_RESULT_ERROR_INVALID_ARGUMENT;
  }

  try {
    shim::IntrusivePtr<shim::ze::Event> signal =
        reinterpret_cast<shim::ze::Event *>(hSignalEvent);
    auto wait = GetEvents(numWaitEvents, phWaitEvents);
    return list->Emplace(kernel, *pLaunchFuncArgs, signal, std::move(wait));
  } catch (std::bad_alloc &e) {
    return ZE_RESULT_ERROR_OUT_OF_HOST_MEMORY;
  }
}

ZE_APIEXPORT ze_result_t ZE_APICALL
SHIM_CALL(zeCommandListAppendLaunchCooperativeKernel)(
    ze_command_list_handle_t hCommandList, ze_kernel_handle_t hKernel,
    const ze_group_count_t *pLaunchFuncArgs, ze_event_handle_t hSignalEvent,
    uint32_t numWaitEvents, ze_event_handle_t *phWaitEvents) {
  return ZE_RESULT_ERROR_UNSUPPORTED_FEATURE;
}

ZE_APIEXPORT ze_result_t ZE_APICALL
SHIM_CALL(zeCommandListAppendLaunchKernelIndirect)(
    ze_command_list_handle_t hCommandList, ze_kernel_handle_t hKernel,
    const ze_group_count_t *pLaunchArgumentsBuffer,
    ze_event_handle_t hSignalEvent, uint32_t numWaitEvents,
    ze_event_handle_t *phWaitEvents) {
  return ZE_RESULT_ERROR_UNSUPPORTED_FEATURE;
}

ZE_APIEXPORT ze_result_t ZE_APICALL
SHIM_CALL(zeCommandListAppendLaunchMultipleKernelsIndirect)(
    ze_command_list_handle_t hCommandList, uint32_t numKernels,
    ze_kernel_handle_t *phKernels, const uint32_t *pCountBuffer,
    const ze_group_count_t *pLaunchArgumentsBuffer,
    ze_event_handle_t hSignalEvent, uint32_t numWaitEvents,
    ze_event_handle_t *phWaitEvents) {
  return ZE_RESULT_ERROR_UNSUPPORTED_FEATURE;
}

ZE_APIEXPORT ze_result_t ZE_APICALL SHIM_CALL(zeCommandListAppendSignalEvent)(
    ze_command_list_handle_t hCommandList, ze_event_handle_t hEvent) {
  if (hEvent == nullptr) {
    return ZE_RESULT_ERROR_INVALID_NULL_POINTER;
  }

  return SHIM_CALL(zeCommandListAppendBarrier)(hCommandList, hEvent, 0,
                                               nullptr);
}

ZE_APIEXPORT ze_result_t ZE_APICALL SHIM_CALL(zeCommandListAppendWaitOnEvents)(
    ze_command_list_handle_t hCommandList, uint32_t numEvents,
    ze_event_handle_t *phEvents) {
  if (phEvents == nullptr) {
    return ZE_RESULT_ERROR_INVALID_NULL_POINTER;
  }

  return SHIM_CALL(zeCommandListAppendBarrier)(hCommandList, nullptr, numEvents,
                                               phEvents);
}

ZE_APIEXPORT ze_result_t ZE_APICALL SHIM_CALL(zeCommandListAppendEventReset)(
    ze_command_list_handle_t hCommandList, ze_event_handle_t hEvent) {
  if (hCommandList == nullptr || hEvent == nullptr) {
    return ZE_RESULT_ERROR_INVALID_NULL_HANDLE;
  }

  shim::IntrusivePtr<shim::ze::CommandList> list =
      reinterpret_cast<shim::ze::CommandList *>(hCommandList);
  shim::IntrusivePtr<shim::ze::Event> event =
      reinterpret_cast<shim::ze::Event *>(hEvent);

  try {
    return list->Emplace(event);
  } catch (std::bad_alloc &e) {
    return ZE_RESULT_ERROR_OUT_OF_HOST_MEMORY;
  }
}

ZE_APIEXPORT ze_result_t ZE_APICALL
SHIM_CALL(zeCommandListAppendQueryKernelTimestamps)(
    ze_command_list_handle_t hCommandList, uint32_t numEvents,
    ze_event_handle_t *phEvents, void *dstptr, const size_t *pOffsets,
    ze_event_handle_t hSignalEvent, uint32_t numWaitEvents,
    ze_event_handle_t *phWaitEvents) {
  if (hCommandList == nullptr) {
    return ZE_RESULT_ERROR_INVALID_NULL_HANDLE;
  }

  if (phEvents == nullptr || dstptr == nullptr) {
    return ZE_RESULT_ERROR_INVALID_NULL_POINTER;
  }

  if (numWaitEvents > 0 && phWaitEvents == nullptr) {
    return ZE_RESULT_ERROR_INVALID_SIZE;
  }

  shim::IntrusivePtr<shim::ze::CommandList> list =
      reinterpret_cast<shim::ze::CommandList *>(hCommandList);

  try {
    auto events = GetEvents(numEvents, phEvents);
    shim::IntrusivePtr<shim::ze::Event> signal =
        reinterpret_cast<shim::ze::Event *>(hSignalEvent);
    auto wait = GetEvents(numWaitEvents, phWaitEvents);
    return list->Emplace(std::move(events), dstptr, pOffsets, signal,
                         std::move(wait));
  } catch (std::bad_alloc &e) {
    return ZE_RESULT_ERROR_OUT_OF_HOST_MEMORY;
  }
}
