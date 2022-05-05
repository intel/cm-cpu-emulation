/*========================== begin_copyright_notice ============================

Copyright (C) 2017 Intel Corporation

SPDX-License-Identifier: MIT

============================= end_copyright_notice ===========================*/

#include "cm_include.h"
#include "cm.h"
#include "rt.h"

#include "cm_queue_emumode.h"
#include "cm_event_emumode.h"
#include "cm_kernel_emumode.h"
#include "cm_device_emumode.h"
#include "cm_task_emumode.h"
#include "cm_thread_space_emumode.h"
#include "cm_group_space_emumode.h"
#include "cm_buffer_emumode.h"
#include "cm_surface_2d_emumode.h"
#include "cm_statistics.h"
#include "cm_mem_fast_copy.h"

#include "emu_utils.h"
#include "emu_cfg.h"

using namespace std;

#define INVISIBLE_EVENT_MAGIC_NUM ((CmEvent *)(-1))  // Magic Number for invisible event.
#define CM_MAX_THREADSPACE_WIDTH_FOR_MO 512
#define CM_MAX_THREADSPACE_HEIGHT_FOR_MO 512

//////////////////////////////////////////////////////////////////////////////////
int32_t CmQueueEmu::Create(CmDeviceEmu *pDevice, CmQueueEmu *&pQueue)
{
    int32_t result = CM_SUCCESS;
    pQueue         = new CmQueueEmu(pDevice);
    if (pQueue)
    {
        result = pQueue->Initialize();
        if (result != CM_SUCCESS)
        {
            CmQueueEmu::Destroy(pQueue);
        }
    }
    else
    {
        GFX_EMU_ASSERT(0);
        result = CM_OUT_OF_HOST_MEMORY;
    }
    return result;
}

int32_t CmQueueEmu::Destroy(CmQueueEmu *&pQueue)
{
    CmSafeRelease(pQueue);
    return CM_SUCCESS;
}

#if 0
int32_t CmQueueEmu::Flush( void )
{
    return CM_SUCCESS;
}
#endif

CM_RT_API int32_t CmQueueEmu::DestroyEvent(CmEvent *&pEvent)
{
    uint32_t    status = 0;
    uint32_t    index  = -1;
    CmEventEmu *temp   = dynamic_cast<CmEventEmu *>(pEvent);
    if (temp == nullptr)
    {
        return CM_FAILURE;
    }
    temp->GetIndex(index);

    status = CmEventEmu::Destroy(temp);
    if (status == CM_SUCCESS)
    {
        m_EventArray.SetElement(index, nullptr);
        pEvent = nullptr;
    }
    return status;
}

int32_t CmQueueEmu::GetTaskHasThreadArg(CmTask *pKernelArray, bool &threadArgExists)
{
    uint32_t     kernelCount   = 0;
    CmKernelEmu *kernel        = nullptr;
    uint32_t     numThreadArgs = 0;

    threadArgExists = false;

    CmKernelArrayEmu *pKA = (CmKernelArrayEmu *)pKernelArray;
    if (!pKA)
    {
        GFX_EMU_ASSERT(0);
        return CM_FAILURE;
    }

    kernelCount = pKA->GetKernelCount();

    for (uint32_t i = 0; i < kernelCount; ++i)
    {
        kernel = (CmKernelEmu *)pKA->GetKernelPointer(i);
        if (!kernel)
        {
            GFX_EMU_ASSERT(0);
            return CM_FAILURE;
        }

        kernel->GetThreadArgCount(numThreadArgs);
        if (numThreadArgs)
        {
            threadArgExists = true;
            break;
        }
    }

    return CM_SUCCESS;
}

int32_t CmQueueEmu::Enqueue_preG12(
    CmTask *             pKernelArray,
    CmEvent *&           pEvent,
    const CmThreadSpace *pTS)
{
    int32_t  result     = 0;
    uint32_t numThreads = 0;
    uint32_t numMaxArgs = 0;
    uint32_t x, y = 0;
    uint32_t i              = 0;
    uint32_t threadArgCount = 0;
    uint32_t kernelArgCount = 0;
    uint32_t kernelCount    = 0;
    int      totalOffset    = 0;
    void (*fncPt)()         = nullptr;
    CmKernelEmu *kernel     = NULL;
    //GfxEmu::KernelArg *     pArg       = nullptr;

    bool threadArgExists = false;

    CmThreadSpaceEmu *threadSpace = dynamic_cast<CmThreadSpaceEmu *>(const_cast<CmThreadSpace *>(pTS));

    if (pKernelArray == nullptr)
    {
        GFX_EMU_ERROR_MESSAGE("Kernel array is NULL.");
        GFX_EMU_ASSERT(0);
        //GFX_EMU_MESSAGE(("Kernel array is NULL.") );
        return CM_INVALID_ARG_VALUE;
    }

    uint32_t totalThreadCount = 0;
    uint32_t count = 0;
    auto pKATmp = (CmKernelArrayEmu *)pKernelArray;
    kernelCount = pKATmp->GetKernelCount();

    if (kernelCount > this->m_pMaxVhalVals->maxKernelsPerTask)
    {
        GFX_EMU_ERROR_MESSAGE("Maximum number of Kernels per task exceeded.");
        GFX_EMU_ASSERT(0);
        //GFX_EMU_MESSAGE(("Maximum number of Kernels per task exceeded.") );
        return CM_EXCEED_MAX_KERNEL_PER_ENQUEUE;
    }

    CLock locker(m_CriticalSection_Tasks);

    //selectively copies from SRC to volatile buffer for SM and UP surfaces/buffers
    this->m_pDevice->DoGPUCopySelect();

    for (uint32_t i = 0; i < kernelCount; i++)
    {
        numThreads                       = 0;
        numMaxArgs                       = 0;
        threadArgCount                   = 0;
        CmThreadSpaceEmu *threadSpaceEmu = threadSpace;
        kernel                           = (CmKernelEmu *)pKATmp->GetKernelPointer(i);
        kernel->SetIndexInTask(i);
        if (threadSpaceEmu == nullptr)
        {
            kernel->GetThreadSpace(threadSpaceEmu);
        }
        if (threadSpaceEmu)
        {
            if (!threadSpaceEmu->IntegrityCheck(pKATmp))
            {
                GFX_EMU_ASSERT(0);
                return CM_INVALID_THREAD_SPACE;
            }
        }

        kernel->GetThreadArgCount(threadArgCount);
        //kernel->GetArgs(pArg);
        kernel->GetMaxArgCount(numMaxArgs);
        kernel->GetArgCount(kernelArgCount);
        numThreads = GetThreadCount(kernel, threadSpaceEmu);
        fncPt      = (void (*)())kernel->GetFuncPnt();
        if (!threadArgCount)
        {
            if (threadSpaceEmu != nullptr)
            {
                threadSpaceEmu->InitDependency();
                if (!threadSpaceEmu->IsThreadAssociated())
                    threadSpaceEmu->AssociateKernel(kernel);
                if ((result = ExecuteScoreBoard_1(threadSpaceEmu, true)) != CM_SUCCESS)
                {
                    GFX_EMU_ASSERT(0);
                    return result;
                }
            }
            else
            {
                for (uint32_t i = 0; i < numThreads; i++)
                {
                    set_thread_origin_x(i % 511);
                    set_thread_origin_y(i / 511);
                    Execute(*kernel, i);
                }
            }
        }

        else if (threadSpaceEmu != nullptr)
        {
            threadSpaceEmu->InitDependency();
            if (!threadSpaceEmu->IsThreadAssociated())
                threadSpaceEmu->AssociateKernel(kernel);

            if (ExecuteScoreBoard_1(threadSpaceEmu, false) == CM_FAILURE)
            {
                GFX_EMU_ASSERT(0);
                return CM_FAILURE;
            }
        }
        else
        {
            for (uint32_t threadId = 0; threadId < numThreads; threadId++)
            {
                //was already executed during scoreboard.
                if (kernel->GetSCBCoord(threadId, x, y))
                    continue;
                set_thread_origin_x(threadId % 512);
                set_thread_origin_y(threadId / 512);
                Execute(*kernel, threadId);
            }
        }
        m_pDevice->DoCopyAll();
    }

    if (kernelCount == 0)
    {
        GFX_EMU_ERROR_MESSAGE("There are no valid kernels!");
        GFX_EMU_ASSERT(0);
        return CM_FAILURE;
    }

    if (pEvent != INVISIBLE_EVENT_MAGIC_NUM)
    {
        CmEventEmu *pTmp = nullptr;
        if (CmEventEmu::Create(m_EventCount, pTmp) == CM_SUCCESS)
        {
            m_EventArray.SetElement(m_EventCount, pTmp);
            m_EventCount++;
            pEvent = static_cast<CmEvent *>(pTmp);
        }
    }
    else
    {
        // if the input pEvent equals to INVISIBLE_EVENT_MAGIC_NUM, event will not be created.
        pEvent = nullptr;
    }
#ifdef GFX_EMU_DEBUG_ENABLED
    //////////////////////////////////////////////////////////////////////////////////////
    if (CmStatistics::Get() != nullptr)
    {
        CmStatistics::Get()->TrackRunnedKernels(pKernelArray);
    }
#endif

    return CM_SUCCESS;
}

