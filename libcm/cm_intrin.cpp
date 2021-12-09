/*========================== begin_copyright_notice ============================

Copyright (C) 2017 Intel Corporation

SPDX-License-Identifier: MIT

============================= end_copyright_notice ===========================*/

#include "cm_lib.h"
#include "genx_lib.h"
#include "cm_intrin.h"

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
#ifdef __GNUC__
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
#else
    unsigned int xval;
    switch(val)
    {
    case CM_RTE:
        xval = _RC_NEAR;
        break;
    case CM_RTP:
        xval = _RC_UP;
        break;
    case CM_RTN:
        xval = _RC_DOWN;
        break;
    case CM_RTZ:
        xval = _RC_CHOP;
        break;
    default:
        GFX_EMU_ERROR_MESSAGE("Unexpected rounding mode: %d\n", val);
        exit(EXIT_FAILURE);
    }
    _control87(xval, _MCW_RC);
#endif

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

#ifdef __GNUC__
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
#else
    unsigned int xval = _control87(0, 0);
    switch (xval & _MCW_RC)
    {
    case _RC_NEAR:
        val = CM_RTE;
        break;
    case _RC_UP:
        val = CM_RTP;
        break;
    case _RC_DOWN:
        val = CM_RTN;
        break;
    case _RC_CHOP:
        val = CM_RTZ;
        break;
    default:
        GFX_EMU_ERROR_MESSAGE("Retrieved unexpected rounding mode from control work: %04x\n", xval & _MCW_RC);
        exit(EXIT_FAILURE);
    }
#endif

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

CM_API unsigned
cm_addc(unsigned src0, unsigned src1, unsigned &carry)
{
    vector<unsigned, 1> Src0 = src0;
    vector<unsigned, 1> Src1 = src1;
    vector<unsigned, 1> Carry;
    vector<unsigned, 1> Result = cm_addc<1>(Src0, Src1, Carry);
    carry = Carry(0);
    return Result(0);
}

CM_API unsigned
cm_subb(unsigned minuend, unsigned subtrahend, unsigned &borrow)
{
    vector<unsigned, 1> Minuend = minuend;
    vector<unsigned, 1> Subtrahend = subtrahend;
    vector<unsigned, 1> Borrow;
    vector<unsigned, 1> Result = cm_subb<1>(Minuend, Subtrahend, Borrow);
    borrow = Borrow(0);
    return Result(0);
}
