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

//==-------------- half_type.hpp --- SYCL half type ------------------------==

// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

//===----------------------------------------------------------------------===

// Imported from
// - https://github.com/intel/llvm/blob/sycl/sycl/include/CL/sycl/half_type.hpp
// - https://github.com/intel/llvm/blob/sycl/sycl/source/half_type.cpp

#pragma once

#include <cmath>
#include <cstdint>
#include <cstring>
#include <functional>
#include <iostream>
#include <limits>

#ifdef min
#undef min
#undef max
#endif // min

#define HALF_SUPPORT_INTERFACE

#ifndef __builtin_expect
#define __builtin_expect(X, Y) (X)
#endif // __builtin_expect

namespace hfimpl {

static uint16_t float2Half(const float &Val) {
  const uint32_t Bits = *reinterpret_cast<const uint32_t *>(&Val);

  // Extract the sign from the float value
  const uint16_t Sign = (Bits & 0x80000000) >> 16;
  // Extract the fraction from the float value
  const uint32_t Frac32 = Bits & 0x7fffff;
  // Extract the exponent from the float value
  const uint8_t Exp32 = (Bits & 0x7f800000) >> 23;

  uint16_t Exp16 = 0;
  uint16_t shift = 10;

  // convert 23-bit mantissa to 10-bit mantissa.
  uint16_t Frac16 = (uint16_t)(Frac32 >> 13);

  if (Exp32 == 0xFF)
  { // Infinity or NaN
    Exp16 = 0x1f;
  }
  else if (Exp32 != 0)
  {
    const int8_t Exp32Diff = (int8_t)Exp32 - 127;
    if (Exp32Diff < -24)
    {
      // Very small numbers maps to zero
      Frac16 = 0;
    }
    else if (Exp32Diff < -14)
    {
      // Small 32-bit FP numbers map to denorms/subnormal of 16-bit FP
      Exp16 = (uint16_t)0x0400 >> (-(Exp32Diff + 14));
      Frac16 = (uint16_t)(Frac32 >> (-(Exp32Diff + 1)));
      shift = 0;
    }
    else if (Exp32Diff <= 15)
    {
      // Number numbers just lose precision
      Exp16 = Exp32Diff + 15;
    }
    else // 15 < Exp32Diff <=127
    {
      // Large 32-bit FP numbers map to infinity in 16-bit FP
      Exp16 = 0x1f;
      Frac16 = 0;
    }
  }
  /* else
     {
     // Exp32 == 0 -> Zero or subnormal of 32-bit FP
     // -> Frac16 is used as it is. Exp16 is default-zero.
     }*/

  // Compose the final FP16 binary
  uint16_t Ret = 0;
  Ret |= Sign;
  Ret |= Exp16 << shift;
  Ret |= Frac16;

  return Ret;

}
static float half2Float(const uint16_t &Val) {
  // Extract the sign from the bits
  const uint32_t Sign = static_cast<uint32_t>(Val & 0x8000) << 16;
  // Extract the exponent from the bits
  const uint8_t Exp16 = (Val & 0x7c00) >> 10;
  // Extract the fraction from the bits
  uint16_t Frac16 = Val & 0x3ff;

  uint32_t Exp32 = 0;
  if (__builtin_expect(Exp16 == 0x1f, 0)) {
    Exp32 = 0xff;
  } else if (__builtin_expect(Exp16 == 0, 0)) {
    Exp32 = 0;
  } else {
    Exp32 = static_cast<uint32_t>(Exp16) + 112;
  }

  // corner case: subnormal -> normal
  // The denormal number of FP16 can be represented by FP32, therefore we need
  // to recover the exponent and recalculate the fration.
  if (__builtin_expect(Exp16 == 0 && Frac16 != 0, 0)) {
    uint8_t OffSet = 0;
    do {
      ++OffSet;
      Frac16 <<= 1;
    } while ((Frac16 & 0x400) != 0x400);
    // mask the 9th bit
    Frac16 &= 0x3ff;
    Exp32 = 113 - OffSet;
  }

  uint32_t Frac32 = Frac16 << 13;

  // Compose the final FP32 binary
  uint32_t Bits = 0;

  Bits |= Sign;
  Bits |= (Exp32 << 23);
  Bits |= Frac32;

  float Result;
  std::memcpy(&Result, &Bits, sizeof(Result));
  return Result;
}

} // hfimpl

namespace half_float {

class HALF_SUPPORT_INTERFACE half {
public:
  half() = default;
  half(const half &) = default;
  half(half &&) = default;

  half(const float &rhs) : Buf(hfimpl::float2Half(rhs)) {};

  half &operator=(const half &rhs) = default;

  // Operator +=, -=, *=, /=
  half &operator+=(const half &rhs)
  {
    *this = operator float() + static_cast<float>(rhs);
    return *this;
  }

  half &operator-=(const half &rhs)
  {
    *this = operator float() - static_cast<float>(rhs);
    return *this;
  }

  half &operator*=(const half &rhs)
  {
    *this = operator float() * static_cast<float>(rhs);
    return *this;
  }

  half &operator/=(const half &rhs)
  {
    *this = operator float() / static_cast<float>(rhs);
    return *this;
  }

  // Operator ++, --
  half &operator++() {
    *this += 1;
    return *this;
  }

