/*========================== begin_copyright_notice ============================

Copyright (C) 2017 Intel Corporation

SPDX-License-Identifier: MIT

============================= end_copyright_notice ===========================*/

#include "cm_lib.h"
#include "genx_lib.h"
#include "cm_intrin.h"

CM_API void cm_nbarrier_init(uint count) {
    cmrt::group_named_barriers_init(count);
};

CM_API void cm_nbarrier_signal(
    uint barrierId,
    uint producerConsumerMode,
    uint numProducers,
    uint numConsumers)
{
    if (producerConsumerMode > (int)cmrt::NamedBarrierMode::MaxMode)
    {
        std::cerr << "*** Error: invalid mode constant for cm_nbarrier_signal supplied: "
            << producerConsumerMode << std::endl;
        exit(EXIT_FAILURE);
    }

#ifdef CM_EMU_USE_SIMPLE_BARRIER
    if (barrierId == 0)
    {
        if (!(numProducers == numConsumers == cmrt::group_size ()))
        {
            std::cerr << "Using both simple barrier and named barriersi implementations is only allowed"
                " when all workgroup threads participate"
                " and are in producer+consumer mode." << std::endl;
            exit(EXIT_FAILURE);
        }

        return cmrt::simple_group_barrier_signal();
    }
#endif

    // What was the reason to make ProducerConsumer = 0 instead of Producer|Consumer ?!
    const bool
        isProdCons = producerConsumerMode == (int)cmrt::NamedBarrierMode::ProducerConsumer,
        isProd = isProdCons || producerConsumerMode == (int)cmrt::NamedBarrierMode::Producer,
        isCons = isProdCons || producerConsumerMode == (int)cmrt::NamedBarrierMode::Consumer
    ;

    cmrt::group_barrier_signal(
        barrierId,
        isProd,
        isCons,
        numProducers,
        numConsumers);
}

CM_API void cm_nbarrier_wait(uint barrierId)
{
#ifdef CM_EMU_USE_SIMPLE_BARRIER
    if (barrierId == 0)
        return cmrt::simple_group_barrier_wait();
#endif
    cmrt::group_barrier_wait(barrierId);
}

CM_API void cm_barrier()
{
#if !defined(CM_EMU_USE_SIMPLE_BARRIER)
    cmrt::assert_cm_nbarrier_init_api_usage ();
    cmrt::group_barrier_signal();
    cmrt::group_barrier_wait();
    return;
#endif

    cmrt::simple_group_barrier_signal();
    cmrt::simple_group_barrier_wait();
}

CM_API void cm_sbarrier(uint flag)
{
#if !defined(CM_EMU_USE_SIMPLE_BARRIER)
    cmrt::assert_cm_nbarrier_init_api_usage ();
    flag ? cmrt::group_barrier_signal() :
           cmrt::group_barrier_wait();
    return;
#endif

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

CM_API cmrt::XThreadBroadcastBuf& __cm_dpasw_xthread_broadcast(){
    return cmrt::get_xthread_broadcast();
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
