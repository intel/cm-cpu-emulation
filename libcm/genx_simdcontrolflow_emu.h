/*========================== begin_copyright_notice ============================

Copyright (C) 2017 Intel Corporation

SPDX-License-Identifier: MIT

============================= end_copyright_notice ===========================*/

#ifndef GENX_SIMDCONTROLFLOW_EMU_H
#define GENX_SIMDCONTROLFLOW_EMU_H

#define GENX_EMU_MACROS
#ifdef GENX_EMU_MACROS

#include "libcm_def.h"

//-----------------------------------------------------------------------------
// SIMD "if" definitions
//-----------------------------------------------------------------------------

#define SIMD_IF_BEGIN(...)                                          \
    __CMInternal__::setSIMDMarker(                                  \
        __CMInternal__::__cm_internal_simd_if_begin(__VA_ARGS__));  \
    if ((__CMInternal__::__cm_internal_simd())) {

#define SIMD_ELSE                                       \
    }                                                   \
    __CMInternal__::setSIMDMarker(                      \
        __CMInternal__::__cm_internal_simd_then_end()); \
    if ((__CMInternal__::__cm_internal_simd())) {

#define SIMD_ELSEIF(...)                                                \
    }                                                                   \
    __CMInternal__::setSIMDMarker(                                      \
        __CMInternal__::__cm_internal_simd_then_end());                 \
    __CMInternal__::setSIMDMarker(                                      \
        __CMInternal__::__cm_internal_simd_elseif_begin(__VA_ARGS__));  \
    if ((__CMInternal__::__cm_internal_simd())) {

#define SIMD_IF_END                                     \
    }                                                   \
    __CMInternal__::setSIMDMarker(                      \
        __CMInternal__::__cm_internal_simd_if_end());   \
    __CMInternal__::setSIMDMarker(                      \
        __CMInternal__::__cm_internal_simd_if_join());

//-----------------------------------------------------------------------------
// SIMD "do-while" definitions
//-----------------------------------------------------------------------------

#define SIMD_DO_WHILE_BEGIN                                         \
    __CMInternal__::__cm_internal_simd_do_while_before();           \
    do {                                                            \
        __CMInternal__::setSIMDMarker(                              \
            __CMInternal__::__cm_internal_simd_do_while_begin());

#define SIMD_DO_WHILE_END(...)                                          \
    __CMInternal__::setSIMDMarker(                                      \
        __CMInternal__::__cm_internal_before_do_while_end());           \
    __CMInternal__::setSIMDMarker(                                      \
        __CMInternal__::__cm_internal_simd_do_while_end(__VA_ARGS__));  \
    }                                                                   \
    while ((__CMInternal__::__cm_internal_simd(__VA_ARGS__)));          \
    __CMInternal__::setSIMDMarker(                                      \
        __CMInternal__::__cm_internal_simd_after_do_while_end());

//-----------------------------------------------------------------------------
// SIMD jump definitions
//-----------------------------------------------------------------------------

#define SIMD_BREAK                                      \
    __CMInternal__::setSIMDMarker(                      \
        __CMInternal__::__cm_internal_simd_break());

#define SIMD_CONTINUE                                   \
    __CMInternal__::setSIMDMarker(                      \
        __CMInternal__::__cm_internal_simd_continue());

#else
//-----------------------------------------------------------------------------
// SIMD "if" definitions
//-----------------------------------------------------------------------------

#define SIMD_IF_BEGIN(cond)                     \
    if (cond)

#define SIMD_ELSE                               \
    else

#define SIMD_IF_END

//-----------------------------------------------------------------------------
// SIMD "do-while" definitions
//-----------------------------------------------------------------------------

#define SIMD_DO_WHILE_BEGIN                     \
    do

#define SIMD_DO_WHILE_END(cond)                 \
    while (cond)

//-----------------------------------------------------------------------------
// SIMD jump definitions
//-----------------------------------------------------------------------------

#define SIMD_BREAK                              \
    break

#define SIMD_CONTINUE                           \
    continue

#endif

#endif /* GENX_SIMDCONTROLFLOW_EMU_H */