CM_RT_API int32_t CmQueueEmu::Enqueue(
    CmTask *             pKernelArray,
    CmEvent *&           pEvent,
    const CmThreadSpace *pTS)
{
    int32_t           ret;
    CmThreadSpaceEmu *threadSpace = dynamic_cast<CmThreadSpaceEmu *>(const_cast<CmThreadSpace *>(pTS));

    if (GfxEmu::Cfg::Platform ().getInt () >= GfxEmu::Platform::XEHP_SDV)
    {
        CmThreadGroupSpace *thread_group_space_h = threadSpace ? threadSpace->GetThreadGroupSpace() : nullptr;
        if (!thread_group_space_h)
        {
            CmKernelArrayEmu *                pKATmp      = (CmKernelArrayEmu *)pKernelArray;
            uint32_t                          kernelCount = pKATmp->GetKernelCount();
            CmKernelEmu *                     kernel      = NULL;
            uint32_t                          numThreads  = 0;
            uint32_t                          gwidth      = 0;
            uint32_t                          gheight     = 0;

            //std::vector<std::pair<CmKernelEmu *, CmThreadGroupSpace *>> threadGS;

            for (uint32_t i = 0; i < kernelCount; i++)
            {
                kernel     = (CmKernelEmu *)pKATmp->GetKernelPointer(i);
                numThreads = GetThreadCount(kernel, threadSpace);
                gwidth     = numThreads % 511;
                gheight    = (int)ceil((double)numThreads / 511);
                if (gwidth == 0)
                    gwidth = 1;
                if (gheight == 0)
                    gheight = 1;
                CmThreadGroupSpace *pTGS = NULL;
                kernel->GetThreadGroupSpace(pTGS);
                if (pTGS == nullptr)
                {
                    CmThreadGroupSpace::Create(m_pDevice, 1, 1, gwidth, gheight, pTGS);
                    kernel->AssociateThreadGroupSpace(pTGS);
                    //threadGS.emplace_back(std::pair<CmKernelEmu *, CmThreadGroupSpace *>{kernel, pTGS});
                }
            }

            ret = EnqueueWithGroup(pKernelArray, pEvent, thread_group_space_h);

            for (uint32_t i = 0; i < kernelCount; i++)
            {
                auto kernel = static_cast<CmKernelEmu*> (pKATmp->GetKernelPointer(i));
                CmThreadGroupSpace* pTGS = nullptr;
                kernel->GetThreadGroupSpace (pTGS);
                if (pTGS)
                    kernel->DeAssociateThreadGroupSpace(pTGS);
            }

            //for (uint32_t i = 0; i < threadGS.size(); i++)
            //    threadGS[i].first->DeAssociateThreadGroupSpace(threadGS[i].second);
        }
        else
        {
            ret = EnqueueWithGroup(pKernelArray, pEvent, thread_group_space_h);
        }
    }
    else
    {
        ret = Enqueue_preG12(pKernelArray, pEvent, pTS);
    }
    return ret;
}

typedef struct _RESTORE_INFO
{
    uint32_t gid0;
    uint32_t gid1;
    uint32_t gid2;
    uint32_t lid0;
    uint32_t lid1;
    uint32_t lid2;

    uint32_t barrier_type;
    uint32_t slm_id; // used in cm_slm_load
} RESTORE_INFO;

