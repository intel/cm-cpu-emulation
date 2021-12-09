/*========================== begin_copyright_notice ============================

Copyright (C) 2020 Intel Corporation

SPDX-License-Identifier: MIT

============================= end_copyright_notice ===========================*/

#include <vector>
#include <string.h>

// l0
#include <level_zero/ze_api.h>
// cm
#include <cm_rt.h>

#include "shim.h"
#include "kernel_utils.h"

#include "emu_log.h"
#include "emu_cfg.h"
#include "emu_utils.h"

#if defined(_WIN32)
#define SHIM_CALL(x) shim_ ## x
#undef ZE_APIEXPORT
#define ZE_APIEXPORT extern "C"
#else /* _WIN32 */
#define SHIM_CALL(x) x
#undef ZE_APIEXPORT
#define ZE_APIEXPORT extern "C"
#endif /* _WIN32 */

//#define ZE_APIEXPORT
//#define ZE_APICALL

struct _ze_driver_handle_t
{
} CM_DRIVER;
ze_driver_handle_t CM_DRIVER_HANDLE = &CM_DRIVER;

struct _ze_device_handle_t
{
    CmDevice *cm_device = nullptr;
    bool isSubdevice = false;
    bool subdeviceId = 0;
} CM_DEVICE, CM_RENDER_ENGINE, CM_COPY_ENGINE;
ze_device_handle_t CM_DEVICE_HANDLE = &CM_DEVICE;
ze_device_handle_t CM_RENDER_HANDLE = &CM_RENDER_ENGINE;
ze_device_handle_t CM_COPY_HANDLE = &CM_COPY_ENGINE;

struct _ze_context_handle_t
{
    int dummy;
} CM_CONTEXT;
ze_context_handle_t CM_CONTEXT_HANDLE = &CM_CONTEXT;

struct _ze_event_handle_t
{
    static const bool signaled = true;
};

enum class ZeCmdId
{
    Barrier, LaunchKernel, MemoryCopy, ImgCopyFromMem, ImgCopyToMem, Invalid
};

struct ZeCmdArgs {};

struct ZeBarrierArgs : public ZeCmdArgs
{
    ZeBarrierArgs(ze_event_handle_t e, uint32_t n, ze_event_handle_t* pe):
        hSignalEvent(e), numWaitEvents(n), phWaitEvents(pe) {}
    ze_event_handle_t hSignalEvent;
    uint32_t numWaitEvents;
    ze_event_handle_t* phWaitEvents;
};

struct ZeLaunchKernelArgs: public ZeCmdArgs
{
    ZeLaunchKernelArgs(
        ze_kernel_handle_t k,
        const ze_group_count_t* a,
        ze_event_handle_t e,
        uint32_t n,
        ze_event_handle_t* pe):
            hKernel(k), groupCount(*a),
            hSignalEvent(e), numWaitEvents(n), phWaitEvents(pe)
        {}
    ze_kernel_handle_t hKernel;
    const ze_group_count_t groupCount;
    ze_event_handle_t hSignalEvent;
    uint32_t numWaitEvents;
    ze_event_handle_t* phWaitEvents;
};

struct ZeMemoryCopyArgs: public ZeCmdArgs
{
    ZeMemoryCopyArgs(void* d, const void* s, size_t sz, ze_event_handle_t e, uint32_t n, ze_event_handle_t* p):
        dstptr(d), srcptr(s), size(sz), hSignalEvent(e), numWaitEvents(n), phWaitEvents(p) {}
    void* dstptr;
    const void* srcptr;
    size_t size;
    ze_event_handle_t hSignalEvent;
    uint32_t numWaitEvents;
    ze_event_handle_t* phWaitEvents;
};

struct ZeImgCopyFromMemArgs: public ZeCmdArgs
{
    ZeImgCopyFromMemArgs(ze_image_handle_t h, const void* s, const ze_image_region_t* p, ze_event_handle_t e, uint32_t n, ze_event_handle_t* ph):
        hDstImage(h), srcptr(s), pDstRegion(p), hSignalEvent(e), numWaitEvents(n), phWaitEvents(ph) {}
    ze_image_handle_t hDstImage;
    const void* srcptr;
    const ze_image_region_t* pDstRegion;
    ze_event_handle_t hSignalEvent;
    uint32_t numWaitEvents;
    ze_event_handle_t* phWaitEvents;
};

struct ZeImgCopyToMemArgs: public ZeCmdArgs
{
    ZeImgCopyToMemArgs(void* d, ze_image_handle_t h, const ze_image_region_t* p, ze_event_handle_t e, uint32_t n, ze_event_handle_t* ph):
        dstptr(d), hSrcImage(h), pSrcRegion(p), hSignalEvent(e), numWaitEvents(n), phWaitEvents(ph)  {}
    void* dstptr;
    ze_image_handle_t hSrcImage;
    const ze_image_region_t* pSrcRegion;
    ze_event_handle_t hSignalEvent;
    uint32_t numWaitEvents;
    ze_event_handle_t* phWaitEvents;
};

struct ZeCommand
{
    ZeCommand(ZeCmdId _id, ZeCmdArgs *_args): id(_id), args(_args) {}
    ZeCmdId id = ZeCmdId::Invalid;
    ZeCmdArgs *args = nullptr;
};

using ZeCommandVector = std::vector<ZeCommand>;

struct _ze_command_list_handle_t
{
    ZeCommandVector commands;
    bool is_immediate = false;
    ze_command_queue_handle_t queue = nullptr;
};

struct _ze_command_queue_handle_t
{
    CmQueue *cm_queue = nullptr;
    ze_device_handle_t device = nullptr;
};

struct _ze_module_handle_t
{
    ProgramInfo info;
    CmProgram *cm_program = nullptr;
    ze_device_handle_t device = nullptr;
};

struct _ze_module_build_log_handle_t
{
};

struct _ze_kernel_handle_t
{
    uint32_t sizeX = 1;
    uint32_t sizeY = 1;
    uint32_t sizeZ = 1;
    CmKernel* cm_kernel = nullptr;
    ze_device_handle_t device = nullptr;
    CmArgTypeVector params;
};

typedef struct _ze_device_mem_alloc
{
    enum AllocType
    {
        SharedBuffer, Buffer, Image2d, Image3d
    } type = Buffer;
    union
    {
        CmBuffer *cm_buf = nullptr;
        CmBufferUP *cm_bufup;
        CmSurface2D *cm_surf2d;
        CmSurface3D *cm_surf3d;
    } v;

    template<AllocType TypeV, class C>
    static _ze_device_mem_alloc* Create(C* cm_ptr, ze_device_handle_t dev, void *storage = nullptr)
    {
        auto result = new _ze_device_mem_alloc;

        result->device = dev;
        result->type = TypeV;

        if constexpr (TypeV == Buffer)            result->v.cm_buf = cm_ptr;
        else if constexpr (TypeV == SharedBuffer) result->v.cm_bufup = cm_ptr;
        else if constexpr (TypeV == Image2d)      result->v.cm_surf2d = cm_ptr;
        else if constexpr (TypeV == Image3d)      result->v.cm_surf3d = cm_ptr;

        _ze_device_mem_alloc::Allocations.insert(result);
        if (storage)
            _ze_device_mem_alloc::SharedAllocations.emplace(storage, result);

        return result;
    }

    int GetIndex(SurfaceIndex *&idx)
    {
        switch (type)
        {
        case Image2d:
            return v.cm_surf2d->GetIndex(idx);
        case Image3d:
            return v.cm_surf3d->GetIndex(idx);
        case SharedBuffer:
            return v.cm_bufup->GetIndex(idx);
        default:
        case Buffer:
            return v.cm_buf->GetIndex(idx);
        }
    }
    int Release()
    {
        switch (type)
        {
        case Image2d:
            return device->cm_device->DestroySurface(v.cm_surf2d);
        case Image3d:
            return device->cm_device->DestroySurface(v.cm_surf3d);
        case SharedBuffer:
            return device->cm_device->DestroyBufferUP(v.cm_bufup);
        default:
        case Buffer:
            return device->cm_device->DestroySurface(v.cm_buf);
        }
    }

    size_t size_in_bytes = 0;
    ze_device_handle_t device = nullptr;

    using AllocSet = std::unordered_set<const void*>;
    static inline AllocSet Allocations;

    using Storage2AllocMap = std::unordered_map<const void*, _ze_device_mem_alloc*>;
    static inline Storage2AllocMap SharedAllocations;

} *ze_device_mem_alloc;

