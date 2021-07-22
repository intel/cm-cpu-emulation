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
