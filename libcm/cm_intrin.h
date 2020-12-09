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

#ifndef CM_INTRIN_H
#define CM_INTRIN_H

#include <stdio.h>
#include <math.h>
#include <mutex>

#include "rt.h"
#include "cm_common_macros.h"

#include "genx_dataport.h"

/* Some extras for float rounding support */
#include <fenv.h>

#include "half_type.h"

//------------------------------------------------------------------------------
// SLM-related.
//------------------------------------------------------------------------------
CM_API void cm_slm_init(unsigned int);
CM_API unsigned int cm_slm_alloc(unsigned int);
CM_API void cm_slm_free();
CM_API char* __cm_emu_get_slm();
CM_API size_t __cm_emu_get_slm_size();
CM_API void __cm_emu_aux_barrier();

//------------------------------------------------------------------------------
CM_API uint cm_local_id (uint dim); // Local Thread IDs in current group
CM_API uint cm_local_size(uint dim); // Number of threads per group
CM_API uint cm_group_id(uint dim); // Group IDs
CM_API uint cm_group_count(uint dim); // Number of groups

//------------------------------------------------------------------------------
// Linearization functions for IDs and SIZEs:
//------------------------------------------------------------------------------

// Returns the linear thread ID in a group
CM_API inline uint cm_linear_local_id (void)
{
    return cm_local_id(0) + cm_local_size(0) * (cm_local_id(1) + cm_local_size(1) * cm_local_id(2));
}

// Returns the linear group ID
CM_API inline uint cm_linear_group_id (void)
{
    return cm_group_id(0) + cm_group_count(0) * (cm_group_id(1) + cm_group_count(1) * cm_group_id(2));
}

// Returns the total number of threads per group
CM_API inline uint cm_linear_local_size (void)
{
    return cm_local_size(0) * cm_local_size(1) * cm_local_size(2);
}

// Returns the linear global ID of a thread
CM_API inline uint cm_linear_global_id (void)
{
    return cm_linear_group_id() * cm_linear_local_size() + cm_linear_local_id();
}

// Returns the total number of groups
CM_API inline uint cm_linear_group_count (void)
{
    return cm_group_count(0) * cm_group_count(1);
}

// Returns the total number of threads
CM_API inline uint cm_linear_global_size (void)
{
    return cm_linear_group_count() * cm_linear_local_size();
}

/* BF<->FLOAT */
template<typename RT, typename T, uint SZ>
CM_API vector<RT, SZ>
cm_bf_cvt(const stream<T, SZ>& src0)
{
    int i;
    typename abstype<T>::type  ret;
    vector<RT, SZ> retv;
    static const bool conformable1 = is_fp_type<RT>::value;
    static const bool conformable2 = is_hf_type<RT>::value;
    if (is_fp_type<T>::value)
    {
        if (conformable2)
        {
            //CM_STATIC_ERROR(conformable2, "only fp -> bf (reprented as hf) conversion is supported");
            for (i = 0; i < SZ; i++) {
                SIMDCF_ELEMENT_SKIP(i);
                float tmp = src0.get(i);
                ushort* p = (ushort *) &tmp;
                half ret_tmp;
                ushort* pret_tmp = (ushort *) &ret_tmp;
                pret_tmp[0] = p[1];
                retv(i) = ret_tmp;
            }
        }
        else
        {
            fprintf(stderr, "only fp -> bf (reprented as hf) conversion is supported\n");
            exit(EXIT_FAILURE);
        }
    }
    else if (is_hf_type<T>::value)
    {
        if (conformable1)
        {
            //CM_STATIC_ERROR(conformable1, "only bf -> float (reprented as hf) conversion is supported");
            for (i = 0; i < SZ; i++) {
                SIMDCF_ELEMENT_SKIP(i);
                half tmp = (half)src0.get(i);
                ushort* p = (ushort *)&tmp;
                float ret_tmp;
                ushort* pret_tmp = (ushort *)&ret_tmp;
                pret_tmp[0] = 0;
                pret_tmp[1] = p[0];
                retv(i) = ret_tmp;
            }
        }
        else
        {
            fprintf(stderr, "only bf -> float (reprented as hf) conversion is supported\n");
            exit(EXIT_FAILURE);
        }
    }
    else
    {
        fprintf(stderr, "unsupported data type.\n");
        exit(EXIT_FAILURE);
    }

    return retv;
}

/* Abs */
template<typename RT ,typename T, uint SZ>
CM_API vector<RT, SZ>
cm_abs(const stream<T,SZ>& src0, const uint flags = 0)
{
    int i;
    typename abstype<T>::type  ret;
    vector<RT, SZ> retv;

    for (i = 0; i < SZ; i++) {
        SIMDCF_ELEMENT_SKIP(i);
        if (src0.get(i) < 0) {
            ret = -(src0.get(i));
        } else {
            ret = (src0.get(i));
        }
        retv(i) = CmEmulSys::satur<RT>::saturate(ret, flags);
    }

    return retv;
}

template<typename RT,typename T>
CM_API RT
cm_abs(const T& src0, const typename uint_type<T, T>::type flags = 0)
{
    vector<T, 1> v(src0);
    vector<RT, 1> ret;
    ret = cm_abs<RT>(v, flags);
    return ret(0);
}

/* Max */
template<typename RT, typename T1, typename T2, uint SZ>
CM_API vector<RT, SZ>
cm_max(const stream<T1,SZ>& src0, const stream<T2,SZ>& src1, const uint flags = 0)
{
    int i;
    typename restype_ex<T1,T2>::type ret;
    vector<RT, SZ> retv;

    for (i = 0; i < SZ; i++) {
         SIMDCF_ELEMENT_SKIP(i);
        if (src0.get(i) >= src1.get(i)) {
            ret = src0.get(i);
        }
        else {
            ret = src1.get(i);
        }
        retv(i) = CmEmulSys::satur<RT>::saturate(ret, flags);
    }

    return retv;
}

template<typename RT, typename T1, typename T2, uint SZ>
CM_API vector<RT, SZ>
cm_max(const stream<T1,SZ>& src0, const T2 src1,
       const typename uint_type<T1, T2>::type flags = 0)
{
    vector<T1, SZ> v1;
    vector<T2, SZ> v2(src1);
    vector<RT, SZ> retv;
    int i;

    for (i = 0; i < SZ; i++) {
         SIMDCF_ELEMENT_SKIP(i);
        v1(i) = src0.get(i);
    }

    retv = cm_max<RT>(v1, v2, flags);

    return retv;
}

template<typename RT, typename T1, typename T2, uint SZ>
CM_API vector<RT, SZ>
cm_max(const T1 src0, const stream<T2,SZ>& src1,
       const typename uint_type<T1, T2>::type flags = 0)
{
    vector<T1, SZ> v1(src0);
    vector<T2, SZ> v2;
    vector<RT, SZ> retv;
    int i;

    for (i = 0; i < SZ; i++) {
         SIMDCF_ELEMENT_SKIP(i);
        v2(i) = src1.get(i);
    }

    retv = cm_max<RT>(v1, v2, flags);

    return retv;
}

template <typename RT, typename T1, typename T2>
CM_API RT
cm_max(const T1& src0, const T2& src1,
       const typename uint_type<T1, T2>::type flags = 0)
{
    vector<T1, 1> v1(src0);
    vector<T2, 1> v2(src1);
    vector<RT, 1> retv;

    retv = cm_max<RT>(v1, v2, flags);

    return retv(0);
}

/* Min */
template<typename RT, typename T1, typename T2, uint SZ>
CM_API vector<RT, SZ>
cm_min(const stream<T1,SZ>& src0, const stream<T2,SZ>& src1, const uint flags = 0)
{
    int i;
    RT ret;
    vector<RT, SZ> retv;

    for (i = 0; i < SZ; i++) {
         SIMDCF_ELEMENT_SKIP(i);
        if (src0.get(i) < src1.get(i)) {
            ret = RT(src0.get(i));
        }
        else {
            ret = RT(src1.get(i));
        }
        retv(i) = CmEmulSys::satur<RT>::saturate(ret, flags);
    }

    return retv;
}

template<typename RT, typename T1, typename T2, uint SZ>
CM_API vector<RT, SZ>
cm_min(const stream<T1,SZ>& src0, const T2 src1,
       const typename uint_type<T1, T2>::type flags = 0)
{
    vector<T1, SZ> v1;
    vector<T2, SZ> v2(src1);
    vector<RT, SZ> retv;
    int i;

    for (i = 0; i < SZ; i++) {
         SIMDCF_ELEMENT_SKIP(i);
        v1(i) = src0.get(i);
    }

    retv = cm_min<RT>(v1, v2, flags);

    return retv;
}

template<typename RT, typename T1, typename T2, uint SZ>
CM_API vector<RT, SZ>
cm_min(const T1 src0, const stream<T2,SZ>& src1,
       const typename uint_type<T1, T2>::type flags = 0)
{
    vector<T1, SZ> v1(src0);
    vector<T2, SZ> v2;
    vector<RT, SZ> retv;
    int i;

    for (i = 0; i < SZ; i++) {
         SIMDCF_ELEMENT_SKIP(i);
        v2(i) = src1.get(i);
    }

    retv = cm_min<RT>(v1, v2, flags);

    return retv;
}

template <typename RT, typename T1, typename T2>
CM_API RT
cm_min(const T1& src0, const T2& src1,
       const typename uint_type<T1, T2>::type flags = 0)
{
    vector<T1, 1> v1(src0);
    vector<T2, 1> v2(src1);
    vector<RT, 1> retv;

    retv = cm_min<RT>(v1, v2, flags);

    return retv(0);
}

//************************ Arithmetics ****************************
//CM QUOT
template <typename T, uint SZ>
CM_API vector<T, SZ>
cm_quot(const stream<T, SZ> &src0, const stream<T, SZ> &src1,
        const uint flags = 0)
{
    static const bool conformable1 = dwordtype<T>::value;
    int i;
    vector<T,SZ> retv;

    for (i = 0; i < SZ; i++) {
         SIMDCF_ELEMENT_SKIP(i);
        retv(i) = src0.get(i) / src1.get(i);
    }

    return retv;
}

template <typename T, uint SZ>
CM_API vector<T, SZ>
cm_quot(const stream<T, SZ> &src0, const T &src1,
        const typename uint_type<T, T>::type flags = 0)
{
    static const bool conformable1 = dwordtype<T>::value;
    int i;
    vector<T,SZ> retv;

    for (i = 0; i < SZ; i++) {
         SIMDCF_ELEMENT_SKIP(i);
        retv(i) = src0.get(i) / src1;
    }

    return retv;
}

template <typename T, uint SZ>
CM_API vector<T, SZ>
cm_quot(const T &src0, const stream<T, SZ> &src1,
        const typename uint_type<T, T>::type flags = 0)
{
    static const bool conformable1 = dwordtype<T>::value;
    int i;
    vector<T,SZ> retv;

    for (i = 0; i < SZ; i++) {
         SIMDCF_ELEMENT_SKIP(i);
        retv(i) = src0 / src1.get(i);
    }

    return retv;
}


template <typename T>
CM_API T
cm_quot(const T &src0, const T &src1,
        const typename uint_type<T, T>::type flags = 0)
{
    static const bool conformable1 = dwordtype<T>::value;
    T ret;

    ret = src0 / src1;

    return ret;
}

//CM MOD
template <typename T, uint SZ>
CM_API vector<T, SZ>
cm_mod(const stream<T, SZ> &src0, const stream<T, SZ> &src1,
        const uint flags = 0)
{
    static const bool conformable1 = dwordtype<T>::value;
    int i;
    vector<T,SZ> retv;

    for (i = 0; i < SZ; i++) {
         SIMDCF_ELEMENT_SKIP(i);
        retv(i) = src0.get(i) % src1.get(i);
    }

    return retv;
}

template <typename T, uint SZ>
CM_API vector<T, SZ>
cm_mod(const stream<T, SZ> &src0, const T &src1,
        const typename uint_type<T, T>::type flags = 0)
{
    static const bool conformable1 = dwordtype<T>::value;
    int i;
    vector<T,SZ> retv;

    for (i = 0; i < SZ; i++) {
         SIMDCF_ELEMENT_SKIP(i);
        retv(i) = src0.get(i) % src1;
    }

    return retv;
}

template <typename T, uint SZ>
CM_API vector<T, SZ>
cm_mod(const T &src0, const stream<T, SZ> &src1,
        const typename uint_type<T, T>::type flags = 0)
{
    static const bool conformable1 = dwordtype<T>::value;
    int i;
    vector<T,SZ> retv;

    for (i = 0; i < SZ; i++) {
         SIMDCF_ELEMENT_SKIP(i);
        retv(i) = src0 % src1.get(i);
    }

    return retv;
}


template <typename T>
CM_API T
cm_mod(const T &src0, const T &src1,
        const typename uint_type<T, T>::type flags = 0)
{
    static const bool conformable1 = dwordtype<T>::value;
    T ret;

    ret = src0 % src1;

    return ret;
}

//CM DIV
template <typename T, uint R, uint C>
CM_API vector<T, R * C>
cm_div(matrix<T, R, C> &rmd, const stream<T, R * C> &src0,
       const stream<T, R * C> &src1, const uint flags = 0)
{
    static const bool conformable1 = dwordtype<T>::value;
    int i;
    vector<T,R * C> retv;

    for (i = 0; i < R * C; i++) {
         SIMDCF_ELEMENT_SKIP(i);
        retv(i) = src0.get(i) / src1.get(i);
        rmd(i / C, i % C) = src0.get(i) % src1.get(i);
    }

    return retv;
}

template <typename T, uint R, uint C>
CM_API vector<T, R * C>
cm_div(matrix_ref<T, R, C> rmd, const stream<T, R * C> &src0,
       const stream<T, R * C> &src1,  const uint flags = 0)
{
    static const bool conformable1 = dwordtype<T>::value;
    int i;
    vector<T,R * C> retv;

    for (i = 0; i < R * C; i++) {
         SIMDCF_ELEMENT_SKIP(i);
        retv(i) = src0.get(i) / src1.get(i);
        rmd(i / C, i % C) = src0.get(i) % src1.get(i);
    }

    return retv;
}

template <typename T, uint R, uint C>
CM_API vector<T, R * C>
cm_div(matrix<T, R, C> &rmd, const stream<T, R * C> &src0, const T &src1,
        const typename uint_type<T, T>::type flags = 0)
{
    static const bool conformable1 = dwordtype<T>::value;
    int i;
    vector<T,R * C> retv;

    for (i = 0; i < R * C; i++) {
         SIMDCF_ELEMENT_SKIP(i);
        retv(i) = src0.get(i) / src1;
        rmd(i / C, i % C) = src0.get(i) % src1;
    }

    return retv;
}

template <typename T, uint R, uint C>
CM_API vector<T, R * C>
cm_div(matrix_ref<T, R, C> rmd, const stream<T, R * C> &src0, const T &src1,
       const typename uint_type<T, T>::type flags = 0)
{
    static const bool conformable1 = dwordtype<T>::value;
    int i;
    vector<T,R * C> retv;

    for (i = 0; i < R * C; i++) {
         SIMDCF_ELEMENT_SKIP(i);
        retv(i) = src0.get(i) / src1;
        rmd(i / C, i % C) = src0.get(i) % src1;
    }

    return retv;
}

template <typename T, uint R, uint C>
CM_API vector<T, R * C>
cm_div(matrix<T, R, C> &rmd, const T &src0, const stream<T, R * C> &src1,
        const typename uint_type<T, T>::type flags = 0)
{
    static const bool conformable1 = dwordtype<T>::value;
    int i;
    vector<T,R * C> retv;

    for (i = 0; i < R * C; i++) {
         SIMDCF_ELEMENT_SKIP(i);
        retv(i) = src0 / src1.get(i);
        rmd(i / C, i % C) = src0 % src1.get(i);
    }

    return retv;
}

template <typename T, uint R, uint C>
CM_API vector<T, R * C>
cm_div(matrix_ref<T, R, C> rmd, const T &src0, const stream<T, R * C> &src1,
        const typename uint_type<T, T>::type flags = 0)
{
    static const bool conformable1 = dwordtype<T>::value;
    int i;
    vector<T,R * C> retv;

    for (i = 0; i < R * C; i++) {
         SIMDCF_ELEMENT_SKIP(i);
        retv(i) = src0 / src1.get(i);
        rmd(i / C, i % C) = src0 % src1.get(i);
    }

    return retv;
}


template <typename T>
CM_API T
cm_div(T &rmd, const T &src0, const T &src1,
        const typename uint_type<T, T>::type flags = 0)
{
    static const bool conformable1 = dwordtype<T>::value;
    T ret;

    ret = src0 / src1;
    rmd = src0 % src1;

    return ret;
}

//CM IMUL
template <typename T1, typename T2, typename T3, uint R, uint C>
CM_API vector<T3, R * C>
cm_imul(matrix<T3, R, C> &rmd, const stream<T1, R * C> &src0,
       const stream<T2, R * C> &src1,  const uint flags = 0)
{
    static const bool conformable1 = true;
    bool unsignedOpnd1 = check_true<(unsignedtype <T1>::value)>::value;
    bool unsignedOpnd2 = check_true<(unsignedtype <T2>::value)>::value;

    bool unsignedOpnd = (unsignedOpnd1 && unsignedOpnd2);
    int i;
    vector<T3,R * C> retv;

    if( unsignedOpnd ){
        for (i = 0; i < R * C; i++) {
            unsigned long long temp;
                       SIMDCF_ELEMENT_SKIP(i);
            temp = (long long)src0.get(i) * (long long)src1.get(i);
            retv(i) = temp >> 32;
            rmd(i / C, i % C) = temp & 0xFFFFFFFF;
        }
    }else{
        for (i = 0; i < R * C; i++) {
            long long temp;
                     SIMDCF_ELEMENT_SKIP(i);
            temp = (long long)src0.get(i) * (long long)src1.get(i);
            retv(i) = temp >> 32;
            rmd(i / C, i % C) = temp & 0xFFFFFFFF;
        }
    }

    return retv;
}

template <typename T1, typename T2, typename T3, uint R, uint C>
CM_API vector<T3, R * C>
cm_imul(matrix_ref<T3, R, C> rmd, const stream<T1, R * C> &src0,
       const stream<T2, R * C> &src1,  const uint flags = 0)
{
    static const bool conformable1 = true;
    bool unsignedOpnd1 = check_true<(unsignedtype <T1>::value)>::value;
    bool unsignedOpnd2 = check_true<(unsignedtype <T2>::value)>::value;

    bool unsignedOpnd = (unsignedOpnd1 && unsignedOpnd2);
    int i;
    vector<T3,R * C> retv;

    if( unsignedOpnd ){
        for (i = 0; i < R * C; i++) {
            unsigned long long temp;
                     SIMDCF_ELEMENT_SKIP(i);
            temp = (long long)src0.get(i) * (long long)src1.get(i);
            retv(i) = temp >> 32;
            rmd(i / C, i % C) = temp & 0xFFFFFFFF;
        }
    }else{
        for (i = 0; i < R * C; i++) {
            long long temp;
                     SIMDCF_ELEMENT_SKIP(i);
            temp = (long long)src0.get(i) * (long long)src1.get(i);
            retv(i) = temp >> 32;
            rmd(i / C, i % C) = temp & 0xFFFFFFFF;
        }
    }
    return retv;
}

template <typename T1, typename T2, typename T3, uint R, uint C>
CM_API vector<T3, R * C>
cm_imul(matrix<T3, R, C> &rmd, const T1 &src0,
       const stream<T2, R * C> &src1, const typename uint_type<T1, T2>::type flags = 0)
{
    static const bool conformable1 = true;
    bool unsignedOpnd1 = check_true<(unsignedtype <T1>::value)>::value;
    bool unsignedOpnd2 = check_true<(unsignedtype <T2>::value)>::value;

    bool unsignedOpnd = (unsignedOpnd1 && unsignedOpnd2);
    int i;
    vector<int,R * C> retv;

    if( unsignedOpnd ){
        for (i = 0; i < R * C; i++) {
            unsigned long long temp;
                     SIMDCF_ELEMENT_SKIP(i);
            temp = (long long)src0 * (long long)src1.get(i);
            retv(i) = temp >> 32;
            rmd(i / C, i % C) = temp & 0xFFFFFFFF;
        }
    }else{
        for (i = 0; i < R * C; i++) {
            unsigned long long temp;
                     SIMDCF_ELEMENT_SKIP(i);
            temp = (long long)src0 * (long long)src1.get(i);
            retv(i) = temp >> 32;
            rmd(i / C, i % C) = temp & 0xFFFFFFFF;
        }
    }
    return retv;
}

template <typename T1, typename T2, typename T3, uint R, uint C>
CM_API vector<T3, R * C>
cm_imul(matrix_ref<T3, R, C> rmd, const T1 &src0,
       const stream<T2, R * C> &src1, const typename uint_type<T1, T2>::type  flags = 0)
{
    static const bool conformable1 = true;
    bool unsignedOpnd1 = check_true<(unsignedtype <T1>::value)>::value;
    bool unsignedOpnd2 = check_true<(unsignedtype <T2>::value)>::value;

    bool unsignedOpnd = (unsignedOpnd1 && unsignedOpnd2);
    int i;
    vector<T3,R * C> retv;

    if( unsignedOpnd ){
        for (i = 0; i < R * C; i++) {
            unsigned long long temp;
                     SIMDCF_ELEMENT_SKIP(i);
            temp = (long long)src0 * (long long)src1.get(i);
            retv(i) = temp >> 32;
            rmd(i / C, i % C) = temp & 0xFFFFFFFF;
        }
    }else{
        for (i = 0; i < R * C; i++) {
            unsigned long long temp;
                     SIMDCF_ELEMENT_SKIP(i);
            temp = (long long)src0 * (long long)src1.get(i);
            retv(i) = temp >> 32;
            rmd(i / C, i % C) = temp & 0xFFFFFFFF;
        }
    }
    return retv;
}

template <typename T1, typename T2, typename T3>
CM_API T3
cm_imul(T3 &rmd, const T1 &src0,
       const T2 &src1, const typename uint_type<T1, T2>::type  flags = 0)
{
    static const bool conformable1 = true;
    bool unsignedOpnd1 = check_true<(unsignedtype <T1>::value)>::value;
    bool unsignedOpnd2 = check_true<(unsignedtype <T2>::value)>::value;

    bool unsignedOpnd = (unsignedOpnd1 && unsignedOpnd2);
    int i;
    T3 retv;

    if( unsignedOpnd ){
        unsigned long long temp;
        temp = (long long)src0 * (long long)src1;
        retv = temp >> 32;
        rmd = temp & 0xFFFFFFFF;
    }else{
        long long temp;
        temp = (long long)src0 * (long long)src1;
        retv = temp >> 32;
        rmd = temp & 0xFFFFFFFF;
    }
    return retv;
}


