/*========================== begin_copyright_notice ============================

Copyright (C) 2017 Intel Corporation

SPDX-License-Identifier: MIT

============================= end_copyright_notice ===========================*/

#include <cmath>

#include "cm_include.h"
#include "cm_thread_space_emumode.h"
#include "cm_group_space_emumode.h"
#include "cm_task_emumode.h"
#include "cm_kernel_emumode.h"
#include "cm_mem.h"
#include "cm_device_emumode.h"

#include "emu_cfg.h"

int32_t CmThreadSpaceEmu::Create( CmDeviceEmu* pDevice, uint32_t width, uint32_t height, CmThreadSpaceEmu* & pTS )
{
    if( ( width == 0 ) || ( height == 0 ) )
    {
        GFX_EMU_ERROR_MESSAGE("Thread space width and height must be non-zero!");
        GFX_EMU_ASSERT( 0 );
        return CM_FAILURE;
    }

    int32_t result = CM_SUCCESS;
    pTS = new CmThreadSpaceEmu( pDevice, width, height );
    if( pTS )
    {
        result = pTS->Initialize( );
        if( result != CM_SUCCESS )
        {
            CmThreadSpaceEmu::Destroy( pTS);
        }
    }
    else
    {
        GFX_EMU_ASSERT( 0 );
        result = CM_OUT_OF_HOST_MEMORY;
    }
    return result;

}

int32_t CmThreadSpaceEmu::Destroy( CmThreadSpaceEmu* &pTS )
{
    CmSafeRelease( pTS );

    return CM_SUCCESS;
}

CmThreadSpaceEmu::CmThreadSpaceEmu( CmDeviceEmu* pDevice ,uint32_t width, uint32_t height ):
    m_pDevice( pDevice ),
    m_Width( width ),
    m_Height( height ),
    m_ColorCount( 1 ),
    m_26ZIBlockWidth ( CM_26ZI_BLOCK_WIDTH ),
    m_26ZIBlockHeight ( CM_26ZI_BLOCK_HEIGHT ),
    m_DependencyPatternType(CM_NONE_DEPENDENCY),
    m_CurrentDependencyPattern(CM_NONE_DEPENDENCY),
    m_WalkingPattern(CM_WALK_DEFAULT),
    m_26ZIDispatchPattern(VVERTICAL_HVERTICAL_26),
    m_Current26ZIDispatchPattern(VVERTICAL_HVERTICAL_26),
    m_pBoardFlag( nullptr ),
    m_pBoardOrderList( nullptr ),
    m_threadGroupSpace_h( nullptr )
{

}

CmThreadSpaceEmu::~CmThreadSpaceEmu( void )
{
    CmSafeDeleteArray(m_pThreadSpaceUnit);
    CmSafeDeleteArray(m_pBoardFlag);
    CmSafeDeleteArray(m_pBoardOrderList);
    if (m_threadGroupSpace_h)
    {
        CmThreadGroupSpace::Destroy(m_threadGroupSpace_h);
    }
}

int32_t CmThreadSpaceEmu::Initialize( void )
{
    m_Mask = 0;

    CmSafeMemSet( &m_Dependency, 0, sizeof(CM_DEPENDENCY) );
    m_ThreadAssociated = false;

    m_Dependency.count = 0;
    m_pThreadSpaceUnit = new CM_THREAD_SPACE_UNIT[m_Height * m_Width];
    if (m_pThreadSpaceUnit)
    {
        CmSafeMemSet(m_pThreadSpaceUnit, 0, sizeof(CM_THREAD_SPACE_UNIT) * m_Height * m_Width);
        return CM_SUCCESS;
    }
    else
    {
        GFX_EMU_ASSERT( 0 );
        return CM_OUT_OF_HOST_MEMORY;
    }

    return CM_SUCCESS;
}

//!
//! Associate a thread to one uint in the 2-dimensional dependency board.
//! If call this function twice with same x/y pair and different thread, the 2nd one will fail
//! Enqueue will make sure each x/y pair in the CmThreadSpace_RT object is associated with
//! a unique thread in the task to enqueue.Otherwise enqueue will fail.
//! Input :
//!     1) X/Y coordinats of the uint in dependency board
//!     2) pointer to CmKernel_RT
//!     3) thread index. It is the same as the read index in
//!     CmKernel_RT::SetThreadArg(uint32_t threadId, uint32_t index, size_t size, const void * pValue )
//! OUTPUT :
//!     CM_SUCCESS if the association is successful
//!
CM_RT_API int32_t CmThreadSpaceEmu::AssociateThread( uint32_t x, uint32_t y, CmKernel* pKernel , uint32_t threadId )
{
    return AssociateThreadInternal(x, y, pKernel, threadId, 0xFF);
}

CM_RT_API int32_t CmThreadSpaceEmu::AssociateThreadWithMask( uint32_t x, uint32_t y, CmKernel* pKernel, uint32_t threadId, uint8_t dependencyMask)
{
    return AssociateThreadInternal(x, y, pKernel, threadId, dependencyMask);
}

CM_RT_API int32_t CmThreadSpaceEmu::SetThreadSpaceColorCount( uint32_t colorCount )
{
    if (colorCount == CM_INVALID_COLOR_COUNT ||
        (GfxEmu::Cfg::Platform ().getInt () <= GfxEmu::Platform::KBL
        )  ?
            (colorCount > CM_THREADSPACE_MAX_COLOR_COUNT) :
            (colorCount > CM_THREADSPACE_MAX_COLOR_COUNT_GEN11_PLUS))
    {
        GFX_EMU_ASSERT(0);
        return CM_INVALID_ARG_VALUE;
    }

    m_ColorCount = colorCount;

    return CM_SUCCESS;
}