CM_RT_API int32_t CmQueueEmu::EnqueueWithGroup(
    CmTask *                  pKernelArray,
    CmEvent *&                pEvent,
    const CmThreadGroupSpace *pTGS)
{
    uint32_t kernelCount = 0;
    int      totalOffset = 0;
    void (*fncPt)()      = nullptr;
    CmKernelEmu *kernel  = NULL;
    //GfxEmu::KernelArg *     pArg    = nullptr;
    uint32_t     threadSpaceWidth = -1, threadSpaceHeight = -1, threadSpaceDepth = -1,
        groupSpaceWidth = -1, groupSpaceHeight = -1, groupSpaceDepth = -1;

    if (pKernelArray == nullptr)
    {
        GFX_EMU_ERROR_MESSAGE("Kernel array is NULL.");
        GFX_EMU_ASSERT(0);
        //GFX_EMU_MESSAGE(("Kernel array is NULL.") );
        return CM_INVALID_ARG_VALUE;
    }

    CmKernelArrayEmu *pKATmp = (CmKernelArrayEmu *)pKernelArray;

    kernelCount = pKATmp->GetKernelCount();

    if (kernelCount > this->m_pMaxVhalVals->maxKernelsPerTask)
    {
        GFX_EMU_ERROR_MESSAGE("Maximum number of Kernels per task exceeded.");
        GFX_EMU_ASSERT(0);
        //GFX_EMU_MESSAGE(("Maximum number of Kernels per task exceeded.") );
        return CM_EXCEED_MAX_KERNEL_PER_ENQUEUE;
    }

    CLock locker(m_CriticalSection_Tasks);

    for (uint32_t i = 0; i < kernelCount; i++)
    {
        auto kernel = (CmKernelEmu *)pKATmp->GetKernelPointer(i);
        kernel->SetIndexInTask(i);

        auto threadGroupSpaceEmu = const_cast<CmThreadGroupSpace *>(pTGS);

        if (!threadGroupSpaceEmu)
        {
            kernel->GetThreadGroupSpace(threadGroupSpaceEmu);
        }

        if (!threadGroupSpaceEmu)
        {
            return CM_NULL_POINTER;
        }

        threadGroupSpaceEmu->GetThreadGroupSpaceSize(
            threadSpaceWidth, threadSpaceHeight, threadSpaceDepth,
            groupSpaceWidth, groupSpaceHeight, groupSpaceDepth);

        uint32_t numMaxArgs = 0;
        uint32_t threadArgCount = 0;

        kernel->GetThreadArgCount(threadArgCount);
        //kernel->GetArgs(pArg);
        kernel->GetMaxArgCount(numMaxArgs);
        fncPt = (void (*)())kernel->GetFuncPnt();

        if (threadArgCount)
        {
            GFX_EMU_ERROR_MESSAGE("No thread Args allowed when using group space");
            GFX_EMU_ASSERT(0);
            return CM_THREAD_ARG_NOT_ALLOWED;
        }

        m_pDevice->DoGPUCopySelect(); // Selectively copies from SRC to volatile buffer for SM and UP surfaces/buffers

        if (!cmrt::CmEmuMt_Kernel {
            {groupSpaceWidth, groupSpaceHeight, groupSpaceDepth},
            {threadSpaceWidth, threadSpaceHeight, threadSpaceDepth},
            m_ResidentGroupNum,
            m_ParallelThreadNum,
            cmrt::CmEmu_KernelLauncher {
                kernel->GetName (),
                kernel->GetProgramModule (),
                kernel->GetArgsVecRef (),
                cmrt::CmEmu_KernelLauncher::kThreadIdUnset,
                reinterpret_cast<void(*)()> (const_cast<void*>(kernel->GetFuncPnt ()))
            }}.run ())
        {
            GFX_EMU_ERROR_MESSAGE("Kernel group execution timeout.");
            return CM_FAILURE;
        }

        m_pDevice->DoCopyAll();
    }

    //End_of_enqueue:
    if (pEvent != INVISIBLE_EVENT_MAGIC_NUM)
    {
        CmEventEmu *pTmp = nullptr;
        if (CmEventEmu::Create(m_EventCount, pTmp) == CM_SUCCESS)
        {
            m_EventArray.SetElement(m_EventCount, pTmp);
            m_EventCount++;
            pEvent = static_cast<CmEvent *>(pTmp);
        }
    }
    else
    {
        // if the input pEvent equals to INVISIBLE_EVENT_MAGIC_NUM, event will not be created.
        pEvent = nullptr;
    }
#ifdef GFX_EMU_DEBUG_ENABLED
    //////////////////////////////////////////////////////////////////////////////////////
    if (CmStatistics::Get() != nullptr)
    {
        CmStatistics::Get()->TrackRunnedKernels(pKernelArray);
    }
#endif

    return CM_SUCCESS;
}

int32_t CmQueueEmu::Initialize(void)
{
    this->m_pDevice->GetHalMaxValues(m_pMaxVhalVals);

    return CM_SUCCESS;
}

CmQueueEmu::CmQueueEmu(CmDeviceEmu *pDevice) : m_pDevice(pDevice),
                                               m_EventArray(CM_INIT_EVENT_COUNT),
                                               m_pGPUCopyTask(nullptr)
{
    m_EventCount = 0;
    m_ResidentGroupNum  = 1;

    const auto hwThreads = std::thread::hardware_concurrency();
    m_ParallelThreadNum = hwThreads ? hwThreads : 1;
}

CmQueueEmu::~CmQueueEmu(void)
{
    for (int i = 0; i < m_EventCount; i++)
    {
        CmEventEmu *pEvent = (CmEventEmu *)m_EventArray.GetElement(i);
        if (pEvent)
        {
            CmEventEmu::Destroy(pEvent);
        }
    }
    m_EventArray.Delete();
}

CM_RT_API int32_t CmQueueEmu::EnqueueCopyCPUToGPU(CmSurface2D *pSurface, const unsigned char *pSysMem, CmEvent *&pEvent)
{
    int32_t     ret  = CM_FAILURE;
    CmEventEmu *pTmp = nullptr;

    CmSurface2DEmu *pSurface2DTemp = nullptr;
    pSurface2DTemp                 = dynamic_cast<CmSurface2DEmu *>(pSurface);

    uint32_t width  = pSurface2DTemp->GetWidth();
    uint32_t height = pSurface2DTemp->GetHeight();

    if (pEvent != INVISIBLE_EVENT_MAGIC_NUM)
    {
        CmEventEmu *pTmp = nullptr;
        if (CmEventEmu::Create(m_EventCount, pTmp) == CM_SUCCESS)
        {
            m_EventArray.SetElement(m_EventCount, pTmp);
            m_EventCount++;
            pEvent = static_cast<CmEvent *>(pTmp);
        }
    }
    else
    {
        // if the input pEvent equals to INVISIBLE_EVENT_MAGIC_NUM, event will not be created.
        pEvent = nullptr;
    }

    ret = pSurface->WriteSurface(pSysMem, nullptr, width * height);
    return ret;
}

CM_RT_API int32_t CmQueueEmu::EnqueueCopyGPUToCPU(CmSurface2D *pSurface, unsigned char *pSysMem, CmEvent *&pEvent)
{
    int32_t     ret  = CM_FAILURE;
    CmEventEmu *pTmp = nullptr;

    CmSurface2DEmu *pSurface2DTemp = nullptr;
    pSurface2DTemp                 = dynamic_cast<CmSurface2DEmu *>(pSurface);

    uint32_t width  = pSurface2DTemp->GetWidth();
    uint32_t height = pSurface2DTemp->GetHeight();

    if (pEvent != INVISIBLE_EVENT_MAGIC_NUM)
    {
        CmEventEmu *pTmp = nullptr;
        if (CmEventEmu::Create(m_EventCount, pTmp) == CM_SUCCESS)
        {
            m_EventArray.SetElement(m_EventCount, pTmp);
            m_EventCount++;
            pEvent = static_cast<CmEvent *>(pTmp);
        }
        else
        {
            return CM_FAILURE;
        }
    }
    else
    {
        // if the input pEvent equals to INVISIBLE_EVENT_MAGIC_NUM, event will not be created.
        pEvent = nullptr;
    }

    ret = pSurface->ReadSurface(pSysMem, pEvent, width * height);
    return ret;
}