template <typename T1, typename T2, typename T3, uint R, uint C>
CM_API vector<T3, R * C>
cm_imul(matrix<T3, R, C> &rmd, const stream<T1, R * C> &src0,
       const T2 &src1, const typename uint_type<T1, T2>::type  flags = 0)
{
    static const bool conformable1 = true;
    bool unsignedOpnd1 = check_true<(unsignedtype <T1>::value)>::value;
    bool unsignedOpnd2 = check_true<(unsignedtype <T2>::value)>::value;

    bool unsignedOpnd = (unsignedOpnd1 && unsignedOpnd2);
    int i;
    vector<T3,R * C> retv;

    if( unsignedOpnd ){
        for (i = 0; i < R * C; i++) {
            unsigned long long temp;
                     SIMDCF_ELEMENT_SKIP(i);
            temp = (long long)src0.get(i) * (long long)src1;
            retv(i) = temp >> 32;
            rmd(i / C, i % C) = temp & 0xFFFFFFFF;
        }
    }else{
        for (i = 0; i < R * C; i++) {
            unsigned long long temp;
                     SIMDCF_ELEMENT_SKIP(i);
            temp = (long long)src0.get(i) * (long long)src1;
            retv(i) = temp >> 32;
            rmd(i / C, i % C) = temp & 0xFFFFFFFF;
        }
    }
    return retv;
}

template <typename T1, typename T2, typename T3, uint R, uint C>
CM_API vector<T3, R * C>
cm_imul(matrix_ref<T3, R, C> rmd, const stream<T1, R * C> &src0,
       const T2 &src1, const typename uint_type<T1, T2>::type flags = 0)
{
    static const bool conformable1 = true;
    bool unsignedOpnd1 = check_true<(unsignedtype <T1>::value)>::value;
    bool unsignedOpnd2 = check_true<(unsignedtype <T2>::value)>::value;

    bool unsignedOpnd = (unsignedOpnd1 && unsignedOpnd2);
    vector<T3,R * C> retv;
    int i;
    if( unsignedOpnd ){
        for (i = 0; i < R * C; i++) {
            unsigned long long temp;
                     SIMDCF_ELEMENT_SKIP(i);
            temp = (long long)src0.get(i) * (long long)src1;
            retv(i) = temp >> 32;
            rmd(i / C, i % C) = temp & 0xFFFFFFFF;
        }
    }else{
        for (i = 0; i < R * C; i++) {
            unsigned long long temp;
                     SIMDCF_ELEMENT_SKIP(i);
            temp = (long long)src0.get(i) * (long long)src1;
            retv(i) = temp >> 32;
            rmd(i / C, i % C) = temp & 0xFFFFFFFF;
        }
    }
    return retv;
}

//Add
template <typename RT, typename T1, typename T2, uint SZ>
CM_API vector<RT, SZ>
cm_add(const stream<T1,SZ>& src0, const stream<T2,SZ>& src1,
       const uint flags = 0)
{
    static const bool conformable =
        check_true<!is_float_dword<T1, T2>::value>::value;

    static const bool conformable1 =
        check_true<!(dftype<T1>::value || dftype<T2>::value || dftype<RT>::value) ||
                   (dftype<T1>::value && dftype<T2>::value && dftype<RT>::value)>::value;

    int i;
    typename restype_ex<T1,T2>::type ret;
    vector<RT, SZ> retv;

    uint sat1 = CmEmulSys::_SetSatur<T1, is_inttype<RT>::value>::SetSatur();

    for (i = 0; i < SZ; i++) {
        if(flags | sat1) {
            ret = (typename restype_ex<T1,T2>::type) src0.get(i) + (typename restype_ex<T1,T2>::type) src1.get(i);
        } else {
            ret = src0.get(i) + src1.get(i);
        }
        retv(i) = CmEmulSys::satur<RT>::saturate(ret, flags | sat1);
    }
    return retv;
}

template <typename RT, typename T1, typename T2, uint SZ>
CM_API vector<RT, SZ>
cm_add(const stream<T1,SZ>& src0, const T2& src1,
       const typename uint_type<T1, T2>::type flags = 0)
{
    vector<T1, SZ> v1;
    vector<T2, SZ> v2(src1);
    vector<RT, SZ> retv;
    int i;

    for (i = 0; i < SZ; i++) {
        v1(i) = src0.get(i);
    }

    retv = cm_add<RT>(v1, v2, flags);

    return retv;
}

template <typename RT, typename T1, typename T2>
CM_API RT
cm_add(const T1& src0, const T2& src1,
       const typename uint_type<T1, T2>::type flags = 0)
{
    vector<T1, 1> v1(src0);
    vector<T2, 1> v2(src1);
    vector<RT, 1> retv;

    v1(0) = src0;
    v2(0) = src1;
    retv = cm_add<RT>(v1, v2, flags);
    return retv(0);
}

//Mul
template <typename RT, typename T1, typename T2, uint SZ>
CM_API vector<RT, SZ>
cm_mul(const stream<T1,SZ>& src0, const stream<T2,SZ>& src1,
       const uint flags = 0)
{
    static const bool conformable =
        check_true<!is_float_dword<T1, T2>::value>::value;

    static const bool conformable1 =
        check_true<!((is_dword_type<T1>::value | is_dword_type<T2>::value) &&
        is_fp_type<RT>::value)>::value;

    static const bool conformable2 =
        check_true<!(dftype<T1>::value || dftype<T2>::value || dftype<RT>::value) ||
                   (dftype<T1>::value && dftype<T2>::value && dftype<RT>::value)>::value;

    int i;
    typename restype_sat<T1, T2>::type ret;
    vector<RT, SZ> retv;

    uint sat1 = CmEmulSys::_SetSatur<T1, is_inttype<RT>::value>::SetSatur();
    for (i = 0; i < SZ; i++) {
        SIMDCF_ELEMENT_SKIP(i);
        if(flags | sat1) {
            ret = ((typename restype_sat<T1, T2>::type) src0.get(i)) * ((typename restype_sat<T1, T2>::type) src1.get(i));
        } else {
            ret = src0.get(i) * src1.get(i);
        }
        retv(i) = CmEmulSys::satur<RT>::saturate(ret, flags | sat1);
    }

    return retv;
}

template <typename RT, typename T1, typename T2, uint SZ>
CM_API vector<RT, SZ>
cm_mul(const stream<T1,SZ>& src0, const T2& src1,
       const typename uint_type<T1, T2>::type flags = 0)
{
    vector<T1, SZ> v1;
    vector<T2, SZ> v2(src1);
    vector<RT, SZ> retv;
    int i;

    for (i = 0; i < SZ; i++) {
        v1(i) = src0.get(i);
    }

    retv = cm_mul<RT>(v1, v2, flags);

    return retv;
}

template <typename RT, typename T1, typename T2>
CM_API RT
cm_mul(const T1& src0, const T2& src1,
       const typename uint_type<T1, T2>::type flags = 0)
{
    vector<T1, 1> v1(src0);
    vector<T2, 1> v2(src1);
    vector<RT, 1> retv;

    retv = cm_mul<RT>(v1, v2, flags);

    return retv(0);
}

//*************** General Purpose instructions with SAT *************

/* Average */
template <typename RT, typename T1, typename T2, uint SZ>
CM_API vector<RT, SZ>
cm_avg(const stream<T1,SZ>& src0, const stream<T2,SZ>& src1,
       const uint flags = 0)
{
    static const bool conformable1 = inttype<T1>::value;
    static const bool conformable2 = inttype<T2>::value;
    static const bool conformable3 = inttype<RT>::value;

    int i;
    typename restype_ex<T1,T2>::type ret;
    vector<RT, SZ> retv;

    uint sat1 = CmEmulSys::_SetSatur<T1, is_inttype<RT>::value>::SetSatur();
    for (i = 0; i < SZ; i++) {
        SIMDCF_ELEMENT_SKIP(i);
        ret = (src0.get(i) + src1.get(i) + 1) >> 1;
        retv(i) = CmEmulSys::satur<RT>::saturate(ret, flags | sat1);
    }

    return retv;
}

template <typename RT, typename T1, typename T2, uint SZ>
CM_API vector<RT, SZ>
cm_avg(const stream<T1,SZ>& src0, const T2& src1,
       const typename uint_type<T1, T2>::type flags = 0)
{
    vector<T1, SZ> v1;
    vector<T2, SZ> v2(src1);
    vector<RT, SZ> retv;
    int i;

    for (i = 0; i < SZ; i++) {
        SIMDCF_ELEMENT_SKIP(i);
        v1(i) = src0.get(i);
    }

    retv = cm_avg<RT>(v1, v2, flags);

    return retv;
}

template <typename RT, typename T1, typename T2>
CM_API RT
cm_avg(const T1& src0, const T2& src1,
       const typename uint_type<T1, T2>::type flags = 0)
{
    vector<T1, 1> v1(src0);
    vector<T2, 1> v2(src1);
    vector<RT, 1> retv;

    retv = cm_avg<RT>(v1, v2, flags);

    return retv(0);
}

//Additional explanation from CG is needed
//Dot products
template <typename RT, typename T1, typename T2, uint SZ>
CM_API vector<RT, SZ>
cm_dp2(const stream<T1,SZ>& src0, const stream<T2,SZ>& src1,
       const uint flags = 0)
{
//    static const bool conformable1 = fptype<T1>::value;
//    static const bool conformable2 = fptype<T2>::value;
//    static const bool conformable2 = fptype<T2>::value;
    static const bool conformable4 = check_true<!(SZ%4)>::value;
    static const bool conformable5 =
        check_true<!(is_inttype<T1>::value || is_inttype<T2>::value)>::value;

    int i;
    typename restype_ex<T1,T2>::type ret;
    vector<RT, SZ> retv;

    uint sat1 = CmEmulSys::_SetSatur<T1, is_inttype<RT>::value>::SetSatur();
    for (i = 0; i < SZ; i += 4) {
        SIMDCF_ELEMENT_SKIP(i);
        ret = src0.get(i) * src1.get(i) + src0.get(i + 1) * src1.get(i + 1);
        retv(i) = retv(i + 1) = retv(i + 2) = retv(i + 3) = CmEmulSys::satur<RT>::saturate(ret, flags | sat1);
    }

    return retv;
}

template <typename RT, typename T1, typename T2, uint SZ>
CM_API vector<RT, SZ>
cm_dp2(const stream<T1,SZ>& src0, const T2& src1,
       const typename uint_type<T1, T2>::type flags = 0)
{
    vector<T1, SZ> v1;
    vector<T2, SZ> v2(src1);
    vector<RT, SZ> retv;
    int i;

    for (i = 0; i < SZ; i++) {
        v1(i) = src0.get(i);
    }

    retv = cm_dp2<RT>(v1, v2, flags);

    return retv;
}

template <typename RT, typename T1, typename T2, uint SZ>
CM_API vector<RT, SZ>
cm_dp3(const stream<T1,SZ>& src0, const stream<T2,SZ>& src1,
       const uint flags = 0)
{
    static const bool conformable4 = check_true<!(SZ%4)>::value;
    static const bool conformable5 =
        check_true<!(is_inttype<T1>::value || is_inttype<T2>::value)>::value;

    int i;
    typename restype_ex<T1,T2>::type ret;
    vector<RT, SZ> retv;

    uint sat1 = CmEmulSys::_SetSatur<T1, is_inttype<RT>::value>::SetSatur();
    for (i = 0; i < SZ; i += 4) {
        SIMDCF_ELEMENT_SKIP(i);
        ret = src0.get(i) * src1.get(i) + src0.get(i + 1) * src1.get(i + 1) + src0.get(i + 2) * src1.get(i + 2);
        retv(i) = retv(i + 1) = retv(i + 2) = retv(i + 3) = CmEmulSys::satur<RT>::saturate(ret, flags | sat1);
    }

    return retv;
}

template <typename RT, typename T1, typename T2, uint SZ>
CM_API vector<RT, SZ>
cm_dp3(const stream<T1,SZ>& src0, const T2& src1,
       const typename uint_type<T1, T2>::type flags = 0)
{
    vector<T1, SZ> v1;
    vector<T2, SZ> v2(src1);
    vector<RT, SZ> retv;
    int i;

    for (i = 0; i < SZ; i++) {
        v1(i) = src0.get(i);
    }

    retv = cm_dp3<RT>(v1, v2, flags);

    return retv;
}

template <typename RT, typename T1, typename T2, uint SZ>
CM_API vector<RT, SZ>
cm_dp4(const stream<T1,SZ>& src0, const stream<T2,SZ>& src1,
       const uint flags = 0)
{
    static const bool conformable4 = check_true<!(SZ%4)>::value;
    static const bool conformable5 =
        check_true<!(is_inttype<T1>::value || is_inttype<T2>::value)>::value;

    int i;
    typename restype_ex<T1,T2>::type ret;
    vector<RT, SZ> retv;

    uint sat1 = CmEmulSys::_SetSatur<T1, is_inttype<RT>::value>::SetSatur();
    for (i = 0; i < SZ; i += 4) {
        SIMDCF_ELEMENT_SKIP(i);
        ret = src0.get(i) * src1.get(i) + src0.get(i + 1) * src1.get(i + 1) + src0.get(i + 2) * src1.get(i + 2) + src0.get(i + 3) * src1.get(i + 3);
        retv(i) = retv(i + 1) = retv(i + 2) = retv(i + 3) = CmEmulSys::satur<RT>::saturate(ret, flags | sat1);
    }

    return retv;
}

template <typename RT, typename T1, typename T2, uint SZ>
CM_API vector<RT, SZ>
cm_dp4(const stream<T1,SZ>& src0, const T2& src1,
       const typename uint_type<T1, T2>::type flags = 0)
{
    vector<T1, SZ> v1;
    vector<T2, SZ> v2(src1);
    vector<RT, SZ> retv;
    int i;

    for (i = 0; i < SZ; i++) {
        v1(i) = src0.get(i);
    }

    retv = cm_dp4<RT>(v1, v2, flags);

    return retv;
}

template <class RetT = uint64_t, class SrcT = uint64_t>
RetT extract(
    const uint width,
    const uint offset,
    const SrcT src,
    const bool sign_extend = false)
{
    assert(sizeof(SrcT)*8 >= width);
    using SrcUT = typename std::make_unsigned<SrcT>::type;
    const auto mask = ~SrcUT(0) >> (sizeof(SrcT)*8 - width);
    auto ret = (src >> offset) & mask;
    if (sign_extend && (ret >> (width - 1)) & 1) ret |= ~mask;
    return ret;
}

template <typename RT, typename T0, typename T1, typename T2, uint SZ>
CM_API vector<RT, SZ>
cm_dp4a(const stream<T0, SZ>& src0, const stream<T1, SZ>& src1,
    const stream<T2, SZ>& src2, const uint flags = 0)
{
    static const bool conformable1 =
        is_dword_type<RT>::value && is_dword_type<T0>::value &&
        is_dword_type<T1>::value && is_dword_type<T2>::value;
    static const bool conformable2 = check_true<SZ==4>::value;
    CM_STATIC_ERROR(conformable1, "only int/uint element type is supported");
    CM_STATIC_ERROR(conformable2, "only SZ of 4 is supported");

    typename restype_ex<T0, typename restype_ex<T1, T2>::type>::type reta;
    vector<RT, SZ> retv;

    int src1_a, src1_b, src1_c, src1_d, src2_a, src2_b, src2_c, src2_d, ret;

    uint sat1 = CmEmulSys::_SetSatur<T0, is_inttype<RT>::value>::SetSatur() ||
        CmEmulSys::_SetSatur<T1, is_inttype<RT>::value>::SetSatur() ||
        CmEmulSys::_SetSatur<T2, is_inttype<RT>::value>::SetSatur();

    for (uint i = 0; i < SZ; i++) {

        SIMDCF_ELEMENT_SKIP(i);

        uint signed_extend_T1 = std::is_same_v <T1, int>;
        uint signed_extend_T2 = std::is_same_v <T2, int>;
        src1_a = extract<int>(8, 0, src1.get(i), signed_extend_T1);
        src1_b = extract<int>(8, 8, src1.get(i), signed_extend_T1);
        src1_c = extract<int>(8, 16, src1.get(i), signed_extend_T1);
        src1_d = extract<int>(8, 24, src1.get(i), signed_extend_T1);
        src2_a = extract<int>(8, 0, src2.get(i), signed_extend_T2);
        src2_b = extract<int>(8, 8, src2.get(i), signed_extend_T2);
        src2_c = extract<int>(8, 16, src2.get(i), signed_extend_T2);
        src2_d = extract<int>(8, 24, src2.get(i), signed_extend_T2);

        ret = src1_a * src2_a + src1_b * src2_b + src1_c * src2_c + src1_d * src2_d;
        reta = ret + src0.get(i);
        retv(i) = CmEmulSys::satur<RT>::saturate(reta, flags | sat1);
    }

    return retv;
}

template <typename RT, typename T1, typename T2, uint SZ>
CM_API vector<RT, SZ>
cm_dph(const stream<T1,SZ>& src0, const stream<T2,SZ>& src1,
       const uint flags = 0)
{
    static const bool conformable4 = check_true<!(SZ%4)>::value;
    static const bool conformable5 =
        check_true<!(is_inttype<T1>::value || is_inttype<T2>::value)>::value;

    int i;
    typename restype_ex<T1,T2>::type ret;
    vector<RT, SZ> retv;

    uint sat1 = CmEmulSys::_SetSatur<T1, is_inttype<RT>::value>::SetSatur();
    for (i = 0; i < SZ; i += 4) {
        SIMDCF_ELEMENT_SKIP(i);
        ret = src0.get(i) * src1.get(i) +
              src0.get(i + 1) * src1.get(i + 1) +
              src0.get(i + 2) * src1.get(i + 2) +
              1.0 * src1.get(i + 3);

        retv(i) = retv(i + 1) = retv(i + 2) = retv(i + 3) = CmEmulSys::satur<RT>::saturate(ret, flags | sat1);
    }

    return retv;
}

template <typename RT, typename T1, typename T2, uint SZ>
CM_API vector<RT, SZ>
cm_dph(const stream<T1,SZ>& src0, const T2& src1,
       const typename uint_type<T1, T2>::type flags = 0)
{
    vector<T1, SZ> v1;
    vector<T2, SZ> v2(src1);
    vector<RT, SZ> retv;
    int i;

    for (i = 0; i < SZ; i++) {
        v1(i) = src0.get(i);
    }

    retv = cm_dph<RT>(v1, v2, flags);

    return retv;
}

template <typename RT, typename T1, typename T2, uint SZ>
CM_API vector<RT, SZ>
cm_dph(const T1& src0, const stream<T2,SZ>& src1,
       const typename uint_type<T1, T2>::type flags = 0)
{
    vector<T1, SZ> v1(src0);
    vector<T1, SZ> v2;
    vector<RT, SZ> retv;
    int i;

    for (i = 0; i < SZ; i++) {
        v2(i) = src1.get(i);
    }

    retv = cm_dph<RT>(v1, v2, flags);

    return retv;
}

//Fraction
template <typename T, uint SZ>
CM_API vector<T, SZ>
cm_frc(const stream<T, SZ>& src0,  const uint flags = 0)
{
    static const bool conformable1 = fptype<T>::value;

    int i;
    float ret;
    vector<float, SZ> retv;

    for (i = 0; i < SZ; i++) {
        SIMDCF_ELEMENT_SKIP(i);
        ret = src0.get(i) - floor(src0.get(i));
        retv(i) = CmEmulSys::satur<float>::saturate(ret, flags);
    }

    return retv;
}

template <typename T>
CM_API T
cm_frc(const T& src0, const typename uint_type<T, T>::type flags = 0)
{
    static const bool conformable1 = fptype<T>::value;

    vector<float, 1> v(src0);
    v = cm_frc(v, flags);
    return v(0);
}

//Leading zero detection
template <typename T1, uint SZ>
CM_API vector<uint,SZ>
cm_lzd(const stream<T1,SZ>& src0, const uint flags = 0)
{
    static const bool conformable1 = uinttype<T1>::value;

    int i;
    uint ret;
    vector<uint, SZ> retv;

    for (i = 0; i < SZ; i++) {
        SIMDCF_ELEMENT_SKIP(i);
        ret = src0.get(i);
        uint cnt = 0;
        while ( (ret & 1u<<31u) == 0 && cnt != 32) {
            cnt ++;
            ret = ret << 1;
        }
        retv(i) = cnt;
    }

    return retv;
}

template <typename T1>
CM_API uint
cm_lzd(const T1& src0, const typename uint_type<T1, T1>::type flags = 0)
{
//    static const bool conformable1 = uinttype<T1>::value;
    vector<T1, 1> v(src0);
    vector<uint, 1> retv;

    retv = cm_lzd(v, flags);

    return retv(0);
}

//Round Down
template <typename RT, uint SZ>
CM_API vector<RT, SZ>
cm_rndd(const stream<float,SZ>& src0, const uint flags = 0)
{
    int i;
    float ret;
    vector<RT, SZ> retv;

    uint sat1 = CmEmulSys::_SetSatur<float, is_inttype<RT>::value>::SetSatur();
    for (i = 0; i < SZ; i++) {
        SIMDCF_ELEMENT_SKIP(i);
        ret = floor(src0.get(i));
        retv(i) = CmEmulSys::satur<RT>::saturate(ret, flags | sat1);
    }

    return retv;
}

template <typename RT>
CM_API RT
cm_rndd(const float& src0, const uint flags = 0)
{
    vector<float, 1> v(src0);
    vector<RT, 1> retv;

    retv = cm_rndd<RT>(v, flags);

    return retv(0);
}

//Round up
template <typename RT, uint SZ>
CM_API vector<RT, SZ>
cm_rndu(const stream<float,SZ>& src0, const uint flags = 0)
{
    int i;
    float ret;
    vector<RT, SZ> retv;
    int increment;

    uint sat1 = CmEmulSys::_SetSatur<float, is_inttype<RT>::value>::SetSatur();
    for (i = 0; i < SZ; i++) {
        SIMDCF_ELEMENT_SKIP(i);
        if (src0.get(i) - floor(src0.get(i)) > 0.0f) {
            increment = 1;
        } else {
            increment = 0;
        }

        ret = floor(src0.get(i)) + increment;
        retv(i) = CmEmulSys::satur<RT>::saturate(ret, flags | sat1);
    }

    return retv;
}