struct _ze_image_handle_t : public _ze_device_mem_alloc {};

/// functions ///
bool ExecuteCommand(ze_command_queue_handle_t hCommandQueue, const ZeCommand& command);
///

ZE_APIEXPORT ze_result_t ZE_APICALL
SHIM_CALL(zeInit)(
    ze_init_flags_t flags                            ///< [in] initialization flags
        )
{
    CmDevice *device = nullptr;
    unsigned int version = 0;
    int result = ::CreateCmDevice(device, version);
    CM_DEVICE.cm_device = device;
    CM_RENDER_ENGINE.cm_device = device;
    CM_RENDER_ENGINE.isSubdevice = true;
    CM_RENDER_ENGINE.subdeviceId = 1;
    CM_COPY_ENGINE.cm_device = device;
    CM_COPY_ENGINE.isSubdevice = true;
    CM_COPY_ENGINE.subdeviceId = 2;
    if ((result != CM_SUCCESS) || (version < CM_1_0))
    {
        return ZE_RESULT_ERROR_UNSUPPORTED_FEATURE;
    }
    return ZE_RESULT_SUCCESS;
}

ZE_APIEXPORT ze_result_t ZE_APICALL
SHIM_CALL(zeDriverGet)(
    uint32_t* pCount,                               ///< [in,out] pointer to the number of driver instances.
                                                    ///< if count is zero, then the loader will update the value with the total
                                                    ///< number of drivers available.
                                                    ///< if count is non-zero, then the loader will only retrieve that number
                                                    ///< of drivers.
                                                    ///< if count is larger than the number of drivers available, then the
                                                    ///< loader will update the value with the correct number of drivers available.
    ze_driver_handle_t* phDrivers                   ///< [in,out][optional][range(0, *pCount)] array of driver instance handles
    )
{
    if (pCount)
    {
        *pCount = 1;
    }
    if (phDrivers)
    {
        *phDrivers = CM_DRIVER_HANDLE;
    }
    return ZE_RESULT_SUCCESS;
}

ZE_APIEXPORT ze_result_t ZE_APICALL
SHIM_CALL(zeDeviceGet)(
    ze_driver_handle_t hDriver,                     ///< [in] handle of the driver instance
    uint32_t* pCount,                               ///< [in,out] pointer to the number of devices.
                                                    ///< if count is zero, then the driver will update the value with the total
                                                    ///< number of devices available.
                                                    ///< if count is non-zero, then driver will only retrieve that number of devices.
                                                    ///< if count is larger than the number of devices available, then the
                                                    ///< driver will update the value with the correct number of devices available.
    ze_device_handle_t* phDevices                   ///< [in,out][optional][range(0, *pCount)] array of handle of devices
    )
{
    if (hDriver != CM_DRIVER_HANDLE)
    {
        return ZE_RESULT_ERROR_UNSUPPORTED_FEATURE;
    }
    if (pCount)
    {
        *pCount = 1;
    }
    if (phDevices)
    {
        *phDevices = CM_DEVICE_HANDLE;
    }
    return ZE_RESULT_SUCCESS;
}

ZE_APIEXPORT ze_result_t ZE_APICALL
SHIM_CALL(zeDeviceGetProperties)(
    ze_device_handle_t hDevice,                     ///< [in] handle of the device
    ze_device_properties_t* pDeviceProperties       ///< [in,out] query result for device properties
    )
{
    pDeviceProperties->type = ZE_DEVICE_TYPE_GPU;
    pDeviceProperties->vendorId = 0;
    pDeviceProperties->deviceId = 0;
    pDeviceProperties->uuid = {{0xff}};
    if (hDevice->isSubdevice)
    {
        pDeviceProperties->flags = ZE_DEVICE_PROPERTY_FLAG_SUBDEVICE;
        pDeviceProperties->subdeviceId = hDevice->subdeviceId;
    }
    else
    {
        pDeviceProperties->subdeviceId = 0;
    }
    pDeviceProperties->coreClockRate = 100000;
    pDeviceProperties->maxMemAllocSize = 0x1FFFFFFFFFFFFFFFull;
    pDeviceProperties->maxHardwareContexts = 1;

    pDeviceProperties->maxCommandQueuePriority = 1;
    pDeviceProperties->numThreadsPerEU = 7;
    pDeviceProperties->physicalEUSimdWidth = 8;
    pDeviceProperties->numEUsPerSubslice = 8;
    pDeviceProperties->numSubslicesPerSlice = 2;
    pDeviceProperties->numSlices = 1;

    pDeviceProperties->timerResolution = 1000;
    pDeviceProperties->timestampValidBits = 60;
    pDeviceProperties->kernelTimestampValidBits = 60;
    strcpy(pDeviceProperties->name, GfxEmu::Utils::toUpper(GfxEmu::Cfg::Platform ().getStr ()).c_str());
    return ZE_RESULT_SUCCESS;
}

ZE_APIEXPORT ze_result_t ZE_APICALL
SHIM_CALL(zeDeviceGetComputeProperties)(
    ze_device_handle_t hDevice,                         ///< [in] handle of the device
    ze_device_compute_properties_t* pComputeProperties  ///< [in,out] query result for compute properties
    )
{
    if (pComputeProperties) {
        pComputeProperties->maxTotalGroupSize = 0xffu;
        pComputeProperties->maxGroupSizeX = 0xffu;
        pComputeProperties->maxGroupSizeY = 0xffu;
        pComputeProperties->maxGroupSizeZ = 0xffu;
        pComputeProperties->maxGroupCountX = 0xffffffffu;
        pComputeProperties->maxGroupCountY = 0xffffffffu;
        pComputeProperties->maxGroupCountZ = 0xffffffffu;
        pComputeProperties->maxSharedLocalMemory = 0x1000u;
        pComputeProperties->numSubGroupSizes = 3;
        pComputeProperties->subGroupSizes[0] = 8;
        pComputeProperties->subGroupSizes[1] = 16;
        pComputeProperties->subGroupSizes[2] = 32;
        return ZE_RESULT_SUCCESS;
    }
    return ZE_RESULT_ERROR_INVALID_NULL_POINTER;
}

ZE_APIEXPORT ze_result_t ZE_APICALL
SHIM_CALL(zeContextCreate)(
    ze_driver_handle_t hDriver,                     ///< [in] handle of the driver object
    const ze_context_desc_t* desc,                  ///< [in] pointer to context descriptor
    ze_context_handle_t* phContext                  ///< [out] pointer to handle of context object created
    )
{
    if (phContext)
    {
        *phContext = CM_CONTEXT_HANDLE;
    }
    return ZE_RESULT_SUCCESS;
}