CM_RT_API int32_t CmThreadSpaceEmu::SelectMediaWalkingPattern( CM_WALKING_PATTERN pattern )
{
    int result = CM_SUCCESS;

    if( m_DependencyPatternType != CM_NONE_DEPENDENCY )
    {
        GFX_EMU_ASSERT( 0 );
        return CM_INVALID_DEPENDENCY_WITH_WALKING_PATTERN;
    }

    switch( pattern )
    {
        case CM_WALK_DEFAULT:
            m_WalkingPattern = CM_WALK_DEFAULT;
            break;
        case CM_WALK_HORIZONTAL:
            m_WalkingPattern = CM_WALK_HORIZONTAL;
            break;
        case CM_WALK_VERTICAL:
            m_WalkingPattern = CM_WALK_VERTICAL;
            break;
        case CM_WALK_WAVEFRONT:
            m_WalkingPattern = CM_WALK_WAVEFRONT;
            break;
        case CM_WALK_WAVEFRONT26:
            m_WalkingPattern = CM_WALK_WAVEFRONT26;
            break;
        case CM_WALK_WAVEFRONT26X:
            m_WalkingPattern = CM_WALK_WAVEFRONT26X;
            break;
        case CM_WALK_WAVEFRONT26ZIG:
            m_WalkingPattern = CM_WALK_WAVEFRONT26ZIG;
            break;
        case CM_WALK_WAVEFRONT45D:
            m_WalkingPattern = CM_WALK_WAVEFRONT45D;
            break;
        case CM_WALK_WAVEFRONT45XD_2:
            m_WalkingPattern = CM_WALK_WAVEFRONT45XD_2;
            break;
        default:
            GFX_EMU_ASSERT( 0 );
            result = CM_INVALID_MEDIA_WALKING_PATTERN;
            break;
    }

    return result;
}

CM_RT_API int32_t CmThreadSpaceEmu::Set26ZIDispatchPattern( CM_26ZI_DISPATCH_PATTERN pattern )
{
    int result = CM_SUCCESS;

     switch( pattern )
     {
     case VVERTICAL_HVERTICAL_26:
         m_26ZIDispatchPattern = VVERTICAL_HVERTICAL_26;
         break;
     case VVERTICAL_HHORIZONTAL_26:
         m_26ZIDispatchPattern = VVERTICAL_HHORIZONTAL_26;
         break;
     case VVERTICAL26_HHORIZONTAL26:
         m_26ZIDispatchPattern = VVERTICAL26_HHORIZONTAL26;
         break;
     case VVERTICAL1X26_HHORIZONTAL1X26:
         m_26ZIDispatchPattern = VVERTICAL1X26_HHORIZONTAL1X26;
         break;
      default:
        result = CM_FAILURE;
        break;
     }

     return result;
}

CM_RT_API int32_t CmThreadSpaceEmu::Set26ZIMacroBlockSize( uint32_t width, uint32_t height )
{
    m_26ZIBlockWidth = width;
    m_26ZIBlockHeight = height;

    return CM_SUCCESS;
}

int32_t CmThreadSpaceEmu::AssociateThreadInternal( uint32_t x, uint32_t y, CmKernel* pKernel, uint32_t threadId, uint8_t dependencyMask )
{
    if(x >= m_Width || y >= m_Height ||
        pKernel == nullptr)
    {
        GFX_EMU_ASSERT( 0 );
        return CM_INVALID_ARG_VALUE;
    }
    uint32_t linear_offset = y*m_Width + x;
    m_pThreadSpaceUnit[linear_offset].pKernel = pKernel;
    m_pThreadSpaceUnit[linear_offset].threadId = threadId;
    m_pThreadSpaceUnit[linear_offset].numEdges = 0;
    m_ThreadAssociated = true;
    CmKernelEmu *temp = dynamic_cast<CmKernelEmu *>(pKernel);
    if(temp==NULL)
    {
        GFX_EMU_ASSERT(0);
        return CM_FAILURE;
    }
    CmKernelEmu*pKernel_RT = dynamic_cast<CmKernelEmu *> (pKernel);
    pKernel_RT->SetAssociatedToTSFlag(true);

    temp->AddScoreBoardCoord(x,y,threadId);

    m_pThreadSpaceUnit[linear_offset].dependencyMask = dependencyMask;
    m_pThreadSpaceUnit[linear_offset].scoreboardCoordinates.x = x;
    m_pThreadSpaceUnit[linear_offset].scoreboardCoordinates.y = y;

    return CM_SUCCESS;

}

CmThreadGroupSpace *CmThreadSpaceEmu::GetThreadGroupSpace()
{
    if (!m_threadGroupSpace_h)
    {
        CmThreadGroupSpace::Create(m_pDevice, 1, 1, m_Width, m_Height, m_threadGroupSpace_h);
    }
    return m_threadGroupSpace_h;
}

//!
//! Set the dependency pattern. There can be at most 8 dependent unit in the pattern.
//! Each dependent unit is indicated as the delta in X coordinat and the delta in Y coordinat
//! The call will fail if there is a pair of deltaX/Y with value ( 0, 0 )
//! By default, there is no dependent unit, i.e. count is 0.
//! Input :
//!     1) Total number of dependent units. It is <= 8.
//!     2) Array of deltaX. Array size is the first argument.
//!        Each deltaX is in the range of [-8, 7]
//!     3) Array of deltaY. Array size is the first argument.
//!        Each deltaY is in the range of [-8, 7]
//! OUTPUT :
//!     CM_SUCCESS if the pattern is set
//!
CM_RT_API int32_t CmThreadSpaceEmu::SetThreadDependencyPattern( uint32_t count, int32_t *deltaX, int32_t *deltaY )
{
    if( count > CM_MAX_DEPENDENCY_COUNT )
    {
        GFX_EMU_ERROR_MESSAGE("Exceed dependency count limitation, which is 8!");
        GFX_EMU_ASSERT( 0 );
        return CM_FAILURE;

    }

    m_Dependency.count = count;

    CmSafeMemCopy( m_Dependency.deltaX, deltaX, sizeof( int32_t ) * count );
    CmSafeMemCopy( m_Dependency.deltaY, deltaY, sizeof( int32_t ) * count );

    return CM_SUCCESS;
}

//!
//! Select from X predefined dependency patterns.
//! Input :
//!     1) pattern index
//! OUTPUT :
//!     CM_SUCCESS if the pattern is selected
//!