int32_t CmQueueEmu::Execute(const CmKernelEmu& kernel, uint32_t threadId)
{
    if (!cmrt::CmEmuMt_Kernel {
        {1,1,1},
        {1,1,1},
        1,
        1,
        cmrt::CmEmu_KernelLauncher{
            kernel.GetName (),
            kernel.GetProgramModule (),
            kernel.GetArgsVecRef (),
            threadId,
            reinterpret_cast<void(*)()> (const_cast<void*>(kernel.GetFuncPnt ()))
        }}.run()
    ) {
        GFX_EMU_ERROR_MESSAGE("Kernel execution timeout.");
        return CM_FAILURE;
    }
    else
        return CM_SUCCESS;
}

int32_t CmQueueEmu::ExecuteScoreBoard(CmThreadSpaceEmu *threadSpace, bool be_walker)
{
    CM_THREAD_SPACE_UNIT *pThreadSpaceUnit = nullptr;
    uint32_t              height      = 0;
    uint32_t              width       = 0;
    uint32_t              numMaxArgs  = 0;
    uint32_t              numExecuted = 1;
    uint32_t              numSkipped  = 0;
    uint32_t              xCoord, yCoord = 0;
    //GfxEmu::KernelArg *              pArg = nullptr;
    void (*fncPt)()            = nullptr;
    CM_DEPENDENCY *pDependency = nullptr;

    if (threadSpace == nullptr)
    {
        GFX_EMU_ASSERT(0);
        return CM_INVALID_ARG_VALUE;
    }
    threadSpace->GetThreadSpaceUnit(pThreadSpaceUnit);
    threadSpace->GetThreadSpaceSize(width, height);
    threadSpace->GetDependency(pDependency);

    while (numExecuted)
    {
        numExecuted = 0;
        numSkipped  = 0;
        for (uint32_t y = 0; y < height; y++)
        {
            for (uint32_t x = 0; x < width; x++)
            {
                if (pThreadSpaceUnit[y * width + x].numEdges != 0)
                {
                    if (pThreadSpaceUnit[y * width + x].numEdges > 0)
                        numSkipped++;
                    continue;
                }

                CmKernelEmu *kernel = (CmKernelEmu *)pThreadSpaceUnit[y * width + x].pKernel;

                if (kernel == nullptr)
                {
                    GFX_EMU_ASSERT(0);
                    return CM_FAILURE;
                }

                fncPt = (void (*)())kernel->GetFuncPnt();

                //kernel->GetArgs(pArg);
                kernel->GetMaxArgCount(numMaxArgs);

                set_thread_origin_x(x);
                set_thread_origin_y(y);

                Execute(*kernel, pThreadSpaceUnit[y * width + x].threadId);
                pThreadSpaceUnit[y * width + x].numEdges--;
                numExecuted++;

                for (uint32_t i = 0; i < pDependency->count; i++)
                {
                    //checking for valid dependency
                    if (pDependency->deltaX[i] == 0 && pDependency->deltaY[i] == 0)
                        continue;

                    xCoord = x + pDependency->deltaX[i] * -1;
                    yCoord = y + pDependency->deltaY[i] * -1;

                    //checking for valid coordinates
                    if (xCoord >= width || yCoord >= height)
                        continue;
                    uint32_t linear_offset = yCoord * width + xCoord;
                    pThreadSpaceUnit[linear_offset].numEdges--;
                }
            }
        }
    }

    //Dead lock occured
    if (numSkipped)
        return CM_FAILURE;

    return CM_SUCCESS;
}

int32_t CmQueueEmu::ExecuteScoreBoard26Z(CmThreadSpaceEmu *threadSpace)
{
    int32_t               result           = CM_SUCCESS;
    CM_THREAD_SPACE_UNIT *pThreadSpaceUnit = nullptr;
    uint32_t              height           = 0;
    uint32_t              width            = 0;
    void (*fncPt)()                        = nullptr;
    //GfxEmu::KernelArg *  pArg                         = nullptr;
    uint32_t  numMaxArgs                   = 0;
    uint32_t  x                            = 0;
    uint32_t  y                            = 0;
    uint32_t  index                        = 0;
    uint32_t *pBoardOrder                  = nullptr;

    if (!threadSpace)
    {
        GFX_EMU_ASSERT(0);
        result = CM_FAILURE;
        goto finish;
    }

    threadSpace->GetThreadSpaceUnit(pThreadSpaceUnit);
    if (!pThreadSpaceUnit)
    {
        GFX_EMU_ASSERT(0);
        result = CM_FAILURE;
        goto finish;
    }

    // Generate Wavefront26Z sequence
    result = threadSpace->Wavefront26ZSequence();
    if (result != CM_SUCCESS)
    {
        GFX_EMU_ASSERT(0);
        goto finish;
    }

    threadSpace->GetBoardOrder(pBoardOrder);
    if (!pBoardOrder)
    {
        GFX_EMU_ASSERT(0);
        result = CM_FAILURE;
        goto finish;
    }

    threadSpace->GetThreadSpaceSize(width, height);

    for (uint32_t i = 0; i < width * height; ++i)
    {
        index = pBoardOrder[i];

        x = pThreadSpaceUnit[index].scoreboardCoordinates.x;
        y = pThreadSpaceUnit[index].scoreboardCoordinates.y;

        CmKernelEmu *kernel = (CmKernelEmu *)pThreadSpaceUnit[index].pKernel;
        if (!kernel)
        {
            GFX_EMU_ASSERT(0);
            result = CM_FAILURE;
            goto finish;
        }

        fncPt = (void (*)())kernel->GetFuncPnt();

        //kernel->GetArgs(pArg);
        kernel->GetMaxArgCount(numMaxArgs);

        set_thread_origin_x(x);
        set_thread_origin_y(y);

        Execute(*kernel, pThreadSpaceUnit[index].threadId);
    }

finish:
    return result;
}

