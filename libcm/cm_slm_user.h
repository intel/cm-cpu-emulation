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

//------------------------------------------------------------------------------
// Functions implemented as user-level Cm kernels
//------------------------------------------------------------------------------

#ifndef CM_SLM_USER_H
#define CM_SLM_USER_H

#include "cm_intrin.h"

// This needs to be set by the user if GPGPU_MODE is needed
#define GPGPU_MODE
#ifdef GPGPU_MODE
// LOAD 'loadSize' bytes from memory surface 'memSurfIndex' starting at 'memOffset' to the SLM buffer 'slmBuffer'
// Use all the threads in the current group to read from memory and write to SLM
template <typename T1, typename T2>
_GENX_ inline void cm_slm_load (uint         slmBuffer,    // SLM buffer
                                SurfaceIndex memSurfIndex, // Memory SurfaceIndex
                                T1           memOffset,    // Byte-Offset in Memory Surface
                                T2           loadSize      // Bytes to be Loaded from Memory
                                )
{
    constexpr ushort __cm_init_seq[8] = {0,1,2,3,4,5,6,7};

    vector<ushort, 16> v_Offset(__cm_init_seq);
    v_Offset.select<8,1>(8) = v_Offset.select<8,1>(0) + 8;

    //check that loadSize is a multiple of 256 * #_threads_in_a_group
    if( loadSize % 256 != 0 ) {
        GFX_EMU_ERROR_MESSAGE("Warning: load size for cm_slm_load() must be multiple of 256\n");
    }

    int numTotalBlocks = loadSize / 256;
    int numGroups = cm_linear_local_size();
    int numBlocks = numTotalBlocks / numGroups;
    int numLeftOver = numTotalBlocks % numGroups;
    numBlocks += cm_linear_local_id() < numLeftOver ? 1 : 0;

    // We just need numBlocks and numGroups
    ushort elemSize = sizeof(float);
    int threadOffsetInSLM = cm_linear_local_id() * 256;
    // in bytes
    int threadOffsetInMemory = memOffset + threadOffsetInSLM;
    // in unit of elements
    vector<ushort, 16> v_Offsets = (threadOffsetInSLM / elemSize) + v_Offset * 4;

    for( int block = 0; block < numBlocks; block++ ) {
        vector<uint,   32> row0; // 32 floats or 128 Bytes or 4 GRF-registers
        vector<uint,   32> row1;
        vector<uint,   64> rowTrans;
        read (memSurfIndex, threadOffsetInMemory, row0);
        read (memSurfIndex, threadOffsetInMemory + 128, row1);

        //Transpose
        rowTrans.select<8,1>(0)  = row0.select<8,4>(0);
        rowTrans.select<8,1>(16) = row0.select<8,4>(1);
        rowTrans.select<8,1>(32) = row0.select<8,4>(2);
        rowTrans.select<8,1>(48) = row0.select<8,4>(3);

        rowTrans.select<8,1>(8)  = row1.select<8,4>(0);
        rowTrans.select<8,1>(24) = row1.select<8,4>(1);
        rowTrans.select<8,1>(40) = row1.select<8,4>(2);
        rowTrans.select<8,1>(56) = row1.select<8,4>(3);

        cm_slm_write4 (slmBuffer, v_Offsets, rowTrans, SLM_ABGR_ENABLE);
        threadOffsetInMemory += numGroups * 256;
        v_Offsets += numGroups * 64;
    }

    cm_barrier();
}

#endif // GPGPU_MODE

#endif // CM_SLM_USER_H
