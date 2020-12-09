/*===================== begin_copyright_notice ==================================

 Copyright (c) 2020, Intel Corporation


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

#include <vector>
#include <string>
#include <map>
#include <unordered_set>
#include <unordered_map>
#include <algorithm>

// OpenCL
#include <CL/cl.h>
// CM
#include <cm_rt.h>

#include "kernel_utils.h"
#include "shim.h"

#define SHIM_CALL(x) x

#define SET_ERRCODE(v) if (errcode_ret) *errcode_ret = (v)

using namespace std::string_literals;

// but we can hide details behind a ptr to a C++ class

class RefCounter
{
    size_t count = 1;

public:
    void AddRef() { ++count; }
    void Release()
    {
        if (count > 0)
        {
            --count;
        }
    }
    bool IsZero()
    {
        return count == 0;
    }

    RefCounter() = default;
    ~RefCounter() = default;
};

class UsersTracker
{
    std::unordered_set<void*> users;

public:
    void AddUser(void* user)
    {
        users.insert(user);
    }
    void DeleteUser(void* user)
    {
        users.erase(user);
    }
    bool AreUsersPresent()
    {
        return !users.empty();
    }

    UsersTracker() = default;
    ~UsersTracker() = default;
};

class CmDefaults
{
    bool m_overrideDefaults = false;
    uint32_t m_residentGroupNum = 0;
    uint32_t m_parallelThreadNum = 0;
public:
    bool IsOverrideDefaults()
    {
        return m_overrideDefaults;
    }
    void SetOverrideDefaults(bool v)
    {
        m_overrideDefaults = v;
    }

    void SetResidentGroupNum(uint32_t v)
    {
        m_residentGroupNum = v;
        SetOverrideDefaults(true);
    }
    uint32_t GetResidentGroupNum()
    {
        return m_residentGroupNum;
    }

    void SetParallelThreadNum(uint32_t v)
    {
        m_parallelThreadNum = v;
        SetOverrideDefaults(true);
    }
    uint32_t GetParallelThreadNum()
    {
        return m_parallelThreadNum;
    }

    CmDefaults() = default;
    ~CmDefaults() = default;
};

struct _cl_platform_id
{
    std::string name = "Intel\0"s;
    std::string version = "OpenCL 2.0 Emulated\0"s;
    CmDefaults cmDefaults;
} CM_PLATFORM;
cl_platform_id CM_PLATFORM_ID = &CM_PLATFORM;

struct _cl_device_id
{
    std::string name = "SKL\0"s;
    std::string version = "OpenCL 2.0 Emulated\0"s;
    std::string c_version = "OpenCL C 2.0 Emulated\0"s;
} CM_DEVICE;
cl_device_id CM_DEVICE_ID = &CM_DEVICE;

struct _cl_context
{
    CmDevice *cm_device = nullptr;
    RefCounter ref_ctr;
};

struct _cl_command_queue
{
    CmQueue *cm_queue = nullptr;
    cl_context context = nullptr;
    RefCounter ref_ctr;
};

struct _cl_mem
{
    enum BufferType
    {
        Buffer, Image2d, Image3d
    } type = Buffer;
    union
    {
        CmBuffer *cm_buf = nullptr;
        CmSurface2D *cm_surf2d;
        CmSurface3D *cm_surf3d;
    } v;
    int getIndex(SurfaceIndex *&idx)
    {
        switch (type)
        {
        case Image2d:
            return v.cm_surf2d->GetIndex(idx);
        case Image3d:
            return v.cm_surf3d->GetIndex(idx);
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
            return context->cm_device->DestroySurface(v.cm_surf2d);
        case Image3d:
            return context->cm_device->DestroySurface(v.cm_surf3d);
        default:
        case Buffer:
            return context->cm_device->DestroySurface(v.cm_buf);
        }
    }
    cl_context context = nullptr;
    RefCounter ref_ctr;
};

struct _cl_event
{
    CmEvent *cm_event = nullptr;
    CmQueue *owner_queue = nullptr;
    cl_context context = nullptr;
    bool is_fake = false;
    RefCounter ref_ctr;
};

struct _cl_program
{
    enum SourceType
    {
        SOURCE, BINARY
    } type;
    ProgramInfo info;
    std::string kernel_names_cache;
    CmProgram *cm_program = nullptr;
    cl_context context = nullptr;
    RefCounter ref_ctr;
    UsersTracker users;
};

struct _cl_kernel
{
    CmKernel *cm_kernel;
    std::vector<CmArgumentType> params;
    cl_context context = nullptr;
    cl_program program = nullptr;
    RefCounter ref_ctr;
};

extern "C"
{

void SetDefaultResidentGroupAndParallelThreadNum(uint32_t residentGroupNum,
                                                 uint32_t parallelThreadNum)
{
    CM_PLATFORM.cmDefaults.SetParallelThreadNum(parallelThreadNum);
    CM_PLATFORM.cmDefaults.SetResidentGroupNum(residentGroupNum);
}

void SetResidentGroupAndParallelThreadNumForQueue(cl_command_queue q,
                                                  uint32_t residentGroupNum,
                                                  uint32_t parallelThreadNum)
{
    if (q && q->cm_queue)
    {
        q->cm_queue->SetResidentGroupAndParallelThreadNum(residentGroupNum, parallelThreadNum);
    }
}

CL_API_ENTRY cl_int CL_API_CALL
    SHIM_CALL(clGetPlatformIDs)(cl_uint          num_entries,
                                cl_platform_id * platforms,
                                cl_uint *        num_platforms) CL_API_SUFFIX__VERSION_1_0
{
    if (isHwMode())
    {
#ifdef HAS_OCL_HW_SUPPORT
        return ::clGetPlatformIDs(num_entries, platforms, num_platforms);
#else /* HAS_OCL_HW_SUPPORT */
        return CL_INVALID_VALUE;
#endif /* HAS_OCL_HW_SUPPORT */
    }
    else
    {
        if (platforms && (num_entries == 0))
        {
            return CL_INVALID_VALUE;
        }
        if ((platforms == NULL) && (num_platforms == NULL))
        {
            return CL_INVALID_VALUE;
        }
        if (num_platforms)
        {
            *num_platforms = 1;
        }
        if (platforms)
        {
            *platforms = CM_PLATFORM_ID;
        }
        return CL_SUCCESS;
    }
}

CL_API_ENTRY cl_int CL_API_CALL
SHIM_CALL(clGetDeviceIDs)(cl_platform_id platform,
                          cl_device_type   device_type,
                          cl_uint          num_entries,
                          cl_device_id *   devices,
                          cl_uint *        num_devices) CL_API_SUFFIX__VERSION_1_0
{
    if (isHwMode())
    {
#ifdef HAS_OCL_HW_SUPPORT
        return ::clGetDeviceIDs(platform, device_type, num_entries, devices, num_devices);
#else /* HAS_OCL_HW_SUPPORT */
        return CL_INVALID_PLATFORM;
#endif /* HAS_OCL_HW_SUPPORT */
    }
    else
    {
        if (platform != CM_PLATFORM_ID)
        {
            return CL_INVALID_PLATFORM;
        }
        if (devices && (num_entries == 0))
        {
            return CL_INVALID_VALUE;
        }
        if ((devices == NULL) && (num_devices == NULL))
        {
            return CL_INVALID_VALUE;
        }

        if (num_devices)
        {
            *num_devices = 1;
        }
        if (devices)
        {
            *devices = CM_DEVICE_ID;
        }
        return CL_SUCCESS;
    }
}