CM_RT_API int32_t CmThreadSpaceEmu::SelectThreadDependencyPattern ( CM_DEPENDENCY_PATTERN pattern )
{
    int result = CM_SUCCESS;

    if( m_pBoardFlag == nullptr )
    {
        m_pBoardFlag = new uint32_t[m_Height * m_Width];
        if ( m_pBoardFlag )
        {
            CmSafeMemSet(m_pBoardFlag, 0, sizeof(uint32_t) * m_Height * m_Width);
        }
        else
        {
            GFX_EMU_ASSERT( 0 );
            return CM_OUT_OF_HOST_MEMORY;
        }
    }

    if( m_pBoardOrderList == nullptr )
    {
        m_pBoardOrderList = new uint32_t[m_Height * m_Width];
        if( m_pBoardOrderList )
        {
            CmSafeMemSet(m_pBoardOrderList, 0, sizeof(uint32_t) * m_Height * m_Width);
        }
        else
        {
            GFX_EMU_ASSERT( 0 );
            if( m_pBoardFlag )
                CmSafeDeleteArray(m_pBoardFlag);
            return CM_OUT_OF_HOST_MEMORY;
        }
    }

    if( ( pattern != CM_NONE_DEPENDENCY) && (m_WalkingPattern != CM_WALK_DEFAULT) )
    {
        GFX_EMU_ASSERT( 0 );
        return CM_INVALID_DEPENDENCY_WITH_WALKING_PATTERN;
    }

    switch(pattern)
    {
        case CM_VERTICAL_WAVE:
            {
                int32_t DeltaX[ 1 ] = {-1};
                int32_t DeltaY[ 1 ] = {0};
                m_DependencyPatternType = CM_VERTICAL_WAVE;
                result = this->SetThreadDependencyPattern(1,DeltaX,DeltaY);
                break;
            }
        case CM_HORIZONTAL_WAVE:
            {
                int32_t DeltaX[ 1 ] = {0};
                int32_t DeltaY[ 1 ] = {-1};
                m_DependencyPatternType = CM_HORIZONTAL_WAVE;
                result = this->SetThreadDependencyPattern(1,DeltaX,DeltaY);
                break;
            }
        case CM_WAVEFRONT:
            {
                int32_t DeltaX[ 2 ] = {-1, 0};
                int32_t DeltaY[ 2 ] = {0, -1};
                m_DependencyPatternType = CM_WAVEFRONT;
                result = this->SetThreadDependencyPattern(2,DeltaX,DeltaY);
                break;
            }
        case CM_WAVEFRONT26:
            {
                int32_t DeltaX[ 3 ] = {-1, 0, 1};
                int32_t DeltaY[ 3 ] = {0, -1, -1};
                m_DependencyPatternType = CM_WAVEFRONT26;
                result = this->SetThreadDependencyPattern(3,DeltaX,DeltaY);
                break;
            }
        case CM_WAVEFRONT26Z:
            {
                int32_t DeltaX[ 5 ] = {-1, -1, -1, 0, 1};
                int32_t DeltaY[ 5 ] = {1, 0, -1, -1, -1};
                m_DependencyPatternType = CM_WAVEFRONT26Z;
                result = this->SetThreadDependencyPattern(5,DeltaX,DeltaY);
                break;
            }
        case CM_WAVEFRONT26ZI:
            {
                int32_t DeltaX[ 7 ] = {-1, -2, -1, -1, 0, 1, 1};
                int32_t DeltaY[ 7 ] = {1, 0, 0, -1, -1, -1, 0};
                m_DependencyPatternType = CM_WAVEFRONT26ZI;
                result = this->SetThreadDependencyPattern(7, DeltaX, DeltaY);
                break;
            }
        case CM_WAVEFRONT26ZIG:
            {
                int32_t DeltaX[5] = { -1, -1, -1, 0, 1 };
                int32_t DeltaY[5] = { 1, 0, -1, -1, -1 };
                m_DependencyPatternType = CM_WAVEFRONT26ZIG;
                result = this->SetThreadDependencyPattern(5, DeltaX, DeltaY);
                break;
            }

        case CM_WAVEFRONT26X:
            {
                int32_t DeltaX[7] = { -1, -1, -1, 0, 0, 0, 1 };
                int32_t DeltaY[7] = { 3, 1, -1, -1, -2, -3, -3 };
                m_DependencyPatternType = CM_WAVEFRONT26X;
                result = this->SetThreadDependencyPattern(7, DeltaX, DeltaY);
                break;
            }
        case CM_NONE_DEPENDENCY:
            {
                m_DependencyPatternType = CM_NONE_DEPENDENCY;
                result = CM_SUCCESS;
                break;
            }

        default:
            {
                result = CM_FAILURE;
                break;
            }
    }
    return result;

}

int32_t CmThreadSpaceEmu::GetDependencyPatternType(CM_DEPENDENCY_PATTERN &DependencyPatternType)
{
    DependencyPatternType = m_DependencyPatternType;

    return CM_SUCCESS;
}

int32_t CmThreadSpaceEmu::Get26ZIDispatchPattern( CM_26ZI_DISPATCH_PATTERN &pattern)
{
    pattern = m_26ZIDispatchPattern;

    return CM_SUCCESS;
}

bool CmThreadSpaceEmu::IntegrityCheck(CmKernelArrayEmu* pTask)
{
    CmKernelEmu *pKernel_RT = nullptr;
    uint32_t i;
    uint32_t KernelCount = 0;
    uint32_t ThreadNumber = 0;
    uint32_t KernelIndex = 0;
    uint32_t unassociated = 0;
    uint32_t threadArgCount = 0;
    CM_DEPENDENCY_PATTERN dependencyPattern = CM_NONE_DEPENDENCY;

    KernelCount = pTask->GetKernelCount();
    //Check if it is mult-kernel task, since no threadspace is allowed for multi-kernel tasks
    pKernel_RT = dynamic_cast<CmKernelEmu *> (pTask->GetKernelPointer(0));

    pKernel_RT->GetThreadArgCount(threadArgCount);

    //Till now, all disallowed settings are abort, now we need check if the thread space association is correct.
    if (this->IsThreadAssociated())
    {
        //For future extending to multiple kernels cases, we're using a general mechanism to check the integrity
    #if defined(_WIN32)
        bool** pTSMapping = new bool*[KernelCount];
    #else
        bool** pTSMapping = new bool*[KernelCount];
    #endif
        bool* pKernelInScoreboard = new bool[KernelCount];
        CmSafeMemSet(pTSMapping, 0, KernelCount * sizeof(bool*));
        for (i = 0; i < KernelCount; i++)
        {
            pKernel_RT = dynamic_cast<CmKernelEmu *> (pTask->GetKernelPointer(i));
            pKernel_RT->GetThreadCount(ThreadNumber);
            if (ThreadNumber == 0)
            {
                ThreadNumber = m_Width*m_Height;
            }
            pTSMapping[i] = new bool[ThreadNumber];
            CmSafeMemSet(pTSMapping[i], false, ThreadNumber * sizeof(bool));
            pKernelInScoreboard[i] = false;
        }

        for (i = 0; i < m_Width * m_Height; i++)
        {
            pKernel_RT = static_cast<CmKernelEmu *> (m_pThreadSpaceUnit[i].pKernel);
            KernelIndex = pKernel_RT->GetIndexInTask();
            pTSMapping[KernelIndex][m_pThreadSpaceUnit[i].threadId] = true;
            pKernelInScoreboard[KernelIndex] = true;
        }

        for (i = 0; i < KernelCount; i ++)
        {
            if(pKernelInScoreboard[i] == true) {
                pKernel_RT = dynamic_cast<CmKernelEmu *> (pTask->GetKernelPointer(i));
                pKernel_RT->GetThreadCount(ThreadNumber);
                if (ThreadNumber == 0)
                {
                    ThreadNumber = m_Width*m_Height;
                }
                pKernel_RT->SetAssociatedToTSFlag(true);
                for (uint32_t j = 0; j < ThreadNumber; j++)
                {
                    if (pTSMapping[i][j] == false)
                    {
                        unassociated ++;
                        break;
                    }
                }
            }
            CmSafeDeleteArray(pTSMapping[i]);
        }

        CmSafeDeleteArray(pTSMapping);
        CmSafeDeleteArray(pKernelInScoreboard);

        if (unassociated != 0)
        {
            //GFX_EMU_ASSERT(0);
            GFX_EMU_MESSAGE(("pTS->IntegrityCheck Failed: ThreadSpace association is not correct!\n"));
            return false;
        }
        else
        {
            return true;
        }
    }

    //For no association case, just return true
    return true;
}