int32_t CmQueueEmu::ExecuteScoreBoard26ZI(CmThreadSpaceEmu *threadSpace)
{
    int32_t               result           = CM_SUCCESS;
    CM_THREAD_SPACE_UNIT *pThreadSpaceUnit = nullptr;
    uint32_t              height           = 0;
    uint32_t              width            = 0;
    void (*fncPt)()                        = nullptr;
    //GfxEmu::KernelArg *  pArg                         = nullptr;
    uint32_t  numMaxArgs                   = 0;
    uint32_t  x                            = 0;
    uint32_t  y                            = 0;
    uint32_t  index                        = 0;
    uint32_t *pBoardOrder                  = nullptr;

    if (!threadSpace)
    {
        GFX_EMU_ASSERT(0);
        result = CM_FAILURE;
        goto finish;
    }

    threadSpace->GetThreadSpaceUnit(pThreadSpaceUnit);
    if (!pThreadSpaceUnit)
    {
        GFX_EMU_ASSERT(0);
        result = CM_FAILURE;
        goto finish;
    }

    // Generate Wavefront26ZI sequence
    CM_26ZI_DISPATCH_PATTERN pattern;
    threadSpace->Get26ZIDispatchPattern(pattern);
    switch (pattern)
    {
    case VVERTICAL_HVERTICAL_26:
        result = threadSpace->Wavefront26ZISeqVVHV26();
        break;
    case VVERTICAL_HHORIZONTAL_26:
        result = threadSpace->Wavefront26ZISeqVVHH26();
        break;
    case VVERTICAL26_HHORIZONTAL26:
        result = threadSpace->Wavefront26ZISeqVV26HH26();
        break;
    case VVERTICAL1X26_HHORIZONTAL1X26:
        result = threadSpace->Wavefront26ZISeqVV1x26HH1x26();
        break;
    default:
        result = threadSpace->Wavefront26ZISeqVVHV26();
        break;
    }

    if (result != CM_SUCCESS)
    {
        GFX_EMU_ASSERT(0);
        goto finish;
    }

    threadSpace->GetBoardOrder(pBoardOrder);
    if (!pBoardOrder)
    {
        GFX_EMU_ASSERT(0);
        result = CM_FAILURE;
        goto finish;
    }

    threadSpace->GetThreadSpaceSize(width, height);

    for (uint32_t i = 0; i < width * height; ++i)
    {
        index = pBoardOrder[i];

        x = pThreadSpaceUnit[index].scoreboardCoordinates.x;
        y = pThreadSpaceUnit[index].scoreboardCoordinates.y;

        CmKernelEmu *kernel = (CmKernelEmu *)pThreadSpaceUnit[index].pKernel;
        if (!kernel)
        {
            GFX_EMU_ASSERT(0);
            result = CM_FAILURE;
            goto finish;
        }

        fncPt = (void (*)())kernel->GetFuncPnt();

        //kernel->GetArgs(pArg);
        kernel->GetMaxArgCount(numMaxArgs);

        set_thread_origin_x(x);
        set_thread_origin_y(y);

        Execute(*kernel, pThreadSpaceUnit[index].threadId);
    }

finish:
    return result;
}

/*
 * This is the new implementation for ExecuteScoreBoard. It executes the threads according to the dispatch patterns.
 * FIXEME: ExecuteScoreBoard() needs be deleted and replaced with this function. It is kept to see if there is poential usage for it.
 */