CL_API_ENTRY cl_context CL_API_CALL
SHIM_CALL(clCreateContext)(const cl_context_properties * properties,
                           cl_uint                 num_devices,
                           const cl_device_id *    devices,
                           void (CL_CALLBACK * pfn_notify)(const char *, const void *, size_t, void *),
                           void *                  user_data,
                           cl_int *                errcode_ret) CL_API_SUFFIX__VERSION_1_0
{
    if (isHwMode())
    {
#ifdef HAS_OCL_HW_SUPPORT
        return ::clCreateContext(properties, num_devices, devices, pfn_notify, user_data, errcode_ret);
#else /* HAS_OCL_HW_SUPPORT */
        SET_ERRCODE(CL_OUT_OF_RESOURCES);
        return nullptr;
#endif /* HAS_OCL_HW_SUPPORT */
    }
    else
    {
        SET_ERRCODE(CL_OUT_OF_HOST_MEMORY);
        if ((num_devices != 1) || (*devices != CM_DEVICE_ID))
        {
            SET_ERRCODE(CL_OUT_OF_RESOURCES);
        }
        CmDevice *device = nullptr;
        unsigned int version = 0;
        int result = ::CreateCmDevice(device, version);
        if ((result != CM_SUCCESS) || (version < CM_1_0))
        {
            SET_ERRCODE(CL_OUT_OF_RESOURCES);
        }
        else
        {
            cl_context ctx = new _cl_context;
            ctx->cm_device = device;
            SET_ERRCODE(CL_SUCCESS);
            return ctx;
        }
        return nullptr;
    }
}

CL_API_ENTRY cl_command_queue CL_API_CALL
SHIM_CALL(clCreateCommandQueueWithProperties)(cl_context context,
                                              cl_device_id device,
                                              const cl_queue_properties *properties,
                                              cl_int *errcode_ret) CL_API_SUFFIX__VERSION_2_0
{
    if (isHwMode())
    {
#ifdef HAS_OCL_HW_SUPPORT
        return ::clCreateCommandQueueWithProperties(context, device, properties, errcode_ret);
#else /* HAS_OCL_HW_SUPPORT */
        SET_ERRCODE(CL_OUT_OF_RESOURCES);
        return nullptr;
#endif /* HAS_OCL_HW_SUPPORT */
    }
    else
    {
        SET_ERRCODE(CL_OUT_OF_HOST_MEMORY);
        if ((context == nullptr) || (context->cm_device == nullptr))
        {
            SET_ERRCODE(CL_INVALID_CONTEXT);
        }
        if (device != CM_DEVICE_ID)
        {
            SET_ERRCODE(CL_INVALID_DEVICE);
        }
        CmQueue *queue = nullptr;
        int result = context->cm_device->CreateQueue(queue);
        if (result != CM_SUCCESS)
        {
            SET_ERRCODE(CL_OUT_OF_RESOURCES);
        }
        else
        {
            if (CM_PLATFORM.cmDefaults.IsOverrideDefaults())
            {
                queue->SetResidentGroupAndParallelThreadNum(
                    CM_PLATFORM.cmDefaults.GetResidentGroupNum(),
                    CM_PLATFORM.cmDefaults.GetParallelThreadNum());
            }
            cl_command_queue q = new _cl_command_queue;
            q->cm_queue = queue;
            q->context = context;
            SET_ERRCODE(CL_SUCCESS);
            return q;
        }
        return nullptr;
    }
}

CL_API_ENTRY cl_mem CL_API_CALL
SHIM_CALL(clCreateBuffer)(cl_context   context,
                          cl_mem_flags flags,
                          size_t       size,
                          void *       host_ptr,
                          cl_int *     errcode_ret) CL_API_SUFFIX__VERSION_1_0
{
    if (isHwMode())
    {
#ifdef HAS_OCL_HW_SUPPORT
        return ::clCreateBuffer(context, flags, size, host_ptr, errcode_ret);
#else /* HAS_OCL_HW_SUPPORT */
        SET_ERRCODE(CL_OUT_OF_RESOURCES);
        return nullptr;
#endif /* HAS_OCL_HW_SUPPORT */
    }
    else
    {
        /// HACKY
        SET_ERRCODE(CL_OUT_OF_HOST_MEMORY);
        CmBuffer *buf = nullptr;
        int status = context->cm_device->CreateBuffer(static_cast<unsigned int>(size), buf);
        if (status != CM_SUCCESS)
        {
            SET_ERRCODE(CL_OUT_OF_RESOURCES);
            return nullptr;
        }
        if (host_ptr)
        {
            status = buf->WriteSurface(reinterpret_cast<const unsigned char*>(host_ptr),
                nullptr, size);
        }
        cl_mem cl_buffer = new _cl_mem;
        cl_buffer->type = _cl_mem::Buffer;
        cl_buffer->v.cm_buf = buf;
        cl_buffer->context = context;
        cl_int retcode = (status == CM_SUCCESS) ? CL_SUCCESS : CL_INVALID_HOST_PTR;
        SET_ERRCODE(retcode);
        return cl_buffer;
    }
}

CL_API_ENTRY cl_int CL_API_CALL
SHIM_CALL(clEnqueueWriteBuffer)(cl_command_queue   command_queue,
        cl_mem             buffer,
        cl_bool            blocking_write,
        size_t             offset,
        size_t             size,
        const void *       ptr,
        cl_uint            num_events_in_wait_list,
        const cl_event *   event_wait_list,
        cl_event *         event) CL_API_SUFFIX__VERSION_1_0
{
    if (isHwMode())
    {
#ifdef HAS_OCL_HW_SUPPORT
        return ::clEnqueueWriteBuffer(command_queue, buffer, blocking_write,
            offset, size, ptr, num_events_in_wait_list, event_wait_list, event);
#else /* HAS_OCL_HW_SUPPORT */
        return CL_OUT_OF_RESOURCES;
#endif /* HAS_OCL_HW_SUPPORT */
    }
    else
    {
        /// HACKY
        // writing size bytes from ptr to buffer
        int status = buffer->v.cm_buf->WriteSurface(reinterpret_cast<const unsigned char*>(ptr), nullptr, size);
        if (event)
        {
            cl_event cl_e = new _cl_event;
            cl_e->is_fake = true;
            *event = cl_e;
        }
        if (status != CM_SUCCESS)
        {
            return CL_OUT_OF_HOST_MEMORY;
        }
        if (blocking_write == CL_TRUE)
        {
            // wait
        }
        // currently all calls are blocking
        // wait lists and events are not supported yet
        return CL_SUCCESS;
    }
}