//*--------------------------------------------------------------------------
//| Purpose: Generate Wave26Z Sequence
//*--------------------------------------------------------------------------
int32_t CmThreadSpaceEmu::Wavefront26ZSequence()
{
    if ( m_CurrentDependencyPattern == CM_WAVEFRONT26Z )
    {
        return CM_SUCCESS;
    }
    m_CurrentDependencyPattern = CM_WAVEFRONT26Z;

    if ( ( m_Height % 2 != 0 ) || ( m_Width % 2 != 0 ) )
    {
        return CM_INVALID_ARG_SIZE;
    }
    CmSafeMemSet( m_pBoardFlag, WHITE, m_Width * m_Height * sizeof( uint32_t ) );
    m_IndexInList = 0;

    uint32_t iX, iY, nOffset;
    iX = iY = nOffset = 0;

    uint32_t *pWaveFrontPos = new uint32_t[ m_Width ];
    uint32_t *pWaveFrontOffset = new uint32_t[ m_Width ];
    if ( ( pWaveFrontPos == nullptr ) || ( pWaveFrontOffset == nullptr ) )
    {
        CmSafeDeleteArray( pWaveFrontPos );
        CmSafeDeleteArray( pWaveFrontOffset );
        return CM_FAILURE;
    }
    CmSafeMemSet( pWaveFrontPos, 0, m_Width * sizeof( int ) );

    // set initial value
    m_pBoardFlag[ 0 ] = BLACK;
    m_pBoardOrderList[ 0 ] = 0;
    pWaveFrontPos[ 0 ] = 1;
    m_IndexInList = 0;

    CM_COORDINATE pMask[ 8 ];
    uint32_t nMaskNumber = 0;

    while ( m_IndexInList < m_Width * m_Height - 1 )
    {
        CmSafeMemSet( pWaveFrontOffset, 0, m_Width * sizeof( int ) );
        for ( uint32_t iX = 0; iX < m_Width; ++iX )
        {
            uint32_t iY = pWaveFrontPos[ iX ];
            nOffset = iY * m_Width + iX;
            CmSafeMemSet( pMask, 0, sizeof( pMask ) );

            if ( m_pBoardFlag[ nOffset ] == WHITE )
            {
                if ( ( iX % 2 == 0 ) && ( iY % 2 == 0 ) )
                {
                    if ( iX == 0 )
                    {
                        pMask[ 0 ].x = 0;
                        pMask[ 0 ].y = -1;
                        pMask[ 1 ].x = 1;
                        pMask[ 1 ].y = -1;
                        nMaskNumber = 2;
                    }
                    else if ( iY == 0 )
                    {
                        pMask[ 0 ].x = -1;
                        pMask[ 0 ].y = 1;
                        pMask[ 1 ].x = -1;
                        pMask[ 1 ].y = 0;
                        nMaskNumber = 2;
                    }
                    else
                    {
                        pMask[ 0 ].x = -1;
                        pMask[ 0 ].y = 1;
                        pMask[ 1 ].x = -1;
                        pMask[ 1 ].y = 0;
                        pMask[ 2 ].x = 0;
                        pMask[ 2 ].y = -1;
                        pMask[ 3 ].x = 1;
                        pMask[ 3 ].y = -1;
                        nMaskNumber = 4;
                    }
                }
                else if ( ( iX % 2 == 0 ) && ( iY % 2 == 1 ) )
                {
                    if ( iX == 0 )
                    {
                        pMask[ 0 ].x = 0;
                        pMask[ 0 ].y = -1;
                        pMask[ 1 ].x = 1;
                        pMask[ 1 ].y = -1;
                        nMaskNumber = 2;
                    }
                    else
                    {
                        pMask[ 0 ].x = -1;
                        pMask[ 0 ].y = 0;
                        pMask[ 1 ].x = 0;
                        pMask[ 1 ].y = -1;
                        pMask[ 2 ].x = 1;
                        pMask[ 2 ].y = -1;
                        nMaskNumber = 3;
                    }
                }
                else if ( ( iX % 2 == 1 ) && ( iY % 2 == 0 ) )
                {
                    if ( iY == 0 )
                    {
                        pMask[ 0 ].x = -1;
                        pMask[ 0 ].y = 0;
                        nMaskNumber = 1;
                    }
                    else if ( iX == m_Width - 1 )
                    {
                        pMask[ 0 ].x = -1;
                        pMask[ 0 ].y = 0;
                        pMask[ 1 ].x = 0;
                        pMask[ 1 ].y = -1;
                        nMaskNumber = 2;
                    }
                    else
                    {
                        pMask[ 0 ].x = -1;
                        pMask[ 0 ].y = 0;
                        pMask[ 1 ].x = 0;
                        pMask[ 1 ].y = -1;
                        pMask[ 2 ].x = 1;
                        pMask[ 2 ].y = -1;
                        nMaskNumber = 3;
                    }
                }
                else
                {
                    pMask[ 0 ].x = -1;
                    pMask[ 0 ].y = 0;
                    pMask[ 1 ].x = 0;
                    pMask[ 1 ].y = -1;
                    nMaskNumber = 2;
                }

                // check if all of the dependencies are in the dispatch queue
                bool bAllInQueue = true;
                for ( uint32_t i = 0; i < nMaskNumber; ++i )
                {
                    if ( m_pBoardFlag[ nOffset + pMask[ i ].x + pMask[ i ].y * m_Width ] == WHITE )
                    {
                        bAllInQueue = false;
                        break;
                    }
                }
                if ( bAllInQueue )
                {
                    pWaveFrontOffset[ iX ] = nOffset;
                    if( pWaveFrontPos[ iX ] < m_Height - 1 )
                    {
                        pWaveFrontPos[ iX ]++;
                    }
                }
            }
        }

        for ( uint32_t iX = 0; iX < m_Width; ++iX )
        {
            if ( ( m_pBoardFlag[ pWaveFrontOffset[ iX ] ] == WHITE ) && ( pWaveFrontOffset[ iX ] != 0 ) )
            {
                m_IndexInList++;
                m_pBoardOrderList[ m_IndexInList ] = pWaveFrontOffset[ iX ];
                m_pBoardFlag[ pWaveFrontOffset[ iX ] ] = BLACK;
            }
        }

    }

    CmSafeDeleteArray( pWaveFrontPos );
    CmSafeDeleteArray( pWaveFrontOffset );

    return CM_SUCCESS;

}