template <typename RT>
CM_API RT
cm_rndu(const float& src0, const uint flags = 0)
{

    vector<float, 1> v(src0);
    vector<RT, 1> retv;

    retv = cm_rndu<RT>(v, flags);

    return retv(0);
}

//Round even
template <typename RT, uint SZ>
CM_API vector<RT,SZ>
cm_rnde(const stream<float,SZ>& src0, const uint flags = 0)
{
    int i;
    float ret;
    vector<RT, SZ> retv;
    int increment;

    uint sat1 = CmEmulSys::_SetSatur<float, is_inttype<RT>::value>::SetSatur();
    for (i = 0; i < SZ; i++) {
        SIMDCF_ELEMENT_SKIP(i);
        if (src0.get(i) - floor(src0.get(i)) > 0.5f) {
            increment = 1;
        } else if (src0.get(i) - floor(src0.get(i)) < 0.5f) {
            increment = 0;
        } else {
            increment = (int(floor(src0.get(i))) % 2 == 1);
        }

        ret = floor(src0.get(i)) + increment;
        retv(i) = CmEmulSys::satur<RT>::saturate(ret, flags | sat1);
    }

    return retv;
}

template <typename RT>
CM_API RT
cm_rnde(const float& src0, const uint flags = 0)
{
    vector<float, 1> v(src0);
    vector<RT, 1> retv;

    retv = cm_rnde<RT>(v, flags);

    return retv(0);
}

//Round zero
template <typename RT, uint SZ>
CM_API vector<RT, SZ>
cm_rndz(const stream<float,SZ>& src0, const uint flags = 0)
{
    int i;
    float ret;
    vector<RT, SZ> retv;
    int increment;

    uint sat1 = CmEmulSys::_SetSatur<float, is_inttype<RT>::value>::SetSatur();
    for (i = 0; i < SZ; i++) {
        SIMDCF_ELEMENT_SKIP(i);
        if (fabs(src0.get(i)) < fabs(floor(src0.get(i)))) {
            increment = 1;
        } else {
            increment = 0;
        }
        ret = floor(src0.get(i)) + increment;
        retv(i) = CmEmulSys::satur<RT>::saturate(ret, flags | sat1);
    }

    return retv;
}

template <typename RT>
CM_API RT
cm_rndz(const float& src0, const uint flags = 0)
{
    vector<float, 1> v(src0);
    vector<RT, 1> retv;

    retv = cm_rndz<RT>(v, flags);

    return retv(0);
}

//Sum of Absolute Difference 2
template <typename RT, typename T1, typename T2, uint SZ>
CM_API vector<RT, SZ>
cm_sad2(const stream<T1,SZ>& src0, const stream<T2,SZ>& src1,
        const uint flags = 0)
{
    static const bool conformable1 = bytetype<T1>::value;
    static const bool conformable2 = bytetype<T2>::value;
    static const bool conformable3 = wordtype<RT>::value;
    static const bool conformable4 = check_true<!(SZ&1)>::value;

    int i;
    typename restype_ex<T1,T2>::type ret;
    vector<RT, SZ> retv;

    uint sat1 = CmEmulSys::_SetSatur<T1, is_inttype<RT>::value>::SetSatur();
    for (i = 0; i < SZ; i+=2) {
         SIMDCF_ELEMENT_SKIP(i);
        ret = CmEmulSys::abs(src0.get(i) - src1.get(i))
                + CmEmulSys::abs(src0.get(i+1) - src1.get(i+1));

        retv(i) = CmEmulSys::satur<RT>::saturate(ret, flags | sat1);
    }

    return retv;
}

template <typename RT, typename T1, typename T2, uint SZ>
CM_API vector<RT, SZ>
cm_sad2(const stream<T1,SZ>& src0, const T2& src1,
       const typename uint_type<T1, T2>::type flags = 0)
{
    vector<T1, SZ> v1;
    vector<T2, SZ> v2(src1);
    vector<RT, SZ> retv;
    int i;

    for (i = 0; i < SZ; i++) {
        v1(i) = src0.get(i);
    }

    retv = cm_sad2<RT>(v1, v2, flags);

    return retv;
}

//Logical shift left
template <typename RT, typename T1, typename T2, uint SZ>
CM_API vector<RT, SZ>
cm_shl(const stream<T1,SZ>& src0, const stream<T2,SZ>& src1,
        const uint flags = 0)
{
    static const bool conformable1 = inttype<T1>::value;
    static const bool conformable2 = unsignedtype<T2>::value;
    static const bool conformable3 = inttype<RT>::value;

    int i;
    typename maxtype<T1>::type ret;
    vector<RT, SZ> retv;

    for (i = 0; i < SZ; i++) {
         SIMDCF_ELEMENT_SKIP(i);
        ret = src0.get(i) << src1.get(i);
        retv(i) = CmEmulSys::satur<RT>::saturate(ret, flags);
    }

    return retv;
}

template <typename RT, typename T1, typename T2, uint SZ>
CM_API vector<RT, SZ>
cm_shl(const stream<T1,SZ>& src0, const T2& src1,
       const typename uint_type<T1, T2>::type flags = 0)
{
    vector<T1, SZ> v1;
    vector<T2, SZ> v2(src1);
    vector<RT, SZ> retv;
    int i;

    for (i = 0; i < SZ; i++) {
        v1(i) = src0.get(i);
    }

    retv = cm_shl<RT>(v1, v2, flags);

    return retv;
}

template <typename RT, typename T1, typename T2, uint SZ>
CM_API vector<RT, SZ>
cm_shl(const T1& src0, const stream<T2,SZ>& src1,
       const typename uint_type<T1, T2>::type flags = 0)
{
    vector<T1, SZ> v1(src0);
    vector<T2, SZ> v2;
    vector<RT, SZ> retv;
    int i;

    for (i = 0; i < SZ; i++) {
        v2(i) = src1.get(i);
    }

    retv = cm_shl<RT>(v1, v2, flags);

    return retv;
}

template <typename RT, typename T1, typename T2>
CM_API RT
cm_shl(const T1& src0, const T2& src1,
       const typename uint_type<T1, T2>::type flags = 0)
{
    vector<T1, 1> v1(src0);
    vector<T2, 1> v2(src1);
    vector<RT, 1> retv;

    retv = cm_shl<RT>(v1, v2, flags);

    return retv(0);
}

//Logical shift right
template <typename RT, typename T1, typename T2, uint SZ>
CM_API vector<RT, SZ>
cm_shr(const stream<T1,SZ>& src0, const stream<T2,SZ>& src1,
        const uint flags = 0)
{
    static const bool conformable1 = unsignedtype<T1>::value;
    static const bool conformable2 = unsignedtype<T2>::value;
    static const bool conformable3 = unsignedtype<RT>::value;

    int i;
    typename maxtype<T1>::type ret;
    vector<RT, SZ> retv;

    for (i = 0; i < SZ; i++) {
         SIMDCF_ELEMENT_SKIP(i);
        ret = src0.get(i) >> src1.get(i);
        retv(i) = CmEmulSys::satur<RT>::saturate(ret, flags);
    }

    return retv;
}

template <typename RT, typename T1, typename T2, uint SZ>
CM_API vector<RT, SZ>
cm_shr(const stream<T1,SZ>& src0, const T2& src1,
       const typename uint_type<T1, T2>::type flags = 0)
{
    vector<T1, SZ> v1;
    vector<T2, SZ> v2(src1);
    vector<RT, SZ> retv;
    int i;

    for (i = 0; i < SZ; i++) {
        v1(i) = src0.get(i);
    }

    retv = cm_shr<RT>(v1, v2, flags);

    return retv;
}

template <typename RT, typename T1, typename T2, uint SZ>
CM_API vector<RT, SZ>
cm_shr(const T1& src0, const stream<T2,SZ>& src1,
       const typename uint_type<T1, T2>::type flags = 0)
{
    vector<T1, SZ> v1(src0);
    vector<T2, SZ> v2;
    vector<RT, SZ> retv;
    int i;

    for (i = 0; i < SZ; i++) {
        v2(i) = src1.get(i);
    }

    retv = cm_shr<RT>(v1, v2, flags);

    return retv;
}

template <typename RT, typename T1, typename T2>
CM_API RT
cm_shr(const T1& src0, const T2& src1,
       const typename uint_type<T1, T2>::type flags = 0)
{
    vector<T1, 1> v1(src0);
    vector<T2, 1> v2(src1);
    vector<RT, 1> retv;

    retv = cm_shr<RT>(v1, v2, flags);

    return retv(0);
}

//Ariphmetic shift right
template <typename RT, typename T1, typename T2, uint SZ>
CM_API vector<RT, SZ>
cm_asr(const stream<T1,SZ>& src0, const stream<T2,SZ>& src1,
        const uint flags = 0)
{
    static const bool conformable1 = inttype<T1>::value;
    static const bool conformable2 = unsignedtype<T2>::value;
    static const bool conformable3 = inttype<RT>::value;

    int i;
    typename maxtype<T1>::type ret;
    vector<RT, SZ> retv;

    for (i = 0; i < SZ; i++) {
         SIMDCF_ELEMENT_SKIP(i);
        ret = src0.get(i) >> src1.get(i);
        retv(i) = CmEmulSys::satur<RT>::saturate(ret, flags);
    }

    return retv;
}

template <typename RT, typename T1, typename T2, uint SZ>
CM_API vector<RT, SZ>
cm_asr(const stream<T1,SZ>& src0, const T2& src1,
       const typename uint_type<T1, T2>::type flags = 0)
{
    vector<T1, SZ> v1;
    vector<T2, SZ> v2(src1);
    vector<RT, SZ> retv;
    int i;

    for (i = 0; i < SZ; i++) {
        v1(i) = src0.get(i);
    }

    retv = cm_asr<RT>(v1, v2, flags);

    return retv;
}

template <typename RT, typename T1, typename T2, uint SZ>
CM_API vector<RT, SZ>
cm_asr(const T1& src0, const stream<T2,SZ>& src1,
       const typename uint_type<T1, T2>::type flags = 0)
{
    vector<T1, SZ> v1(src0);
    vector<T2, SZ> v2;
    vector<RT, SZ> retv;
    int i;

    for (i = 0; i < SZ; i++) {
        v2(i) = src1.get(i);
    }

    retv = cm_asr<RT>(v1, v2, flags);

    return retv;
}

template <typename RT, typename T1, typename T2>
CM_API RT
cm_asr(const T1& src0, const T2& src1,
       const typename uint_type<T1, T2>::type flags = 0)
{
    vector<T1, 1> v1(src0);
    vector<T2, 1> v2(src1);
    vector<RT, 1> retv;

    retv = cm_asr<RT>(v1, v2, flags);

    return retv(0);
}

template <typename RT, typename T1, typename T2, uint SZ>
CM_API vector<RT, SZ>
cm_line(const stream<T1, 4>& src0, const stream<T2,SZ>& src1,
        const typename uint_type<T1, T2>::type flags = 0)
{
    int i;
    vector<RT, SZ> retv;
    typename restype_ex<T1,T2>::type ret;

    uint sat1 = CmEmulSys::_SetSatur<float, is_inttype<RT>::value>::SetSatur();
    for (i = 0; i < SZ; i++) {
          SIMDCF_ELEMENT_SKIP(i);
         ret = src0.get(0) * src1.get(i) + src0.get(3);
        retv(i) = CmEmulSys::satur<RT>::saturate(ret, flags | sat1);
    }

    return retv;
}

template <typename RT, typename T1, typename T2, uint SZ>
CM_API vector<RT, SZ>
cm_line(const T1& P, const T1& Q, const stream<T2,SZ>& src1,
        const typename uint_type<T1, T2>::type flags = 0)
{
    vector<T1, 4> src0;
    vector<RT, SZ> retv;
    src0(0) = P;
    src0(3) = Q;

    retv = cm_line<RT>(src0, src1, flags);

    return retv;
}

//*********************Reduction inrinsics***********************
template <typename RT, typename T, uint SZ>
CM_API RT
cm_sum(const stream<T,SZ>& src1, const uint flags = 0)
{
    int i;
    RT retv = 0;

    if (std::numeric_limits<RT>::is_integer) {
        for (i = 0; i < SZ; i++) {
            SIMDCF_ELEMENT_SKIP(i);
            retv = cm_add<RT>(retv, src1.get(i), flags);
        }
    }
    else {
        for (i = 0; i < SZ; i++) {
            SIMDCF_ELEMENT_SKIP(i);
            retv = cm_add<RT>(retv, src1.get(i));
        }
        retv = CmEmulSys::satur<RT>::saturate(retv, flags);
    }

    return retv;
}

//*********************Reduction inrinsics***********************
template <typename RT, typename T, uint SZ>
CM_API RT
cm_reduced_max(const stream<T,SZ>& src1, const uint flags = 0)
{
    int i;
    RT retv = 0;
    T tmp = 0;

    for (i = 0; i < SZ; i++) {
        SIMDCF_ELEMENT_SKIP(i);
        tmp = src1.get(i);
    }

    for (i = 0; i < SZ; i++) {
        SIMDCF_ELEMENT_SKIP(i);
        if (src1.get(i) > tmp) {
            tmp = src1.get(i);
        }
    }

    retv = CmEmulSys::satur<RT>::saturate(tmp, flags);
    return retv;
}

//*********************Reduction inrinsics***********************
template <typename RT, typename T, uint SZ>
CM_API RT
cm_reduced_min(const stream<T,SZ>& src1, const uint flags = 0)
{
    int i;
    RT retv = 0;
    T tmp = 0;

    for (i = 0; i < SZ; i++) {
        SIMDCF_ELEMENT_SKIP(i);
        tmp = src1.get(i);
    }

    for (i = 0; i < SZ; i++) {
        SIMDCF_ELEMENT_SKIP(i);
        if (src1.get(i) < tmp) {
            tmp = src1.get(i);
        }
    }

    retv = CmEmulSys::satur<RT>::saturate(tmp, flags);
    return retv;
}

template <typename RT, typename T, uint SZ>
CM_API RT
cm_prod(const stream<T,SZ>& src1, const uint flags = 0)
{
    int i;
    RT retv = 1;

    if (std::numeric_limits<RT>::is_integer) {
        int tmp= 1;
        for (i = 0; i < SZ; i++) {
            SIMDCF_ELEMENT_SKIP(i);
            tmp *= src1.get(i);
        }
        retv = CmEmulSys::satur<RT>::saturate(tmp, flags);
    }
    else {
        float tmp= 1.0;
        for (i = 0; i < SZ; i++) {
            SIMDCF_ELEMENT_SKIP(i);
            tmp *= src1.get(i);
        }
        retv = CmEmulSys::satur<RT>::saturate(tmp, flags);
    }
    return retv;
}

//***************** Extended Math Unit Intrinsics*****************
// dst = 1.0/src
template <uint SZ>
CM_API vector<float, SZ>
cm_inv(const stream<float, SZ>& src0, const uint flags = 0)
{
    vector<float, SZ> retv;

    for (int i = 0; i < SZ; i++) {
        SIMDCF_ELEMENT_SKIP(i);
        retv(i) = CmEmulSys::satur<float>::saturate(1. / src0.get(i), flags);
    }

    return retv;
}
template <typename T>
CM_API float
cm_inv(const T& src0, const typename uint_type<T, T>::type flags = 0)
{
    static const bool conformable1 = fptype<T>::value;
    vector<float, 1> v(src0);
    v = cm_inv(v, flags);
    return v(0);
}

//dst = log_2(scr)
template <uint SZ>
CM_API vector<float, SZ>
cm_log(const stream<float, SZ>& src0, const uint flags = 0)
{
    vector<float, SZ> retv;

    for (int i = 0; i < SZ; i++) {
        SIMDCF_ELEMENT_SKIP(i);
        retv(i) = CmEmulSys::satur<float>::saturate(logf(src0.get(i)) / logf(2.), flags);
    }
    return retv;
}
template <typename T>
CM_API float
cm_log(const T& src0, const typename uint_type<T, T>::type flags = 0)
{
    static const bool conformable1 = fptype<T>::value;
    vector<float, 1> v(src0);
    v = cm_log(v, flags);
    return v(0);
}


//dst = 2**src
template <uint SZ>
CM_API vector<float, SZ>
cm_exp(const stream<float, SZ>& src0, const uint flags = 0)
{
    vector<float, SZ> retv;

    for (int i = 0; i < SZ; i++) {
        SIMDCF_ELEMENT_SKIP(i);
        retv(i) = CmEmulSys::satur<float>::saturate(powf(2., src0.get(i)), flags);
    }
    return retv;
}
template <typename T>
CM_API float
cm_exp(const T& src0, const typename uint_type<T, T>::type flags = 0)
{
    static const bool conformable1 = fptype<T>::value;
    vector<float, 1> v(src0);
    v = cm_exp(v, flags);
    return v(0);
}

//dst = sqrt(src)
template <uint SZ>
CM_API vector<float, SZ>
cm_sqrt(const stream<float, SZ>& src0, const uint flags = 0)
{
    vector<float, SZ> retv;

    for (int i = 0; i < SZ; i++) {
        SIMDCF_ELEMENT_SKIP(i);
        retv(i) = CmEmulSys::satur<float>::saturate(sqrt(src0.get(i)), flags);
    }
    return retv;
}
template <typename T>
CM_API float
cm_sqrt(const T& src0, const typename uint_type<T, T>::type flags = 0)
{
    static const bool conformable1 = fptype<T>::value;
    vector<float, 1> v(src0);
    v = cm_sqrt(v, flags);
    return v(0);
}

//dst = 1.0/sqrt(src)
template <uint SZ>
CM_API vector<float, SZ>
cm_rsqrt(const stream<float, SZ>& src0, const uint flags = 0)
{
    vector<float, SZ> retv;

    for (int i = 0; i < SZ; i++) {
        SIMDCF_ELEMENT_SKIP(i);
        retv(i) = CmEmulSys::satur<float>::saturate(1. / sqrt(src0.get(i)), flags);
    }
    return retv;
}
template <typename T>
CM_API float
cm_rsqrt(const T& src0, const typename uint_type<T, T>::type flags = 0)
{
    static const bool conformable1 = fptype<T>::value;
    vector<float, 1> v(src0);
    v = cm_rsqrt(v, flags);
    return v(0);
}

#if defined(CM_GEN4) || defined(CM_GEN5)
template <uint SZ>
CM_API vector<float, SZ>
cm_pow(const stream<float, SZ>& src0, const stream<float, SZ>& src1, const uint flags = 0)
{
    std::cerr << "\ncm_pow intrinsic does not support stream power for ILK" << std::endl;
}
template <uint SZ>
CM_API vector<float, SZ>
cm_pow( const float& src0, const stream<float, SZ>& src1, const uint flags = 0)
{
    std::cerr << "\ncm_pow intrinsic does not support stream power for ILK" << std::endl;
}
//dst = pow(src0,src1), src1 is always scalar
template <uint SZ>
CM_API vector<float, SZ>
cm_pow(const stream<float, SZ>& src0, const float& src1, const uint flags = 0)
{
    vector<float, SZ> retv;

    for (int i = 0; i < SZ; i++) {
        retv(i) =
            CmEmulSys::satur<float>::saturate(powf(fabs(src0.get(i)), src1), flags);
    }
    return retv;
}
template <typename T>
CM_API float
cm_pow(const T& src0, const T& src1,
       const typename uint_type<T, T>::type flags = 0)
{
    static const bool conformable1 = fptype<T>::value;
    vector<float, 1> v(src0);
    v = cm_pow(v, src1, flags);
    return v(0);
}
#else
template <uint SZ>
CM_API vector<float, SZ>
cm_pow(const stream<float, SZ>& src0, const stream<float, SZ>& src1, const uint flags = 0)
{
    vector<float, SZ> retv;

    for (int i = 0; i < SZ; i++) {
        SIMDCF_ELEMENT_SKIP(i);
        retv(i) = CmEmulSys::satur<float>::saturate(powf(fabs(src0.get(i)), src1.get(i)), flags);
    }
    return retv;
}
template <uint SZ>
CM_API vector<float, SZ>
cm_pow( const float& src0, const stream<float, SZ>& src1, const uint flags = 0)
{
    vector<float, SZ> retv;

    for (int i = 0; i < SZ; i++) {
        SIMDCF_ELEMENT_SKIP(i);
        retv(i) = CmEmulSys::satur<float>::saturate(powf(fabs(src0), src1.get(i)), flags);
    }
    return retv;
}
template <uint SZ>
CM_API vector<float, SZ>
cm_pow(const stream<float, SZ>& src0, const float& src1, const uint flags = 0)
{
    vector<float, SZ> retv;

    for (int i = 0; i < SZ; i++) {
        SIMDCF_ELEMENT_SKIP(i);
        retv(i) = CmEmulSys::satur<float>::saturate(powf(fabs(src0.get(i)), src1), flags);
    }
    return retv;
}
template <typename T>
CM_API float
cm_pow(const T& src0, const T& src1,
       const typename uint_type<T, T>::type flags = 0)
{
    static const bool conformable1 = fptype<T>::value;
    vector<float, 1> v(src0);
    vector<float, 1> v1(src1);
    v = cm_pow(v, v1, flags);
    return v(0);
}
#endif

/// cm_div_ieee : start

template <uint SZ>
CM_API vector<float, SZ>
cm_div_ieee(const stream<float, SZ>& src0,
            const stream<float, SZ>& src1,
            const uint flags = 0)
{
    vector<float, SZ> divinv;
    vector<float, SZ> retv;
    float             oneret;

    for (int idx = 0; idx < SZ; idx += 1)
    {
        SIMDCF_ELEMENT_SKIP(idx);
        if (src1.get(idx) == 0.0f)
        {
            /// Handle Divide-by-zero
            retv(idx) = (src0.get(idx) < 0) ? (-INFINITY) : INFINITY;
        }
        else
        {
            oneret = src0.get(idx) / src1.get(idx);
            retv(idx) = CmEmulSys::satur<float>::saturate(oneret, flags);
        }
    }

    return retv;
}

template <uint SZ>
CM_API vector<float, SZ>
cm_div_ieee(const float& src0,
            const stream<float, SZ>& src1,
            const uint flags = 0)
{
    vector<float, SZ> v0;
    vector<float, SZ> v1;
    vector<float, SZ> retv;

    for (int i = 0; i < SZ; i += 1)
    {
        v0(i) = src0;
        v1(i) = src1.get(i);
    }

    retv = cm_div_ieee(v0, v1, flags);

    return retv;
}

template <uint SZ>
CM_API vector<float, SZ>
cm_div_ieee(const stream<float, SZ>& src0,
            const float& src1,
            const uint flags = 0)
{
    vector<float, SZ> v0;
    vector<float, SZ> v1;
    vector<float, SZ> retv;

    for (int i = 0; i < SZ; i += 1)
    {
        v0(i) = src0.get(i);
        v1(i) = src1;
    }

    retv = cm_div_ieee(v0, v1, flags);

    return retv;
}