CL_API_ENTRY cl_program CL_API_CALL
SHIM_CALL(clCreateProgramWithSource)(cl_context        context,
        cl_uint           count,
        const char **     strings,
        const size_t *    lengths,
        cl_int *          errcode_ret) CL_API_SUFFIX__VERSION_1_0
{
    if (isHwMode())
    {
#ifdef HAS_OCL_HW_SUPPORT
        return ::clCreateProgramWithSource(context, count, strings, lengths, errcode_ret);
#else /* HAS_OCL_HW_SUPPORT */
        SET_ERRCODE(CL_OUT_OF_HOST_MEMORY);
        return nullptr;
#endif /* HAS_OCL_HW_SUPPORT */
    }
    else
    {
        SET_ERRCODE(CL_OUT_OF_HOST_MEMORY);
        return nullptr;
    }
}

CL_API_ENTRY cl_program CL_API_CALL
SHIM_CALL(clCreateProgramWithBinary)(
    cl_context context,
    cl_uint                        num_devices,
    const cl_device_id *           device_list,
    const size_t *                 lengths,
    const unsigned char **         binaries,
    cl_int *                       binary_status,
    cl_int *                       errcode_ret) CL_API_SUFFIX__VERSION_1_0
{
    if (isHwMode())
    {
#ifdef HAS_OCL_HW_SUPPORT
        return ::clCreateProgramWithBinary(context, num_devices, device_list, lengths, binaries, binary_status, errcode_ret);
#else /* HAS_OCL_HW_SUPPORT */
        SET_ERRCODE(CL_OUT_OF_HOST_MEMORY);
        return nullptr;
#endif /* HAS_OCL_HW_SUPPORT */
    }
    else
    {
        SET_ERRCODE(CL_OUT_OF_HOST_MEMORY);
        // HACKY
        CmProgram *p = nullptr;
        int status = context->cm_device->LoadProgram(0, 0, p);
        if (status != CM_SUCCESS)
        {
            SET_ERRCODE(CL_OUT_OF_RESOURCES);
            return nullptr;
        }
        cl_program program = new _cl_program;
        program->type = _cl_program::BINARY;
        program->cm_program = p;
        program->context = context;
        // here (below) we copy a whole vector of kernel data. This can be avoided by adding
        // a proper constructor for _cl_program,
        // but since clCreateProgram is not that frequent, let's skip it for now
        program->info = ProgramManager::instance().AddProgram(binaries[0], lengths[0]);
        bool success = program->info.isValid();
        cl_int retcode = success ? CL_SUCCESS : CL_INVALID_BINARY;
        SET_ERRCODE(retcode);
        if (binary_status != nullptr)
        {
            *binary_status = success ? CL_SUCCESS : CL_INVALID_BINARY;
        }
        return program;
    }
}

CL_API_ENTRY cl_int CL_API_CALL
SHIM_CALL(clBuildProgram)(cl_program           program,
        cl_uint              num_devices,
        const cl_device_id * device_list,
        const char *         options,
        void (CL_CALLBACK *  pfn_notify)(cl_program program, void * user_data),
        void *               user_data) CL_API_SUFFIX__VERSION_1_0
{
    if (isHwMode())
    {
#ifdef HAS_OCL_HW_SUPPORT
        return ::clBuildProgram(program, num_devices, device_list, options, pfn_notify, user_data);
#else /* HAS_OCL_HW_SUPPORT */
        return CL_OUT_OF_RESOURCES;
#endif /* HAS_OCL_HW_SUPPORT */
    }
    else
    {
        bool success = ProgramManager::instance().IsProgramValid(program->info.handle);
        return success ?
            CL_SUCCESS :
            CL_BUILD_PROGRAM_FAILURE;
    }
}

CL_API_ENTRY cl_int CL_API_CALL
SHIM_CALL(clGetProgramBuildInfo)(cl_program            program,
        cl_device_id          device,
        cl_program_build_info param_name,
        size_t                param_value_size,
        void *                param_value,
        size_t *              param_value_size_ret) CL_API_SUFFIX__VERSION_1_0
{
    if (isHwMode())
    {
#ifdef HAS_OCL_HW_SUPPORT
        return ::clGetProgramBuildInfo(program, device, param_name, param_value_size, param_value, param_value_size_ret);
#else /* HAS_OCL_HW_SUPPORT */
        return CL_OUT_OF_HOST_MEMORY;
#endif /* HAS_OCL_HW_SUPPORT */
    }
    else
    {
        // HACKY
        if (param_name == CL_PROGRAM_BUILD_LOG)
        {
            const char *log = "something bad happened...\n";
            if (param_value_size_ret)
            {
                *param_value_size_ret = strlen(log) + 1;
            }
            if (param_value)
            {
                strncpy((char*)param_value, log, param_value_size);
            }
            return CL_SUCCESS;
        }
        return CL_OUT_OF_HOST_MEMORY;
    }
}

CL_API_ENTRY cl_kernel CL_API_CALL
SHIM_CALL(clCreateKernel)(cl_program      program,
        const char *    kernel_name,
        cl_int *        errcode_ret) CL_API_SUFFIX__VERSION_1_0
{
    if (isHwMode())
    {
#ifdef HAS_OCL_HW_SUPPORT
        return ::clCreateKernel(program, kernel_name, errcode_ret);
#else /* HAS_OCL_HW_SUPPORT */
        SET_ERRCODE(CL_OUT_OF_RESOURCES);
        return nullptr;
#endif /* HAS_OCL_HW_SUPPORT */
    }
    else
    {
        auto kernel_data_it = program->info.kernels.find(kernel_name);
        if (kernel_data_it == program->info.kernels.end())
        {
            SET_ERRCODE(CL_OUT_OF_RESOURCES);
            return nullptr;
        }
        void *proc = kernel_data_it->second.entry_point;
        if (proc == nullptr)
        {
            SET_ERRCODE(CL_OUT_OF_RESOURCES);
            return nullptr;
        }
        CmKernel *k = nullptr;
        int status = program->context->cm_device->CreateKernel(program->cm_program, kernel_name, proc, k);
        if (status != CM_SUCCESS)
        {
            SET_ERRCODE(CL_OUT_OF_RESOURCES);
            return nullptr;
        }
        cl_kernel kernel = new _cl_kernel;
        kernel->cm_kernel = k;
        kernel->context = program->context;
        kernel->program = program;
        kernel->program->users.AddUser(kernel);
        kernel->params = kernel_data_it->second.args;
        SET_ERRCODE(CL_SUCCESS);
        return kernel;
    }
}

CL_API_ENTRY cl_int CL_API_CALL
SHIM_CALL(clSetKernelArg)(cl_kernel    kernel,
        cl_uint      arg_index,
        size_t       arg_size,
        const void * arg_value) CL_API_SUFFIX__VERSION_1_0
{
    if (isHwMode())
    {
#ifdef HAS_OCL_HW_SUPPORT
        return ::clSetKernelArg(kernel, arg_index, arg_size, arg_value);
#else /* HAS_OCL_HW_SUPPORT */
        return CL_OUT_OF_RESOURCES;
#endif /* HAS_OCL_HW_SUPPORT */
    }
    else
    {
        int status = CM_FAILURE;
        switch (kernel->params[arg_index])
        {
        case CmArgumentType::SurfaceIndex:
        {
            cl_mem arg = *reinterpret_cast<const cl_mem*>(arg_value);
            SurfaceIndex *idx = nullptr;
            status = arg->getIndex(idx);
            if (status != CM_SUCCESS)
            {
                break;
            }
            status = kernel->cm_kernel->SetKernelArg(arg_index, sizeof(SurfaceIndex), idx);
            break;
        }
        case CmArgumentType::Scalar:
        default:
            status = kernel->cm_kernel->SetKernelArg(arg_index, arg_size, arg_value);
            break;
        }
        return status == CM_SUCCESS ?
            CL_SUCCESS :
            CL_OUT_OF_RESOURCES;
    }
}

