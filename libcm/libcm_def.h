/*========================== begin_copyright_notice ============================

Copyright (C) 2017 Intel Corporation

SPDX-License-Identifier: MIT

============================= end_copyright_notice ===========================*/

#ifndef CM_DEF_H
#define CM_DEF_H

#include <limits>
#include <limits.h>
#include <stdlib.h>

#include "half_type.h"
#include "cm_common_macros.h"

class SurfaceIndex;

// DPAS-related.
enum class CmPrecisionType {
    CM_Precision_U1 = 0,      // unsigned 1 bit
    CM_Precision_U2 = 1,      // unsigned 2 bits
    CM_Precision_U4 = 2,      // unsigned 4 bits
    CM_Precision_U8 = 3,      // unsigned 8 bits
    CM_Precision_S1 = 4,      // signed 1 bit
    CM_Precision_S2 = 5,      // signed 2 bits
    CM_Precision_S4 = 6,      // signed 4 bits
    CM_Precision_S8 = 7,      // signed 8 bits
#ifdef CM_HAS_BF16
    CM_Precision_BF16 = 8,    // bfloat 16
#endif
    CM_Precision_FP16 = 9,    // half float
    CM_Precision_TF32 = 11,   // tensorfloat 32
};

#define CM_PRECISION_U1 CmPrecisionType::CM_Precision_U1
#define CM_PRECISION_U2 CmPrecisionType::CM_Precision_U2
#define CM_PRECISION_U4 CmPrecisionType::CM_Precision_U4
#define CM_PRECISION_U8 CmPrecisionType::CM_Precision_U8
#define CM_PRECISION_S1 CmPrecisionType::CM_Precision_S1
#define CM_PRECISION_S2 CmPrecisionType::CM_Precision_S2
#define CM_PRECISION_S4 CmPrecisionType::CM_Precision_S4
#define CM_PRECISION_S8 CmPrecisionType::CM_Precision_S8
#ifdef CM_HAS_BF16
#define CM_PRECISION_BF CmPrecisionType::CM_Precision_BF16
#endif
#define CM_PRECISION_HF CmPrecisionType::CM_Precision_FP16
#define CM_PRECISION_TF32 CmPrecisionType::CM_Precision_TF32

#define _GENX_ROUNDING_MODE_(x)
#define _GENX_FLOAT_CONTROL_(x)

typedef unsigned int   uint;
typedef unsigned short ushort;
typedef unsigned char  uchar;

typedef unsigned char U8;
typedef unsigned short U16;
typedef unsigned int U32;

#ifdef __GNUC__
#ifndef __int8
typedef char __int8;
#endif

#ifndef __int16
typedef short __int16;
#endif

#ifndef __int32
typedef int __int32;
#endif

#ifndef __int64
typedef long long __int64;
#endif

#ifndef __uint64
typedef unsigned long long __uint64;
#endif

#ifndef UINT
typedef unsigned int  UINT;
#endif

#ifndef DWORD
typedef unsigned int DWORD;
#endif

#ifndef BYTE
typedef unsigned char BYTE;
#endif

#endif //__GNUC__

#if !defined(__CLANG_CM) && (!defined(_WIN32) || _MSC_VER >= 1600)
#include <stdint.h>
#else
typedef signed   __int8   int8_t;
typedef signed   __int16  int16_t;
typedef signed   __int32  int32_t;
typedef signed   __int64  int64_t;
typedef unsigned __int8   uint8_t;
typedef unsigned __int16  uint16_t;
typedef unsigned __int32  uint32_t;
typedef unsigned __int64  uint64_t;
#endif

#define SAT 1

typedef enum _CmAtomicOpType_
{
    ATOMIC_ADD                     = 0x0,
    ATOMIC_SUB                     = 0x1,
    ATOMIC_INC                     = 0x2,
    ATOMIC_DEC                     = 0x3,
    ATOMIC_MIN                     = 0x4,
    ATOMIC_MAX                     = 0x5,
    ATOMIC_XCHG                    = 0x6,
    ATOMIC_CMPXCHG                 = 0x7,
    ATOMIC_AND                     = 0x8,
    ATOMIC_OR                      = 0x9,
    ATOMIC_XOR                     = 0xa,
    ATOMIC_MINSINT                 = 0xb,
    ATOMIC_MAXSINT                 = 0xc,
    ATOMIC_FADD                    = 0x10,
    ATOMIC_FSUB                    = 0x11,
    ATOMIC_FCMPWR                  = 0x12,
    ATOMIC_FMIN                    = 0x13,
    ATOMIC_FMAX                    = 0x14
} CmAtomicOpType;

typedef enum _ChannelMaskType_
{	CM_R_ENABLE         = 1,
    CM_G_ENABLE         = 2,
    CM_GR_ENABLE        = 3,
    CM_B_ENABLE         = 4,
    CM_BR_ENABLE        = 5,
    CM_BG_ENABLE        = 6,
    CM_BGR_ENABLE       = 7,
    CM_A_ENABLE         = 8,
    CM_AR_ENABLE        = 9,
    CM_AG_ENABLE        = 10,
    CM_AGR_ENABLE       = 11,
    CM_AB_ENABLE        = 12,
    CM_ABR_ENABLE       = 13,
    CM_ABG_ENABLE       = 14,
    CM_ABGR_ENABLE      = 15
} ChannelMaskType;

typedef enum _OutputFormatControl_
{   CM_16_FULL        = 0,
    CM_16_DOWN_SAMPLE = 1,
    CM_8_FULL         = 2,
    CM_8_DOWN_SAMPLE  = 3
} OutputFormatControl;

typedef enum _CmRoundingMode_
{
    // These values are stored pre-shifted to remove the need to shift the values when applying to
    // the control word
    CM_RTE                = 0,       // Round to nearest or even
    CM_RTP                = 1 << 4,  // Round towards +ve inf
    CM_RTN                = 2 << 4,  // Round towards -ve inf
    CM_RTZ                = 3 << 4   // Round towards zero
} CmRoundingMode;

typedef enum _CmFPMode_
{
    CM_IEEE               = 0,
    CM_ALT                = 1
} CmFPMode;

namespace CmEmulSys
{

static inline long long
abs(long long a)
{
    if (a < 0) {
        return -a;
    } else {
        return a;
    }
}

template<typename RT>
struct satur {
template<typename T> static RT
saturate(const T val, const int flags) {
    if ((flags & SAT) == 0) {
        return (RT) val;
    }

#ifdef max
#undef max
#undef min
#endif
    const RT t_max = std::numeric_limits<RT>::max();
    const RT t_min = std::numeric_limits<RT>::min();

    if (val > t_max) {
        return t_max;
    } else if ((val >= 0 ) && (t_min < 0)) {
        // RT is "signed" if t_min < 0
        // when comparing a signed and a unsigned variable, the signed one cast to unsigned first.
        return (RT) val;
    } else if (val < t_min) {
        return t_min;
    } else {
        return (RT) val;
    }
}
};

template<>
struct satur<half> {
    template<typename T> static half
        saturate(const T val, const int flags) {
        if ((flags & SAT) == 0) {
            return (half)val;
        }

        if (val < 0.) {
            return (half) 0.;
        }
        else if (val > 1.) {
            return (half) 1.;
        }
        else {
            return (half)val;
        }
    }
};

template<>
struct satur<float> {
template<typename T> static float
saturate(const T val, const int flags) {
    if ((flags & SAT) == 0) {
        return (float) val;
    }

    if (val < 0.) {
        return 0;
    } else if (val > 1.) {
        return 1.;
    } else {
        return (float) val;
    }
}
};

template<>
struct satur<double> {
template<typename T> static double
saturate(const T val, const int flags) {
    if ((flags & SAT) == 0) {
        return (double) val;
    }

    if (val < 0.) {
        return 0;
    } else if (val > 1.) {
        return 1.;
    } else {
        return (double) val;
    }
}
};

template<typename T1, bool B> struct _SetSatur {
    static uint SetSatur() {
        return 0;
    }
};

template <> struct _SetSatur<half, true> {
    static uint SetSatur() {
        return SAT;
    }
};

template <> struct _SetSatur<float, true> {
    static uint SetSatur() {
        return SAT;
    }
};

template <> struct _SetSatur<double, true> {
    static uint SetSatur() {
        return SAT;
    }
};

} /* ::CmEmulSys */

template <typename T1, typename T2> struct restype {
private:
    restype();
};

//#if defined(CM_NOCONV) || defined(CM_NONSTRICT)
#if defined(CM_NOCONV)

template <> struct restype<char, char>            { typedef short  type; };
template <> struct restype<char, unsigned char>   { typedef short  type; };
template <> struct restype<char, short>           { typedef short type; };
template <> struct restype<char, unsigned short>  { typedef short type; };
template <> struct restype<char, int>             { typedef int   type; };
template <> struct restype<char, unsigned int>    { typedef int   type; };
template <> struct restype<char, half>            { typedef half  type; };
template <> struct restype<char, float>           { typedef float type; };
template <> struct restype<char, double>          { typedef double type; };
template <> struct restype<char, long long>       { typedef long long type; };
template <> struct restype<char, unsigned long long>           { typedef long long type; };

template <> struct restype<unsigned char, char>            { typedef short  type; };
template <> struct restype<unsigned char, unsigned char>   { typedef short  type; };
template <> struct restype<unsigned char, short>           { typedef short type; };
template <> struct restype<unsigned char, unsigned short>  { typedef short type; };
template <> struct restype<unsigned char, int>             { typedef int   type; };
template <> struct restype<unsigned char, unsigned int>    { typedef int   type; };
template <> struct restype<unsigned char, half>            { typedef half  type; };
template <> struct restype<unsigned char, float>           { typedef float type; };
template <> struct restype<unsigned char, double>          { typedef double type; };
template <> struct restype<unsigned char, long long>       { typedef long long type; };
template <> struct restype<unsigned char, unsigned long long>           { typedef long long type; };

template <> struct restype<short, char>            { typedef short type; };
template <> struct restype<short, unsigned char>   { typedef short type; };
template <> struct restype<short, short>           { typedef short type; };
template <> struct restype<short, unsigned short>  { typedef short type; };
template <> struct restype<short, int>             { typedef int   type; };
template <> struct restype<short, unsigned int>    { typedef int   type; };
template <> struct restype<short, half>            { typedef half  type; };
template <> struct restype<short, float>           { typedef float type; };
template <> struct restype<short, double>          { typedef double type; };
template <> struct restype<short, long long>       { typedef long long type; };
template <> struct restype<short, unsigned long long>           { typedef long long type; };

template <> struct restype<unsigned short, char>            { typedef short type; };
template <> struct restype<unsigned short, unsigned char>   { typedef short type; };
template <> struct restype<unsigned short, short>           { typedef short type; };
template <> struct restype<unsigned short, unsigned short>  { typedef short type; };
template <> struct restype<unsigned short, int>             { typedef int type; };
template <> struct restype<unsigned short, unsigned int>    { typedef int type; };
template <> struct restype<unsigned short, half>            { typedef half  type; };
template <> struct restype<unsigned short, float>           { typedef float type; };
template <> struct restype<unsigned short, double>          { typedef double type; };
template <> struct restype<unsigned short, long long>       { typedef long long type; };
template <> struct restype<unsigned short, unsigned long long>           { typedef long long type; };

template <> struct restype<int, char>            { typedef int type; };
template <> struct restype<int, unsigned char>   { typedef int type; };
template <> struct restype<int, short>           { typedef int type; };
template <> struct restype<int, unsigned short>  { typedef int type; };
template <> struct restype<int, int>             { typedef int type; };
template <> struct restype<int, unsigned int>    { typedef int type; };
template <> struct restype<int, half>            { typedef half  type; };
template <> struct restype<int, float>           { typedef float type; };
template <> struct restype<int, double>          { typedef double type; };
template <> struct restype<int, long long>       { typedef long long type; };
template <> struct restype<int, unsigned long long>           { typedef long long type; };

template <> struct restype<unsigned int, char>            { typedef int type; };
template <> struct restype<unsigned int, unsigned char>   { typedef int type; };
template <> struct restype<unsigned int, short>           { typedef int type; };
template <> struct restype<unsigned int, unsigned short>  { typedef int type; };
template <> struct restype<unsigned int, int>             { typedef int type; };
template <> struct restype<unsigned int, unsigned int>    { typedef int type; };
template <> struct restype<unsigned int, half>            { typedef half type; };
template <> struct restype<unsigned int, float>           { typedef float type; };
template <> struct restype<unsigned int, double>          { typedef double type; };
template <> struct restype<unsigned int, long long>       { typedef long long type; };
template <> struct restype<unsigned int, unsigned long long>           { typedef long long type; };

