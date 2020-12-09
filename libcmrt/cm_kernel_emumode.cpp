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

#include <memory>

#include "cm_include.h"

#include "cm_mem.h"
#include "cm_kernel_emumode.h"
#include "cm_device_emumode.h"
#include "cm_surface_manager_emumode.h"
#include "cm_surface_2d_emumode.h"
#include "cm_thread_space_emumode.h"
#include "cm.h"

CmKernelEmu::CmKernelEmu( CmDeviceEmu *device, const void * fncPt )
{
    funcPt = fncPt;
    m_pCmDev = device;
    m_Options[0] = '\0';
    m_KernelName[0] = '\0';
    this->m_ArgCount = 0;
    this->m_MaxArgCount = 0;
    this->m_ThreadArgCount = 0;
    this->m_ThreadCount = 0;
    this->m_argSizeTotal = 0;
    this->m_AssociatedToTS = false;
    this->m_IndexInTask = 0;
    m_refcount = 0;
    m_threadSpace = nullptr;
    m_threadGroupSpace = nullptr;
}

int32_t CmKernelEmu::Create( CmDeviceEmu * device, CmProgram* pProgram, const char* kernelName, const void* funcPt, CmKernelEmu* & pKernel, const char* options )
{
    int32_t result = CM_SUCCESS;
    pKernel = new CmKernelEmu( device, funcPt );

    if( pKernel )
    {
        pKernel->Acquire();
        result = pKernel->Initialize( kernelName, options );
        if( result != CM_SUCCESS )
        {
            CmKernelEmu::Destroy( pKernel);
        }
    }
    else
    {
        CmAssert( 0 );
        result = CM_OUT_OF_HOST_MEMORY;
    }
    return result;
}

int32_t CmKernelEmu::Initialize( const char* kernelName, const char* options )
{
    if( kernelName )
    {
        size_t length = strnlen( kernelName, CM_MAX_KERNEL_NAME_SIZE_IN_BYTE );
        if( length < CM_MAX_KERNEL_NAME_SIZE_IN_BYTE  )
        {
            CmSafeMemCopy( m_KernelName, kernelName, length );
            m_KernelName[ length ] = '\0';
        }
        else
        {
            CmAssert( 0 );
            return CM_FAILURE;
        }
    }
    else
    {
        CmAssert( 0 );
        return CM_FAILURE;
    }

    if( options )
    {
        size_t length = strnlen( options, CM_MAX_OPTION_SIZE_IN_BYTE );
        if( length < CM_MAX_OPTION_SIZE_IN_BYTE  )
        {
            CmSafeMemCopy( m_Options, options, length );
            m_Options[ length ] = '\0';
        }
    }

    this->m_pCmDev->GetHalMaxValues(m_pMaxVhalVals);

    try {
        m_Args.resize (m_pMaxVhalVals->maxArgsPerKernel);
    } catch (...) {
        CmAssert( 0 );
        return CM_OUT_OF_HOST_MEMORY;
    }

    //initialize_global_surface_index();
    return CM_SUCCESS;
}

const void * CmKernelEmu::getFuncPnt()
{
    return funcPt;
}

CM_RT_API int32_t CmKernelEmu::SetKernelArg(uint32_t index, size_t size, const void * pValue )
{
    if( index >= m_pMaxVhalVals->maxArgsPerKernel )
    {
        CmAssert( 0 );
        return CM_INVALID_ARG_INDEX;
        //return CM_FAILURE;

    }else if((index + 1) > m_MaxArgCount)
    {
        m_MaxArgCount = index + 1;
    }

    //if( size == 0 )
    if( size ==0 || ((int)size < 0))
    {
        CmAssert( 0 );
        return CM_INVALID_ARG_SIZE;
    }

    if( !pValue )
    {
        CmAssert( 0 );
        return CM_INVALID_ARG_VALUE;
    }

    this->m_ArgCount++;

    if (!m_Args[index].setValueFrom (pValue, size))
    {
        CmAssert( 0 );
        return CM_KERNEL_ARG_SETTING_FAILED;
    }

    m_argSizeTotal += size;
    return CM_SUCCESS;
}

CM_RT_API int32_t CmKernelEmu::SetStaticBuffer(uint32_t index,const void * pValue )
{
    if(index >= CM_MAX_GLOBAL_SURFACE_NUMBER) {
        CmAssert( 0 );
        return CM_INVALID_GLOBAL_BUFFER_INDEX;
    }

    if(!pValue) {
        CmAssert( 0 );
        return CM_INVALID_BUFFER_HANDLER;
    }

    m_GlobalSurfaces[index] = (SurfaceIndex *)pValue;
    set_global_surface_index(index, (SurfaceIndex *)pValue);

    return CM_SUCCESS;
}