template <typename T>
CM_API float
cm_div_ieee(const T& src0,
            const T& src1,
            const typename uint_type<T, T>::type flags = 0)
{
    static const bool conformable1 = fptype<T>::value;
    vector<float, 1> v(src0);
    vector<float, 1> v1(src1);
    v = cm_div_ieee(v, v1, flags);
    return v(0);
}
/// cm_div_ieee : end

//dst = sin(src)
template <uint SZ>
CM_API vector<float, SZ>
cm_sin(const stream<float, SZ>& src0, const uint flags = 0)
{
    vector<float, SZ> retv;

    for (int i = 0; i < SZ; i++) {
        SIMDCF_ELEMENT_SKIP(i);
        retv(i) = CmEmulSys::satur<float>::saturate(sin(src0.get(i)), flags);
    }
    return retv;
}
template <typename T>
CM_API float
cm_sin(const T& src0, const typename uint_type<T, T>::type flags = 0)
{
    static const bool conformable1 = fptype<T>::value;
    vector<float, 1> v(src0);
    v = cm_sin(v, flags);
    return v(0);
}

//dst = cos(src)
template <uint SZ>
CM_API vector<float, SZ>
cm_cos(const stream<float, SZ>& src0, const uint flags = 0)
{
    vector<float, SZ> retv;

    for (int i = 0; i < SZ; i++) {
        SIMDCF_ELEMENT_SKIP(i);
        retv(i) = CmEmulSys::satur<float>::saturate(cos(src0.get(i)), flags);
    }
    return retv;
}
template <typename T>
CM_API float
cm_cos(const T& src0, const typename uint_type<T, T>::type flags = 0)
{
    static const bool conformable1 = fptype<T>::value;
    vector<float, 1> v(src0);
    v = cm_cos(v, flags);
    return v(0);
}


//sincos
template <uint SZ>
CM_API vector<float, SZ>
cm_sincos(vector<float, SZ> &cosv, const stream<float, SZ>& src0,
          const uint flags = 0)
{
    vector<float, SZ> retv;

    for (int i = 0; i < SZ; i++) {
        SIMDCF_ELEMENT_SKIP(i);
        retv(i) = CmEmulSys::satur<float>::saturate(sin(src0.get(i)), flags);
        cosv(i) = CmEmulSys::satur<float>::saturate(cos(src0.get(i)), flags);
    }
    return retv;
}

template <typename T>
CM_API float
cm_sincos(T& cosr, const T& src0,
          const typename uint_type<T, T>::type flags = 0)
{
    vector<float, 1> v(src0);
    vector<float, 1> v1;
    v = cm_sincos(v1,v, flags);
    cosr = v1(0);
    return v(0);
}

template <uint SZ>
CM_API vector<float, SZ>
cm_acos(const stream<float, SZ>& src0, const uint flags = 0)
{
    vector<float, SZ> retv;

    for (int i = 0; i < SZ; i++) {
        SIMDCF_ELEMENT_SKIP(i);
        retv(i) = CmEmulSys::satur<float>::saturate(acos(src0.get(i)), flags);
    }
    return retv;
}
template <typename T>
CM_API float
cm_acos(const T& src0, const typename uint_type<T, T>::type flags = 0)
{
    static const bool conformable1 = fptype<T>::value;
    vector<float, 1> v(src0);
    v = cm_acos(v, flags);
    return v(0);
}

template <uint SZ>
CM_API vector<float, SZ>
cm_asin(const stream<float, SZ>& src0, const uint flags = 0)
{
    vector<float, SZ> retv;
    for (int i = 0; i < SZ; i++) {
        SIMDCF_ELEMENT_SKIP(i);
        retv(i) = CmEmulSys::satur<float>::saturate(asin(src0.get(i)), flags);
    }
    return retv;
}
template <typename T>
CM_API float
cm_asin(const T& src0, const typename uint_type<T, T>::type flags = 0)
{
    static const bool conformable1 = fptype<T>::value;
    vector<float, 1> v(src0);
    v = cm_asin(v, flags);
    return v(0);
}

template <uint SZ>
CM_API vector<float, SZ>
cm_atan(const stream<float, SZ>& src0, const uint flags = 0)
{
    vector<float, SZ> retv;
    for (int i = 0; i < SZ; i++) {
        SIMDCF_ELEMENT_SKIP(i);
        retv(i) = CmEmulSys::satur<float>::saturate(atan(src0.get(i)), flags);
    }
    return retv;
}
template <typename T>
CM_API float
cm_atan(const T& src0, const typename uint_type<T, T>::type flags = 0)
{
    static const bool conformable1 = fptype<T>::value;
    vector<float, 1> v(src0);
    v = cm_atan(v, flags);
    return v(0);
}

template <typename T, uint R, uint C>
CM_API void
cm_input(matrix<T,R,C> &in)
{
    //printf("workarroundForIpoBugOrFeature\n");
}

template <typename T, uint R, uint C>
CM_API void
cm_input(matrix_ref<T,R,C> in)
{
    //printf("workarroundForIpoBugOrFeature\n");
}

template <typename T, uint S>
CM_API void
cm_input(vector<T, S> &in)
{
    //printf("workarroundForIpoBugOrFeature\n");
}

template <typename T, uint S>
CM_API void
cm_input(vector_ref<T, S> in)
{
    //printf("workarroundForIpoBugOrFeature\n");
}

template<typename T>
CM_API void
cm_input(T& in)
{
    //printf("workarroundForIpoBugOrFeature\n");
}

template <typename T, uint R, uint C>
CM_API void
cm_output(const matrix<T,R,C> &out)
{
    //printf("workarroundForIpoBugOrFeature\n");
}

template <typename T, uint R, uint C>
CM_API void
cm_output(const matrix_ref<T,R,C> out)
{
    //printf("workarroundForIpoBugOrFeature\n");
}

template <typename T, uint S>
CM_API void
cm_output(const vector<T, S> &out)
{
    //printf("workarroundForIpoBugOrFeature\n");
}

template <typename T, uint S>
CM_API void
cm_output(const vector_ref<T, S> out)
{
    //printf("workarroundForIpoBugOrFeature\n");
}

template<typename T>
CM_API void
cm_output(const T& out)
{
    //printf("workarroundForIpoBugOrFeature\n");
}

template <typename T, uint SZ>
CM_API uint
__cm_pack_mask(const vector_ref<T,SZ>& src0) {
    uint retv = 0;
    for (int i = 0; i < SZ; i++) {
        if (src0.get(i) & 0x1) {
            retv |= 0x1 << i;
        }
    }

    return retv;
}

template <typename T, uint R, uint C>
CM_API uint
__cm_pack_mask(const matrix_ref<T,R,C>& src0) {
    uint retv = 0;
    for (int i = 0; i < R*C; i++) {
        if (src0(i / C, i % C) & 0x1) {
            retv |= 0x1 << i;
        }
    }

    return retv;
}

template <typename T, uint SZ>
_GENX_ inline uint
cm_pack_mask(const vector_ref<T,SZ>& src0) {
    static const bool conformable1 = is_ushort_type<T>::value;
    static const bool conformable2 = check_true< SZ == 8 || SZ == 16 || SZ == 32>::value;
    CM_STATIC_ERROR(conformable1, "only ushort element type is supported");
    CM_STATIC_WARNING(conformable2, "argument isn't 8, 16 or 32 elements - temporary will be introduced");

    // We rely on the compiler doing the right thing with the SZ compile time constant to reduce
    // this code to the minimum required
    if ( ! conformable2 ) {
        vector<T, SZ < 8 ? 8 : ( SZ < 16 ? 16 : 32 ) > _Src0 = 0;
        _Src0.template select<SZ,1>() = src0;
        return __cm_pack_mask(_Src0.select_all());
    }

    return __cm_pack_mask(src0);
}

template <typename T, uint SZ>
_GENX_ inline uint
cm_pack_mask(const vector<T,SZ>& src0) {
    return cm_pack_mask(src0.select_all());
}

template <typename T, uint R, uint C>
_GENX_ inline uint
cm_pack_mask(const matrix_ref<T,R,C>& src0) {
    static const bool conformable1 = is_ushort_type<T>::value;
    static const bool conformable2 = check_true< R*C == 8 || R*C == 16 || R*C == 32>::value;
    CM_STATIC_ERROR(conformable1, "only ushort element type is supported");
    CM_STATIC_WARNING(conformable2, "argument isn't 8, 16 or 32 elements - temporary will be introduced");

    if ( ! conformable2 ) {
        vector<T, (R*C) < 8 ? 8 : ( (R*C) < 16 ? 16 : 32 ) > _Src0 = 0;
        _Src0.template select<R*C,1>() = src0.template format<T>();
        return __cm_pack_mask(_Src0.select_all());
    }

    return __cm_pack_mask(src0);
}

template <typename T, uint R, uint C>
_GENX_ inline uint
cm_pack_mask(const matrix<T,R,C>& src0) {
    return cm_pack_mask(src0.select_all());
}

template <typename RT, uint SZ>
CM_API vector<RT,SZ>
cm_unpack_mask(const uint& src0) {
    static const bool conformable1 = is_ushort_type<RT>::value;
    static const bool conformable2 = check_true< SZ == 8 || SZ == 16 || SZ == 32>::value;
    CM_STATIC_ERROR(conformable1, "only ushort element type is supported");
    CM_STATIC_WARNING(conformable2, "argument isn't 8, 16 or 32 elements - temporary will be introduced");

    vector<RT,SZ> retv;
    for (int i = 0; i < SZ; i++) {
        if ((src0 >> i) & 0x1) {
            retv(i) = 1;
        } else {
            retv(i) = 0;
        }
    }

    return retv;
}

// Count component-wise the total bits set in source operand
template <typename T1, uint SZ>
CM_API vector<uint,SZ>
cm_cbit(const stream<T1,SZ>& src0, const uint flags = 0)
{
    static const bool conformable1 = inttype<T1>::value;

    int i;
    uint ret;
    vector<uint, SZ> retv;

    for (i = 0; i < SZ; i++) {
        SIMDCF_ELEMENT_SKIP(i);
        ret = src0.get(i);
        uint cnt = 0;
        for(int j = 0; j < sizeof(T1)*8; j++) {
            if((ret & 1u) == 1) {
                cnt ++;
            }
            ret = ret >> 1;
        }
        retv(i) = cnt;
    }

    return retv;
}

template <typename T1>
CM_API uint
cm_cbit(const T1& src0, const typename uint_type<T1, T1>::type flags = 0)
{
    static const bool conformable1 = inttype<T1>::value;

    vector<T1, 1> v(src0);
    vector<uint, 1> retv;

    retv = cm_cbit(v, flags);

    return retv(0);
}

// Find component-wise the first bit from LSB side
template <typename T1, uint SZ>
CM_API vector<uint,SZ>
cm_fbl(const stream<T1,SZ>& src0, const uint flags = 0)
{
    static const bool conformable1 = uinttype<T1>::value;

    int i;
    uint ret;
    vector<uint, SZ> retv;

    for (i = 0; i < SZ; i++) {
        SIMDCF_ELEMENT_SKIP(i);
        ret = src0.get(i);
        uint cnt = 0;
        while ((ret & 1u) == 0 && cnt != 32) {
            cnt ++;
            ret = ret >> 1;
        }
        if(src0.get(i) == 0x0) {
            retv(i) = 0xFFFFFFFF;
        } else {
            retv(i) = cnt;
        }
    }

    return retv;
}

template <typename T1>
CM_API uint
cm_fbl(const T1& src0, const typename uint_type<T1, T1>::type flags = 0)
{
    static const bool conformable1 = uinttype<T1>::value;
    vector<T1, 1> v(src0);
    vector<uint, 1> retv;

    retv = cm_fbl(v, flags);

    return retv(0);
}

// Find component-wise the first bit from MSB side
template <typename T1, uint SZ>
CM_API vector<T1,SZ>
cm_fbh(const stream<T1,SZ>& src0, const uint flags = 0)
{
    static const bool conformable1 = dwordtype<T1>::value;

    bool uintOpnd = check_true<(unsignedtype <T1>::value)>::value;

    int i, cval;
    T1 ret;
    vector<T1, SZ> retv;

    for (i = 0; i < SZ; i++) {
        SIMDCF_ELEMENT_SKIP(i);
        ret = src0.get(i);
        uint cnt = 0;
        if (uintOpnd) {
            while ((ret & (1u << 31u)) == 0 && cnt != 32) {
                cnt ++;
                ret = ret << 1;
            }
            if (src0.get(i) == 0x00000000) {
                retv(i) = 0xFFFFFFFF;
            } else {
                retv(i) = cnt;
            }
        }
        else {
            if (((ret >> 31u) & 1u) == 1) {
                cval = 1;
            }
            else {
                cval = 0;
            }
            while (((ret >> 31u) & 1u) == cval && cnt != 32) {
                cnt ++;
                ret = ret << 1;
            }

            if ((src0.get(i) == 0xFFFFFFFF) ||
                 (src0.get(i) == 0x00000000)) {
                retv(i) = 0xFFFFFFFF;
            } else {
                retv(i) = cnt;
            }
        }
    }

    return retv;
}

template <typename T1>
CM_API T1
cm_fbh(const T1& src0, const typename uint_type<T1, T1>::type flags = 0)
{
    static const bool conformable1 = dwordtype<T1>::value;
    vector<T1, 1> v(src0);
    vector<T1, 1> retv;

    retv = cm_fbh(v, flags);

    return retv(0);
}


template <typename T>
CM_API vector<T, 4>
cm_rdtsc()
{
    static const bool conformable1 = uinttype<T>::value;
    vector<T, 4> dst = 0;
    return dst;
}

//------------------------------------------------------------------------------
CM_API void cm_fsetround(CmRoundingMode val); // Set the fp rounding mode
CM_API CmRoundingMode cm_fgetround(void); // Get the fp rounding mode
CM_API void cm_fsetmode(CmFPMode val); // Set the fp mode (ALT/IEEE)
CM_API CmFPMode cm_fgetmode(void); // Get the fp mode (ALT/IEEE)

//------------------------------------------------------------------------------
CM_API void cm_pause(unsigned short length);

//------------------------------------------------------------------------------
// Insert value into src bitfield of defined width at defined offset
// The instruction restricts the src and dst types to D or UD types
template<typename RT, typename T1, typename T2, typename T3, typename T4, uint SZ>
CM_API vector<RT, SZ>
cm_bf_insert(const stream<T1,SZ>& width, const stream<T2, SZ>& offset,
             const stream<T3, SZ>& val, const stream<T4,SZ>& src,
             const uint flags = 0, const uint flags2 = 0)
{
    static const bool conformable1 = dwordtype<T1>::value;
    static const bool conformable2 = dwordtype<T2>::value;
    static const bool conformable3 = dwordtype<T3>::value;
    static const bool conformable4 = dwordtype<T4>::value;
    static const bool conformable5 = dwordtype<RT>::value;
    static const bool is_unsigned  = unsignedtype<RT>::value;

    int i;
    typename maxtype<T4>::type ret;
    vector<RT, SZ> retv;

    for (i = 0; i < SZ; i++) {
        SIMDCF_ELEMENT_SKIP(i);
        const uint mask = ((1 << width.get(i)) - 1) << offset.get(i);
        const uint imask = ~mask;
        ret = (src.get(i) & imask) | ((val.get(i) << offset.get(i) & mask));
        // Sign extend if signed type
        if (!is_unsigned) {
            const int m = 1U << (width.get(i) - 1);
            ret = (ret ^ m) - m;
        }
        retv(i) = ret;
    }

    return retv;
}

template<typename RT, typename T1, typename T2, typename T3, typename T4, uint SZ>
CM_API vector<RT, SZ>
cm_bf_insert(const stream<T1,SZ>& width, const stream<T2, SZ>& offset,
             const stream<T3, SZ>& val, const T4& src,
             const typename uint_type<T1, T2>::type flags = 0,
             const typename uint_type<T3, T4>::type flags2 = 0)
{
    vector<T1, SZ> v1;
    vector<T2, SZ> v2;
    vector<T3, SZ> v3;
    vector<T4, SZ> v4(src);
    vector<RT, SZ> retv;

    int i;
    for (i = 0; i < SZ; i++) {
        v1(i) = width.get(i);
        v2(i) = offset.get(i);
        v3(i) = val.get(i);
    }

    retv = cm_bf_insert<RT>(v1, v2, v3, v4, flags);

    return retv;
}

template<typename RT, typename T1, typename T2, typename T3, typename T4, uint SZ>
CM_API vector<RT, SZ>
cm_bf_insert(const stream<T1,SZ>& width, const stream<T2, SZ>& offset,
             const T3& val, const stream<T4,SZ>& src,
             const typename uint_type<T1, T2>::type flags = 0,
             const typename uint_type<T3, T4>::type flags2 = 0)
{
    vector<T1, SZ> v1;
    vector<T3, SZ> v2;
    vector<T2, SZ> v3(val);
    vector<T4, SZ> v4;
    vector<RT, SZ> retv;

    int i;
    for (i = 0; i < SZ; i++) {
        v1(i) = width.get(i);
        v2(i) = offset.get(i);
        v4(i) = src.get(i);
    }

    retv = cm_bf_insert<RT>(v1, v2, v3, v4, flags);

    return retv;
}

template<typename RT, typename T1, typename T2, typename T3, typename T4, uint SZ>
CM_API vector<RT, SZ>
cm_bf_insert(const T1& width, const T2& offset,
             const stream<T3, SZ>& val, const stream<T4,SZ>& src,
             const typename uint_type<T1, T2>::type flags = 0,
             const typename uint_type<T3, T4>::type flags2 = 0)
{
    vector<T1, SZ> v1(width);
    vector<T2, SZ> v2(offset);
    vector<T3, SZ> v3;
    vector<T4, SZ> v4;
    vector<RT, SZ> retv;

    int i;
    for (i = 0; i < SZ; i++) {
        v3(i) = val.get(i);
        v4(i) = src.get(i);
    }

    retv = cm_bf_insert<RT>(v1, v2, v3, v4, flags);

    return retv;
}

template<typename RT, typename T1, typename T2, typename T3, typename T4, uint SZ>
CM_API vector<RT, SZ>
cm_bf_insert(const T1& width, const T2& offset,
             const stream<T3, SZ>& val, const T4& src,
             const typename uint_type<T1, T2>::type flags = 0,
             const typename uint_type<T3, T4>::type flags2 = 0)
{
    vector<T1, SZ> v1(width);
    vector<T2, SZ> v2(offset);
    vector<T3, SZ> v3;
    vector<T4, SZ> v4(src);
    vector<RT, SZ> retv;

    int i;
    for (i = 0; i < SZ; i++) {
        v3(i) = val.get(i);
    }

    retv = cm_bf_insert<RT>(v1, v2, v3, v4, flags);

    return retv;
}

template<typename RT, typename T1, typename T2, typename T3, typename T4, uint SZ>
CM_API vector<RT, SZ>
cm_bf_insert(const T1& width, const T2& offset,
             const T3& val, const stream<T3,SZ>& src,
             const typename uint_type<T1, T2>::type flags = 0,
             const typename uint_type<T3, T4>::type flags2 = 0)
{
    vector<T1, SZ> v1(width);
    vector<T2, SZ> v2(offset);
    vector<T3, SZ> v3(val);
    vector<T4, SZ> v4;
    vector<RT, SZ> retv;

    int i;
    for (i = 0; i < SZ; i++) {
        v4(i) = src.get(i);
    }

    retv = cm_bf_insert<RT>(v1, v2, v3, v4, flags);

    return retv;
}

template<typename RT, typename T1, typename T2, typename T3, typename T4>
CM_API RT
cm_bf_insert(const T1& width, const T2& offset,
             const T3& val, const T4& src,
             const typename uint_type<T1, T2>::type flags = 0,
             const typename uint_type<T3, T4>::type flags2 = 0)
{
    vector<T3, 1> v1(width);
    vector<T4, 1> v2(offset);
    vector<T2, 1> v3(val);
    vector<T1, 1> v4(src);
    vector<RT, 1> retv;

    retv = cm_bf_insert<RT>(v1, v2, v3, v4, flags);

    return retv(0);
}


// Extract value from src bitfield of defined width at defined offset
template<typename RT, typename T1, typename T2, typename T3, uint SZ>
CM_API vector<RT, SZ>
cm_bf_extract(const stream<T1,SZ>& width, const stream<T2,SZ>& offset,
              const stream<T3,SZ>& src,
              const uint flags = 0)
{
    static const bool conformable1 = dwordtype<T1>::value;
    static const bool conformable2 = dwordtype<T2>::value;
    static const bool conformable3 = dwordtype<T3>::value;
    static const bool conformable5 = dwordtype<RT>::value;

    int i;
    typename maxtype<T3>::type ret;
    vector<RT, SZ> retv;

    for (i = 0; i < SZ; i++) {
        SIMDCF_ELEMENT_SKIP(i);
        const uint mask = ((1 << width.get(i)) - 1) << offset.get(i);
        ret = (src.get(i) & mask) >> offset.get(i);
        retv(i) = ret;
    }

    return retv;
}

template<typename RT, typename T1, typename T2, typename T3, uint SZ>
CM_API vector<RT, SZ>
cm_bf_extract(const T1& width, const T2& offset,
              const stream<T3,SZ>& src,
              const typename uint_type<T3, T1>::type flags = 0)
{
    vector<T1, SZ> v1(width);
    vector<T2, SZ> v2(offset);
    vector<T3, SZ> v3;
    vector<RT, SZ> retv;

    int i;
    for (i = 0; i < SZ; i++) {
        v3(i) = src.get(i);
    }

    retv = cm_bf_extract<RT>(v1, v2, v3, flags);

    return retv;
}

template<typename RT, typename T1, typename T2, typename T3>
CM_API RT
cm_bf_extract(const T1& width, const T2& offset,
              const T3& src,
              const typename uint_type<T3, T1>::type flags = 0)
{
    vector<T1, 1> v1(width);
    vector<T2, 1> v2(offset);
    vector<T3, 1> v3(src);
    vector<RT, 1> retv;

    retv = cm_bf_extract<RT>(v1, v2, v3, flags);

    return retv;
}

