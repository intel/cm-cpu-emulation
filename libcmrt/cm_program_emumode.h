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