CL_API_ENTRY cl_int CL_API_CALL
SHIM_CALL(clSetKernelArgSVMPointer)(cl_kernel    kernel,
    cl_uint      arg_index,
    const void * arg_value) CL_API_SUFFIX__VERSION_2_0
{
    if (isHwMode())
    {
#ifdef HAS_OCL_HW_SUPPORT
        return ::clSetKernelArgSVMPointer(kernel, arg_index, arg_value);
#else /* HAS_OCL_HW_SUPPORT */
        return CL_OUT_OF_RESOURCES;
#endif /* HAS_OCL_HW_SUPPORT */
    }
    else
    {
        return SHIM_CALL(clSetKernelArg)(kernel, arg_index, sizeof(void*), &arg_value);
        /*int status = kernel->cm_kernel->SetKernelArgPointer(arg_index, sizeof(void*), arg_value);
        return status == CM_SUCCESS ?
            CL_SUCCESS :
            CL_OUT_OF_RESOURCES;
            */
    }
}

CL_API_ENTRY cl_int CL_API_CALL
SHIM_CALL(clEnqueueNDRangeKernel)(cl_command_queue command_queue,
        cl_kernel        kernel,
        cl_uint          work_dim,
        const size_t *   global_work_offset,
        const size_t *   global_work_size,
        const size_t *   local_work_size,
        cl_uint          num_events_in_wait_list,
        const cl_event * event_wait_list,
        cl_event *       event) CL_API_SUFFIX__VERSION_1_0
{
    if (isHwMode())
    {
#ifdef HAS_OCL_HW_SUPPORT
        return ::clEnqueueNDRangeKernel(command_queue, kernel, work_dim, global_work_offset,
            global_work_size, local_work_size, num_events_in_wait_list, event_wait_list, event);
#else /* HAS_OCL_HW_SUPPORT */
        return CL_OUT_OF_RESOURCES;
#endif /* HAS_OCL_HW_SUPPORT */
    }
    else
    {
        // enqueue kernel to command queue, iter space = global_work_size[0]*...*global_work_size[work_dim - 1]
        // wg_size = local_work_size[0]*...*local_work_size[work_dim - 1]
        CmTask *t = nullptr;
        int status = command_queue->context->cm_device->CreateTask(t);
        if (status != CM_SUCCESS)
        {
            return CL_OUT_OF_RESOURCES;
        }
        if (work_dim > 2)
        {
            // do not support 3-dimensional enqueue yet
            return CL_OUT_OF_RESOURCES;
        }
        CmThreadGroupSpace *tgs = nullptr;
        unsigned int t_w = local_work_size[0];
        unsigned int t_h = work_dim > 1 ? local_work_size[1] : 1;
        unsigned int tg_w = global_work_size[0] / t_w;
        unsigned int tg_h = work_dim > 1 ? global_work_size[1] / t_h : 1;
        status = command_queue->context->cm_device->CreateThreadGroupSpace(t_w, t_h, tg_w, tg_h, tgs);
        if (status != CM_SUCCESS)
        {
            command_queue->context->cm_device->DestroyTask(t);
            return CL_OUT_OF_RESOURCES;
        }
        status = t->AddKernel(kernel->cm_kernel);
        if (status != CM_SUCCESS)
        {
            command_queue->context->cm_device->DestroyTask(t);
            return CL_OUT_OF_RESOURCES;
        }
        CmEvent *e = nullptr;
        status = command_queue->cm_queue->EnqueueWithGroup(t, e, tgs);
        if (event)
        {
            cl_event cl_e = new _cl_event;
            cl_e->cm_event = e;
            cl_e->owner_queue = command_queue->cm_queue;
            cl_e->context = command_queue->context;
            *event = cl_e;
        }

        return status == CM_SUCCESS?
            CL_SUCCESS:
            CL_OUT_OF_RESOURCES;
    }
}

CL_API_ENTRY cl_int CL_API_CALL
SHIM_CALL(clEnqueueReadBuffer)(cl_command_queue    command_queue,
        cl_mem              buffer,
        cl_bool             blocking_read,
        size_t              offset,
        size_t              size,
        void *              ptr,
        cl_uint             num_events_in_wait_list,
        const cl_event *    event_wait_list,
        cl_event *          event) CL_API_SUFFIX__VERSION_1_0
{
    if (isHwMode())
    {
#ifdef HAS_OCL_HW_SUPPORT
        return ::clEnqueueReadBuffer(command_queue, buffer, blocking_read,
            offset, size, ptr, num_events_in_wait_list, event_wait_list, event);
#else /* HAS_OCL_HW_SUPPORT */
        return CL_OUT_OF_RESOURCES;
#endif /* HAS_OCL_HW_SUPPORT */
    }
    else
    {
        /// HACKY
        // reading size bytes from ptr to buffer
        int status = buffer->v.cm_buf->ReadSurface(reinterpret_cast<unsigned char*>(ptr), nullptr, size);
        if (event)
        {
            cl_event cl_e = new _cl_event;
            cl_e->is_fake = true;
            *event = cl_e;
        }
        if (status != CM_SUCCESS)
        {
            return CL_OUT_OF_HOST_MEMORY;
        }
        if (blocking_read == CL_TRUE)
        {
            // wait
        }
        // currently all calls are blocking
        // wait lists and events are not supported yet
        return CL_SUCCESS;
    }
}

CL_API_ENTRY cl_int CL_API_CALL
SHIM_CALL(clWaitForEvents)(cl_uint num_events,
    const cl_event *event_list) CL_API_SUFFIX__VERSION_1_0
{
    if (isHwMode())
    {
#ifdef HAS_OCL_HW_SUPPORT
        return ::clWaitForEvents(num_events, event_list);
#else /* HAS_OCL_HW_SUPPORT */
        return CL_OUT_OF_RESOURCES;
#endif /* HAS_OCL_HW_SUPPORT */
    }
    else
    {
        ///HACKY
        for (int i = 0; i < num_events; ++i)
        {
            if (event_list[i]->is_fake)
            {
                continue;
            }
            int result = event_list[i]->cm_event->WaitForTaskFinished();
            if (result != CM_SUCCESS)
            {
                return CL_OUT_OF_RESOURCES;
            }
        }
        return CL_SUCCESS;
    }
}

/**** Releasers ****/

CL_API_ENTRY cl_int CL_API_CALL
SHIM_CALL(clReleaseKernel)(cl_kernel kernel) CL_API_SUFFIX__VERSION_1_0
{
    if (isHwMode())
    {
#ifdef HAS_OCL_HW_SUPPORT
        return ::clReleaseKernel(kernel);
#else /* HAS_OCL_HW_SUPPORT */
        return CL_OUT_OF_RESOURCES;
#endif /* HAS_OCL_HW_SUPPORT */
    }
    else
    {
        if (kernel == nullptr)
        {
            return CL_INVALID_KERNEL;
        }
        kernel->ref_ctr.Release();
        if (!kernel->ref_ctr.IsZero())
        {
            return CL_SUCCESS;
        }
        kernel->program->users.DeleteUser(kernel);
        kernel->context->cm_device->DestroyKernel(kernel->cm_kernel);
        delete kernel;
        return CL_SUCCESS;
    }
}