template <> struct restype<half, char>                    { typedef half type; };
template <> struct restype<half, unsigned char>           { typedef half type; };
template <> struct restype<half, short>                   { typedef half type; };
template <> struct restype<half, unsigned short>          { typedef half type; };
template <> struct restype<half, int>                     { typedef half type; };
template <> struct restype<half, unsigned int>            { typedef half type; };
template <> struct restype<half, half>                    { typedef half type; };
template <> struct restype<half, float>                   { typedef float type; };
template <> struct restype<half, double>                  { typedef double type; };
template <> struct restype<half, long long>               { typedef half type; };
template <> struct restype<half, unsigned long long>      { typedef half type; };

template <> struct restype<float, char>            { typedef float type; };
template <> struct restype<float, unsigned char>   { typedef float type; };
template <> struct restype<float, short>           { typedef float type; };
template <> struct restype<float, unsigned short>  { typedef float type; };
template <> struct restype<float, int>             { typedef float type; };
template <> struct restype<float, unsigned int>    { typedef float type; };
template <> struct restype<float, half>            { typedef float type; };
template <> struct restype<float, float>           { typedef float type; };
template <> struct restype<float, double>          { typedef double type; };
template <> struct restype<float, long long>       { typedef float type; };
template <> struct restype<float, unsigned long long>           { typedef float type; };

template <> struct restype<double, char>            { typedef double type; };
template <> struct restype<double, unsigned char>   { typedef double type; };
template <> struct restype<double, short>           { typedef double type; };
template <> struct restype<double, unsigned short>  { typedef double type; };
template <> struct restype<double, int>             { typedef double type; };
template <> struct restype<double, unsigned int>    { typedef double type; };
template <> struct restype<double, half>            { typedef double type; };
template <> struct restype<double, float>           { typedef double type; };
template <> struct restype<double, double>          { typedef double type; };
template <> struct restype<double, long long>       { typedef double type; };
template <> struct restype<double, unsigned long long>           { typedef double type; };

template <> struct restype<long long, char>            { typedef long long type; };
template <> struct restype<long long, unsigned char>   { typedef long long type; };
template <> struct restype<long long, short>           { typedef long long type; };
template <> struct restype<long long, unsigned short>  { typedef long long type; };
template <> struct restype<long long, int>             { typedef long long type; };
template <> struct restype<long long, unsigned int>    { typedef long long type; };
template <> struct restype<long long, half>            { typedef half type; };
template <> struct restype<long long, float>           { typedef float type; };
template <> struct restype<long long, double>          { typedef double type; };
template <> struct restype<long long, long long>       { typedef long long type; };
template <> struct restype<long long, unsigned long long>           { typedef long long type; };

template <> struct restype<unsigned long long, char>             { typedef long long type; };
template <> struct restype<unsigned long long, unsigned char>    { typedef long long type; };
template <> struct restype<unsigned long long, short>            { typedef long long type; };
template <> struct restype<unsigned long long, unsigned short>   { typedef long long type; };
template <> struct restype<unsigned long long, int>              { typedef long long type; };
template <> struct restype<unsigned long long, unsigned int>     { typedef long long type; };
template <> struct restype<unsigned long long, half>             { typedef half type; };
template <> struct restype<unsigned long long, float>            { typedef float type; };
template <> struct restype<unsigned long long, double>           { typedef double type; };
template <> struct restype<unsigned long long, long long>        { typedef long long type; };
template <> struct restype<unsigned long long, unsigned long long>            { typedef long long type; };

#else

template <> struct restype<char, char>            { typedef int  type; };
template <> struct restype<char, unsigned char>   { typedef int  type; };
template <> struct restype<char, short>           { typedef int type; };
template <> struct restype<char, unsigned short>  { typedef int type; };
template <> struct restype<char, int>             { typedef int   type; };
template <> struct restype<char, unsigned int>    { typedef unsigned int   type; };
template <> struct restype<char, long>            { typedef long   type; };
template <> struct restype<char, unsigned long>   { typedef unsigned long   type; };
template <> struct restype<char, half>            { typedef half type; };
template <> struct restype<char, float>           { typedef float type; };
template <> struct restype<char, double>          { typedef double type; };
template <> struct restype<char, long long>       { typedef long long type; };
template <> struct restype<char, unsigned long long>           { typedef unsigned long long type; };

template <> struct restype<unsigned char, char>            { typedef int  type; };
template <> struct restype<unsigned char, unsigned char>   { typedef int  type; };
template <> struct restype<unsigned char, short>           { typedef int type; };
template <> struct restype<unsigned char, unsigned short>  { typedef int type; };
template <> struct restype<unsigned char, int>             { typedef int   type; };
template <> struct restype<unsigned char, unsigned int>    { typedef unsigned int   type; };
template <> struct restype<unsigned char, long>            { typedef long   type; };
template <> struct restype<unsigned char, unsigned long>   { typedef unsigned long   type; };
template <> struct restype<unsigned char, half>            { typedef half type; };
template <> struct restype<unsigned char, float>           { typedef float type; };
template <> struct restype<unsigned char, double>          { typedef double type; };
template <> struct restype<unsigned char, long long>       { typedef long long type; };
template <> struct restype<unsigned char, unsigned long long>           { typedef unsigned long long type; };

template <> struct restype<short, char>            { typedef int type; };
template <> struct restype<short, unsigned char>   { typedef int type; };
template <> struct restype<short, short>           { typedef int type; };
template <> struct restype<short, unsigned short>  { typedef int type; };
template <> struct restype<short, int>             { typedef int   type; };
template <> struct restype<short, unsigned int>    { typedef unsigned int   type; };
template <> struct restype<short, long>            { typedef long   type; };
template <> struct restype<short, unsigned long>   { typedef unsigned long   type; };
template <> struct restype<short, half>            { typedef half type; };
template <> struct restype<short, float>           { typedef float type; };
template <> struct restype<short, double>          { typedef double type; };
template <> struct restype<short, long long>       { typedef long long type; };
template <> struct restype<short, unsigned long long>           { typedef unsigned long long type; };

template <> struct restype<unsigned short, char>            { typedef int type; };
template <> struct restype<unsigned short, unsigned char>   { typedef int type; };
template <> struct restype<unsigned short, short>           { typedef int type; };
template <> struct restype<unsigned short, unsigned short>  { typedef int type; };
template <> struct restype<unsigned short, int>             { typedef int type; };
template <> struct restype<unsigned short, unsigned int>    { typedef unsigned int type; };
template <> struct restype<unsigned short, long>            { typedef long type; };
template <> struct restype<unsigned short, unsigned long>   { typedef unsigned long type; };
template <> struct restype<unsigned short, half>            { typedef half type; };
template <> struct restype<unsigned short, float>           { typedef float type; };
template <> struct restype<unsigned short, double>          { typedef double type; };
template <> struct restype<unsigned short, long long>       { typedef long long type; };
template <> struct restype<unsigned short, unsigned long long>           { typedef unsigned long long type; };

template <> struct restype<int, char>            { typedef int type; };
template <> struct restype<int, unsigned char>   { typedef int type; };
template <> struct restype<int, short>           { typedef int type; };
template <> struct restype<int, unsigned short>  { typedef int type; };
template <> struct restype<int, int>             { typedef int type; };
template <> struct restype<int, unsigned int>    { typedef unsigned int type; };
template <> struct restype<int, long>            { typedef long type; };
template <> struct restype<int, unsigned long>   { typedef unsigned long type; };
template <> struct restype<int, half>            { typedef half type; };
template <> struct restype<int, float>           { typedef float type; };
template <> struct restype<int, double>          { typedef double type; };
template <> struct restype<int, long long>       { typedef long long type; };
template <> struct restype<int, unsigned long long>           { typedef unsigned long long type; };

template <> struct restype<unsigned int, char>            { typedef unsigned int type; };
template <> struct restype<unsigned int, unsigned char>   { typedef unsigned int type; };
template <> struct restype<unsigned int, short>           { typedef unsigned int type; };
template <> struct restype<unsigned int, unsigned short>  { typedef unsigned int type; };
template <> struct restype<unsigned int, int>             { typedef unsigned int type; };
template <> struct restype<unsigned int, unsigned int>    { typedef unsigned int type; };
template <> struct restype<unsigned int, long>            { typedef long type; };
template <> struct restype<unsigned int, unsigned long>   { typedef unsigned long type; };
template <> struct restype<unsigned int, half>            { typedef half type; };
template <> struct restype<unsigned int, float>           { typedef float type; };
template <> struct restype<unsigned int, double>          { typedef double type; };
template <> struct restype<unsigned int, long long>       { typedef long long type; };
template <> struct restype<unsigned int, unsigned long long>           { typedef unsigned long long type; };

template <> struct restype<long, char>            { typedef long type; };
template <> struct restype<long, unsigned char>   { typedef long type; };
template <> struct restype<long, short>           { typedef long type; };
template <> struct restype<long, unsigned short>  { typedef long type; };
template <> struct restype<long, int>             { typedef long type; };
template <> struct restype<long, unsigned int>    { typedef long type; };
template <> struct restype<long, long>            { typedef long type; };
template <> struct restype<long, unsigned long>   { typedef unsigned long type; };
template <> struct restype<long, half>            { typedef half type; };
template <> struct restype<long, float>           { typedef float type; };
template <> struct restype<long, double>          { typedef double type; };
template <> struct restype<long, long long>       { typedef long long type; };
template <> struct restype<long, unsigned long long>           { typedef unsigned long long type; };

template <> struct restype<unsigned long, char>            { typedef unsigned long type; };
template <> struct restype<unsigned long, unsigned char>   { typedef unsigned long type; };
template <> struct restype<unsigned long, short>           { typedef unsigned long type; };
template <> struct restype<unsigned long, unsigned short>  { typedef unsigned long type; };
template <> struct restype<unsigned long, int>             { typedef unsigned long type; };
template <> struct restype<unsigned long, unsigned int>    { typedef unsigned long type; };
template <> struct restype<unsigned long, long>            { typedef unsigned long type; };
template <> struct restype<unsigned long, unsigned long>   { typedef unsigned long type; };
template <> struct restype<unsigned long, half>            { typedef half type; };
template <> struct restype<unsigned long, float>           { typedef float type; };
template <> struct restype<unsigned long, double>          { typedef double type; };
template <> struct restype<unsigned long, long long>       { typedef long long type; };
template <> struct restype<unsigned long, unsigned long long>           { typedef unsigned long long type; };

template <> struct restype<half, char>             { typedef half type; };
template <> struct restype<half, unsigned char>    { typedef half type; };
template <> struct restype<half, short>            { typedef half type; };
template <> struct restype<half, unsigned short>   { typedef half type; };
template <> struct restype<half, int>              { typedef half type; };
template <> struct restype<half, unsigned int>     { typedef half type; };
template <> struct restype<half, long>             { typedef half type; };
template <> struct restype<half, unsigned long>    { typedef half type; };
template <> struct restype<half, half>             { typedef half type; };
template <> struct restype<half, float>            { typedef float type; };
template <> struct restype<half, double>           { typedef double type; };
template <> struct restype<half, long long>        { typedef half type; };
template <> struct restype<half, unsigned long long> { typedef half type; };

template <> struct restype<float, char>            { typedef float type; };
template <> struct restype<float, unsigned char>   { typedef float type; };
template <> struct restype<float, short>           { typedef float type; };
template <> struct restype<float, unsigned short>  { typedef float type; };
template <> struct restype<float, int>             { typedef float type; };
template <> struct restype<float, unsigned int>    { typedef float type; };
template <> struct restype<float, long>            { typedef float type; };
template <> struct restype<float, unsigned long>   { typedef float type; };
template <> struct restype<float, half>            { typedef float type; };
template <> struct restype<float, float>           { typedef float type; };
template <> struct restype<float, double>          { typedef double type; };
template <> struct restype<float, long long>       { typedef float type; };
template <> struct restype<float, unsigned long long>           { typedef float type; };

