/*========================== begin_copyright_notice ============================

Copyright (C) 2017 Intel Corporation

SPDX-License-Identifier: MIT

============================= end_copyright_notice ===========================*/

#pragma once
#include <map>
#include <vector>

#include "cm_def.h"
#include "cm_kernel_base.h"
#include "emu_kernel_support.h"

class CmDevice;
class CmDeviceEmu;
class CmProgram;
class CmKernel;
class CmThreadSpaceEmu;
//!
//! CM Kernel
//!
class CmKernelEmu : public CmKernel
{
public:

    static int32_t Create( CmDevice* pCmDev, CmProgram* pProgram, const char* kernelName, CmKernelEmu* &pKernel, const char* options );
    static int32_t Create( CmDeviceEmu *device, CmProgram* pProgram, const char* kernelName, const void *fncPnt, CmKernelEmu* &pKernel, const char* options );
    static int32_t Destroy( CmKernelEmu* &pKernel );

    int Acquire();
    int SafeRelease();

    int32_t GetBinary(void* & pBinary, uint32_t & size );
    //int32_t GetThreadCount(uint32_t& count );
    const void* GetFuncPnt() const;

    const GfxEmu::KernelSupport::ProgramModule& GetProgramModule() const {
        return m_programModule;
    }

    const GfxEmu::KernelSupport::FunctionDesc& GetFunctionDesc() const {
        return m_functionDesc;
    }

    CM_RT_API int32_t SetThreadCount(uint32_t count );
    /*CM_RT_API */int32_t GetThreadCount(uint32_t& count );
    CM_RT_API int32_t SetKernelArg(uint32_t index, size_t size, const void * pValue );
    CM_RT_API int32_t SetStaticBuffer(uint32_t index, const void * pValue );

    CM_RT_API int32_t SetThreadArg(uint32_t threadId, uint32_t index, size_t size, const void * pValue );

    CM_RT_API int32_t SetThreadDependencyMask( uint32_t threadId, uint8_t mask );
    CM_RT_API int32_t SetSurfaceBTI(SurfaceIndex* pSurface, uint32_t BTIndex);

    CM_RT_API int32_t AssociateThreadSpace(CmThreadSpace* & pTS);
    CM_RT_API int32_t AssociateThreadGroupSpace(CmThreadGroupSpace* & pTGS);
    CM_RT_API int32_t DeAssociateThreadSpace(CmThreadSpace* & pTS);
    CM_RT_API int32_t DeAssociateThreadGroupSpace(CmThreadGroupSpace* & pTGS);
    CM_RT_API int32_t QuerySpillSize(unsigned int &spillSize);
    CM_RT_API int32_t SetKernelArgPointer(uint32_t index, size_t size, const void *pValue);
    int32_t ResetArgs( void );

    bool CheckArgs();
    const std::vector<GfxEmu::KernelArg>& GetArgsVecRef( ) const;
    int32_t GetArgs( GfxEmu::KernelArg* & pArg );
    int32_t GetArgCount( uint32_t & argCount );
    int32_t GetThreadArgCount( uint32_t & threadArgCount );
    int32_t GetMaxArgCount( uint32_t & maxArgCount );

    char *GetName( void ) const { return (char *)m_KernelName; }
    char *GetOptions(void) { return (char *)m_Options; }

    void AddScoreBoardCoord(uint32_t x, uint32_t y, uint32_t threadID);

    bool GetSCBCoord(uint32_t threadID, uint32_t &x, uint32_t &y);

    int32_t SetIndexInTask(uint32_t index);

    uint32_t GetIndexInTask(void);

    int32_t SetAssociatedToTSFlag(bool b);

    int32_t GetThreadSpace(CmThreadSpaceEmu* &threadSpace);

    int32_t GetThreadGroupSpace(CmThreadGroupSpace* &threadGroupSpace);
protected:
    CmKernelEmu(CmDeviceEmu *device, const void * fncPt, const GfxEmu::KernelSupport::ProgramModule&,
        const GfxEmu::KernelSupport::FunctionDesc&);

    ~CmKernelEmu() = default;

    int32_t Initialize( const char* kernelName, const char* options );
    int32_t AssociateThreadSpace_preG12(CmThreadSpace *&pTS);
    int32_t DeAssociateThreadSpace_preG12(CmThreadSpace *&pTS);

    CmDeviceEmu * m_pCmDev;
    CmProgram * m_pProgram;
    uint8_t m_KernelName[ CM_MAX_KERNEL_NAME_SIZE_IN_BYTE ];
    uint8_t m_Options[ CM_MAX_OPTION_SIZE_IN_BYTE ];
    void* m_pBinary;
    const void * funcPt;
    uint32_t m_BinaryCodeSize;

    const GfxEmu::KernelSupport::ProgramModule& m_programModule;
    const GfxEmu::KernelSupport::FunctionDesc& m_functionDesc;

    uint32_t m_ThreadCount;

    uint32_t m_ArgCount;
    uint32_t m_ThreadArgCount;
    uint32_t m_MaxArgCount;
    uint32_t m_argSizeTotal;

    uint32_t m_IndexInTask;
    bool m_AssociatedToTS;      //Indicates if this kernel is associated the task threadspace (scoreboard)

    CM_HAL_MAX_VALUES* m_pMaxVhalVals;
    std::vector <GfxEmu::KernelArg> m_Args;
    SurfaceIndex*  m_GlobalSurfaces[CM_MAX_GLOBAL_SURFACE_NUMBER];

    class SIM_SCB_COORD
    {
    public:
        uint32_t x;
        uint32_t y;
    };
    std::map<uint32_t,SIM_SCB_COORD*> m_scoreboard_coord;
    int m_refcount;

    CmThreadSpaceEmu* m_threadSpace;
    CmThreadGroupSpace* m_threadGroupSpace;
};
