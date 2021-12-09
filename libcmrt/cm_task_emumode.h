/*========================== begin_copyright_notice ============================

Copyright (C) 2017 Intel Corporation

SPDX-License-Identifier: MIT

============================= end_copyright_notice ===========================*/

#pragma once

#include "cm_task_base.h"
#include "cm_def.h"
#include "emu_log.h"

class CmKernel;

class CmTask_RT : public CmTask
{
public:
    static int32_t Create( uint32_t max_kernel_count, CmTask_RT* &pKernelArray);
    static int32_t Destroy(CmTask_RT* &pKernelArray );

    CM_RT_API int32_t AddKernel( CmKernel *pKernel );
    CM_RT_API int32_t Reset(void);
    CM_RT_API int32_t AddSync(void);
    CM_RT_API int32_t SetPowerOption( PCM_POWER_OPTION pCmPowerOption ) { return CmNotImplemented(__PRETTY_FUNCTION__); }
    CM_RT_API int32_t AddConditionalEnd(SurfaceIndex* pConditionalSurface, uint32_t offset, CM_CONDITIONAL_END_PARAM *pCondParam) { return CmNotImplemented(__PRETTY_FUNCTION__); }
    CM_RT_API int32_t SetProperty(const CM_TASK_CONFIG &taskConfig){ return CmNotImplemented(__PRETTY_FUNCTION__); }
    CM_RT_API int32_t GetProperty(CM_TASK_CONFIG &taskConfig) { return CmNotImplemented(__PRETTY_FUNCTION__); };
    CM_RT_API int32_t AddKernelWithConfig( CmKernel *pKernel, const CM_EXECUTION_CONFIG *config );
    CM_RT_API int32_t AddSyncEx(const CM_KERNEL_SYNC_CONFIG *config) { return CM_SUCCESS; }
    uint32_t GetKernelCount();
    CmKernel* GetKernelPointer(uint32_t index);

protected:
    CmTask_RT(uint32_t max_kernel_count);
    ~CmTask_RT( void );

    int32_t Initialize( );

    uint32_t m_MaxKernelCount;
    CmKernel ** m_pKernelArray;
    uint32_t m_KernelCount;
};

#define CmKernelArrayEmu CmTask_RT
