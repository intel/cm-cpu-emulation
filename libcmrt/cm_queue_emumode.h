/*===================== begin_copyright_notice ==================================

 Copyright (c) 2021, Intel Corporation


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

#pragma once

#include "cm_array.h"
#include "cm_def.h"
#include "cm_kernel_base.h"
#include "cm_queue_base.h"

class CmKernel;
class CmKernelEmu;
class CmQueue;
class CmEvent;
class CmEventEmu;
class CmSurface2D;
class CmTaskInternal;
class CmDevice;
class CmDeviceEmu;
class CmThreadSpace;
class CmThreadSpaceEmu;
class CmThreadGroupSpace;

//!
//! CM task queue. Each task have one or more kernels running concurrently.
//! Each kernel can run in multiple threads concurrently. It is a in-order queue.
//! Tasks get executed according to the order they get enqueued. The next task
//! doesn't start execute until the current task finishs.
//!
class CmQueueEmu : public CmQueue
{
public:
    static int32_t Create( CmDeviceEmu* pDevice, CmQueueEmu* & pQueue );
    static int32_t Destroy( CmQueueEmu* & pQueue );

    //CM_RT_API int32_t Enqueue( CmKernel* pKernelArray[], CmEvent* & pEvent, bool flush = true, const CmThreadSpace* pTS = nullptr );
    CM_RT_API int32_t Enqueue( CmTask* pKernelArray, CmEvent* & pEvent, const CmThreadSpace* pTS = nullptr );

    //int32_t Flush(void);
    //CM_RT_API int32_t Finish(void); // blocking call

    CM_RT_API int32_t DestroyEvent( CmEvent* & pEvent );

    CM_RT_API int32_t EnqueueWithGroup( CmTask* pTask, CmEvent* & pEvent, const CmThreadGroupSpace* pTGS = nullptr);

    CM_RT_API int32_t EnqueueCopyCPUToGPU( CmSurface2D* pSurface,
        const unsigned char* pSysMem,
        CmEvent* & pEvent );
    CM_RT_API int32_t EnqueueCopyGPUToCPU( CmSurface2D* pSurface,
        unsigned char* pSysMem,
        CmEvent* & pEvent );

    CM_RT_API int32_t EnqueueInitSurface2D( CmSurface2D* pSurface, const uint32_t initValue, CmEvent* &pEvent);

    CM_RT_API int32_t EnqueueCopyGPUToGPU( CmSurface2D* pOutputSurface, CmSurface2D* pInputSurface, uint32_t option, CmEvent* & pEvent );
    CM_RT_API int32_t EnqueueCopyCPUToCPU( unsigned char* pDstSysMem, unsigned char* pSrcSysMem,   uint32_t size, uint32_t option, CmEvent* & pEvent );

    // enqueue CmBuffer read from GPU 1D surface to CPU system memory
    CM_RT_API int32_t EnqueueReadBuffer(CmBuffer* buffer,
                                        size_t offset,
                                        const unsigned char* sysMem,
                                        uint64_t sysMemSize,
                                        CmEvent* wait_event,
                                        CmEvent*& event,
                                        unsigned option = 0);

    // enqueue CmBuffer write from CPU system memory to GPU 1D surface
    CM_RT_API int32_t EnqueueWriteBuffer(CmBuffer* buffer,
                                         size_t offset,
                                         const unsigned char* sysMem,
                                         uint64_t sysMemSize,
                                         CmEvent* wait_event,
                                         CmEvent*& event,
                                         unsigned option = 0);

    CM_RT_API int32_t EnqueueWithHints( CmTask* pTask, CmEvent* & pEvent, uint32_t hints = 0){ return CmNotImplemented(__PRETTY_FUNCTION__); }

    CM_RT_API int32_t EnqueueCopyCPUToGPUFullStride( CmSurface2D* pSurface, const unsigned char* pSysMem, const uint32_t widthStride, const uint32_t heightStride, const uint32_t option, CmEvent* & pEvent ) {return CmNotImplemented(__PRETTY_FUNCTION__);}
    CM_RT_API int32_t EnqueueCopyGPUToCPUFullStride( CmSurface2D* pSurface, unsigned char* pSysMem, const uint32_t widthStride, const uint32_t heightStride, const uint32_t option, CmEvent* & pEvent ){return CmNotImplemented(__PRETTY_FUNCTION__);}

    CM_RT_API int32_t EnqueueCopyCPUToGPUFullStrideDup( CmSurface2D* pSurface, const unsigned char* pSysMem, const uint32_t widthStride, const uint32_t heightStride, const uint32_t option, CmEvent* & pEvent ) {return CmNotImplemented(__PRETTY_FUNCTION__);}
    CM_RT_API int32_t EnqueueCopyGPUToCPUFullStrideDup( CmSurface2D* pSurface, unsigned char* pSysMem, const uint32_t widthStride, const uint32_t heightStride, const uint32_t option, CmEvent* & pEvent ){return CmNotImplemented(__PRETTY_FUNCTION__);}

    CM_RT_API int32_t EnqueueFast(CmTask* task,
                                  CmEvent* &event,
                                  const CmThreadSpace *threadSpace = nullptr)
    { return Enqueue(task, event, threadSpace); }

    CM_RT_API int32_t DestroyEventFast(CmEvent* &event)
    { return DestroyEvent(event); }

    CM_RT_API int32_t EnqueueWithGroupFast(CmTask *task,
                                  CmEvent *&event,
                                  const CmThreadGroupSpace *threadGroupSpace = nullptr)
    { return EnqueueWithGroup(task, event, threadGroupSpace); }

    CM_RT_API int32_t SetResidentGroupAndParallelThreadNum(uint32_t residentGroupNum, uint32_t parallelThreadNum);

    int32_t GetTaskHasThreadArg(CmTask* pTask, bool& hasThreadArg);
    uint32_t GetThreadCount(CmKernelEmu *kernel,
                            CmThreadSpaceEmu *threadSpace);
    void SetDeviceTileID(int32_t deviceTileID) { m_deviceTileID = deviceTileID; }
    CM_QUEUE_TYPE Type() { return m_type; }

protected:
    CmQueueEmu( CmDeviceEmu* pDevice );
    ~CmQueueEmu( void );

    int32_t Initialize( void );

    int32_t Execute(const CmKernelEmu&, uint32_t threadId);
    int32_t ExecuteScoreBoard(CmThreadSpaceEmu * threadSpace, bool be_walker);
    bool inner_loop_iteration(int last_x, int last_y, int bound_x, int bound_y, int x_stride, int y_stride, int &x, int &y);
    bool outer_loop_iteration(int last_x, int last_y,
                              int width, int height,
                              int width_factor, int height_factor,
                              int x_stride, int y_stride,
                              int inner_x_stride, int inner_y_stride,
                              int &x, int &y,
                              int &outer_x, int &outer_y);

    int32_t ExecuteScoreBoard26Z(CmThreadSpaceEmu * threadSpace);
    int32_t ExecuteScoreBoard26ZI(CmThreadSpaceEmu * threadSpace);
    int32_t ExecuteScoreBoard_1(CmThreadSpaceEmu * threadSpace, bool be_walker);

    int32_t Enqueue_preG12(CmTask *pKernelArray, CmEvent *&pEvent, const CmThreadSpace *pTS = nullptr);
    int m_EventCount;
    CmDeviceEmu* m_pDevice;

    CM_HAL_MAX_VALUES* m_pMaxVhalVals;

    CmDynamicArray m_EventArray;

    CSync m_CriticalSection_Tasks;
    CSync m_CriticalSection_FlushTasks;

    CmTask *m_pGPUCopyTask;

    CM_QUEUE_TYPE m_type;
    uint32_t m_ResidentGroupNum;
    uint32_t m_ParallelThreadNum;
    int m_deviceTileID; // if it is >= 0, it is multiTile
};