CM_RT_API int32_t CmKernelEmu::SetThreadArg(uint32_t threadId, uint32_t index, size_t size, const void * pValue )
{
    if(m_ThreadCount > this->m_pMaxVhalVals-> maxUserThreadsPerTask || (int)m_ThreadCount <=0)
    {
        CmErrorMessage("Minimum or Maximum number of threads exceeded.");
        CmAssert( 0 );
        return CM_FAILURE;
    }

    if( index >= m_pMaxVhalVals->maxArgsPerKernel )
    {
        CmAssert( 0 );
        return CM_EXCEED_KERNEL_ARG_AMOUNT;

    }else if((index + 1) > m_MaxArgCount)
    {
        m_MaxArgCount = index + 1;
    }

    if( threadId >= m_ThreadCount )
    {
        CmAssert( 0 );
        return CM_INVALID_THREAD_INDEX;

    }

    //if( size ==0 )
    if( size ==0 || ((int)size < 0))
    {
        CmAssert( 0 );
        return CM_INVALID_ARG_SIZE;
    }

    if( !pValue)
    {
        CmAssert( 0 );
        return CM_INVALID_ARG_VALUE;
    }

    this->m_ThreadArgCount++;

    if(!m_Args[index].setValueFrom (pValue, size, threadId, m_ThreadCount))
    {
        CmAssert( 0 );
        return CM_KERNEL_ARG_SETTING_FAILED;
    }

    return CM_SUCCESS;
}

int32_t CmKernelEmu::ResetArgs( void )
{
    for (auto& arg: m_Args)
        arg.reset ();

    this->m_ArgCount = 0;
    this->m_MaxArgCount = 0;
    this->m_ThreadCount = 0;
    this->m_argSizeTotal = 0;

    m_threadSpace = nullptr;
    m_threadGroupSpace = nullptr;

    return CM_SUCCESS;
}

int32_t CmKernelEmu::GetBinary(void* & pBinary, uint32_t & size )
{
    pBinary = m_pBinary;
    size = 0;// m_KernelInfo.genxBinarySize ;

    return CM_SUCCESS;
}
/*
 * Makes sure all the arguments were set
*/
bool CmKernelEmu::CheckArgs()
{
    return true;
}

std::vector<CmEmuArg>& CmKernelEmu::GetArgsVecRef( )
{
    return m_Args;
}

int32_t CmKernelEmu::GetArgs( CmEmuArg* & pArg )
{
    pArg = m_Args.data ();
    return CM_SUCCESS;
}

int32_t CmKernelEmu::GetArgCount( uint32_t & argCount )
{
    argCount = m_ArgCount;
    return CM_SUCCESS;
}

int32_t CmKernelEmu::GetThreadArgCount( uint32_t & threadArgCount )
{
    threadArgCount = m_ThreadArgCount;
    return CM_SUCCESS;
}

int32_t CmKernelEmu::GetMaxArgCount( uint32_t & maxArgCount )
{
    maxArgCount = m_MaxArgCount;
    return CM_SUCCESS;
}

CM_RT_API int32_t CmKernelEmu::SetThreadCount(uint32_t count )
{
    if ((int)count <= 0)
        return CM_INVALID_ARG_VALUE;

    if (m_threadSpace == nullptr)
    {
        if (m_ThreadCount)
        {
            // Setting threadCount twice with different values will cause reset of kernels
            if (m_ThreadCount != count)
            {
                this->ResetArgs();
                m_ThreadCount = count;

            }
        }
        else // first time
        {
            m_ThreadCount = count;
        }
    }
    return CM_SUCCESS;
}

/*CM_RT_API */int32_t CmKernelEmu::GetThreadCount(uint32_t& count )
{
    count = m_ThreadCount;
    return CM_SUCCESS;
}

int32_t CmKernelEmu::Destroy( CmKernelEmu* &pKernel )
{
    int refCount = pKernel->SafeRelease();
    if (refCount == 0)
    {
        pKernel = nullptr;
    }
    return CM_SUCCESS;
}

CM_RT_API int32_t CmKernelEmu::SetThreadDependencyMask( uint32_t threadId, uint8_t mask )
{
    return CmNotImplemented(__PRETTY_FUNCTION__);
}

void CmKernelEmu::AddScoreBoardCoord(uint32_t x, uint32_t y, uint32_t threadID)
{
    SIM_SCB_COORD * temp = new SIM_SCB_COORD();
    temp->x = x;
    temp->y = y;
    m_scoreboard_coord[threadID] = temp;
}