template <> struct restype<double, char>            { typedef double type; };
template <> struct restype<double, unsigned char>   { typedef double type; };
template <> struct restype<double, short>           { typedef double type; };
template <> struct restype<double, unsigned short>  { typedef double type; };
template <> struct restype<double, int>             { typedef double type; };
template <> struct restype<double, unsigned int>    { typedef double type; };
template <> struct restype<double, long>            { typedef double type; };
template <> struct restype<double, unsigned long>   { typedef double type; };
template <> struct restype<double, half>            { typedef double type; };
template <> struct restype<double, float>           { typedef double type; };
template <> struct restype<double, double>          { typedef double type; };
template <> struct restype<double, long long>       { typedef double type; };
template <> struct restype<double, unsigned long long>           { typedef double type; };

template <> struct restype<unsigned long long, char>            { typedef unsigned long long type; };
template <> struct restype<unsigned long long, unsigned char>   { typedef unsigned long long type; };
template <> struct restype<unsigned long long, short>           { typedef unsigned long long type; };
template <> struct restype<unsigned long long, unsigned short>  { typedef unsigned long long type; };
template <> struct restype<unsigned long long, int>             { typedef unsigned long long type; };
template <> struct restype<unsigned long long, unsigned int>    { typedef unsigned long long type; };
template <> struct restype<unsigned long long, long>            { typedef unsigned long long type; };
template <> struct restype<unsigned long long, unsigned long>   { typedef unsigned long long type; };
template <> struct restype<unsigned long long, half>            { typedef half type; };
template <> struct restype<unsigned long long, float>           { typedef float type; };
template <> struct restype<unsigned long long, double>          { typedef double type; };
template <> struct restype<unsigned long long, long long>       { typedef unsigned long long type; };
template <> struct restype<unsigned long long, unsigned long long>           { typedef unsigned long long type; };

template <> struct restype<long long, char>            { typedef long long type; };
template <> struct restype<long long, unsigned char>   { typedef long long type; };
template <> struct restype<long long, short>           { typedef long long type; };
template <> struct restype<long long, unsigned short>  { typedef long long type; };
template <> struct restype<long long, int>             { typedef long long type; };
template <> struct restype<long long, unsigned int>    { typedef long long type; };
template <> struct restype<long long, long>            { typedef long long type; };
template <> struct restype<long long, unsigned long>   { typedef long long type; };
template <> struct restype<long long, half>            { typedef half type; };
template <> struct restype<long long, float>           { typedef float type; };
template <> struct restype<long long, double>          { typedef double type; };
template <> struct restype<long long, long long>       { typedef long long type; };
template <> struct restype<long long, unsigned long long>           { typedef unsigned long long type; };

#endif

template <typename T1, typename T2> struct bitwise_restype {
private:
    bitwise_restype();
};

#if defined(CM_NOCONV)

template <> struct bitwise_restype<char, char>            { typedef char  type; };
template <> struct bitwise_restype<char, unsigned char>   { typedef short  type; };
template <> struct bitwise_restype<char, short>           { typedef short type; };
template <> struct bitwise_restype<char, unsigned short>  { typedef short type; };
template <> struct bitwise_restype<char, int>             { typedef int   type; };
template <> struct bitwise_restype<char, unsigned int>    { typedef int   type; };
template <> struct bitwise_restype<char, half>            { typedef half type; };
template <> struct bitwise_restype<char, float>           { typedef float type; };
template <> struct bitwise_restype<char, double>          { typedef double type; };
template <> struct bitwise_restype<char, long long>       { typedef long long type; };
template <> struct bitwise_restype<char, unsigned long long>           { typedef long long type; };

template <> struct bitwise_restype<unsigned char, char>            { typedef char  type; };
template <> struct bitwise_restype<unsigned char, unsigned char>   { typedef char  type; };
template <> struct bitwise_restype<unsigned char, short>           { typedef short type; };
template <> struct bitwise_restype<unsigned char, unsigned short>  { typedef short type; };
template <> struct bitwise_restype<unsigned char, int>             { typedef int   type; };
template <> struct bitwise_restype<unsigned char, unsigned int>    { typedef int   type; };
template <> struct bitwise_restype<unsigned char, half>            { typedef half type; };
template <> struct bitwise_restype<unsigned char, float>           { typedef float type; };
template <> struct bitwise_restype<unsigned char, double>          { typedef double type; };
template <> struct bitwise_restype<unsigned char, long long>       { typedef long long type; };
template <> struct bitwise_restype<unsigned char, unsigned long long>           { typedef long long type; };

template <> struct bitwise_restype<short, char>            { typedef short type; };
template <> struct bitwise_restype<short, unsigned char>   { typedef short type; };
template <> struct bitwise_restype<short, short>           { typedef short type; };
template <> struct bitwise_restype<short, unsigned short>  { typedef short type; };
template <> struct bitwise_restype<short, int>             { typedef int   type; };
template <> struct bitwise_restype<short, unsigned int>    { typedef int   type; };
template <> struct bitwise_restype<short, half>            { typedef half type; };
template <> struct bitwise_restype<short, float>           { typedef float type; };
template <> struct bitwise_restype<short, double>          { typedef double type; };
template <> struct bitwise_restype<short, long long>       { typedef long long type; };
template <> struct bitwise_restype<short, unsigned long long>           { typedef long long type; };

template <> struct bitwise_restype<unsigned short, char>            { typedef short type; };
template <> struct bitwise_restype<unsigned short, unsigned char>   { typedef short type; };
template <> struct bitwise_restype<unsigned short, short>           { typedef short type; };
template <> struct bitwise_restype<unsigned short, unsigned short>  { typedef short type; };
template <> struct bitwise_restype<unsigned short, int>             { typedef int type; };
template <> struct bitwise_restype<unsigned short, unsigned int>    { typedef int type; };
template <> struct bitwise_restype<unsigned short, half>            { typedef half type; };
template <> struct bitwise_restype<unsigned short, float>           { typedef float type; };
template <> struct bitwise_restype<unsigned short, double>          { typedef double type; };
template <> struct bitwise_restype<unsigned short, long long>       { typedef long long type; };
template <> struct bitwise_restype<unsigned short, unsigned long long>           { typedef long long type; };

template <> struct bitwise_restype<int, char>            { typedef int type; };
template <> struct bitwise_restype<int, unsigned char>   { typedef int type; };
template <> struct bitwise_restype<int, short>           { typedef int type; };
template <> struct bitwise_restype<int, unsigned short>  { typedef int type; };
template <> struct bitwise_restype<int, int>             { typedef int type; };
template <> struct bitwise_restype<int, unsigned int>    { typedef int type; };
template <> struct bitwise_restype<int, half>            { typedef half type; };
template <> struct bitwise_restype<int, float>           { typedef float type; };
template <> struct bitwise_restype<int, double>          { typedef double type; };
template <> struct bitwise_restype<int, long long>       { typedef long long type; };
template <> struct bitwise_restype<int, unsigned long long>           { typedef long long type; };

template <> struct bitwise_restype<unsigned int, char>            { typedef int type; };
template <> struct bitwise_restype<unsigned int, unsigned char>   { typedef int type; };
template <> struct bitwise_restype<unsigned int, short>           { typedef int type; };
template <> struct bitwise_restype<unsigned int, unsigned short>  { typedef int type; };
template <> struct bitwise_restype<unsigned int, int>             { typedef int type; };
template <> struct bitwise_restype<unsigned int, unsigned int>    { typedef int type; };
template <> struct bitwise_restype<unsigned int, half>            { typedef half type; };
template <> struct bitwise_restype<unsigned int, float>           { typedef float type; };
template <> struct bitwise_restype<unsigned int, double>          { typedef double type; };
template <> struct bitwise_restype<unsigned int, long long>       { typedef long long type; };
template <> struct bitwise_restype<unsigned int, unsigned long long>           { typedef long long type; };

template <> struct bitwise_restype<half, char>             { typedef half type; };
template <> struct bitwise_restype<half, unsigned char>    { typedef half type; };
template <> struct bitwise_restype<half, short>            { typedef half type; };
template <> struct bitwise_restype<half, unsigned short>   { typedef half type; };
template <> struct bitwise_restype<half, int>              { typedef half type; };
template <> struct bitwise_restype<half, unsigned int>     { typedef half type; };
template <> struct bitwise_restype<half, half>             { typedef half type; };
template <> struct bitwise_restype<half, float>            { typedef float type; };
template <> struct bitwise_restype<half, double>           { typedef double type; };
template <> struct bitwise_restype<half, long long>        { typedef half type; };
template <> struct bitwise_restype<half, unsigned long long>            { typedef half type; };

template <> struct bitwise_restype<float, char>            { typedef float type; };
template <> struct bitwise_restype<float, unsigned char>   { typedef float type; };
template <> struct bitwise_restype<float, short>           { typedef float type; };
template <> struct bitwise_restype<float, unsigned short>  { typedef float type; };
template <> struct bitwise_restype<float, int>             { typedef float type; };
template <> struct bitwise_restype<float, unsigned int>    { typedef float type; };
template <> struct bitwise_restype<float, half>            { typedef float type; };
template <> struct bitwise_restype<float, float>           { typedef float type; };
template <> struct bitwise_restype<float, double>          { typedef double type; };
template <> struct bitwise_restype<float, long long>       { typedef float type; };
template <> struct bitwise_restype<float, unsigned long long>           { typedef float type; };

template <> struct bitwise_restype<double, char>            { typedef double type; };
template <> struct bitwise_restype<double, unsigned char>   { typedef double type; };
template <> struct bitwise_restype<double, short>           { typedef double type; };
template <> struct bitwise_restype<double, unsigned short>  { typedef double type; };
template <> struct bitwise_restype<double, int>             { typedef double type; };
template <> struct bitwise_restype<double, unsigned int>    { typedef double type; };
template <> struct bitwise_restype<double, half>            { typedef double type; };
template <> struct bitwise_restype<double, float>           { typedef double type; };
template <> struct bitwise_restype<double, double>          { typedef double type; };
template <> struct bitwise_restype<double, long long>       { typedef double type; };
template <> struct bitwise_restype<double, unsigned long long>           { typedef double type; };

template <> struct bitwise_restype<long long, char>            { typedef long long type; };
template <> struct bitwise_restype<long long, unsigned char>   { typedef long long type; };
template <> struct bitwise_restype<long long, short>           { typedef long long type; };
template <> struct bitwise_restype<long long, unsigned short>  { typedef long long type; };
template <> struct bitwise_restype<long long, int>             { typedef long long type; };
template <> struct bitwise_restype<long long, unsigned int>    { typedef long long type; };
template <> struct bitwise_restype<long long, half>            { typedef half type; };
template <> struct bitwise_restype<long long, float>           { typedef float type; };
template <> struct bitwise_restype<long long, double>          { typedef double type; };
template <> struct bitwise_restype<long long, long long>       { typedef long long type; };
template <> struct bitwise_restype<long long, unsigned long long>           { typedef long long type; };

template <> struct bitwise_restype<unsigned long long, char>            { typedef long long type; };
template <> struct bitwise_restype<unsigned long long, unsigned char>   { typedef long long type; };
template <> struct bitwise_restype<unsigned long long, short>           { typedef long long type; };
template <> struct bitwise_restype<unsigned long long, unsigned short>  { typedef long long type; };
template <> struct bitwise_restype<unsigned long long, int>             { typedef long long type; };
template <> struct bitwise_restype<unsigned long long, unsigned int>    { typedef long long type; };
template <> struct bitwise_restype<unsigned long long, half>            { typedef half type; };
template <> struct bitwise_restype<unsigned long long, float>           { typedef float type; };
template <> struct bitwise_restype<unsigned long long, double>          { typedef double type; };
template <> struct bitwise_restype<unsigned long long, long long>       { typedef long long type; };
template <> struct bitwise_restype<unsigned long long, unsigned long long>           { typedef long long type; };
#else

template <> struct bitwise_restype<char, char>            { typedef char  type; };
template <> struct bitwise_restype<char, unsigned char>   { typedef unsigned char  type; };
template <> struct bitwise_restype<char, short>           { typedef short type; };
template <> struct bitwise_restype<char, unsigned short>  { typedef unsigned short type; };
template <> struct bitwise_restype<char, int>             { typedef int   type; };
template <> struct bitwise_restype<char, unsigned int>    { typedef unsigned int   type; };
template <> struct bitwise_restype<char, long>            { typedef long   type; };
template <> struct bitwise_restype<char, unsigned long>   { typedef unsigned long   type; };
template <> struct bitwise_restype<char, half>            { typedef half type; };
template <> struct bitwise_restype<char, float>           { typedef float type; };
template <> struct bitwise_restype<char, double>          { typedef double type; };
template <> struct bitwise_restype<char, long long>       { typedef long long type; };
template <> struct bitwise_restype<char, unsigned long long>           { typedef unsigned long long type; };

