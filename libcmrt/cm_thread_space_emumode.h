/*========================== begin_copyright_notice ============================

Copyright (C) 2017 Intel Corporation

SPDX-License-Identifier: MIT

============================= end_copyright_notice ===========================*/

#pragma once

#include <cstdint>
#include "cm_thread_space_base.h"
#include "cm_def.h"
#include "emu_log.h"

#define CM_MAX_DEPENDENCY_COUNT 8

class CmThreadSpace;
class CmDeviceEmu;
class CmKernel;
class CmTask_RT;
class CmThreadGroupSpace;

class CmThreadSpaceEmu : public CmThreadSpace
{
public:
    static int32_t Create( CmDeviceEmu* pDevice, uint32_t width, uint32_t height, CmThreadSpaceEmu* & pTS );
    static int32_t Destroy( CmThreadSpaceEmu* & pTS );

    CM_RT_API int32_t AssociateThread( uint32_t x, uint32_t y, CmKernel* pKernel , uint32_t threadId );
    CM_RT_API int32_t SetThreadDependencyPattern( uint32_t count, int32_t *deltaX, int32_t *deltaY );
    CM_RT_API int32_t SetGlobalThreadDependencyMask( uint8_t mask );
    CM_RT_API int32_t SelectThreadDependencyPattern ( CM_DEPENDENCY_PATTERN pattern );
    CM_RT_API int32_t AssociateThreadWithMask( uint32_t x, uint32_t y, CmKernel* pKernel , uint32_t threadId, uint8_t nDependencyMask );
    CM_RT_API int32_t SetThreadSpaceColorCount( uint32_t colorCount );
    CM_RT_API int32_t SelectMediaWalkingPattern( CM_WALKING_PATTERN pattern );
    CM_RT_API int32_t Set26ZIDispatchPattern( CM_26ZI_DISPATCH_PATTERN pattern );
    CM_RT_API int32_t Set26ZIMacroBlockSize( uint32_t width, uint32_t height );
    CM_RT_API int32_t SetMediaWalkerGroupSelect(CM_MW_GROUP_SELECT groupSelect) { return CmNotImplemented(__PRETTY_FUNCTION__); }
    CM_RT_API int32_t SelectMediaWalkingParameters( CM_WALKING_PARAMETERS parameters ) { return CmNotImplemented(__PRETTY_FUNCTION__); }
    CM_RT_API int32_t SelectThreadDependencyVectors( CM_DEPENDENCY dependVectors ) { return CmNotImplemented(__PRETTY_FUNCTION__); }
    CM_RT_API int32_t SetThreadSpaceOrder(uint32_t threadCount, PCM_THREAD_PARAM pThreadSpaceOrder){ return CmNotImplemented(__PRETTY_FUNCTION__); }
    int32_t GetThreadSpaceSize(uint32_t & width, uint32_t & height);
    int32_t GetThreadSpaceUnit(CM_THREAD_SPACE_UNIT* &pThreadSpaceUnit);
    int32_t GetDependency(CM_DEPENDENCY* &pDependency);
    int32_t GetColorCount(uint32_t & colorCount);
    int32_t GetWalkingPattern(CM_WALKING_PATTERN &WalkingPattern);
    int32_t Get26ZIDispatchPattern( CM_26ZI_DISPATCH_PATTERN &pattern);
    int32_t GetBoardOrder(uint32_t *&pBoardOrder);
    int32_t InitDependency();
    int32_t AssociateKernel(CmKernel* kernel);
    bool IsThreadAssociated();
    bool IsDependencySet();
    int32_t GetDependencyPatternType(CM_DEPENDENCY_PATTERN &DependencyPatternType);
    bool IntegrityCheck(CmTask_RT* pTask);
    int32_t Wavefront26ZSequence();
    int32_t Wavefront26ZISeqVVHV26();
    int32_t Wavefront26ZISeqVVHH26();
    int32_t Wavefront26ZISeqVV26HH26();
    int32_t Wavefront26ZISeqVV1x26HH1x26();
    int32_t AssociateThreadInternal( uint32_t x, uint32_t y, CmKernel* pKernel, uint32_t threadId, uint8_t dependencyMask );
    CmThreadGroupSpace *GetThreadGroupSpace();

protected:
    CmThreadSpaceEmu( CmDeviceEmu* pDevice, uint32_t width, uint32_t height );
    ~CmThreadSpaceEmu( void );

    int32_t Initialize( void );

    CmDeviceEmu* m_pDevice;

    uint32_t m_Width;
    uint32_t m_Height;
    uint32_t m_ColorCount;

    uint32_t m_26ZIBlockWidth;
    uint32_t m_26ZIBlockHeight;

    uint8_t m_Mask; // CM_MAX_DEPENDENCY_COUNT bit

    CM_DEPENDENCY m_Dependency;
    CM_DEPENDENCY_PATTERN m_DependencyPatternType;
    CM_DEPENDENCY_PATTERN m_CurrentDependencyPattern;
    CM_WALKING_PATTERN m_WalkingPattern;

    CM_26ZI_DISPATCH_PATTERN m_26ZIDispatchPattern;
    CM_26ZI_DISPATCH_PATTERN m_Current26ZIDispatchPattern;

    CM_THREAD_SPACE_UNIT *m_pThreadSpaceUnit;
    bool m_ThreadAssociated;

    uint32_t *m_pBoardFlag;
    uint32_t *m_pBoardOrderList;
    uint32_t m_IndexInList;

    CmThreadGroupSpace *m_threadGroupSpace_h;
};