int32_t CmThreadSpaceEmu::Wavefront26ZISeqVVHV26()
{
    if ( m_CurrentDependencyPattern == CM_WAVEFRONT26ZI &&
        (m_Current26ZIDispatchPattern == VVERTICAL_HVERTICAL_26))
    {
        return CM_SUCCESS;
    }

    m_CurrentDependencyPattern = CM_WAVEFRONT26ZI;
    m_Current26ZIDispatchPattern = VVERTICAL_HVERTICAL_26;

    CmSafeMemSet(m_pBoardFlag, WHITE, m_Width*m_Height*sizeof(uint32_t));
    m_IndexInList = 0;

    for( uint32_t y = 0; y < m_Height; y = y + m_26ZIBlockHeight )
    {
        for( uint32_t x = 0; x < m_Width; x = x + m_26ZIBlockWidth )
        {
            CM_COORDINATE temp_xyFor26;
            temp_xyFor26.x = x;
            temp_xyFor26.y = y;

            do
            {
                if( m_pBoardFlag[temp_xyFor26.y * m_Width + temp_xyFor26.x] == WHITE )
                {
                    m_pBoardOrderList[m_IndexInList ++] = temp_xyFor26.y * m_Width + temp_xyFor26.x;
                    m_pBoardFlag[temp_xyFor26.y * m_Width + temp_xyFor26.x] = BLACK;

                    // do vertical edges
                    for( uint32_t widthCount = 0; widthCount < m_26ZIBlockWidth; widthCount = widthCount + 2 )
                    {
                        CM_COORDINATE temp_xy;
                        uint32_t localHeightCounter = 0;

                        temp_xy.x = temp_xyFor26.x + widthCount;
                        temp_xy.y = temp_xyFor26.y;
                        while( (temp_xy.x >= 0) && (temp_xy.y >=0) &&
                            (temp_xy.x < (int32_t)m_Width) && (temp_xy.y < (int32_t)m_Height) &&
                            (localHeightCounter < m_26ZIBlockHeight))
                        {
                            if( m_pBoardFlag[temp_xy.y * m_Width + temp_xy.x] == WHITE)
                            {
                                m_pBoardOrderList[m_IndexInList ++ ] = temp_xy.y * m_Width + temp_xy.x;
                                m_pBoardFlag[temp_xy.y * m_Width + temp_xy.x] = BLACK;
                            }
                            temp_xy.y = temp_xy.y + 1;
                            localHeightCounter++;
                        }
                    } // vertical edges

                     // do horizontal edges
                    for( uint32_t widthCount = 1; widthCount < m_26ZIBlockWidth; widthCount = widthCount + 2 )
                    {
                        CM_COORDINATE temp_xy;
                        uint32_t localHeightCounter = 0;

                        temp_xy.x = temp_xyFor26.x + widthCount;
                        temp_xy.y = temp_xyFor26.y;
                        while( (temp_xy.x >= 0) && (temp_xy.y >=0) &&
                            (temp_xy.x < (int32_t)m_Width) && (temp_xy.y < (int32_t)m_Height) &&
                            (localHeightCounter < m_26ZIBlockHeight))
                        {
                            if( m_pBoardFlag[temp_xy.y * m_Width + temp_xy.x] == WHITE)
                            {
                                m_pBoardOrderList[m_IndexInList ++ ] = temp_xy.y * m_Width + temp_xy.x;
                                m_pBoardFlag[temp_xy.y * m_Width + temp_xy.x] = BLACK;
                            }
                            temp_xy.y = temp_xy.y + 1;
                            localHeightCounter++;
                        }
                    } // horizontal edges
                }

                temp_xyFor26.x = temp_xyFor26.x - (2 * m_26ZIBlockWidth);
                temp_xyFor26.y = temp_xyFor26.y + (1 * m_26ZIBlockHeight);

            } while( ( temp_xyFor26.x >= 0) && (temp_xyFor26.y >= 0)
                && (temp_xyFor26.x < (int32_t)m_Width) && ( temp_xyFor26.y < (int32_t)m_Height));
        }
    }

    return CM_SUCCESS;
}

