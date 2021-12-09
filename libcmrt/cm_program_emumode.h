/*========================== begin_copyright_notice ============================

Copyright (C) 2017 Intel Corporation

SPDX-License-Identifier: MIT

============================= end_copyright_notice ===========================*/

#pragma once

#include "cm_def.h"
#include "cm_program_base.h"
#include "emu_log.h"
#include "emu_kernel_support.h"

class CmProgram;
class CmDeviceEmu;

class CmProgramEmu: public CmProgram
{
private:
    const GfxEmu::KernelSupport::ProgramModule m_programModule;

public:
    const GfxEmu::KernelSupport::ProgramModule& GetProgramModule() const {
        return m_programModule;
    }

    static int32_t Create(
        CmDeviceEmu* pCmDev,
        CmProgramEmu*& pProgram,
        void *programAddr = nullptr,
        size_t size = 0);

    static int32_t Destroy( CmProgramEmu* &pProgram );

    int Acquire();
    int SafeRelease();

    int32_t GetCommonISACode( void* & pCommonISACode, uint32_t & size ) {
        pCommonISACode = nullptr; size = 0;
        return CmNotImplemented(__PRETTY_FUNCTION__);
    }

protected:
    CmProgramEmu( CmDeviceEmu* pCmDev, void *programAddr = nullptr, size_t size = 0);
    ~CmProgramEmu( void );

    CmDeviceEmu* m_pCmDev;
    int m_refCount;
};
