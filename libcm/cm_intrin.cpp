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

#include "cm_lib.h"
#include "genx_lib.h"
#include "cm_intrin.h"

void __this_thread_yield(){
    cmrt::this_thread_yield();
}

CM_API void cm_barrier()
{

    cmrt::simple_group_barrier_signal();
    cmrt::simple_group_barrier_wait();
}

CM_API void cm_sbarrier(uint flag)
{

    flag ? cmrt::simple_group_barrier_signal() :
           cmrt::simple_group_barrier_wait();
}

CM_API char* __cm_emu_get_slm(void)
{
    return cmrt::get_slm();
}

CM_API size_t __cm_emu_get_slm_size(void)
{
    return cmrt::get_slm_size();
}

CM_API void __cm_emu_aux_barrier()
{
    cmrt::aux_barrier_signal();
    cmrt::aux_barrier_wait();
}

CM_API void cm_slm_init(unsigned int size)
{
    cmrt::set_slm_size(size);
}

CM_API unsigned int cm_slm_alloc(unsigned int size)
{
    return cmrt::alloc_slm(size);
}

CM_API void cm_slm_free(void)
{

}

void check_dimention(uint dim)
{
    if (dim > 2)
    {
        GFX_EMU_ERROR_MESSAGE("Error in SLM Emulation: Bad dimension!\n");
        exit(EXIT_FAILURE);
    }
}

CM_API uint cm_local_id (uint dim) // Local Thread IDs in current group
{
    check_dimention(dim);
    return cmrt::thread_idx(dim);
}

CM_API uint cm_local_size(uint dim) // Number of threads per group
{
    check_dimention(dim);
    return cmrt::thread_count(dim);
}

CM_API uint cm_group_id(uint dim) // Group IDs
{
    check_dimention(dim);
    return cmrt::group_idx(dim);
}

CM_API uint cm_group_count(uint dim) // Number of groups
{
    check_dimention(dim);
    return cmrt::group_count(dim);
}

// Set the fp rounding mode
CM_API void  cm_fsetround(CmRoundingMode val)
{
    unsigned int xval;
    switch(val)
    {
    case CM_RTE:
        xval = FE_TONEAREST;
        break;
    case CM_RTP:
        xval = FE_UPWARD;
        break;
    case CM_RTN:
        xval = FE_DOWNWARD;
        break;
    case CM_RTZ:
        xval = FE_TOWARDZERO;
        break;
    default:
        GFX_EMU_ERROR_MESSAGE("Unexpected rounding mode: %d\n", val);
        exit(EXIT_FAILURE);
    }
    fesetround(xval);

    return;
}

CM_API void cm_pause(unsigned short length)
{
    std::cerr << "\ncm_pause intrinsic is not supported in emulation mode and doesn't do anything";
}

// Get the fp rounding mode
CM_API CmRoundingMode cm_fgetround(void)
{
    CmRoundingMode val;

    unsigned int xval = fegetround();
    switch (xval)
    {
    case FE_TONEAREST:
        val = CM_RTE;
        break;
    case FE_UPWARD:
        val = CM_RTP;
        break;
    case FE_DOWNWARD:
        val = CM_RTN;
        break;
    case FE_TOWARDZERO:
        val = CM_RTZ;
        break;
    default:
        GFX_EMU_ERROR_MESSAGE("Retrieved unexpected rounding mode from control work: %04x\n", xval);
        exit(EXIT_FAILURE);
    }

    return val;
}

// Set the fp mode ALT/IEEE, CM EMU doesn't support this so always use IEEE mode.
CM_API void cm_fsetmode(CmFPMode val)
{
    switch(val)
    {
        case CM_ALT: break;
        case CM_IEEE: break;
        default:
            GFX_EMU_ERROR_MESSAGE("Unexpected fp mode %dn", val);
            exit(EXIT_FAILURE);
    }
}

// Get the fp mode (ALT/IEEE). CM EMU doesn't support this so always use IEEE mode.
CM_API CmFPMode cm_fgetmode()
{
    return CM_IEEE;
}