int32_t CmThreadSpaceEmu::Wavefront26ZISeqVVHH26()
{
    if ( m_CurrentDependencyPattern == CM_WAVEFRONT26ZI &&
        (m_Current26ZIDispatchPattern == VVERTICAL_HHORIZONTAL_26))
    {
        return CM_SUCCESS;
    }

    m_CurrentDependencyPattern = CM_WAVEFRONT26ZI;
    m_Current26ZIDispatchPattern = VVERTICAL_HHORIZONTAL_26;

    CmSafeMemSet(m_pBoardFlag, WHITE, m_Width*m_Height*sizeof(uint32_t));
    m_IndexInList = 0;

    for( uint32_t y = 0; y < m_Height; y = y + m_26ZIBlockHeight )
    {
        for( uint32_t x = 0; x < m_Width; x = x + m_26ZIBlockWidth )
        {
            CM_COORDINATE temp_xyFor26;
            temp_xyFor26.x = x;
            temp_xyFor26.y = y;

            do
            {
                if( m_pBoardFlag[temp_xyFor26.y * m_Width + temp_xyFor26.x] == WHITE )
                {
                    m_pBoardOrderList[m_IndexInList ++] = temp_xyFor26.y * m_Width + temp_xyFor26.x;
                    m_pBoardFlag[temp_xyFor26.y * m_Width + temp_xyFor26.x] = BLACK;

                    // do vertical edges
                    for( uint32_t widthCount = 0; widthCount < m_26ZIBlockWidth; widthCount = widthCount + 2 )
                    {
                        CM_COORDINATE temp_xy;
                        uint32_t localHeightCounter = 0;

                        temp_xy.x = temp_xyFor26.x + widthCount;
                        temp_xy.y = temp_xyFor26.y;
                        while( (temp_xy.x >= 0) && (temp_xy.y >=0) &&
                            (temp_xy.x < (int32_t)m_Width) && (temp_xy.y < (int32_t)m_Height) &&
                            (localHeightCounter < m_26ZIBlockHeight))
                        {
                            if( m_pBoardFlag[temp_xy.y * m_Width + temp_xy.x] == WHITE)
                            {
                                m_pBoardOrderList[m_IndexInList ++ ] = temp_xy.y * m_Width + temp_xy.x;
                                m_pBoardFlag[temp_xy.y * m_Width + temp_xy.x] = BLACK;
                            }
                            temp_xy.y = temp_xy.y + 1;
                            localHeightCounter++;
                        }
                    } // vertical edges

                    // horizontal edges
                    for( uint32_t heightCount = 0; heightCount < m_26ZIBlockHeight; ++heightCount )
                    {
                        CM_COORDINATE temp_xy;
                        uint32_t localWidthCounter = 0;

                        temp_xy.x = temp_xyFor26.x + 1;
                        temp_xy.y = temp_xyFor26.y + heightCount;
                        while ( (temp_xy.x >= 0) && (temp_xy.y >= 0) &&
                            (temp_xy.x< (int32_t)m_Width) && (temp_xy.y < (int32_t)m_Height) &&
                            (localWidthCounter < (m_26ZIBlockWidth / 2) ) )
                        {
                            if( m_pBoardFlag[temp_xy.y * m_Width + temp_xy.x] == WHITE)
                            {
                                m_pBoardOrderList[m_IndexInList ++ ] = temp_xy.y * m_Width + temp_xy.x;
                                m_pBoardFlag[temp_xy.y * m_Width + temp_xy.x] = BLACK;
                            }
                            temp_xy.x = temp_xy.x + 2;
                            localWidthCounter++;
                        }
                    }
                    // horizontal edges
                }

                temp_xyFor26.x = temp_xyFor26.x - (2 * m_26ZIBlockWidth);
                temp_xyFor26.y = temp_xyFor26.y + (1 * m_26ZIBlockHeight);

            } while( ( temp_xyFor26.x >= 0) && (temp_xyFor26.y >= 0)
                && (temp_xyFor26.x < (int32_t)m_Width) && ( temp_xyFor26.y < (int32_t)m_Height));
        }
    }

    return CM_SUCCESS;
}

int32_t CmThreadSpaceEmu::Wavefront26ZISeqVV26HH26()
{
    if( (m_CurrentDependencyPattern == CM_WAVEFRONT26ZI) &&
        (m_Current26ZIDispatchPattern == VVERTICAL26_HHORIZONTAL26) )
    {
        return CM_SUCCESS;
    }

    m_CurrentDependencyPattern = CM_WAVEFRONT26ZI;
    m_Current26ZIDispatchPattern = VVERTICAL26_HHORIZONTAL26;

    CmSafeMemSet(m_pBoardFlag, WHITE, m_Width*m_Height*sizeof(uint32_t));
    m_IndexInList = 0;

    uint32_t waveFrontNum = 0;
    uint32_t waveFrontStartX = 0;
    uint32_t waveFrontStartY = 0;

    uint32_t adjustHeight = 0;

    CM_COORDINATE temp_xyFor26;
    temp_xyFor26.x = 0;
    temp_xyFor26.y = 0;

    while( (temp_xyFor26.x >= 0) && (temp_xyFor26.y >= 0) &&
        (temp_xyFor26.x < (int32_t)m_Width) && (temp_xyFor26.y < (int32_t)m_Height) )
    {
        // use horizontal coordinates to save starting (x,y) for overall 26
        CM_COORDINATE temp_xyForHorz;
        temp_xyForHorz.x = temp_xyFor26.x;
        temp_xyForHorz.y = temp_xyFor26.y;

       do
        {
            CM_COORDINATE temp_xyForVer;

            for( uint32_t widthCount = 0; widthCount < m_26ZIBlockWidth; widthCount += 2 )
            {
                uint32_t localHeightCounter = 0;
                temp_xyForVer.x = temp_xyFor26.x + widthCount;
                temp_xyForVer.y = temp_xyFor26.y;

                while( (temp_xyForVer.x < (int32_t)m_Width) && (temp_xyForVer.y < (int32_t)m_Height) &&
                        (temp_xyForVer.x >= 0) && (temp_xyForVer.y >= 0) && (localHeightCounter < m_26ZIBlockHeight) )
                {
                    if(m_pBoardFlag[temp_xyForVer.y * m_Width + temp_xyForVer.x] == WHITE )
                    {
                        m_pBoardOrderList[m_IndexInList ++] = temp_xyForVer.y * m_Width + temp_xyForVer.x;
                        m_pBoardFlag[temp_xyForVer.y * m_Width + temp_xyForVer.x] = BLACK;
                    }
                    temp_xyForVer.y += 1;
                    localHeightCounter++;
                }
            }

            temp_xyFor26.x = temp_xyFor26.x + (2 * m_26ZIBlockWidth);
            temp_xyFor26.y = temp_xyFor26.y - ( 1 * m_26ZIBlockHeight);

        } while( (temp_xyFor26.x >= 0) && (temp_xyFor26.y >= 0) &&
            (temp_xyFor26.x < (int32_t)m_Width) && (temp_xyFor26.y < (int32_t)m_Height) );

        temp_xyFor26.x = temp_xyForHorz.x;
        temp_xyFor26.y = temp_xyForHorz.y;

        do
        {
            // do horizontal edges
            for ( uint32_t heightCount = 0; heightCount < m_26ZIBlockHeight; ++heightCount )
            {
                uint32_t localWidthCounter = 0;
                temp_xyForHorz.x = temp_xyFor26.x + 1;
                temp_xyForHorz.y = temp_xyFor26.y + heightCount;
                while( (temp_xyForHorz.x >= 0) && (temp_xyForHorz.y >= 0) &&
                    (temp_xyForHorz.x < (int32_t)m_Width) && (temp_xyForHorz.y < (int32_t)m_Height) &&
                    (localWidthCounter < (m_26ZIBlockWidth / 2)) )
                {
                    if( m_pBoardFlag[temp_xyForHorz.y * m_Width + temp_xyForHorz.x] == WHITE )
                    {
                        m_pBoardOrderList[m_IndexInList ++] = temp_xyForHorz.y * m_Width + temp_xyForHorz.x;
                        m_pBoardFlag[temp_xyForHorz.y * m_Width + temp_xyForHorz.x] = BLACK;
                    }

                    temp_xyForHorz.x += 2;
                    localWidthCounter++;
                }
            }

            temp_xyFor26.x = temp_xyFor26.x + (2 * m_26ZIBlockWidth);
            temp_xyFor26.y = temp_xyFor26.y - ( 1 * m_26ZIBlockHeight);

        } while( (temp_xyFor26.x >= 0) && (temp_xyFor26.y >= 0) &&
            (temp_xyFor26.x < (int32_t)m_Width) && (temp_xyFor26.y < (int32_t)m_Height) );

        if( m_Width <= m_26ZIBlockWidth )
        {
            temp_xyFor26.x = 0;
            temp_xyFor26.y = temp_xyForHorz.y + m_26ZIBlockHeight;
        }
        else
        {
            // update wavefront number
            waveFrontNum++;
            adjustHeight = (uint32_t)std::ceil((double)m_Height/m_26ZIBlockHeight);

            if( waveFrontNum < (2 * adjustHeight) )
            {
                waveFrontStartX = waveFrontNum & 1;
                waveFrontStartY = (uint32_t)std::floor((double)waveFrontNum/2);
            }
            else
            {
                waveFrontStartX = (waveFrontNum - 2 * adjustHeight) + 2;
                waveFrontStartY = (adjustHeight) - 1;
            }

            temp_xyFor26.x = waveFrontStartX * m_26ZIBlockWidth;
            temp_xyFor26.y = waveFrontStartY * m_26ZIBlockHeight;
        }
     }

    return CM_SUCCESS;
}