template <> struct bitwise_restype<unsigned char, char>            { typedef char type; };
template <> struct bitwise_restype<unsigned char, unsigned char>   { typedef unsigned char type; };
template <> struct bitwise_restype<unsigned char, short>           { typedef short type; };
template <> struct bitwise_restype<unsigned char, unsigned short>  { typedef unsigned short type; };
template <> struct bitwise_restype<unsigned char, int>             { typedef int   type; };
template <> struct bitwise_restype<unsigned char, unsigned int>    { typedef unsigned int   type; };
template <> struct bitwise_restype<unsigned char, long>            { typedef long   type; };
template <> struct bitwise_restype<unsigned char, unsigned long>   { typedef unsigned long   type; };
template <> struct bitwise_restype<unsigned char, half>            { typedef half type; };
template <> struct bitwise_restype<unsigned char, float>           { typedef float type; };
template <> struct bitwise_restype<unsigned char, double>          { typedef double type; };
template <> struct bitwise_restype<unsigned char, long long>       { typedef long long type; };
template <> struct bitwise_restype<unsigned char, unsigned long long>           { typedef unsigned long long type; };

template <> struct bitwise_restype<short, char>            { typedef short type; };
template <> struct bitwise_restype<short, unsigned char>   { typedef short type; };
template <> struct bitwise_restype<short, short>           { typedef short type; };
template <> struct bitwise_restype<short, unsigned short>  { typedef unsigned short type; };
template <> struct bitwise_restype<short, int>             { typedef int   type; };
template <> struct bitwise_restype<short, unsigned int>    { typedef unsigned int   type; };
template <> struct bitwise_restype<short, long>            { typedef long   type; };
template <> struct bitwise_restype<short, unsigned long>   { typedef unsigned long   type; };
template <> struct bitwise_restype<short, half>            { typedef half type; };
template <> struct bitwise_restype<short, float>           { typedef float type; };
template <> struct bitwise_restype<short, double>          { typedef double type; };
template <> struct bitwise_restype<short, long long>       { typedef long long type; };
template <> struct bitwise_restype<short, unsigned long long>           { typedef unsigned long long type; };

template <> struct bitwise_restype<unsigned short, char>            { typedef unsigned short type; };
template <> struct bitwise_restype<unsigned short, unsigned char>   { typedef unsigned short type; };
template <> struct bitwise_restype<unsigned short, short>           { typedef unsigned short type; };
template <> struct bitwise_restype<unsigned short, unsigned short>  { typedef unsigned short type; };
template <> struct bitwise_restype<unsigned short, int>             { typedef int type; };
template <> struct bitwise_restype<unsigned short, unsigned int>    { typedef unsigned int type; };
template <> struct bitwise_restype<unsigned short, long>            { typedef long type; };
template <> struct bitwise_restype<unsigned short, unsigned long>   { typedef unsigned long type; };
template <> struct bitwise_restype<unsigned short, half>            { typedef half type; };
template <> struct bitwise_restype<unsigned short, float>           { typedef float type; };
template <> struct bitwise_restype<unsigned short, double>          { typedef double type; };
template <> struct bitwise_restype<unsigned short, long long>       { typedef long long type; };
template <> struct bitwise_restype<unsigned short, unsigned long long>           { typedef unsigned long long type; };

template <> struct bitwise_restype<int, char>            { typedef int type; };
template <> struct bitwise_restype<int, unsigned char>   { typedef int type; };
template <> struct bitwise_restype<int, short>           { typedef int type; };
template <> struct bitwise_restype<int, unsigned short>  { typedef int type; };
template <> struct bitwise_restype<int, int>             { typedef int type; };
template <> struct bitwise_restype<int, unsigned int>    { typedef unsigned int type; };
template <> struct bitwise_restype<int, long>            { typedef long type; };
template <> struct bitwise_restype<int, unsigned long>   { typedef unsigned long type; };
template <> struct bitwise_restype<int, half>            { typedef half type; };
template <> struct bitwise_restype<int, float>           { typedef float type; };
template <> struct bitwise_restype<int, double>          { typedef double type; };
template <> struct bitwise_restype<int, long long>       { typedef long long type; };
template <> struct bitwise_restype<int, unsigned long long>           { typedef unsigned long long type; };

template <> struct bitwise_restype<unsigned int, char>            { typedef unsigned int type; };
template <> struct bitwise_restype<unsigned int, unsigned char>   { typedef unsigned int type; };
template <> struct bitwise_restype<unsigned int, short>           { typedef unsigned int type; };
template <> struct bitwise_restype<unsigned int, unsigned short>  { typedef unsigned int type; };
template <> struct bitwise_restype<unsigned int, int>             { typedef unsigned int type; };
template <> struct bitwise_restype<unsigned int, unsigned int>    { typedef unsigned int type; };
template <> struct bitwise_restype<unsigned int, long>            { typedef unsigned long type; };
template <> struct bitwise_restype<unsigned int, unsigned long>   { typedef unsigned long type; };
template <> struct bitwise_restype<unsigned int, half>            { typedef half type; };
template <> struct bitwise_restype<unsigned int, float>           { typedef float type; };
template <> struct bitwise_restype<unsigned int, double>          { typedef double type; };
template <> struct bitwise_restype<unsigned int, long long>       { typedef long long type; };
template <> struct bitwise_restype<unsigned int, unsigned long long>           { typedef unsigned long long type; };

template <> struct bitwise_restype<half, char>             { typedef half type; };
template <> struct bitwise_restype<half, unsigned char>    { typedef half type; };
template <> struct bitwise_restype<half, short>            { typedef half type; };
template <> struct bitwise_restype<half, unsigned short>   { typedef half type; };
template <> struct bitwise_restype<half, int>              { typedef half type; };
template <> struct bitwise_restype<half, unsigned int>     { typedef half type; };
template <> struct bitwise_restype<half, long>             { typedef half type; };
template <> struct bitwise_restype<half, unsigned long>    { typedef half type; };
template <> struct bitwise_restype<half, half>             { typedef half type; };
template <> struct bitwise_restype<half, float>            { typedef float type; };
template <> struct bitwise_restype<half, double>           { typedef double type; };
template <> struct bitwise_restype<half, long long>        { typedef half type; };
template <> struct bitwise_restype<half, unsigned long long>            { typedef half type; };

template <> struct bitwise_restype<float, char>            { typedef float type; };
template <> struct bitwise_restype<float, unsigned char>   { typedef float type; };
template <> struct bitwise_restype<float, short>           { typedef float type; };
template <> struct bitwise_restype<float, unsigned short>  { typedef float type; };
template <> struct bitwise_restype<float, int>             { typedef float type; };
template <> struct bitwise_restype<float, unsigned int>    { typedef float type; };
template <> struct bitwise_restype<float, long>            { typedef float type; };
template <> struct bitwise_restype<float, unsigned long>   { typedef float type; };
template <> struct bitwise_restype<float, half>            { typedef float type; };
template <> struct bitwise_restype<float, float>           { typedef float type; };
template <> struct bitwise_restype<float, double>          { typedef double type; };
template <> struct bitwise_restype<float, long long>       { typedef float type; };
template <> struct bitwise_restype<float, unsigned long long>           { typedef float type; };

template <> struct bitwise_restype<double, char>            { typedef double type; };
template <> struct bitwise_restype<double, unsigned char>   { typedef double type; };
template <> struct bitwise_restype<double, short>           { typedef double type; };
template <> struct bitwise_restype<double, unsigned short>  { typedef double type; };
template <> struct bitwise_restype<double, int>             { typedef double type; };
template <> struct bitwise_restype<double, unsigned int>    { typedef double type; };
template <> struct bitwise_restype<double, long>            { typedef double type; };
template <> struct bitwise_restype<double, unsigned long>   { typedef double type; };
template <> struct bitwise_restype<double, half>            { typedef double type; };
template <> struct bitwise_restype<double, float>           { typedef double type; };
template <> struct bitwise_restype<double, double>          { typedef double type; };
template <> struct bitwise_restype<double, long long>       { typedef double type; };
template <> struct bitwise_restype<double, unsigned long long>           { typedef double type; };

template <> struct bitwise_restype<long long, char>            { typedef long long type; };
template <> struct bitwise_restype<long long, unsigned char>   { typedef long long type; };
template <> struct bitwise_restype<long long, short>           { typedef long long type; };
template <> struct bitwise_restype<long long, unsigned short>  { typedef long long type; };
template <> struct bitwise_restype<long long, int>             { typedef long long type; };
template <> struct bitwise_restype<long long, unsigned int>    { typedef long long type; };
template <> struct bitwise_restype<long long, long>            { typedef long long type; };
template <> struct bitwise_restype<long long, unsigned long>   { typedef long long type; };
template <> struct bitwise_restype<long long, half >           { typedef half type; };
template <> struct bitwise_restype<long long, float>           { typedef float type; };
template <> struct bitwise_restype<long long, double>          { typedef double type; };
template <> struct bitwise_restype<long long, long long>       { typedef long long type; };
template <> struct bitwise_restype<long long, unsigned long long>           { typedef unsigned long long type; };

template <> struct bitwise_restype<unsigned long long, char>            { typedef unsigned long long type; };
template <> struct bitwise_restype<unsigned long long, unsigned char>   { typedef unsigned long long type; };
template <> struct bitwise_restype<unsigned long long, short>           { typedef unsigned long long type; };
template <> struct bitwise_restype<unsigned long long, unsigned short>  { typedef unsigned long long type; };
template <> struct bitwise_restype<unsigned long long, int>             { typedef unsigned long long type; };
template <> struct bitwise_restype<unsigned long long, unsigned int>    { typedef unsigned long long type; };
template <> struct bitwise_restype<unsigned long long, long>            { typedef unsigned long long type; };
template <> struct bitwise_restype<unsigned long long, unsigned long>   { typedef unsigned long long type; };
template <> struct bitwise_restype<unsigned long long, half>            { typedef half type; };
template <> struct bitwise_restype<unsigned long long, float>           { typedef float type; };
template <> struct bitwise_restype<unsigned long long, double>          { typedef double type; };
template <> struct bitwise_restype<unsigned long long, long long>       { typedef unsigned long long type; };
template <> struct bitwise_restype<unsigned long long, unsigned long long>           { typedef unsigned long long type; };

#endif

template <typename T1, typename T2> struct restype_ex {
private:
    restype_ex();
};
//#ifdef CM_NONSTRICT

template <> struct restype_ex<char, char>            { typedef int  type; };
template <> struct restype_ex<char, unsigned char>   { typedef int  type; };
template <> struct restype_ex<char, short>           { typedef int type; };
template <> struct restype_ex<char, unsigned short>  { typedef int type; };
template <> struct restype_ex<char, int>             { typedef long long   type; };
template <> struct restype_ex<char, unsigned int>    { typedef long long   type; };
template <> struct restype_ex<char, long>            { typedef long long   type; };
template <> struct restype_ex<char, unsigned long>   { typedef long long   type; };
template <> struct restype_ex<char, half>            { typedef half  type; };
template <> struct restype_ex<char, float>           { typedef float type; };
template <> struct restype_ex<char, double>           { typedef double type; };
template <> struct restype_ex<char, long long>       { typedef long long type; };
template <> struct restype_ex<char, unsigned long long>           { typedef long long type; };

template <> struct restype_ex<unsigned char, char>            { typedef int  type; };
template <> struct restype_ex<unsigned char, unsigned char>   { typedef int  type; };
template <> struct restype_ex<unsigned char, short>           { typedef int type; };
template <> struct restype_ex<unsigned char, unsigned short>  { typedef int type; };
template <> struct restype_ex<unsigned char, int>             { typedef long long   type; };
template <> struct restype_ex<unsigned char, unsigned int>    { typedef long long   type; };
template <> struct restype_ex<unsigned char, long>            { typedef long long   type; };
template <> struct restype_ex<unsigned char, unsigned long>   { typedef long long   type; };
template <> struct restype_ex<unsigned char, half>            { typedef half  type; };
template <> struct restype_ex<unsigned char, float>           { typedef float type; };
template <> struct restype_ex<unsigned char, double>          { typedef double type; };
template <> struct restype_ex<unsigned char, long long>       { typedef long long type; };
template <> struct restype_ex<unsigned char, unsigned long long>           { typedef long long type; };

