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

#include "cm_queue_base.h"
#include "cm_include.h"
#include "cm_debug.h"
#include "cm_device_emumode.h"
#include "cm_kernel_emumode.h"
#include "cm_surface_manager_emumode.h"
#include "cm_surface_2d_emumode.h"
#include "cm_buffer_emumode.h"
#include "cm_surface_3d_emumode.h"
#include "cm_queue_emumode.h"
#include "cm_program_emumode.h"
#include "cm_thread_space_emumode.h"
#include "cm_group_space_emumode.h"
#include "cm_utils.h"

#include "cm.h"

#include "cm_task_emumode.h"

#define CM_ZERO_TILEMASK (-1)

CmEmuPlatformUse CmDeviceEmu::CurrentPlatform = CmEmuPlatformUse::UNDEFINED;

int32_t CmDeviceEmu::Create( CmDeviceEmu* &pDevice)
{
    int32_t result;
    pDevice = new CmDeviceEmu();
    if( pDevice )
    {
        pDevice->Acquire();
        result = pDevice->Initialize();
        if( result != CM_SUCCESS )
        {
            CmDeviceEmu::Destroy( pDevice);
        }
    }
    else
    {
        CmAssert( 0 );
        result = CM_OUT_OF_HOST_MEMORY;
    }
    // leave critical section
    return result;
}