ZE_APIEXPORT ze_result_t ZE_APICALL
SHIM_CALL(zeContextDestroy)(
    ze_context_handle_t hContext                    ///< [in][release] handle of context object to destroy
    )
{
    return ZE_RESULT_SUCCESS;
}

ZE_APIEXPORT ze_result_t ZE_APICALL
SHIM_CALL(zeCommandListCreate)(
    ze_context_handle_t hContext,                   ///< [in] handle of the context object
    ze_device_handle_t hDevice,                     ///< [in] handle of the device object
    const ze_command_list_desc_t* desc,             ///< [in] pointer to command list descriptor
    ze_command_list_handle_t* phCommandList         ///< [out] pointer to handle of command list object created
    )
{
    // let's ignore flags for now
    if (phCommandList)
    {
        *phCommandList = new _ze_command_list_handle_t;
        return ZE_RESULT_SUCCESS;
    }
    else
    {
        return ZE_RESULT_ERROR_UNSUPPORTED_FEATURE;
    }
}

void* allocSharedMem(ze_device_handle_t hDevice, size_t size)
{
    CmBufferUP *buf = nullptr;
    void *storage = malloc(size);
    int status = hDevice->cm_device->CreateBufferUP(static_cast<unsigned int>(size), storage, buf);
    if (status != CM_SUCCESS)
    {
        return nullptr;
    }
    ze_device_mem_alloc a = _ze_device_mem_alloc::Create<_ze_device_mem_alloc::SharedBuffer>(buf, hDevice, storage);
    return storage;
}

void *allocDeviceMem(ze_device_handle_t hDevice, size_t size)
{
    CmBuffer *buf = nullptr;
    int status = hDevice->cm_device->CreateBuffer(static_cast<unsigned int>(size), buf);
    if (status != CM_SUCCESS)
    {
        return nullptr;
    }
    ze_device_mem_alloc a = _ze_device_mem_alloc::Create<_ze_device_mem_alloc::Buffer>(buf, hDevice);
    return a;
}

ZE_APIEXPORT ze_result_t ZE_APICALL
SHIM_CALL(zeMemAllocDevice)(
    ze_context_handle_t hContext,                   ///< [in] handle of the context object
    const ze_device_mem_alloc_desc_t* device_desc,  ///< [in] pointer to device mem alloc descriptor
    size_t size,                                    ///< [in] size in bytes to allocate
    size_t alignment,                               ///< [in] minimum alignment in bytes for the allocation
    ze_device_handle_t hDevice,                     ///< [in] handle of the device
    void** pptr                                     ///< [out] pointer to device allocation
    )
{
    // Looks like some pieces of host code and kernels do want to use both
    // raw pointers and SurfaceIndex even for device allocations.
    // Let's allocate all memory as "shared" for now.
    //void* allocation = allocDeviceMem(hDevice, size);
    void* allocation = allocSharedMem(hDevice, size);
    if (!allocation)
    {
        return ZE_RESULT_ERROR_UNSUPPORTED_FEATURE;
    }
    *pptr = allocation;
    return ZE_RESULT_SUCCESS;
}

ZE_APIEXPORT ze_result_t ZE_APICALL
SHIM_CALL(zeCommandListClose)(
    ze_command_list_handle_t hCommandList           ///< [in] handle of command list object to close
    )
{
    return ZE_RESULT_SUCCESS;
}

ZE_APIEXPORT ze_result_t ZE_APICALL
SHIM_CALL(zeMemFree)(
    ze_context_handle_t hContext,                   ///< [in] handle of the context object
    void* ptr                                       ///< [in][release] pointer to memory to free
    )
{
    if (_ze_device_mem_alloc::Allocations.count(ptr) > 0)
    {
        ze_device_mem_alloc a = reinterpret_cast<ze_device_mem_alloc>(ptr);
        a->Release();
        _ze_device_mem_alloc::Allocations.erase(a);
        delete a;
    }
    else
    {
        auto storage2alloc = _ze_device_mem_alloc::SharedAllocations.find(ptr);
        if (storage2alloc != _ze_device_mem_alloc::SharedAllocations.end())
        {
            storage2alloc->second->Release();
            delete storage2alloc->second;
            _ze_device_mem_alloc::Allocations.erase(storage2alloc->second);
            _ze_device_mem_alloc::SharedAllocations.erase(storage2alloc);
        }
        free(ptr);
    }
    return ZE_RESULT_SUCCESS;
}

ZE_APIEXPORT ze_result_t ZE_APICALL
SHIM_CALL(zeCommandQueueCreate)(
    ze_context_handle_t hContext,                   ///< [in] handle of the context object
    ze_device_handle_t hDevice,                     ///< [in] handle of the device object
    const ze_command_queue_desc_t* desc,            ///< [in] pointer to command queue descriptor
    ze_command_queue_handle_t* phCommandQueue       ///< [out] pointer to handle of command queue object created
    )
{
    ze_command_queue_handle_t q = new _ze_command_queue_handle_t;
    if (phCommandQueue)
    {
        *phCommandQueue = q;
    }
    CmQueue *queue = nullptr;
    int result = hDevice->cm_device->CreateQueue(queue);
    if (result != CM_SUCCESS)
    {
        return ZE_RESULT_ERROR_UNSUPPORTED_FEATURE;
    }
    q->cm_queue = queue;
    q->device = hDevice;
    return ZE_RESULT_SUCCESS;
}

ZE_APIEXPORT ze_result_t ZE_APICALL
SHIM_CALL(zeModuleCreate)(
    ze_context_handle_t hContext,                   ///< [in] handle of the context object
    ze_device_handle_t hDevice,                     ///< [in] handle of the device
    const ze_module_desc_t* desc,                   ///< [in] pointer to module descriptor
    ze_module_handle_t* phModule,                   ///< [out] pointer to handle of module object created
    ze_module_build_log_handle_t* phBuildLog        ///< [out][optional] pointer to handle of module's build log.
    )
{
    ze_module_handle_t m = new _ze_module_handle_t;
    if (phModule)
    {
        *phModule = m;
    }
    CmProgram *p = nullptr;
    int status = hDevice->cm_device->LoadProgram(0, 0, p);
    if (status != CM_SUCCESS)
    {
        return ZE_RESULT_ERROR_UNSUPPORTED_FEATURE;
    }
    m->cm_program = p;
    m->info = ProgramManager::instance().AddProgram(desc->pInputModule, desc->inputSize);
    m->device = hDevice;
    bool success = m->info.isValid();
    return success ?
        ZE_RESULT_SUCCESS :
        ZE_RESULT_ERROR_UNSUPPORTED_FEATURE;
}

ZE_APIEXPORT ze_result_t ZE_APICALL
SHIM_CALL(zeModuleBuildLogDestroy)(
    ze_module_build_log_handle_t hModuleBuildLog    ///< [in][release] handle of the module build log object.
    )
{
    return ZE_RESULT_SUCCESS;
}

ZE_APIEXPORT ze_result_t ZE_APICALL
SHIM_CALL(zeModuleBuildLogGetString)(
    ze_module_build_log_handle_t hModuleBuildLog,
    size_t* pSize,
    char* pBuildLog
    )
{
    if(hModuleBuildLog == nullptr)
        return ZE_RESULT_ERROR_INVALID_NULL_HANDLE;

    if(pSize == nullptr)
        return ZE_RESULT_ERROR_INVALID_NULL_POINTER;

    *pSize = 0;
    return ZE_RESULT_SUCCESS;
}