template <> struct restype_ex<short, char>            { typedef int type; };
template <> struct restype_ex<short, unsigned char>   { typedef int type; };
template <> struct restype_ex<short, short>           { typedef int type; };
template <> struct restype_ex<short, unsigned short>  { typedef int type; };
template <> struct restype_ex<short, int>             { typedef long long   type; };
template <> struct restype_ex<short, unsigned int>    { typedef long long   type; };
template <> struct restype_ex<short, long>            { typedef long long   type; };
template <> struct restype_ex<short, unsigned long>   { typedef long long   type; };
template <> struct restype_ex<short, half>            { typedef half  type; };
template <> struct restype_ex<short, float>           { typedef float type; };
template <> struct restype_ex<short, double>          { typedef double type; };
template <> struct restype_ex<short, long long>       { typedef long long type; };
template <> struct restype_ex<short, unsigned long long>           { typedef long long type; };

template <> struct restype_ex<unsigned short, char>            { typedef int type; };
template <> struct restype_ex<unsigned short, unsigned char>   { typedef int type; };
template <> struct restype_ex<unsigned short, short>           { typedef int type; };
template <> struct restype_ex<unsigned short, unsigned short>  { typedef int type; };
template <> struct restype_ex<unsigned short, int>             { typedef long long type; };
template <> struct restype_ex<unsigned short, unsigned int>    { typedef long long type; };
template <> struct restype_ex<unsigned short, long>            { typedef long long type; };
template <> struct restype_ex<unsigned short, unsigned long>   { typedef long long type; };
template <> struct restype_ex<unsigned short, half>            { typedef half type; };
template <> struct restype_ex<unsigned short, float>           { typedef float type; };
template <> struct restype_ex<unsigned short, double>          { typedef double type; };
template <> struct restype_ex<unsigned short, long long>       { typedef long long type; };
template <> struct restype_ex<unsigned short, unsigned long long>           { typedef long long type; };

template <> struct restype_ex<int, char>            { typedef long long type; };
template <> struct restype_ex<int, unsigned char>   { typedef long long type; };
template <> struct restype_ex<int, short>           { typedef long long type; };
template <> struct restype_ex<int, unsigned short>  { typedef long long type; };
template <> struct restype_ex<int, int>             { typedef long long type; };
template <> struct restype_ex<int, unsigned int>    { typedef long long type; };
template <> struct restype_ex<int, long>            { typedef long long type; };
template <> struct restype_ex<int, unsigned long>   { typedef long long type; };
template <> struct restype_ex<int, half>            { typedef half type; };
template <> struct restype_ex<int, float>           { typedef float type; };
template <> struct restype_ex<int, double>          { typedef double type; };
template <> struct restype_ex<int, long long>       { typedef long long type; };
template <> struct restype_ex<int, unsigned long long>           { typedef long long type; };

template <> struct restype_ex<unsigned int, char>            { typedef long long type; };
template <> struct restype_ex<unsigned int, unsigned char>   { typedef long long type; };
template <> struct restype_ex<unsigned int, short>           { typedef long long type; };
template <> struct restype_ex<unsigned int, unsigned short>  { typedef long long type; };
template <> struct restype_ex<unsigned int, int>             { typedef long long type; };
template <> struct restype_ex<unsigned int, unsigned int>    { typedef long long type; };
template <> struct restype_ex<unsigned int, long>            { typedef long long type; };
template <> struct restype_ex<unsigned int, unsigned long>   { typedef long long type; };
template <> struct restype_ex<unsigned int, half>            { typedef half type; };
template <> struct restype_ex<unsigned int, float>           { typedef float type; };
template <> struct restype_ex<unsigned int, double>          { typedef double type; };
template <> struct restype_ex<unsigned int, long long>       { typedef long long type; };
template <> struct restype_ex<unsigned int, unsigned long long>           { typedef long long type; };

template <> struct restype_ex<half, char>             { typedef half type; };
template <> struct restype_ex<half, unsigned char>    { typedef half type; };
template <> struct restype_ex<half, short>            { typedef half type; };
template <> struct restype_ex<half, unsigned short>   { typedef half type; };
template <> struct restype_ex<half, int>              { typedef half type; };
template <> struct restype_ex<half, unsigned int>     { typedef half type; };
template <> struct restype_ex<half, long>             { typedef half type; };
template <> struct restype_ex<half, unsigned long>    { typedef half type; };
template <> struct restype_ex<half, half>             { typedef half type; };
template <> struct restype_ex<half, float>            { typedef float type; };
template <> struct restype_ex<half, double>           { typedef double type; };
template <> struct restype_ex<half, long long>        { typedef half type; };
template <> struct restype_ex<half, unsigned long long>            { typedef half type; };

template <> struct restype_ex<float, char>            { typedef float type; };
template <> struct restype_ex<float, unsigned char>   { typedef float type; };
template <> struct restype_ex<float, short>           { typedef float type; };
template <> struct restype_ex<float, unsigned short>  { typedef float type; };
template <> struct restype_ex<float, int>             { typedef float type; };
template <> struct restype_ex<float, unsigned int>    { typedef float type; };
template <> struct restype_ex<float, long>            { typedef float type; };
template <> struct restype_ex<float, unsigned long>   { typedef float type; };
template <> struct restype_ex<float, half>            { typedef float type; };
template <> struct restype_ex<float, float>           { typedef float type; };
template <> struct restype_ex<float, double>          { typedef double type; };
template <> struct restype_ex<float, long long>       { typedef float type; };
template <> struct restype_ex<float, unsigned long long>           { typedef float type; };

template <> struct restype_ex<double, char>            { typedef double type; };
template <> struct restype_ex<double, unsigned char>   { typedef double type; };
template <> struct restype_ex<double, short>           { typedef double type; };
template <> struct restype_ex<double, unsigned short>  { typedef double type; };
template <> struct restype_ex<double, int>             { typedef double type; };
template <> struct restype_ex<double, unsigned int>    { typedef double type; };
template <> struct restype_ex<double, long>            { typedef double type; };
template <> struct restype_ex<double, unsigned long>   { typedef double type; };
template <> struct restype_ex<double, half>            { typedef double type; };
template <> struct restype_ex<double, float>           { typedef double type; };
template <> struct restype_ex<double, double>          { typedef double type; };
template <> struct restype_ex<double, long long>       { typedef double type; };
template <> struct restype_ex<double, unsigned long long>           { typedef double type; };

template <> struct restype_ex<long long, char>            { typedef long long type; };
template <> struct restype_ex<long long, unsigned char>   { typedef long long type; };
template <> struct restype_ex<long long, short>           { typedef long long type; };
template <> struct restype_ex<long long, unsigned short>  { typedef long long type; };
template <> struct restype_ex<long long, int>             { typedef long long type; };
template <> struct restype_ex<long long, unsigned int>    { typedef long long type; };
template <> struct restype_ex<long long, long>            { typedef long long type; };
template <> struct restype_ex<long long, unsigned long>   { typedef long long type; };
template <> struct restype_ex<long long, half>            { typedef half type; };
template <> struct restype_ex<long long, float>           { typedef float type; };
template <> struct restype_ex<long long, double>          { typedef double type; };
template <> struct restype_ex<long long, long long>       { typedef long long type; };
template <> struct restype_ex<long long, unsigned long long>           { typedef long long type; };

template <> struct restype_ex<unsigned long long, char>            { typedef long long type; };
template <> struct restype_ex<unsigned long long, unsigned char>   { typedef long long type; };
template <> struct restype_ex<unsigned long long, short>           { typedef long long type; };
template <> struct restype_ex<unsigned long long, unsigned short>  { typedef long long type; };
template <> struct restype_ex<unsigned long long, int>             { typedef long long type; };
template <> struct restype_ex<unsigned long long, unsigned int>    { typedef long long type; };
template <> struct restype_ex<unsigned long long, long>            { typedef long long type; };
template <> struct restype_ex<unsigned long long, unsigned long>   { typedef long long type; };
template <> struct restype_ex<unsigned long long, half>            { typedef half type; };
template <> struct restype_ex<unsigned long long, float>           { typedef float type; };
template <> struct restype_ex<unsigned long long, double>          { typedef double type; };
template <> struct restype_ex<unsigned long long, long long>       { typedef long long type; };
template <> struct restype_ex<unsigned long long, unsigned long long>           { typedef long long type; };

template <typename T> struct maxtype;
template<> struct maxtype<half>        { typedef half type; };
template<> struct maxtype<float>       { typedef float type; };
template<> struct maxtype<char>        { typedef int type; };
template<> struct maxtype<short>       { typedef int type; };
template<> struct maxtype<int>         { typedef int type; };
template<> struct maxtype<uchar>       { typedef uint type; };
template<> struct maxtype<ushort>      { typedef uint type; };
template<> struct maxtype<uint>        { typedef uint type; };
template<> struct maxtype<double>      { typedef double type; };
template<> struct maxtype<long long>   { typedef long long type; };
template<> struct maxtype<unsigned long long>       { typedef unsigned long long type; };

template <typename T1, typename T2> struct uchar_type {
private:
        uchar_type();
};
template <> struct uchar_type<char, char>            { typedef uchar type; };
template <> struct uchar_type<char, unsigned char>   { typedef uchar type; };
template <> struct uchar_type<char, short>           { typedef uchar type; };
template <> struct uchar_type<char, unsigned short>  { typedef uchar type; };
template <> struct uchar_type<char, int>             { typedef uchar type; };
template <> struct uchar_type<char, unsigned int>    { typedef uchar type; };
template <> struct uchar_type<char, half>            { typedef uchar type; };
template <> struct uchar_type<char, float>           { typedef uchar type; };
template <> struct uchar_type<char, double>          { typedef uchar type; };
template <> struct uchar_type<char, long long>       { typedef uchar type; };
template <> struct uchar_type<char, unsigned long long>           { typedef uchar type; };

template <> struct uchar_type<unsigned char, char>            { typedef uchar type; };
template <> struct uchar_type<unsigned char, unsigned char>   { typedef uchar type; };
template <> struct uchar_type<unsigned char, short>           { typedef uchar type; };
template <> struct uchar_type<unsigned char, unsigned short>  { typedef uchar type; };
template <> struct uchar_type<unsigned char, int>             { typedef uchar type; };
template <> struct uchar_type<unsigned char, unsigned int>    { typedef uchar type; };
template <> struct uchar_type<unsigned char, half>            { typedef uchar type; };
template <> struct uchar_type<unsigned char, float>           { typedef uchar type; };
template <> struct uchar_type<unsigned char, double>          { typedef uchar type; };
template <> struct uchar_type<unsigned char, long long>       { typedef uchar type; };
template <> struct uchar_type<unsigned char, unsigned long long>           { typedef uchar type; };

template <> struct uchar_type<short, char>            { typedef uchar type; };
template <> struct uchar_type<short, unsigned char>   { typedef uchar type; };
template <> struct uchar_type<short, short>           { typedef uchar type; };
template <> struct uchar_type<short, unsigned short>  { typedef uchar type; };
template <> struct uchar_type<short, int>             { typedef uchar type; };
template <> struct uchar_type<short, unsigned int>    { typedef uchar type; };
template <> struct uchar_type<short, half>            { typedef uchar type; };
template <> struct uchar_type<short, float>           { typedef uchar type; };
template <> struct uchar_type<short, double>          { typedef uchar type; };
template <> struct uchar_type<short, long long>       { typedef uchar type; };
template <> struct uchar_type<short, unsigned long long>           { typedef uchar type; };

template <> struct uchar_type<unsigned short, char>            { typedef uchar type; };
template <> struct uchar_type<unsigned short, unsigned char>   { typedef uchar type; };
template <> struct uchar_type<unsigned short, short>           { typedef uchar type; };
template <> struct uchar_type<unsigned short, unsigned short>  { typedef uchar type; };
template <> struct uchar_type<unsigned short, int>             { typedef uchar type; };
template <> struct uchar_type<unsigned short, unsigned int>    { typedef uchar type; };
template <> struct uchar_type<unsigned short, half>            { typedef uchar type; };
template <> struct uchar_type<unsigned short, float>           { typedef uchar type; };
template <> struct uchar_type<unsigned short, double>          { typedef uchar type; };
template <> struct uchar_type<unsigned short, long long>       { typedef uchar type; };
template <> struct uchar_type<unsigned short, unsigned long long>           { typedef uchar type; };