// Reverse src bitfield
template<typename RT, typename T1, uint SZ>
CM_API vector<RT, SZ>
cm_bf_reverse(const stream<T1,SZ>& src,
              const uint flags = 0)
{
    static const bool conformable1 = dwordtype<T1>::value;
    static const bool conformable5 = dwordtype<RT>::value;

    int i,j;
    vector<RT, SZ> retv;

    for (i = 0; i < SZ; i++) {
        SIMDCF_ELEMENT_SKIP(i);
        RT input = src.get(i);
        RT output = 0;
        for (j = 0; j < sizeof(RT) * 8; j++) {
            output |= input & 0x1;

            // Don't shift if this was the last one
            if ((j+1) < (sizeof(RT) * 8)) {
                output <<= 1;
                input >>= 1;
            }
        }
        retv(i) = output;
    }

    return retv;
}

// Convert from short to float32 (maps to hardware instruction)
// where the short is in the form of an IEEE float16
// We use short to represent as we don't have a native half type
template<typename T1, uint SZ>
CM_API vector<float, SZ>
cm_f16tof32(const stream<T1, SZ>& src,
            const uint flags = 0)
{
    static const bool conformable1 = wordtype<T1>::value;

    int i;
    vector<float, SZ> retv;

    for (i = 0; i < SZ; i++) {
        SIMDCF_ELEMENT_SKIP(i);
        T1 input = src.get(i);
        //float output = Float16Conversion::tof32(input);
        half val;

        // Need to replace this.
        // val.setBits(input);
        assert(0);
        float output = (float) val;
        retv(i) = output;
    }

    return retv;
}

// Convert from float32 to short (maps to hardware instruction)
// where the short is in the form of an IEEE float16
// We use short to represent as we don't have a native half type
template<typename T1, uint SZ>
CM_API vector<ushort, SZ>
cm_f32tof16(const stream<T1, SZ>& src,
            const uint flags = 0)
{
    static const bool conformable2 = is_fp_type<T1>::value;

    int i;
    vector<ushort, SZ> retv;

    for (i = 0; i < SZ; i++) {
        SIMDCF_ELEMENT_SKIP(i);
        T1 input = src.get(i);
        //ushort output = Float16Conversion::tof16(input);
        half output = half(input);

        // Need to replace it.
        // retv(i) = output.getBits();
        assert(0);
    }

    return retv;
}


CM_API void cm_barrier();
CM_API void cm_sbarrier(uint  flag);
CM_API inline void cm_slm_fence(uint flag) {}

// Type-Checking Templates
template <typename T> struct Allowed_Type_Float_Or_Dword {};
template <> struct Allowed_Type_Float_Or_Dword<int> {
    static const bool value = true;
};
template <> struct Allowed_Type_Float_Or_Dword<uint> {
    static const bool value = true;
};
template <> struct Allowed_Type_Float_Or_Dword<float> {
    static const bool value = true;
};
template <uint N> struct Allowed_Vector_Length_8_Or_16 {};
template <> struct Allowed_Vector_Length_8_Or_16<8> {
    static const bool value = true;
};
template <> struct Allowed_Vector_Length_8_Or_16<16> {
    static const bool value = true;
};

//-------------------------------------------------------------------------
template <class T, int SZ>
void assert_slm_access () {
    static_assert(!((SZ*sizeof(T)) % 16 /*OWord*/), "Vector width must be OWord-aligned.");

}

template <class T, uint SZ>
CM_API void cm_slm_read(uint slmBuffer, vector_ref<ushort, SZ> offsets, vector_ref<T, SZ> dst) {
    assert_slm_access<T,SZ>();
    const auto base = __cm_emu_get_slm() + slmBuffer;
    for (int i = 0; i < SZ; i++) // NB: implemented with Byte-Scattered-Read in Gen7+
        dst(i) = *reinterpret_cast<T*>(base + sizeof(T) * offsets(i));
}

// Catch all real world use cases with vector passed instead of vector_ref to the above documentation-based signature.
template<class OT, template <class,uint> class DT, class T, uint SZ>
CM_API void cm_slm_read(uint slmBuffer, OT offsets, DT<T, SZ>& dst)
{ cm_slm_read(slmBuffer,vector_ref<ushort, SZ>{offsets},vector_ref<T, SZ>{dst}); }

template <class T, uint SZ>
CM_API void cm_slm_block_read(uint slmBuffer, int offset, vector_ref<T,SZ> dst) {
    assert_slm_access<T,SZ>();
    const auto base = __cm_emu_get_slm() + slmBuffer + offset;
    for (int i = 0; i < SZ; i++)
        dst(i) = *reinterpret_cast<T*>(base + sizeof(T) * i);
}

// Catch all real world use cases with vector passed instead of vector_ref to the above documentation-based signature.
template<template<class,uint> class DT, class T, uint SZ>
CM_API void cm_slm_block_read(uint slmBuffer, int offset, DT<T,SZ>& dst)
{ cm_slm_block_read(slmBuffer,offset,vector_ref<T, SZ>{dst}); }

enum SLM_ChannelMaskType
{
    SLM_R_ENABLE         = 14,
    SLM_G_ENABLE         = 13,
    SLM_GR_ENABLE        = 12,
    SLM_B_ENABLE         = 11,
    SLM_BR_ENABLE        = 10,
    SLM_BG_ENABLE        = 9,
    SLM_BGR_ENABLE       = 8,
    SLM_A_ENABLE         = 7,
    SLM_AR_ENABLE        = 6,
    SLM_AG_ENABLE        = 5,
    SLM_AGR_ENABLE       = 4,
    SLM_AB_ENABLE        = 3,
    SLM_ABR_ENABLE       = 2,
    SLM_ABG_ENABLE       = 1,
    SLM_ABGR_ENABLE      = 0
};

template <typename T1, typename T, uint M>
inline void
cm_slm_read4_internal (uint slmBuffer, T1 v_Addr, vector_ref<T,M> v_Dst, SLM_ChannelMaskType mask, uint N, uint M1)
{
    //std::lock_guard<std::mutex> lock(m_mutex);

    static const bool conformable1 = Allowed_Type_Float_Or_Dword<T>::value;
    char *baseOffset, *byteOffset;
    char numColors=0, color[4]={0,0,0,0}, colorNext=0;
    baseOffset = __cm_emu_get_slm() + slmBuffer;
    // mask = mask & 0x1111;
    if (!(mask & 0x1)) {color[0]=1; numColors++;}
    if (!(mask & 0x2)) {color[1]=1; numColors++;}
    if (!(mask & 0x4)) {color[2]=1; numColors++;}
    if (!(mask & 0x8)) {color[3]=1; numColors++;}
    if (numColors == 0) {
        fprintf(stderr, "cm_slm_read4 error: At least one"
                "destination vector has to be read!\n");
        exit(EXIT_FAILURE);
    }
    if (M1 < numColors*N) {
        fprintf(stderr, "cm_slm_read4 error: destination vector"
                "does not have enough space to hold data\n");
        exit(EXIT_FAILURE);
    }
    for (uint j=0; j<4; j++) {
        if (color[j] == 0) continue;
        for (uint i=0; i<N; i++) {
            byteOffset = baseOffset +  sizeof(T) * (v_Addr(i) + j);
            v_Dst(i+colorNext*N) = *( (T *)byteOffset );
        }
        colorNext++;
    }
}

template <typename T, uint N, uint M>
CM_API void
cm_slm_read4 (uint slmBuffer, vector_ref<ushort,N> v_Addr, vector<T,M> &v_Dst, SLM_ChannelMaskType mask)
{
    vector_ref<T,M> dst = v_Dst;
    cm_slm_read4_internal(slmBuffer, v_Addr, dst, mask, N, M);
}

template <typename T, uint N, uint M>
CM_API void
cm_slm_read4 (uint slmBuffer, vector<ushort,N> &v_Addr, vector_ref<T,M> v_Dst, SLM_ChannelMaskType mask)
{
    vector_ref<T,M> dst = v_Dst;
    cm_slm_read4_internal(slmBuffer, v_Addr, dst, mask, N, M);
}

template <typename T, uint N, uint M>
CM_API void
cm_slm_read4 (uint slmBuffer, vector_ref<ushort,N> v_Addr, vector_ref<T,M> v_Dst, SLM_ChannelMaskType mask)
{
    vector_ref<T,M> dst = v_Dst;
    cm_slm_read4_internal(slmBuffer, v_Addr, dst, mask, N, M);
}

template <typename T, uint N, uint M>
CM_API void
cm_slm_read4 (uint slmBuffer, vector<ushort,N> &v_Addr, vector<T,M> &v_Dst, SLM_ChannelMaskType mask)
{
    vector_ref<T,M> dst = v_Dst;
    cm_slm_read4_internal(slmBuffer, v_Addr, dst, mask, N, M);
}

//---------------------------------
template <typename T, uint N, uint M>
CM_API void
cm_slm_read4 (uint slmBuffer, vector_ref<ushort,N> v_Addr, vector<T,M> &v_Dst, int imask)
{
    vector_ref<T,M> dst = v_Dst;
    SLM_ChannelMaskType mask = (SLM_ChannelMaskType) imask;
    cm_slm_read4_internal(slmBuffer, v_Addr, dst, mask, N, M);
}

template <typename T, uint N, uint M>
CM_API void
cm_slm_read4 (uint slmBuffer, vector<ushort,N> &v_Addr, vector_ref<T,M> v_Dst, int imask)
{
    vector_ref<T,M> dst = v_Dst;
    SLM_ChannelMaskType mask = (SLM_ChannelMaskType) imask;
    cm_slm_read4_internal(slmBuffer, v_Addr, dst, mask, N, M);
}

template <typename T, uint N, uint M>
CM_API void
cm_slm_read4 (uint slmBuffer, vector_ref<ushort,N> v_Addr, vector_ref<T,M> v_Dst, int imask)
{
    vector_ref<T,M> dst = v_Dst;
    SLM_ChannelMaskType mask = (SLM_ChannelMaskType) imask;
    cm_slm_read4_internal(slmBuffer, v_Addr, dst, mask, N, M);
}

template <typename T, uint N, uint M>
CM_API void
cm_slm_read4 (uint slmBuffer, vector<ushort,N> &v_Addr, vector<T,M> &v_Dst, int imask)
{
    vector_ref<T,M> dst = v_Dst;
    SLM_ChannelMaskType mask = (SLM_ChannelMaskType) imask;
    cm_slm_read4_internal(slmBuffer, v_Addr, dst, mask, N, M);
}

// Write the vector 'v_Src' to the SLM buffer 'slmBuffer' at the
// addresses given in 'v_Addr'
// Implement with Byte-Scattered-Write in Gen7+
template <typename T, uint N> // Supported: N = 8 or 16; T = Byte, Word, or Dword
CM_API void
cm_slm_write (uint  slmBuffer,           // SLM buffer
              vector<ushort, N> &v_Addr, // Byte-Offsets into SLM Buffer
              vector<T, N>      &v_Src   // Data vector to be written to SLM
             )
{
    char *baseOffset, *byteOffset;
    baseOffset = __cm_emu_get_slm() + slmBuffer;
    for (int i=0; i<N; i++) {
        byteOffset = baseOffset +  sizeof(T) * v_Addr(i);
        *( (T *)byteOffset ) = v_Src(i);
    }
}

template <typename T, uint N> // Supported: N = 8 or 16; T = Byte, Word, or Dword
CM_API void
cm_slm_write (uint  slmBuffer,              // SLM buffer
              vector_ref<ushort, N> v_Addr, // Byte-Offsets into SLM Buffer
              vector_ref<T, N>      v_Src   // Data vector to be written to SLM
             )
{
    char *baseOffset, *byteOffset;
    baseOffset = __cm_emu_get_slm() + slmBuffer;
    for (int i=0; i<N; i++) {
        byteOffset = baseOffset +  sizeof(T) * v_Addr(i);
        *( (T *)byteOffset ) = v_Src(i);
    }
}

template <typename T, uint N> // Supported: N = 8 or 16; T = Byte, Word, or Dword
CM_API void
cm_slm_write (uint  slmBuffer,               // SLM buffer
              vector<ushort, N>     &v_Addr, // Byte-Offsets into SLM Buffer
              vector_ref<T, N>      v_Src    // Data vector to be written to SLM
             )
{
    char *baseOffset, *byteOffset;
    baseOffset = __cm_emu_get_slm() + slmBuffer;
    for (int i=0; i<N; i++) {
        byteOffset = baseOffset +  sizeof(T) * v_Addr(i);
        *( (T *)byteOffset ) = v_Src(i);
    }
}

template <typename T, uint N> // Supported: N = 8 or 16; T = Byte, Word, or Dword
CM_API void
cm_slm_write (uint  slmBuffer,              // SLM buffer
              vector_ref<ushort, N> v_Addr, // Byte-Offsets into SLM Buffer
              vector<T, N>          &v_Src  // Data vector to be written to SLM
             )
{
    char *baseOffset, *byteOffset;
    baseOffset = __cm_emu_get_slm() + slmBuffer;
    for (int i=0; i<N; i++) {
        byteOffset = baseOffset +  sizeof(T) * v_Addr(i);
        *( (T *)byteOffset ) = v_Src(i);
    }
}

// Block write the vector 'v_Src' to the SLM buffer 'slmBuffer' at the
// addresses given in 'v_Addr'
// Implement with Byte-Scattered-Write in Gen7+
template <typename T, uint N> // Supported: N = 8 or 16; T = Byte, Word, or Dword
CM_API void
cm_slm_block_write(uint  slmBuffer,              // SLM buffer
    int v_Addr, // Byte-Offsets into SLM Buffer
    vector_ref<T, N>      v_Src   // Data vector to be written to SLM, the width of vector v can be only 1, 2, 4 or 8 OWords.
)
{
    char *baseOffset, *byteOffset;
    baseOffset = __cm_emu_get_slm() + slmBuffer;
    for (int i = 0; i < N; i++) {
        byteOffset = baseOffset + v_Addr + sizeof(T) * i;
        *((T *)byteOffset) = v_Src(i);
    }
}

template <typename T, uint N> // Supported: N = 8 or 16; T = Byte, Word, or Dword
CM_API void
cm_slm_block_write(uint  slmBuffer,              // SLM buffer
    int v_Addr, // Byte-Offsets into SLM Buffer
    vector<T, N>          &v_Src  // Data vector to be written to SLM, the width of vector v can be only 1, 2, 4 or 8 OWords.
)
{
    char *baseOffset, *byteOffset;
    baseOffset = __cm_emu_get_slm() + slmBuffer;
    for (int i = 0; i < N; i++) {
        byteOffset = baseOffset + v_Addr + sizeof(T) * i;
        *((T *)byteOffset) = v_Src(i);
    }
}

template <class T1, class T2, uint AddrCount, uint SrcElCount, class MaskT>
void cm_slm_write4 (
    uint slmBuffer,
    const stream<T1 /*
        ... no comments.
    */, AddrCount>& v_Addr,
    const stream<T2,SrcElCount>& v_Src,
    MaskT mask)
{
    //constexpr bool conformable1 = Allowed_Type_Float_Or_Dword<T2>::value;
    // const auto mask = static_cast<SLM_ChannelMaskType> (mask);
    char numColors=0, color[4]={0,0,0,0};

    if (!(mask & 0x1)) {color[0]=1; numColors++;}
    if (!(mask & 0x2)) {color[1]=1; numColors++;}
    if (!(mask & 0x4)) {color[2]=1; numColors++;}
    if (!(mask & 0x8)) {color[3]=1; numColors++;}

    if (numColors == 0) {

        fprintf(stdout, "cm_slm_read4 error: At least one"
                "destination vector has to be read!\n");
        exit(EXIT_FAILURE);
    }

    if (SrcElCount < numColors*AddrCount) {
        fprintf(stdout, "cm_slm_read4 error: destination vector"
                "does not have enough space to hold data\n");
         exit(EXIT_FAILURE);
    }

    const auto baseOffset = reinterpret_cast<T2*>(__cm_emu_get_slm() + slmBuffer);
    int color_src_i = 0;
    for (int color_i=0; color_i < 4; color_i++)
    {
        if (!color[color_i])
            continue;

        for (uint addr_i = 0; addr_i < AddrCount; addr_i++)
            *(baseOffset + (v_Addr.get(addr_i) + color_i)) =
                v_Src.get(addr_i + color_src_i * AddrCount);

        color_src_i++;
    }
}

template <typename T, uint R, uint C>
bool
read_local(SurfaceIndex & buf_id, int x_pos, int y_pos, int block_width, int block_height, int num_block, int data_size, int transpose_type, matrix_ref<T, R, C> &in)
{
    uint i, j, m;
    uint offset;
    cm_list<CmEmulSys::iobuffer>::iterator buff_iter =
        CmEmulSys::search_buffer(buf_id.get_data() & 0xFF);

    if (buff_iter == CmEmulSys::iobuffers.end()) {
        printf("Error reading buffer %d: buffer %d is not registered!\n", buf_id.get_data() & 0xFF, buf_id.get_data() & 0xFF);
        exit(EXIT_FAILURE);
    }

    int width = buff_iter->width;
    int height = buff_iter->height;

    int x_pos_a, y_pos_a;  /* Actual positions */
    int sizeofT = sizeof(T); /* Make this into a signed integer */
    uint bpp = CM_genx_bytes_per_pixel(buff_iter->pixelFormat);
    uint block_width_in_bytes = block_width * data_size * num_block;
    assert(((bpp - 1) & bpp) == 0); // Is power-of-2 number
    assert(bpp <= 4);
    assert(R * C * sizeofT >= block_width * block_height * data_size);

    vector<uchar, 4096> data; //allocate the max size to read the data from surface
    for (i = 0; i < block_height; i++) {
        y_pos_a = y_pos + i;
        if (y_pos_a > height - 1 || y_pos_a < 0) {
            continue;
        }
        for (j = 0; j < block_width_in_bytes; j++) {
            x_pos_a = x_pos + j ;
            if (x_pos_a > width - 1 || x_pos_a < 0) {
                continue;
            }

            offset = y_pos_a * width + x_pos_a; //assume width and pitch is the same.

            {
                data(i * block_width_in_bytes + j) = *((char*)buff_iter->p + offset);
            }

        }
    }

    if (transpose_type == 0) // no transpose
    {
        for (m = 0; m < num_block; m++)
        {
            for (i = 0; i < block_height; i++)
            {
                for (j = 0; j < block_width * data_size; j++)
                {
                    //copy data per byte block by block
                    in.template format<uchar>()(m * block_width * data_size * block_height + i * block_width * data_size + j) =
                        data(i * block_width * data_size * num_block + m * block_width * data_size + j);
                }
            }
        }
    }
    else if (transpose_type == 1) //DW tranpose
    {
        for (m = 0; m < num_block; m++)
        {
            for (j = 0; j < block_width * data_size / 4; j++)
            {
                for (i = 0; i < block_height; i++)
                {
                    in.template format<uint>()(m * block_width * data_size * block_height / 4 + j * block_height + i) =
                        data.template format<uint>()(i * block_width * data_size * num_block / 4 + m * block_width * data_size / 4 + j);
                }
            }
        }
    }
    else if (transpose_type == 2)//per DW transpose
    {
        for (m = 0; m < num_block; m++)
        {
            for (i = 0; i < block_height / (4 / sizeofT); i++)
            {
                for (j = 0; j < block_width / sizeofT * (4 / sizeofT); j++)
                {
                    in.template format<T>()(m * block_width * data_size * block_height / sizeofT + i * block_width * data_size / sizeofT * (4 / sizeofT) + j) =
                        data.template format<T>()((i * (4 / sizeofT) + j % (4 / sizeofT)) * block_width * data_size * num_block / sizeofT + m * block_width * data_size / sizeofT + j / (4 / sizeofT));
                }
            }
        }
    }
    else
    {
        fprintf(stderr, "this transpose is not supported in emulation mode.\n");
        exit(EXIT_FAILURE);
    }
    return true;
}

inline void __cm_send_unsupported()
{
    std::cerr << "cm_send is not supported in emulation mode." << std::endl;
    exit(EXIT_FAILURE);
}

template <class R, class M>
void send_local(
    R rspVar,
    M msgVar,
    unsigned int exDesc, unsigned int msgDesc, unsigned int sendc)
{
    uchar bti = msgDesc & 0xFF;
    uchar flag_rd_wr = (msgDesc >> 14) & 0x1F;
    uchar target_func_id = exDesc & 0xF;

    // A hack for prefetch.
    if (exDesc & 0x80000000)
        return;

    if (bti == 0xFE) //SLM
    {
        if (flag_rd_wr == 0x0) //slm block read
        {
            uint readlen = (msgDesc >> 20) & 0x1F;
            uint read_offset = msgVar.template format<uint>()(2);
            cm_slm_block_read(0,           // SLM buffer
                read_offset, // Byte-Offsets into SLM Buffer
                rspVar.template format<uint>()   // Data vector to be written from SLM, the width of vector v must be OWord (i.e. 16 bytes) aligned.
            );
            return;
        }
        else if (flag_rd_wr == 0x8) //slm block write
        {

        }
    }
    else if (bti < 0xEF) //global mem access
    {
        if (flag_rd_wr == 0x0 || flag_rd_wr == 0x1) //block read
        {
            uint readlen = (msgDesc >> 20) & 0x1F;
            uint read_offset = msgVar.template format<uint>()(2);
            SurfaceIndex sIndex(bti);
            read(sIndex, read_offset, rspVar.template format<uint>());
            return;
        }
        else if (flag_rd_wr == 0x4 && target_func_id == 0xA) //2d block read
        {
            uint block_msg_control = msgVar.template format<uint>()(2);
            uint block_height = (block_msg_control >> 16) & 0x3F;
            uint block_width = block_msg_control & 0x3F;
            uint num_block = (block_msg_control>>6) & 0x3;
            int xoffset = msgVar.template format<int>()(0);
            int yoffset = msgVar.template format<int>()(1);
            int transpose_type = (msgDesc >> 11) & 0x3;
            int data_size = 1<<((exDesc >> 29) & 0x3);
            SurfaceIndex sIndex(bti);
            read_local(sIndex, xoffset, yoffset, block_width, block_height, num_block, data_size, transpose_type, rspVar);
            return;
        }
        else if (flag_rd_wr == 0x8) //slm block write
        {

        }
    }

    __cm_send_unsupported();
}

template <class M>
void send_local(int rspVar, M msgVar, unsigned int exDesc, unsigned int msgDesc, unsigned int sendc)
{
    // A hack for prefetch.
    if (exDesc & 0x8000)
        return;

    uchar bti = msgDesc & 0xFF;
    uchar flag_rd_wr = (msgDesc >> 14) & 0x1F;
    uchar target_func_id = exDesc & 0xF;

    __cm_send_unsupported();
}

template <typename T, uint R, uint C>
CM_API void
cm_send(int rspVar,  matrix<T,R,C> msgVar, unsigned int exDesc, unsigned int msgDesc, unsigned int sendc)
{
    send_local(rspVar, msgVar, exDesc, msgDesc, sendc);
}