ZE_APIEXPORT ze_result_t ZE_APICALL
SHIM_CALL(zeKernelCreate)(
    ze_module_handle_t hModule,                     ///< [in] handle of the module
    const ze_kernel_desc_t* desc,                   ///< [in] pointer to kernel descriptor
    ze_kernel_handle_t* phKernel                    ///< [out] handle of the Function object
    )
{
    auto kernel_data_it = hModule->info.kernels.find(desc->pKernelName);
    if (kernel_data_it == hModule->info.kernels.end())
    {
        return ZE_RESULT_ERROR_UNSUPPORTED_FEATURE;
    }
    void *proc = kernel_data_it->second.entry_point;
    if (proc == nullptr)
    {
        return ZE_RESULT_ERROR_UNSUPPORTED_FEATURE;
    }
    CmKernel *k = nullptr;
    int status = hModule->device->cm_device->CreateKernel(hModule->cm_program, desc->pKernelName, proc, k);
    if (status != CM_SUCCESS)
    {
        return ZE_RESULT_ERROR_UNSUPPORTED_FEATURE;
    }
    ze_kernel_handle_t kernel = new _ze_kernel_handle_t;
    kernel->cm_kernel = k;
    kernel->params = kernel_data_it->second.args;
    kernel->device = hModule->device;
    if (phKernel)
    {
        *phKernel = kernel;
    }
    return ZE_RESULT_SUCCESS;
}

void HandleImmediateCmdList(ze_command_list_handle_t hCommandList)
{
    if (hCommandList->is_immediate)
    {
        auto& cmd = hCommandList->commands.back();
        ExecuteCommand(hCommandList->queue, cmd);
        hCommandList->commands.pop_back();
    }
}

ZE_APIEXPORT ze_result_t ZE_APICALL
SHIM_CALL(zeCommandListAppendBarrier)(
    ze_command_list_handle_t hCommandList,          ///< [in] handle of the command list
    ze_event_handle_t hSignalEvent,                 ///< [in][optional] handle of the event to signal on completion
    uint32_t numWaitEvents,                         ///< [in][optional] number of events to wait on before executing barrier
    ze_event_handle_t* phWaitEvents                 ///< [in][optional][range(0, numWaitEvents)] handle of the events to wait
                                                    ///< on before executing barrier
    )
{
    if (hCommandList)
    {
        ZeBarrierArgs *args = new ZeBarrierArgs(hSignalEvent, numWaitEvents, phWaitEvents);
        hCommandList->commands.emplace_back(ZeCmdId::Barrier, args);
        HandleImmediateCmdList(hCommandList);
        return ZE_RESULT_SUCCESS;
    }
    return ZE_RESULT_ERROR_UNSUPPORTED_FEATURE;
}

ZE_APIEXPORT ze_result_t ZE_APICALL
SHIM_CALL(zeKernelSetArgumentValue)(
    ze_kernel_handle_t hKernel,                     ///< [in] handle of the kernel object
    uint32_t argIndex,                              ///< [in] argument index in range [0, num args - 1]
    size_t argSize,                                 ///< [in] size of argument type
    const void* pArgValue                           ///< [in][optional] argument value represented as matching arg type. If
                                                    ///< null then argument value is considered null.
    )
{
    int status = CM_FAILURE;
    switch (hKernel->params[argIndex])
    {
    case CmArgumentType::SurfaceIndex:
    {
        auto storage2alloc = _ze_device_mem_alloc::SharedAllocations.find(*reinterpret_cast<const void* const*>(pArgValue));
        ze_device_mem_alloc arg = nullptr;
        if (storage2alloc != _ze_device_mem_alloc::SharedAllocations.end())
        {
            arg = storage2alloc->second;
        }
        else
        {
            const ze_device_mem_alloc *pArg = reinterpret_cast<const ze_device_mem_alloc*>(pArgValue);
            arg = *pArg;
        }
        SurfaceIndex *idx = nullptr;
        status = arg->GetIndex(idx);
        if (status != CM_SUCCESS)
        {
            break;
        }
        status = hKernel->cm_kernel->SetKernelArg(argIndex, sizeof(SurfaceIndex), idx);
        break;
    }
    case CmArgumentType::Scalar:
    default:
        status = hKernel->cm_kernel->SetKernelArg(argIndex, argSize, pArgValue);
        break;
    }
    return status == CM_SUCCESS ?
        ZE_RESULT_SUCCESS :
        ZE_RESULT_ERROR_UNSUPPORTED_FEATURE;
}

ZE_APIEXPORT ze_result_t ZE_APICALL
SHIM_CALL(zeKernelSetGroupSize)(
    ze_kernel_handle_t hKernel,                     ///< [in] handle of the kernel object
    uint32_t groupSizeX,                            ///< [in] group size for X dimension to use for this kernel
    uint32_t groupSizeY,                            ///< [in] group size for Y dimension to use for this kernel
    uint32_t groupSizeZ                             ///< [in] group size for Z dimension to use for this kernel
    )
{
    if (hKernel)
    {
        hKernel->sizeX = groupSizeX;
        hKernel->sizeY = groupSizeY;
        hKernel->sizeZ = groupSizeZ;
        return ZE_RESULT_SUCCESS;
    }
    else
    {
        return ZE_RESULT_ERROR_UNSUPPORTED_FEATURE;
    }
}

ZE_APIEXPORT ze_result_t ZE_APICALL
SHIM_CALL(zeCommandListAppendLaunchKernel)(
    ze_command_list_handle_t hCommandList,          ///< [in] handle of the command list
    ze_kernel_handle_t hKernel,                     ///< [in] handle of the kernel object
    const ze_group_count_t* pLaunchFuncArgs,        ///< [in] thread group launch arguments
    ze_event_handle_t hSignalEvent,                 ///< [in][optional] handle of the event to signal on completion
    uint32_t numWaitEvents,                         ///< [in][optional] number of events to wait on before launching
    ze_event_handle_t* phWaitEvents                 ///< [in][optional][range(0, numWaitEvents)] handle of the events to wait
                                                    ///< on before launching
    )
{
    if (hCommandList)
    {
        ZeLaunchKernelArgs *args = new ZeLaunchKernelArgs(hKernel, pLaunchFuncArgs, hSignalEvent, numWaitEvents, phWaitEvents);
        hCommandList->commands.emplace_back(ZeCmdId::LaunchKernel, args);
        HandleImmediateCmdList(hCommandList);
        return ZE_RESULT_SUCCESS;
    }
    return ZE_RESULT_ERROR_UNSUPPORTED_FEATURE;
}

bool ExecuteBarrier(ze_command_queue_handle_t hCommandQueue, ZeBarrierArgs* args)
{
    // let's make it NOP for now
    return true;
}

bool ExecuteLaunchKernel(ze_command_queue_handle_t hCommandQueue, ZeLaunchKernelArgs *args)
{
    CmTask *t = nullptr;
    int status = hCommandQueue->device->cm_device->CreateTask(t);
    if (status != CM_SUCCESS)
    {
        return false;
    }
    if ((args->hKernel->sizeZ > 1) || (args->groupCount.groupCountZ > 1))
    {
        // do not support 3-dimensional enqueue yet
        GFX_EMU_ERROR_MESSAGE("SHIM layer doesn't yet support 3-dimentional enqueue, "
            "while was asked for z == %u", args->groupCount.groupCountZ);
        return false;
    }
    CmThreadGroupSpace *tgs = nullptr;
    unsigned int t_w = args->hKernel->sizeX;
    unsigned int t_h = args->hKernel->sizeY;
    unsigned int tg_w = args->groupCount.groupCountX;
    unsigned int tg_h = args->groupCount.groupCountY;
    status = hCommandQueue->device->cm_device->CreateThreadGroupSpace(t_w, t_h, tg_w, tg_h, tgs);
    if (status != CM_SUCCESS)
    {
        hCommandQueue->device->cm_device->DestroyTask(t);
        return false;
    }
    status = t->AddKernel(args->hKernel->cm_kernel);
    if (status != CM_SUCCESS)
    {
        hCommandQueue->device->cm_device->DestroyTask(t);
        return false;
    }
    CmEvent *e = nullptr;
    status = hCommandQueue->cm_queue->EnqueueWithGroup(t, e, tgs);
    return status == CM_SUCCESS;
}