template <> struct uchar_type<int, char>            { typedef uchar type; };
template <> struct uchar_type<int, unsigned char>   { typedef uchar type; };
template <> struct uchar_type<int, short>           { typedef uchar type; };
template <> struct uchar_type<int, unsigned short>  { typedef uchar type; };
template <> struct uchar_type<int, int>             { typedef uchar type; };
template <> struct uchar_type<int, unsigned int>    { typedef uchar type; };
template <> struct uchar_type<int, half>            { typedef uchar type; };
template <> struct uchar_type<int, float>           { typedef uchar type; };
template <> struct uchar_type<int, double>          { typedef uchar type; };
template <> struct uchar_type<int, long long>       { typedef uchar type; };
template <> struct uchar_type<int, unsigned long long>           { typedef uchar type; };

template <> struct uchar_type<unsigned int, char>            { typedef uchar type; };
template <> struct uchar_type<unsigned int, unsigned char>   { typedef uchar type; };
template <> struct uchar_type<unsigned int, short>           { typedef uchar type; };
template <> struct uchar_type<unsigned int, unsigned short>  { typedef uchar type; };
template <> struct uchar_type<unsigned int, int>             { typedef uchar type; };
template <> struct uchar_type<unsigned int, unsigned int>    { typedef uchar type; };
template <> struct uchar_type<unsigned int, half>            { typedef uchar type; };
template <> struct uchar_type<unsigned int, float>           { typedef uchar type; };
template <> struct uchar_type<unsigned int, double>          { typedef uchar type; };
template <> struct uchar_type<unsigned int, long long>       { typedef uchar type; };
template <> struct uchar_type<unsigned int, unsigned long long>           { typedef uchar type; };

template <> struct uchar_type<half, char>                    { typedef uchar type; };
template <> struct uchar_type<half, unsigned char>           { typedef uchar type; };
template <> struct uchar_type<half, short>                   { typedef uchar type; };
template <> struct uchar_type<half, unsigned short>          { typedef uchar type; };
template <> struct uchar_type<half, int>                     { typedef uchar type; };
template <> struct uchar_type<half, unsigned int>            { typedef uchar type; };
template <> struct uchar_type<half, half>                    { typedef uchar type; };
template <> struct uchar_type<half, float>                   { typedef uchar type; };
template <> struct uchar_type<half, double>                  { typedef uchar type; };
template <> struct uchar_type<half, long long>               { typedef uchar type; };
template <> struct uchar_type<half, unsigned long long>                   { typedef uchar type; };

template <> struct uchar_type<float, char>            { typedef uchar type; };
template <> struct uchar_type<float, unsigned char>   { typedef uchar type; };
template <> struct uchar_type<float, short>           { typedef uchar type; };
template <> struct uchar_type<float, unsigned short>  { typedef uchar type; };
template <> struct uchar_type<float, int>             { typedef uchar type; };
template <> struct uchar_type<float, unsigned int>    { typedef uchar type; };
template <> struct uchar_type<float, half>            { typedef uchar type; };
template <> struct uchar_type<float, float>           { typedef uchar type; };
template <> struct uchar_type<float, double>          { typedef uchar type; };
template <> struct uchar_type<float, long long>       { typedef uchar type; };
template <> struct uchar_type<float, unsigned long long>           { typedef uchar type; };

template <> struct uchar_type<double, char>            { typedef uchar type; };
template <> struct uchar_type<double, unsigned char>   { typedef uchar type; };
template <> struct uchar_type<double, short>           { typedef uchar type; };
template <> struct uchar_type<double, unsigned short>  { typedef uchar type; };
template <> struct uchar_type<double, int>             { typedef uchar type; };
template <> struct uchar_type<double, unsigned int>    { typedef uchar type; };
template <> struct uchar_type<double, half>            { typedef uchar type; };
template <> struct uchar_type<double, float>           { typedef uchar type; };
template <> struct uchar_type<double, double>          { typedef uchar type; };
template <> struct uchar_type<double, long long>       { typedef uchar type; };
template <> struct uchar_type<double, unsigned long long>           { typedef uchar type; };

template <> struct uchar_type<long long, char>            { typedef uchar type; };
template <> struct uchar_type<long long, unsigned char>   { typedef uchar type; };
template <> struct uchar_type<long long, short>           { typedef uchar type; };
template <> struct uchar_type<long long, unsigned short>  { typedef uchar type; };
template <> struct uchar_type<long long, int>             { typedef uchar type; };
template <> struct uchar_type<long long, unsigned int>    { typedef uchar type; };
template <> struct uchar_type<long long, half>            { typedef uchar type; };
template <> struct uchar_type<long long, float>           { typedef uchar type; };
template <> struct uchar_type<long long, double>          { typedef uchar type; };
template <> struct uchar_type<long long, long long>       { typedef uchar type; };
template <> struct uchar_type<long long, unsigned long long>           { typedef uchar type; };

template <> struct uchar_type<unsigned long long, char>            { typedef uchar type; };
template <> struct uchar_type<unsigned long long, unsigned char>   { typedef uchar type; };
template <> struct uchar_type<unsigned long long, short>           { typedef uchar type; };
template <> struct uchar_type<unsigned long long, unsigned short>  { typedef uchar type; };
template <> struct uchar_type<unsigned long long, int>             { typedef uchar type; };
template <> struct uchar_type<unsigned long long, unsigned int>    { typedef uchar type; };
template <> struct uchar_type<unsigned long long, half>            { typedef uchar type; };
template <> struct uchar_type<unsigned long long, float>           { typedef uchar type; };
template <> struct uchar_type<unsigned long long, double>          { typedef uchar type; };
template <> struct uchar_type<unsigned long long, long long>       { typedef uchar type; };
template <> struct uchar_type<unsigned long long, unsigned long long>           { typedef uchar type; };

template <typename T1, typename T2> struct ushort_type {
private:
        ushort_type();
};
template <> struct ushort_type<char, char>            { typedef ushort type; };
template <> struct ushort_type<char, unsigned char>   { typedef ushort type; };
template <> struct ushort_type<char, short>           { typedef ushort type; };
template <> struct ushort_type<char, unsigned short>  { typedef ushort type; };
template <> struct ushort_type<char, int>             { typedef ushort type; };
template <> struct ushort_type<char, unsigned int>    { typedef ushort type; };
template <> struct ushort_type<char, half>            { typedef ushort type; };
template <> struct ushort_type<char, float>           { typedef ushort type; };
template <> struct ushort_type<char, double>          { typedef ushort type; };
template <> struct ushort_type<char, long long>       { typedef ushort type; };
template <> struct ushort_type<char, unsigned long long>           { typedef ushort type; };

template <> struct ushort_type<unsigned char, char>            { typedef ushort type; };
template <> struct ushort_type<unsigned char, unsigned char>   { typedef ushort type; };
template <> struct ushort_type<unsigned char, short>           { typedef ushort type; };
template <> struct ushort_type<unsigned char, unsigned short>  { typedef ushort type; };
template <> struct ushort_type<unsigned char, int>             { typedef ushort type; };
template <> struct ushort_type<unsigned char, unsigned int>    { typedef ushort type; };
template <> struct ushort_type<unsigned char, half>            { typedef ushort type; };
template <> struct ushort_type<unsigned char, float>           { typedef ushort type; };
template <> struct ushort_type<unsigned char, double>          { typedef ushort type; };
template <> struct ushort_type<unsigned char, long long>       { typedef ushort type; };
template <> struct ushort_type<unsigned char, unsigned long long>           { typedef ushort type; };

template <> struct ushort_type<short, char>            { typedef ushort type; };
template <> struct ushort_type<short, unsigned char>   { typedef ushort type; };
template <> struct ushort_type<short, short>           { typedef ushort type; };
template <> struct ushort_type<short, unsigned short>  { typedef ushort type; };
template <> struct ushort_type<short, int>             { typedef ushort type; };
template <> struct ushort_type<short, unsigned int>    { typedef ushort type; };
template <> struct ushort_type<short, half>            { typedef ushort type; };
template <> struct ushort_type<short, float>           { typedef ushort type; };
template <> struct ushort_type<short, double>          { typedef ushort type; };
template <> struct ushort_type<short, long long>       { typedef ushort type; };
template <> struct ushort_type<short, unsigned long long>           { typedef ushort type; };

template <> struct ushort_type<unsigned short, char>            { typedef ushort type; };
template <> struct ushort_type<unsigned short, unsigned char>   { typedef ushort type; };
template <> struct ushort_type<unsigned short, short>           { typedef ushort type; };
template <> struct ushort_type<unsigned short, unsigned short>  { typedef ushort type; };
template <> struct ushort_type<unsigned short, int>             { typedef ushort type; };
template <> struct ushort_type<unsigned short, unsigned int>    { typedef ushort type; };
template <> struct ushort_type<unsigned short, half>            { typedef ushort type; };
template <> struct ushort_type<unsigned short, float>           { typedef ushort type; };
template <> struct ushort_type<unsigned short, double>          { typedef ushort type; };
template <> struct ushort_type<unsigned short, long long>       { typedef ushort type; };
template <> struct ushort_type<unsigned short, unsigned long long>           { typedef ushort type; };

template <> struct ushort_type<int, char>            { typedef ushort type; };
template <> struct ushort_type<int, unsigned char>   { typedef ushort type; };
template <> struct ushort_type<int, short>           { typedef ushort type; };
template <> struct ushort_type<int, unsigned short>  { typedef ushort type; };
template <> struct ushort_type<int, int>             { typedef ushort type; };
template <> struct ushort_type<int, unsigned int>    { typedef ushort type; };
template <> struct ushort_type<int, half>            { typedef ushort type; };
template <> struct ushort_type<int, float>           { typedef ushort type; };
template <> struct ushort_type<int, double>          { typedef ushort type; };
template <> struct ushort_type<int, long long>       { typedef ushort type; };
template <> struct ushort_type<int, unsigned long long>           { typedef ushort type; };

template <> struct ushort_type<unsigned int, char>            { typedef ushort type; };
template <> struct ushort_type<unsigned int, unsigned char>   { typedef ushort type; };
template <> struct ushort_type<unsigned int, short>           { typedef ushort type; };
template <> struct ushort_type<unsigned int, unsigned short>  { typedef ushort type; };
template <> struct ushort_type<unsigned int, int>             { typedef ushort type; };
template <> struct ushort_type<unsigned int, unsigned int>    { typedef ushort type; };
template <> struct ushort_type<unsigned int, half>            { typedef ushort type; };
template <> struct ushort_type<unsigned int, float>           { typedef ushort type; };
template <> struct ushort_type<unsigned int, double>          { typedef ushort type; };
template <> struct ushort_type<unsigned int, long long>       { typedef ushort type; };
template <> struct ushort_type<unsigned int, unsigned long long>           { typedef ushort type; };

template <> struct ushort_type<half, char>                    { typedef ushort type; };
template <> struct ushort_type<half, unsigned char>           { typedef ushort type; };
template <> struct ushort_type<half, short>                   { typedef ushort type; };
template <> struct ushort_type<half, unsigned short>          { typedef ushort type; };
template <> struct ushort_type<half, int>                     { typedef ushort type; };
template <> struct ushort_type<half, unsigned int>            { typedef ushort type; };
template <> struct ushort_type<half, half>                    { typedef ushort type; };
template <> struct ushort_type<half, float>                   { typedef ushort type; };
template <> struct ushort_type<half, double>                  { typedef ushort type; };
template <> struct ushort_type<half, long long>               { typedef ushort type; };
template <> struct ushort_type<half, unsigned long long>                   { typedef ushort type; };

template <> struct ushort_type<float, char>            { typedef ushort type; };
template <> struct ushort_type<float, unsigned char>   { typedef ushort type; };
template <> struct ushort_type<float, short>           { typedef ushort type; };
template <> struct ushort_type<float, unsigned short>  { typedef ushort type; };
template <> struct ushort_type<float, int>             { typedef ushort type; };
template <> struct ushort_type<float, unsigned int>    { typedef ushort type; };
template <> struct ushort_type<float, half>            { typedef ushort type; };
template <> struct ushort_type<float, float>           { typedef ushort type; };
template <> struct ushort_type<float, double>          { typedef ushort type; };
template <> struct ushort_type<float, long long>       { typedef ushort type; };
template <> struct ushort_type<float, unsigned long long>           { typedef ushort type; };

