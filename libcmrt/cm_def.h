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

#ifndef CMRTLIB_AGNOSTIC_SHARE_CM_DEF_H_
#define CMRTLIB_AGNOSTIC_SHARE_CM_DEF_H_

#include "cm_include.h"
#include "cm_def_os.h"
#include "cm_version_defs.h"

//CM DDI version in UMD layer
#define CM_DDI_1_0 100
#define CM_DDI_1_1 101
#define CM_DDI_1_2 102
#define CM_DDI_1_3 103
#define CM_DDI_1_4 104
#define CM_DDI_2_0 200
#define CM_DDI_2_1 201
#define CM_DDI_2_2 202
#define CM_DDI_2_3 203
#define CM_DDI_2_4 204
#define CM_DDI_3_0 300
#define CM_DDI_4_0 400
#define CM_DDI_5_0 500
#define CM_DDI_6_0 600
#define CM_DDI_7_0 700
#define CM_DDI_7_2 702 //for MDFRT API refreshment.

#include "type_return_code.h"

#define __CODEGEN_UNIQUE(name)                  _NAME_LABEL_(name, __LINE__)
#define BITFIELD_RANGE( startbit, endbit )     ((endbit)-(startbit)+1)
#define BITFIELD_BIT(bit)                        1

#define CM_MIN_SURF_WIDTH       1
#define CM_MIN_SURF_HEIGHT      1
#define CM_MIN_SURF_DEPTH       2
#define CM_MAX_1D_SURF_WIDTH    0x80000000 // 2^31 2 GB

#define CM_MAX_3D_SURF_WIDTH 2048
#define CM_MAX_3D_SURF_HEIGHT 2048
#define CM_MAX_3D_SURF_DEPTH 2048

// hard ceiling
#define CM_MAX_OPTION_SIZE_IN_BYTE 512
#define CM_MAX_KERNEL_NAME_SIZE_IN_BYTE 256
#define CM_MAX_ISA_FILE_NAME_SIZE_IN_BYTE 256

struct CM_HAL_MAX_VALUES
{
    uint32_t maxTasks;                          // [in] Max Tasks
    uint32_t maxKernelsPerTask;                 // [in] Max kernels per task
    uint32_t maxKernelBinarySize;               // [in] Max kernel binary size
    uint32_t maxSpillSizePerHwThread;           // [in] Max spill size per thread
    uint32_t maxSamplerTableSize;               // [in] Max sampler table size
    uint32_t maxBufferTableSize;                // [in] Buffer table Size
    uint32_t max2DSurfaceTableSize;             // [in] Buffer table Size
    uint32_t max3DSurfaceTableSize;             // [in] Buffer table Size
    uint32_t maxArgsPerKernel;                  // [in] Max arguments per kernel
    uint32_t maxArgByteSizePerKernel;           // [in] Max argument size in byte per kernel
    uint32_t maxSurfacesPerKernel;              // [in] Max Surfaces Per Kernel
    uint32_t maxSamplersPerKernel;              // [in] Max Samplers per kernel
    uint32_t maxHwThreads;                      // [in] Max HW threads
    uint32_t maxUserThreadsPerTask;             // [in] Max user threads per task
    uint32_t maxUserThreadsPerTaskNoThreadArg;  // [in] Max user threads per task without a thread arg
};
typedef CM_HAL_MAX_VALUES *PCM_HAL_MAX_VALUES;

//---------------------------------------------------------------------------
//! HAL CM Max Values extention which has more entries which are not included
//! in CM_HAL_MAX_VALUES
//---------------------------------------------------------------------------
struct CM_HAL_MAX_VALUES_EX
{
    uint32_t max2DUPSurfaceTableSize;       // [in] Max 2D UP surface table Size
    uint32_t maxSampler8x8TableSize;        // [in] Max sampler 8x8 table size
    uint32_t maxCURBESizePerKernel;         // [in] Max CURBE size per kernel
    uint32_t maxCURBESizePerTask;           // [in] Max CURBE size per task
    uint32_t maxIndirectDataSizePerKernel;  // [in] Max indirect data size per kernel
    uint32_t maxUserThreadsPerMediaWalker;  // [in] Max user threads per media walker
    uint32_t maxUserThreadsPerThreadGroup;  // [in] Max user threads per thread group
};
typedef CM_HAL_MAX_VALUES_EX *PCM_HAL_MAX_VALUES_EX;

class CLock
{
public:
    CLock(CSync &refSync) : m_refSync(refSync) { Lock(); }
    ~CLock() { Unlock(); }

private:
    CSync &m_refSync;                     // Synchronization object

    CLock(const CLock &refcSource);
    CLock &operator=(const CLock &refcSource);
    void Lock() { m_refSync.Acquire(); }
    void Unlock() { m_refSync.Release(); }
};

#endif  // #ifndef CMRTLIB_AGNOSTIC_SHARE_CM_DEF_H_