bool ExecuteMemoryCopy(ze_command_queue_handle_t hCommandQueue, ZeMemoryCopyArgs *args)
{
    bool srcIsDevice = _ze_device_mem_alloc::Allocations.count(args->srcptr) > 0;
    bool dstIsDevice = _ze_device_mem_alloc::Allocations.count(args->dstptr) > 0;
    int status = CM_FAILURE;
    if (srcIsDevice && dstIsDevice)
    {
        // wow, dev->dev is a legal case now!
        unsigned char* buffer = new unsigned char[args->size];

        const _ze_device_mem_alloc *deviceSrcMem = reinterpret_cast<const _ze_device_mem_alloc*>(args->srcptr);
        status = deviceSrcMem->v.cm_buf->ReadSurface(buffer, nullptr, args->size);
        if (status != CM_SUCCESS)
        {
            delete[] buffer;
            return false;
        }

        ze_device_mem_alloc deviceDstMem = reinterpret_cast<ze_device_mem_alloc>(args->dstptr);
        status = deviceDstMem->v.cm_buf->WriteSurface(buffer, nullptr, args->size);
        delete[] buffer;
    }
    else if (!srcIsDevice && !dstIsDevice)
    {
        // host -> host and shared -> shared
        // src and dest may overlap.
        status = std::memmove(args->dstptr, args->srcptr, args->size) != nullptr;
    }
    else
    {
        if (dstIsDevice)
        {
            ze_device_mem_alloc deviceMem = reinterpret_cast<ze_device_mem_alloc>(args->dstptr);
            status = deviceMem->v.cm_buf->WriteSurface(reinterpret_cast<const unsigned char*>(args->srcptr),
                nullptr, args->size);
        }
        if (srcIsDevice)
        {
            const _ze_device_mem_alloc *deviceMem = reinterpret_cast<const _ze_device_mem_alloc*>(args->srcptr);
            status = deviceMem->v.cm_buf->ReadSurface(reinterpret_cast<unsigned char*>(args->dstptr),
                nullptr, args->size);
        }
    }
    return status == CM_SUCCESS;
}

bool ExecuteImgCopyToMem(ze_command_queue_handle_t hCommandQueue, ZeImgCopyToMemArgs *args)
{
/*  void* dstptr,                                   ///< [in] pointer to destination memory to copy to
    ze_image_handle_t hSrcImage,                    ///< [in] handle of source image to copy from
    const ze_image_region_t* pSrcRegion,            ///< [in][optional] source region descriptor
    ze_event_handle_t hEvent                        ///< [in][optional] handle of the event to signal on completion
    */
    const _ze_device_mem_alloc *alloc = args->hSrcImage;
    int status = CM_FAILURE;
    if (alloc->type == _ze_device_mem_alloc::Buffer)
    {
        status = alloc->v.cm_buf->ReadSurface(reinterpret_cast<unsigned char*>(args->dstptr),
            nullptr, alloc->size_in_bytes);
    } else if (alloc->type == _ze_device_mem_alloc::Image2d)
    {
        status = alloc->v.cm_surf2d->ReadSurface(reinterpret_cast<unsigned char*>(args->dstptr),
            nullptr, alloc->size_in_bytes);
    }
    return status == CM_SUCCESS;
}

bool ExecuteImgCopyFromMem(ze_command_queue_handle_t hCommandQueue, ZeImgCopyFromMemArgs *args)
{
/*  ze_image_handle_t hDstImage,                    ///< [in] handle of destination image to copy to
    const void* srcptr,                             ///< [in] pointer to source memory to copy from
    const ze_image_region_t* pDstRegion,            ///< [in][optional] destination region descriptor
    ze_event_handle_t hEvent                        ///< [in][optional] handle of the event to signal on completion
    */
    const _ze_device_mem_alloc *alloc = args->hDstImage;
    int status = CM_FAILURE;
    if (alloc->type == _ze_device_mem_alloc::Buffer)
    {
        status = alloc->v.cm_buf->WriteSurface(reinterpret_cast<const unsigned char*>(args->srcptr),
            nullptr, alloc->size_in_bytes);
    } else if (alloc->type == _ze_device_mem_alloc::Image2d)
    {
        status = alloc->v.cm_surf2d->WriteSurface(reinterpret_cast<const unsigned char*>(args->srcptr),
            nullptr, alloc->size_in_bytes);
    }
    return status == CM_SUCCESS;

}

bool ExecuteCommand(ze_command_queue_handle_t hCommandQueue, const ZeCommand& command)
{
    switch(command.id)
    {
    case ZeCmdId::Barrier:
        return ExecuteBarrier(hCommandQueue, static_cast<ZeBarrierArgs*>(command.args));
    case ZeCmdId::LaunchKernel:
        return ExecuteLaunchKernel(hCommandQueue, static_cast<ZeLaunchKernelArgs*>(command.args));
    case ZeCmdId::MemoryCopy:
        return ExecuteMemoryCopy(hCommandQueue, static_cast<ZeMemoryCopyArgs*>(command.args));
    case ZeCmdId::ImgCopyFromMem:
        return ExecuteImgCopyFromMem(hCommandQueue, static_cast<ZeImgCopyFromMemArgs*>(command.args));
    case ZeCmdId::ImgCopyToMem:
        return ExecuteImgCopyToMem(hCommandQueue, static_cast<ZeImgCopyToMemArgs*>(command.args));
    case ZeCmdId::Invalid:
    default:
        break;
    }
    return false;
}

ZE_APIEXPORT ze_result_t ZE_APICALL
SHIM_CALL(zeCommandQueueExecuteCommandLists)(
    ze_command_queue_handle_t hCommandQueue,        ///< [in] handle of the command queue
    uint32_t numCommandLists,                       ///< [in] number of command lists to execute
    ze_command_list_handle_t* phCommandLists,       ///< [in][range(0, numCommandLists)] list of handles of the command lists
                                                    ///< to execute
    ze_fence_handle_t hFence                        ///< [in][optional] handle of the fence to signal on completion
    )
{
    for (auto& command : phCommandLists[0]->commands)
    {
        ExecuteCommand(hCommandQueue, command);
        // should we clear the list after execution?
        //delete command.args;
    }
    return ZE_RESULT_SUCCESS;
}

ZE_APIEXPORT ze_result_t ZE_APICALL
SHIM_CALL(zeCommandQueueSynchronize)(
    ze_command_queue_handle_t hCommandQueue,        ///< [in] handle of the command queue
    uint64_t timeout                                ///< [in] if non-zero, then indicates the maximum time to yield before
                                                    ///< returning ::ZE_RESULT_SUCCESS or ::ZE_RESULT_NOT_READY
                                                    ///< if zero, then operates exactly like ::zeFenceQueryStatus
                                                    ///< if UINT32_MAX, then function will not return until complete or device
                                                    ///< is lost.
    )
{
    return ZE_RESULT_SUCCESS;
}