CL_API_ENTRY cl_int CL_API_CALL
SHIM_CALL(clReleaseProgram)(cl_program program) CL_API_SUFFIX__VERSION_1_0
{
    if (isHwMode())
    {
#ifdef HAS_OCL_HW_SUPPORT
        return ::clReleaseProgram(program);
#else /* HAS_OCL_HW_SUPPORT */
        return CL_OUT_OF_RESOURCES;
#endif /* HAS_OCL_HW_SUPPORT */
    }
    else
    {

        program->ref_ctr.Release();
        if (!program->ref_ctr.IsZero() || program->users.AreUsersPresent())
        {
            return CL_SUCCESS;
        }
        bool success = ProgramManager::instance().FreeProgram(program->info.handle);
        return success ?
            CL_SUCCESS:
            CL_INVALID_PROGRAM;
    }
}

CL_API_ENTRY cl_int CL_API_CALL
SHIM_CALL(clReleaseCommandQueue)(cl_command_queue command_queue) CL_API_SUFFIX__VERSION_1_0
{
    if (isHwMode())
    {
#ifdef HAS_OCL_HW_SUPPORT
        return ::clReleaseCommandQueue(command_queue);
#else /* HAS_OCL_HW_SUPPORT */
        return CL_OUT_OF_RESOURCES;
#endif /* HAS_OCL_HW_SUPPORT */
    }
    else
    {
        if ((command_queue == nullptr) || (command_queue->cm_queue == nullptr))
        {
            return CL_INVALID_COMMAND_QUEUE;
        }
        command_queue->ref_ctr.Release();
        if (!command_queue->ref_ctr.IsZero())
        {
            return CL_SUCCESS;
        }
        delete command_queue;
        return CL_SUCCESS;
    }
}

CL_API_ENTRY cl_int CL_API_CALL
SHIM_CALL(clReleaseMemObject)(cl_mem memobj) CL_API_SUFFIX__VERSION_1_0
{
    if (isHwMode())
    {
#ifdef HAS_OCL_HW_SUPPORT
        return ::clReleaseMemObject(memobj);
#else /* HAS_OCL_HW_SUPPORT */
        return CL_OUT_OF_RESOURCES;
#endif /* HAS_OCL_HW_SUPPORT */
    }
    else
    {
        if (memobj == nullptr)
        {
            return CL_INVALID_MEM_OBJECT;
        }
        memobj->ref_ctr.Release();
        if (!memobj->ref_ctr.IsZero())
        {
            return CL_SUCCESS;
        }

        int status = memobj->Release();
        delete memobj;
        if (status != CM_SUCCESS)
        {
            return CL_INVALID_MEM_OBJECT;
        }
        return CL_SUCCESS;
    }
}

CL_API_ENTRY cl_int CL_API_CALL
SHIM_CALL(clReleaseContext)(cl_context context) CL_API_SUFFIX__VERSION_1_0
{
    if (isHwMode())
    {
#ifdef HAS_OCL_HW_SUPPORT
        return ::clReleaseContext(context);
#else /* HAS_OCL_HW_SUPPORT */
        return CL_OUT_OF_RESOURCES;
#endif /* HAS_OCL_HW_SUPPORT */
    }
    else
    {
        if ((context == nullptr) || (context->cm_device == nullptr))
        {
            return CL_INVALID_CONTEXT;
        }
        context->ref_ctr.Release();
        if (!context->ref_ctr.IsZero())
        {
            return CL_SUCCESS;
        }
        int status = ::DestroyCmDevice(context->cm_device);
        delete context;
        if (status != CM_SUCCESS)
        {
            return CL_INVALID_CONTEXT;
        }
        return CL_SUCCESS;
    }
}

CL_API_ENTRY cl_int CL_API_CALL
SHIM_CALL(clReleaseEvent)(cl_event event) CL_API_SUFFIX__VERSION_1_0
{
    if (isHwMode())
    {
#ifdef HAS_OCL_HW_SUPPORT
        return ::clReleaseEvent(event);
#else /* HAS_OCL_HW_SUPPORT */
        return CL_OUT_OF_RESOURCES;
#endif /* HAS_OCL_HW_SUPPORT */
    }
    else
    {

        event->ref_ctr.Release();
        if (!event->ref_ctr.IsZero())
        {
            return CL_SUCCESS;
        }
        if (!event->is_fake)
        {
            if ((event->cm_event == nullptr) || (event->owner_queue == nullptr))
            {
                return CL_INVALID_EVENT;
            }
            int status = event->owner_queue->DestroyEvent(event->cm_event);
            if (status != CM_SUCCESS)
            {
                return CL_INVALID_EVENT;
            }
        }
        delete event;
        return CL_SUCCESS;
    }
}

CL_API_ENTRY cl_int CL_API_CALL
SHIM_CALL(clFinish)(cl_command_queue command_queue) CL_API_SUFFIX__VERSION_1_0
{
    if (isHwMode())
    {
#ifdef HAS_OCL_HW_SUPPORT
        return ::clFinish(command_queue);
#else /* HAS_OCL_HW_SUPPORT */
        return CL_OUT_OF_RESOURCES;
#endif /* HAS_OCL_HW_SUPPORT */
    }
    else
    {

        return CL_SUCCESS;// CL_OUT_OF_RESOURCES;
    }
}

CL_API_ENTRY cl_int CL_API_CALL
SHIM_CALL(clEnqueueSVMMap)(cl_command_queue  command_queue,
    cl_bool           blocking_map,
    cl_map_flags      flags,
    void *            svm_ptr,
    size_t            size,
    cl_uint           num_events_in_wait_list,
    const cl_event *  event_wait_list,
    cl_event *        event) CL_API_SUFFIX__VERSION_2_0
{
    if (isHwMode())
    {
#ifdef HAS_OCL_HW_SUPPORT
        return ::clEnqueueSVMMap(command_queue, blocking_map, flags, svm_ptr, size, num_events_in_wait_list, event_wait_list, event);
#else /* HAS_OCL_HW_SUPPORT */
        return CL_OUT_OF_RESOURCES;
#endif /* HAS_OCL_HW_SUPPORT */
    }
    else
    {

        if (event)
        {
            cl_event cl_e = new _cl_event;
            cl_e->is_fake = true;
            *event = cl_e;
        }
        return CL_SUCCESS;
    }
}

CL_API_ENTRY cl_int CL_API_CALL
SHIM_CALL(clEnqueueSVMUnmap)(cl_command_queue  command_queue,
    void *            svm_ptr,
    cl_uint           num_events_in_wait_list,
    const cl_event *  event_wait_list,
    cl_event *        event) CL_API_SUFFIX__VERSION_2_0
{
    if (isHwMode())
    {
#ifdef HAS_OCL_HW_SUPPORT
        return ::clEnqueueSVMUnmap(command_queue, svm_ptr, num_events_in_wait_list, event_wait_list, event);
#else /* HAS_OCL_HW_SUPPORT */
        return CL_OUT_OF_RESOURCES;
#endif /* HAS_OCL_HW_SUPPORT */
    }
    else
    {

        if (event)
        {
            cl_event cl_e = new _cl_event;
            cl_e->is_fake = true;
            *event = cl_e;
        }
        return CL_SUCCESS;
    }
}