bool CmKernelEmu::GetSCBCoord(uint32_t threadID, uint32_t &x, uint32_t &y)
{
    SIM_SCB_COORD *temp = nullptr;

    temp = m_scoreboard_coord[threadID];

    if (temp == nullptr)
    {
        return false;
    }

    x = temp->x;
    y = temp->y;

    return true;
}

int32_t CmKernelEmu::SetIndexInTask(uint32_t index)
{
    m_IndexInTask = index;
    return CM_SUCCESS;
}

uint32_t CmKernelEmu::GetIndexInTask(void)
{
    return m_IndexInTask;
}

int32_t CmKernelEmu::SetAssociatedToTSFlag(bool b)
{
    m_AssociatedToTS = b;
    return CM_SUCCESS;
}

int32_t CmKernelEmu::SetSurfaceBTI(SurfaceIndex* pSurfaceIndex, uint32_t BTIndex)
{
    int status = CM_FAILURE;

    if(pSurfaceIndex == nullptr)
    {
        CmAssert(0);
        return CM_FAILURE;
    }

    //check the dst BTIndex is valid or not
    if(!m_pCmDev->IsValidSurfaceIndex(BTIndex))
    {
        CmAssert(0);
        return CM_KERNELPAYLOAD_SURFACE_INVALID_BTINDEX;
    }

    //avoid creating duplicate surfaces
    if (pSurfaceIndex->get_data() == BTIndex)
    {
        return CM_SUCCESS;
    }

    //get the surface manager
    CmSurfaceManagerEmu* m_pSurfaceMg = nullptr;
    m_pCmDev->GetSurfaceManagerEmu( m_pSurfaceMg );
    if(m_pSurfaceMg == nullptr)
    {
        CmAssert(0);
        return CM_FAILURE;
    }

    //get src surface , if it is invalid, return false
    CmSurfaceEmu* pSrcSurfaceEmu = nullptr;
    m_pSurfaceMg->GetSurface( pSurfaceIndex->get_data(), pSrcSurfaceEmu );
    if( pSrcSurfaceEmu == nullptr ){
        return CM_FAILURE;
    }
    //for some special surface such as NV12
    int leftpos=0;
    int rightpos=0;
    m_pSurfaceMg->FindSurfaceArea(pSurfaceIndex->get_data(),leftpos,rightpos);

    //check the dst BTIndex is available or not
    bool flag_available = true;
    CmSurfaceEmu* pDstSurfaceEmu = nullptr;
    for(int i=leftpos; i<= rightpos; i++)
    {
        m_pSurfaceMg->GetSurface( BTIndex+i, pDstSurfaceEmu );
        if(pDstSurfaceEmu != nullptr)
        {
            flag_available = false;
            break;
        }
    }

    // BTIndex is not available
    if( !flag_available )
    {
        uint32_t dst_index = 0;
        status = m_pSurfaceMg->findFreeIndex(rightpos-leftpos, dst_index);
        if( status != CM_SUCCESS)
        {
            return CM_NO_AVAILABLE_SURFACE;
        }

        status = m_pSurfaceMg->MoveSurface(BTIndex, dst_index);
        if( status != CM_SUCCESS)
        {
            return status;
        }
    }

    // move surface to BTIndex
    status = m_pSurfaceMg->MoveSurface(pSurfaceIndex->get_data(), BTIndex);
    if( status != CM_SUCCESS)
    {
        return status;
    }

    return CM_SUCCESS;
}

int32_t CmKernelEmu::QuerySpillSize(unsigned int &spillSize)
{
    /* EMU mode doesn't use Jitter */
    return CM_FAILURE;
}

//*-----------------------------------------------------------------------------
//| Purpose:    Acuqire Kernel: increment refcount
//| Returns:    Result of the operation.
//*-----------------------------------------------------------------------------
int CmKernelEmu::Acquire()
{
    m_refcount++;
    return m_refcount;
}

//*-----------------------------------------------------------------------------
//| Purpose:    SafeRelease Kernel: Delete the instance
//| Returns:    Result of the operation.
//*-----------------------------------------------------------------------------
int CmKernelEmu::SafeRelease()
{
    --m_refcount;
    if (m_refcount == 0)
    {
        delete this;
        return 0;
    }
    return m_refcount;
}

int32_t CmKernelEmu::AssociateThreadSpace_preG12(CmThreadSpace* & threadSpace)
{
    if (threadSpace == nullptr)
    {
        CmAssert(0);
        return CM_NULL_POINTER;
    }

    if (m_threadGroupSpace != nullptr)
    {
        CmAssert(0);
        return CM_INVALID_KERNEL_THREADSPACE;
    }

    m_threadSpace = dynamic_cast<CmThreadSpaceEmu*>(threadSpace);

    uint32_t threadSpaceWidth = 0;
    uint32_t threadSpaceHeight = 0;
    m_threadSpace->GetThreadSpaceSize(threadSpaceWidth, threadSpaceHeight);
    m_ThreadCount = threadSpaceWidth * threadSpaceHeight;

    return CM_SUCCESS;
}

