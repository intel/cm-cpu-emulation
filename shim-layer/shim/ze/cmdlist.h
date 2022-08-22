/*========================== begin_copyright_notice ============================

Copyright (C) 2022 Intel Corporation

SPDX-License-Identifier: MIT

============================= end_copyright_notice ===========================*/

#ifndef CM_EMU_SHIM_ZE_CMDLIST_H
#define CM_EMU_SHIM_ZE_CMDLIST_H

#include "context.h"
#include "event.h"
#include "image.h"
#include "kernel.h"
#include "queue.h"

#include <memory>

namespace shim {
namespace ze {
class Command {
public:
  enum class Type {
    Barrier,
    BufferCopy,
    BufferFill,
    EventReset,
    EventTimestamp,
    ImageCopyFromMem,
    ImageCopyToMem,
    Kernel,
    Invalid,
  };

  Command(IntrusivePtr<Event> signal, std::vector<IntrusivePtr<Event>> &&wait)
      : type_(Type::Barrier), signal_(signal), wait_(std::move(wait)) {}

  Command(void *dst, const void *src, size_t size, IntrusivePtr<Event> signal,
          std::vector<IntrusivePtr<Event>> &&wait)
      : type_(Type::BufferCopy), copy_({size, dst, src}), signal_(signal),
        wait_(std::move(wait)) {}

  Command(void *dst, uint8_t pattern, size_t size, IntrusivePtr<Event> signal,
          std::vector<IntrusivePtr<Event>> &&wait)
      : type_(Type::BufferFill), fill_({size, dst, pattern}), signal_(signal),
        wait_(std::move(wait)) {}

  Command(IntrusivePtr<Kernel> kernel, ze_group_count_t group_count,
          IntrusivePtr<Event> signal, std::vector<IntrusivePtr<Event>> &&wait)
      : type_(Type::Kernel), kernel_(kernel), group_count_(group_count),
        signal_(signal), wait_(std::move(wait)) {}

  Command(IntrusivePtr<Image> image, const void *src,
          IntrusivePtr<Event> signal, std::vector<IntrusivePtr<Event>> &&wait)
      : type_(Type::ImageCopyFromMem), image_(image), copy_({0, nullptr, src}),
        signal_(signal), wait_(std::move(wait)) {}

  Command(void *dst, IntrusivePtr<Image> image, IntrusivePtr<Event> signal,
          std::vector<IntrusivePtr<Event>> &&wait)
      : type_(Type::ImageCopyToMem), image_(image), copy_({0, dst, nullptr}),
        signal_(signal), wait_(std::move(wait)) {}

  Command(IntrusivePtr<Event> event)
      : type_(Type::EventReset), events_(1, event), signal_(nullptr), wait_() {}

  Command(std::vector<IntrusivePtr<Event>> &&events, void *dst,
          const size_t *offsets, IntrusivePtr<Event> signal,
          std::vector<IntrusivePtr<Event>> &&wait)
      : type_(Type::EventTimestamp), events_(std::move(events)),
        timestamps_({dst, offsets}), signal_(signal), wait_(std::move(wait)) {}

  Type type_;

  IntrusivePtr<Event> signal_;
  std::vector<IntrusivePtr<Event>> wait_;

  IntrusivePtr<Kernel> kernel_;
  IntrusivePtr<Image> image_;
  std::vector<IntrusivePtr<Event>> events_;

  union {
    struct {
      size_t size;
      void *dst;
      const void *src;
    } copy_;
    struct {
      size_t size;
      void *dst;
      uint8_t pattern;
    } fill_;
    struct {
      void *dst;
      const size_t *offsets;
    } timestamps_;
    ze_group_count_t group_count_;
  };
};

class CommandList : public IntrusiveRefCounter<CommandList> {
public:
  CommandList(IntrusivePtr<Context> context,
              IntrusivePtr<Queue> queue = nullptr)
      : context_(context), queue_(queue) {}

  ze_result_t Append(const Command &cmd);

  template <typename... Args> ze_result_t Emplace(Args &&...args) {
    if (queue_) {
      return queue_->Execute(Command(std::forward<Args>(args)...));
    }

    list_.emplace_back(std::forward<Args>(args)...);
    return ZE_RESULT_SUCCESS;
  }

  void Reset() { list_.clear(); }

  auto begin() { return list_.begin(); }
  auto end() { return list_.end(); }

private:
  IntrusivePtr<Context> context_;
  IntrusivePtr<Queue> queue_;
  std::vector<Command> list_;
};
} // namespace ze
} // namespace shim