ZE_APIEXPORT ze_result_t ZE_APICALL
SHIM_CALL(zeCommandListAppendMemoryCopy)(
    ze_command_list_handle_t hCommandList,          ///< [in] handle of command list
    void* dstptr,                                   ///< [in] pointer to destination memory to copy to
    const void* srcptr,                             ///< [in] pointer to source memory to copy from
    size_t size,                                    ///< [in] size in bytes to copy
    ze_event_handle_t hSignalEvent,                 ///< [in][optional] handle of the event to signal on completion
    uint32_t numWaitEvents,                         ///< [in][optional] number of events to wait on before launching; must be 0
                                                    ///< if `nullptr == phWaitEvents`
    ze_event_handle_t* phWaitEvents                 ///< [in][optional][range(0, numWaitEvents)] handle of the events to wait
                                                    ///< on before launching
    )
{
    if (hCommandList)
    {
        ZeMemoryCopyArgs *args = new ZeMemoryCopyArgs(dstptr, srcptr, size, hSignalEvent, numWaitEvents, phWaitEvents);
        hCommandList->commands.emplace_back(ZeCmdId::MemoryCopy, args);
        HandleImmediateCmdList(hCommandList);
        return ZE_RESULT_SUCCESS;
    }
    return ZE_RESULT_ERROR_UNSUPPORTED_FEATURE;
}

CM_SURFACE_FORMAT zeImageFormatToCmFormat(const ze_image_format_t& format)
{
    // format:
    // ->layout = various
    // ->type = SINT/unsigned int/UNORM/SNORM/FLOAT
    // ->x/y/z/w = swizzling channels
    //
    switch(format.layout)
    {
    case ZE_IMAGE_FORMAT_LAYOUT_32:
        {
            if (format.type == ZE_IMAGE_FORMAT_TYPE_UINT)
            {
                return CM_SURFACE_FORMAT_A8R8G8B8;
            } else if (format.type == ZE_IMAGE_FORMAT_TYPE_FLOAT)
            {
                return CM_SURFACE_FORMAT_A8R8G8B8;
            }
        }
        break;
    case ZE_IMAGE_FORMAT_LAYOUT_8:
        return CM_SURFACE_FORMAT_A8;
    default:
        break;
    }
    return CM_SURFACE_FORMAT_A8R8G8B8;
}

ZE_APIEXPORT ze_result_t ZE_APICALL
SHIM_CALL(zeImageCreate)(
    ze_context_handle_t hContext,                   ///< [in] handle of the context object
    ze_device_handle_t hDevice,                     ///< [in] handle of the device
    const ze_image_desc_t* desc,                    ///< [in] pointer to image descriptor
    ze_image_handle_t* phImage                      ///< [out] pointer to handle of image object created
    )
{
    if (!desc)
    {
        return ZE_RESULT_ERROR_UNSUPPORTED_FEATURE;
    }
    // desc->width/height/depth in pixels (or width = bytes, for Buffer)
    switch(desc->type)
    {
    case ZE_IMAGE_TYPE_2D:
        {
            CmSurface2D *surface = nullptr;
            int status = hDevice->cm_device->CreateSurface2D(desc->width, desc->height,
                zeImageFormatToCmFormat(desc->format), surface);
            if (status != CM_SUCCESS)
            {
                return ZE_RESULT_ERROR_UNSUPPORTED_FEATURE;
            }
            ze_device_mem_alloc a = _ze_device_mem_alloc::Create<_ze_device_mem_alloc::Image2d>(surface, hDevice);
            a->size_in_bytes = desc->width * desc->height * 4;
            *phImage = static_cast<ze_image_handle_t>(a);
            return ZE_RESULT_SUCCESS;
        }
        break;
    case ZE_IMAGE_TYPE_BUFFER:
        {
            CmBuffer *buf = nullptr;
            int status = hDevice->cm_device->CreateBuffer(static_cast<unsigned int>(desc->width), buf);
            if (status != CM_SUCCESS)
            {
                return ZE_RESULT_ERROR_UNSUPPORTED_FEATURE;
            }
            ze_device_mem_alloc a = _ze_device_mem_alloc::Create<_ze_device_mem_alloc::Buffer>(buf, hDevice);
            a->size_in_bytes = desc->width;
            *phImage = static_cast<ze_image_handle_t>(a);
            return ZE_RESULT_SUCCESS;
        }
        break;
    case ZE_IMAGE_TYPE_1D:
    case ZE_IMAGE_TYPE_1DARRAY:
    case ZE_IMAGE_TYPE_2DARRAY:
    case ZE_IMAGE_TYPE_3D:
        // not supported
        return ZE_RESULT_ERROR_UNSUPPORTED_FEATURE;
    }
    return ZE_RESULT_ERROR_UNSUPPORTED_FEATURE;
}

ZE_APIEXPORT ze_result_t ZE_APICALL
SHIM_CALL(zeImageDestroy)(
    ze_image_handle_t hImage                        ///< [in][release] handle of image object to destroy
    )
{
    if (hImage)
    {
        hImage->Release();
        delete hImage;
        return ZE_RESULT_SUCCESS;
    }
    return ZE_RESULT_ERROR_UNSUPPORTED_FEATURE;
}

ZE_APIEXPORT ze_result_t ZE_APICALL
SHIM_CALL(zeCommandListDestroy)(
    ze_command_list_handle_t hCommandList           ///< [in][release] handle of command list object to destroy
    )
{
    if (hCommandList)
    {
        for (auto& c : hCommandList->commands)
        {
            delete c.args;
        }
        delete hCommandList;
        return ZE_RESULT_SUCCESS;
    }
    return ZE_RESULT_ERROR_UNSUPPORTED_FEATURE;
}

ZE_APIEXPORT ze_result_t ZE_APICALL
SHIM_CALL(zeCommandQueueDestroy)(
    ze_command_queue_handle_t hCommandQueue         ///< [in][release] handle of command queue object to destroy
    )
{
    if (hCommandQueue)
    {
        if (hCommandQueue->cm_queue && hCommandQueue->device)
        {
            // no DestroyQueue in the public CmDevice iface!
            // hCommandQueue->device->cm_device->DestroyQueue(hCommandQueue->cm_queue);
            delete hCommandQueue;
            return ZE_RESULT_SUCCESS;
        }
    }
    return ZE_RESULT_ERROR_UNSUPPORTED_FEATURE;
}

ZE_APIEXPORT ze_result_t ZE_APICALL
SHIM_CALL(zeCommandListAppendImageCopyToMemory)(
    ze_command_list_handle_t hCommandList,          ///< [in] handle of command list
    void* dstptr,                                   ///< [in] pointer to destination memory to copy to
    ze_image_handle_t hSrcImage,                    ///< [in] handle of source image to copy from
    const ze_image_region_t* pSrcRegion,            ///< [in][optional] source region descriptor
    ze_event_handle_t hSignalEvent,                 ///< [in][optional] handle of the event to signal on completion
    uint32_t numWaitEvents,                         ///< [in][optional] number of events to wait on before launching; must be 0
                                                    ///< if `nullptr == phWaitEvents`
    ze_event_handle_t* phWaitEvents                 ///< [in][optional][range(0, numWaitEvents)] handle of the events to wait
                                                    ///< on before launching
    )
{
    if (hCommandList)
    {
        ZeImgCopyToMemArgs *args = new ZeImgCopyToMemArgs(dstptr, hSrcImage, pSrcRegion, hSignalEvent, numWaitEvents, phWaitEvents);
        hCommandList->commands.emplace_back(ZeCmdId::ImgCopyToMem, args);
        HandleImmediateCmdList(hCommandList);
        return ZE_RESULT_SUCCESS;
    }
    return ZE_RESULT_ERROR_UNSUPPORTED_FEATURE;
}