CL_API_ENTRY void * CL_API_CALL
SHIM_CALL(clSVMAlloc)(cl_context       context,
    cl_svm_mem_flags flags,
    size_t           size,
    cl_uint          alignment) CL_API_SUFFIX__VERSION_2_0
{
    if (isHwMode())
    {
#ifdef HAS_OCL_HW_SUPPORT
        return ::clSVMAlloc(context, flags, size, alignment);
#else /* HAS_OCL_HW_SUPPORT */
        return nullptr;
#endif /* HAS_OCL_HW_SUPPORT */
    }
    else
    {
        // HACKY

        //return _aligned_malloc(size, alignment);
        return ::malloc(size);
    }
}

extern CL_API_ENTRY void CL_API_CALL
SHIM_CALL(clSVMFree)(cl_context context,
                     void *svm_pointer) CL_API_SUFFIX__VERSION_2_0
{
    if (isHwMode())
    {
#ifdef HAS_OCL_HW_SUPPORT
        return ::clSVMFree(context, svm_pointer);
#else /* HAS_OCL_HW_SUPPORT */
        return;
#endif /* HAS_OCL_HW_SUPPORT */
    }
    else
    {
        // HACKY

        //return _aligned_free(size, alignment);
        ::free(svm_pointer);
    }
}

void fillInfoString(const std::string& value, void *dest, size_t *size_ret)
{
    char *destination = reinterpret_cast<char*>(dest);
    if (dest != nullptr)
    {
        std::copy(value.begin(), value.end(), destination);
        destination[value.size()] = '\0';
    }
    if (size_ret != nullptr)
    {
        *size_ret = value.size() + 1;
    }
}

void setCmDeviceNameFromEnv()
{
    CM_DEVICE.name = os::GetEnvVarValue("CM_RT_PLATFORM");
    std::transform(CM_DEVICE.name.begin(), CM_DEVICE.name.end(), CM_DEVICE.name.begin(),
                   [](unsigned char c){ return std::toupper(c); }
                  );
}

CL_API_ENTRY cl_int CL_API_CALL
SHIM_CALL(clGetDeviceInfo)(cl_device_id    device,
                           cl_device_info  param_name,
                           size_t          param_value_size,
                           void *          param_value,
                           size_t *        param_value_size_ret) CL_API_SUFFIX__VERSION_1_0
{
    if (isHwMode())
    {
#ifdef HAS_OCL_HW_SUPPORT
        return ::clGetDeviceInfo(device, param_name, param_value_size, param_value, param_value_size_ret);
#else /* HAS_OCL_HW_SUPPORT */
        return CL_OUT_OF_RESOURCES;
#endif /* HAS_OCL_HW_SUPPORT */
    }
    else
    {
        if (device != CM_DEVICE_ID)
        {
            return CL_OUT_OF_RESOURCES;
        }
        if ((param_value == nullptr) && (param_value_size_ret == nullptr))
        {
            return CL_OUT_OF_RESOURCES;
        }
        switch (param_name)
        {
        case CL_DEVICE_NAME:
            setCmDeviceNameFromEnv();
            fillInfoString(CM_DEVICE.name, param_value, param_value_size_ret);
            return CL_SUCCESS;
        case CL_DEVICE_VERSION:
            fillInfoString(CM_DEVICE.version, param_value, param_value_size_ret);
            return CL_SUCCESS;
        case CL_DEVICE_OPENCL_C_VERSION:
            fillInfoString(CM_DEVICE.c_version, param_value, param_value_size_ret);
            return CL_SUCCESS;
        case CL_DEVICE_PLATFORM:
            if (param_value != nullptr)
            {
                cl_platform_id *id = reinterpret_cast<cl_platform_id*>(param_value);
                *id = CM_PLATFORM_ID;
            }
            if (param_value_size_ret != nullptr)
            {
                *param_value_size_ret = sizeof(cl_platform_id);
            }
            return CL_SUCCESS;
        case CL_DEVICE_SVM_CAPABILITIES:
            if (param_value != nullptr)
            {
                cl_device_svm_capabilities *caps = reinterpret_cast<cl_device_svm_capabilities*>(param_value);
                *caps = CL_DEVICE_SVM_COARSE_GRAIN_BUFFER | CL_DEVICE_SVM_FINE_GRAIN_BUFFER;
            }
            if (param_value_size_ret != nullptr)
            {
                *param_value_size_ret = sizeof(cl_device_svm_capabilities);
            }
            return CL_SUCCESS;
        default:
            break;
        }
        return CL_OUT_OF_RESOURCES;
    }
}

CL_API_ENTRY cl_int CL_API_CALL
SHIM_CALL(clGetPlatformInfo)(cl_platform_id   platform,
                             cl_platform_info param_name,
                             size_t           param_value_size,
                             void *           param_value,
                             size_t *         param_value_size_ret) CL_API_SUFFIX__VERSION_1_0
{
    if (isHwMode())
    {
#ifdef HAS_OCL_HW_SUPPORT
        return ::clGetPlatformInfo(platform, param_name, param_value_size, param_value, param_value_size_ret);
#else /* HAS_OCL_HW_SUPPORT */
        return CL_OUT_OF_RESOURCES;
#endif /* HAS_OCL_HW_SUPPORT */
    }
    else
    {
        if (platform != CM_PLATFORM_ID)
        {
            return CL_OUT_OF_RESOURCES;
        }
        if ((param_value == nullptr) && (param_value_size_ret == nullptr))
        {
            return CL_OUT_OF_RESOURCES;
        }
        switch (param_name)
        {
        case CL_PLATFORM_NAME:
            fillInfoString(CM_PLATFORM.name, param_value, param_value_size_ret);
            return CL_SUCCESS;
        case CL_PLATFORM_VERSION:
            fillInfoString(CM_PLATFORM.version, param_value, param_value_size_ret);
            return CL_SUCCESS;
        default:
            break;
        }
        return CL_OUT_OF_RESOURCES;
    }
}

CL_API_ENTRY cl_int CL_API_CALL
SHIM_CALL(clGetKernelWorkGroupInfo)(cl_kernel kernel,
    cl_device_id               device,
    cl_kernel_work_group_info  param_name,
    size_t                     param_value_size,
    void *                     param_value,
    size_t *                   param_value_size_ret) CL_API_SUFFIX__VERSION_1_0
{
    if (isHwMode())
    {
#ifdef HAS_OCL_HW_SUPPORT
        return ::clGetKernelWorkGroupInfo(kernel, device, param_name, param_value_size, param_value, param_value_size_ret);
#else /* HAS_OCL_HW_SUPPORT */
        return CL_OUT_OF_RESOURCES;
#endif /* HAS_OCL_HW_SUPPORT */
    }
    else
    {
        return CL_OUT_OF_RESOURCES;
    }
}