int32_t CmQueueEmu::ExecuteScoreBoard_1(CmThreadSpaceEmu *threadSpace, bool be_walker)
{
    int32_t               result = CM_SUCCESS;
    CM_THREAD_SPACE_UNIT *pThreadSpaceUnit = nullptr;
    uint32_t              height      = 0;
    uint32_t              width       = 0;
    uint32_t              colorCount  = 1;
    uint32_t              numMaxArgs  = 0;
    uint32_t              numExecuted = 1;
    uint32_t              numSkipped  = 0;
    uint32_t              xCoord      = 0;
    uint32_t              yCoord      = 0;
    //GfxEmu::KernelArg *              pArg        = nullptr;
    void (*fncPt)()                   = nullptr;
    CM_DEPENDENCY *       pDependency = nullptr;
    CM_DEPENDENCY_PATTERN DependencyPatternType;
    CM_WALKING_PATTERN    WalkingPattern;

    int local_loop_exec_count = 0, global_loop_exec_count = 0;
    int local_outer_count = 0, global_outer_count = 0;
    int globalresX = 0, globalresY = 0;
    int globalStartX = 0, globalStartY = 0;
    int global_outer_x = 0, global_outer_y = 0;
    int global_inner_x = 0, global_inner_y = 0;
    int global_inner_x_copy = 0, global_inner_y_copy = 0;
    int global_outer_stepx, global_outer_stepy       = 0;
    int global_inner_stepx, global_inner_stepy       = 0;
    int middle_stepx = 0, middle_stepy = 0, extrasteps = 0;
    int mid_x = 0, mid_y = 0, mid_step = 0;
    int localblockresX = 0, localblockresY = 0;
    int localStartX = 0, localStartY = 0;
    int outer_x = 0, outer_y = 0;
    int local_inner_x = 0, local_inner_y = 0;
    int local_outer_stepx = 0, local_outer_stepy = 0;
    int local_inner_stepx = 0, local_inner_stepy = 0;
    int block_size_x = 0, block_size_y = 0;
    int x, y;

    if (threadSpace == nullptr)
    {
        GFX_EMU_ASSERT(0);
        result = CM_INVALID_ARG_VALUE;
        goto finish;
    }

    threadSpace->GetDependencyPatternType(DependencyPatternType);
    threadSpace->GetThreadSpaceSize(width, height);
    threadSpace->GetColorCount(colorCount);
    threadSpace->GetWalkingPattern(WalkingPattern);

    if (be_walker)
    {
        if (GfxEmu::Cfg::Platform ().getInt () < GfxEmu::Platform::SKL)
        {
            if (width > CM_MAX_THREADSPACE_WIDTH_FOR_MW)
            {
                GFX_EMU_ASSERT(0);
                result = CM_INVALID_THREAD_SPACE;
                goto finish;
            }
            if (height > CM_MAX_THREADSPACE_HEIGHT_FOR_MW)
            {
                GFX_EMU_ASSERT(0);
                result = CM_INVALID_THREAD_SPACE;
                goto finish;
            }
        }
        else
        {
            if (width > CM_MAX_THREADSPACE_WIDTH_SKLUP_FOR_MW)
            {
                GFX_EMU_ASSERT(0);
                result = CM_INVALID_THREAD_SPACE;
                goto finish;
            }
            if (height > CM_MAX_THREADSPACE_HEIGHT_SKLUP_FOR_MW)
            {
                GFX_EMU_ASSERT(0);
                result = CM_INVALID_THREAD_SPACE;
                goto finish;
            }
        }
    }
    else
    {
        // for now 1.21.15 only allow 512x512, 16 MB in second level batch buffer regardless of HW platform
        if (width > CM_MAX_THREADSPACE_WIDTH_FOR_MO)
        {
            GFX_EMU_ASSERT(0);
            result = CM_INVALID_THREAD_SPACE;
            goto finish;
        }
        if (height > CM_MAX_THREADSPACE_HEIGHT_FOR_MO)
        {
            GFX_EMU_ASSERT(0);
            result = CM_INVALID_THREAD_SPACE;
            goto finish;
        }
    }

    switch (DependencyPatternType)
    {
    case CM_NONE_DEPENDENCY:
        break;
    case CM_HORIZONTAL_WAVE:
        WalkingPattern = CM_WALK_HORIZONTAL;
        break;
    case CM_VERTICAL_WAVE:
        WalkingPattern = CM_WALK_VERTICAL;
        break;
    case CM_WAVEFRONT:
        WalkingPattern = CM_WALK_WAVEFRONT;
        break;
    case CM_WAVEFRONT26:
        WalkingPattern = CM_WALK_WAVEFRONT26;
        break;
    case CM_WAVEFRONT26Z:
        // no walking pattern, needs to be media object
        result = ExecuteScoreBoard26Z(threadSpace);
        goto finish;
        break;
    case CM_WAVEFRONT26ZI:
        result = ExecuteScoreBoard26ZI(threadSpace);
        goto finish;
        break;
    case CM_WAVEFRONT26X:
        WalkingPattern = CM_WALK_WAVEFRONT26X;
        break;
    case CM_WAVEFRONT26ZIG:
        WalkingPattern = CM_WALK_WAVEFRONT26ZIG;
        break;
    default:
        GFX_EMU_ASSERT(0);
        WalkingPattern = CM_WALK_DEFAULT;
        break;
    }

    switch (WalkingPattern)
    {
    case CM_WALK_DEFAULT:
    case CM_WALK_HORIZONTAL:
        local_outer_stepx      = 0;
        local_outer_stepy      = 1;
        local_inner_stepx      = 1;
        local_inner_stepy      = 0;
        localblockresX         = width;
        localblockresY         = height;
        local_loop_exec_count  = localblockresY - 1;
        global_loop_exec_count = 1;
        globalresX             = width;
        globalresY             = height;
        global_outer_stepx     = width;
        global_outer_stepy     = 0;
        global_inner_stepx     = 0;
        global_inner_stepy     = height;
        break;
    case CM_WALK_VERTICAL:
        local_outer_stepx      = 1;
        local_outer_stepy      = 0;
        local_inner_stepx      = 0;
        local_inner_stepy      = 1;
        localblockresX         = width;
        localblockresY         = height;
        local_loop_exec_count  = localblockresX - 1;
        global_loop_exec_count = 1;
        globalresX             = width;
        globalresY             = height;
        global_outer_stepx     = width;
        global_outer_stepy     = 0;
        global_inner_stepx     = 0;
        global_inner_stepy     = height;
        break;
    case CM_WALK_WAVEFRONT:
        local_outer_stepx      = 1;
        local_outer_stepy      = 0;
        local_inner_stepx      = -1;
        local_inner_stepy      = 1;
        localblockresX         = width;
        localblockresY         = height;
        local_loop_exec_count  = localblockresX - 1 + localblockresY - 1;
        global_loop_exec_count = 1;
        globalresX             = width;
        globalresY             = height;
        global_outer_stepx     = width;
        global_outer_stepy     = 0;
        global_inner_stepx     = 0;
        global_inner_stepy     = height;
        break;
    case CM_WALK_WAVEFRONT26:
        local_outer_stepx      = 1;
        local_outer_stepy      = 0;
        local_inner_stepx      = -2;
        local_inner_stepy      = 1;
        localblockresX         = width;
        localblockresY         = height;
        local_loop_exec_count  = localblockresX - 1 + (localblockresY - 1) * 2;
        global_loop_exec_count = 1;
        globalresX             = width;
        globalresY             = height;
        global_outer_stepx     = width;
        global_outer_stepy     = 0;
        global_inner_stepx     = 0;
        global_inner_stepy     = height;
        break;
    case CM_WALK_WAVEFRONT26X:
        local_outer_stepx      = 1;
        local_outer_stepy      = 0;
        local_inner_stepx      = -2;
        local_inner_stepy      = 2;
        localblockresX         = width;
        localblockresY         = height;
        local_loop_exec_count  = 0x3ff;
        global_loop_exec_count = 0x3ff;
        globalresX             = localblockresX;
        globalresY             = localblockresY;
        global_outer_stepx     = globalresX;
        global_outer_stepy     = 0;
        global_inner_stepx     = 0;
        global_inner_stepy     = globalresY;
        middle_stepx           = 0;
        middle_stepy           = 1;
        extrasteps             = 1;
        break;
    case CM_WALK_WAVEFRONT26ZIG:
        local_outer_stepx      = 0;
        local_outer_stepy      = 1;
        local_inner_stepx      = 1;
        local_inner_stepy      = 0;
        localblockresX         = 2;
        localblockresY         = 2;
        local_loop_exec_count  = 1;
        global_loop_exec_count = ((((height + 1) >> 1) << 1) / 2 - 1) * 2 + (((width + 1) >> 1) << 1) / 2 - 1;
        globalresX             = width;
        globalresY             = height;
        global_outer_stepx     = 2;
        global_outer_stepy     = 0;
        global_inner_stepx     = -4;
        global_inner_stepy     = 2;
        break;
    case CM_WALK_WAVEFRONT45D:
        local_outer_stepx      = 1;
        local_outer_stepy      = 0;
        local_inner_stepx      = -1;
        local_inner_stepy      = 1;
        localblockresX         = width;
        localblockresY         = height;
        local_loop_exec_count  = 2047;
        global_loop_exec_count = 2047;
        globalresX             = width;
        globalresY             = height;
        global_outer_stepx     = width;
        global_outer_stepy     = 0;
        global_inner_stepx     = 0;
        global_inner_stepy     = height;
        localStartX            = width;
        break;
    case CM_WALK_WAVEFRONT45XD_2:
        local_outer_stepx      = 1;
        local_outer_stepy      = 0;
        local_inner_stepx      = -1;
        local_inner_stepy      = 2;
        localblockresX         = width;
        localblockresY         = height;
        local_loop_exec_count  = 2047;
        global_loop_exec_count = 2047;
        globalresX             = width;
        globalresY             = height;
        global_outer_stepx     = width;
        global_outer_stepy     = 0;
        global_inner_stepx     = 0;
        global_inner_stepy     = height;
        middle_stepx           = 0;
        middle_stepy           = 1;
        extrasteps             = 1;
        localStartX            = width;
        break;
    default:
        GFX_EMU_ASSERT(0);
        result = CM_FAILURE;
        goto finish;
    }

    threadSpace->GetThreadSpaceUnit(pThreadSpaceUnit);
    threadSpace->GetDependency(pDependency);

    global_outer_x = globalStartX;
    global_outer_y = globalStartY;

    //do global_outer_looper initialization
    while (((global_outer_x >= globalresX) && (global_inner_stepx < 0)) ||
           (((global_outer_x + localblockresX) < 0) && (global_inner_stepx > 0)) ||
           ((global_outer_y >= globalresY) && (global_inner_stepy < 0)) ||
           (((global_outer_x + localblockresY) < 0) && (global_inner_stepy > 0)))
    {
        global_outer_x += global_inner_stepx;
        global_outer_y += global_inner_stepy;
    }

    //global_ouer_loop_in_bounds()
    while ((global_outer_x < globalresX) &&
           (global_outer_y < globalresY) &&
           (global_outer_x + localblockresX > 0) &&
           (global_outer_y + localblockresY > 0) &&
           (global_outer_count <= global_loop_exec_count))
    {
        global_inner_x = global_outer_x;
        global_inner_y = global_outer_y;

        //global_inner_loop_in_bounds()
        while ((global_inner_x < globalresX) &&
               (global_inner_y < globalresY) &&
               (global_inner_x + localblockresX > 0) &&
               (global_inner_y + localblockresY > 0))
        {
            global_inner_x_copy = global_inner_x;
            global_inner_y_copy = global_inner_y;
            if (global_inner_x < 0)
                global_inner_x_copy = 0;
            if (global_inner_y < 0)
                global_inner_y_copy = 0;

            if (global_inner_x < 0)
                block_size_x = localblockresX + global_inner_x;
            else if ((globalresX - global_inner_x) < localblockresX)
                block_size_x = globalresX - global_inner_x;
            else
                block_size_x = localblockresX;
            if (global_inner_y < 0)
                block_size_y = localblockresY + global_inner_y;
            else if ((globalresY - global_inner_y) < localblockresY)
                block_size_y = globalresY - global_inner_y;
            else
                block_size_y = localblockresY;

            outer_x           = localStartX;
            outer_y           = localStartY;
            local_outer_count = 0;

            while ((outer_x >= block_size_x && local_inner_stepx < 0) ||
                   (outer_x < 0 && local_inner_stepx > 0) ||
                   (outer_y >= block_size_y && local_inner_stepy < 0) ||
                   (outer_y < 0 && local_inner_stepy > 0))
            {
                outer_x += local_inner_stepx;
                outer_y += local_inner_stepy;
            }

            //local_outer_loop_in_bounds()
            while ((outer_x < block_size_x) &&
                   (outer_y < block_size_y) &&
                   (outer_x >= 0) &&
                   (outer_y >= 0) &&
                   (local_outer_count <= local_loop_exec_count))
            {
                mid_x    = outer_x;
                mid_y    = outer_y;
                mid_step = 0;
                //local_middle_steps_remaining()
                while ((mid_step <= extrasteps) &&
                       (mid_x < block_size_x) &&
                       (mid_y < block_size_y) &&
                       (mid_x >= 0) &&
                       (mid_y >= 0))
                {
                    local_inner_x = mid_x;
                    local_inner_y = mid_y;

                    //local_inner_loop_shrinking()
                    while ((local_inner_x < block_size_x) &&
                           (local_inner_y < block_size_y) &&
                           (local_inner_x >= 0) &&
                           (local_inner_y >= 0))
                    {
                        x = local_inner_x + global_inner_x_copy;
                        y = local_inner_y + global_inner_y_copy;

                        CmKernelEmu *kernel = (CmKernelEmu *)pThreadSpaceUnit[y * width + x].pKernel;

                        if (kernel == nullptr)
                        {
                            GFX_EMU_ASSERT(0);
                            result = CM_FAILURE;
                            goto finish;
                        }

                        fncPt = (void (*)())kernel->GetFuncPnt();

                        //kernel->GetArgs(pArg);
                        kernel->GetMaxArgCount(numMaxArgs);

                        set_thread_origin_x(x);
                        set_thread_origin_y(y);
                        for (uint32_t c = 0; c < colorCount; ++c)
                        {
                            set_color(c);

                            Execute(*kernel, pThreadSpaceUnit[y * width + x].threadId);
                            numExecuted++;
                        }

                        local_inner_x += local_inner_stepx;
                        local_inner_y += local_inner_stepy;
                    }
                    mid_step++;
                    mid_x += middle_stepx;
                    mid_y += middle_stepy;
                }
                local_outer_count += 1;
                outer_x += local_outer_stepx;
                outer_y += local_outer_stepy;
                while ((outer_x >= block_size_x && local_inner_stepx < 0) ||
                       (outer_x < 0 && local_inner_stepx > 0) ||
                       (outer_y >= block_size_y && local_inner_stepy < 0) ||
                       (outer_y < 0 && local_inner_stepy > 0))
                {
                    outer_x += local_inner_stepx;
                    outer_y += local_inner_stepy;
                }
            }
            global_inner_x += global_inner_stepx;
            global_inner_y += global_inner_stepy;
        }
        global_outer_count += 1;
        global_outer_x += global_outer_stepx;
        global_outer_y += global_outer_stepy;
        while (((global_outer_x >= globalresX) && (global_inner_stepx < 0)) ||
               (((global_outer_x + localblockresX) < 0) && (global_inner_stepx > 0)) ||
               ((global_outer_y >= globalresY) && (global_inner_stepy < 0)) ||
               (((global_outer_x + localblockresY) < 0) && (global_inner_stepy > 0)))
        {
            global_outer_x += global_inner_stepx;
            global_outer_y += global_inner_stepy;
        }
    }

finish:
    return result;
}