template <typename T1, uint R1, uint C1, typename T2, uint R2, uint C2>
CM_API void
cm_send(matrix<T1, R1, C1> &rspVar, matrix<T2, R2, C2> &msgVar, unsigned int exDesc, unsigned int msgDesc, unsigned int sendc)
{
    send_local(rspVar, msgVar.select_all(), exDesc, msgDesc, sendc);
}

template <typename T1, uint R1, uint C1, typename T2, uint R2, uint C2>
CM_API void
cm_send(matrix_ref<T1, R1, C1> rspVar, matrix_ref<T2, R2, C2> msgVar, unsigned int exDesc, unsigned int msgDesc, unsigned int sendc)
{
    send_local(rspVar, msgVar, exDesc, msgDesc, sendc);
}

template <typename T1, uint R1, uint C1, typename T2, uint R2, uint C2, typename T3, uint R3, uint C3>
void
sends_local(matrix_ref<T1, R1, C1> rspVar, matrix_ref<T2, R2, C2> msgVar, matrix_ref<T3, R3, C3> msg2Var, unsigned int exDesc, unsigned int msgDesc, unsigned int sendc)
{
    uchar bti = msgDesc & 0xFF;
    uchar flag_rd_wr = (msgDesc >> 14) & 0x1F;
    uchar target_func_id = exDesc & 0xF;

    if (exDesc & 0x80000000) return; // it is a hack for prefetch

    if (bti == 0xFE) //SLM
    {
        if (flag_rd_wr == 0x0) //slm block read
        {
            uint readlen = (msgDesc >> 20) & 0x1F;
            uint read_offset = msgVar.template format<uint>()(2);
            cm_slm_block_read(0,           // SLM buffer
                read_offset, // Byte-Offsets into SLM Buffer
                rspVar.template format<uint>()   // Data vector to be written from SLM, the width of vector v must be OWord (i.e. 16 bytes) aligned.
            );

            return;
        }
        else if (flag_rd_wr == 0x8 && target_func_id == 0xa) //slm block write
        {
            uint writelen = (exDesc >> 6) & 0xF;
            uint write_offset = msgVar.template format<uint>()(2);
            cm_slm_block_write(0,           // SLM buffer
                write_offset, // Byte-Offsets into SLM Buffer
                msg2Var.template format<uint>()   // Data vector to be written from SLM, the width of vector v must be OWord (i.e. 16 bytes) aligned.
            );

            return;
        }
    }
    else if (bti < 0xEF) //global mem access
    {
        if (flag_rd_wr == 0x0 || flag_rd_wr == 0x1) //block read
        {
            uint readlen = (msgDesc >> 20) & 0x1F;
            uint read_offset = msgVar.template format<uint>()(2);
            SurfaceIndex sIndex(bti);
            read(sIndex, read_offset, rspVar);

            return;
        }
        else if (flag_rd_wr == 0x8 && target_func_id == 0xa) //slm block write
        {
            uint writelen = (exDesc >> 6) & 0xF;
            uint write_offset = msgVar.template format<uint>()(2);
            SurfaceIndex sIndex(bti);
            write(sIndex, write_offset, msg2Var);

            return;
        }
        else if (flag_rd_wr == 0xa && target_func_id == 0xa) //2-d block write
        {
            uint block_msg_control = msgVar.template format<uint>()(2);
            uint block_height = (block_msg_control >> 16) & 0x3F;
            uint block_witdth = block_msg_control & 0x3F;
            int xoffset = msgVar.template format<int>()(0);
            int yoffset = msgVar.template format<int>()(1);
            SurfaceIndex sIndex(bti);
            write(sIndex, xoffset, yoffset, msg2Var);

            return;
        }
    }

    __cm_send_unsupported();
}

template <typename T, uint R, uint C, typename T3, uint R3, uint C3>
void
sends_local(int rspVar, matrix_ref<T, R, C> msgVar, matrix_ref<T3, R3, C3> msg2Var, unsigned int exDesc, unsigned int msgDesc, unsigned int sendc)
{
    uchar bti = msgDesc & 0xFF;
    uchar flag_rd_wr = (msgDesc >> 14) & 0x1F;
    uchar target_func_id = exDesc & 0xF;

    if (exDesc & 0x80000000) return; // it is a hack for prefetch

    if (bti == 0xFE) //SLM
    {
        if (flag_rd_wr == 0x8 && target_func_id == 0xa) //slm block write
        {
            uint writelen = (exDesc >> 6) & 0xF;
            uint write_offset = msgVar.template format<uint>()(2);
            cm_slm_block_write(0,           // SLM buffer
                write_offset, // Byte-Offsets into SLM Buffer
                msg2Var.template format<uint>()   // Data vector to be written from SLM, the width of vector v must be OWord (i.e. 16 bytes) aligned.
            );

            return;
        }
    }
    else if (bti < 0xEF) //global mem access
    {
        if (flag_rd_wr == 0x8 && target_func_id == 0xa) //slm block write
        {
            uint writelen = (exDesc >> 6) & 0xF;
            uint write_offset = msgVar.template format<uint>()(2);
            SurfaceIndex sIndex(bti);
            write(sIndex, write_offset, msg2Var.template format<uint>());

            return;
        }
        else if (flag_rd_wr == 0xa && target_func_id == 0xa) //2-d block write
        {
            uint block_msg_control = msgVar.template format<uint>()(2);
            uint block_height = (block_msg_control >> 16) & 0x3F;
            uint block_witdth = block_msg_control & 0x3F;
            int xoffset = msgVar.template format<int>()(0);
            int yoffset = msgVar.template format<int>()(1);
            SurfaceIndex sIndex(bti);
            write(sIndex, xoffset, yoffset, msg2Var);

            return;
        }
    }

    __cm_send_unsupported();
}

template <typename T1, uint R1, uint C1, typename T2, uint R2, uint C2, typename T3, uint R3, uint C3>
CM_API void
cm_sends(matrix<T1, R1, C1> &rspVar, matrix<T2, R2, C2> &msgVar, matrix<T3, R3, C3> &msg2Var, unsigned int exDesc, unsigned int msgDesc, unsigned int sendc)
{
    sends_local(rspVar.select_all(), msgVar.select_all(), msg2Var.select_all(), exDesc, msgDesc, sendc);
}

template <typename T1, uint R1, uint C1, typename T2, uint R2, uint C2, typename T3, uint R3, uint C3>
CM_API void
cm_sends(matrix_ref<T1, R1, C1> rspVar, matrix_ref<T2, R2, C2> msgVar, matrix_ref<T3, R3, C3> msg2Var, unsigned int exDesc, unsigned int msgDesc, unsigned int sendc)
{
    sends_local(rspVar, msgVar, msg2Var, exDesc, msgDesc, sendc);
}

template <typename T, uint R, uint C, typename T3, uint R3, uint C3>
CM_API void
cm_sends(int rspVar, matrix_ref<T, R, C> msgVar, matrix_ref<T3, R3, C3> msg2Var, unsigned int exDesc, unsigned int msgDesc, unsigned int sendc)
{
    sends_local(rspVar, msgVar, msg2Var, exDesc, msgDesc, sendc);
}

template <typename T>
CM_API unsigned int
cm_get_value(T &index)
{
    return index.get_data();
}

template <typename T>
CM_API vector<T, 8>
cm_get_r0( )
{
    fprintf(stderr, "cm_get_r0 is not supported in emulation mode.\n");
    exit(EXIT_FAILURE);
    return {};
}

template <typename T>
CM_API vector<T, 4>
cm_get_sr0( )
{
    fprintf(stderr, "cm_get_sr0 is not supported in emulation mode.\n");
    exit(EXIT_FAILURE);
    return {};
}

template <typename T>
CM_API void
cm_label(const char *pLabel)
{
    fprintf(stderr, "\nInsert label %s:\n", pLabel);
}

template <typename T>
CM_API void
cm_label(const char *pLabel, int i)
{
    fprintf(stderr, "\nInsert label %s_%d:\n", pLabel, i);
}

template <typename T>
CM_API void
cm_label(const char *pLabel, int i, int j)
{
    fprintf(stderr, "\nInsert label %s_%d_%d\n", pLabel, i, j);
}

template <typename T>
CM_API void
cm_label(const char *pLabel, int i, int j, int k)
{
    fprintf(stderr, "\nInsert label %s_%d_%d_%d:\n", pLabel, i, j, k);
}

// Implementation of SLM atomic operations

/*
typedef enum _AtomicOpType_
{
    CM_ATOMIC_ADD                     = 0x0,
    CM_ATOMIC_SUB                     = 0x1,
    CM_ATOMIC_INC                     = 0x2,
    CM_ATOMIC_DEC                     = 0x3,
    CM_ATOMIC_MIN                     = 0x4,
    CM_ATOMIC_MAX                     = 0x5,
    CM_ATOMIC_XCHG                    = 0x6,
    CM_ATOMIC_CMPXCHG                 = 0x7,
    CM_ATOMIC_AND                     = 0x8,
    CM_ATOMIC_OR                      = 0x9,
    CM_ATOMIC_XOR                     = 0xa,
    CM_ATOMIC_MINSINT                 = 0xb,
    CM_ATOMIC_MAXSINT                 = 0xc
} AtomicOpType;
*/

typedef enum _CmSLMAtomicOpType_
{
    // ATOMIC_CMPWR8B = 0x0 is not supported for SLM
    SLM_ATOMIC_AND                     = 0x1,
    SLM_ATOMIC_OR                      = 0x2,
    SLM_ATOMIC_XOR                     = 0x3,
    SLM_ATOMIC_MOV                     = 0x4,
    SLM_ATOMIC_INC                     = 0x5,
    SLM_ATOMIC_DEC                     = 0x6,
    SLM_ATOMIC_ADD                     = 0x7,
    SLM_ATOMIC_SUB                     = 0x8,
    SLM_ATOMIC_REVSUB                  = 0x9,
    SLM_ATOMIC_IMAX                    = 0xa,
    SLM_ATOMIC_IMIN                    = 0xb,
    SLM_ATOMIC_UMAX                    = 0xc,
    SLM_ATOMIC_UMIN                    = 0xd,
    SLM_ATOMIC_CMPWR                   = 0xe,
    SLM_ATOMIC_PREDEC                  = 0xf
} CmSLMAtomicOpType;

// This funtion performs atomic scattered DWord write to SLM

template <typename T, uint N>
inline void
cm_slm_atomic_internal(uint slmBuffer, CmSLMAtomicOpType op, vector<ushort,N> &v_Addr,
                       vector<T,N> &v_Src0, vector<T,N> &v_Src1, vector<T,N> &v_Dst)
{
    static const bool conformable1 = Allowed_Type_Float_Or_Dword<T>::value;
    static const bool conformable2 = Allowed_Vector_Length_8_Or_16<N>::value;

    int    i;
    char   *baseOffset, *byteOffset;
    uint   *uintPtr;
    int    *intPtr;

    baseOffset = __cm_emu_get_slm() + slmBuffer;

    // InitializeCriticalSection(&dataport_cs);

    for (i = 0; i < N; i++) {

        SIMDCF_ELEMENT_SKIP(i);

        byteOffset = baseOffset +  sizeof(T) * v_Addr(i);
        uintPtr    = (uint *) byteOffset;
        intPtr     = (int *) byteOffset;
        v_Dst(i)   = *uintPtr;

        // To Do: How to handle out-of-bound accesses to SLM for atomic writes?

        switch (op) {
            case SLM_ATOMIC_AND:
                *uintPtr = *uintPtr & v_Src0(i);
                break;
            case SLM_ATOMIC_OR:
                *uintPtr = *uintPtr | v_Src0(i);
                break;
            case SLM_ATOMIC_XOR:
                *uintPtr = *uintPtr ^ v_Src0(i);
                break;
            case SLM_ATOMIC_MOV:
                *uintPtr = (uint) v_Src0(i);
                break;
            case SLM_ATOMIC_INC:
                *uintPtr = *uintPtr + 1;
                break;
            case SLM_ATOMIC_DEC:
                *uintPtr = *uintPtr - 1;
                break;
            case SLM_ATOMIC_ADD:
                *uintPtr = *uintPtr + (uint) v_Src0(i);
                break;
            case SLM_ATOMIC_SUB:
                *uintPtr = *uintPtr - (uint) v_Src0(i);
                break;
            case SLM_ATOMIC_REVSUB:
                *uintPtr = (uint) v_Src0(i) - *uintPtr;
                break;
            case SLM_ATOMIC_IMAX:
                *intPtr = (*intPtr > (int) v_Src0(i)) ? *intPtr : (int) v_Src0(i);
                break;
            case SLM_ATOMIC_IMIN:
                *intPtr = (*intPtr > (int) v_Src0(i)) ? (int) v_Src0(i) : *intPtr;
                break;
            case SLM_ATOMIC_UMAX:
                *uintPtr = (*uintPtr > (uint) v_Src0(i)) ? *uintPtr : (uint) v_Src0(i);
                break;
            case SLM_ATOMIC_UMIN:
                *uintPtr = (*uintPtr < (uint) v_Src0(i)) ? *uintPtr : (uint) v_Src0(i);
                break;
            case SLM_ATOMIC_CMPWR:
                *uintPtr = (*uintPtr == (uint) v_Src0(i)) ? (uint) v_Src1(i) : *uintPtr;
                break;
            case SLM_ATOMIC_PREDEC:
                *uintPtr = *uintPtr - 1;
                v_Dst(i) = *uintPtr;
                break;
            default:
                printf("Error writing SLM: invalid opcode for SLM atomic write!\n");
                exit(EXIT_FAILURE);
        }
    }

    // DeleteCriticalSection(&dataport_cs);
}

//------------------------------------------------------------------------------
template <typename T, uint N>
CM_API void
cm_slm_atomic_generic(uint slmBuffer, CmSLMAtomicOpType op, vector_ref<ushort,N> v_Addr,
              vector_ref<T,N> v_Src0, vector_ref<T,N> v_Src1, vector_ref<T,N> v_Dst)
{
    vector<T,N> dst = v_Dst;
    vector<T,N> src0 = v_Src0;
    vector<T,N> src1 = v_Src1;
    vector<T,N> addr = v_Addr;
    cm_slm_atomic_internal(slmBuffer, op, addr, src0, src1, dst);
}

template <typename T, uint N>
CM_API void
cm_slm_atomic_generic(uint slmBuffer, CmSLMAtomicOpType op, vector_ref<ushort,N> v_Addr,
              vector_ref<T,N> v_Src0, vector_ref<T,N> v_Src1, vector<T,N> &v_Dst)
{
    vector<T,N> src0 = v_Src0;
    vector<T,N> src1 = v_Src1;
    vector<T,N> addr = v_Addr;
    cm_slm_atomic_internal(slmBuffer, op, addr, src0, src1, v_Dst);
}

template <typename T, uint N>
CM_API void
cm_slm_atomic_generic(uint slmBuffer, CmSLMAtomicOpType op, vector_ref<ushort,N> v_Addr,
              vector_ref<T,N> v_Src0, vector<T,N> &v_Src1, vector_ref<T,N> v_Dst)
{
    vector<T,N> dst = v_Dst;
    vector<T,N> src0 = v_Src0;
    vector<T,N> addr = v_Addr;
    cm_slm_atomic_internal(slmBuffer, op, addr, src0, v_Src1, dst);
}

template <typename T, uint N>
CM_API void
cm_slm_atomic_generic(uint slmBuffer, CmSLMAtomicOpType op, vector_ref<ushort,N> v_Addr,
              vector_ref<T,N> v_Src0, vector<T,N> &v_Src1, vector<T,N> &v_Dst)
{
    vector<T,N> src0 = v_Src0;
    vector<T,N> addr = v_Addr;
    cm_slm_atomic_internal(slmBuffer, op, addr, src0, v_Src1, v_Dst);
}

template <typename T, uint N>
CM_API void
cm_slm_atomic_generic(uint slmBuffer, CmSLMAtomicOpType op, vector_ref<ushort,N> v_Addr,
              vector<T,N> &v_Src0, vector_ref<T,N> v_Src1, vector_ref<T,N> v_Dst)
{
    vector<T,N> dst = v_Dst;
    vector<T,N> src1 = v_Src1;
    vector<T,N> addr = v_Addr;
    cm_slm_atomic_internal(slmBuffer, op, addr, v_Src0, src1, dst);
}

template <typename T, uint N>
CM_API void
cm_slm_atomic_generic(uint slmBuffer, CmSLMAtomicOpType op, vector_ref<ushort,N> v_Addr,
              vector<T,N> &v_Src0, vector_ref<T,N> v_Src1, vector<T,N> &v_Dst)
{
    vector<T,N> src1 = v_Src1;
    vector<T,N> addr = v_Addr;
    cm_slm_atomic_internal(slmBuffer, op, addr, v_Src0, src1, v_Dst);
}

template <typename T, uint N>
CM_API void
cm_slm_atomic_generic(uint slmBuffer, CmSLMAtomicOpType op, vector_ref<ushort,N> v_Addr,
              vector<T,N> &v_Src0, vector<T,N> &v_Src1, vector_ref<T,N> v_Dst)
{
    vector<T,N> dst = v_Dst;
    vector<T,N> addr = v_Addr;
    cm_slm_atomic_internal(slmBuffer, op, addr, v_Src0, v_Src1, dst);
}

template <typename T, uint N>
CM_API void
cm_slm_atomic_generic(uint slmBuffer, CmSLMAtomicOpType op, vector_ref<ushort,N> v_Addr,
              vector<T,N> &v_Src0, vector<T,N> &v_Src1, vector<T,N> &v_Dst)
{
    vector<T,N> addr = v_Addr;
    cm_slm_atomic_internal(slmBuffer, op, addr, v_Src0, v_Src1, v_Dst);
}
//-----------------------------------------------------------------------------

template <typename T, uint N>
CM_API void
cm_slm_atomic_generic(uint slmBuffer, CmSLMAtomicOpType op, vector<ushort,N> &v_Addr,
              vector_ref<T,N> v_Src0, vector_ref<T,N> v_Src1, vector_ref<T,N> v_Dst)
{
    vector<T,N> dst = v_Dst;
    vector<T,N> src0 = v_Src0;
    vector<T,N> src1 = v_Src1;
    cm_slm_atomic_internal(slmBuffer, op, v_Addr, src0, src1, dst);
}

template <typename T, uint N>
CM_API void
cm_slm_atomic_generic(uint slmBuffer, CmSLMAtomicOpType op, vector<ushort,N> &v_Addr,
              vector_ref<T,N> v_Src0, vector_ref<T,N> v_Src1, vector<T,N> &v_Dst)
{
    vector<T,N> src0 = v_Src0;
    vector<T,N> src1 = v_Src1;
    cm_slm_atomic_internal(slmBuffer, op, v_Addr, src0, src1, v_Dst);
}

template <typename T, uint N>
CM_API void
cm_slm_atomic_generic(uint slmBuffer, CmSLMAtomicOpType op, vector<ushort,N> &v_Addr,
              vector_ref<T,N> v_Src0, vector<T,N> &v_Src1, vector_ref<T,N> v_Dst)
{
    vector<T,N> dst = v_Dst;
    vector<T,N> src0 = v_Src0;
    cm_slm_atomic_internal(slmBuffer, op, v_Addr, src0, v_Src1, dst);
}

template <typename T, uint N>
CM_API void
cm_slm_atomic_generic(uint slmBuffer, CmSLMAtomicOpType op, vector<ushort,N> &v_Addr,
              vector_ref<T,N> v_Src0, vector<T,N> &v_Src1, vector<T,N> &v_Dst)
{
    vector<T,N> src0 = v_Src0;
    cm_slm_atomic_internal(slmBuffer, op, v_Addr, src0, v_Src1, v_Dst);
}

template <typename T, uint N>
CM_API void
cm_slm_atomic_generic(uint slmBuffer, CmSLMAtomicOpType op, vector<ushort,N> &v_Addr,
              vector<T,N> &v_Src0, vector_ref<T,N> v_Src1, vector_ref<T,N> v_Dst)
{
    vector<T,N> dst = v_Dst;
    vector<T,N> src1 = v_Src1;
    cm_slm_atomic_internal(slmBuffer, op, v_Addr, v_Src0, src1, dst);
}

template <typename T, uint N>
CM_API void
cm_slm_atomic_generic(uint slmBuffer, CmSLMAtomicOpType op, vector<ushort,N> &v_Addr,
              vector<T,N> &v_Src0, vector_ref<T,N> v_Src1, vector<T,N> &v_Dst)
{
    vector<T,N> src1 = v_Src1;
    cm_slm_atomic_internal(slmBuffer, op, v_Addr, v_Src0, src1, v_Dst);
}

template <typename T, uint N>
CM_API void
cm_slm_atomic_generic(uint slmBuffer, CmSLMAtomicOpType op, vector<ushort,N> &v_Addr,
              vector<T,N> &v_Src0, vector<T,N> &v_Src1, vector_ref<T,N> v_Dst)
{
    vector<T,N> dst = v_Dst;
    cm_slm_atomic_internal(slmBuffer, op, v_Addr, v_Src0, v_Src1, dst);
}

template <typename T, uint N>
CM_API void
cm_slm_atomic_generic(uint slmBuffer, CmSLMAtomicOpType op, vector<ushort,N> &v_Addr,
              vector<T,N> &v_Src0, vector<T,N> &v_Src1, vector<T,N> &v_Dst)
{
    cm_slm_atomic_internal(slmBuffer, op, v_Addr, v_Src0, v_Src1, v_Dst);
}
//------------------------------------------------------------------------------

static CmSLMAtomicOpType Get_Gen_Atomic_Opcode( CmAtomicOpType aop )
{
    switch (aop) {
    case ATOMIC_ADD:
        return SLM_ATOMIC_ADD;
    case ATOMIC_SUB:
        return SLM_ATOMIC_SUB;
    case ATOMIC_INC:
        return SLM_ATOMIC_INC;
    case ATOMIC_DEC:
        return SLM_ATOMIC_DEC;
    case ATOMIC_MIN:
        return SLM_ATOMIC_UMIN;
    case ATOMIC_MAX:
        return SLM_ATOMIC_UMAX;
    case ATOMIC_XCHG:
        return SLM_ATOMIC_MOV;
    case ATOMIC_CMPXCHG:
        return SLM_ATOMIC_CMPWR;
    case ATOMIC_AND:
        return SLM_ATOMIC_AND;
    case ATOMIC_OR:
        return SLM_ATOMIC_OR;
    case ATOMIC_XOR:
        return SLM_ATOMIC_XOR;
    case ATOMIC_MINSINT:
        return SLM_ATOMIC_IMIN;
    case ATOMIC_MAXSINT:
        return SLM_ATOMIC_IMAX;
    default:
        exit(EXIT_FAILURE);
    }
}