extern "C" {
ZE_APIEXPORT ze_result_t ZE_APICALL SHIM_CALL(zeCommandListCreate)(
    ze_context_handle_t hContext, ze_device_handle_t hDevice,
    const ze_command_list_desc_t *desc,
    ze_command_list_handle_t *phCommandList);
ZE_APIEXPORT ze_result_t ZE_APICALL SHIM_CALL(zeCommandListCreateImmediate)(
    ze_context_handle_t hContext, ze_device_handle_t hDevice,
    const ze_command_queue_desc_t *altdesc,
    ze_command_list_handle_t *phCommandList);
ZE_APIEXPORT ze_result_t ZE_APICALL
    SHIM_CALL(zeCommandListDestroy)(ze_command_list_handle_t hCommandList);
ZE_APIEXPORT ze_result_t ZE_APICALL
    SHIM_CALL(zeCommandListClose)(ze_command_list_handle_t hCommandList);
ZE_APIEXPORT ze_result_t ZE_APICALL
    SHIM_CALL(zeCommandListReset)(ze_command_list_handle_t hCommandList);
ZE_APIEXPORT
    ze_result_t ZE_APICALL SHIM_CALL(zeCommandListAppendWriteGlobalTimestamp)(
        ze_command_list_handle_t hCommandList, uint64_t *dstptr,
        ze_event_handle_t hSignalEvent, uint32_t numWaitEvents,
        ze_event_handle_t *phWaitEvents);
ZE_APIEXPORT ze_result_t ZE_APICALL SHIM_CALL(zeCommandListAppendMemoryCopy)(
    ze_command_list_handle_t hCommandList, void *dstptr, const void *srcptr,
    size_t size, ze_event_handle_t hSignalEvent, uint32_t numWaitEvents,
    ze_event_handle_t *phWaitEvents);
ZE_APIEXPORT ze_result_t ZE_APICALL SHIM_CALL(zeCommandListAppendMemoryFill)(
    ze_command_list_handle_t hCommandList, void *ptr, const void *pattern,
    size_t pattern_size, size_t size, ze_event_handle_t hSignalEvent,
    uint32_t numWaitEvents, ze_event_handle_t *phWaitEvents);
ZE_APIEXPORT
    ze_result_t ZE_APICALL SHIM_CALL(zeCommandListAppendMemoryCopyRegion)(
        ze_command_list_handle_t hCommandList, void *dstptr,
        const ze_copy_region_t *dstRegion, uint32_t dstPitch,
        uint32_t dstSlicePitch, const void *srcptr,
        const ze_copy_region_t *srcRegion, uint32_t srcPitch,
        uint32_t srcSlicePitch, ze_event_handle_t hSignalEvent,
        uint32_t numWaitEvents, ze_event_handle_t *phWaitEvents);
ZE_APIEXPORT
    ze_result_t ZE_APICALL SHIM_CALL(zeCommandListAppendMemoryCopyFromContext)(
        ze_command_list_handle_t hCommandList, void *dstptr,
        ze_context_handle_t hContextSrc, const void *srcptr, size_t size,
        ze_event_handle_t hSignalEvent, uint32_t numWaitEvents,
        ze_event_handle_t *phWaitEvents);
ZE_APIEXPORT ze_result_t ZE_APICALL SHIM_CALL(zeCommandListAppendImageCopy)(
    ze_command_list_handle_t hCommandList, ze_image_handle_t hDstImage,
    ze_image_handle_t hSrcImage, ze_event_handle_t hSignalEvent,
    uint32_t numWaitEvents, ze_event_handle_t *phWaitEvents);
ZE_APIEXPORT
    ze_result_t ZE_APICALL SHIM_CALL(zeCommandListAppendImageCopyRegion)(
        ze_command_list_handle_t hCommandList, ze_image_handle_t hDstImage,
        ze_image_handle_t hSrcImage, const ze_image_region_t *pDstRegion,
        const ze_image_region_t *pSrcRegion, ze_event_handle_t hSignalEvent,
        uint32_t numWaitEvents, ze_event_handle_t *phWaitEvents);
ZE_APIEXPORT
    ze_result_t ZE_APICALL SHIM_CALL(zeCommandListAppendImageCopyToMemory)(
        ze_command_list_handle_t hCommandList, void *dstptr,
        ze_image_handle_t hSrcImage, const ze_image_region_t *pSrcRegion,
        ze_event_handle_t hSignalEvent, uint32_t numWaitEvents,
        ze_event_handle_t *phWaitEvents);
ZE_APIEXPORT
    ze_result_t ZE_APICALL SHIM_CALL(zeCommandListAppendImageCopyFromMemory)(
        ze_command_list_handle_t hCommandList, ze_image_handle_t hDstImage,
        const void *srcptr, const ze_image_region_t *pDstRegion,
        ze_event_handle_t hSignalEvent, uint32_t numWaitEvents,
        ze_event_handle_t *phWaitEvents);
ZE_APIEXPORT ze_result_t ZE_APICALL SHIM_CALL(
    zeCommandListAppendMemoryPrefetch)(ze_command_list_handle_t hCommandList,
                                       const void *ptr, size_t size);
ZE_APIEXPORT ze_result_t ZE_APICALL SHIM_CALL(zeCommandListAppendMemAdvise)(
    ze_command_list_handle_t hCommandList, ze_device_handle_t hDevice,
    const void *ptr, size_t size, ze_memory_advice_t advice);
ZE_APIEXPORT ze_result_t ZE_APICALL SHIM_CALL(zeCommandListAppendBarrier)(
    ze_command_list_handle_t hCommandList, ze_event_handle_t hSignalEvent,
    uint32_t numWaitEvents, ze_event_handle_t *phWaitEvents);
ZE_APIEXPORT
    ze_result_t ZE_APICALL SHIM_CALL(zeCommandListAppendMemoryRangesBarrier)(
        ze_command_list_handle_t hCommandList, uint32_t numRanges,
        const size_t *pRangeSizes, const void **pRanges,
        ze_event_handle_t hSignalEvent, uint32_t numWaitEvents,
        ze_event_handle_t *phWaitEvents);
ZE_APIEXPORT ze_result_t ZE_APICALL SHIM_CALL(zeCommandListAppendLaunchKernel)(
    ze_command_list_handle_t hCommandList, ze_kernel_handle_t hKernel,
    const ze_group_count_t *pLaunchFuncArgs, ze_event_handle_t hSignalEvent,
    uint32_t numWaitEvents, ze_event_handle_t *phWaitEvents);
ZE_APIEXPORT ze_result_t ZE_APICALL
    SHIM_CALL(zeCommandListAppendLaunchCooperativeKernel)(
        ze_command_list_handle_t hCommandList, ze_kernel_handle_t hKernel,
        const ze_group_count_t *pLaunchFuncArgs, ze_event_handle_t hSignalEvent,
        uint32_t numWaitEvents, ze_event_handle_t *phWaitEvents);
ZE_APIEXPORT
    ze_result_t ZE_APICALL SHIM_CALL(zeCommandListAppendLaunchKernelIndirect)(
        ze_command_list_handle_t hCommandList, ze_kernel_handle_t hKernel,
        const ze_group_count_t *pLaunchArgumentsBuffer,
        ze_event_handle_t hSignalEvent, uint32_t numWaitEvents,
        ze_event_handle_t *phWaitEvents);
ZE_APIEXPORT ze_result_t ZE_APICALL
    SHIM_CALL(zeCommandListAppendLaunchMultipleKernelsIndirect)(
        ze_command_list_handle_t hCommandList, uint32_t numKernels,
        ze_kernel_handle_t *phKernels, const uint32_t *pCountBuffer,
        const ze_group_count_t *pLaunchArgumentsBuffer,
        ze_event_handle_t hSignalEvent, uint32_t numWaitEvents,
        ze_event_handle_t *phWaitEvents);
ZE_APIEXPORT ze_result_t ZE_APICALL SHIM_CALL(zeCommandListAppendSignalEvent)(
    ze_command_list_handle_t hCommandList, ze_event_handle_t hEvent);
ZE_APIEXPORT ze_result_t ZE_APICALL SHIM_CALL(zeCommandListAppendWaitOnEvents)(
    ze_command_list_handle_t hCommandList, uint32_t numEvents,
    ze_event_handle_t *phEvents);
ZE_APIEXPORT ze_result_t ZE_APICALL SHIM_CALL(zeCommandListAppendEventReset)(
    ze_command_list_handle_t hCommandList, ze_event_handle_t hEvent);
ZE_APIEXPORT
    ze_result_t ZE_APICALL SHIM_CALL(zeCommandListAppendQueryKernelTimestamps)(
        ze_command_list_handle_t hCommandList, uint32_t numEvents,
        ze_event_handle_t *phEvents, void *dstptr, const size_t *pOffsets,
        ze_event_handle_t hSignalEvent, uint32_t numWaitEvents,
        ze_event_handle_t *phWaitEvents);
} // extern "C"
#endif // CM_EMU_SHIM_ZE_CMDLIST_H