CL_API_ENTRY cl_int CL_API_CALL
SHIM_CALL(clGetEventProfilingInfo)(cl_event event,
    cl_profiling_info   param_name,
    size_t              param_value_size,
    void *              param_value,
    size_t *            param_value_size_ret) CL_API_SUFFIX__VERSION_1_0
{
    if (isHwMode())
    {
#ifdef HAS_OCL_HW_SUPPORT
        return ::clGetEventProfilingInfo(event, param_name, param_value_size, param_value, param_value_size_ret);
#else /* HAS_OCL_HW_SUPPORT */
        return CL_OUT_OF_RESOURCES;
#endif /* HAS_OCL_HW_SUPPORT */
    }
    else
    {
        if (!event->cm_event || event->is_fake)
        {
            return CL_OUT_OF_RESOURCES;
        }
        switch (param_name)
        {
        case CL_PROFILING_COMMAND_START:
            {
                // NB: GetProfilingInfo() is not implemented in emu mode.
                //int status = event->cm_event->GetProfilingInfo(CM_EVENT_PROFILING_HWSTART, param_value_size, NULL, param_value);
                if (param_value_size_ret)
                {
                    *param_value_size_ret = param_value_size;
                }
                //return status == CM_SUCCESS ? CL_SUCCESS : CL_OUT_OF_RESOURCES;
                *reinterpret_cast<cl_ulong*>(param_value) = 0;
                return CL_SUCCESS;
            }
        case CL_PROFILING_COMMAND_END:
            {
                // NB: GetProfilingInfo() is not implemented in emu mode.
                //int status = event->cm_event->GetProfilingInfo(CM_EVENT_PROFILING_HWEND, param_value_size, NULL, param_value);
                // NB: GetProfilingInfo() is not implemented in emu mode.
                if (param_value_size_ret)
                {
                    *param_value_size_ret = param_value_size;
                }
                //return status == CM_SUCCESS ? CL_SUCCESS : CL_OUT_OF_RESOURCES;
                *reinterpret_cast<cl_ulong*>(param_value) = 100;
                return CL_SUCCESS;
            }
        default:
            break;
        }
        return CL_OUT_OF_RESOURCES;
    }
}

CL_API_ENTRY cl_int CL_API_CALL
SHIM_CALL(clRetainDevice)(cl_device_id device) CL_API_SUFFIX__VERSION_1_2
{
    if (isHwMode())
    {
#ifdef HAS_OCL_HW_SUPPORT
        return ::clRetainDevice(device);
#else /* HAS_OCL_HW_SUPPORT */
        return CL_OUT_OF_RESOURCES;
#endif /* HAS_OCL_HW_SUPPORT */
    }
    else
    {
        if (device == CM_DEVICE_ID)
        {
            return CL_SUCCESS;
        }
        else
        {
            return CL_OUT_OF_RESOURCES;
        }
    }
}

CL_API_ENTRY cl_int CL_API_CALL
SHIM_CALL(clRetainContext)(cl_context context) CL_API_SUFFIX__VERSION_1_0
{
    if (isHwMode())
    {
#ifdef HAS_OCL_HW_SUPPORT
        return ::clRetainContext(context);
#else /* HAS_OCL_HW_SUPPORT */
        return CL_OUT_OF_RESOURCES;
#endif /* HAS_OCL_HW_SUPPORT */
    }
    else
    {
        if (context)
        {
            context->ref_ctr.AddRef();
            return CL_SUCCESS;
        }
        else
        {
            return CL_INVALID_CONTEXT;
        }
    }
}

CL_API_ENTRY cl_int CL_API_CALL
SHIM_CALL(clRetainCommandQueue)(cl_command_queue command_queue) CL_API_SUFFIX__VERSION_1_0
{
    if (isHwMode())
    {
#ifdef HAS_OCL_HW_SUPPORT
        return ::clRetainCommandQueue(command_queue);
#else /* HAS_OCL_HW_SUPPORT */
        return CL_OUT_OF_RESOURCES;
#endif /* HAS_OCL_HW_SUPPORT */
    }
    else
    {
        if (command_queue)
        {
            command_queue->ref_ctr.AddRef();
            return CL_SUCCESS;
        }
        else
        {
            return CL_INVALID_COMMAND_QUEUE;
        }
    }
}

CL_API_ENTRY cl_int CL_API_CALL
SHIM_CALL(clRetainMemObject)(cl_mem memobj) CL_API_SUFFIX__VERSION_1_0
{
    if (isHwMode())
    {
#ifdef HAS_OCL_HW_SUPPORT
        return ::clRetainMemObject(memobj);
#else /* HAS_OCL_HW_SUPPORT */
        return CL_OUT_OF_RESOURCES;
#endif /* HAS_OCL_HW_SUPPORT */
    }
    else
    {
        if (memobj)
        {
            memobj->ref_ctr.AddRef();
            return CL_SUCCESS;
        }
        else
        {
            return CL_INVALID_MEM_OBJECT;
        }
    }
}

CL_API_ENTRY cl_int CL_API_CALL
SHIM_CALL(clRetainEvent)(cl_event event) CL_API_SUFFIX__VERSION_1_0
{
    if (isHwMode())
    {
#ifdef HAS_OCL_HW_SUPPORT
        return ::clRetainEvent(event);
#else /* HAS_OCL_HW_SUPPORT */
        return CL_OUT_OF_RESOURCES;
#endif /* HAS_OCL_HW_SUPPORT */
    }
    else
    {
        if (event)
        {
            event->ref_ctr.AddRef();
            return CL_SUCCESS;
        }
        else
        {
            return CL_INVALID_EVENT;
        }
    }
}

CL_API_ENTRY cl_int CL_API_CALL
SHIM_CALL(clReleaseDevice)(cl_device_id device) CL_API_SUFFIX__VERSION_1_2
{
    if (isHwMode())
    {
#ifdef HAS_OCL_HW_SUPPORT
        return ::clReleaseDevice(device);
#else /* HAS_OCL_HW_SUPPORT */
        return CL_OUT_OF_RESOURCES;
#endif /* HAS_OCL_HW_SUPPORT */
    }
    else
    {
        return CL_OUT_OF_RESOURCES;
    }
}

CM_SURFACE_FORMAT clImageFormatToCmFormat(const cl_image_format * format)
{
    using ULongPair = std::pair<unsigned long, unsigned long>;
    using FmtMap = std::map<ULongPair, CM_SURFACE_FORMAT>;
    static const FmtMap cl2cm = {
        {{CL_UNORM_INT8, CL_RGBA}, CM_SURFACE_FORMAT_A8R8G8B8},
        {{CL_UNORM_INT8, CL_ARGB}, CM_SURFACE_FORMAT_A8R8G8B8},
    };

    auto result = cl2cm.find({ format->image_channel_data_type, format->image_channel_order });
    if (result != cl2cm.end())
    {
        return result->second;
    }
    return CM_SURFACE_FORMAT_A8R8G8B8;
}