// Top-level atomic functions:
// Either needs two sources, one source, or no sources
// Or two, one or no sources and NULL dst

template <typename T, typename T1, typename T2, typename T3>
CM_API void
cm_slm_atomic(uint slmBuffer, CmAtomicOpType aop, T v_Addr,
              T1 v_Dst, T2 v_Src0, T3 v_Src1)
{
    CmSLMAtomicOpType op = Get_Gen_Atomic_Opcode(aop);

    if (op != SLM_ATOMIC_CMPWR) {
        fprintf(stderr, "Two sources not allowed for the Atomic Operation! \n");
        exit(EXIT_FAILURE);
    }
    cm_slm_atomic_generic(slmBuffer, op, v_Addr, v_Src0, v_Src1, v_Dst);
}

template <typename T, typename T1, typename T2>
CM_API void
cm_slm_atomic(uint slmBuffer, CmAtomicOpType aop, T v_Addr,
              T1 v_Dst, T2 v_Src0)
{
    T1 src1 = 0;
    CmSLMAtomicOpType op = Get_Gen_Atomic_Opcode(aop);

    switch (op) {
        case SLM_ATOMIC_CMPWR:
            fprintf(stderr, "Two sources needed for ATOMIC_CMPWR! \n");
            exit(EXIT_FAILURE);
        case SLM_ATOMIC_INC:
        case SLM_ATOMIC_DEC:
        case SLM_ATOMIC_PREDEC:
            fprintf(stderr, "No sources allowed for the Atomic Operation! \n");
            exit(EXIT_FAILURE);
        default:
        break;
    }
    cm_slm_atomic_generic(slmBuffer, op, v_Addr, v_Src0, src1, v_Dst);
}

template <typename T, typename T1>
CM_API void
cm_slm_atomic(uint slmBuffer, CmAtomicOpType aop, T v_Addr,
              T1 v_Dst)
{
    T1 src0 = 0;
    T1 src1 = 0;
    CmSLMAtomicOpType op = Get_Gen_Atomic_Opcode(aop);

    switch (op) {
        case SLM_ATOMIC_INC:
        case SLM_ATOMIC_DEC:
        case SLM_ATOMIC_PREDEC:
            break;
        default:
            fprintf(stderr, "Need source(s) for the Atomic Operation! \n");
            exit(EXIT_FAILURE);
    }
    cm_slm_atomic_generic(slmBuffer, op, v_Addr, src0, src1, v_Dst);
}

template <typename T, typename T1, typename T2>
CM_API void
cm_slm_atomic(uint slmBuffer, CmAtomicOpType aop, T v_Addr,
              int dst, T1 v_Src0, T2 v_Src1)
{
    T2 v_Dst = 0; // We don't actually care about the contents of this
    CmSLMAtomicOpType op = Get_Gen_Atomic_Opcode(aop);

    if (dst != 0) {
        fprintf(stderr, "cm_slm_atomic passed destination vec as int but not NULL %x\n", dst);
        exit(EXIT_FAILURE);
    }

    if (op != SLM_ATOMIC_CMPWR) {
        fprintf(stderr, "Two sources not allowed for the Atomic Operation! \n");
        exit(EXIT_FAILURE);
    }
    cm_slm_atomic_generic(slmBuffer, op, v_Addr, v_Src0, v_Src1, v_Dst);
}

template <typename T, typename T1>
CM_API void
cm_slm_atomic(uint slmBuffer, CmAtomicOpType aop, T v_Addr,
              int dst, T1 v_Src0)
{
    T1 src1 = 0;
    T1 v_Dst = 0; // We don't actually care about the contents of this
    CmSLMAtomicOpType op = Get_Gen_Atomic_Opcode(aop);

    if (dst != 0) {
        fprintf(stderr, "cm_slm_atomic passed destination vec as int but not NULL %x\n", dst);
        exit(EXIT_FAILURE);
    }

    switch (op) {
        case SLM_ATOMIC_CMPWR:
            fprintf(stderr, "Two sources needed for ATOMIC_CMPWR! \n");
            exit(EXIT_FAILURE);
        case SLM_ATOMIC_INC:
        case SLM_ATOMIC_DEC:
        case SLM_ATOMIC_PREDEC:
            fprintf(stderr, "No sources allowed for the Atomic Operation! \n");
            exit(EXIT_FAILURE);
        default:
        break;
    }
    cm_slm_atomic_generic(slmBuffer, op, v_Addr, v_Src0, src1, v_Dst);
}

template <typename T, uint N>
CM_API void
cm_slm_atomic(uint slmBuffer, CmAtomicOpType aop, vector<T,N> &v_Addr,
              int dst)
{
    // We can use the same type as the v_Addr as this will define the size of the vector correctly
    // and we don't actually care about the type otherwise (in this case it will be ushort)
    vector<uint,N> src0 = 0;
    vector<uint,N> src1 = 0;
    vector<uint,N> v_Dst = 0; // We don't actually care about the contents of this
    CmSLMAtomicOpType op = Get_Gen_Atomic_Opcode(aop);

    if (dst != 0) {
        fprintf(stderr, "cm_slm_atomic passed destination vec as int but not NULL %x\n", dst);
        exit(EXIT_FAILURE);
    }

    switch (op) {
        case SLM_ATOMIC_INC:
        case SLM_ATOMIC_DEC:
        case SLM_ATOMIC_PREDEC:
            break;
        default:
            fprintf(stderr, "Need source(s) for the Atomic Operation! \n");
            exit(EXIT_FAILURE);
    }
    cm_slm_atomic_generic(slmBuffer, op, v_Addr, src0, src1, v_Dst);
}

template <typename T, uint N>
CM_API void
cm_slm_atomic(uint slmBuffer, CmAtomicOpType aop, vector_ref<T,N> v_Addr,
              int dst)
{
    // We can use the same type as the v_Addr as this will define the size of the vector correctly
    // and we don't actually care about the type otherwise (in this case it will be ushort)
    vector<uint,N> src0 = 0;
    vector<uint,N> src1 = 0;
    vector<uint,N> v_Dst = 0; // We don't actually care about the contents of this
    CmSLMAtomicOpType op = Get_Gen_Atomic_Opcode(aop);

    if (dst != 0) {
        fprintf(stderr, "cm_slm_atomic passed destination vec as int but not NULL %x\n", dst);
        exit(EXIT_FAILURE);
    }

    switch (op) {
        case SLM_ATOMIC_INC:
        case SLM_ATOMIC_DEC:
        case SLM_ATOMIC_PREDEC:
            break;
        default:
            fprintf(stderr, "Need source(s) for the Atomic Operation! \n");
            exit(EXIT_FAILURE);
    }
    cm_slm_atomic_generic(slmBuffer, op, v_Addr, src0, src1, v_Dst);
}

template <typename RT, typename T1, typename T2, typename T3, uint SZ>
CM_API vector<RT, SZ>
cm_sada2(const stream<T1,SZ>& src0, const stream<T2,SZ>& src1, const stream<T3,SZ>& src2,
        const uint flags = 0)
{
    static const bool conformable1 = bytetype<T1>::value;
    static const bool conformable2 = bytetype<T2>::value;
    static const bool conformable3 = wordtype<T3>::value;
    static const bool conformable4 = wordtype<RT>::value;
    static const bool conformable5 = check_true<!(SZ&1)>::value;

    int i;
    typename restype_ex<T1,T2>::type ret;
    vector<RT, SZ> retv;

    for (i = 0; i < SZ; i+=1) {
        retv(i) = 0;
    }

    uint sat1 = CmEmulSys::_SetSatur<T1, is_inttype<RT>::value>::SetSatur();
    for (i = 0; i < SZ; i+=2) {
        SIMDCF_ELEMENT_SKIP(i);
        ret = CmEmulSys::abs(src0.get(i) - src1.get(i))
                + CmEmulSys::abs(src0.get(i+1) - src1.get(i+1))
                + src2.get(i);

        retv(i) = CmEmulSys::satur<RT>::saturate(ret, flags | sat1);
   }

   return retv;
}

template <typename RT, typename T1, typename T2, typename T3, uint SZ>
CM_API vector<RT, SZ>
cm_sada2(const stream<T1,SZ>& src0, const T2& src1, const stream<T3,SZ>& src2,
       const typename uint_type<T1, T2>::type flags = 0)
{
    vector<T1, SZ> v1;
    vector<T2, SZ> v2(src1);
    vector<T3, SZ> v3;
    vector<RT, SZ> retv;
    int i;

    for (i = 0; i < SZ; i++) {
        v1(i) = src0.get(i);
        v3(i) = src2.get(i);
    }

    retv = cm_sada2<RT>(v1, v2, v3, flags);

    return retv;
}

template <uint SZ>
CM_API vector<float, SZ>
cm_pln(const stream<float, 4>& src0,
       const stream<float,SZ>& src1,
       const stream<float,SZ>& src2,
       const uint flags = 0)
{
    int i;
    static const bool conformable2 = check_true<!(SZ%8)>::value;
    vector<float, SZ> retv;
    float ret;

    for (i = 0; i < SZ; i++) {
         ret = src0.get(0) * src1.get(i) + src0.get(1) * src2.get(i) + src0.get(3);
         SIMDCF_WRAPPER(retv(i) = CmEmulSys::satur<float>::saturate(ret, flags), SZ, i);
     }

    return retv;
}

template <uint SZ>
CM_API vector<float, SZ>
cm_lrp(const stream<float,SZ>& src0,
       const stream<float,SZ>& src1,
       const stream<float,SZ>& src2,
       const uint flags = 0)
{
    int i;
    vector<float, SZ> retv;
    float ret;

    for (i = 0; i < SZ; i++) {
         ret = src1.get(i) * src0.get(i) + src2.get(i) * (1.0 - src0.get(i));
         SIMDCF_WRAPPER(retv(i) = ret, SZ, i);
     }

    return retv;
}

//------------------------------------------------------------------------------
// Temporary SVM functions that take svmptr_t address

// The /DCM_PTRSIZE=32 and /DCM_PTRSIZE=64 options allow the user to specify
// the size of svmptr_t in a CM program.
#ifdef CM_PTRSIZE
#if CM_PTRSIZE==32
typedef uint32_t svmptr_t;
#elif CM_PTRSIZE==64
typedef uint64_t svmptr_t;
#else
#error CM_PTRSIZE must be 32 or 64
#endif
#else
typedef uintptr_t svmptr_t;
#endif // def CM_PTRSIZE

// Read from SVM at the addresses starting at 'addr'
// and write it back to the destination vector 'v_Dst'
template <typename T, uint N>
CM_API void
cm_svm_block_read(uint64_t addr, // SVM pointer
                  vector_ref<T, N> v_Dst)   // Data vector to be written from SVM
{
    if (addr & 15) {
        fprintf(stderr, "cm_svm_block_read: address unaligned\n");
        exit(EXIT_FAILURE);
    }
    for (int i = 0; i != N; i++) {
        SIMDCF_ELEMENT_SKIP(i);
        v_Dst(i) = ((T *)addr)[i];
    }
}

template <typename T, uint N>
CM_API void
cm_svm_block_read(uint64_t addr, // SVM pointer
                  vector<T, N> &v_Dst)   // Data vector to be written from SVM
{
    if (addr & 15) {
        fprintf(stderr, "cm_svm_block_read: address unaligned\n");
        exit(EXIT_FAILURE);
    }
    for (int i = 0; i != N; i++) {
        SIMDCF_ELEMENT_SKIP(i);
        v_Dst(i) = ((T *)addr)[i];
    }
}

template <typename T, uint R, uint C>
CM_API void
cm_svm_block_read(uint64_t addr, // SVM pointer
                  matrix_ref<T, R, C> v_Dst)   // Data vector to be written from SVM
{
    if (addr & 15) {
        fprintf(stderr, "cm_svm_block_read: address unaligned\n");
        exit(EXIT_FAILURE);
    }
    for (int i = 0; i != R; i++)
        for (int j = 0; j != C; j++) {
            SIMDCF_ELEMENT_SKIP(i * C + j);
            v_Dst(i, j) = ((T *)addr)[i * C + j];
        }
}

template <typename T, uint R, uint C>
CM_API void
cm_svm_block_read(uint64_t addr, // SVM pointer
                  matrix<T, R, C> &v_Dst)   // Data vector to be written from SVM
{
    if (addr & 15) {
        fprintf(stderr, "cm_svm_block_read: address unaligned\n");
        exit(EXIT_FAILURE);
    }
    for (int i = 0; i != R; i++)
        for (int j = 0; j != C; j++) {
            SIMDCF_ELEMENT_SKIP(i * C + j);
            v_Dst(i, j) = ((T *)addr)[i * C + j];
        }
}

// Read from SVM at the addresses starting at 'addr'
// and write it back to the destination vector 'v_Dst'.
// The address does not need to be oword aligned.
template <typename T, uint N>
CM_API void
cm_svm_block_read_unaligned(uint64_t addr, // SVM pointer
                  vector_ref<T, N> v_Dst)   // Data vector to be written from SVM
{
    if (addr & 3) {
        fprintf(stderr, "cm_svm_block_read_unaligned: address unaligned\n");
        exit(EXIT_FAILURE);
    }
    for (int i = 0; i != N; i++) {
        SIMDCF_ELEMENT_SKIP(i);
        v_Dst(i) = ((T *)addr)[i];
    }
}

template <typename T, uint N>
CM_API void
cm_svm_block_read_unaligned(uint64_t addr, // SVM pointer
                  vector<T, N> &v_Dst)   // Data vector to be written from SVM
{
    if (addr & 3) {
        fprintf(stderr, "cm_svm_block_read_unaligned: address unaligned\n");
        exit(EXIT_FAILURE);
    }
    for (int i = 0; i != N; i++) {
        SIMDCF_ELEMENT_SKIP(i);
        v_Dst(i) = ((T *)addr)[i];
    }
}

template <typename T, uint R, uint C>
CM_API void
cm_svm_block_read_unaligned(uint64_t addr, // SVM pointer
                  matrix_ref<T, R, C> v_Dst)   // Data vector to be written from SVM
{
    if (addr & 3) {
        fprintf(stderr, "cm_svm_block_read_unaligned: address unaligned\n");
        exit(EXIT_FAILURE);
    }
    for (int i = 0; i != R; i++)
        for (int j = 0; j != C; j++) {
            SIMDCF_ELEMENT_SKIP(i * C + j);
            v_Dst(i, j) = ((T *)addr)[i * C + j];
        }
}

template <typename T, uint R, uint C>
CM_API void
cm_svm_block_read_unaligned(uint64_t addr, // SVM pointer
                  matrix<T, R, C> &v_Dst)   // Data vector to be written from SVM
{
    if (addr & 3) {
        fprintf(stderr, "cm_svm_block_read_unaligned: address unaligned\n");
        exit(EXIT_FAILURE);
    }
    for (int i = 0; i != R; i++)
        for (int j = 0; j != C; j++) {
            SIMDCF_ELEMENT_SKIP(i * C + j);
            v_Dst(i, j) = ((T *)addr)[i * C + j];
        }
}

// pointer interface.
template <typename T, uint N>
CM_API void
cm_ptr_block_read(const T *const addr, // pointer
                  vector_ref<T, N> v_Dst)   // Data vector to be written from SVM
{
    uint64_t _addr = reinterpret_cast<uint64_t>(addr);
    cm_svm_block_read(_addr, v_Dst);
}

template <typename T, uint N>
CM_API void
cm_ptr_block_read(const T *const addr, // pointer
                  vector<T, N> &v_Dst)   // Data vector to be written from SVM
{
    uint64_t _addr = reinterpret_cast<uint64_t>(addr);
    cm_svm_block_read(_addr, v_Dst);
}

template <typename T, uint R, uint C>
CM_API void
cm_ptr_block_read(const T *const addr, // pointer
                  matrix_ref<T, R, C> v_Dst)   // Data vector to be written from SVM
{
    uint64_t _addr = reinterpret_cast<uint64_t>(addr);
    cm_svm_block_read(_addr, v_Dst);
}

template <typename T, uint R, uint C>
CM_API void
cm_ptr_block_read(const T *const addr, // pointer
                  matrix<T, R, C> &v_Dst)   // Data vector to be written from SVM
{
    uint64_t _addr = reinterpret_cast<uint64_t>(addr);
    cm_svm_block_read(_addr, v_Dst);
}

template <typename T, uint N>
CM_API void
cm_ptr_block_read_unaligned(const T *const addr, // pointer
                  vector_ref<T, N> v_Dst)   // Data vector to be written from SVM
{
    uint64_t _addr = reinterpret_cast<uint64_t>(addr);
    cm_svm_block_read_unaligned(_addr, v_Dst);
}

template <typename T, uint N>
CM_API void
cm_ptr_block_read_unaligned(const T *const addr, // pointer
                  vector<T, N> &v_Dst)   // Data vector to be written from SVM
{
    uint64_t _addr = reinterpret_cast<uint64_t>(addr);
    cm_svm_block_read_unaligned(_addr, v_Dst);
}

template <typename T, uint R, uint C>
CM_API void
cm_ptr_block_read_unaligned(const T *const addr, // pointer
                  matrix_ref<T, R, C> v_Dst)   // Data vector to be written from SVM
{
    uint64_t _addr = reinterpret_cast<uint64_t>(addr);
    cm_svm_block_read_unaligned(_addr, v_Dst);
}

template <typename T, uint R, uint C>
CM_API void
cm_ptr_block_read_unaligned(const T *const addr, // pointer
                  matrix<T, R, C> &v_Dst)   // Data vector to be written from SVM
{
    uint64_t _addr = reinterpret_cast<uint64_t>(addr);
    cm_svm_block_read_unaligned(_addr, v_Dst);
}

// Read from SVM at the addresses given in 'v_Addr'
// and write it back to the destination vector 'v_Dst'
template <typename T, uint N>
CM_API void
cm_svm_scatter_read64(vector_ref<uint64_t, N> v_Addr, // vector of SVM pointers
                    vector_ref<T, N> v_Dst   // Data vector to be written from SVM
            )
{
    for (int i = 0; i != N; i++) {
        SIMDCF_ELEMENT_SKIP(i);
        v_Dst(i) = *(T *)v_Addr(i);
    }
}

template <typename T, uint N>
CM_API void
cm_svm_scatter_read64(vector<uint64_t, N> &v_Addr, // vector of SVM pointers
                    vector_ref<T, N> v_Dst   // Data vector to be written from SVM
            )
{
    for (int i = 0; i != N; i++) {
        SIMDCF_ELEMENT_SKIP(i);
        v_Dst(i) = *(T *)v_Addr(i);
    }
}

template <typename T, uint N>
CM_API void
cm_svm_scatter_read64(vector_ref<uint64_t, N> v_Addr, // vector of SVM pointers
                    vector<T, N> &v_Dst   // Data vector to be written from SVM
            )
{
    for (int i = 0; i != N; i++) {
        SIMDCF_ELEMENT_SKIP(i);
        v_Dst(i) = *(T *)v_Addr(i);
    }
}

template <typename T, uint N>
CM_API void
cm_svm_scatter_read64(vector<uint64_t, N> &v_Addr, // vector of SVM pointers
                    vector<T, N> &v_Dst   // Data vector to be written from SVM
            )
{
    for (int i = 0; i != N; i++) {
        SIMDCF_ELEMENT_SKIP(i);
        v_Dst(i) = *(T *)v_Addr(i);
    }
}

template <typename T, uint R, uint C>
CM_API void
cm_svm_scatter_read64(matrix_ref<uint64_t, R, C> v_Addr, // vector of SVM pointers
                    matrix_ref<T, R, C> v_Dst   // Data vector to be written from SVM
            )
{
    for (int i = 0; i != R; i++)
        for (int j = 0; j != C; j++) {
            SIMDCF_ELEMENT_SKIP(i * C + j);
            v_Dst(i, j) = *(T *)v_Addr(i, j);
        }
}

template <typename T, uint R, uint C>
CM_API void
cm_svm_scatter_read64(matrix<uint64_t, R, C> &v_Addr, // vector of SVM pointers
                    matrix_ref<T, R, C> v_Dst   // Data vector to be written from SVM
            )
{
    for (int i = 0; i != R; i++)
        for (int j = 0; j != C; j++) {
            SIMDCF_ELEMENT_SKIP(i * C + j);
            v_Dst(i, j) = *(T *)v_Addr(i, j);
        }
}

template <typename T, uint R, uint C>
CM_API void
cm_svm_scatter_read64(matrix_ref<uint64_t, R, C> v_Addr, // vector of SVM pointers
                    matrix<T, R, C> &v_Dst   // Data vector to be written from SVM
            )
{
    for (int i = 0; i != R; i++)
        for (int j = 0; j != C; j++) {
            SIMDCF_ELEMENT_SKIP(i * C + j);
            v_Dst(i, j) = *(T *)v_Addr(i, j);
        }
}

template <typename T, uint R, uint C>
CM_API void
cm_svm_scatter_read64(matrix<uint64_t, R, C> &v_Addr, // vector of SVM pointers
                    matrix<T, R, C> &v_Dst   // Data vector to be written from SVM
            )
{
    for (int i = 0; i != R; i++)
        for (int j = 0; j != C; j++) {
            SIMDCF_ELEMENT_SKIP(i * C + j);
            v_Dst(i, j) = *(T *)v_Addr(i, j);
        }
}

// Write to SVM at the addresses starting at 'addr'
// from the source vector 'v_Src'
template <typename T, uint N>
CM_API void
cm_svm_block_write(uint64_t addr, // SVM pointer
                  vector<T, N> &v_Src   // Data vector to write into SVM
            )
{
    if (addr & 15) {
        fprintf(stderr, "cm_svm_block_write: address unaligned\n");
        exit(EXIT_FAILURE);
    }
    for (int i = 0; i != N; i++) {
        SIMDCF_ELEMENT_SKIP(i);
        ((T *)addr)[i] = v_Src(i);
    }
}

template <typename T, uint N>
CM_API void
cm_svm_block_write(uint64_t addr, // SVM pointer
                  vector_ref<T, N> v_Src   // Data vector to write into SVM
            )
{
    if (addr & 15) {
        fprintf(stderr, "cm_svm_block_write: address unaligned\n");
        exit(EXIT_FAILURE);
    }
    for (int i = 0; i != N; i++) {
        SIMDCF_ELEMENT_SKIP(i);
        ((T *)addr)[i] = v_Src(i);
    }
}