int32_t CmThreadSpaceEmu::Wavefront26ZISeqVV1x26HH1x26()
{
    if ( (m_CurrentDependencyPattern == CM_WAVEFRONT26ZI) &&
        (m_Current26ZIDispatchPattern == VVERTICAL1X26_HHORIZONTAL1X26))
    {
        return CM_SUCCESS;
    }

    m_CurrentDependencyPattern = CM_WAVEFRONT26ZI;
    m_Current26ZIDispatchPattern = VVERTICAL1X26_HHORIZONTAL1X26;

    CmSafeMemSet(m_pBoardFlag, WHITE, m_Width*m_Height*sizeof(uint32_t));
    m_IndexInList = 0;

    uint32_t waveFrontNum = 0;
    uint32_t waveFrontStartX = 0;
    uint32_t waveFrontStartY = 0;

    uint32_t adjustHeight = 0;

    CM_COORDINATE temp_xyFor26;
    temp_xyFor26.x = 0;
    temp_xyFor26.y = 0;

    CM_COORDINATE saveTemp_xyFor26;
    saveTemp_xyFor26.x = 0;
    saveTemp_xyFor26.y = 0;

    CM_COORDINATE temp_xyForVer;
    CM_COORDINATE temp_xyForHorz;

    while( (temp_xyFor26.x >= 0) && (temp_xyFor26.y >= 0) &&
        (temp_xyFor26.x < (int32_t)m_Width) && (temp_xyFor26.y < (int32_t)m_Height) )
    {
        saveTemp_xyFor26.x = temp_xyFor26.x;
        saveTemp_xyFor26.y = temp_xyFor26.y;

        // do vertical edges
        for( uint32_t widthCount = 0; widthCount < m_26ZIBlockWidth; widthCount += 2 )
        {
            // restore original starting point
            temp_xyFor26.x = saveTemp_xyFor26.x;
            temp_xyFor26.y = saveTemp_xyFor26.y;

            do
            {
                uint32_t localHeightCounter = 0;
                temp_xyForVer.x = temp_xyFor26.x + widthCount;
                temp_xyForVer.y = temp_xyFor26.y;
                while( (temp_xyForVer.x < (int32_t)m_Width) && (temp_xyForVer.y < (int32_t)m_Height) &&
                        (temp_xyForVer.x >= 0) && (temp_xyForVer.y >= 0) && (localHeightCounter < m_26ZIBlockHeight) )
                {
                    if(m_pBoardFlag[temp_xyForVer.y * m_Width + temp_xyForVer.x] == WHITE )
                    {
                        m_pBoardOrderList[m_IndexInList ++] = temp_xyForVer.y * m_Width + temp_xyForVer.x;
                        m_pBoardFlag[temp_xyForVer.y * m_Width + temp_xyForVer.x] = BLACK;
                    }
                    temp_xyForVer.y += 1;
                    localHeightCounter++;
                }

                temp_xyFor26.x = temp_xyFor26.x + (2 * m_26ZIBlockWidth);
                temp_xyFor26.y = temp_xyFor26.y - ( 1 * m_26ZIBlockHeight);

            } while( (temp_xyFor26.x >= 0) && (temp_xyFor26.y >= 0) &&
            (temp_xyFor26.x < (int32_t)m_Width) && (temp_xyFor26.y < (int32_t)m_Height) );
        }

        // do horizontal edges
        // restore original starting position
        temp_xyFor26.x = saveTemp_xyFor26.x;
        temp_xyFor26.y = saveTemp_xyFor26.y;

        for(uint32_t heightCount = 0; heightCount < m_26ZIBlockHeight; ++heightCount )
        {
            // restore original starting point
            temp_xyFor26.x = saveTemp_xyFor26.x;
            temp_xyFor26.y = saveTemp_xyFor26.y;

            do
            {
                uint32_t localWidthCounter = 0;
                temp_xyForHorz.x = temp_xyFor26.x + 1;
                temp_xyForHorz.y = temp_xyFor26.y + heightCount;
                while( (temp_xyForHorz.x >= 0) && (temp_xyForHorz.y >= 0) &&
                    (temp_xyForHorz.x < (int32_t)m_Width) && (temp_xyForHorz.y < (int32_t)m_Height) &&
                    (localWidthCounter < (m_26ZIBlockWidth / 2)) )
                {
                    if( m_pBoardFlag[temp_xyForHorz.y * m_Width + temp_xyForHorz.x] == WHITE )
                    {
                        m_pBoardOrderList[m_IndexInList ++] = temp_xyForHorz.y * m_Width + temp_xyForHorz.x;
                        m_pBoardFlag[temp_xyForHorz.y * m_Width + temp_xyForHorz.x] = BLACK;
                    }

                    temp_xyForHorz.x += 2;
                    localWidthCounter++;
                }

                temp_xyFor26.x = temp_xyFor26.x + (2 * m_26ZIBlockWidth);
                temp_xyFor26.y = temp_xyFor26.y - ( 1 * m_26ZIBlockHeight);

            } while( (temp_xyFor26.x >= 0) && (temp_xyFor26.y >= 0) &&
            (temp_xyFor26.x < (int32_t)m_Width) && (temp_xyFor26.y < (int32_t)m_Height) );

        }

        if (m_Width <= m_26ZIBlockWidth)
        {
            temp_xyFor26.x = 0;
            temp_xyFor26.y = saveTemp_xyFor26.y + m_26ZIBlockHeight;
        }
        else
        {
            // update wavefront number
            waveFrontNum++;
            adjustHeight = (uint32_t)std::ceil((double)m_Height/m_26ZIBlockHeight);

            if( waveFrontNum < (2 * adjustHeight) )
            {
                waveFrontStartX = waveFrontNum & 1;
                waveFrontStartY = (uint32_t)std::floor((double)waveFrontNum/2);
            }
            else
            {
                waveFrontStartX = (waveFrontNum - 2 * adjustHeight) + 2;
                waveFrontStartY = (adjustHeight) - 1;
            }

            temp_xyFor26.x = waveFrontStartX * m_26ZIBlockWidth;
            temp_xyFor26.y = waveFrontStartY * m_26ZIBlockHeight;
        }
    }

    return CM_SUCCESS;
}