CL_API_ENTRY cl_mem CL_API_CALL
SHIM_CALL(clCreateImage)(cl_context              context,
    cl_mem_flags            flags,
    const cl_image_format * image_format,
    const cl_image_desc *   image_desc,
    void *                  host_ptr,
    cl_int *                errcode_ret) CL_API_SUFFIX__VERSION_1_2
{
    if (isHwMode())
    {
#ifdef HAS_OCL_HW_SUPPORT
        return ::clCreateImage(context, flags, image_format, image_desc, host_ptr, errcode_ret);
#else /* HAS_OCL_HW_SUPPORT */
        SET_ERRCODE(CL_OUT_OF_RESOURCES);
        return nullptr;
#endif /* HAS_OCL_HW_SUPPORT */
    }
    else
    {
        SET_ERRCODE(CL_OUT_OF_RESOURCES);
        if ((image_format == nullptr) || (image_desc == nullptr))
        {
            SET_ERRCODE(CL_INVALID_IMAGE_DESCRIPTOR);
            return nullptr;
        }
        switch (image_desc->image_type)
        {
        case CL_MEM_OBJECT_IMAGE2D:
            break;
        default:
            SET_ERRCODE(CL_INVALID_IMAGE_DESCRIPTOR);
            return nullptr;
        }
        CmSurface2D *surface = nullptr;
        int status = context->cm_device->CreateSurface2D(image_desc->image_width, image_desc->image_height,
            clImageFormatToCmFormat(image_format), surface);
        if (status != CM_SUCCESS)
        {
            return nullptr;
        }
        if (host_ptr)
        {
            status = surface->WriteSurface(reinterpret_cast<const unsigned char*>(host_ptr),
                nullptr, image_desc->image_height * image_desc->image_width * 4 /* HACKY!!!!!!! support size ASAP*/);
        }
        cl_mem cl_buffer = new _cl_mem;
        cl_buffer->type = _cl_mem::Image2d;
        cl_buffer->v.cm_surf2d = surface;
        cl_buffer->context = context;
        cl_int retcode = (status == CM_SUCCESS) ? CL_SUCCESS : CL_INVALID_HOST_PTR;
        SET_ERRCODE(retcode);
        return cl_buffer;
    }
}

CL_API_ENTRY cl_int CL_API_CALL
SHIM_CALL(clEnqueueReadImage)(cl_command_queue     command_queue,
    cl_mem               image,
    cl_bool              blocking_read,
    const size_t *       origin/*[3]*/,
    const size_t *       region/*[3]*/,
    size_t               row_pitch,
    size_t               slice_pitch,
    void *               ptr,
    cl_uint              num_events_in_wait_list,
    const cl_event *     event_wait_list,
    cl_event *           event) CL_API_SUFFIX__VERSION_1_0
{
    if (isHwMode())
    {
#ifdef HAS_OCL_HW_SUPPORT
        return ::clEnqueueReadImage(command_queue, image, blocking_read, origin, region, row_pitch, slice_pitch,
            ptr, num_events_in_wait_list, event_wait_list, event);
#else /* HAS_OCL_HW_SUPPORT */
        return CL_OUT_OF_RESOURCES;
#endif /* HAS_OCL_HW_SUPPORT */
    }
    else
    {
        /// HACKY, reimplement ASAP
        if (image->type != _cl_mem::Image2d)
        {
            return CL_OUT_OF_RESOURCES;
        }
        if (!origin || !region)
        {
            return CL_OUT_OF_RESOURCES;
        }
        if ((row_pitch != 0) || (slice_pitch != 0))
        {
            return CL_OUT_OF_RESOURCES;
        }
        if (event || event_wait_list)
        {
            return CL_OUT_OF_RESOURCES;
        }
        if (!ptr)
        {
            return CL_OUT_OF_RESOURCES;
        }
        unsigned char *destination = reinterpret_cast<unsigned char*>(ptr);
        int status = image->v.cm_surf2d->ReadSurface(destination, 0);
        return status == CM_SUCCESS ? CL_SUCCESS : CL_OUT_OF_RESOURCES;
    }
}

CL_API_ENTRY cl_int CL_API_CALL
SHIM_CALL(clGetProgramInfo)(cl_program         program,
    cl_program_info    param_name,
    size_t             param_value_size,
    void *             param_value,
    size_t *           param_value_size_ret) CL_API_SUFFIX__VERSION_1_0
{
    if (isHwMode())
    {
#ifdef HAS_OCL_HW_SUPPORT
        return ::clGetProgramInfo(program, param_name, param_value_size, param_value, param_value_size_ret);
#else /* HAS_OCL_HW_SUPPORT */
        return CL_OUT_OF_RESOURCES;
#endif /* HAS_OCL_HW_SUPPORT */
    }
    else
    {
        if (!ProgramManager::instance().IsProgramValid(program->info.handle))
        {
            return CL_OUT_OF_RESOURCES;
        }
        if ((param_value == nullptr) && (param_value_size_ret == nullptr))
        {
            return CL_OUT_OF_RESOURCES;
        }
        switch (param_name)
        {
        case CL_PROGRAM_NUM_KERNELS:
            if (param_value)
            {
                size_t* result = reinterpret_cast<size_t*>(param_value);
                *result = program->info.kernels.size();
            }
            if (param_value_size_ret)
            {
                *param_value_size_ret = sizeof(size_t);
            }
            return CL_SUCCESS;
        case CL_PROGRAM_KERNEL_NAMES:
            // program->kernels separated by ;
            if (program->kernel_names_cache.empty())
            {
                for (auto& kernel_descr : program->info.kernels)
                {
                    // inefficient, but since we do this exactly one time per program - who cares..
                    program->kernel_names_cache += kernel_descr.first + ";"s;
                }
                if (!program->kernel_names_cache.empty())
                {
                    // remove the last delimiter
                    program->kernel_names_cache.pop_back();
                }
            }
            fillInfoString(program->kernel_names_cache, param_value, param_value_size_ret);
            return CL_SUCCESS;
        default:
            break;
        }
        return CL_OUT_OF_RESOURCES;
    }
}

CL_API_ENTRY cl_command_queue CL_API_CALL
SHIM_CALL(clCreateCommandQueue)(cl_context                     context,
    cl_device_id                   device,
    cl_command_queue_properties    properties,
    cl_int *                       errcode_ret)
{
    if (isHwMode())
    {
#ifdef HAS_OCL_HW_SUPPORT
        return ::clCreateCommandQueue(context, device, properties, errcode_ret);
#else /* HAS_OCL_HW_SUPPORT */
        SET_ERRCODE(CL_OUT_OF_RESOURCES);
        return nullptr;
#endif /* HAS_OCL_HW_SUPPORT */
    }
    else
    {

        return SHIM_CALL(clCreateCommandQueueWithProperties)(context, device, 0, errcode_ret);
    }
}

CL_API_ENTRY void * CL_API_CALL
SHIM_CALL(clGetExtensionFunctionAddressForPlatform)(cl_platform_id platform,
                                                    const char *func_name) CL_API_SUFFIX__VERSION_1_2
{
    if (!func_name)
    {
        return NULL;
    }
    if (!strcmp(func_name, "SetDefaultResidentGroupAndParallelThreadNum"))
    {
        return reinterpret_cast<void*>(&SetDefaultResidentGroupAndParallelThreadNum);
    }
    if (!strcmp(func_name, "SetResidentGroupAndParallelThreadNumForQueue"))
    {
        return reinterpret_cast<void*>(&SetResidentGroupAndParallelThreadNumForQueue);
    }
    return 0;
}

}