template <> struct ushort_type<double, char>            { typedef ushort type; };
template <> struct ushort_type<double, unsigned char>   { typedef ushort type; };
template <> struct ushort_type<double, short>           { typedef ushort type; };
template <> struct ushort_type<double, unsigned short>  { typedef ushort type; };
template <> struct ushort_type<double, int>             { typedef ushort type; };
template <> struct ushort_type<double, unsigned int>    { typedef ushort type; };
template <> struct ushort_type<double, half>            { typedef ushort type; };
template <> struct ushort_type<double, float>           { typedef ushort type; };
template <> struct ushort_type<double, double>          { typedef ushort type; };
template <> struct ushort_type<double, long long>       { typedef ushort type; };
template <> struct ushort_type<double, unsigned long long>           { typedef ushort type; };

template <> struct ushort_type<long long, char>            { typedef ushort type; };
template <> struct ushort_type<long long, unsigned char>   { typedef ushort type; };
template <> struct ushort_type<long long, short>           { typedef ushort type; };
template <> struct ushort_type<long long, unsigned short>  { typedef ushort type; };
template <> struct ushort_type<long long, int>             { typedef ushort type; };
template <> struct ushort_type<long long, unsigned int>    { typedef ushort type; };
template <> struct ushort_type<long long, half>            { typedef ushort type; };
template <> struct ushort_type<long long, float>           { typedef ushort type; };
template <> struct ushort_type<long long, double>          { typedef ushort type; };
template <> struct ushort_type<long long, long long>       { typedef ushort type; };
template <> struct ushort_type<long long, unsigned long long>           { typedef ushort type; };

template <> struct ushort_type<unsigned long long, char>            { typedef ushort type; };
template <> struct ushort_type<unsigned long long, unsigned char>   { typedef ushort type; };
template <> struct ushort_type<unsigned long long, short>           { typedef ushort type; };
template <> struct ushort_type<unsigned long long, unsigned short>  { typedef ushort type; };
template <> struct ushort_type<unsigned long long, int>             { typedef ushort type; };
template <> struct ushort_type<unsigned long long, unsigned int>    { typedef ushort type; };
template <> struct ushort_type<unsigned long long, half>            { typedef ushort type; };
template <> struct ushort_type<unsigned long long, float>           { typedef ushort type; };
template <> struct ushort_type<unsigned long long, double>          { typedef ushort type; };
template <> struct ushort_type<unsigned long long, long long>       { typedef ushort type; };
template <> struct ushort_type<unsigned long long, unsigned long long>           { typedef ushort type; };

template <typename T1, typename T2> struct uint_type {
private:
        uint_type();
};
template <> struct uint_type<char, char>            { typedef uint type; };
template <> struct uint_type<char, unsigned char>   { typedef uint type; };
template <> struct uint_type<char, short>           { typedef uint type; };
template <> struct uint_type<char, unsigned short>  { typedef uint type; };
template <> struct uint_type<char, int>             { typedef uint type; };
template <> struct uint_type<char, unsigned int>    { typedef uint type; };
template <> struct uint_type<char, half>            { typedef uint type; };
template <> struct uint_type<char, float>           { typedef uint type; };
template <> struct uint_type<char, double>          { typedef uint type; };
template <> struct uint_type<char, long long>       { typedef uint type; };
template <> struct uint_type<char, unsigned long long>           { typedef uint type; };

template <> struct uint_type<unsigned char, char>            { typedef uint type; };
template <> struct uint_type<unsigned char, unsigned char>   { typedef uint type; };
template <> struct uint_type<unsigned char, short>           { typedef uint type; };
template <> struct uint_type<unsigned char, unsigned short>  { typedef uint type; };
template <> struct uint_type<unsigned char, int>             { typedef uint type; };
template <> struct uint_type<unsigned char, unsigned int>    { typedef uint type; };
template <> struct uint_type<unsigned char, half>            { typedef uint type; };
template <> struct uint_type<unsigned char, float>           { typedef uint type; };
template <> struct uint_type<unsigned char, double>          { typedef uint type; };
template <> struct uint_type<unsigned char, long long>       { typedef uint type; };
template <> struct uint_type<unsigned char, unsigned long long>           { typedef uint type; };

template <> struct uint_type<short, char>            { typedef uint type; };
template <> struct uint_type<short, unsigned char>   { typedef uint type; };
template <> struct uint_type<short, short>           { typedef uint type; };
template <> struct uint_type<short, unsigned short>  { typedef uint type; };
template <> struct uint_type<short, int>             { typedef uint type; };
template <> struct uint_type<short, unsigned int>    { typedef uint type; };
template <> struct uint_type<short, half>            { typedef uint type; };
template <> struct uint_type<short, float>           { typedef uint type; };
template <> struct uint_type<short, double>          { typedef uint type; };
template <> struct uint_type<short, long long>       { typedef uint type; };
template <> struct uint_type<short, unsigned long long>           { typedef uint type; };

template <> struct uint_type<unsigned short, char>            { typedef uint type; };
template <> struct uint_type<unsigned short, unsigned char>   { typedef uint type; };
template <> struct uint_type<unsigned short, short>           { typedef uint type; };
template <> struct uint_type<unsigned short, unsigned short>  { typedef uint type; };
template <> struct uint_type<unsigned short, int>             { typedef uint type; };
template <> struct uint_type<unsigned short, unsigned int>    { typedef uint type; };
template <> struct uint_type<unsigned short, half>            { typedef uint type; };
template <> struct uint_type<unsigned short, float>           { typedef uint type; };
template <> struct uint_type<unsigned short, double>          { typedef uint type; };
template <> struct uint_type<unsigned short, long long>       { typedef uint type; };
template <> struct uint_type<unsigned short, unsigned long long>           { typedef uint type; };

template <> struct uint_type<int, char>            { typedef uint type; };
template <> struct uint_type<int, unsigned char>   { typedef uint type; };
template <> struct uint_type<int, short>           { typedef uint type; };
template <> struct uint_type<int, unsigned short>  { typedef uint type; };
template <> struct uint_type<int, int>             { typedef uint type; };
template <> struct uint_type<int, unsigned int>    { typedef uint type; };
template <> struct uint_type<int, half>            { typedef uint type; };
template <> struct uint_type<int, float>           { typedef uint type; };
template <> struct uint_type<int, double>          { typedef uint type; };
template <> struct uint_type<int, long long>       { typedef uint type; };
template <> struct uint_type<int, unsigned long long>           { typedef uint type; };

template <> struct uint_type<unsigned int, char>            { typedef uint type; };
template <> struct uint_type<unsigned int, unsigned char>   { typedef uint type; };
template <> struct uint_type<unsigned int, short>           { typedef uint type; };
template <> struct uint_type<unsigned int, unsigned short>  { typedef uint type; };
template <> struct uint_type<unsigned int, int>             { typedef uint type; };
template <> struct uint_type<unsigned int, unsigned int>    { typedef uint type; };
template <> struct uint_type<unsigned int, half>            { typedef uint type; };
template <> struct uint_type<unsigned int, float>           { typedef uint type; };
template <> struct uint_type<unsigned int, double>          { typedef uint type; };
template <> struct uint_type<unsigned int, long long>       { typedef uint type; };
template <> struct uint_type<unsigned int, unsigned long long>           { typedef uint type; };

template <> struct uint_type<half, char>            { typedef uint type; };
template <> struct uint_type<half, unsigned char>   { typedef uint type; };
template <> struct uint_type<half, short>           { typedef uint type; };
template <> struct uint_type<half, unsigned short>  { typedef uint type; };
template <> struct uint_type<half, int>             { typedef uint type; };
template <> struct uint_type<half, unsigned int>    { typedef uint type; };
template <> struct uint_type<half, half>            { typedef uint type; };
template <> struct uint_type<half, float>           { typedef uint type; };
template <> struct uint_type<half, double>          { typedef uint type; };
template <> struct uint_type<half, long long>       { typedef uint type; };
template <> struct uint_type<half, unsigned long long>           { typedef uint type; };

template <> struct uint_type<float, char>            { typedef uint type; };
template <> struct uint_type<float, unsigned char>   { typedef uint type; };
template <> struct uint_type<float, short>           { typedef uint type; };
template <> struct uint_type<float, unsigned short>  { typedef uint type; };
template <> struct uint_type<float, int>             { typedef uint type; };
template <> struct uint_type<float, unsigned int>    { typedef uint type; };
template <> struct uint_type<float, half>            { typedef uint type; };
template <> struct uint_type<float, float>           { typedef uint type; };
template <> struct uint_type<float, double>          { typedef uint type; };
template <> struct uint_type<float, long long>       { typedef uint type; };
template <> struct uint_type<float, unsigned long long>           { typedef uint type; };

template <> struct uint_type<double, char>            { typedef uint type; };
template <> struct uint_type<double, unsigned char>   { typedef uint type; };
template <> struct uint_type<double, short>           { typedef uint type; };
template <> struct uint_type<double, unsigned short>  { typedef uint type; };
template <> struct uint_type<double, int>             { typedef uint type; };
template <> struct uint_type<double, unsigned int>    { typedef uint type; };
template <> struct uint_type<double, half>            { typedef uint type; };
template <> struct uint_type<double, float>           { typedef uint type; };
template <> struct uint_type<double, double>          { typedef uint type; };
template <> struct uint_type<double, long long>       { typedef uint type; };
template <> struct uint_type<double, unsigned long long>           { typedef uint type; };

template <> struct uint_type<long long, char>            { typedef uint type; };
template <> struct uint_type<long long, unsigned char>   { typedef uint type; };
template <> struct uint_type<long long, short>           { typedef uint type; };
template <> struct uint_type<long long, unsigned short>  { typedef uint type; };
template <> struct uint_type<long long, int>             { typedef uint type; };
template <> struct uint_type<long long, unsigned int>    { typedef uint type; };
template <> struct uint_type<long long, half>            { typedef uint type; };
template <> struct uint_type<long long, float>           { typedef uint type; };
template <> struct uint_type<long long, double>          { typedef uint type; };
template <> struct uint_type<long long, long long>       { typedef uint type; };
template <> struct uint_type<long long, unsigned long long>           { typedef uint type; };

template <> struct uint_type<unsigned long long, char>            { typedef uint type; };
template <> struct uint_type<unsigned long long, unsigned char>   { typedef uint type; };
template <> struct uint_type<unsigned long long, short>           { typedef uint type; };
template <> struct uint_type<unsigned long long, unsigned short>  { typedef uint type; };
template <> struct uint_type<unsigned long long, int>             { typedef uint type; };
template <> struct uint_type<unsigned long long, unsigned int>    { typedef uint type; };
template <> struct uint_type<unsigned long long, half>            { typedef uint type; };
template <> struct uint_type<unsigned long long, float>           { typedef uint type; };
template <> struct uint_type<unsigned long long, double>          { typedef uint type; };
template <> struct uint_type<unsigned long long, long long>       { typedef uint type; };
template <> struct uint_type<unsigned long long, unsigned long long>           { typedef uint type; };

template <typename T1> struct int_uint_type {
private:
        int_uint_type();
};
template <> struct int_uint_type<char>             { typedef int type; };
template <> struct int_uint_type<short>             { typedef int type; };
template <> struct int_uint_type<int>               { typedef int type; };
template <> struct int_uint_type<long long>               { typedef long long type; };
template <> struct int_uint_type<unsigned char>             { typedef unsigned int type; };
template <> struct int_uint_type<unsigned short>             { typedef unsigned int type; };
template <> struct int_uint_type<unsigned int>             { typedef unsigned int type; };
template <> struct int_uint_type<unsigned long long>             { typedef unsigned long long type; };

template <typename T1, typename T2> struct restype_sat {
private:
    restype_sat();
};

template <> struct restype_sat<char, char>            { typedef int  type; };
template <> struct restype_sat<char, unsigned char>   { typedef int  type; };
template <> struct restype_sat<char, short>           { typedef int type; };
template <> struct restype_sat<char, unsigned short>  { typedef int type; };
template <> struct restype_sat<char, int>             { typedef long long   type; };
template <> struct restype_sat<char, unsigned int>    { typedef long long   type; };
template <> struct restype_sat<char, half>           { typedef half type; };
template <> struct restype_sat<char, float>           { typedef float type; };
template <> struct restype_sat<char, double>          { typedef double type; };
template <> struct restype_sat<char, long long>       { typedef long long type; };
template <> struct restype_sat<char, unsigned long long>           { typedef long long type; };