ZE_APIEXPORT ze_result_t ZE_APICALL
SHIM_CALL(zeCommandListAppendImageCopyFromMemory)(
    ze_command_list_handle_t hCommandList,          ///< [in] handle of command list
    ze_image_handle_t hDstImage,                    ///< [in] handle of destination image to copy to
    const void* srcptr,                             ///< [in] pointer to source memory to copy from
    const ze_image_region_t* pDstRegion,            ///< [in][optional] destination region descriptor
    ze_event_handle_t hSignalEvent,                 ///< [in][optional] handle of the event to signal on completion
    uint32_t numWaitEvents,                         ///< [in][optional] number of events to wait on before launching; must be 0
                                                    ///< if `nullptr == phWaitEvents`
    ze_event_handle_t* phWaitEvents                 ///< [in][optional][range(0, numWaitEvents)] handle of the events to wait
                                                    ///< on before launching
    )
{
    if (hCommandList)
    {
        ZeImgCopyFromMemArgs *args = new ZeImgCopyFromMemArgs(hDstImage, srcptr, pDstRegion, hSignalEvent, numWaitEvents, phWaitEvents);
        hCommandList->commands.emplace_back(ZeCmdId::ImgCopyFromMem, args);
        HandleImmediateCmdList(hCommandList);
        return ZE_RESULT_SUCCESS;
    }
    return ZE_RESULT_ERROR_UNSUPPORTED_FEATURE;
}

ZE_APIEXPORT ze_result_t ZE_APICALL
SHIM_CALL(zeEventHostSynchronize)(
    ze_event_handle_t hEvent,                       ///< [in] handle of the event
    uint64_t timeout                                ///< [in] if non-zero, then indicates the maximum time (in nanoseconds) to
                                                    ///< yield before returning ::ZE_RESULT_SUCCESS or ::ZE_RESULT_NOT_READY;
                                                    ///< if zero, then operates exactly like ::zeEventQueryStatus;
                                                    ///< if UINT32_MAX, then function will not return until complete or device
                                                    ///< is lost.
    )
{
    if (!hEvent)
    {
        return ZE_RESULT_ERROR_INVALID_NULL_HANDLE;
    }
    if (hEvent->signaled)
    {
        return ZE_RESULT_SUCCESS;
    }
    else
    {
        return ZE_RESULT_NOT_READY;
    }
}

ZE_APIEXPORT ze_result_t ZE_APICALL
SHIM_CALL(zeEventQueryKernelTimestamp)(
    ze_event_handle_t hEvent,                       ///< [in] handle of the event
    ze_kernel_timestamp_result_t* dstptr            ///< [in,out] pointer to memory for where timestamp result will be written.
    )
{
    if (!dstptr)
    {
        return ZE_RESULT_ERROR_INVALID_NULL_POINTER;
    }
    dstptr->global.kernelStart = 0;
    dstptr->global.kernelEnd = 100;
    dstptr->context.kernelStart = 0;
    dstptr->context.kernelEnd = 100;
    return ZE_RESULT_SUCCESS;

}

ZE_APIEXPORT ze_result_t ZE_APICALL
SHIM_CALL(zeEventHostReset)(
    ze_event_handle_t hEvent                        ///< [in] handle of the event
    )
{
    if (hEvent)
    {
        //hEvent->signaled = false;
        return ZE_RESULT_SUCCESS;
    }
    else
    {
        return ZE_RESULT_ERROR_INVALID_NULL_HANDLE;
    }
}

ZE_APIEXPORT ze_result_t ZE_APICALL
SHIM_CALL(zeEventPoolCreate)(
    ze_context_handle_t hContext,                   ///< [in] handle of the context object
    const ze_event_pool_desc_t* desc,               ///< [in] pointer to event pool descriptor
    uint32_t numDevices,                            ///< [in][optional] number of device handles; must be 0 if `nullptr ==
                                                    ///< phDevices`
    ze_device_handle_t* phDevices,                  ///< [in][optional][range(0, numDevices)] array of device handles which
                                                    ///< have visibility to the event pool.
                                                    ///< if nullptr, then event pool is visible to all devices supported by the
                                                    ///< driver instance.
    ze_event_pool_handle_t* phEventPool             ///< [out] pointer handle of event pool object created
    )
{
    return ZE_RESULT_SUCCESS;
}

ZE_APIEXPORT ze_result_t ZE_APICALL
SHIM_CALL(zeEventPoolDestroy)(
    ze_event_pool_handle_t hEventPool               ///< [in][release] handle of event pool object to destroy
    )
{
    return ZE_RESULT_SUCCESS;
}

ZE_APIEXPORT ze_result_t ZE_APICALL
SHIM_CALL(zeEventCreate)(
    ze_event_pool_handle_t hEventPool,              ///< [in] handle of the event pool
    const ze_event_desc_t* desc,                    ///< [in] pointer to event descriptor
    ze_event_handle_t* phEvent                      ///< [out] pointer to handle of event object created
    )
{
    if (phEvent)
    {
        *phEvent = new _ze_event_handle_t;
        return ZE_RESULT_SUCCESS;
    }
    else
    {
        return ZE_RESULT_ERROR_INVALID_NULL_POINTER;
    }
}

ZE_APIEXPORT ze_result_t ZE_APICALL
SHIM_CALL(zeEventDestroy)(
    ze_event_handle_t hEvent                        ///< [in][release] handle of event object to destroy
    )
{
  delete hEvent;
  return ZE_RESULT_SUCCESS;
}

ZE_APIEXPORT ze_result_t ZE_APICALL
SHIM_CALL(zeCommandListCreateImmediate)(
    ze_context_handle_t hContext,                   ///< [in] handle of the context object
    ze_device_handle_t hDevice,                     ///< [in] handle of the device object
    const ze_command_queue_desc_t* altdesc,         ///< [in] pointer to command queue descriptor
    ze_command_list_handle_t* phCommandList         ///< [out] pointer to handle of command list object created
    )
{
    if (!phCommandList)
    {
        return ZE_RESULT_ERROR_UNSUPPORTED_FEATURE;
    }
     // let's ignore flags for now
    ze_command_list_handle_t list = new _ze_command_list_handle_t;
    list->is_immediate = true;
    ze_command_queue_handle_t pq = nullptr;
    SHIM_CALL(zeCommandQueueCreate)(hContext, hDevice, altdesc, &pq);
    list->queue = pq;
    *phCommandList = list;
    return ZE_RESULT_SUCCESS;
}

ZE_APIEXPORT ze_result_t ZE_APICALL
SHIM_CALL(zeCommandListReset)(
    ze_command_list_handle_t hCommandList           ///< [in] handle of command list object to reset
    )
{
    for (auto& command : hCommandList->commands)
    {
        delete command.args;
    }
    hCommandList->commands.clear();
    return ZE_RESULT_SUCCESS;
}

ZE_APIEXPORT ze_result_t ZE_APICALL
SHIM_CALL(zeMemAllocShared)(
    ze_context_handle_t hContext,                   ///< [in] handle of the context object
    const ze_device_mem_alloc_desc_t* device_desc,  ///< [in] pointer to device mem alloc descriptor
    const ze_host_mem_alloc_desc_t* host_desc,      ///< [in] pointer to host mem alloc descriptor
    size_t size,                                    ///< [in] size in bytes to allocate
    size_t alignment,                               ///< [in] minimum alignment in bytes for the allocation
    ze_device_handle_t hDevice,                     ///< [in][optional] device handle to associate with
    void** pptr                                     ///< [out] pointer to shared allocation
    )
{
    void* allocation = allocSharedMem(hDevice, size);
    if (!allocation)
    {
        return ZE_RESULT_ERROR_UNSUPPORTED_FEATURE;
    }
    *pptr = allocation;
    return ZE_RESULT_SUCCESS;
}