template <typename T, uint R, uint C>
CM_API void
cm_svm_block_write(uint64_t addr, // SVM pointer
                  matrix<T, R, C> &v_Src   // Data vector to write into SVM
            )
{
    if (addr & 15) {
        fprintf(stderr, "cm_svm_block_write: address unaligned\n");
        exit(EXIT_FAILURE);
    }
    for (int i = 0; i != R; i++)
        for (int j = 0; j != C; j++) {
            SIMDCF_ELEMENT_SKIP(i * C + j);
            ((T *)addr)[i * C + j] = v_Src(i, j);
        }
}

template <typename T, uint R, uint C>
CM_API void
cm_svm_block_write(uint64_t addr, // SVM pointer
                  matrix_ref<T, R, C> v_Src   // Data vector to write into SVM
            )
{
    if (addr & 15) {
        fprintf(stderr, "cm_svm_block_write: address unaligned\n");
        exit(EXIT_FAILURE);
    }
    for (int i = 0; i != R; i++)
        for (int j = 0; j != C; j++) {
            SIMDCF_ELEMENT_SKIP(i * C + j);
            ((T *)addr)[i * C + j] = v_Src(i, j);
        }
}

// pointer interface
template <typename T, uint N>
CM_API void
cm_ptr_block_write(T *const addr, // pointer
                  vector<T, N> &v_Src   // Data vector to write into SVM
            )
{
    uint64_t _addr = reinterpret_cast<uint64_t>(addr);
    cm_svm_block_write(_addr, v_Src);
}

template <typename T, uint N>
CM_API void
cm_ptr_block_write(T *const addr, // pointer
                  vector_ref<T, N> v_Src   // Data vector to write into SVM
            )
{
    uint64_t _addr = reinterpret_cast<uint64_t>(addr);
    cm_svm_block_write(_addr, v_Src);
}

template <typename T, uint R, uint C>
CM_API void
cm_ptr_block_write(T *const addr, // pointer
                  matrix<T, R, C> &v_Src   // Data vector to write into SVM
            )
{
    uint64_t _addr = reinterpret_cast<uint64_t>(addr);
    cm_svm_block_write(_addr, v_Src);
}

template <typename T, uint R, uint C>
CM_API void
cm_ptr_block_write(T *const addr, // pointer
                  matrix_ref<T, R, C> v_Src   // Data vector to write into SVM
            )
{
    uint64_t _addr = reinterpret_cast<uint64_t>(addr);
    cm_svm_block_write(_addr, v_Src);
}

// Write to SVM at the addresses given in 'v_Addr'
// from the source vector 'v_Src'
template <typename T, uint N>
CM_API void
cm_svm_scatter_write64(vector_ref<uint64_t, N> v_Addr, // vector of SVM pointers
                     vector<T, N> &v_Src   // Data vector to write64 into SVM
            )
{
    for (int i = 0; i != N; i++) {
        SIMDCF_ELEMENT_SKIP(i);
        *(T *)v_Addr(i) = v_Src(i);
    }
}

template <typename T, uint N>
CM_API void
cm_svm_scatter_write64(vector<uint64_t, N> &v_Addr, // vector of SVM pointers
                     vector<T, N> &v_Src   // Data vector to write64 into SVM
            )
{
    for (int i = 0; i != N; i++) {
        SIMDCF_ELEMENT_SKIP(i);
        *(T *)v_Addr(i) = v_Src(i);
    }
}

template <typename T, uint N>
CM_API void
cm_svm_scatter_write64(vector_ref<uint64_t, N> v_Addr, // vector of SVM pointers
                     vector_ref<T, N> v_Src   // Data vector to write64 into SVM
            )
{
    for (int i = 0; i != N; i++) {
        SIMDCF_ELEMENT_SKIP(i);
        *(T *)v_Addr(i) = v_Src(i);
    }
}

template <typename T, uint N>
CM_API void
cm_svm_scatter_write64(vector<uint64_t, N> &v_Addr, // vector of SVM pointers
                     vector_ref<T, N> v_Src   // Data vector to write64 into SVM
            )
{
    for (int i = 0; i != N; i++) {
        SIMDCF_ELEMENT_SKIP(i);
        *(T *)v_Addr(i) = v_Src(i);
    }
}

template <typename T, uint R, uint C>
CM_API void
cm_svm_scatter_write64(matrix_ref<uint64_t, R, C> v_Addr, // vector of SVM pointers
                     matrix<T, R, C> &v_Src   // Data vector to write64 into SVM
            )
{
    for (int i = 0; i != R; i++)
        for (int j = 0; j != C; j++) {
            SIMDCF_ELEMENT_SKIP(i * C + j);
            *(T *)v_Addr(i, j) = v_Src(i, j);
        }
}

template <typename T, uint R, uint C>
CM_API void
cm_svm_scatter_write64(matrix<uint64_t, R, C> &v_Addr, // vector of SVM pointers
                     matrix<T, R, C> &v_Src   // Data vector to write64 into SVM
            )
{
    for (int i = 0; i != R; i++)
        for (int j = 0; j != C; j++) {
            SIMDCF_ELEMENT_SKIP(i * C + j);
            *(T *)v_Addr(i, j) = v_Src(i, j);
        }
}

template <typename T, uint R, uint C>
CM_API void
cm_svm_scatter_write64(matrix_ref<uint64_t, R, C> v_Addr, // vector of SVM pointers
                     matrix_ref<T, R, C> v_Src   // Data vector to write64 into SVM
            )
{
    for (int i = 0; i != R; i++)
        for (int j = 0; j != C; j++) {
            SIMDCF_ELEMENT_SKIP(i * C + j);
            *(T *)v_Addr(i, j) = v_Src(i, j);
        }
}

template <typename T, uint R, uint C>
CM_API void
cm_svm_scatter_write64(matrix<uint64_t, R, C> &v_Addr, // vector of SVM pointers
                     matrix_ref<T, R, C> v_Src   // Data vector to write64 into SVM
            )
{
    for (int i = 0; i != R; i++)
        for (int j = 0; j != C; j++) {
            SIMDCF_ELEMENT_SKIP(i * C + j);
            *(T *)v_Addr(i, j) = v_Src(i, j);
        }
}

// Wrappers for svm scatter/gather functions that take address vector in
// actual pointer size (32 or 64 bit) for the platform.
template <typename T, uint N> _GENX_ inline void cm_svm_scatter_read(vector<svmptr_t, N> v_Addr, vector_ref<T, N> v_Dst)
{   vector<uint64_t, N> v_Addr64(v_Addr);
    cm_svm_scatter_read64(v_Addr64, v_Dst);
}

template <typename T, uint N> _GENX_ inline void cm_svm_scatter_read(vector<svmptr_t, N> v_Addr, vector<T, N> &v_Dst)
{   vector<uint64_t, N> v_Addr64(v_Addr);
    cm_svm_scatter_read64(v_Addr64, v_Dst);
}

template <typename T, uint R, uint C> _GENX_ inline void cm_svm_scatter_read(matrix<svmptr_t, R, C> v_Addr, matrix_ref<T, R, C> v_Dst)
{   matrix<uint64_t, R, C> v_Addr64(v_Addr);
    cm_svm_scatter_read64(v_Addr64, v_Dst);
}

template <typename T, uint R, uint C> _GENX_ inline void cm_svm_scatter_read(matrix<svmptr_t, R, C> v_Addr, matrix<T, R, C> &v_Dst)
{   matrix<uint64_t, R, C> v_Addr64(v_Addr);
    cm_svm_scatter_read64(v_Addr64, v_Dst);
}

template <typename T, uint N> _GENX_ inline void cm_svm_scatter_write(vector<svmptr_t, N> v_Addr, vector_ref<T, N> v_Dst)
{   vector<uint64_t, N> v_Addr64(v_Addr);
    cm_svm_scatter_write64(v_Addr64, v_Dst);
}

template <typename T, uint N> _GENX_ inline void cm_svm_scatter_write(vector<svmptr_t, N> v_Addr, vector<T, N> &v_Dst)
{   vector<uint64_t, N> v_Addr64(v_Addr);
    cm_svm_scatter_write64(v_Addr64, v_Dst);
}

template <typename T, uint R, uint C> _GENX_ inline void cm_svm_scatter_write(matrix<svmptr_t, R, C> v_Addr, matrix_ref<T, R, C> v_Dst)
{   matrix<uint64_t, R, C> v_Addr64(v_Addr);
    cm_svm_scatter_write64(v_Addr64, v_Dst);
}

template <typename T, uint R, uint C> _GENX_ inline void cm_svm_scatter_write(matrix<svmptr_t, R, C> v_Addr, matrix<T, R, C> &v_Dst)
{   matrix<uint64_t, R, C> v_Addr64(v_Addr);
    cm_svm_scatter_write64(v_Addr64, v_Dst);
}

//------------------------------------------------------------------------------
// Implementation of SVM atomic operations

// This funtion performs atomic scattered DWord write to SVM
template <typename T, uint N>
inline void
cm_svm_atomic_internal(CmAtomicOpType op, vector<uint64_t,N> &v_Addr,
              vector<T,N> &v_Src0, vector<T,N> &v_Src1, vector<T,N> &v_Dst)
{
    static const bool conformable1 = Allowed_Type_Float_Or_Dword<T>::value;
    static const bool conformable2 = Allowed_Vector_Length_8_Or_16<N>::value;

    int    i;
    uint   *uintPtr;
    int    *intPtr;

    // InitializeCriticalSection(&dataport_cs);

    for (i = 0; i < N; i++) {

        SIMDCF_ELEMENT_SKIP(i);

        uintPtr    = (uint *)v_Addr(i);
        intPtr     = (int *)uintPtr;
        v_Dst(i)   = *uintPtr;

        // To Do: How to handle out-of-bound accesses to SVM for atomic writes?

        switch (op) {
            case ATOMIC_AND:
                *uintPtr = *uintPtr & v_Src0(i);
                break;
            case ATOMIC_OR:
                *uintPtr = *uintPtr | v_Src0(i);
                break;
            case ATOMIC_XOR:
                *uintPtr = *uintPtr ^ v_Src0(i);
                break;
            case ATOMIC_INC:
                *uintPtr = *uintPtr + 1;
                break;
            case ATOMIC_DEC:
                *uintPtr = *uintPtr - 1;
                break;
            case ATOMIC_ADD:
                *uintPtr = *uintPtr + (uint) v_Src0(i);
                break;
            case ATOMIC_SUB:
                *uintPtr = *uintPtr - (uint) v_Src0(i);
                break;
            case ATOMIC_MAXSINT:
                *intPtr = (*intPtr > (int) v_Src0(i)) ? *intPtr : (int) v_Src0(i);
                break;
            case ATOMIC_MINSINT:
                *intPtr = (*intPtr > (int) v_Src0(i)) ? (int) v_Src0(i) : *intPtr;
                break;
            case ATOMIC_MAX:
                *uintPtr = (*uintPtr > (uint) v_Src0(i)) ? *uintPtr : (uint) v_Src0(i);
                break;
            case ATOMIC_MIN:
                *uintPtr = (*uintPtr < (uint) v_Src0(i)) ? *uintPtr : (uint) v_Src0(i);
                break;
            case ATOMIC_CMPXCHG:
                *uintPtr = (*uintPtr == (uint) v_Src0(i)) ? (uint) v_Src1(i) : *uintPtr;
                break;
            default:
                printf("Error writing SVM: invalid opcode for SVM atomic write!\n");
                exit(EXIT_FAILURE);
        }
    }

    // DeleteCriticalSection(&dataport_cs);
}
//------------------------------------------------------------------------------
template <typename T, uint N>
CM_API void
cm_svm_atomic_generic(CmAtomicOpType op, vector_ref<uint64_t,N> v_Addr,
              vector_ref<T,N> v_Src0, vector_ref<T,N> v_Src1, vector_ref<T,N> v_Dst)
{
    vector<T,N> dst = v_Dst;
    vector<T,N> src0 = v_Src0;
    vector<T,N> src1 = v_Src1;
    vector<T,N> addr = v_Addr;
    cm_svm_atomic_internal(op, addr, src0, src1, dst);
}

template <typename T, uint N>
CM_API void
cm_svm_atomic_generic(CmAtomicOpType op, vector_ref<uint64_t,N> v_Addr,
              vector_ref<T,N> v_Src0, vector_ref<T,N> v_Src1, vector<T,N> &v_Dst)
{
    vector<T,N> src0 = v_Src0;
    vector<T,N> src1 = v_Src1;
    vector<T,N> addr = v_Addr;
    cm_svm_atomic_internal(op, addr, src0, src1, v_Dst);
}

template <typename T, uint N>
CM_API void
cm_svm_atomic_generic(CmAtomicOpType op, vector_ref<uint64_t,N> v_Addr,
              vector_ref<T,N> v_Src0, vector<T,N> &v_Src1, vector_ref<T,N> v_Dst)
{
    vector<T,N> dst = v_Dst;
    vector<T,N> src0 = v_Src0;
    vector<T,N> addr = v_Addr;
    cm_svm_atomic_internal(op, addr, src0, v_Src1, dst);
}

template <typename T, uint N>
CM_API void
cm_svm_atomic_generic(CmAtomicOpType op, vector_ref<uint64_t,N> v_Addr,
              vector_ref<T,N> v_Src0, vector<T,N> &v_Src1, vector<T,N> &v_Dst)
{
    vector<T,N> src0 = v_Src0;
    vector<T,N> addr = v_Addr;
    cm_svm_atomic_internal(op, addr, src0, v_Src1, v_Dst);
}

template <typename T, uint N>
CM_API void
cm_svm_atomic_generic(CmAtomicOpType op, vector_ref<uint64_t,N> v_Addr,
              vector<T,N> &v_Src0, vector_ref<T,N> v_Src1, vector_ref<T,N> v_Dst)
{
    vector<T,N> dst = v_Dst;
    vector<T,N> src1 = v_Src1;
    vector<T,N> addr = v_Addr;
    cm_svm_atomic_internal(op, addr, v_Src0, src1, dst);
}

template <typename T, uint N>
CM_API void
cm_svm_atomic_generic(CmAtomicOpType op, vector_ref<uint64_t,N> v_Addr,
              vector<T,N> &v_Src0, vector_ref<T,N> v_Src1, vector<T,N> &v_Dst)
{
    vector<T,N> src1 = v_Src1;
    vector<T,N> addr = v_Addr;
    cm_svm_atomic_internal(op, addr, v_Src0, src1, v_Dst);
}

template <typename T, uint N>
CM_API void
cm_svm_atomic_generic(CmAtomicOpType op, vector_ref<uint64_t,N> v_Addr,
              vector<T,N> &v_Src0, vector<T,N> &v_Src1, vector_ref<T,N> v_Dst)
{
    vector<T,N> dst = v_Dst;
    vector<T,N> addr = v_Addr;
    cm_svm_atomic_internal(op, addr, v_Src0, v_Src1, dst);
}

template <typename T, uint N>
CM_API void
cm_svm_atomic_generic(CmAtomicOpType op, vector_ref<uint64_t,N> v_Addr,
              vector<T,N> &v_Src0, vector<T,N> &v_Src1, vector<T,N> &v_Dst)
{
    vector<T,N> addr = v_Addr;
    cm_svm_atomic_internal(op, addr, v_Src0, v_Src1, v_Dst);
}
//-----------------------------------------------------------------------------

template <typename T, uint N>
CM_API void
cm_svm_atomic_generic(CmAtomicOpType op, vector<uint64_t,N> &v_Addr,
              vector_ref<T,N> v_Src0, vector_ref<T,N> v_Src1, vector_ref<T,N> v_Dst)
{
    vector<T,N> dst = v_Dst;
    vector<T,N> src0 = v_Src0;
    vector<T,N> src1 = v_Src1;
    cm_svm_atomic_internal(op, v_Addr, src0, src1, dst);
}

template <typename T, uint N>
CM_API void
cm_svm_atomic_generic(CmAtomicOpType op, vector<uint64_t,N> &v_Addr,
              vector_ref<T,N> v_Src0, vector_ref<T,N> v_Src1, vector<T,N> &v_Dst)
{
    vector<T,N> src0 = v_Src0;
    vector<T,N> src1 = v_Src1;
    cm_svm_atomic_internal(op, v_Addr, src0, src1, v_Dst);
}

template <typename T, uint N>
CM_API void
cm_svm_atomic_generic(CmAtomicOpType op, vector<uint64_t,N> &v_Addr,
              vector_ref<T,N> v_Src0, vector<T,N> &v_Src1, vector_ref<T,N> v_Dst)
{
    vector<T,N> dst = v_Dst;
    vector<T,N> src0 = v_Src0;
    cm_svm_atomic_internal(op, v_Addr, src0, v_Src1, dst);
}

template <typename T, uint N>
CM_API void
cm_svm_atomic_generic(CmAtomicOpType op, vector<uint64_t,N> &v_Addr,
              vector_ref<T,N> v_Src0, vector<T,N> &v_Src1, vector<T,N> &v_Dst)
{
    vector<T,N> src0 = v_Src0;
    cm_svm_atomic_internal(op, v_Addr, src0, v_Src1, v_Dst);
}

template <typename T, uint N>
CM_API void
cm_svm_atomic_generic(CmAtomicOpType op, vector<uint64_t,N> &v_Addr,
              vector<T,N> &v_Src0, vector_ref<T,N> v_Src1, vector_ref<T,N> v_Dst)
{
    vector<T,N> dst = v_Dst;
    vector<T,N> src1 = v_Src1;
    cm_svm_atomic_internal(op, v_Addr, v_Src0, src1, dst);
}

template <typename T, uint N>
CM_API void
cm_svm_atomic_generic(CmAtomicOpType op, vector<uint64_t,N> &v_Addr,
              vector<T,N> &v_Src0, vector_ref<T,N> v_Src1, vector<T,N> &v_Dst)
{
    vector<T,N> src1 = v_Src1;
    cm_svm_atomic_internal(op, v_Addr, v_Src0, src1, v_Dst);
}

template <typename T, uint N>
CM_API void
cm_svm_atomic_generic(CmAtomicOpType op, vector<uint64_t,N> &v_Addr,
              vector<T,N> &v_Src0, vector<T,N> &v_Src1, vector_ref<T,N> v_Dst)
{
    vector<T,N> dst = v_Dst;
    cm_svm_atomic_internal(op, v_Addr, v_Src0, v_Src1, dst);
}

template <typename T, uint N>
CM_API void
cm_svm_atomic_generic(CmAtomicOpType op, vector<uint64_t,N> &v_Addr,
              vector<T,N> &v_Src0, vector<T,N> &v_Src1, vector<T,N> &v_Dst)
{
    cm_svm_atomic_internal(op, v_Addr, v_Src0, v_Src1, v_Dst);
}

// Top-level atomic functions:
// Either needs two sources, one source, or no sources

template <typename T1, typename T2, typename T3>
CM_API void
cm_svm_atomic64(CmAtomicOpType op, vector<uint64_t, 8> &v_Addr,
              T1 &v_Dst, T2 v_Src0, T3 v_Src1)
{
    if (op != ATOMIC_CMPXCHG) {
        fprintf(stderr, "Two sources not allowed for the Atomic Operation! \n");
        exit(EXIT_FAILURE);
    }
    cm_svm_atomic_generic(op, v_Addr, v_Src0, v_Src1, v_Dst);
}

template <typename T1, typename T2>
CM_API void
cm_svm_atomic64(CmAtomicOpType op, vector<uint64_t, 8> &v_Addr,
              T1 &v_Dst, T2 v_Src0)
{
    T1 src1 = 0;
    switch (op) {
        case ATOMIC_CMPXCHG:
            fprintf(stderr, "Two sources needed for ATOMIC_CMPXCHG! \n");
            exit(EXIT_FAILURE);
        case ATOMIC_INC:
        case ATOMIC_DEC:
            fprintf(stderr, "No sources allowed for the Atomic Operation! \n");
            exit(EXIT_FAILURE);
        default:
        break;
    }
    cm_svm_atomic_generic(op, v_Addr, v_Src0, src1, v_Dst);
}

template <typename T1>
CM_API void
cm_svm_atomic64(CmAtomicOpType op, vector<uint64_t, 8> &v_Addr, T1 &v_Dst)
{
    T1 src0 = 0;
    T1 src1 = 0;
    switch (op) {
        case ATOMIC_INC:
        case ATOMIC_DEC:
            break;
        default:
            fprintf(stderr, "Need source(s) for the Atomic Operation! \n");
            exit(EXIT_FAILURE);
    }
    cm_svm_atomic_generic(op, v_Addr, src0, src1, v_Dst);
}

template <typename T1, typename T2, typename T3>
_GENX_ inline void cm_svm_atomic(CmAtomicOpType op, vector<svmptr_t, 8> v_Addr, T1 &v_Dst, T2 v_Src0, T3 v_Src1)
{
    vector<uint64_t, 8> v_Addr64;
    v_Addr64[0] = v_Addr[0];
    v_Addr64[1] = v_Addr[1];
    v_Addr64[2] = v_Addr[2];
    v_Addr64[3] = v_Addr[3];
    v_Addr64[4] = v_Addr[4];
    v_Addr64[5] = v_Addr[5];
    v_Addr64[6] = v_Addr[6];
    v_Addr64[7] = v_Addr[7];
    cm_svm_atomic64(op, v_Addr64, v_Dst, v_Src0, v_Src1);
}

template <typename T1, typename T2>
_GENX_ inline void cm_svm_atomic(CmAtomicOpType op, vector<svmptr_t, 8> v_Addr, T1 &v_Dst, T2 v_Src0)
{
    vector<uint64_t, 8> v_Addr64;
    v_Addr64[0] = v_Addr[0];
    v_Addr64[1] = v_Addr[1];
    v_Addr64[2] = v_Addr[2];
    v_Addr64[3] = v_Addr[3];
    v_Addr64[4] = v_Addr[4];
    v_Addr64[5] = v_Addr[5];
    v_Addr64[6] = v_Addr[6];
    v_Addr64[7] = v_Addr[7];
    cm_svm_atomic64(op, v_Addr64, v_Dst, v_Src0);
}

template <typename T1>
_GENX_ inline void cm_svm_atomic(CmAtomicOpType op, vector<svmptr_t, 8> v_Addr, T1 &v_Dst)
{
    vector<uint64_t, 8> v_Addr64;
    v_Addr64[0] = v_Addr[0];
    v_Addr64[1] = v_Addr[1];
    v_Addr64[2] = v_Addr[2];
    v_Addr64[3] = v_Addr[3];
    v_Addr64[4] = v_Addr[4];
    v_Addr64[5] = v_Addr[5];
    v_Addr64[6] = v_Addr[6];
    v_Addr64[7] = v_Addr[7];
    cm_svm_atomic64(op, v_Addr64, v_Dst);
}

#endif /* CM_INTRIN_H */