CM_RT_API int32_t CmQueueEmu::EnqueueInitSurface2D(CmSurface2D *pSurface, const uint32_t initValue, CmEvent *&pEvent)
{
    int32_t ret = CM_FAILURE;
    ret         = pSurface->InitSurface(initValue, pEvent);

    if (ret != CM_SUCCESS)
    {
        return CM_FAILURE;
    }

    if (pEvent != INVISIBLE_EVENT_MAGIC_NUM)
    {
        CmEventEmu *pTmp = nullptr;
        if (CmEventEmu::Create(m_EventCount, pTmp) == CM_SUCCESS)
        {
            m_EventArray.SetElement(m_EventCount, pTmp);
            m_EventCount++;
            pEvent = static_cast<CmEvent *>(pTmp);
        }
        else
        {
            return CM_FAILURE;
        }
    }
    else
    {
        // if the input pEvent equals to INVISIBLE_EVENT_MAGIC_NUM, event will not be created.
        pEvent = nullptr;
    }
    return ret;
}

CM_RT_API int32_t CmQueueEmu::EnqueueCopyGPUToGPU(CmSurface2D *pOutputSurface, CmSurface2D *pInputSurface, uint32_t option, CmEvent *&pEvent)
{
    int32_t     ret  = CM_FAILURE;
    CmEventEmu *pTmp = nullptr;

    CmSurface2DEmu *pSrcSurf2D = nullptr;
    pSrcSurf2D                 = dynamic_cast<CmSurface2DEmu *>(pInputSurface);

    CmSurface2DEmu *pDstSurf2D = nullptr;
    pDstSurf2D                 = dynamic_cast<CmSurface2DEmu *>(pOutputSurface);

    if ((pSrcSurf2D == nullptr) || (pDstSurf2D == nullptr))
    {
        return CM_FAILURE;
    }

    uint32_t          DstSurfaceWidth  = pDstSurf2D->GetWidth();
    uint32_t          DstSurfaceHeight = pDstSurf2D->GetHeight();
    CmSurfaceFormatID DstSurfaceFormat = INVALID_SURF_FORMAT;
    pDstSurf2D->GetSurfaceFormat(DstSurfaceFormat);

    uint32_t          SrcSurfaceWidth  = pSrcSurf2D->GetWidth();
    uint32_t          SrcSurfaceHeight = pSrcSurf2D->GetHeight();
    CmSurfaceFormatID SrcSurfaceFormat = INVALID_SURF_FORMAT;
    pSrcSurf2D->GetSurfaceFormat(SrcSurfaceFormat);

    if ((DstSurfaceWidth != SrcSurfaceWidth) ||
        (DstSurfaceHeight != SrcSurfaceHeight) ||
        (DstSurfaceFormat != SrcSurfaceFormat))
    {
        return CM_GPUCOPY_INVALID_SURFACES;
    }

    void *Dstbuffer = pDstSurf2D->getBuffer();
    pInputSurface->ReadSurface((unsigned char *)Dstbuffer, nullptr);

    if (pEvent != INVISIBLE_EVENT_MAGIC_NUM)
    {
        CmEventEmu *pTmp = nullptr;
        if (CmEventEmu::Create(m_EventCount, pTmp) == CM_SUCCESS)
        {
            m_EventArray.SetElement(m_EventCount, pTmp);
            m_EventCount++;
            pEvent = static_cast<CmEvent *>(pTmp);
        }
        else
        {
            return CM_FAILURE;
        }
    }
    else
    {
        // if the input pEvent equals to INVISIBLE_EVENT_MAGIC_NUM, event will not be created.
        pEvent = nullptr;
    }
    return CM_SUCCESS;
}