int32_t CmDeviceEmu::Initialize()
{
    int32_t result = CM_SUCCESS;
    m_pSurfaceMgr = nullptr;

    m_HalMaxValues.maxTasks = CM_MAX_TASKS;
    m_HalMaxValues.maxKernelsPerTask = CM_MAX_KERNELS_PER_TASK;
    m_HalMaxValues.maxKernelBinarySize = CM_MAX_KERNEL_BINARY_SIZE;
    m_HalMaxValues.maxSpillSizePerHwThread = CM_MAX_SPILL_SIZE_IN_BYTE_PER_HW_THREAD;
    m_HalMaxValues.maxSamplerTableSize = CM_MAX_SAMPLER_TABLE_SIZE;
    m_HalMaxValues.maxBufferTableSize = CM_MAX_BUFFER_TABLE_SIZE;
    m_HalMaxValues.max2DSurfaceTableSize = CM_MAX_2DSURFACE_TABLE_SIZE;
    m_HalMaxValues.max3DSurfaceTableSize = CM_MAX_3DSURFACE_TABLE_SIZE;
    m_HalMaxValues.maxArgsPerKernel = CM_MAX_ARG_COUNT;
    m_HalMaxValues.maxArgByteSizePerKernel = CM_MAX_ARG_BYTE_PER_KERNEL;
    m_HalMaxValues.maxSurfacesPerKernel = CM_MAX_SURFACES_PER_KERNEL;
    m_HalMaxValues.maxSamplersPerKernel = CM_MAX_SAMPLERS_PER_KERNEL;
    m_HalMaxValues.maxHwThreads = CM_MAX_HW_THREADS;
    m_HalMaxValues.maxUserThreadsPerTask = CM_MAX_USER_THREADS_PER_TASK;
    m_HalMaxValues.maxUserThreadsPerTaskNoThreadArg = CM_MAX_USER_THREADS_PER_TASK_NO_THREADARG;
    m_HalMaxValuesEx.max2DUPSurfaceTableSize = CM_MAX_2DSURFACEUP_TABLE_SIZE;
    m_HalMaxValuesEx.maxSampler8x8TableSize = CM_MAX_SAMPLER_8X8_TABLE_SIZE;

    result = CmSurfaceManagerEmu::Create(m_pSurfaceMgr,
        m_HalMaxValues, m_HalMaxValuesEx);

    auto envPlatformStr = CmUtils::String::toLower(
        CmUtils::String::getEnvStrNoSynch (CM_RT_PLATFORM));
    if (envPlatformStr == "icl") envPlatformStr = "icllp";

    char* envSku         = nullptr;
    char* envMaxThreads  = nullptr;

    CM_GETENV(envSku, CM_RT_SKU);
    CM_GETENV(envMaxThreads, CM_RT_MAX_THREADS);

    CmDeviceEmu::CurrentPlatform = CmEmuPlatformUse::UNDEFINED;
    auto subPlatform = CM_SUB_PLATFORM_USE_SKU::CM_SKU_UNDEFINED;

    if(envPlatformStr != "")
    {
        bool platformSet = false;

#define SET_CurrentPlatform(c) \
        if(envPlatformStr == CmUtils::String::toLower(#c)) { \
            CmDeviceEmu::CurrentPlatform = CmEmuPlatformUse:: c; platformSet = true; }
        SET_CurrentPlatform(BDW);
        SET_CurrentPlatform(SKL);
        SET_CurrentPlatform(BXT);
        SET_CurrentPlatform(KBL);
        SET_CurrentPlatform(ICLLP);
        SET_CurrentPlatform(TGLLP);
#undef SET_CurrentPlatform

        if (!platformSet)
        {
            CmAssertMessage(false, "Unknown platform supplied: %s", envPlatformStr.c_str());
            return CM_FAILURE;
        }
    }

    if (envSku)
    {
        auto skuNumber = 0;
        sscanf(envSku, "gt%d", &skuNumber);
        subPlatform = static_cast<CM_SUB_PLATFORM_USE_SKU>(skuNumber);
    }

    size_t threadsPerEU = 0,
           euPerSubslice = 0,
           maxThreadsNum = 0;

    // -----
    auto initSubPlatformCfg = [&] (
            auto& maxThreadsDataRef,
            auto& threadsPerEuDataRef,
            auto& euPerSubsliceDataRef,
            const std::vector<int>& validSubplatforms,
            CM_SUB_PLATFORM_USE_SKU defaultSubplatform = CM_SUB_PLATFORM_USE_SKU::CM_SKU_UNDEFINED
        )
        {
            if (!std::any_of(
                    validSubplatforms.begin(),
                    validSubplatforms.end (),
                    [=] (auto a) { return a == subPlatform; })
            ) {
                CmPrintMessage("Supplied subplatform %s is not defined for currently"
                    " selected platform %s, using default.\n", envSku, envPlatformStr.c_str());

                if (defaultSubplatform == CM_SUB_PLATFORM_USE_SKU::CM_SKU_UNDEFINED)
                {
                    CmFailWithMessage("Supported suplatform must be supplied.");
                }
                else
                    subPlatform = defaultSubplatform;
            }

            threadsPerEU   = threadsPerEuDataRef[subPlatform];
            euPerSubslice  = euPerSubsliceDataRef[subPlatform];
            maxThreadsNum  = maxThreadsDataRef[subPlatform];

            if (!threadsPerEU || !euPerSubslice || !maxThreadsNum) {
                if(!threadsPerEU)
                    CmErrorMessage("Threads per EU can't be zero, check config.");
                if(!euPerSubslice)
                    CmErrorMessage("EU per subslice can't be zero, check config.");
                if(!maxThreadsNum)
                    CmErrorMessage("Max threads number can't be zero, check config.");

                CmErrorMessage("Supplied subplatform %s for platform %s", envSku, envPlatformStr.c_str());
                exit(EXIT_FAILURE);
            }
        };

    switch(CmDeviceEmu::CurrentPlatform)
    {
        case CmEmuPlatformUse::SKL:
            initSubPlatformCfg (
                skl_max_threads,
                skl_threads_per_eu,
                skl_eu_per_subslice,
                {GT1,GT2,GT3,GT4},GT2);
             break;
        case CmEmuPlatformUse::BXT:
            initSubPlatformCfg (
                bxt_max_threads,
                bxt_threads_per_eu,
                bxt_eu_per_subslice,
                {GTA,GTC,GTX},GTX);
            break;
        case CmEmuPlatformUse::KBL:
            initSubPlatformCfg (
                kbl_max_threads,
                kbl_threads_per_eu,
                kbl_eu_per_subslice,
                {GT1,GT2,GT3,GT4},GT2);
            break;
        case CmEmuPlatformUse::ICLLP:
            initSubPlatformCfg (
                icllp_max_threads,
                icllp_threads_per_eu,
                icllp_eu_per_subslice,
                {GT1,GT2},GT2);
            break;
        case CmEmuPlatformUse::TGLLP:
            initSubPlatformCfg (
                tgllp_max_threads,
                tgllp_threads_per_eu,
                tgllp_eu_per_subslice,
                {GT1,GT2},GT2);
            break;
        default:
        {
            CmAssertMessage(false, "Platform %s is not supported.", envPlatformStr.c_str ());
            return CM_FAILURE;
        }
    }

    CmAssert(maxThreadsNum * threadsPerEU * euPerSubslice);

    {
        int user_defined_max_threads_number = envMaxThreads ? atoi(envMaxThreads) : 0;
        if (user_defined_max_threads_number &&
            user_defined_max_threads_number < maxThreadsNum)
                maxThreadsNum = user_defined_max_threads_number;
    }
    m_HalMaxValues.maxHwThreads = maxThreadsNum;

    m_HalMaxValuesEx.maxUserThreadsPerMediaWalker =
        CmDeviceEmu::CurrentPlatform < CmEmuPlatformUse::SKL ?
            CM_MAX_USER_THREADS_PER_MEDIA_WALKER_PRE_SKL :
        CmDeviceEmu::CurrentPlatform >= CmEmuPlatformUse::SKL &&
        (CmDeviceEmu::CurrentPlatform <= CmEmuPlatformUse::KBL
        )?
            CM_MAX_USER_THREADS_PER_MEDIA_WALKER_SKL_PLUS :
            CM_MAX_USER_THREADS_PER_MEDIA_WALKER_GEN11_PLUS;

    m_HalMaxValuesEx.maxUserThreadsPerThreadGroup = threadsPerEU*euPerSubslice;

    CM_GETENV_FREE(envSku);
    CM_GETENV_FREE(envMaxThreads);

    return result;
}

CM_RT_API int32_t CmDeviceEmu::CreateKernel( CmProgram* pProgram,
                                             const char* kernelName,
                                             const void *fncPnt,
                                             CmKernel* & pKernel,
                                             const char* options )
{

    if( !pProgram ||
        !kernelName ||
        !strcmp(kernelName, "") ||
        !fncPnt)
    {
        CmAssert(0);
        return CM_INVALID_ARG_VALUE;
    }

    CLock locker(m_CriticalSection_Kernel);

    CmKernelEmu * kernelPtr = nullptr;
    int32_t result = CmKernelEmu::Create( this, pProgram, kernelName, fncPnt, kernelPtr, options );
    if( result == CM_SUCCESS )
    {
        m_KernelArray.SetElement( m_KernelCount, kernelPtr );
        m_KernelCount ++;
        pKernel = static_cast< CmKernel* >(kernelPtr);
    }

    return result;
}

CM_RT_API int32_t CmDeviceEmu::CreateTask(CmTask *& pKernelArray)
{
    CmKernelArrayEmu* ptmp = nullptr;
    int32_t result = CmKernelArrayEmu::Create(m_HalMaxValues.maxKernelsPerTask, ptmp);
    if( result == CM_SUCCESS )
    {
       pKernelArray = static_cast< CmTask* >(ptmp);
    }
    return result;
}

CM_RT_API int32_t CmDeviceEmu::CreateBuffer(uint32_t width, CmBuffer* & pSurface )
{
    CmBufferEmu * ptemp = nullptr;
    if( ( width < CM_MIN_SURF_WIDTH ) || ( width > CM_MAX_1D_SURF_WIDTH ) )
    {
        CmAssert(0);
        return CM_INVALID_WIDTH;
    }

    CLock locker(m_CriticalSection_Surface);
    void *pSysMem = nullptr;
    int32_t status = m_pSurfaceMgr->CreateBuffer(width, ptemp, pSysMem);
    pSurface = static_cast<CmBuffer*>(ptemp);
    return status;
}

CM_RT_API int32_t CmDeviceEmu::CreateBufferEx(uint32_t width, CmBuffer* & pSurface, uint32_t tileID)
{
    uint32_t genTileCount;
    size_t   size = sizeof(genTileCount);

    GetCaps(CAP_MAX_SUBDEV_COUNT, size, &genTileCount);

    if ((tileID >= genTileCount))
    {
        CmFailWithMessage("TileID more than supported. Failed to create a buffer!");
    }

    int32_t status = CreateBuffer(width, pSurface );

    if (CM_SUCCESS == status)
    {
        (static_cast<CmBufferEmu*>(pSurface))->SetDeviceTileID(tileID);
    }

    return status;
}

CM_RT_API int32_t CmDeviceEmu::CreateSurface2D(uint32_t width,
                                               uint32_t height,
                                               CM_SURFACE_FORMAT format,
                                               CmSurface2D* & pSurface )
{
    CmSurface2DEmu * ptemp = nullptr;

    if( ( width < CM_MIN_SURF_WIDTH ) || ( width > CM_MAX_2D_SURF_WIDTH ) )
    {
        CmAssert(0);
        return CM_INVALID_WIDTH;
    }

    if( height < CM_MIN_SURF_HEIGHT )
    {
        CmAssert(0);
        return CM_INVALID_HEIGHT;
    }

    if ( height > CM_MAX_2D_SURF_HEIGHT )
    {
            CmAssert(0);
            return CM_INVALID_HEIGHT;
    }

    if( ( format == CM_SURFACE_FORMAT_NV12 ) &&
        ( height & 0x1 ) )
    {
        CmAssert( 0 );
        return CM_INVALID_HEIGHT;
    }

    if( ( format == CM_SURFACE_FORMAT_NV12 ) &&
        ( width & 0x1 ) )
    {
        CmAssert( 0 );
        return CM_INVALID_WIDTH;
    }

    if( ( format == CM_SURFACE_FORMAT_YUY2 ) &&
        ( width & 0x1 ) )
    {
        CmAssert( 0 );
        return CM_INVALID_WIDTH;
    }

    CLock locker(m_CriticalSection_Surface);
    int32_t status = m_pSurfaceMgr->CreateSurface2D( width, height, format, ptemp );

    pSurface = static_cast< CmSurface2D* >(ptemp);
    return status;
}

CM_RT_API int32_t CmDeviceEmu::CreateSurface2D( VASurfaceID iVASurface, CmSurface2D* & pSurface )
{
    return CmNotImplemented(__PRETTY_FUNCTION__);
};

CM_RT_API int32_t CmDeviceEmu::CreateSurface2D( VASurfaceID* iVASurface,
                                                const uint32_t surfaceCount,
                                                CmSurface2D** pSurface )
{
    return CmNotImplemented(__PRETTY_FUNCTION__);
};

/*CM_RT_API*/ int32_t CmDeviceEmu::Destroy( CmDeviceEmu* &pDevice )
{
    if (pDevice == nullptr)
    {
        CmAssert(0);
        return CM_NULL_POINTER;
    }

    int refCount = pDevice->SafeRelease();
    if (refCount == 0)
    {
        pDevice = nullptr;
    }

    return CM_SUCCESS;
}

CM_RT_API int32_t CmDeviceEmu::DestroyTask( CmTask*& pKernelArray)
{
    CmKernelArrayEmu* temp = dynamic_cast< CmKernelArrayEmu* >(pKernelArray);
    if( temp == nullptr )
    {
        return CM_FAILURE;
    }

    int32_t status = CmTask_RT::Destroy(temp);
    if(status == CM_SUCCESS)
    {
        pKernelArray = nullptr;
     }

    return status;
}

//!
//! Create a task queue, CmQueue. It is an in-order queue of tasks. Each task can
//! have multiple kernels running concurrently, each kernel can run in multiple threads.
//! For now only one CmQueue is supported. Trying to create a second CmQueue will fail.
//! Input :
//!     Reference to the pointer to the CmQueue .
//! Output:
//!     CM_SUCCESS if the CmQueue is successfully created;
//!     CM_OUT_OF_HOST_MEMORY if out of system memory;
//!     CM_FAILURE otherwise;
//!
CM_RT_API int32_t CmDeviceEmu::CreateQueue( CmQueue* & pQueue )
{
    if( m_pQueue )
    {
        pQueue = static_cast< CmQueue* >(m_pQueue);
        return CM_SUCCESS;
    }

    int32_t result = CmQueueEmu::Create( this, m_pQueue );
    if( result == CM_SUCCESS )
    {
        pQueue = static_cast< CmQueue* >(m_pQueue);
    }
    else
    {
        CmErrorMessage("Failed to create queue!");
        CmAssert( 0 )
    }

    return result;
}

CM_RT_API int32_t CmDeviceEmu::LoadProgram( void* pCommonISACode,
                                            const uint32_t size,
                                            CmProgram*& pProgram,
                                            const char* options )
{
    CmProgramEmu* pTmpProgramEmu = nullptr;
    int32_t result = CmProgramEmu::Create( this, pTmpProgramEmu );
    if( result == CM_SUCCESS )
    {
        m_ProgramArray.SetElement( m_ProgramCount, pTmpProgramEmu );
        m_ProgramCount ++;
        pProgram = static_cast< CmProgram* >(pTmpProgramEmu);
    }
    return result;
}

CM_RT_API int32_t CmDeviceEmu::DestroySurface(CmSurface2D *&pSurface)
{
    if(pSurface == nullptr)
    {
        return CM_FAILURE;
    }
    CLock locker(m_CriticalSection_Surface);
    CmSurface2DEmu* temp = (CmSurface2DEmu*)(pSurface);
    int32_t status = this->m_pSurfaceMgr->DestroySurface(temp);
    if(status == CM_SUCCESS)
    {
        pSurface = nullptr;
    }
    return status;
}

CM_RT_API int32_t CmDeviceEmu::DestroySurface(CmBuffer *&pSurface)
{
    if(pSurface == nullptr)
    {
        return CM_FAILURE;
    }
    CLock locker(m_CriticalSection_Surface);
    CmBufferEmu* temp = (CmBufferEmu*)(pSurface);
    int32_t status = this->m_pSurfaceMgr->DestroySurface(temp);
    if(status == CM_SUCCESS)
    {
        pSurface = nullptr;
    }
    return status;
}

CmDeviceEmu::CmDeviceEmu():
    m_pQueue( nullptr ),
    m_pSurfaceMgr( nullptr ),
    m_ProgramArray( CM_INIT_PROGRAM_COUNT ),
    m_ProgramCount( 0 ),
    m_KernelArray( CM_INIT_KERNEL_COUNT ),
    m_KernelCount( 0 ),
    m_tileCount(0),
    m_refcount(0)
{
    CmEmulSys::init_tm();
}

CmDeviceEmu::~CmDeviceEmu()
{
    CLock locker(m_CriticalSection_Surface);
    CLock locker2(m_CriticalSection_Kernel);

    CmSurfaceManagerEmu::Destroy( m_pSurfaceMgr );
    CmQueueEmu::Destroy(this->m_pQueue);

    for_each(m_queueArray.begin(), m_queueArray.end(),
             CmQueueEmu::Destroy);

    for( uint32_t i = 0; i < m_KernelCount; i ++ )
    {
        CmKernelEmu* pKernel = (CmKernelEmu* )m_KernelArray.GetElement( i );
        if( pKernel )
        {
            CmKernelEmu::Destroy( pKernel );
        }
    }
    m_KernelArray.Delete();

    for( uint32_t i = 0; i < m_ProgramCount; i ++ )
    {
        CmProgramEmu* pProgram = (CmProgramEmu*)m_ProgramArray.GetElement( i );
        if( pProgram )
        {
            CmProgramEmu::Destroy( pProgram );
        }
    }
    m_ProgramArray.Delete();

    CmEmulSys::finalize_tm();
}

CM_RT_API int32_t CmDeviceEmu::DestroyKernel( CmKernel*& pKernel)
{
    CLock locker(m_CriticalSection_Kernel);
    CmKernelEmu* temp = dynamic_cast< CmKernelEmu* >(pKernel);
    if( temp == nullptr )
    {
        return CM_FAILURE;
    }
    for( uint32_t i = 0; i < m_KernelCount; i ++ )
    {
        if( temp == m_KernelArray.GetElement( i ) )
        {
            int32_t status = CmKernelEmu::Destroy( temp );
            if(status == CM_SUCCESS)
            {
                m_KernelArray.SetElement( i, nullptr );
                pKernel = nullptr;
            }
            if(i == m_KernelCount -1)
            {
                for( int j = m_KernelCount - 1; j>=0; j--)
                {
                    if(m_KernelArray.GetElement( j ) == nullptr)
                    {
                        m_KernelCount--;
                    }
                }
            }
            return status;
        }
    }

    return CM_FAILURE;
}

CM_RT_API int32_t CmDeviceEmu::DestroyProgram( CmProgram*& pProgram )
{
    CmProgramEmu* pTemp = dynamic_cast< CmProgramEmu* >(pProgram);
    if( pTemp == nullptr )
    {
        return CM_FAILURE;
    }

    CLock locker(m_CriticalSection_Program);
    for( uint32_t i = 0; i < m_ProgramCount; i ++ )
    {
        if( pTemp == m_ProgramArray.GetElement( i ) )
        {
            CmProgramEmu::Destroy( pTemp );
            m_ProgramArray.SetElement( i, nullptr );
            pProgram = nullptr;

            return CM_SUCCESS;
        }
    }

    return CM_FAILURE;
}

CM_RT_API int32_t CmDeviceEmu::CreateSurface3D(uint32_t width,
                                               uint32_t height,
                                               uint32_t depth,
                                               CM_SURFACE_FORMAT format,
                                               CmSurface3D* & pSurface )
{
    CmSurface3DEmu * ptemp = nullptr;

    if( ( width < CM_MIN_SURF_WIDTH ) || ( width > CM_MAX_3D_SURF_WIDTH ) )
    {
        CmAssert(0);
        return CM_INVALID_WIDTH;

    }
    if( ( height < CM_MIN_SURF_HEIGHT ) || ( height > CM_MAX_3D_SURF_HEIGHT ) )
    {
        CmAssert(0);
        return CM_INVALID_HEIGHT;
    }
    if( ( depth < CM_MIN_SURF_DEPTH ) || ( depth > CM_MAX_3D_SURF_DEPTH ) )
    {
        CmAssert(0);
        return CM_INVALID_DEPTH;
    }
    CLock locker(m_CriticalSection_Surface);
    int32_t status = m_pSurfaceMgr->CreateSurface3D( width, height, depth, format, ptemp );
    pSurface = static_cast< CmSurface3D* >(ptemp);
    return status;
}

CM_RT_API int32_t CmDeviceEmu::DestroySurface( CmSurface3D* & pSurface)
{
    if(pSurface == nullptr)
    {
        return CM_FAILURE;
    }
    CLock locker(m_CriticalSection_Surface);
    CmSurface3DEmu* temp = (CmSurface3DEmu*)(pSurface);
    int32_t status = this->m_pSurfaceMgr->DestroySurface(temp);
    if(status == CM_SUCCESS)
    {
        pSurface = nullptr;
    }
    return status;
}

CM_RT_API int32_t CmDeviceEmu::CreateThreadSpace( uint32_t width, uint32_t height, CmThreadSpace* & pTS )
{
    CmThreadSpaceEmu *pTSSim = nullptr;
    int32_t result = CmThreadSpaceEmu::Create( this, width, height, pTSSim );
    pTS = static_cast<CmThreadSpace *>(pTSSim);
    if (result == CM_SUCCESS && (
        CmDeviceEmu::CurrentPlatform == CmEmuPlatformUse::TGLLP
    ))
    {
        CmThreadGroupSpace *pTGSSim = pTSSim->GetThreadGroupSpace();

        if (pTGSSim == nullptr)
        {
            result = CM_FAILURE;
        }
    }
    return result;
}

CM_RT_API int32_t CmDeviceEmu::DestroyThreadSpace( CmThreadSpace* & pTS )
{
    CmThreadSpaceEmu *pTSSim = dynamic_cast<CmThreadSpaceEmu *> (pTS);
    if(pTSSim == nullptr)
    {
        CmAssert( 0 );
        return CM_FAILURE;
    }
    int32_t result = CmThreadSpaceEmu::Destroy(pTSSim);
    pTS = nullptr;
    return result;
}

//! Function to create a thread group space
//! Arguments:
//!     1. Width/height (in unit of thread ) of each thread group
//!     2. Width/height(in unit of group) of thread group space.
//!     3. Reference to the point to CmThreadGroupSpace object to created.
//! Return Value:
//!     CM_SUCCESS if the CmThreadGroupSpace is successfully created
//! Notes:
//!     The total thread count is width*height*grpWidth*grpHeight.
//!     The thread count will check against the thread count set by CmKernel::SetThreadCount if CmKernel::SetThreadCount is called.
//!     CmKernel::SetThreadCount needs to be called if CmKernel::SetThreadArg is to be called.

CM_RT_API int32_t CmDeviceEmu::CreateThreadGroupSpace( uint32_t thrdSpaceWidth,
                                                       uint32_t thrdSpaceHeight,
                                                       uint32_t grpSpaceWidth,
                                                       uint32_t grpSpaceHeight,
                                                       CmThreadGroupSpace*& pTGS )
{
    CmDevice* pCmDev = dynamic_cast<CmDevice *>(this);
    int32_t result = CmThreadGroupSpace::Create( pCmDev, thrdSpaceWidth, thrdSpaceHeight, grpSpaceWidth, grpSpaceHeight, pTGS );
    return result;
}

CM_RT_API int32_t CmDeviceEmu::CreateThreadGroupSpaceEx(uint32_t thrdSpaceWidth,
                                                        uint32_t thrdSpaceHeight,
                                                        uint32_t thrdSpaceDepth,
                                                        uint32_t grpSpaceWidth,
                                                        uint32_t grpSpaceHeight,
                                                        uint32_t grpSpaceDepth,
                                                        CmThreadGroupSpace*& pTGS)
{
    CmDevice* pCmDev = dynamic_cast<CmDevice *>(this);
    int32_t result = CmThreadGroupSpace::Create(pCmDev, thrdSpaceWidth, thrdSpaceHeight, thrdSpaceDepth, grpSpaceWidth, grpSpaceHeight, grpSpaceDepth, pTGS);
    return result;
}

CM_RT_API int32_t CmDeviceEmu::DestroyThreadGroupSpace(CmThreadGroupSpace*& pTGS)
{
    if(pTGS == nullptr)
    {
        CmAssert( 0 );
        return CM_FAILURE;
    }
    int32_t result = CmThreadGroupSpace::Destroy(pTGS);
    pTGS = nullptr;
    return result;
}

CM_RT_API int32_t CmDeviceEmu::GetCaps(CM_DEVICE_CAP_NAME capName, size_t& capValueSize, void* pCapValue )
{
    switch( capName )
    {
    case CAP_KERNEL_COUNT_PER_TASK:
        if( capValueSize >= sizeof( m_HalMaxValues.maxKernelsPerTask ) )
        {
            capValueSize = sizeof( m_HalMaxValues.maxKernelsPerTask );
            CmSafeMemCopy( pCapValue, &m_HalMaxValues.maxKernelsPerTask, capValueSize );
            return CM_SUCCESS;
        }
        else
        {
            return CM_FAILURE;
        }

    case CAP_KERNEL_BINARY_SIZE:
        if( capValueSize >= sizeof( m_HalMaxValues.maxKernelBinarySize ) )
        {
            capValueSize = sizeof( m_HalMaxValues.maxKernelBinarySize );
            CmSafeMemCopy( pCapValue, &m_HalMaxValues.maxKernelBinarySize, capValueSize );
            return CM_SUCCESS;
        }
        else
        {
            return CM_FAILURE;
        }

    case CAP_SAMPLER_COUNT:
        if( capValueSize >= sizeof( m_HalMaxValues.maxSamplerTableSize ) )
        {
            capValueSize = sizeof( m_HalMaxValues.maxSamplerTableSize );
            CmSafeMemCopy( pCapValue, &m_HalMaxValues.maxSamplerTableSize, capValueSize );
            return CM_SUCCESS;
        }
        else
        {
            return CM_FAILURE;
        }

    case CAP_SAMPLER_COUNT_PER_KERNEL:
        if( capValueSize >= sizeof( m_HalMaxValues.maxSamplersPerKernel ) )
        {
            capValueSize = sizeof( m_HalMaxValues.maxSamplersPerKernel );
            CmSafeMemCopy( pCapValue, &m_HalMaxValues.maxSamplersPerKernel, capValueSize );
            return CM_SUCCESS;
        }
        else
        {
            return CM_FAILURE;
        }

    case CAP_BUFFER_COUNT:
        if( capValueSize >= sizeof( m_HalMaxValues.maxBufferTableSize ) )
        {
            capValueSize = sizeof( m_HalMaxValues.maxBufferTableSize );
            CmSafeMemCopy( pCapValue, &m_HalMaxValues.maxBufferTableSize, capValueSize );
            return CM_SUCCESS;
        }
        else
        {
            return CM_FAILURE;
        }

    case CAP_SURFACE2D_COUNT:
        if( capValueSize >= sizeof( m_HalMaxValues.max2DSurfaceTableSize ) )
        {
            capValueSize = sizeof( m_HalMaxValues.max2DSurfaceTableSize );
            CmSafeMemCopy( pCapValue, &m_HalMaxValues.max2DSurfaceTableSize, capValueSize );
            return CM_SUCCESS;
        }
        else
        {
            return CM_FAILURE;
        }

    case CAP_SURFACE3D_COUNT:
        if( capValueSize >= sizeof( m_HalMaxValues.max3DSurfaceTableSize ) )
        {
            capValueSize = sizeof( m_HalMaxValues.max3DSurfaceTableSize );
            CmSafeMemCopy( pCapValue, &m_HalMaxValues.max3DSurfaceTableSize, capValueSize );
            return CM_SUCCESS;
        }
        else
        {
            return CM_FAILURE;
        }

    case CAP_SURFACE2DUP_COUNT:
        if( capValueSize >= sizeof( m_HalMaxValuesEx.max2DUPSurfaceTableSize ) )
        {
            capValueSize = sizeof( m_HalMaxValuesEx.max2DUPSurfaceTableSize );
            CmSafeMemCopy( pCapValue, &m_HalMaxValuesEx.max2DUPSurfaceTableSize, capValueSize );
            return CM_SUCCESS;
        }
        else
        {
            return CM_FAILURE;
        }

    case CAP_SURFACE_COUNT_PER_KERNEL:
        if( capValueSize >= sizeof( m_HalMaxValues.maxSurfacesPerKernel ) )
        {
            capValueSize = sizeof( m_HalMaxValues.maxSurfacesPerKernel );
            CmSafeMemCopy( pCapValue, &m_HalMaxValues.maxSurfacesPerKernel, capValueSize );
            return CM_SUCCESS;
        }
        else
        {
            return CM_FAILURE;
        }

    case CAP_ARG_COUNT_PER_KERNEL:
        if( capValueSize >= sizeof( m_HalMaxValues.maxArgsPerKernel ) )
        {
            capValueSize = sizeof( m_HalMaxValues.maxArgsPerKernel );
            CmSafeMemCopy( pCapValue, &m_HalMaxValues.maxArgsPerKernel, capValueSize );
            return CM_SUCCESS;
        }
        else
        {
            return CM_FAILURE;
        }

    case CAP_ARG_SIZE_PER_KERNEL:
        if( capValueSize >= sizeof( m_HalMaxValues.maxArgByteSizePerKernel ) )
        {
            capValueSize = sizeof( m_HalMaxValues.maxArgByteSizePerKernel );
            CmSafeMemCopy( pCapValue, &m_HalMaxValues.maxArgByteSizePerKernel, capValueSize );
            return CM_SUCCESS;
        }
        else
        {
            return CM_FAILURE;
        }

    case CAP_USER_DEFINED_THREAD_COUNT_PER_TASK:
        if( capValueSize >= sizeof( m_HalMaxValues.maxUserThreadsPerTask ) )
        {
            capValueSize = sizeof( m_HalMaxValues.maxUserThreadsPerTask );
            CmSafeMemCopy( pCapValue, &m_HalMaxValues.maxUserThreadsPerTask, capValueSize );
            return CM_SUCCESS;
        }
        else
        {
            return CM_FAILURE;
        }

    case CAP_USER_DEFINED_THREAD_COUNT_PER_MEDIA_WALKER:
        if( capValueSize >= sizeof( m_HalMaxValuesEx.maxUserThreadsPerMediaWalker ) )
        {
            capValueSize = sizeof( m_HalMaxValuesEx.maxUserThreadsPerMediaWalker );
            CmSafeMemCopy( pCapValue, &m_HalMaxValuesEx.maxUserThreadsPerMediaWalker, capValueSize );
            return CM_SUCCESS;
        }
        else
        {
            return CM_FAILURE;
        }

    case CAP_USER_DEFINED_THREAD_COUNT_PER_THREAD_GROUP:
        if( capValueSize >= sizeof( m_HalMaxValuesEx.maxUserThreadsPerThreadGroup ) )
        {
            capValueSize = sizeof( m_HalMaxValuesEx.maxUserThreadsPerThreadGroup );
            CmSafeMemCopy( pCapValue, &m_HalMaxValuesEx.maxUserThreadsPerThreadGroup, capValueSize );
            return CM_SUCCESS;
        }
        else
        {
            return CM_FAILURE;
        }

    case CAP_USER_DEFINED_THREAD_COUNT_PER_TASK_NO_THREAD_ARG:
        if( capValueSize >= sizeof( m_HalMaxValues.maxUserThreadsPerTaskNoThreadArg ) )
        {
            capValueSize = sizeof( m_HalMaxValues.maxUserThreadsPerTaskNoThreadArg );
            CmSafeMemCopy( pCapValue, &m_HalMaxValues.maxUserThreadsPerTaskNoThreadArg, capValueSize);
            return CM_SUCCESS;
        }
        else
        {
            return CM_FAILURE;
        }

    case CAP_HW_THREAD_COUNT:
        if( capValueSize >= sizeof( m_HalMaxValues.maxHwThreads ) )
        {
            capValueSize = sizeof( m_HalMaxValues.maxHwThreads );
            CmSafeMemCopy( pCapValue, &m_HalMaxValues.maxHwThreads, capValueSize );
            return CM_SUCCESS;
        }
        else
        {
            return CM_FAILURE;
        }

    case CAP_SURFACE2D_FORMAT_COUNT:
        if( capValueSize >= sizeof( uint32_t ) )
        {
            capValueSize = sizeof( uint32_t );
            uint32_t formatCount = CM_MAX_SURFACE2D_FORMAT_COUNT;
            CmSafeMemCopy( pCapValue, &formatCount, capValueSize );
            return CM_SUCCESS;
        }
        else
        {
            return CM_FAILURE;
        }

    case CAP_SURFACE2D_FORMATS:
        if( capValueSize >= CM_MAX_SURFACE2D_FORMAT_COUNT  * sizeof( CM_SURFACE_FORMAT ) )
        {
            capValueSize = CM_MAX_SURFACE2D_FORMAT_COUNT  * sizeof( CM_SURFACE_FORMAT ) ;
            CM_SURFACE_FORMAT formats[ CM_MAX_SURFACE2D_FORMAT_COUNT ] =
            {   CM_SURFACE_FORMAT_A16B16G16R16,
                CM_SURFACE_FORMAT_R10G10B10A2,
                CM_SURFACE_FORMAT_X8R8G8B8,
                CM_SURFACE_FORMAT_A8R8G8B8,
                CM_SURFACE_FORMAT_R32G32B32A32F,
                CM_SURFACE_FORMAT_NV12,
                CM_SURFACE_FORMAT_YUY2,
                CM_SURFACE_FORMAT_UYVY,
                CM_SURFACE_FORMAT_A8,
                CM_SURFACE_FORMAT_P8,
                CM_SURFACE_FORMAT_R32F,
                CM_SURFACE_FORMAT_V8U8,
            };
            CmSafeMemCopy( pCapValue, formats, capValueSize );
            return CM_SUCCESS;
        }
        else
        {
            return CM_FAILURE;
        }

    case CAP_SURFACE3D_FORMAT_COUNT:
        if( capValueSize >= sizeof( uint32_t ) )
        {
            capValueSize = sizeof( uint32_t );
            uint32_t formatCount = CM_MAX_SURFACE3D_FORMAT_COUNT;
            CmSafeMemCopy( pCapValue, &formatCount, capValueSize );
            return CM_SUCCESS;
        }
        else
        {
            return CM_FAILURE;
        }

    case CAP_SURFACE3D_FORMATS:
        if( capValueSize >= CM_MAX_SURFACE3D_FORMAT_COUNT  * sizeof( CM_SURFACE_FORMAT ) )
        {
            capValueSize = CM_MAX_SURFACE3D_FORMAT_COUNT  * sizeof( CM_SURFACE_FORMAT ) ;
            CM_SURFACE_FORMAT formats[ CM_MAX_SURFACE3D_FORMAT_COUNT ] =
            {   CM_SURFACE_FORMAT_X8R8G8B8,
                CM_SURFACE_FORMAT_A8R8G8B8,
                CM_SURFACE_FORMAT_R32G32B32A32F
            };
            CmSafeMemCopy( pCapValue, formats, capValueSize );
            return CM_SUCCESS;
        }
        else
        {
            return CM_FAILURE;
        }

    case CAP_GPU_PLATFORM:
        if( capValueSize >= sizeof( uint32_t ) )
        {
            uint32_t platform = PLATFORM_INTEL_UNKNOWN;

            auto genPlatform = CmEmuPlatformUse::UNDEFINED;
            GetGenPlatform(genPlatform);

            capValueSize = sizeof( uint32_t );

            switch (genPlatform)
            {
                case CmEmuPlatformUse::BDW:
                    platform = PLATFORM_INTEL_BDW;
                    break;
                case CmEmuPlatformUse::SKL:
                    platform = PLATFORM_INTEL_SKL;
                    break;
                case CmEmuPlatformUse::BXT:
                    platform = PLATFORM_INTEL_BXT;
                    break;
                case CmEmuPlatformUse::ICLLP:
                    platform = PLATFORM_INTEL_ICLLP;
                    break;
                case CmEmuPlatformUse::TGLLP:
                    platform = PLATFORM_INTEL_TGLLP;
                    break;
                default:
                    break;
            }

            CmSafeMemCopy( pCapValue, &platform, capValueSize );
            return CM_SUCCESS;
        }
        else
        {
            return CM_FAILURE;
        }

    case CAP_GT_PLATFORM:
        if( capValueSize >= sizeof( uint32_t ) )
        {
            auto genPlatform = CmEmuPlatformUse::UNDEFINED;
            GetGenPlatform(genPlatform);

            if (genPlatform >= CmEmuPlatformUse::BXT)
            {
                return CmNotImplemented("getting CAP_GT_PLATFORM for platform >= BXT");
            }

            char * envSku = nullptr;
            CM_GETENV(envSku, CM_RT_SKU);

            capValueSize = sizeof( uint32_t );
            uint32_t gtPlatform = 0;

            if(envSku == nullptr)
                gtPlatform = PLATFORM_INTEL_GT2;
            else if(strcmp(envSku, "gt1")==0)
                gtPlatform = PLATFORM_INTEL_GT1;
            else if(strcmp(envSku, "gt2")==0)
                gtPlatform = PLATFORM_INTEL_GT2;
            else if(strcmp(envSku, "gt3")==0)
                gtPlatform = PLATFORM_INTEL_GT3;
            else if(strcmp(envSku, "gt4")==0)
                gtPlatform = PLATFORM_INTEL_GT4;
            else if(strcmp(envSku, "gta")==0)
                gtPlatform = PLATFORM_INTEL_GTA;
            else if(strcmp(envSku, "gtc")==0)
                gtPlatform = PLATFORM_INTEL_GTC;
            else if(strcmp(envSku, "gtx")==0)
                gtPlatform = PLATFORM_INTEL_GTX;
            else
                gtPlatform = PLATFORM_INTEL_GT_UNKNOWN;

            CmSafeMemCopy( pCapValue, &gtPlatform, capValueSize );

            CM_GETENV_FREE(envSku);

            return CM_SUCCESS;
        }
        else
        {
            return CM_FAILURE;
        }

    case CAP_PLATFORM_INFO:
        return CmNotImplemented("Getting CAP_PLATFORM_INFO");

    case CAP_MAX_BUFFER_SIZE:
        if (capValueSize >= sizeof(unsigned int))
        {
            capValueSize = sizeof(unsigned int);
            unsigned int maxBufferSize = CM_MAX_1D_SURF_WIDTH;
            CmSafeMemCopy(pCapValue, &maxBufferSize, capValueSize);
            return CM_SUCCESS;
        }
        else
        {
            return CM_FAILURE;
        }

    case CAP_MAX_SUBDEV_COUNT:
        if (capValueSize >= sizeof(unsigned int))
        {
            uint32_t genTileCount = 1;
            char *envTileCount = nullptr;
            CM_GETENV(envTileCount, CM_RT_TILE_COUNT);
            capValueSize = sizeof(uint32_t);

            if (envTileCount == nullptr)
                genTileCount = 1;
            else if (strcmp(envTileCount, "4") == 0)
                genTileCount = 4;
            else if (strcmp(envTileCount, "2") == 0)
                genTileCount = 2;
            else
                genTileCount = 1;

            CmSafeMemCopy(pCapValue, &genTileCount, capValueSize);

            CM_GETENV_FREE(envTileCount);

            return CM_SUCCESS;
        }
        else
        {
            return CM_FAILURE;
        }

    default:
        return CM_FAILURE;
    }
}

int32_t CmDeviceEmu::GetHalMaxValues( CM_HAL_MAX_VALUES* & pHalMaxValues )
{
    pHalMaxValues = &m_HalMaxValues;
    return CM_SUCCESS;
}

int32_t CmDeviceEmu::GetSurfManager( CmSurfaceManagerEmu* &pSurfaceMgr)
{
    pSurfaceMgr = this->m_pSurfaceMgr;
    return CM_SUCCESS;
}

int32_t CmDeviceEmu::GetSurfaceManagerEmu( CmSurfaceManagerEmu* & pSurfaceMgr )
{
    pSurfaceMgr = m_pSurfaceMgr;
    return CM_SUCCESS;
}

CM_RT_API int32_t CmDeviceEmu::CreateBufferUP(uint32_t size, void* pSysMem, CmBufferUP* & pSurface )
{
    CmBufferEmu * ptemp = nullptr;
    if((size < CM_MIN_SURF_WIDTH) || (size > CM_MAX_1D_SURF_WIDTH))
    {
        CmAssert(0);
        return CM_INVALID_WIDTH;
    }

    if(pSysMem == nullptr)
    {
        CmAssert(0);
        return CM_INVALID_ARG_VALUE;
    }
    CLock locker(m_CriticalSection_Surface);
    int32_t status = m_pSurfaceMgr->CreateBuffer(size, ptemp,pSysMem);
    if(status != CM_SUCCESS)
    {
        pSurface = nullptr;
        return status;
    }
    pSurface = static_cast<CmBufferUP*>(ptemp);
    ptemp->setISSmUpSurface();
    return status;
}

CM_RT_API int32_t CmDeviceEmu::DestroyBufferUP( CmBufferUP* & pSurface)
{
    if(pSurface == nullptr)
    {
        return CM_FAILURE;
    }
    CLock locker(m_CriticalSection_Surface);
    CmBufferEmu* temp = (CmBufferEmu*)(pSurface);
    int32_t status = this->m_pSurfaceMgr->DestroySurface(temp);
    if(status == CM_SUCCESS)
    {
        pSurface = nullptr;
    }
    return status;
}

int32_t CmDeviceEmu::DoCopyAll( )
{

    this->m_pSurfaceMgr->DoCopyAll();
    return CM_SUCCESS;
}

/**
    Copies from SRC to DST in case src was modified in the host.
*/
int32_t CmDeviceEmu::DoGPUCopySelect( )
{

    this->m_pSurfaceMgr->DoGPUCopySelect();
    return CM_SUCCESS;
}

CM_RT_API int32_t CmDeviceEmu::GetSurface2DInfo(uint32_t width,
                                                uint32_t height,
                                                CM_SURFACE_FORMAT format,
                                                uint32_t & pitch,
                                                uint32_t & physicalSize )
{
    int temp=0;
    uint32_t incr = 0;
    uint newHeight = 0;
    int32_t result = m_pSurfaceMgr->Surface2DSanityCheck(width, height, format);
    if (result != CM_SUCCESS)
    {
        CmAssert(0);
        return result;
    }
    switch( format )
    {
    case CM_SURFACE_FORMAT_NV12:
    case CM_SURFACE_FORMAT_P010:
    case CM_SURFACE_FORMAT_P016:
        pitch=width*m_pSurfaceMgr->getBytesPerPixel(format, &incr);
        newHeight = height+height/2;
        break;
    default:
        pitch = width*m_pSurfaceMgr->getBytesPerPixel(format, &incr);
        newHeight = height;
        break;
    }
    physicalSize = pitch*newHeight;
    return CM_SUCCESS;
}

CM_RT_API int32_t CmDeviceEmu::CreateSurface2DUP( uint32_t width,
                                                  uint32_t height,
                                                  CM_SURFACE_FORMAT format,
                                                  void* pSysMem,
                                                  CmSurface2DUP* & pSurface )
{

    CmSurface2DEmu * ptemp = nullptr;

    if( ( format == CM_SURFACE_FORMAT_NV12 ) &&
        ( height & 0x1 ) )
    {
        CmAssert( 0 );
        return CM_INVALID_HEIGHT;
    }

    if( ( format == CM_SURFACE_FORMAT_NV12 ) &&
        ( width & 0x1 ) )
    {
        CmAssert( 0 );
        return CM_INVALID_WIDTH;
    }

    if( ( format == CM_SURFACE_FORMAT_YUY2 ) &&
        ( width & 0x1 ) )
    {
        CmAssert( 0 );
        return CM_INVALID_WIDTH;
    }
    CLock locker(m_CriticalSection_Surface);
    int32_t status = m_pSurfaceMgr->CreateSurface2DUP( width, height, format, ptemp, pSysMem );
    pSurface = static_cast< CmSurface2DUP* >(ptemp);
    if(pSurface != nullptr)
    {
        ptemp->setISSmUpSurface();
    }
    return status;
}

CM_RT_API int32_t CmDeviceEmu::DestroySurface2DUP( CmSurface2DUP* & pSurface)
{
    if(pSurface == nullptr)
    {
        CmAssert( 0 );
        return CM_FAILURE;
    }
    CLock locker(m_CriticalSection_Surface);
    CmSurface2DEmu* temp = (CmSurface2DEmu*)(pSurface);
    int32_t status = this->m_pSurfaceMgr->DestroySurface2DUP(temp);
    if(status == CM_SUCCESS)
    {
        pSurface = nullptr;
    }
    return status;
}

int32_t CmDeviceEmu::GetGenPlatform(CmEmuPlatformUse& platform )
{
    platform = CmDeviceEmu::CurrentPlatform;
    return CM_SUCCESS;
}

bool CmDeviceEmu::IsValidSurfaceIndex(uint32_t surfBTI)
{
    CmEmuPlatformUse platformid;
    GetGenPlatform(platformid);

    if (platformid >= CmEmuPlatformUse::SKL)
    {
        if (surfBTI >= (
                CM_GLOBAL_SURFACE_INDEX_START_GEN9_PLUS + CM_MAX_GLOBAL_SURFACE_NUMBER
                ) && surfBTI < GT_RESERVED_INDEX_START_GEN9_PLUS)
            return true;
        else
            return false;
    }
    else
    {
        if (surfBTI > CM_NULL_SURFACE_BINDING_INDEX &&  surfBTI < CM_GLOBAL_SURFACE_INDEX_START)
            return true;
        else
            return false;
    }
}

CM_RT_API int32_t CmDeviceEmu::SetCaps(CM_DEVICE_CAP_NAME capsName, size_t capValueSize, void* pCapValue)
{
    return CmNotImplemented(__PRETTY_FUNCTION__);
}

// Emu mode could keep track of SVM buffers and ensure that each read/write is
// within a buffer. For now we don't do that.
CM_RT_API int32_t CmDeviceEmu::CreateBufferSVM( uint32_t size,
                                                void* & pSystMem,
                                                uint32_t access_flag,
                                                CmBufferSVM* & pSurface )
{
    CmBufferEmu * ptemp = nullptr;
    if( size < CM_MIN_SURF_WIDTH  )
    {
        CmAssert(0);
        return CM_INVALID_WIDTH;
    }

    CLock locker(m_CriticalSection_Surface);
    int32_t status = m_pSurfaceMgr->CreateBuffer(size, ptemp, pSystMem, true/*noRegisterBuffer*/);
    if(status != CM_SUCCESS)
    {
        pSurface = nullptr;
        return status;
    }
    pSurface = static_cast<CmBufferSVM*>(ptemp);
    return status;
}

CM_RT_API int32_t CmDeviceEmu::DestroyBufferSVM(CmBufferSVM *&pSurface)
{
    if(pSurface == nullptr)
    {
        return CM_FAILURE;
    }
    CLock locker(m_CriticalSection_Surface);
    CmBufferEmu* temp = (CmBufferEmu*)(pSurface);
    int32_t status = m_pSurfaceMgr->DestroySurface(temp);
    if(status == CM_SUCCESS)
    {
        pSurface = nullptr;
    }
    return status;
}

CM_RT_API int32_t CmDeviceEmu::CloneKernel( CmKernel * &pKernelDest, CmKernel *pKernelSrc )
{
    return CmNotImplemented(__PRETTY_FUNCTION__);
}

int32_t CmDeviceEmu::Acquire()
{
    ++m_refcount;
    return m_refcount;
}

int32_t CmDeviceEmu::SafeRelease()
{
    --m_refcount;
    if (m_refcount == 0)
    {
        delete this;
        return 0;
    }
    else
    {
        return m_refcount;
    }
}

CM_RT_API int32_t CmDeviceEmu::CreateBufferAlias(CmBuffer *buffer,
                                                 SurfaceIndex* &aliasIndex)
{
    int32_t result = CM_SUCCESS;

    if (!buffer)
    {
        CmAssert(0);
        CmErrorMessage("Error: Pointer to CmBuffer is null.");
        return CM_NULL_POINTER;
    }

    CmBufferEmu *bufferEmu = static_cast<CmBufferEmu *>(buffer);

    result = bufferEmu->CreateBufferAlias(aliasIndex);
    if (result != CM_SUCCESS)
    {
        CmAssert(0);
        CmErrorMessage("Error: Failed to create buffer alias.");
        return result;
    }

    return CM_SUCCESS;
}

CM_RT_API int32_t CmDeviceEmu::CreateQueueEx(CmQueue *& pQueue, CM_QUEUE_CREATE_OPTION createOption)
{
    int32_t result;

    //use round-robin policy to allocate job to usable device tiles
    std::vector<uint8_t> usableTiles;
    int32_t deviceTile_id = 0;
    bool isMultiTile = false;

    if( m_pQueue )
    {
        pQueue = static_cast< CmQueue* >(m_pQueue);
        return CM_SUCCESS;
    }

        isMultiTile = false;
        usableTiles.emplace_back(0);
        deviceTile_id = CM_ZERO_TILEMASK; //assign a neg num to indicate zero tile mask

    auto IsTheSameType      = [&createOption](CmQueueEmu *pQueue) { return createOption.QueueType == pQueue->Type(); };
    auto exising_queue_iter = std::find_if(m_queueArray.begin(),
        m_queueArray.end(),
        IsTheSameType);

    if (m_queueArray.end() != exising_queue_iter)  // Requested queue has been created.
    {
        pQueue = *exising_queue_iter;

        return CM_SUCCESS;
    }

    CmQueueEmu *new_queue_emu = nullptr;

    result = CmQueueEmu::Create(this, new_queue_emu);

    if (CM_SUCCESS == result)
    {
        pQueue = new_queue_emu;
        new_queue_emu->SetDeviceTileID(deviceTile_id);
        m_queueArray.push_back(new_queue_emu);
    }
    else
    {
        CmFailWithMessage("Failed to create a queue!");
    }

    return result;
}

CM_RT_API int32_t CmDeviceEmu::CreateSurface2DAlias(CmSurface2D* surface2d,
                                                    SurfaceIndex* &aliasIndex)
{
    int32_t result = CM_SUCCESS;

    if (!surface2d)
    {
        CmErrorMessage("Error: Pointer to surface 2d is null.");
        CmAssert(0);
        return CM_NULL_POINTER;
    }

    CmSurface2DEmu *surface2dEmu = static_cast<CmSurface2DEmu *>(surface2d);

    result = surface2dEmu->CreateSurface2DAlias(aliasIndex);
    if (result != CM_SUCCESS)
    {
        CmErrorMessage("Error: Failed to create surface2d alias.");
        CmAssert(0);
        return result;
    }

    return CM_SUCCESS;
}