CM_RT_API int32_t CmKernelEmu::AssociateThreadSpace(CmThreadSpace *&threadSpace)
{
    int32_t ret;

    if (
        CmDeviceEmu::CurrentPlatform == CmEmuPlatformUse::TGLLP
    )
    {
        CmThreadSpaceEmu * threadSpace_h = dynamic_cast<CmThreadSpaceEmu *>(threadSpace);
        CmThreadGroupSpace *thread_group_space_h = threadSpace_h ?
                threadSpace_h->GetThreadGroupSpace() : nullptr;
        ret = AssociateThreadGroupSpace(thread_group_space_h);
    }
    else
    {
        ret = AssociateThreadSpace_preG12(threadSpace);
    }

    return ret;
}

CM_RT_API int32_t CmKernelEmu::AssociateThreadGroupSpace(CmThreadGroupSpace* & threadGroupSpace)
{
    if (threadGroupSpace == nullptr)
    {
        CmAssert(0);
        return CM_NULL_POINTER;
    }

    if (m_threadSpace != nullptr)
    {
        CmAssert(0);
        return CM_INVALID_KERNEL_THREADGROUPSPACE;
    }

    m_threadGroupSpace = threadGroupSpace;

    return CM_SUCCESS;
}

int32_t CmKernelEmu::DeAssociateThreadSpace_preG12(CmThreadSpace* & threadSpace)
{
    if (threadSpace == nullptr)
    {
        CmAssert(0);
        return CM_NULL_POINTER;
    }
    if (m_threadSpace != dynamic_cast<CmThreadSpaceEmu *>(threadSpace))
    {
        CmAssert(0);
        return CM_INVALID_ARG_VALUE;
    }

    m_threadSpace = nullptr;

    return CM_SUCCESS;
}

CM_RT_API int32_t CmKernelEmu::DeAssociateThreadSpace(CmThreadSpace *&threadSpace)
{
    int32_t ret;

    if (
        CmDeviceEmu::CurrentPlatform == CmEmuPlatformUse::TGLLP
    )
    {
        CmThreadSpaceEmu * threadSpace_h = dynamic_cast<CmThreadSpaceEmu *>(threadSpace);
        CmThreadGroupSpace *thread_group_space_h = threadSpace_h ?
                threadSpace_h->GetThreadGroupSpace() : nullptr;
        ret = DeAssociateThreadGroupSpace(thread_group_space_h);
    }
    else
    {
        ret = DeAssociateThreadSpace_preG12(threadSpace);
    }

    return ret;
}

CM_RT_API int32_t CmKernelEmu::DeAssociateThreadGroupSpace(CmThreadGroupSpace* & threadGroupSpace)
{
    if (threadGroupSpace == nullptr)
    {
        CmAssert(0);
        return CM_NULL_POINTER;
    }

    if (m_threadGroupSpace != threadGroupSpace)
    {
        CmFailWithMessage("Trying to deassociate "
            "thread group space not equal to the one associated with this kernel.");
        return CM_INVALID_ARG_VALUE;
    }

    m_threadGroupSpace = nullptr;

    return CM_SUCCESS;
}

int32_t CmKernelEmu::GetThreadSpace(CmThreadSpaceEmu* &threadSpace)
{
    threadSpace = m_threadSpace;
    return CM_SUCCESS;
}

int32_t CmKernelEmu::GetThreadGroupSpace(CmThreadGroupSpace* &threadGroupSpace)
{
    threadGroupSpace = m_threadGroupSpace;
    return CM_SUCCESS;
}

CM_RT_API int32_t CmKernelEmu::SetKernelArgPointer(uint32_t index,
                                                   size_t size,
                                                   const void * pValue)
{
    if( !pValue )
    {
        CmAssert( 0 );
        return CM_INVALID_ARG_VALUE;
    }

    CmSurfaceManagerEmu* surfMgr;
    uint64_t gfxAddress;
    bool isValid = false;

    /* # */
    void *pTemp = new uint8_t[sizeof(uint64_t)];
    CmSafeMemCopy(pTemp, pValue, size);
    gfxAddress = *(reinterpret_cast<uint64_t *>(pTemp));

    CmAssert( gfxAddress != 0 );

    if (!isValid)
    {
        CmErrorMessage("Error: the kernel arg pointer is not valid.");
        CmAssert(0);
        return CM_INVALID_KERNEL_ARG_POINTER;
    }

    return SetKernelArg(index, size, &gfxAddress);
}