//!
//! Set the mask to enable/disable dependent units in the dependency pattern.
//! Each bit is corresponding to one dependent unit in the dependency pattern.
//! bit 0 is corresponding to the first dependent unit, i.e. unit notated by ( X + deltaX[0], Y + deltaY[0] )
//! bit 1 is corresponding to the second dependent unit, i.e. unit notated by ( X + deltaX[2], Y + deltaY[2] )
//! 1 means enable this dependent unit. 0 means disable this dependent unit.
//! This mask is a for all threads in one task. Each thread also has its own mask.
//! The two masks are ANDed to decide to enable/disable the dependent units for the thread.
//! By default, mask is zero.
//! Input :
//!     8bit mask
//! OUTPUT :
//!     CM_SUCCESS if the mask is set
//!
CM_RT_API int32_t CmThreadSpaceEmu::SetGlobalThreadDependencyMask( uint8_t mask )
{
#if 0
    m_Mask = mask;
    return CM_SUCCESS;
#else
    return CmNotImplemented(__PRETTY_FUNCTION__);
#endif
}

int32_t CmThreadSpaceEmu::GetThreadSpaceSize(uint32_t & width, uint32_t & height)
{
    width = m_Width;
    height = m_Height;

    return CM_SUCCESS;
}

int32_t CmThreadSpaceEmu::GetThreadSpaceUnit(CM_THREAD_SPACE_UNIT* &pThreadSpaceUnit)
{
    pThreadSpaceUnit = m_pThreadSpaceUnit;
    return CM_SUCCESS;
}

int32_t CmThreadSpaceEmu::GetDependency(CM_DEPENDENCY* &pDependency)
{
    pDependency = &m_Dependency;
    return CM_SUCCESS;
}

int32_t CmThreadSpaceEmu::GetColorCount(uint32_t & colorCount)
{
    colorCount = m_ColorCount;
    return CM_SUCCESS;
}

int32_t CmThreadSpaceEmu::GetWalkingPattern(CM_WALKING_PATTERN &WalkingPattern)
{
    WalkingPattern = m_WalkingPattern;
    return CM_SUCCESS;
}

int32_t CmThreadSpaceEmu::GetBoardOrder(uint32_t *&pBoardOrder)
{
    pBoardOrder = m_pBoardOrderList;
    return CM_SUCCESS;
}

int32_t CmThreadSpaceEmu::InitDependency()
{
    int32_t xCoord = 0;
    int32_t yCoord = 0;

    //reset the check if it was done twice.
    for(uint32_t y = 0; y < m_Height; y++)
    {
        for(uint32_t x = 0; x < m_Width; x++)
        {
            m_pThreadSpaceUnit[y*m_Width +x].numEdges = 0;
        }
    }

    for(uint32_t y = 0; y < m_Height; y++)
    {
        for(uint32_t x = 0; x < m_Width; x++)
        {
            for(uint32_t i = 0; i < m_Dependency.count; i++)
            {
                //checking for valid dependency
                if(m_Dependency.deltaX[i] == 0 && m_Dependency.deltaY[i] == 0)
                    continue;

                xCoord = x + m_Dependency.deltaX[i];
                yCoord = y + m_Dependency.deltaY[i];

                //checking for valid coordinates
                if(xCoord < 0 || xCoord >= (int) m_Width ||
                   yCoord < 0 || yCoord >= (int)m_Height)
                    continue;

                uint32_t linear_offset = yCoord*m_Width + xCoord;
                m_pThreadSpaceUnit[y*m_Width +x].numEdges++;
            }
        }
    }
    return CM_SUCCESS;
}

int32_t CmThreadSpaceEmu::AssociateKernel(CmKernel* kernel)
{
    for(uint32_t y = 0; y < m_Height; y++)
        for(uint32_t x = 0; x < m_Width; x++) {
            m_pThreadSpaceUnit[y*m_Width +x].pKernel = dynamic_cast<CmKernelEmu *>(kernel);
            m_pThreadSpaceUnit[y*m_Width +x].threadId= y*m_Width +x;
        }

//    m_ThreadAssociated = true;

    return CM_SUCCESS;
}

bool CmThreadSpaceEmu::IsThreadAssociated()
{
    return m_ThreadAssociated;
}

bool CmThreadSpaceEmu::IsDependencySet()
{
    return ((m_Dependency.count != 0) ? true : false);
}