  half operator++(int) {
    half ret(*this);
    operator++();
    return ret;
  }

  half &operator--() {
    *this -= 1;
    return *this;
  }

  half operator--(int) {
    half ret(*this);
    operator--();
    return ret;
  }

  // Operator float
  operator float() const { return hfimpl::half2Float(Buf); }

  template <typename Key> friend struct std::hash;

private:
  uint16_t Buf;
};
} // namespace half_float

// Accroding to C++ standard math functions from cmath/math.h should work only
// on arithmetic types. We can't specify half type as arithmetic/floating
// point(via std::is_floating_point) since only float, double and long double
// types are "floating point" according to the standard. In order to use half
// type with these math functions we cast half to float using template
// function helper.
template <typename T> inline T cast_if_host_half(T val) { return val; }

inline float cast_if_host_half(half_float::half val) {
  return static_cast<float>(val);
}

#ifdef __SYCL_DEVICE_ONLY__
using half = _Float16;
#else
using half = half_float::half;
#endif

// Partial specialization of some functions in namespace `std`
namespace std {

#ifdef __SYCL_DEVICE_ONLY__
// `constexpr` could work because the implicit conversion from `float` to
// `_Float16` can be `constexpr`.
#define CONSTEXPR_QUALIFIER constexpr
#else
// The qualifier is `const` instead of `constexpr` that is original to be
// because the constructor is not `constexpr` function.
#define CONSTEXPR_QUALIFIER const
#endif

// Partial specialization of `std::hash<cl::sycl::half>`
template <> struct hash<half> {
  size_t operator()(half const &Key) const noexcept {
    return hash<uint16_t>{}(reinterpret_cast<const uint16_t &>(Key));
  }
};

// Partial specialization of `std::numeric<cl::sycl::half>`

// All following values are either calculated based on description of each
// function/value on https://en.cppreference.com/w/cpp/types/numeric_limits, or
// cl_platform.h.
#define SYCL_HLF_MIN 6.103515625e-05F

#define SYCL_HLF_MAX 65504.0F

#define SYCL_HLF_MAX_10_EXP 4

#define SYCL_HLF_MAX_EXP 16

#define SYCL_HLF_MIN_10_EXP -4

#define SYCL_HLF_MIN_EXP -13

#define SYCL_HLF_MANT_DIG 11

#define SYCL_HLF_DIG 3

#define SYCL_HLF_DECIMAL_DIG 5

#define SYCL_HLF_EPSILON 9.765625e-04F

#define SYCL_HLF_RADIX 2

template <> struct numeric_limits<half> {
  static constexpr const bool is_specialized = true;

  static constexpr const bool is_signed = true;

  static constexpr const bool is_integer = false;

  static constexpr const bool is_exact = false;

  static constexpr const bool has_infinity = true;

  static constexpr const bool has_quiet_NaN = true;

  static constexpr const bool has_signaling_NaN = true;

  static constexpr const float_denorm_style has_denorm = denorm_present;

  static constexpr const bool has_denorm_loss = false;

  static constexpr const bool tinyness_before = false;

  static constexpr const bool traps = false;

  static constexpr const int max_exponent10 = SYCL_HLF_MAX_10_EXP;

  static constexpr const int max_exponent = SYCL_HLF_MAX_EXP;

  static constexpr const int min_exponent10 = SYCL_HLF_MIN_10_EXP;

  static constexpr const int min_exponent = SYCL_HLF_MIN_EXP;

  static constexpr const int radix = SYCL_HLF_RADIX;

  static constexpr const int max_digits10 = SYCL_HLF_DECIMAL_DIG;

  static constexpr const int digits = SYCL_HLF_MANT_DIG;

  static constexpr const bool is_bounded = true;

  static constexpr const int digits10 = SYCL_HLF_DIG;

  static constexpr const bool is_modulo = false;

  static constexpr const bool is_iec559 = true;

  static constexpr const float_round_style round_style = round_to_nearest;

  static CONSTEXPR_QUALIFIER half min() noexcept { return SYCL_HLF_MIN; }

  static CONSTEXPR_QUALIFIER half max() noexcept { return SYCL_HLF_MAX; }

  static CONSTEXPR_QUALIFIER half lowest() noexcept { return -SYCL_HLF_MAX; }

  static CONSTEXPR_QUALIFIER half epsilon() noexcept {
    return SYCL_HLF_EPSILON;
  }

  static CONSTEXPR_QUALIFIER half round_error() noexcept { return 0.5F; }

  static CONSTEXPR_QUALIFIER half infinity() noexcept {
    return __builtin_huge_valf();
  }

  static CONSTEXPR_QUALIFIER half quiet_NaN() noexcept {
    return __builtin_nanf("");
  }

  static CONSTEXPR_QUALIFIER half signaling_NaN() noexcept {
    return __builtin_nansf("");
  }

  static CONSTEXPR_QUALIFIER half denorm_min() noexcept { return 5.96046e-08F; }
};

#undef CONSTEXPR_QUALIFIER

} // namespace std

inline std::ostream &operator<<(std::ostream &O, half const &rhs) {
  O << static_cast<float>(rhs);
  return O;
}

inline std::istream &operator>>(std::istream &I, half &rhs) {
  float ValFloat = 0.0f;
  I >> ValFloat;
  rhs = ValFloat;
  return I;
}