#define BYTE_COPY_ONE_THREAD 1024  //1K for each thread
CM_RT_API int32_t CmQueueEmu::EnqueueCopyCPUToCPU(unsigned char *pDstSysMem, unsigned char *pSrcSysMem, uint32_t size, uint32_t option, CmEvent *&pEvent)
{
    int         result              = CM_SUCCESS;
    CmEventEmu *pTmp                = nullptr;
    size_t      InputLinearAddress  = (size_t)pSrcSysMem;
    size_t      OutputLinearAddress = (size_t)pDstSysMem;

    size_t pInputLinearAddressAligned  = 0;
    size_t pOutputLinearAddressAligned = 0;

    size_t SrcLeftShiftOffset = 0;
    size_t DstLeftShiftOffset = 0;

    if ((InputLinearAddress & 0xf) || (OutputLinearAddress & 0xf) ||
        (InputLinearAddress == 0) || (OutputLinearAddress == 0))
    {
        return CM_GPUCOPY_INVALID_SYSMEM;
    }

    // Get page aligned address
#ifdef __CT__
    pInputLinearAddressAligned  = InputLinearAddress & 0xFFFFFFFFFFFF000;   // make sure the address page aligned.
    pOutputLinearAddressAligned = OutputLinearAddress & 0xFFFFFFFFFFFF000;  // make sure the address page aligned.
#else
    pInputLinearAddressAligned  = InputLinearAddress & 0xFFFFF000;   // make sure the address page aligned.
    pOutputLinearAddressAligned = OutputLinearAddress & 0xFFFFF000;  // make sure the address page aligned.
#endif

    SrcLeftShiftOffset = InputLinearAddress - pInputLinearAddressAligned;
    DstLeftShiftOffset = OutputLinearAddress - pOutputLinearAddressAligned;

    if ((size & 0xf) ||
        ((size + SrcLeftShiftOffset) > CM_MAX_1D_SURF_WIDTH) ||
        ((size + DstLeftShiftOffset) > CM_MAX_1D_SURF_WIDTH))
    {
        return CM_GPUCOPY_INVALID_SIZE;
    }

    CmFastMemCopy(pDstSysMem, pSrcSysMem, size);  //SSE copy used in CMRT.

    if (size >= BYTE_COPY_ONE_THREAD &&
        pEvent != INVISIBLE_EVENT_MAGIC_NUM)
    {
        CmEventEmu *pTmp = nullptr;
        if (CmEventEmu::Create(m_EventCount, pTmp) == CM_SUCCESS)
        {
            m_EventArray.SetElement(m_EventCount, pTmp);
            m_EventCount++;
            pEvent = static_cast<CmEvent *>(pTmp);
        }
        else
        {
            return CM_FAILURE;
        }
    }
    else
    {
        //less than 1K
        pEvent = nullptr;
    }
    return CM_SUCCESS;
}

CM_RT_API int32_t CmQueueEmu::EnqueueReadBuffer(CmBuffer* buffer,
                                                size_t offset,
                                                const unsigned char* sysMem,
                                                uint64_t sysMemSize,
                                                CmEvent* wait_event,
                                                CmEvent*& event,
                                                unsigned option)
{
    return CmNotImplemented(__PRETTY_FUNCTION__);
}

CM_RT_API int32_t CmQueueEmu::EnqueueWriteBuffer(CmBuffer* buffer,
                                                 size_t offset,
                                                 const unsigned char* sysMem,
                                                 uint64_t sysMemSize,
                                                 CmEvent* wait_event,
                                                 CmEvent*& event,
                                                 unsigned option)
{
    return CmNotImplemented(__PRETTY_FUNCTION__);
}

uint32_t CmQueueEmu::GetThreadCount(CmKernelEmu *kernel,
    CmThreadSpaceEmu *                           threadSpace)
{
    uint32_t threadCount = 0;
    if (kernel)
    {
        kernel->GetThreadCount(threadCount);
    }

    if (threadCount == 0)
    {
        uint32_t width  = 0;
        uint32_t height = 0;
        if (threadSpace)
        {
            threadSpace->GetThreadSpaceSize(width, height);
        }

        threadCount = width * height;
    }

    return threadCount;
}

CM_RT_API int32_t CmQueueEmu::SetResidentGroupAndParallelThreadNum(uint32_t residentGroupNum, uint32_t parallelThreadNum)
{
    m_ResidentGroupNum  = residentGroupNum;
    m_ParallelThreadNum = parallelThreadNum;
    return CM_SUCCESS;
}