ZE_APIEXPORT ze_result_t ZE_APICALL
SHIM_CALL(zeDeviceGetSubDevices)(
    ze_device_handle_t hDevice,                     ///< [in] handle of the device object
    uint32_t* pCount,                               ///< [in,out] pointer to the number of sub-devices.
                                                    ///< if count is zero, then the driver will update the value with the total
                                                    ///< number of sub-devices available.
                                                    ///< if count is non-zero, then driver will only retrieve that number of sub-devices.
                                                    ///< if count is larger than the number of sub-devices available, then the
                                                    ///< driver will update the value with the correct number of sub-devices available.
    ze_device_handle_t* phSubdevices                ///< [in,out][optional][range(0, *pCount)] array of handle of sub-devices
    )
{
    if (!pCount)
    {
        return ZE_RESULT_ERROR_INVALID_NULL_POINTER;

    }
    if (!hDevice)
    {
        return ZE_RESULT_ERROR_INVALID_NULL_HANDLE;
    }
    if (*pCount == 0)
    {
        *pCount = 2;
    } else if (*pCount > 2)
    {
        *pCount = 2;
    }
    if (phSubdevices)
    {
        phSubdevices[0] = CM_RENDER_HANDLE;
        if (*pCount > 1)
        {
            phSubdevices[1] = CM_COPY_HANDLE;
        }
    }
    return ZE_RESULT_SUCCESS;
}

ZE_APIEXPORT ze_result_t ZE_APICALL
SHIM_CALL(zeModuleDestroy)(
    ze_module_handle_t hModule                      ///< [in][release] handle of the module
    )
{
    if (hModule)
    {
        bool success = ProgramManager::instance().FreeProgram(hModule->info.handle);
        delete hModule;
    }
    return ZE_RESULT_SUCCESS;
}

ZE_APIEXPORT ze_result_t ZE_APICALL
SHIM_CALL(zeKernelDestroy)(
    ze_kernel_handle_t hKernel                      ///< [in][release] handle of the kernel object
    )
{
    if (hKernel)
    {
        hKernel->device->cm_device->DestroyKernel(hKernel->cm_kernel);
        delete hKernel;

    }
    return ZE_RESULT_SUCCESS;
}

ZE_APIEXPORT ze_result_t ZE_APICALL
SHIM_CALL(zeDeviceGetModuleProperties)(
    ze_device_handle_t hDevice,                     ///< [in] handle of the device
    ze_device_module_properties_t* pModuleProperties///< [in,out] query result for module properties
    )
{
    if (pModuleProperties)
    {
        pModuleProperties->spirvVersionSupported = 0; // Shim will never support spirv
        pModuleProperties->flags = ZE_DEVICE_MODULE_FLAG_FP16
                                 | ZE_DEVICE_MODULE_FLAG_FP64
                                 | ZE_DEVICE_MODULE_FLAG_INT64_ATOMICS
                                 | ZE_DEVICE_MODULE_FLAG_DP4A;
        pModuleProperties->fp16flags =
        pModuleProperties->fp32flags =
        pModuleProperties->fp64flags = ZE_DEVICE_FP_FLAG_DENORM
                                     | ZE_DEVICE_FP_FLAG_INF_NAN
                                     | ZE_DEVICE_FP_FLAG_ROUND_TO_NEAREST
                                     | ZE_DEVICE_FP_FLAG_ROUND_TO_ZERO
                                     | ZE_DEVICE_FP_FLAG_ROUND_TO_INF
                                     | ZE_DEVICE_FP_FLAG_FMA
                                     | ZE_DEVICE_FP_FLAG_ROUNDED_DIVIDE_SQRT;
    }
    return ZE_RESULT_SUCCESS;
}

ZE_APIEXPORT ze_result_t ZE_APICALL
SHIM_CALL(zeDeviceGetP2PProperties)(
    ze_device_handle_t hDevice,                     ///< [in] handle of the device performing the access
    ze_device_handle_t hPeerDevice,                 ///< [in] handle of the peer device with the allocation
    ze_device_p2p_properties_t* pP2PProperties      ///< [in,out] Peer-to-Peer properties between source and peer device
    )
{
    if (pP2PProperties)
    {
        pP2PProperties->flags = ZE_DEVICE_P2P_PROPERTY_FLAG_ACCESS;
    }
    return ZE_RESULT_SUCCESS;
}

ZE_APIEXPORT ze_result_t ZE_APICALL
SHIM_CALL(zeDeviceCanAccessPeer)(
    ze_device_handle_t hDevice,                     ///< [in] handle of the device performing the access
    ze_device_handle_t hPeerDevice,                 ///< [in] handle of the peer device with the allocation
    ze_bool_t* value                                ///< [out] returned access capability
    )
{
    if (value)
    {
        *value = true;
    }
    return ZE_RESULT_SUCCESS;
}

ZE_APIEXPORT ze_result_t ZE_APICALL
SHIM_CALL(zeKernelSuggestGroupSize)(
    ze_kernel_handle_t hKernel,                     ///< [in] handle of the kernel object
    uint32_t globalSizeX,                           ///< [in] global width for X dimension
    uint32_t globalSizeY,                           ///< [in] global width for Y dimension
    uint32_t globalSizeZ,                           ///< [in] global width for Z dimension
    uint32_t* groupSizeX,                           ///< [out] recommended size of group for X dimension
    uint32_t* groupSizeY,                           ///< [out] recommended size of group for Y dimension
    uint32_t* groupSizeZ                            ///< [out] recommended size of group for Z dimension
    )
{
    if (groupSizeX)
    {
        *groupSizeX = 1;
    }
    if (groupSizeY)
    {
        *groupSizeY = 1;
    }
    if (groupSizeZ)
    {
        *groupSizeZ = 1;
    }
    return ZE_RESULT_SUCCESS;
}

ZE_APIEXPORT ze_result_t ZE_APICALL
SHIM_CALL(zeDeviceGetCommandQueueGroupProperties)(
    ze_device_handle_t hDevice,                     ///< [in] handle of the device
    uint32_t* pCount,                               ///< [in,out] pointer to the number of command queue group properties.
                                                    ///< if count is zero, then the driver will update the value with the total
                                                    ///< number of command queue group properties available.
                                                    ///< if count is non-zero, then driver will only retrieve that number of
                                                    ///< command queue group properties.
                                                    ///< if count is larger than the number of command queue group properties
                                                    ///< available, then the driver will update the value with the correct
                                                    ///< number of command queue group properties available.
    ze_command_queue_group_properties_t* pCommandQueueGroupProperties   ///< [in,out][optional][range(0, *pCount)] array of query results for
                                                    ///< command queue group properties
    )
{
    if (!pCount)
    {
        return ZE_RESULT_ERROR_INVALID_NULL_POINTER;
    }

	if (pCommandQueueGroupProperties)
    {
        constexpr int numQueues = 2;
        for (int i = 0; i < numQueues; i++) {
            pCommandQueueGroupProperties[i].stype = ZE_STRUCTURE_TYPE_COMMAND_QUEUE_GROUP_PROPERTIES;
            pCommandQueueGroupProperties[i].flags =
                ZE_COMMAND_QUEUE_GROUP_PROPERTY_FLAG_COPY |
                ZE_COMMAND_QUEUE_GROUP_PROPERTY_FLAG_COMPUTE;
            pCommandQueueGroupProperties[i].numQueues = numQueues;
        }
    }

    if (*pCount == 0 || *pCount > 2)
    {
        *pCount = 2;
    }

    return ZE_RESULT_SUCCESS;
}