template <> struct restype_sat<unsigned char, char>            { typedef int  type; };
template <> struct restype_sat<unsigned char, unsigned char>   { typedef int  type; };
template <> struct restype_sat<unsigned char, short>           { typedef int type; };
template <> struct restype_sat<unsigned char, unsigned short>  { typedef int type; };
template <> struct restype_sat<unsigned char, int>             { typedef long long   type; };
template <> struct restype_sat<unsigned char, unsigned int>    { typedef long long   type; };
template <> struct restype_sat<unsigned char, half>            { typedef half type; };
template <> struct restype_sat<unsigned char, float>           { typedef float type; };
template <> struct restype_sat<unsigned char, double>          { typedef double type; };
template <> struct restype_sat<unsigned char, long long>       { typedef long long type; };
template <> struct restype_sat<unsigned char, unsigned long long>           { typedef long long type; };

template <> struct restype_sat<short, char>            { typedef int type; };
template <> struct restype_sat<short, unsigned char>   { typedef int type; };
template <> struct restype_sat<short, short>           { typedef int type; };
template <> struct restype_sat<short, unsigned short>  { typedef int type; };
template <> struct restype_sat<short, int>             { typedef long long   type; };
template <> struct restype_sat<short, unsigned int>    { typedef long long   type; };
template <> struct restype_sat<short, half>            { typedef half type; };
template <> struct restype_sat<short, float>           { typedef float type; };
template <> struct restype_sat<short, double>          { typedef double type; };
template <> struct restype_sat<short, long long>       { typedef long long type; };
template <> struct restype_sat<short, unsigned long long>           { typedef long long type; };

template <> struct restype_sat<unsigned short, char>            { typedef int type; };
template <> struct restype_sat<unsigned short, unsigned char>   { typedef int type; };
template <> struct restype_sat<unsigned short, short>           { typedef int type; };
template <> struct restype_sat<unsigned short, unsigned short>  { typedef unsigned int type; };
template <> struct restype_sat<unsigned short, int>             { typedef long long type; };
template <> struct restype_sat<unsigned short, unsigned int>    { typedef long long type; };
template <> struct restype_sat<unsigned short, half>            { typedef half type; };
template <> struct restype_sat<unsigned short, float>           { typedef float type; };
template <> struct restype_sat<unsigned short, double>          { typedef double type; };
template <> struct restype_sat<unsigned short, long long>       { typedef long long type; };
template <> struct restype_sat<unsigned short, unsigned long long>           { typedef long long type; };

template <> struct restype_sat<int, char>            { typedef long long type; };
template <> struct restype_sat<int, unsigned char>   { typedef long long type; };
template <> struct restype_sat<int, short>           { typedef long long type; };
template <> struct restype_sat<int, unsigned short>  { typedef long long type; };
template <> struct restype_sat<int, int>             { typedef long long type; };
template <> struct restype_sat<int, unsigned int>    { typedef long long type; };
template <> struct restype_sat<int, float>           { typedef float type; };
template <> struct restype_sat<int, double>          { typedef double type; };
template <> struct restype_sat<int, long long>       { typedef long long type; };
template <> struct restype_sat<int, unsigned long long>           { typedef long long type; };

template <> struct restype_sat<unsigned int, char>            { typedef long long type; };
template <> struct restype_sat<unsigned int, unsigned char>   { typedef long long type; };
template <> struct restype_sat<unsigned int, short>           { typedef long long type; };
template <> struct restype_sat<unsigned int, unsigned short>  { typedef long long type; };
template <> struct restype_sat<unsigned int, int>             { typedef long long type; };
template <> struct restype_sat<unsigned int, unsigned int>    { typedef long long type; };
template <> struct restype_sat<unsigned int, float>           { typedef float type; };
template <> struct restype_sat<unsigned int, double>          { typedef double type; };
template <> struct restype_sat<unsigned int, long long>       { typedef long long type; };
template <> struct restype_sat<unsigned int, unsigned long long>           { typedef long long type; };

template <> struct restype_sat<half, char>            { typedef half type; };
template <> struct restype_sat<half, unsigned char>   { typedef half type; };
template <> struct restype_sat<half, short>           { typedef half type; };
template <> struct restype_sat<half, unsigned short>  { typedef half type; };
template <> struct restype_sat<half, half>            { typedef half type; };
template <> struct restype_sat<half, float>           { typedef float type; };
template <> struct restype_sat<half, double>          { typedef double type; };

template <> struct restype_sat<float, char>            { typedef float type; };
template <> struct restype_sat<float, unsigned char>   { typedef float type; };
template <> struct restype_sat<float, short>           { typedef float type; };
template <> struct restype_sat<float, unsigned short>  { typedef float type; };
template <> struct restype_sat<float, int>             { typedef float type; };
template <> struct restype_sat<float, unsigned int>    { typedef float type; };
template <> struct restype_sat<float, half>            { typedef float type; };
template <> struct restype_sat<float, float>           { typedef float type; };
template <> struct restype_sat<float, double>           { typedef double type; };

template <> struct restype_sat<double, char>            { typedef double type; };
template <> struct restype_sat<double, unsigned char>   { typedef double type; };
template <> struct restype_sat<double, short>           { typedef double type; };
template <> struct restype_sat<double, unsigned short>  { typedef double type; };
template <> struct restype_sat<double, int>             { typedef double type; };
template <> struct restype_sat<double, unsigned int>    { typedef double type; };
template <> struct restype_sat<double, half>            { typedef double type; };
template <> struct restype_sat<double, float>           { typedef double type; };
template <> struct restype_sat<double, double>          { typedef double type; };
template <> struct restype_sat<double, long long>       { typedef double type; };
template <> struct restype_sat<double, unsigned long long>           { typedef double type; };

template <typename T> struct abstype;
template<> struct abstype<half> { typedef half type; };
template<> struct abstype<float> { typedef float type; };
template<> struct abstype<char> { typedef uchar type; };
template<> struct abstype<short> { typedef ushort type; };
template<> struct abstype<int> { typedef int type; };
template<> struct abstype<uchar> { typedef uchar type; };
template<> struct abstype<ushort> { typedef ushort type; };
template<> struct abstype<uint> { typedef uint type; };
template<> struct abstype<double> { typedef double type; };
template<> struct abstype<long long> { typedef unsigned long long type; };
template<> struct abstype<unsigned long long> { typedef unsigned long long type; };

template <typename T>
struct to_int {
    typedef T Int;
};
template<> struct to_int<half> { typedef int Int; };
template<> struct to_int<float> { typedef int Int; };
template<> struct to_int<double> { typedef int Int; };

template <bool VALUE> struct check_true{
    static const bool value = false;
};
template <> struct check_true<true> {
    static const bool value = true;
};

template <typename T> struct inttype;
template <> struct inttype<char> {
    static const bool value = true;
};
template <> struct inttype<unsigned char> {
    static const bool value = true;
};
template <> struct inttype<short> {
    static const bool value = true;
};
template <> struct inttype<unsigned short> {
    static const bool value = true;
};
template <> struct inttype<int> {
    static const bool value = true;
};
template <> struct inttype<unsigned int> {
    static const bool value = true;
};
template <> struct inttype<long long> {
    static const bool value = true;
};
template <> struct inttype<unsigned long long> {
    static const bool value = true;
};

template <typename T> struct is_inttype {
    static const bool value = false;
};
template <> struct is_inttype<char> {
    static const bool value = true;
};
template <> struct is_inttype<unsigned char> {
    static const bool value = true;
};
template <> struct is_inttype<short> {
    static const bool value = true;
};
template <> struct is_inttype<unsigned short> {
    static const bool value = true;
};
template <> struct is_inttype<int> {
    static const bool value = true;
};
template <> struct is_inttype<unsigned int> {
    static const bool value = true;
};
template <> struct is_inttype<long long> {
    static const bool value = true;
};
template <> struct is_inttype<unsigned long long> {
    static const bool value = true;
};

template <typename T> struct is_byte_type {
    static const bool value = false;
};
template <> struct is_byte_type<char> {
    static const bool value = true;
};
template <> struct is_byte_type<uchar> {
    static const bool value = true;
};

template <typename T> struct is_word_type {
    static const bool value = false;
};
template <> struct is_word_type<short> {
    static const bool value = true;
};
template <> struct is_word_type<ushort> {
    static const bool value = true;
};

template <typename T> struct is_dword_type {
    static const bool value = false;
};
template <> struct is_dword_type<int> {
    static const bool value = true;
};
template <> struct is_dword_type<uint> {
    static const bool value = true;
};

template <typename T> struct is_qf_type {
    static const bool value = false;
};
template <> struct is_qf_type<uchar> {
    static const bool value = true;
};

template <typename T> struct is_hf_type {
    static const bool value = false;
};
template <> struct is_hf_type<half> {
    static const bool value = true;
};

template <typename T> struct is_fp_type {
    static const bool value = false;
};
template <> struct is_fp_type<float> {
    static const bool value = true;
};

template <typename T> struct is_df_type {
    static const bool value = false;
};
template <> struct is_df_type<double> {
    static const bool value = true;
};

template <typename T> struct is_fp_or_dword_type {
    static const bool value = false;
};
template <> struct is_fp_or_dword_type<int> {
    static const bool value = true;
};
template <> struct is_fp_or_dword_type<uint> {
    static const bool value = true;
};
template <> struct is_fp_or_dword_type<float> {
    static const bool value = true;
};
// The check is only used for dataport APIs,
// which also support df data type.
template <> struct is_fp_or_dword_type<double> {
    static const bool value = true;
};

template <typename T> struct is_ushort_type {
    static const bool value = false;
};
template <> struct is_ushort_type<ushort> {
    static const bool value = true;
};

template <typename T1, typename T2> struct is_float_dword {
    static const bool value = false;
};
template <> struct is_float_dword<float, int> {
    static const bool value = true;
};
template <> struct is_float_dword<float, uint> {
    static const bool value = true;
};
template <> struct is_float_dword<int, float> {
    static const bool value = true;
};
template <> struct is_float_dword<uint, float> {
    static const bool value = true;
};

template <typename T> struct hftype {
    static const bool value = false;
};
template <> struct hftype<half> {
    static const bool value = true;
};

template <typename T> struct fptype {
    static const bool value = false;
};
template <> struct fptype<float> {
    static const bool value = true;
};

template <typename T> struct dftype {
    static const bool value = false;
};
template <> struct dftype<double> {
    static const bool value = true;
};

template <typename T> struct cmtype;
template <> struct cmtype<char> {
    static const bool value = true;
};

template <> struct cmtype<signed char> {
    static const bool value = true;
};

template <> struct cmtype<unsigned char> {
    static const bool value = true;
};

template <> struct cmtype<short> {
    static const bool value = true;
};

template <> struct cmtype<unsigned short> {
    static const bool value = true;
};
template <> struct cmtype<int> {
    static const bool value = true;
};

template <> struct cmtype<unsigned int> {
    static const bool value = true;
};

template <> struct cmtype<unsigned long> {
    static const bool value = true;
};

template <> struct cmtype<half> {
    static const bool value = true;
};

template <> struct cmtype<float> {
    static const bool value = true;
};

template <> struct cmtype<double> {
    static const bool value = true;
};

template <> struct cmtype<long long> {
    static const bool value = true;
};

template <> struct cmtype<unsigned long long> {
    static const bool value = true;
};

template <> struct cmtype<SurfaceIndex> {
    static const bool value = true;
};

template<typename T> struct bytetype;
template<> struct bytetype<char> {
    static const bool value = true;
};
template<> struct bytetype<uchar> {
    static const bool value = true;
};

template<typename T> struct wordtype;
template<> struct wordtype<short> {
    static const bool value = true;
};
template<> struct wordtype<ushort> {
    static const bool value = true;
};

template<typename T> struct dwordtype;
template<> struct dwordtype<int> {
    static const bool value = true;
};
template<> struct dwordtype<uint> {
    static const bool value = true;
};

template<typename T> struct unsignedtype{
    static const bool value = false;
};
template<> struct unsignedtype<uint> {
    static const bool value = true;
};
template<> struct unsignedtype<ushort> {
    static const bool value = true;
};
template<> struct unsignedtype<uchar> {
    static const bool value = true;
};
template<> struct unsignedtype<unsigned long long> {
    static const bool value = true;
};

template<typename T> struct uinttype;
template<> struct uinttype<uint> {
    static const bool value = true;
};

template <uint N1, uint N2> struct ressize {
    static const uint size = (N1 > N2)?N1:N2;
    static const bool conformable = check_true<N1%size == 0 && N2%size == 0>::value;
};

#endif /* CM_DEF_H */
