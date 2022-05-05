/*========================== begin_copyright_notice ============================

Copyright (C) 2019 Intel Corporation

SPDX-License-Identifier: MIT

============================= end_copyright_notice ===========================*/

#ifndef _CM_LSC_H_
#define _CM_LSC_H_

#include <mutex>
#include "libcm_common.h"
#include "cm_internal.h"
#include "cm_intrin.h"
#include "genx_dataport.h"

#define SLM_SURFACE_IDX SurfaceIndex(INT_MAX)

/// Implementations enabled by following compilation qualifier must be
/// removed after VectorSize template argument is no longer used in
/// kernels
#define VECTORSIZE_NELEMENTS_SUPPORT

static std::mutex atomicMutex;

enum class EmuBufferType : short {
  UGM = 0,
  SLM = 1,
  UGML = 2,
  TGM = 3
};

/* Definitions imported from cm_lsc.h in CM compiler project : Begin */

// L1 or L3 cache hint kinds.
enum class CacheHint : uint8_t {
  Default = 0,
  Uncached = 1,
  Cached = 2,
  WriteBack = 3,
  WriteThrough = 4,
  Streaming = 5,
  ReadInvalidate = 6
};

// Data size or format to read or store.
enum class DataSize : uint8_t {
  Default = 0,
  U8 = 1,
  U16 = 2,
  U32 = 3,
  U64 = 4,
  U8U32 = 5,  // load 8b, zero extend to 32b; store the opposite
  U16U32 = 6, // load 8b, zero extend to 32b; store the opposite
  U16U32H = 7 // load 16b into high 16 of each 32b; store the high 16
};

// The number of elements to load per address (vector size)
enum class VectorSize : uint8_t {
  N0 = 0,
  N1 = 1,  // 1 element
  N2 = 2,  // 2 element
  N3 = 3,  // 3 element
  N4 = 4,  // 4 element
  N8 = 5,  // 8 element
  N16 = 6, // 16 element
  N32 = 7, // 32 element
  N64 = 8  // 64 element
};

// LSC atomic op kind and encoding.
enum class AtomicOp : uint8_t {
  //               nsrcs
  IINC = 0x08,  // 0
  IDEC = 0x09,  // 0
  LOAD = 0x0A,  // 0
  STORE = 0x0B, // 1
  IADD = 0x0C,  // 1
  ISUB = 0x0D,  // 1
  SMIN = 0x0E,  // 1
  SMAX = 0x0F,  // 1
  UMIN = 0x10,  // 1
  UMAX = 0x11,  // 1
  ICAS = 0x12,  // 2
  FADD = 0x13,  // 1
  FSUB = 0x14,  // 1
  FMIN = 0x15,  // 1
  FMAX = 0x16,  // 1
  FCAS = 0x17,  // 2
  AND = 0x18,   // 1
  OR = 0x19,    // 1
  XOR = 0x1A    // 1
};

namespace details {

static inline constexpr bool isPowerOf2(unsigned n)
{
  return (n & (n - 1)) == 0;
}

template <VectorSize VS> constexpr unsigned lsc_vector_size() {
  static_assert( static_cast<unsigned>(VS) < 9, "Unsupported Vector Size!!");
  constexpr unsigned NElts[] = {0, 1, 2, 3, 4, 8, 16, 32, 64};
  return NElts[static_cast<unsigned>(VS)];
}

template <unsigned N> constexpr VectorSize lsc_vector_size_enum() {
  if constexpr (N == 1)       return VectorSize::N1;
  else if constexpr (N == 2)  return VectorSize::N2;
  else if constexpr (N == 3)  return VectorSize::N3;
  else if constexpr (N == 4)  return VectorSize::N4;
  else if constexpr (N == 8)  return VectorSize::N8;
  else if constexpr (N == 16) return VectorSize::N16;
  else if constexpr (N == 32) return VectorSize::N32;
  else if constexpr (N == 64) return VectorSize::N64;
  else
    static_assert(N != N,"Unsupported Vector size for enumeration!!");
}

template <typename T, DataSize DS> constexpr DataSize lsc_data_size() {
  if constexpr (DS != DataSize::Default)
    return DS;
  else if constexpr (sizeof(T) == 1)
    return DataSize::U8;
  else if constexpr (sizeof(T) == 2)
    return DataSize::U16;
  else if constexpr (sizeof(T) == 4)
    return DataSize::U32;
  else if constexpr (sizeof(T) == 8)
    return DataSize::U64;
  else if constexpr (DS == DataSize::Default)
    static_assert(DS != DataSize::Default, "unsupported data type");
  return DS;
}

template <typename T, int N, VectorSize VS>
auto lsc_data_type() {
  constexpr unsigned NumElts = lsc_vector_size<VS>() * N;
  static_assert(NumElts > 0, "unexpected number of elements");
  return vector<T, NumElts>();
}

template <AtomicOp Op> constexpr int lsc_atomic_nsrcs() {
  switch (Op) {
  case AtomicOp::IINC:
  case AtomicOp::IDEC:
  case AtomicOp::LOAD:
    return 0;
  case AtomicOp::STORE:
  case AtomicOp::IADD:
  case AtomicOp::ISUB:
  case AtomicOp::SMIN:
  case AtomicOp::SMAX:
  case AtomicOp::UMIN:
  case AtomicOp::UMAX:
  case AtomicOp::FSUB:
  case AtomicOp::FMIN:
  case AtomicOp::FMAX:
  case AtomicOp::FADD:
  case AtomicOp::AND:
  case AtomicOp::OR:
  case AtomicOp::XOR:
    return 1;
  case AtomicOp::ICAS:
  case AtomicOp::FCAS:
    return 2;
  default:
    break;
  }
  return 0;
}

// Return the default SIMT width.
template <typename T = void> constexpr int lsc_default_simt() {
#if defined(CM_XEHPG)
  return 16; // DG2, SIMD16
#elif defined(CM_XEHPC)
  return 32; // PVC+, SIMD32
#else
  // static_assert(false, "Target Platform definition is missing - CM_XEHPG/7");
  return 0;
#endif
}

// SIMD/SIMT width sanity check
template <int N>
constexpr bool lsc_check_simt()
{
#if defined(CM_XEHPG)
  return ((N == 16) || (N == 8)); // DG2, SIMD8/16
#elif defined(CM_XEHPC)
  return ((N == 32) || (N == 16)); // PVC+, SIMD16/32
#else
  // static_assert(false, "Target Platform definition is missing - CM_XEHPG/7");
  return false;
#endif
}

// Load/Store align bitmask generator for 1-D vector load/store
//
// Not only generates address-align bitmask, but also checks
// legitimacy of load/store operation with respect to vector size,
// data size, and SIMT
template <typename T, VectorSize VS, DataSize DS, int N>
constexpr unsigned loadstoreAlignMask()
{
  constexpr DataSize _DS = details::lsc_data_size<T, DS>(); // Actual DataSize

#define _ASSERT_UNSUPPORTED_LOADSTORE_
#ifdef _ASSERT_UNSUPPORTED_LOADSTORE_
  // Transpose = Off
  // Data Size: D32, D64 only for vectorSize=1 with alignment=byte
  // For UGM Load/Store and prefetch, SIMT=32 only.
  //static_assert((_DS == DataSize::U32) || (_DS == DataSize::U64),
  //              "Wrong DataSize - Other than U32 and U64 is not supported yet");
#endif //_ASSERT_UNSUPPORTED_LOADSTORE_

  if constexpr (VS == VectorSize::N1)
  {
    static_assert((_DS == DataSize::U32) || (_DS == DataSize::U64)
                  || (_DS == DataSize::U8) || (_DS == DataSize::U16) // vmit-8489
                  || (_DS == DataSize::U8U32) || (_DS == DataSize::U16U32),
                  "Wrong DataSize for VectorSize == 1\n"
                  "(loadstoreAlignMask)");
    return 0x0;
  }
  else if constexpr ((VS == VectorSize::N2)
           || (VS == VectorSize::N3)
           || (VS == VectorSize::N4)
           || (VS == VectorSize::N8))
  {
//    static_assert(((_DS == DataSize::U32) || (_DS == DataSize::U64)) &&
    static_assert((_DS == DataSize::U32) || (_DS == DataSize::U64),
           "Wrong Data Size for VectorSize == 2/3/4/8\n" \
           "(loadstoreAlignMask)");
    // 0x3 for U32 / 0x7 for U64
    if constexpr (_DS == DataSize::U32) return 0x3;
    else return 0x7;
  }
  else if constexpr ((VS == VectorSize::N16)
                     || (VS == VectorSize::N32)
                     || (VS == VectorSize::N64))
  {
    static_assert((N == 1),
                  "Unsupported SIMT Size for VectorSize = 16/32/64\n"
                  "(loadstoreAlignMask)");
    // 0x3 for U32 / 0x7 for U64
    if constexpr (_DS == DataSize::U32) return 0x3;
    else return 0x7;
  }
  else
  {
    static_assert(N != N, "Wrong Vector Size!!");
  }
}

template <AtomicOp Op, typename T,
          VectorSize VS, DataSize DS,
          int N, EmuBufferType BFT>
constexpr unsigned atomicAlignMask()
{
  constexpr DataSize _DS = details::lsc_data_size<T, DS>(); // Actual DataSize

#define _ASSERT_UNSUPPORTED_ATOMIC_
#ifdef _ASSERT_UNSUPPORTED_ATOMIC_
  static_assert((_DS == DataSize::U16) || (_DS == DataSize::U32) || (_DS == DataSize::U64),
                "Unsupported DataSize - Other than U16/U32/U64 is not supported yet for atomic operations");
  static_assert((BFT == EmuBufferType::UGM) || (BFT == EmuBufferType::SLM),
                "Unsupported Buffer type - Only UGM and SLM are supported for atomic operations");
#endif // _ASSERT_UNSUPPORTED_ATOMIC_

  static_assert(VS == VectorSize::N1,
                "Atomic operation is allowed only for vector size == 1!! (VS == VectorSize::N1)");

  static_assert(details::isPowerOf2(N) && N <= 32,
                "For atomic operations, SIMT must be power of 2 and not greater than 32!!");

  if constexpr ((Op == AtomicOp::FADD)
                || (Op == AtomicOp::FSUB))
  {
    static_assert(BFT != EmuBufferType::SLM,
                  "Atomic FADD/FSUB is not allowed for SLM!!");
    static_assert((_DS == DataSize::U64) || (_DS == DataSize::U32),
                  "For Atomic FADD/FSUB, DataSize must be D32/D64!! (DS == DataSize::U32/U64)");
  }
  else
  {
    if constexpr (BFT == EmuBufferType::SLM)
    {
      if constexpr (Op == AtomicOp::ICAS)
      {
        static_assert((_DS == DataSize::U16U32) || (_DS == DataSize::U32) || (_DS == DataSize::U64),
                      "For Atomic ICAS/CMPXCHG operation on SLM, data size must be D16U32/D32/D64 (DS == DataSize::U16U32/U32/U64)");
      }
      else
      {
        static_assert((_DS == DataSize::U16U32) || (_DS == DataSize::U32),
                      "For Atomic operation on SLM, data size must be D16U32/D32 (DS == DataSize::U16U32/U32)");
      }
    }
    else if constexpr ((BFT == EmuBufferType::UGM) || (BFT == EmuBufferType::UGML))
    {
      static_assert((_DS == DataSize::U16U32)
                    || (_DS == DataSize::U32)
                    || (_DS == DataSize::U64),
                    "For Atomic operation on UGM/UGML, data size must be D16U32/D32/D64 (DS == DataSize::U16U32/U32/U64)");
    }
    else // TGM
    {
      static_assert((_DS == DataSize::U16)
                    || (_DS == DataSize::U32)
                    || (_DS == DataSize::U64),
                    "For Atomic operation on TGM, data size must be D16/D32/D64 (DS == DataSize::U16/U32/U64)");
    }
  }

  if constexpr (_DS == DataSize::U16) return 0x1;
  else if constexpr (_DS == DataSize::U32) return 0x3;
  else if constexpr (_DS == DataSize::U64) return 0x7;
  else
    static_assert(DS != DS, "Atomic operation not supported - U16U32");
}

enum class msgField : short
{
  OP,VNNI, ADDRSIZE, DATASIZE, VECTSIZE, TRANSPOSE, CACHE, DSTLEN, SRC0LEN, ADDRTYPE
};

enum class msgOp : short
{
  DP_LOAD = 0x0, //scatter/vector load
  LOAD_2D = 0x3,
  DP_STORE = 0x4, // scatter/vector store
  STORE_2D = 0x7,
  OP_MAX = 0x3F
};

typedef struct _bitfields_
{
  uint32_t offset;
  uint32_t mask;
} bitfields;

const bitfields BIT_FIELDS[10] =
{
  {0,  0x3F}, // OP / 6 bits
  {7,  0x1}, // VNNI -> LOAD only
  {7,  0x3}, // Address size
  {9,  0x7}, // DATASIZE
  {12, 0x7}, // VECTSIZE
  {15, 0x1}, // TRANSPOSE -> LOAD only
  {17, 0x7}, // CACHE
  {20, 0x1F}, // DSTLEN
  {25, 0xF}, // SRC0LEN,
  {29, 0x3}  // ADDRTYPE
};

uint32_t inline getMsgField(uint32_t msg, msgField field)
{
  uint32_t idx = static_cast<uint32_t>(field);
  return ((msg >> details::BIT_FIELDS[idx].offset) & details::BIT_FIELDS[idx].mask);
}

auto inline getMsgOp(uint32_t msg)
{
  details::msgOp ret;
  ret = static_cast<details::msgOp>(details::getMsgField((uint32_t)msg,
                                                         details::msgField::OP));
  return ret;
}

template<typename T, uint R, uint C,
         template<typename ElmTy, uint U, uint V> typename MatTy>
uint64_t inline getSurfaceBaseAddr(MatTy<T, R, C> addrMsg)
{
  return addrMsg.template format<uint64_t>()(0);
}

template<typename T, uint R, uint C,
         template<typename ElmTy, uint U, uint V> typename MatTy>
uint64_t inline getLaneAddr(MatTy<T, R, C> addrMsg, unsigned lane_id)
{
    return addrMsg.template format<uint32_t>().template select<2, 1>(2 * lane_id).template format<uint64_t>()(0);
}

} // namespace details

/* Definitions imported from cm_lsc.h in CM compiler project : End */

/// \brief Data prefetch.
///
/// @param N The number of channels (platform dependent)
///
/// @param VS Vector size
///
/// @param DS Data size
///
/// @param L1H L1 cache hint
///
/// @param L3H L3 chache hint
///
/// @param Idx Surface index, which must correspond to a buffer.
///
/// @param Offset zero based offset of the input buffer in bytes.
///
/// @param Pred Predicate
///
template <VectorSize VS = VectorSize::N1, DataSize DS = DataSize::U32,
          CacheHint L1H = CacheHint::Default,
          CacheHint L3H = CacheHint::Default,
          unsigned N = details::lsc_default_simt()>
CM_INLINE
void cm_prefetch(SurfaceIndex Idx, vector<unsigned, N> Offset,
                 vector<ushort, N> Pred = 1)
{
  static_assert(details::lsc_check_simt<N>(), "unexpected number of channels");
  // NOP for emulation
  return;
}

// Block Version.
template <unsigned NElts,
          DataSize DS = DataSize::U32,
          CacheHint L1H = CacheHint::Default,
          CacheHint L3H = CacheHint::Default>
CM_INLINE
void cm_prefetch(SurfaceIndex Idx, unsigned Offset)
{
  // NOP for emulation
  return;
}

/// Remove following code enclosed by VECTORSIZE_NELEMENTS_SUPPORT
/// when 'VS' is no longer used in kernel
#ifdef VECTORSIZE_NELEMENTS_SUPPORT
template <VectorSize VS,
          DataSize DS = DataSize::U32,
          CacheHint L1H = CacheHint::Default,
          CacheHint L3H = CacheHint::Default>
CM_INLINE
void cm_prefetch(SurfaceIndex Idx, unsigned Offset)
{
  // NOP for emulation
  return;
}
#endif // VECTORSIZE_NELEMENTS_SUPPORT

/// flat-address prefetch
template <VectorSize VS = VectorSize::N1, DataSize DS = DataSize::U32,
    CacheHint L1H = CacheHint::Default,
    CacheHint L3H = CacheHint::Default,
    unsigned N = details::lsc_default_simt()>
CM_INLINE
    void cm_ptr_prefetch(const unsigned* const Ptr, vector<unsigned, N> Offset,
        vector<ushort, N> Pred = 1) {
    // NOP for emulation
    return;
}

/// Flat-address Block prefetch.
template <VectorSize VS, DataSize DS = DataSize::U32,
    CacheHint L1H = CacheHint::Default,
    CacheHint L3H = CacheHint::Default>
CM_INLINE
    void cm_ptr_prefetch(const unsigned* const Ptr, unsigned Offset) {
    // NOP for emulation
    return;
}

template <typename T,
          VectorSize VS,
          int N,
          unsigned MASK,
          EmuBufferType BFT>
CM_INLINE
auto cm_emu_load(SurfaceIndex Idx,
                 vector<unsigned, N> Offset,
                 vector<ushort, N> Pred)
{
  // Read-in 'N' addresses, each of whose read size is 'VS'
  // elements.
  // 'DS' - 'Data size or format to read or store',
  // referenced for address alignment
  // Ignored : L1H, L3H

  // buffer-read base
  char * buff = NULL;
  int bufByteWidth = 0;

  if constexpr(BFT == EmuBufferType::UGM)
  {
    cm_list<CmEmulSys::iobuffer>::iterator buff_iter =
        CmEmulSys::search_buffer(Idx.get_data()&0xFF);
    assert(buff_iter->height == 1);
    buff = (char*) buff_iter->p_volatile;
    bufByteWidth = buff_iter->width;
  }
  else if constexpr (BFT == EmuBufferType::SLM)
  {
    buff = __cm_emu_get_slm();
    bufByteWidth = (int)__cm_emu_get_slm_size();
  }
  else
  {
    printf("%s:%d - Unsupported Emulation buffer type!!\n",
           __FUNCTION__, __LINE__);
    exit(EXIT_FAILURE);
  }

  // elemCount  : Number of vector elements to be loaded per Offset element
  constexpr uint elemCount = details::lsc_vector_size<VS>();

  vector<T, N * elemCount> _Output = 0;

  constexpr int sizeofT = sizeof(T);

  for (int offsetIdx = 0; offsetIdx < N; offsetIdx += 1)
  {
    if (Pred(offsetIdx) == 0)
    {
      // Skip _Output vector elements correpsonding to
      // predicates whose value is zero
      continue;
    }

    if ((Offset(offsetIdx) & MASK) != 0)
    {
      printf("%s : %d - Alignment Error !!\n"                     \
             "Per-offset vector read element count = %d"          \
             " / OffsetIdx = %d / Offset = %d\n",
             __FUNCTION__, __LINE__,
             elemCount,
             offsetIdx, Offset(offsetIdx));
      exit(EXIT_FAILURE);
    }

    // byteDistance : byte-distance from buffer-read base
    int byteDistance = Offset(offsetIdx);

    for (int elemIdx = 0, vecIdx = offsetIdx;
         elemIdx < elemCount;
         elemIdx += 1, byteDistance += sizeofT, vecIdx += N)
    {
      if ( (byteDistance >= 0)
          && (byteDistance < bufByteWidth))
      {
        _Output(vecIdx) = *((T*)(buff + byteDistance));
      }
    }
  }
  return _Output;
}

/// \brief Data Read.
///
/// @param T The return element data type.
///
/// @param N The number of channels (platform dependent)
///
/// @param VS Vector size
///
/// @param DS Data size
///
/// @param L1H L1 cache hint
///
/// @param L3H L3 chache hint
///
/// @param Pred Predicate
///
/// @param Idx Surface index, which must correspond to a buffer.
///
/// @param Offset zero based offset of the input buffer in bytes.
///
template <typename T,
          VectorSize VS = VectorSize::N1,
          DataSize DS = DataSize::Default,
          CacheHint L1H = CacheHint::Default,
          CacheHint L3H = CacheHint::Default,
          unsigned N = details::lsc_default_simt()>
CM_INLINE
auto cm_load(SurfaceIndex Idx,
             vector<unsigned, N> Offset,
             vector<ushort, N> Pred = 1)
{

  static_assert(details::lsc_check_simt<N>(), "unexpected number of channels");
  constexpr uint MASK = details::loadstoreAlignMask<T, VS, DS, N>();

  return cm_emu_load<T, VS, N, MASK, EmuBufferType::UGM>(Idx,
                                                         Offset,
                                                         Pred);
}

template <typename T,
          VectorSize VS = VectorSize::N1,
          DataSize DS = DataSize::Default,
          CacheHint L1H = CacheHint::Default,
          CacheHint L3H = CacheHint::Default,
          unsigned N = details::lsc_default_simt()>
CM_INLINE
auto cm_load(SurfaceIndex Idx,
             vector_ref<unsigned, N> Offset,
             vector<ushort, N> Pred = 1)
{

  static_assert(details::lsc_check_simt<N>(), "unexpected number of channels");
  constexpr uint MASK = details::loadstoreAlignMask<T, VS, DS, N>();

  return cm_emu_load<T, VS, N, MASK, EmuBufferType::UGM>(Idx,
                                                         Offset,
                                                         Pred);
}

template <typename T,
          VectorSize VS,
          DataSize DS = DataSize::Default,
          CacheHint L1H = CacheHint::Default,
          CacheHint L3H = CacheHint::Default>
CM_INLINE
auto cm_load(SurfaceIndex Idx, unsigned Offset)
{
  vector<unsigned, 1> _offsets = Offset;
  vector<short, 1> _pred = 1;

  constexpr uint MASK = details::loadstoreAlignMask<T, VS, DS, 1>();

  return cm_emu_load<T, VS, 1, MASK, EmuBufferType::UGM>(Idx, _offsets, _pred);
}

// Block Version.
#ifndef VECTORSIZE_NELEMENTS_SUPPORT
template <typename T,
          unsigned NElts,
          DataSize DS = DataSize::Default,
          CacheHint L1H = CacheHint::Default,
          CacheHint L3H = CacheHint::Default>
CM_INLINE
auto cm_load(SurfaceIndex Idx, unsigned Offset)
{
  constexpr VectorSize VS = details::lsc_vector_size_enum<NElts>();

  vector<unsigned, 1> _offsets = Offset;
  vector<short, 1> _pred = 1;

  constexpr uint MASK = details::loadstoreAlignMask<T, VS, DS, 1>();

  return cm_emu_load<T, VS, 1, MASK, EmuBufferType::UGM>(Idx,
                                                         _offsets,
                                                         _pred);
}

#else // VECTORSIZE_NELEMENTS_SUPPORT
/// Remove following code enclosed by VECTORSIZE_NELEMENTS_SUPPORT
/// when 'VS' is no longer used in kernel

template <typename T,
          unsigned NElts>
CM_INLINE
auto cm_load(SurfaceIndex Idx, unsigned Offset)
{
  constexpr VectorSize VS = details::lsc_vector_size_enum<NElts>();
  constexpr DataSize DS = details::lsc_data_size<T, DataSize::Default>();

  vector<unsigned, 1> _offsets = Offset;
  vector<short, 1> _pred = 1;

  constexpr uint MASK = details::loadstoreAlignMask<T, VS, DS, 1>();

  return cm_emu_load<T, VS, 1, MASK, EmuBufferType::UGM>(Idx,
                                                         _offsets,
                                                         _pred);
}

template <typename T,
          VectorSize VS>
CM_INLINE
auto cm_load(SurfaceIndex Idx, unsigned Offset)
{
  vector<unsigned, 1> _offsets = Offset;
  vector<short, 1> _pred = 1;

  constexpr DataSize DS = details::lsc_data_size<T, DataSize::Default>();
  constexpr uint MASK = details::loadstoreAlignMask<T, VS, DS, 1>();

  return cm_emu_load<T, VS, 1, MASK, EmuBufferType::UGM>(Idx,
                                                         _offsets,
                                                         _pred);
}

#endif // VECTORSIZE_NELEMENTS_SUPPORT

///
/// Flat-address version using a base-pointer to a buffer
///

template <typename T,
    VectorSize VS,
    int N,
    unsigned MASK>
    CM_INLINE
    auto cm_emu_ptr_load(T *Ptr,
        vector<unsigned, N> Offset,
        vector<ushort, N> Pred
        )
{
    // Read-in 'N' addresses, each of whose read size is 'VS'
    // elements.
    // 'DS' - 'Data size or format to read or store',
    // referenced for address alignment
    // Ignored : L1H, L3H

    // elemCount  : Number of vector elements to be loaded per Offset element
    constexpr uint elemCount = details::lsc_vector_size<VS>();

    vector<T, N * elemCount> _Output = 0;
    // buffer-read base
    char * buff = (char*)Ptr;

    constexpr int sizeofT = sizeof(T);

    for (int offsetIdx = 0; offsetIdx < N; offsetIdx += 1)
    {
        if (Pred(offsetIdx) == 0)
        {
            // Skip _Output vector elements correpsonding to
            // predicates whose value is zero
            continue;
        }

        if ((Offset(offsetIdx) & MASK) != 0)
        {
            printf("%s : %d - Alignment Error !!\n"                     \
                "Per-offset vector read element count = %d"          \
                " / OffsetIdx = %d / Offset = %d\n",
                __FUNCTION__, __LINE__,
                elemCount,
                offsetIdx, Offset(offsetIdx));
            exit(EXIT_FAILURE);
        }

        // byteDistance : byte-distance from buffer-read base
        int byteDistance = Offset(offsetIdx);

        for (int elemIdx = 0, vecIdx = offsetIdx;
            elemIdx < elemCount;
            elemIdx += 1, byteDistance += sizeofT, vecIdx += N)
        {
            if (byteDistance >= 0)
            {
                _Output(vecIdx) = *((T*)(buff + byteDistance));
            }
        }
    }
    return _Output;
}

template <typename T, VectorSize VS = VectorSize::N1,
    DataSize DS = DataSize::Default, CacheHint L1H = CacheHint::Default,
    CacheHint L3H = CacheHint::Default,
    unsigned N = details::lsc_default_simt()>
CM_INLINE
    auto cm_ptr_load(T* Ptr, vector<unsigned, N> Offset,
        vector<ushort, N> Pred = 1) {
    static_assert(details::lsc_check_simt<N>(), "unexpected number of channels");
    constexpr uint MASK = details::loadstoreAlignMask<T, VS, DS, N>();

    return cm_emu_ptr_load<T, VS, N, MASK>(Ptr,
        Offset,
        Pred);
}
template <typename T, unsigned NElts,
    DataSize DS = DataSize::Default, CacheHint L1H = CacheHint::Default,
    CacheHint L3H = CacheHint::Default,
    unsigned N = details::lsc_default_simt()>
CM_INLINE
    auto cm_ptr_load(T *Ptr, vector<unsigned, N> Offset,
        vector<ushort, N> Pred = 1) {
    static_assert(details::lsc_check_simt<N>(), "unexpected number of channels");
    constexpr VectorSize VS = details::lsc_vector_size_enum<NElts>();
    constexpr uint MASK = details::loadstoreAlignMask<T, VS, DS, N>();
    return cm_emu_ptr_load<T, VS, N, MASK>(Ptr, Offset, Pred);
}

// Block-load with a base-pointer to the buffer
//template <typename T, unsigned NElts, DataSize DS = DataSize::Default,
//    CacheHint L1H = CacheHint::Default,
//    CacheHint L3H = CacheHint::Default>
//    CM_INLINE
//    auto cm_ptr_load(T* Ptr, unsigned Offset) {
//    constexpr VectorSize VS = details::lsc_vector_size_enum<NElts>();
//    constexpr DataSize DS = details::lsc_data_size<T, DataSize::Default>();
//
//    vector<unsigned, 1> _offsets = Offset;
//    vector<short, 1> _pred = 1;
//
//    constexpr uint MASK = details::loadstoreAlignMask<T, VS, DS, 1>();
//
//    return cm_emu_ptr_load<T, VS, 1, MASK>(Ptr, _offsets, _pred);
//}

template <typename T, VectorSize VS, DataSize DS = details::lsc_data_size<T, DataSize::Default>(),
    CacheHint L1H = CacheHint::Default,
    CacheHint L3H = CacheHint::Default>
    CM_INLINE
    auto cm_ptr_load(T* Ptr, unsigned Offset) {
    vector<unsigned, 1> _offsets = Offset;
    vector<short, 1> _pred = 1;

    //constexpr DataSize DS = details::lsc_data_size<T, DataSize::Default>();
    constexpr uint MASK = details::loadstoreAlignMask<T, VS, DS, 1>();

    return cm_emu_ptr_load<T, VS, 1, MASK>(Ptr, _offsets, _pred);
}

template <typename T,
          VectorSize VS,
          int N,
          unsigned MASK,
          EmuBufferType BFT>
CM_INLINE
void cm_emu_store(SurfaceIndex Idx,
                  vector<unsigned, N> Offset,
                  vector<T, N * details::lsc_vector_size<VS>()> Data,
                  vector<ushort, N> Pred)
{

  // buffer-write base
  char * buff;
  int bufByteWidth = 0;

  if constexpr (BFT == EmuBufferType::UGM)
  {
    cm_list<CmEmulSys::iobuffer>::iterator buff_iter =
        CmEmulSys::search_buffer(Idx.get_data() & 0xFF);
    assert(buff_iter->height == 1);
    buff = (char*)buff_iter->p_volatile;
    bufByteWidth = buff_iter->width;
  }
  else if constexpr (BFT == EmuBufferType::SLM)
  {
    buff = __cm_emu_get_slm();
    bufByteWidth = (int)__cm_emu_get_slm_size();
  }
  else
  {
    printf("%s:%d - Unsupported Emulation buffer type!!\n",
           __FUNCTION__, __LINE__);
    exit(EXIT_FAILURE);
  }

  // elemCount  : Number of vector elements to be loaded per Offset element
  constexpr uint elemCount = details::lsc_vector_size<VS>();

  constexpr int sizeofT = sizeof(T);

  for (int offsetIdx = 0; offsetIdx < N; offsetIdx += 1)
  {
    if (Pred(offsetIdx) == 0)
    {
      // Skip _Output vector elements correpsonding to
      // predicates whose value is zero
      continue;
    }

    if ((Offset(offsetIdx) & MASK) != 0)
    {
      printf("%s : %d - Alignment Error !!\n"                     \
             "Per-offset vector write element count = %d"          \
             " / OffsetIdx = %d / Offset = %d\n",
             __FUNCTION__, __LINE__,
             elemCount,
             offsetIdx, Offset(offsetIdx));
      exit(EXIT_FAILURE);
    }

    // byteDistance : byte-distance from buffer-write base
    int byteDistance = Offset(offsetIdx);

    for (int elemIdx = 0, vecIdx = offsetIdx;
         elemIdx < elemCount;
         elemIdx += 1, byteDistance += sizeofT, vecIdx += N)
    {
      if ( (byteDistance >= 0)
          && (byteDistance < bufByteWidth))
      {
         *((T*)(buff + byteDistance)) = Data(vecIdx);
      }
    }
  }
}

/// \brief Data Write.
///
/// @param T The element data type.
///
/// @param N The number of channels (platform dependent)
///
/// @param NElts The number of element to store (for block store only)
///
/// @param VS Vector size
///
/// @param DS Data size
///
/// @param L1H L1 cache hint
///
/// @param L3H L3 chache hint
///
/// @param Pred Predicate
///
/// @param Idx Surface index, which must correspond to a buffer.
///
/// @param Offset zero based offset of the output buffer in bytes.
///
/// @param Data data to write.
///

#define CM_STORE_TEMPLATE(OFS_TYPE, DATA_TYPE)                      \
  template <typename T,                                             \
            VectorSize VS = VectorSize::N1,                         \
            DataSize DS = DataSize::Default,                        \
            CacheHint L1H = CacheHint::Default,                     \
            CacheHint L3H = CacheHint::Default,                     \
            unsigned N = details::lsc_default_simt()>                    \
  CM_INLINE                                                         \
  void cm_store(SurfaceIndex Idx,                                   \
                OFS_TYPE<unsigned, N> Offset,                         \
                DATA_TYPE<T, N * details::lsc_vector_size<VS>()> Data, \
                vector<ushort, N> Pred = 1)                            \
  {                                                                     \
    static_assert(details::lsc_check_simt<N>(), "unexpected number of channels");                  \
    constexpr uint MASK = details::loadstoreAlignMask<T, VS, DS, N>();  \
    cm_emu_store<T, VS, N, MASK, EmuBufferType::UGM>(Idx, Offset, Data, Pred); \
    return;                                                             \
  }                                                                     \

CM_STORE_TEMPLATE(vector,vector)
CM_STORE_TEMPLATE(vector,vector_ref)
CM_STORE_TEMPLATE(vector_ref,vector)
CM_STORE_TEMPLATE(vector_ref,vector_ref)

#undef CM_STORE_TEMPLATE

// Block version.
template <typename T,
          unsigned NElts,
          DataSize DS = DataSize::Default,
          CacheHint L1H = CacheHint::Default,
          CacheHint L3H = CacheHint::Default>
CM_INLINE
void cm_store(SurfaceIndex Idx, unsigned int Offset, vector<T, NElts> Data)
{
  constexpr VectorSize VS = details::lsc_vector_size_enum<NElts>();

  vector<unsigned int, 1> _offsets = Offset;
  vector<ushort, 1> _pred = 1;

  constexpr uint MASK = details::loadstoreAlignMask<T, VS, DS, 1>();

  cm_emu_store<T, VS, 1, MASK, EmuBufferType::UGM>(Idx,
                                                   _offsets,
                                                   Data,
                                                   _pred);
  return;
}

template <typename T,
          unsigned NElts,
          DataSize DS = DataSize::Default,
          CacheHint L1H = CacheHint::Default,
          CacheHint L3H = CacheHint::Default>
CM_INLINE
void cm_store(SurfaceIndex Idx, unsigned int Offset, vector_ref<T, NElts> Data)
{
  constexpr VectorSize VS = details::lsc_vector_size_enum<NElts>();

  vector<unsigned int, 1> _offsets = Offset;
  vector<ushort, 1> _pred = 1;

  constexpr uint MASK = details::loadstoreAlignMask<T, VS, DS, 1>();

  cm_emu_store<T, VS, 1, MASK, EmuBufferType::UGM>(Idx,
                                                   _offsets,
                                                   Data,
                                                   _pred);
  return;
}

/// Flat-address store using a base-address to a buffer
template <typename T,
    uint N1,
    uint N>
    CM_INLINE
    void cm_emu_ptr_store_core(T* Ptr,
        vector<unsigned, N> Offset,
        vector_ref<T, N1> Data,
        vector<ushort, N> Pred,
        uint elemCount,
        uint mask)
{
    // buffer-write base
    char * buff = (char*)Ptr;
    constexpr int sizeofT = sizeof(T);
    for (int offsetIdx = 0; offsetIdx < N; offsetIdx += 1)
    {
        if (Pred(offsetIdx) == 0)
        {
            // Skip _Output vector elements correpsonding to
            // predicates whose value is zero
            continue;
        }

        if ((Offset(offsetIdx) & mask) != 0)
        {
            printf("%s : %d - Alignment Error !!\n"                     \
                "Per-offset vector write element count = %d"          \
                " / OffsetIdx = %d / Offset = %d\n",
                __FUNCTION__, __LINE__,
                elemCount,
                offsetIdx, Offset(offsetIdx));
            exit(EXIT_FAILURE);
        }

        // byteDistance : byte-distance from buffer-write base
        int byteDistance = Offset(offsetIdx);

        for (int elemIdx = 0, vecIdx = offsetIdx;
            elemIdx < elemCount;
            elemIdx += 1, byteDistance += sizeofT, vecIdx += N)
        {
            if (byteDistance >= 0)
            {
                *((T*)(buff + byteDistance)) = Data(vecIdx);
            }
        }
    }
}
template <typename T,
    VectorSize VS,
    int N,
    unsigned MASK>
    CM_INLINE
    void cm_emu_ptr_store(T* Ptr,
        vector<unsigned, N> Offset,
        vector<T, N * details::lsc_vector_size<VS>()> Data,
        vector<ushort, N> Pred)
{
    // elemCount  : Number of vector elements to be loaded per Offset element
    constexpr uint elemCount = details::lsc_vector_size<VS>();
    // buffer-write base
    char * buff = (char*)Ptr;
    constexpr int sizeofT = sizeof(T);

    for (int offsetIdx = 0; offsetIdx < N; offsetIdx += 1)
    {
        if (Pred(offsetIdx) == 0)
        {
            // Skip _Output vector elements correpsonding to
            // predicates whose value is zero
            continue;
        }

        if ((Offset(offsetIdx) & MASK) != 0)
        {
            printf("%s : %d - Alignment Error !!\n"                     \
                "Per-offset vector write element count = %d"          \
                " / OffsetIdx = %d / Offset = %d\n",
                __FUNCTION__, __LINE__,
                elemCount,
                offsetIdx, Offset(offsetIdx));
            exit(EXIT_FAILURE);
        }

        // byteDistance : byte-distance from buffer-write base
        int byteDistance = Offset(offsetIdx);

        for (int elemIdx = 0, vecIdx = offsetIdx;
            elemIdx < elemCount;
            elemIdx += 1, byteDistance += sizeofT, vecIdx += N)
        {
            if (byteDistance >= 0)
            {
                *((T*)(buff + byteDistance)) = Data(vecIdx);
            }
        }
    }
}

#define CM_PTR_STORE_TEMPLATE(OFS_TYPE, DATA_TYPE)                      \
  template <typename T,                                             \
            VectorSize VS = VectorSize::N1,                         \
            DataSize DS = DataSize::Default,                        \
            CacheHint L1H = CacheHint::Default,                     \
            CacheHint L3H = CacheHint::Default,                     \
            unsigned N = details::lsc_default_simt()>                    \
  CM_INLINE                                                         \
  void cm_ptr_store(T* Ptr,                                   \
                OFS_TYPE<unsigned, N> Offset,                         \
                DATA_TYPE<T, N * details::lsc_vector_size<VS>()> Data, \
                vector<ushort, N> Pred = 1)                            \
  {                                                                     \
    static_assert(details::lsc_check_simt<N>(), "unexpected number of channels");                  \
    constexpr uint MASK = details::loadstoreAlignMask<T, VS, DS, N>();  \
    cm_emu_ptr_store<T, VS, N, MASK>(Ptr, Offset, Data, Pred);          \
    return;                                                             \
  }                                                                     \

CM_PTR_STORE_TEMPLATE(vector, vector)
CM_PTR_STORE_TEMPLATE(vector, vector_ref)
CM_PTR_STORE_TEMPLATE(vector_ref, vector)
CM_PTR_STORE_TEMPLATE(vector_ref, vector_ref)

#undef CM_PTR_STORE_TEMPLATE

/// Block store with a base pointer.
template <typename T, unsigned NElts, DataSize DS = DataSize::Default,
    CacheHint L1H = CacheHint::Default,
    CacheHint L3H = CacheHint::Default>
    CM_INLINE
    void cm_ptr_store(T* ptr, unsigned Offset, vector<T, NElts> Data) {
    constexpr VectorSize VS = details::lsc_vector_size_enum<NElts>();

    vector<unsigned int, 1> _offsets = Offset;
    vector<ushort, 1> _pred = 1;

    constexpr uint MASK = details::loadstoreAlignMask<T, VS, DS, 1>();

    cm_emu_ptr_store<T, VS, 1, MASK>(ptr,
        _offsets,
        Data,
        _pred);
    return;
}

template <typename T, unsigned NElts, DataSize DS = DataSize::Default,
    CacheHint L1H = CacheHint::Default,
    CacheHint L3H = CacheHint::Default>
    CM_INLINE
    void cm_ptr_store(T* Ptr, unsigned Offset, vector_ref<T, NElts> Data) {
    constexpr VectorSize VS = details::lsc_vector_size_enum<NElts>();

    vector<unsigned int, 1> _offsets = Offset;
    vector<ushort, 1> _pred = 1;

    constexpr uint MASK = details::loadstoreAlignMask<T, VS, DS, 1>();

    cm_emu_ptr_store<T, VS, 1, MASK>(Ptr,
        _offsets,
        Data,
        _pred);
    return;
}

// raw send

/// \brief SLM Data Read.
///
/// @param T The return element data type.
///
/// @param N The number of channels (platform dependent)
///
/// @param VS Vector size
///
/// @param DS Data size
///
/// @param Pred Predicate
///
/// @param Offset zero based offset of the input SLM buffer in bytes.
///
template <typename T,
          VectorSize VS = VectorSize::N1,
          DataSize DS = DataSize::Default,
          unsigned N = details::lsc_default_simt()>
CM_INLINE
auto cm_load_slm(vector<unsigned, N> Offset,
                 vector<ushort, N> Pred = 1)
{
  static_assert(details::lsc_check_simt<N>(), "unexpected number of channels");

  constexpr uint MASK = details::loadstoreAlignMask<T, VS, DS, N>();

  return cm_emu_load<T, VS, N, MASK, EmuBufferType::SLM>(SLM_SURFACE_IDX,
                                                         Offset,
                                                         Pred);
}

template <typename T,
          VectorSize VS = VectorSize::N1,
          DataSize DS = DataSize::Default,
          unsigned N = details::lsc_default_simt()>
CM_INLINE
auto cm_load_slm(vector_ref<unsigned, N> Offset,
                 vector<ushort, N> Pred = 1)
{
  static_assert(details::lsc_check_simt<N>(), "unexpected number of channels");

  constexpr uint MASK = details::loadstoreAlignMask<T, VS, DS, N>();

  return cm_emu_load<T, VS, N, MASK, EmuBufferType::SLM>(SLM_SURFACE_IDX,
                                                         Offset,
                                                         Pred);
}

// Block Version.
#ifndef VECTORSIZE_NELEMENTS_SUPPORT
template <typename T,
          unsigned NElts,
          DataSize DS = DataSize::Default>
CM_INLINE
auto cm_load_slm(unsigned Offset)
{
  constexpr VectorSize VS = details::lsc_vector_size_enum<NElts>();

  vector<unsigned, 1> _offsets = Offset;
  vector<short, 1> _pred = 1;

  constexpr uint MASK = details::loadstoreAlignMask<T, VS, DS, 1>();

  return cm_emu_load<T, VS, 1, MASK, EmuBufferType::SLM>(SLM_SURFACE_IDX,
                                                         _offsets,
                                                         _pred);
}
#else // VECTORSIZE_NELEMENTS_SUPPORT
/// Remove following code enclosed by VECTORSIZE_NELEMENTS_SUPPORT
/// when 'VS' is no longer used in kernel
template <typename T,
          unsigned NElts>
CM_INLINE
auto cm_load_slm(unsigned Offset)
{
  vector<unsigned, 1> _offsets = Offset;
  vector<short, 1> _pred = 1;

  constexpr VectorSize VS = details::lsc_vector_size_enum<NElts>();
  constexpr DataSize DS = details::lsc_data_size<T, DataSize::Default>();
  constexpr uint MASK = details::loadstoreAlignMask<T, VS, DS, 1>();

  return cm_emu_load<T, VS, 1, MASK, EmuBufferType::SLM>(SLM_SURFACE_IDX,
                                                         _offsets,
                                                         _pred);
}

template <typename T,
          VectorSize VS>
CM_INLINE
auto cm_load_slm(unsigned Offset)
{
  vector<unsigned, 1> _offsets = Offset;
  vector<short, 1> _pred = 1;

  constexpr DataSize DS = details::lsc_data_size<T, DataSize::Default>();
  constexpr uint MASK = details::loadstoreAlignMask<T, VS, DS, 1>();

  return cm_emu_load<T, VS, 1, MASK, EmuBufferType::SLM>(SLM_SURFACE_IDX,
                                                         _offsets,
                                                         _pred);
}
#endif // VECTORSIZE_NELEMENTS_SUPPORT

/// \brief SLM Data Write.
///
/// @param T The element data type.
///
/// @param N The number of channels (platform dependent)
///
/// @param NElts The number of element to store (for block store only)
///
/// @param VS Vector size
///
/// @param DS Data size
///
/// @param Pred Predicate
///
/// @param Offset zero based offset of the output SLM buffer in bytes.
///
/// @param Data data to write.
///

#define CM_STORE_SLM_TEMPLATE(OFS_TYPE, DATA_TYPE) \
  template <typename T, VectorSize VS = VectorSize::N1,                 \
            DataSize DS = DataSize::Default, unsigned N = details::lsc_default_simt()> \
  CM_INLINE                                                             \
  void cm_store_slm(OFS_TYPE<unsigned, N> Offset,                         \
                    DATA_TYPE<T, N * details::lsc_vector_size<VS>()> Data, \
                    vector<ushort, N> Pred = 1)                         \
  {                                                                     \
    static_assert(details::lsc_check_simt<N>(), "unexpected number of channels");                  \
    constexpr uint MASK = details::loadstoreAlignMask<T, VS, DS, N>();  \
    cm_emu_store<T, VS, N, MASK, EmuBufferType::SLM>(SLM_SURFACE_IDX, Offset, Data, Pred); \
  }                                                                     \

CM_STORE_SLM_TEMPLATE(vector,vector)
CM_STORE_SLM_TEMPLATE(vector,vector_ref)
CM_STORE_SLM_TEMPLATE(vector_ref,vector)
CM_STORE_SLM_TEMPLATE(vector_ref,vector_ref)

#undef CM_STORE_SLM_TEMPLATE

// Block version.
template <typename T, unsigned NElts, DataSize DS = DataSize::Default>
CM_INLINE
void cm_store_slm(unsigned Offset, vector<T, NElts> Data)
{
  constexpr VectorSize VS = details::lsc_vector_size_enum<NElts>();

  vector<unsigned int, 1> _offsets = Offset;
  vector<ushort, 1> _pred = 1;

  constexpr uint MASK = details::loadstoreAlignMask<T, VS, DS, 1>();

  cm_emu_store<T, VS, 1, MASK, EmuBufferType::SLM>(SLM_SURFACE_IDX,
                                                   _offsets,
                                                   Data,
                                                   _pred);
  return;
}

template <typename T, unsigned NElts, DataSize DS = DataSize::Default>
CM_INLINE
void cm_store_slm(unsigned Offset, vector_ref<T, NElts> Data)
{
  constexpr VectorSize VS = details::lsc_vector_size_enum<NElts>();

  vector<unsigned int, 1> _offsets = Offset;
  vector<ushort, 1> _pred = 1;

  constexpr uint MASK = details::loadstoreAlignMask<T, VS, DS, 1>();

  cm_emu_store<T, VS, 1, MASK, EmuBufferType::SLM>(SLM_SURFACE_IDX,
                                                   _offsets,
                                                   Data,
                                                   _pred);
  return;
}

template <AtomicOp Op,
          typename T,
          VectorSize VS,
          int N,
          unsigned MASK,
          EmuBufferType BFT>
CM_INLINE auto
cm_emu_ptr_atomic_zero_src(T* Ptr,
                       vector<unsigned, N> Offset,
                       vector<ushort, N> Pred)
{
  char* buff;
  buff = (char*) Ptr;
  int bufByteWidth = 0;

  // elemCount  : Number of vector elements to be loaded per Offset element
  constexpr uint elemCount = details::lsc_vector_size<VS>();

  vector<T, N * elemCount> _Output = 0;

  constexpr int sizeofT = sizeof(T);

  for (int offsetIdx = 0; offsetIdx < N; offsetIdx += 1)
  {
    if (Pred(offsetIdx) == 0)
    {
      // Skip _Output vector elements correpsonding to
      // predicates whose value is zero
      continue;
    }

    if ((Offset(offsetIdx) & MASK) != 0)
    {
      printf("%s : %d - Alignment Error !!\n"                     \
             "Per-offset vector element count = %d"          \
             " / OffsetIdx = %d / Offset = %d\n",
             __FUNCTION__, __LINE__,
             elemCount,
             offsetIdx, Offset(offsetIdx));
      exit(EXIT_FAILURE);
    }
    // byteDistance : byte-distance from buffer base
    int byteDistance = Offset(offsetIdx);

    for (int elemIdx = 0, vecIdx = offsetIdx;
         elemIdx < elemCount;
         elemIdx += 1, byteDistance += sizeofT, vecIdx += N)
    {
      if (byteDistance < 0
          || byteDistance > bufByteWidth)
      {
        // Out-of-bound
        continue;
      }

      _Output(vecIdx) = *((T*)(buff + byteDistance));

      if constexpr (Op == AtomicOp::IINC)
      {
        *((T*)(buff + byteDistance)) += static_cast<T>(1);
      }
      else if constexpr (Op == AtomicOp::IDEC)
      {
        *((T*)(buff + byteDistance)) -= static_cast<T>(1);
      }
      /*
      else if constexpr (Op == AtomicOp::LOAD)
      {
        // LOAD is already done after boundary check
      }
      */
    } // elemIdx loop
  } /// offsetIdx loop

  return _Output;
}

template <AtomicOp Op,
          typename T,
          VectorSize VS,
          int N,
          unsigned MASK,
          EmuBufferType BFT>
CM_INLINE auto
cm_emu_ptr_atomic_single_src(T* Ptr,
                         vector<unsigned, N> Offset,
                         vector<T, N * details::lsc_vector_size<VS>()> Src0,
                         vector<ushort, N> Pred)
{
  // buffer-write base
  char * buff;
  buff = (char*) Ptr;
  int bufByteWidth = 0;

  // elemCount  : Number of vector elements to be loaded per Offset element
  constexpr uint elemCount = details::lsc_vector_size<VS>();

  vector<T, N * elemCount> _Output = 0;

  constexpr int sizeofT = sizeof(T);

  for (int offsetIdx = 0; offsetIdx < N; offsetIdx += 1)
  {
    if (Pred(offsetIdx) == 0)
    {
      // Skip Data vector elements correpsonding to
      // predicates whose value is zero
      continue;
    }

    if ((Offset(offsetIdx) & MASK) != 0)
    {
      printf("%s : %d - Alignment Error !!\n"                     \
             "Per-offset vector read element count = %d"          \
             " / OffsetIdx = %d / Offset = %d\n",
             __FUNCTION__, __LINE__,
             elemCount,
             offsetIdx, Offset(offsetIdx));
      exit(EXIT_FAILURE);
    }

    // byteDistance : byte-distance from buffer-read base
    int byteDistance = Offset(offsetIdx);

    for (int elemIdx = 0, vecIdx = offsetIdx;
         elemIdx < elemCount;
         elemIdx += 1, byteDistance += sizeofT, vecIdx += N)
    {
      if (byteDistance < 0
          || byteDistance > bufByteWidth)
      {
        // Out-of-bound
        continue;
      }

      _Output(vecIdx) = *((T*)(buff + byteDistance));

      if constexpr (Op == AtomicOp::STORE)
      {
        *((T*)(buff + byteDistance)) = Src0(vecIdx);
      }
      else if constexpr (Op == AtomicOp::IADD)
      {
        *((T*)(buff + byteDistance)) += (int)Src0(vecIdx);
      }
      else if constexpr (Op == AtomicOp::ISUB)
      {
        *((T*)(buff + byteDistance)) -= (int)Src0(vecIdx);
      }
      else if constexpr (Op == AtomicOp::SMIN)
      {
        if ((int)Src0(vecIdx) < (int)_Output(vecIdx))
        {
          *((T*)(buff + byteDistance)) = Src0(vecIdx);
        }
      }
      else if constexpr (Op == AtomicOp::SMAX)
      {
        if ((int)Src0(vecIdx) > (int)_Output(vecIdx))
        {
          *((T*)(buff + byteDistance)) = Src0(vecIdx);
        }
      }
      else if constexpr (Op == AtomicOp::UMIN)
      {
        if (!is_fp_type<T>::value)
        {
          if ((uint)Src0(vecIdx) < (uint)_Output(vecIdx))
          {
            *((T*)(buff + byteDistance)) = Src0(vecIdx);
          }
        }
      }
      else if constexpr (Op == AtomicOp::UMAX)
      {
        if (!is_fp_type<T>::value)
        {
          if ((uint)Src0(vecIdx) > (uint)_Output(vecIdx))
          {
            *((T*)(buff + byteDistance)) = Src0(vecIdx);
          }
        }
      }
      else if constexpr (Op == AtomicOp::FADD)
      {
        if (is_fp_type<T>::value)
        {
          *((T*)(buff + byteDistance)) += (float)Src0(vecIdx);
        }
      }
      else if constexpr (Op == AtomicOp::FSUB)
      {
        if (is_fp_type<T>::value)
        {
          *((T*)(buff + byteDistance)) -= (float)Src0(vecIdx);
        }
      }
      else if constexpr (Op == AtomicOp::FMIN)
      {
        if (is_fp_type<T>::value)
        {
          if ((uint)Src0(vecIdx) < (float)_Output(vecIdx))
          {
            *((T*)(buff + byteDistance)) = Src0(vecIdx);
          }
        }
      }
      else if constexpr (Op == AtomicOp::FMAX)
      {
        if (is_fp_type<T>::value)
        {
          if ((uint)Src0(vecIdx) > (float)_Output(vecIdx))
          {
            *((T*)(buff + byteDistance)) = Src0(vecIdx);
          }
        }
      }
      else if constexpr (Op == AtomicOp::AND)
      {
        if (std::is_same_v<T, uint>)
        {
          *((T*)(buff + byteDistance)) &= (uint)Src0(vecIdx);
        }
        else if (std::is_same_v<T, int>)
        {
          *((T*)(buff + byteDistance)) &= (int)Src0(vecIdx);
        }
      }
      else if constexpr (Op == AtomicOp::OR)
      {
        if (std::is_same_v<T, uint>)
        {
          *((T*)(buff + byteDistance)) |= (uint)Src0(vecIdx);
        }
        else if (std::is_same_v<T, int>)
        {
          *((T*)(buff + byteDistance)) |= (int)Src0(vecIdx);
        }
      }
      else if constexpr (Op == AtomicOp::XOR)
      {
        if (std::is_same_v<T, uint>)
        {
          *((T*)(buff + byteDistance)) ^= (uint)Src0(vecIdx);
        }
        else if (std::is_same_v<T, int>)
        {
          *((T*)(buff + byteDistance)) ^= (int)Src0(vecIdx);
        }
      }
    } /// elemIdx loop
  } /// offsetIdx loop

  return _Output;
}

template <AtomicOp Op,
          typename T,
          VectorSize VS,
          int N,
          unsigned MASK,
          EmuBufferType BFT>
CM_INLINE auto
cm_emu_ptr_atomic_binary_src(T* Ptr,
                         vector<unsigned, N> Offset,
                         vector<T, N * details::lsc_vector_size<VS>()> Src0,
                         vector<T, N * details::lsc_vector_size<VS>()> Src1,
                         vector<ushort, N> Pred)
{
  // buffer-write base
  char * buff;
  buff = (char*) Ptr;
  int bufByteWidth = 0;

  // elemCount  : Number of vector elements to be loaded per Offset element
  constexpr uint elemCount = details::lsc_vector_size<VS>();

  vector<T, N * elemCount> _Output = 0;

  constexpr int sizeofT = sizeof(T);

  for (int offsetIdx = 0; offsetIdx < N; offsetIdx += 1)
  {
    if (Pred(offsetIdx) == 0)
    {
      // Skip Data vector elements correpsonding to
      // predicates whose value is zero
      continue;
    }

    if ((Offset(offsetIdx) & MASK) != 0)
    {
      printf("%s : %d - Alignment Error !!\n"                     \
             "Per-offset vector read element count = %d"          \
             " / OffsetIdx = %d / Offset = %d\n",
             __FUNCTION__, __LINE__,
             elemCount,
             offsetIdx, Offset(offsetIdx));
      exit(EXIT_FAILURE);
    }

    // byteDistance : byte-distance from buffer-read base
    int byteDistance = Offset(offsetIdx);

    for (int elemIdx = 0, vecIdx = offsetIdx;
         elemIdx < elemCount;
         elemIdx += 1, byteDistance += sizeofT, vecIdx += N)
    {
      if (byteDistance < 0
          || byteDistance > bufByteWidth)
      {
        // Out-of-bound
        continue;
      }

      _Output(vecIdx) = *((T*)(buff + byteDistance));
      // _Output(vecIdx) contains old value

      // new = (old == src0) ? src1 : old:
      if constexpr (Op == AtomicOp::ICAS)
      {
        if (_Output(vecIdx) == Src0(vecIdx))
        {
          *((T*)(buff + byteDistance)) = Src1(vecIdx);
        }
      } /// ICAS
      else if constexpr (Op == AtomicOp::FCAS)
      {
        if (is_fp_type<T>::value)
        {
          if (_Output(vecIdx) == Src0(vecIdx))
          {
            *((T*)(buff + byteDistance)) = Src1(vecIdx);
          }
        }
      } /// FCAS
    } // elemIdx Loop
  } // offsetIdx Loop

  return _Output;
}

/// \brief LSC Ptr Atomic.
///
/// @param T The element data type.
///
/// @param VS Vector size
///
/// @param DS Data size
///
/// @param L1H L1 cache hint
///
/// @param L3H L3 chache hint
///
/// @param Pred Predicate
///
/// @param Ptr Base-pointer to the buffer
///
/// @param Offset zero based byte offset of the input buffer or SLM byte offset
///

// flat-address atomic
template <AtomicOp Op, typename T, VectorSize VS = VectorSize::N1,
    DataSize DS = DataSize::Default, CacheHint L1H = CacheHint::Default,
    CacheHint L3H = CacheHint::Default,
    unsigned N = details::lsc_default_simt()>
    CM_INLINE auto
    cm_ptr_atomic(T* Ptr, vector<unsigned, N> Offset,
        vector<ushort, N> Pred = 1) ->
    typename std::enable_if<details::lsc_atomic_nsrcs<Op>() == 0,
    decltype(vector<T, N * details::lsc_vector_size<VS>()>())>::type {
      static_assert(details::lsc_check_simt<N>(), "unexpected number of channels");
      constexpr uint MASK = details::atomicAlignMask<Op, T, VS, DS, N, EmuBufferType::UGM>();

      std::unique_lock<std::mutex> lock(atomicMutex);
      return cm_emu_ptr_atomic_zero_src<Op, T, VS, N, MASK, EmuBufferType::UGM>(Ptr,
                                                                        Offset, Pred);
}

template <AtomicOp Op, typename T, VectorSize VS = VectorSize::N1,
    DataSize DS = DataSize::Default, CacheHint L1H = CacheHint::Default,
    CacheHint L3H = CacheHint::Default,
    unsigned N = details::lsc_default_simt()>
    CM_INLINE auto
    cm_ptr_atomic(T* Ptr, vector_ref<unsigned, N> Offset,
        vector<ushort, N> Pred = 1) ->
    typename std::enable_if<details::lsc_atomic_nsrcs<Op>() == 0,
    decltype(vector<T, N * details::lsc_vector_size<VS>()>())>::type {
      static_assert(details::lsc_check_simt<N>(), "unexpected number of channels");
      constexpr uint MASK = details::atomicAlignMask<Op, T, VS, DS, N, EmuBufferType::UGM>();

      std::unique_lock<std::mutex> lock(atomicMutex);
      return cm_emu_ptr_atomic_zero_src<Op, T, VS, N, MASK, EmuBufferType::UGM>(Ptr,
                                                                        Offset, Pred);
}

#define CM_PTR_ATOMIC_SINGLE_SRC_TMPL(OFS_TYPE, SRC_TYPE)                    \
  template <AtomicOp Op, typename T, VectorSize VS = VectorSize::N1,         \
      DataSize DS = DataSize::Default, CacheHint L1H = CacheHint::Default,   \
      CacheHint L3H = CacheHint::Default,                                    \
      unsigned N = details::lsc_default_simt()>                              \
      CM_INLINE auto                                                         \
      cm_ptr_atomic(T* Ptr, OFS_TYPE<unsigned, N> Offset,                    \
          SRC_TYPE<T, N * details::lsc_vector_size<VS>()> Src0,              \
          vector<ushort, N> Pred = 1) ->                                     \
      typename std::enable_if<details::lsc_atomic_nsrcs<Op>() == 1,          \
      decltype(Src0)>::type                                                  \
      {                                                                      \
        static_assert(details::lsc_check_simt<N>(), "unexpected number of channels");     \
        constexpr uint MASK = details::atomicAlignMask<Op, T, VS, DS, N, EmuBufferType::UGM>(); \
        std::unique_lock<std::mutex> lock(atomicMutex);                      \
        return cm_emu_ptr_atomic_single_src<Op, T, VS, N, MASK, EmuBufferType::UGM>(Ptr, Offset, Src0, Pred); \
  }               \

CM_PTR_ATOMIC_SINGLE_SRC_TMPL(vector, vector)
CM_PTR_ATOMIC_SINGLE_SRC_TMPL(vector, vector_ref)
CM_PTR_ATOMIC_SINGLE_SRC_TMPL(vector_ref, vector)
CM_PTR_ATOMIC_SINGLE_SRC_TMPL(vector_ref, vector_ref)

#undef CM_PTR_ATOMIC_SINGLE_SRC_TMPL

#define CM_PTR_ATOMIC_BINARY_SRC_TMPL(OFS_TYPE, SRC0_TYPE, SRC1_TYPE)      \
template <AtomicOp Op, typename T, VectorSize VS = VectorSize::N1,         \
    DataSize DS = DataSize::Default, CacheHint L1H = CacheHint::Default,   \
    CacheHint L3H = CacheHint::Default,                                    \
    unsigned N = details::lsc_default_simt()>                              \
    CM_INLINE auto                                                         \
    cm_ptr_atomic(T* Ptr, OFS_TYPE<unsigned, N> Offset,                      \
        SRC0_TYPE<T, N * details::lsc_vector_size<VS>()> Src0,                \
        SRC1_TYPE<T, N * details::lsc_vector_size<VS>()> Src1,                \
        vector<ushort, N> Pred = 1) ->                                     \
    typename std::enable_if<details::lsc_atomic_nsrcs<Op>() == 2,          \
    decltype(Src0)>::type                                                  \
    {                                                                      \
      static_assert(details::lsc_check_simt<N>(), "unexpected number of channels");   \
      constexpr uint MASK = details::atomicAlignMask<Op, T, VS, DS, N, EmuBufferType::UGM>(); \
      std::unique_lock<std::mutex> lock(atomicMutex);                     \
      return cm_emu_ptr_atomic_binary_src<Op, T, VS, N, MASK, EmuBufferType::UGM>(Ptr, Offset, Src0, Src1, Pred); \
}     \

CM_PTR_ATOMIC_BINARY_SRC_TMPL(vector, vector, vector)
CM_PTR_ATOMIC_BINARY_SRC_TMPL(vector, vector, vector_ref)
CM_PTR_ATOMIC_BINARY_SRC_TMPL(vector, vector_ref, vector)
CM_PTR_ATOMIC_BINARY_SRC_TMPL(vector, vector_ref, vector_ref)
CM_PTR_ATOMIC_BINARY_SRC_TMPL(vector_ref, vector, vector)
CM_PTR_ATOMIC_BINARY_SRC_TMPL(vector_ref, vector, vector_ref)
CM_PTR_ATOMIC_BINARY_SRC_TMPL(vector_ref, vector_ref, vector)
CM_PTR_ATOMIC_BINARY_SRC_TMPL(vector_ref, vector_ref, vector_ref)

#undef CM_PTR_ATOMIC_BINARY_SRC_TMPL

/// \brief LSC Atomic.
///
/// @param T The element data type.
///
/// @param VS Vector size
///
/// @param DS Data size
///
/// @param L1H L1 cache hint
///
/// @param L3H L3 chache hint
///
/// @param Pred Predicate
///
/// @param Idx Surface index, which must correspond to a buffer.
///
/// @param Offset zero based byte offset of the input buffer or SLM byte offset
///

template <AtomicOp Op,
          typename T,
          VectorSize VS,
          int N,
          unsigned MASK,
          EmuBufferType BFT>
CM_INLINE auto
cm_emu_atomic_zero_src(SurfaceIndex Idx,
                       vector<unsigned, N> Offset,
                       vector<ushort, N> Pred)
{
  // buffer-write base
  char * buff;
  int bufByteWidth = 0;

  if constexpr (BFT == EmuBufferType::UGM)
  {
    cm_list<CmEmulSys::iobuffer>::iterator buff_iter =
        CmEmulSys::search_buffer(Idx.get_data() & 0xFF);
    assert(buff_iter->height == 1);
    buff = (char*)buff_iter->p_volatile;
    bufByteWidth = buff_iter->width;
  }
  else if constexpr (BFT == EmuBufferType::SLM)
  {
    buff = __cm_emu_get_slm();
    bufByteWidth = __cm_emu_get_slm_size();
  }
  else
  {
    printf("%s:%d - Unsupported Emulation buffer type!!\n",
           __FUNCTION__, __LINE__);
    exit(EXIT_FAILURE);
  }

  // elemCount  : Number of vector elements to be loaded per Offset element
  constexpr uint elemCount = details::lsc_vector_size<VS>();

  vector<T, N * elemCount> _Output = 0;

  constexpr int sizeofT = sizeof(T);

  for (int offsetIdx = 0; offsetIdx < N; offsetIdx += 1)
  {
    if (Pred(offsetIdx) == 0)
    {
      // Skip _Output vector elements correpsonding to
      // predicates whose value is zero
      continue;
    }

    if ((Offset(offsetIdx) & MASK) != 0)
    {
      printf("%s : %d - Alignment Error !!\n"                     \
             "Per-offset vector element count = %d"          \
             " / OffsetIdx = %d / Offset = %d\n",
             __FUNCTION__, __LINE__,
             elemCount,
             offsetIdx, Offset(offsetIdx));
      exit(EXIT_FAILURE);
    }
    // byteDistance : byte-distance from buffer base
    int byteDistance = Offset(offsetIdx);

    for (int elemIdx = 0, vecIdx = offsetIdx;
         elemIdx < elemCount;
         elemIdx += 1, byteDistance += sizeofT, vecIdx += N)
    {
      if (byteDistance < 0
          || byteDistance > bufByteWidth)
      {
        // Out-of-bound
        continue;
      }

      _Output(vecIdx) = *((T*)(buff + byteDistance));

      if constexpr (Op == AtomicOp::IINC)
      {
        *((T*)(buff + byteDistance)) += static_cast<T>(1);
      }
      else if constexpr (Op == AtomicOp::IDEC)
      {
        *((T*)(buff + byteDistance)) -= static_cast<T>(1);
      }
      /*
      else if constexpr (Op == AtomicOp::LOAD)
      {
        // LOAD is already done after boundary check
      }
      */
    } // elemIdx loop
  } /// offsetIdx loop

  return _Output;
}

template <AtomicOp Op,
          typename T,
          VectorSize VS,
          int N,
          unsigned MASK,
          EmuBufferType BFT>
CM_INLINE auto
cm_emu_atomic_single_src(SurfaceIndex Idx,
                         vector<unsigned, N> Offset,
                         vector<T, N * details::lsc_vector_size<VS>()> Src0,
                         vector<ushort, N> Pred)
{
  // buffer-write base
  char * buff;
  int bufByteWidth = 0;

  if constexpr (BFT == EmuBufferType::UGM)
  {
    cm_list<CmEmulSys::iobuffer>::iterator buff_iter =
        CmEmulSys::search_buffer(Idx.get_data() & 0xFF);
    assert(buff_iter->height == 1);
    buff = (char*)buff_iter->p_volatile;
    bufByteWidth = buff_iter->width;
  }
  else if constexpr (BFT == EmuBufferType::SLM)
  {
    buff = __cm_emu_get_slm();
    bufByteWidth = __cm_emu_get_slm_size();
  }
  else
  {
    printf("%s:%d - Unsupported Emulation buffer type!!\n",
           __FUNCTION__, __LINE__);
    exit(EXIT_FAILURE);
  }

  // elemCount  : Number of vector elements to be loaded per Offset element
  constexpr uint elemCount = details::lsc_vector_size<VS>();

  vector<T, N * elemCount> _Output = 0;

  constexpr int sizeofT = sizeof(T);

  for (int offsetIdx = 0; offsetIdx < N; offsetIdx += 1)
  {
    if (Pred(offsetIdx) == 0)
    {
      // Skip Data vector elements correpsonding to
      // predicates whose value is zero
      continue;
    }

    if ((Offset(offsetIdx) & MASK) != 0)
    {
      printf("%s : %d - Alignment Error !!\n"                     \
             "Per-offset vector read element count = %d"          \
             " / OffsetIdx = %d / Offset = %d\n",
             __FUNCTION__, __LINE__,
             elemCount,
             offsetIdx, Offset(offsetIdx));
      exit(EXIT_FAILURE);
    }

    // byteDistance : byte-distance from buffer-read base
    int byteDistance = Offset(offsetIdx);

    for (int elemIdx = 0, vecIdx = offsetIdx;
         elemIdx < elemCount;
         elemIdx += 1, byteDistance += sizeofT, vecIdx += N)
    {
      if (byteDistance < 0
          || byteDistance > bufByteWidth)
      {
        // Out-of-bound
        continue;
      }

      _Output(vecIdx) = *((T*)(buff + byteDistance));

      if constexpr (Op == AtomicOp::STORE)
      {
        *((T*)(buff + byteDistance)) = Src0(vecIdx);
      }
      else if constexpr (Op == AtomicOp::IADD)
      {
        *((T*)(buff + byteDistance)) += (int)Src0(vecIdx);
      }
      else if constexpr (Op == AtomicOp::ISUB)
      {
        *((T*)(buff + byteDistance)) -= (int)Src0(vecIdx);
      }
      else if constexpr (Op == AtomicOp::SMIN)
      {
        if ((int)Src0(vecIdx) < (int)_Output(vecIdx))
        {
          *((T*)(buff + byteDistance)) = Src0(vecIdx);
        }
      }
      else if constexpr (Op == AtomicOp::SMAX)
      {
        if ((int)Src0(vecIdx) > (int)_Output(vecIdx))
        {
          *((T*)(buff + byteDistance)) = Src0(vecIdx);
        }
      }
      else if constexpr (Op == AtomicOp::UMIN)
      {
        if (!is_fp_type<T>::value)
        {
          if ((uint)Src0(vecIdx) < (uint)_Output(vecIdx))
          {
            *((T*)(buff + byteDistance)) = Src0(vecIdx);
          }
        }
      }
      else if constexpr (Op == AtomicOp::UMAX)
      {
        if (!is_fp_type<T>::value)
        {
          if ((uint)Src0(vecIdx) > (uint)_Output(vecIdx))
          {
            *((T*)(buff + byteDistance)) = Src0(vecIdx);
          }
        }
      }
      else if constexpr (Op == AtomicOp::FADD)
      {
        if (is_fp_type<T>::value)
        {
          *((T*)(buff + byteDistance)) += (float)Src0(vecIdx);
        }
      }
      else if constexpr (Op == AtomicOp::FSUB)
      {
        if (is_fp_type<T>::value)
        {
          *((T*)(buff + byteDistance)) -= (float)Src0(vecIdx);
        }
      }
      else if constexpr (Op == AtomicOp::FMIN)
      {
        if (is_fp_type<T>::value)
        {
          if ((uint)Src0(vecIdx) < (float)_Output(vecIdx))
          {
            *((T*)(buff + byteDistance)) = Src0(vecIdx);
          }
        }
      }
      else if constexpr (Op == AtomicOp::FMAX)
      {
        if (is_fp_type<T>::value)
        {
          if ((uint)Src0(vecIdx) > (float)_Output(vecIdx))
          {
            *((T*)(buff + byteDistance)) = Src0(vecIdx);
          }
        }
      }
      else if constexpr (Op == AtomicOp::AND)
      {
        if (std::is_same_v<T, uint>)
        {
          *((T*)(buff + byteDistance)) &= (uint)Src0(vecIdx);
        }
        else if (std::is_same_v<T, int>)
        {
          *((T*)(buff + byteDistance)) &= (int)Src0(vecIdx);
        }
      }
      else if constexpr (Op == AtomicOp::OR)
      {
        if (std::is_same_v<T, uint>)
        {
          *((T*)(buff + byteDistance)) |= (uint)Src0(vecIdx);
        }
        else if (std::is_same_v<T, int>)
        {
          *((T*)(buff + byteDistance)) |= (int)Src0(vecIdx);
        }
      }
      else if constexpr (Op == AtomicOp::XOR)
      {
        if (std::is_same_v<T, uint>)
        {
          *((T*)(buff + byteDistance)) ^= (uint)Src0(vecIdx);
        }
        else if (std::is_same_v<T, int>)
        {
          *((T*)(buff + byteDistance)) ^= (int)Src0(vecIdx);
        }
      }
    } /// elemIdx loop
  } /// offsetIdx loop

  return _Output;
}

template <AtomicOp Op,
          typename T,
          VectorSize VS,
          int N,
          unsigned MASK,
          EmuBufferType BFT>
CM_INLINE auto
cm_emu_atomic_binary_src(SurfaceIndex Idx,
                         vector<unsigned, N> Offset,
                         vector<T, N * details::lsc_vector_size<VS>()> Src0,
                         vector<T, N * details::lsc_vector_size<VS>()> Src1,
                         vector<ushort, N> Pred)
{
  // buffer-write base
  char * buff;
  int bufByteWidth = 0;

  if constexpr (BFT == EmuBufferType::UGM)
  {
    cm_list<CmEmulSys::iobuffer>::iterator buff_iter =
        CmEmulSys::search_buffer(Idx.get_data() & 0xFF);
    assert(buff_iter->height == 1);
    buff = (char*)buff_iter->p_volatile;
    bufByteWidth = buff_iter->width;
  }
  else if constexpr (BFT == EmuBufferType::SLM)
  {
    buff = __cm_emu_get_slm();
    bufByteWidth = __cm_emu_get_slm_size();
  }
  else
  {
    printf("%s:%d - Unsupported Emulation buffer type!!\n",
           __FUNCTION__, __LINE__);
    exit(EXIT_FAILURE);
  }

  // elemCount  : Number of vector elements to be loaded per Offset element
  constexpr uint elemCount = details::lsc_vector_size<VS>();

  vector<T, N * elemCount> _Output = 0;

  constexpr int sizeofT = sizeof(T);

  for (int offsetIdx = 0; offsetIdx < N; offsetIdx += 1)
  {
    if (Pred(offsetIdx) == 0)
    {
      // Skip Data vector elements correpsonding to
      // predicates whose value is zero
      continue;
    }

    if ((Offset(offsetIdx) & MASK) != 0)
    {
      printf("%s : %d - Alignment Error !!\n"                     \
             "Per-offset vector read element count = %d"          \
             " / OffsetIdx = %d / Offset = %d\n",
             __FUNCTION__, __LINE__,
             elemCount,
             offsetIdx, Offset(offsetIdx));
      exit(EXIT_FAILURE);
    }

    // byteDistance : byte-distance from buffer-read base
    int byteDistance = Offset(offsetIdx);

    for (int elemIdx = 0, vecIdx = offsetIdx;
         elemIdx < elemCount;
         elemIdx += 1, byteDistance += sizeofT, vecIdx += N)
    {
      if (byteDistance < 0
          || byteDistance > bufByteWidth)
      {
        // Out-of-bound
        continue;
      }

      _Output(vecIdx) = *((T*)(buff + byteDistance));
      // _Output(vecIdx) contains old value

      // new = (old == src0) ? src1 : old:
      if constexpr (Op == AtomicOp::ICAS)
      {
        if (_Output(vecIdx) == Src0(vecIdx))
        {
          *((T*)(buff + byteDistance)) = Src1(vecIdx);
        }
      } /// ICAS
      else if constexpr (Op == AtomicOp::FCAS)
      {
        if (is_fp_type<T>::value)
        {
          if (_Output(vecIdx) == Src0(vecIdx))
          {
            *((T*)(buff + byteDistance)) = Src1(vecIdx);
          }
        }
      } /// FCAS
    } // elemIdx Loop
  } // offsetIdx Loop

  return _Output;
}

/// \brief LSC Atomic.
///
/// @param T The element data type.
///
/// @param VS Vector size
///
/// @param DS Data size
///
/// @param L1H L1 cache hint
///
/// @param L3H L3 chache hint
///
/// @param Pred Predicate
///
/// @param Idx Surface index, which must correspond to a buffer.
///
/// @param Offset zero based byte offset of the input buffer or SLM byte offset
///

template <AtomicOp Op, typename T, VectorSize VS = VectorSize::N1,
          DataSize DS = DataSize::Default, CacheHint L1H = CacheHint::Default,
          CacheHint L3H = CacheHint::Default,
          unsigned N = details::lsc_default_simt()>
CM_INLINE auto
cm_atomic(SurfaceIndex Idx, vector<unsigned, N> Offset, vector<ushort, N> Pred = 1)
    -> typename std::enable_if<details::lsc_atomic_nsrcs<Op>() == 0,
                               decltype(vector<T, N * details::lsc_vector_size<VS>()>())>::type
{
  static_assert(details::lsc_check_simt<N>(), "unexpected number of channels");
  constexpr uint MASK = details::atomicAlignMask<Op, T, VS, DS, N, EmuBufferType::UGM>();

  std::unique_lock<std::mutex> lock(atomicMutex);
  return cm_emu_atomic_zero_src<Op, T, VS, N, MASK, EmuBufferType::UGM>(Idx,
                                                                        Offset, Pred);
}

template <AtomicOp Op, typename T, VectorSize VS = VectorSize::N1,
          DataSize DS = DataSize::Default, CacheHint L1H = CacheHint::Default,
          CacheHint L3H = CacheHint::Default,
          unsigned N = details::lsc_default_simt()>
CM_INLINE auto
cm_atomic(SurfaceIndex Idx, vector_ref<unsigned, N> Offset, vector<ushort, N> Pred = 1)
    -> typename std::enable_if<details::lsc_atomic_nsrcs<Op>() == 0,
                               decltype(vector<T, N * details::lsc_vector_size<VS>()>())>::type
{
  static_assert(details::lsc_check_simt<N>(), "unexpected number of channels");
  constexpr uint MASK = details::atomicAlignMask<Op, T, VS, DS, N, EmuBufferType::UGM>();

  std::unique_lock<std::mutex> lock(atomicMutex);
  return cm_emu_atomic_zero_src<Op, T, VS, N, MASK, EmuBufferType::UGM>(Idx,
                                                                        Offset, Pred);
}

#define CM_ATOMIC_SINGLE_SRC_TMPL(OFS_TYPE, SRC_TYPE)               \
  template <AtomicOp Op, typename T, VectorSize VS = VectorSize::N1,    \
            DataSize DS = DataSize::Default, CacheHint L1H = CacheHint::Default, \
            CacheHint L3H = CacheHint::Default,                         \
            unsigned N = details::lsc_default_simt()>                        \
  CM_INLINE auto                                                        \
  cm_atomic(SurfaceIndex Idx, OFS_TYPE<unsigned, N> Offset,               \
            SRC_TYPE<T, N * details::lsc_vector_size<VS>()> Src0,         \
            vector<ushort, N> Pred = 1)                                 \
      -> typename std::enable_if<details::lsc_atomic_nsrcs<Op>() == 1,  \
                                 decltype(vector<T, N * details::lsc_vector_size<VS>()>())>::type \
  {                                                                     \
    static_assert(details::lsc_check_simt<N>(), "unexpected number of channels");                  \
    constexpr uint MASK = details::atomicAlignMask<Op, T, VS, DS, N, EmuBufferType::UGM>(); \
    std::unique_lock<std::mutex> lock(atomicMutex);                     \
    return cm_emu_atomic_single_src<Op, T, VS, N, MASK, EmuBufferType::UGM>(Idx, Offset, Src0, Pred); \
  }                                                                     \

CM_ATOMIC_SINGLE_SRC_TMPL(vector, vector)
CM_ATOMIC_SINGLE_SRC_TMPL(vector, vector_ref)
CM_ATOMIC_SINGLE_SRC_TMPL(vector_ref, vector)
CM_ATOMIC_SINGLE_SRC_TMPL(vector_ref, vector_ref)

#undef CM_ATOMIC_SINGLE_SRC_TMPL

#define CM_ATOMIC_BINARY_SRC_TMPL(OFS_TYPE, SRC0_TYPE, SRC1_TYPE) \
  template <AtomicOp Op, typename T, VectorSize VS = VectorSize::N1,    \
    DataSize DS = DataSize::Default, CacheHint L1H = CacheHint::Default, \
            CacheHint L3H = CacheHint::Default,                         \
            unsigned N = details::lsc_default_simt()>                        \
  CM_INLINE auto                                                        \
  cm_atomic(SurfaceIndex Idx, OFS_TYPE<unsigned, N> Offset,             \
            SRC0_TYPE<T, N * details::lsc_vector_size<VS>()> Src0,      \
            SRC1_TYPE<T, N * details::lsc_vector_size<VS>()> Src1,      \
            vector<ushort, N> Pred = 1)                                 \
      -> typename std::enable_if<details::lsc_atomic_nsrcs<Op>() == 2,  \
                                 decltype(Src0)>::type                  \
  {                                                                     \
    static_assert(details::lsc_check_simt<N>(), "unexpected number of channels");                  \
    constexpr uint MASK = details::atomicAlignMask<Op, T, VS, DS, N, EmuBufferType::UGM>(); \
    std::unique_lock<std::mutex> lock(atomicMutex);                     \
    return cm_emu_atomic_binary_src<Op, T, VS, N, MASK, EmuBufferType::UGM>(Idx, Offset, Src0, Src1, Pred); \
  }                                                                     \

CM_ATOMIC_BINARY_SRC_TMPL(vector, vector, vector)
CM_ATOMIC_BINARY_SRC_TMPL(vector, vector, vector_ref)
CM_ATOMIC_BINARY_SRC_TMPL(vector, vector_ref, vector)
CM_ATOMIC_BINARY_SRC_TMPL(vector, vector_ref, vector_ref)
CM_ATOMIC_BINARY_SRC_TMPL(vector_ref, vector, vector)
CM_ATOMIC_BINARY_SRC_TMPL(vector_ref, vector, vector_ref)
CM_ATOMIC_BINARY_SRC_TMPL(vector_ref, vector_ref, vector)
CM_ATOMIC_BINARY_SRC_TMPL(vector_ref, vector_ref, vector_ref)

#undef CM_ATOMIC_BINARY_SRC_TMPL

template <AtomicOp Op, typename T, VectorSize VS = VectorSize::N1,
          DataSize DS = DataSize::Default, CacheHint L1H = CacheHint::Default,
          CacheHint L3H = CacheHint::Default,
          unsigned N = details::lsc_default_simt()>
CM_INLINE auto
cm_atomic_slm(vector<unsigned, N> Offset, vector<ushort, N> Pred = 1)
    -> typename std::enable_if<details::lsc_atomic_nsrcs<Op>() == 0,
                               decltype(vector<T, N * details::lsc_vector_size<VS>()>())>::type
{
  static_assert(details::lsc_check_simt<N>(), "unexpected number of channels");
  constexpr uint MASK = details::atomicAlignMask<Op, T, VS, DS, N, EmuBufferType::SLM>();

  std::unique_lock<std::mutex> lock(atomicMutex);
  return cm_emu_atomic_zero_src<Op, T, VS, N, MASK, EmuBufferType::SLM>(SLM_SURFACE_IDX,
                                                                        Offset, Pred);
}

template <AtomicOp Op, typename T, VectorSize VS = VectorSize::N1,
          DataSize DS = DataSize::Default, CacheHint L1H = CacheHint::Default,
          CacheHint L3H = CacheHint::Default,
          unsigned N = details::lsc_default_simt()>
CM_INLINE auto
cm_atomic_slm(vector_ref<unsigned, N> Offset, vector<ushort, N> Pred = 1)
    -> typename std::enable_if<details::lsc_atomic_nsrcs<Op>() == 0,
                               decltype(vector<T, N * details::lsc_vector_size<VS>()>())>::type
{
  static_assert(details::lsc_check_simt<N>(), "unexpected number of channels");
  constexpr uint MASK = details::atomicAlignMask<Op, T, VS, DS, N, EmuBufferType::SLM>();

  std::unique_lock<std::mutex> lock(atomicMutex);
  return cm_emu_atomic_zero_src<Op, T, VS, N, MASK, EmuBufferType::SLM>(SLM_SURFACE_IDX,
                                                                        Offset, Pred);
}

#define CM_ATOMIC_SLM_SINGLE_SRC_TMPL(OFS_TYPE, SRC_TYPE)               \
  template <AtomicOp Op, typename T, VectorSize VS = VectorSize::N1,    \
            DataSize DS = DataSize::Default, CacheHint L1H = CacheHint::Default, \
            CacheHint L3H = CacheHint::Default,                         \
            unsigned N = details::lsc_default_simt()>                        \
  CM_INLINE auto                                                        \
  cm_atomic_slm(OFS_TYPE<unsigned, N> Offset,                             \
                SRC_TYPE<T, N * details::lsc_vector_size<VS>()> Src0,     \
                vector<ushort, N> Pred = 1)                             \
      -> typename std::enable_if<details::lsc_atomic_nsrcs<Op>() == 1,  \
                                 decltype(vector<T, N * details::lsc_vector_size<VS>()>())>::type \
  {                                                                     \
    static_assert(details::lsc_check_simt<N>(), "unexpected number of channels");                  \
    constexpr uint MASK = details::atomicAlignMask<Op, T, VS, DS, N, EmuBufferType::SLM>(); \
    std::unique_lock<std::mutex> lock(atomicMutex);                     \
    return cm_emu_atomic_single_src<Op, T, VS, N, MASK, EmuBufferType::SLM>(SLM_SURFACE_IDX, \
                                                                            Offset, Src0, Pred); \
  }                                                                     \

CM_ATOMIC_SLM_SINGLE_SRC_TMPL(vector, vector)
CM_ATOMIC_SLM_SINGLE_SRC_TMPL(vector_ref, vector)
CM_ATOMIC_SLM_SINGLE_SRC_TMPL(vector, vector_ref)
CM_ATOMIC_SLM_SINGLE_SRC_TMPL(vector_ref, vector_ref)

#undef CM_ATOMIC_SLM_SINGLE_SRC_TMPL

#define CM_ATOMIC_SLM_BINARY_SRC_TMPL(OFS_TYPE, SRC0_TYPE, SRC1_TYPE)   \
  template <AtomicOp Op, typename T, VectorSize VS = VectorSize::N1,    \
    DataSize DS = DataSize::Default, CacheHint L1H = CacheHint::Default, \
            CacheHint L3H = CacheHint::Default,                         \
            unsigned N = details::lsc_default_simt()>                        \
  CM_INLINE auto                                                        \
  cm_atomic_slm(OFS_TYPE<unsigned, N> Offset,                           \
                SRC0_TYPE<T, N * details::lsc_vector_size<VS>()> Src0,  \
                SRC1_TYPE<T, N * details::lsc_vector_size<VS>()> Src1,  \
                vector<ushort, N> Pred = 1)                             \
      -> typename std::enable_if<details::lsc_atomic_nsrcs<Op>() == 2,  \
                                 decltype(Src0)>::type                  \
  {                                                                     \
    static_assert(details::lsc_check_simt<N>(), "unexpected number of channels");                 \
    constexpr uint MASK = details::atomicAlignMask<Op, T, VS, DS, N, EmuBufferType::SLM>(); \
    std::unique_lock<std::mutex> lock(atomicMutex);                     \
    return cm_emu_atomic_binary_src<Op, T, VS, N, MASK, EmuBufferType::SLM>(SLM_SURFACE_IDX, Offset, Src0, Src1, Pred); \
  }                                                                     \

CM_ATOMIC_SLM_BINARY_SRC_TMPL(vector, vector, vector)
CM_ATOMIC_SLM_BINARY_SRC_TMPL(vector, vector, vector_ref)
CM_ATOMIC_SLM_BINARY_SRC_TMPL(vector, vector_ref, vector)
CM_ATOMIC_SLM_BINARY_SRC_TMPL(vector, vector_ref, vector_ref)
CM_ATOMIC_SLM_BINARY_SRC_TMPL(vector_ref, vector, vector)
CM_ATOMIC_SLM_BINARY_SRC_TMPL(vector_ref, vector, vector_ref)
CM_ATOMIC_SLM_BINARY_SRC_TMPL(vector_ref, vector_ref, vector)
CM_ATOMIC_SLM_BINARY_SRC_TMPL(vector_ref, vector_ref, vector_ref)

#undef CM_ATOMIC_SLM_BINARY_SRC_TMPL

enum class LSC_SFID : uint8_t {
  LSC_UGM = 0,
  LSC_UGML = 1,
  LSC_TGM = 2,
  LSC_SLM = 3
};

enum class LSC_FENCE_OP : uint8_t {
  LSC_FENCE_OP_NONE = 0,
  LSC_FENCE_OP_EVICT = 1,
  LSC_FENCE_OP_INVALIDATE = 2,
  LSC_FENCE_OP_DISCARD = 3,
  LSC_FENCE_OP_CLEAN = 4,
  LSC_FENCE_OP_FLUSHL3 = 5
};

enum class LSC_SCOPE : uint8_t {
  LSC_SCOPE_GROUP = 0,
  LSC_SCOPE_LOCAL = 1,
  LSC_SCOPE_TILE = 2,
  LSC_SCOPE_GPU = 3,
  LSC_SCOPE_GPUS = 4,
  LSC_SCOPE_SYSTEM = 5,
  LSC_SCOPE_SYSACQ = 6
};

template <LSC_SFID Sfid,
          LSC_FENCE_OP FenceOp,
          LSC_SCOPE Scope,
          unsigned N = details::lsc_default_simt()>
CM_INLINE
void cm_emu_fence(vector<ushort, N> Pred = 1)
{
  if constexpr (Sfid == LSC_SFID::LSC_UGM)
  {
    for (auto it = CmEmulSys::iobuffers.begin();
         it != CmEmulSys::iobuffers.end(); ++it)
    {
      if (it->p_volatile != 0)
      {
         memcpy(it->p, it->p_volatile, it->width * it->height * it->depth);
      }
    }
  }
  else if constexpr (Sfid == LSC_SFID::LSC_SLM)
  {
     // nothing todo now
     // buff = __cm_emu_get_slm();
  }
  else
  {
    printf("%s:%d - Unsupported Emulation buffer type!!\n",
           __FUNCTION__, __LINE__);
    exit(EXIT_FAILURE);
  }
}

///
/// LSC Fence
///
/// \brief LSC Fence.
///
/// @param N The number of channels (platform dependent)
///
/// @param Sfid shaded funnction
///
/// @param FenceOp
///
/// @param Scope
///

template <LSC_SFID Sfid = LSC_SFID::LSC_UGM,
          LSC_FENCE_OP FenceOp = LSC_FENCE_OP::LSC_FENCE_OP_NONE,
          LSC_SCOPE Scope = LSC_SCOPE::LSC_SCOPE_GROUP,
          unsigned N = details::lsc_default_simt()>
CM_INLINE
void cm_fence(vector<ushort, N> Pred = 1) {
  static_assert(details::lsc_check_simt<N>(), "unexpected number of channels");
  cm_emu_fence<Sfid, FenceOp, Scope, N>(Pred);
}

//// Raw send support for LSC
template <typename T,
    uint N1,
    uint N = details::lsc_default_simt()>
    CM_INLINE
    auto cm_emu_ptr_load_core(T *Ptr,
        vector<unsigned, N> Offset,
        vector<ushort, N> Pred,
        vector_ref<T, N1> Output,
        uint elemCount,
        uint mask
    )
{
    // Read-in 'N' addresses, each of whose read size is 'VS'
    // elements.
    // 'DS' - 'Data size or format to read or store',
    // referenced for address alignment
    // Ignored : L1H, L3H

    // buffer-read base
    char * buff = (char*)Ptr;

    constexpr int sizeofT = sizeof(T);

    for (int offsetIdx = 0; offsetIdx < N; offsetIdx += 1)
    {
        if (Pred(offsetIdx) == 0)
        {
            // Skip _Output vector elements correpsonding to
            // predicates whose value is zero
            continue;
        }

        if ((Offset(offsetIdx) & mask) != 0)
        {
            printf("%s : %d - Alignment Error !!\n"                     \
                "Per-offset vector read element count = %d"          \
                " / OffsetIdx = %d / Offset = %d\n",
                __FUNCTION__, __LINE__,
                elemCount,
                offsetIdx, Offset(offsetIdx));
            exit(EXIT_FAILURE);
        }

        // byteDistance : byte-distance from buffer-read base
        int byteDistance = Offset(offsetIdx);

        for (int elemIdx = 0, vecIdx = offsetIdx;
            elemIdx < elemCount;
            elemIdx += 1, byteDistance += sizeofT, vecIdx += N)
        {
            if (byteDistance >= 0)
            {
                Output(vecIdx) = *((T*)(buff + byteDistance));
            }
        }
    }
}

template <typename T1, uint N1, uint N2,
          typename T2, uint N3, uint N4,
          typename T3, uint N5, uint N6,
          uint N,
          template<typename ElmTy, uint U, uint V> typename MatTy1,
          template<typename ElmTy, uint U, uint V> typename MatTy2,
          template<typename ElmTy, uint U, uint V> typename MatTy3>
CM_INLINE
void cm_raw_send_helper(MatTy1<T1, N1, N2> rspVar,
                        MatTy2<T2, N3, N4> msgVar,
                        MatTy3<T3, N5, N6> msgVar2,
                        uint exDesc, uint msgDesc,
                        uchar execSize, uchar sfid,
                        uchar numSrc0, uchar numSrc1, uchar numDst,
                        uchar isEOT = 0, uchar isSendc = 0, vector<ushort, N> mask = 1)
{
  auto op = details::getMsgOp(msgDesc);

  if (op == details::msgOp::DP_LOAD)
  {
      //static_assert(N1 == 1, "Wrong Reference for 2D-load!! (N1 != 1)");
      assert(sfid == 0xF); // UGM type only

      unsigned transpose = details::getMsgField((uint32_t)msgDesc,
          details::msgField::TRANSPOSE);
      unsigned address_size = details::getMsgField((uint32_t)msgDesc,
          details::msgField::ADDRSIZE);
      unsigned address_type = details::getMsgField((uint32_t)msgDesc,
          details::msgField::ADDRTYPE);
      unsigned data_size = details::getMsgField((uint32_t)msgDesc,
          details::msgField::DATASIZE);
      unsigned vect_size = details::getMsgField((uint32_t)msgDesc,
          details::msgField::VECTSIZE);
      uint64_t surfaceBase = 0;
      uint64_t Addr0 = 0;
      vector<unsigned, N> _offset = 0;
      vector<ushort, N> _pred = 1;
      if (transpose) {
          // vector load
          surfaceBase = details::getSurfaceBaseAddr(msgVar);
      }
      else {
          Addr0 = details::getLaneAddr(msgVar, 0);
          for (unsigned i = 1; i < N; i++) {
              unsigned addr = details::getLaneAddr(msgVar, i);
              if (addr < Addr0) {
                  Addr0 = addr;
              }
          }
          for (unsigned i = 0; i < N; i++) {
              unsigned addr = details::getLaneAddr(msgVar, i);
              _offset(i) = addr - Addr0;
          }
      }
      vector_ref<T1, N2> _RetRef = rspVar.template format<T1, 1, N2>();
      assert(vect_size < 9);
      unsigned NElts[] = { 1, 2, 3, 4, 8, 16, 32, 64 };

      uint32_t MASK = 0;
      if (vect_size > 1) {
          if (data_size == 2) {
              MASK = 0x3;
          }
          else if (data_size == 3) {
              MASK = 0x7;
          }
      }
      cm_emu_ptr_load_core((T1*)surfaceBase, _offset, _pred, _RetRef, NElts[vect_size], MASK);

  }
  else  if (op == details::msgOp::DP_STORE)
  {
      assert(sfid == 0xF); // UGM type only
      uint32_t transpose = details::getMsgField((uint32_t)msgDesc,
          details::msgField::TRANSPOSE);
      uint32_t address_size = details::getMsgField((uint32_t)msgDesc,
          details::msgField::ADDRSIZE);
      uint32_t address_type = details::getMsgField((uint32_t)msgDesc,
          details::msgField::ADDRTYPE);
      uint32_t data_size = details::getMsgField((uint32_t)msgDesc,
          details::msgField::DATASIZE);
      uint32_t vect_size = details::getMsgField((uint32_t)msgDesc,
          details::msgField::VECTSIZE);
      uint64_t surfaceBase = 0;
      uint64_t Addr0 = 0;
      vector<unsigned, N> _offset = 0;
      vector<ushort, N> _pred = 1;
      if (transpose) {
          // vector load
          surfaceBase = details::getSurfaceBaseAddr(msgVar);
      }
      else {
          Addr0 = details::getLaneAddr(msgVar, 0);
          for (uint32_t i = 1; i < N; i++) {
              uint32_t addr = details::getLaneAddr(msgVar, i);
              if (addr < Addr0) {
                  Addr0 = addr;
              }
          }
          for (uint32_t i = 0; i < N; i++) {
              uint32_t addr = details::getLaneAddr(msgVar, i);
              _offset(i) = addr - Addr0;
          }
      }
      vector_ref<T3, N6> _DataRef = msgVar2.template format<T3, 1, N6>();
      //static_assert(N5 * N6 == 0, "Wrong Reference for LD-store!! ( N5 * N6 != 0)");
      constexpr unsigned const_vect_size = N6 / N;
      uint32_t MASK = 0;
      if (vect_size > 1) {
          if (data_size == (uint32_t)(DataSize::U32)) {
              MASK = 0x3;
          }
          else if (data_size == (uint32_t)(DataSize::U64)) {
              MASK = 0x7;
          }
      }
      assert(vect_size < 9);
      unsigned NElts[] = { 1, 2, 3, 4, 8, 16, 32, 64 };
      cm_emu_ptr_store_core((T3*)surfaceBase, _offset, _DataRef, _pred, NElts[vect_size], MASK);
  }
  else
  {
    assert(0);
  }
}

// Handles :
// cm_load raw send
template <typename T1, uint N1, uint N2,
          typename T2, uint N3, uint N4,
          template<typename ElmTy, uint U, uint V> typename MsgTy1,
          template<typename ElmTy, uint U, uint V> typename MsgTy2,
          unsigned N = details::lsc_default_simt()>
CM_INLINE
void cm_raw_send(MsgTy1<T1, N1, N2> rspVar,
                 MsgTy2<T2, N3, N4> msgVar,
                 void *dummy,
                 uint exDesc, uint msgDesc,
                 uchar execSize, uchar sfid,
                 uchar numSrc0, uchar numSrc1, uchar numDst,
                 uchar isEOT = 0, uchar isSendc = 0, vector<ushort, N> mask = 1)
{
  assert(dummy == NULL);

  auto op = details::getMsgOp(msgDesc);

  // Argument sanity check. Add handled operation cases in following
  // assert accordingly
  assert(
          op == details::msgOp::DP_LOAD);

  matrix_ref<T1, N1, N2> dummy_ref = rspVar;
  cm_raw_send_helper(rspVar,
                     msgVar,
                     dummy_ref,
                     exDesc, msgDesc, execSize, sfid,
                     numSrc0, numSrc1, numDst,
                     isEOT, isSendc, mask);
}

// Handles :
// cm_store raw send
template <typename T2, uint N3, uint N4,
          typename T3, uint N5, uint N6,
          template<typename ElmTy, uint U, uint V> typename MsgTy2,
          template<typename ElmTy, uint U, uint V> typename MsgTy3,
          unsigned N = details::lsc_default_simt()>
CM_INLINE
void cm_raw_send(void *dummy,
                 MsgTy2<T2, N3, N4> msgVar,
                 MsgTy3<T3, N5, N6> msgVar2,
                 uint exDesc, uint msgDesc,
                 uchar execSize, uchar sfid,
                 uchar numSrc0, uchar numSrc1, uchar numDst,
                 uchar isEOT = 0, uchar isSendc = 0, vector<ushort, N> mask = 1)
{
  assert(dummy == NULL);

  auto op = details::getMsgOp(msgDesc);

  // Argument sanity check. Add handled operation cases in following
  // assert accordingly
  assert(
          op == details::msgOp::DP_STORE);

  matrix_ref<T3, N5, N6> dummy_ref = msgVar2;
  cm_raw_send_helper(dummy_ref,
                     msgVar,
                     msgVar2,
                     exDesc, msgDesc, execSize, sfid,
                     numSrc0, numSrc1, numDst,
                     isEOT, isSendc, mask);
}

// Handles :
// cm_prefetch
template <typename T2,
          uint N3, uint N4,
          template<typename ElmTy, uint U, uint V> typename MsgTy,
          unsigned N = details::lsc_default_simt()>
CM_INLINE
void cm_raw_send(void *dummy,
                 MsgTy<T2, N3, N4> msgVar,
                 void *dummy_2,
                 uint exDesc, uint msgDesc,
                 uchar execSize, uchar sfid,
                 uchar numSrc0, uchar numSrc1, uchar numDst,
                 uchar isEOT = 0, uchar isSendc = 0, vector<ushort, N> mask = 1)
{
  assert(dummy == NULL && dummy_2 == NULL);

  auto op = details::getMsgOp(msgDesc);

  // Argument sanity check. Add handled operation cases in following
  // assert accordingly
  assert(
          op == details::msgOp::DP_LOAD);

  if (
          op == details::msgOp::DP_LOAD)
  {
    return;
  }

  matrix_ref<unsigned, 1, 1> dummy_ref = msgVar.template format<unsigned, 1, 1>();
  matrix_ref<unsigned, 1, 1> dummy_ref2 = dummy_ref;
  cm_raw_send_helper(dummy_ref,
                     msgVar,
                     dummy_ref2,
                     exDesc, msgDesc, execSize, sfid,
                     numSrc0, numSrc1, numDst,
                     isEOT, isSendc, mask);
}

template <typename U1, unsigned N1, typename U2, unsigned N2, typename U3, unsigned N3, unsigned N = 16>
CM_INLINE
typename std::enable_if<sizeof(U1) * N1 % 32 == 0 &&
  sizeof(U2) * N2 % 32 == 0 &&
  sizeof(U3) * N3 % 32 == 0>::type
cm_raw_send(vector_ref<U1, N1> rspVar,
    vector<U2, N2> msgVar,
    vector<U3, N3> msgVar2,
    uint exDesc, uint msgDesc,
    uchar execSize,  uchar sfid,
    uchar numSrc0, uchar numSrc1, uchar numDst,
    uchar isEOT = 0, uchar isSendc = 0,
    vector<ushort, N> mask = 1)
{
  cm_raw_send_helper(rspVar.template format<U1, 1, N1>(),
                     msgVar.template format<U2, 1, N2>(),
                     msgVar2.template format<U3, 1, N3>(),
                     exDesc, msgDesc, execSize, sfid,
                     numSrc0, numSrc1, numDst,
                     isEOT, isSendc, mask);
}

template <typename U2, unsigned N2, typename U3, unsigned N3, unsigned N = 16,
          template<typename ElmTy, uint U> typename MsgTy2,
          template<typename ElmTy, uint U> typename MsgTy3>
CM_INLINE
typename std::enable_if<sizeof(U2) * N2 % 32 == 0 && sizeof(U3) * N3 % 32 == 0>::type
cm_raw_send(
    int dummyDst,
    MsgTy2<U2, N2> msgVar,
    MsgTy3<U3, N3> msgVar2,
    uint exDesc, uint msgDesc,
    uchar execSize, uchar sfid,
    uchar numSrc0, uchar numSrc1, uchar numDst,
    uchar isEOT = 0, uchar isSendc = 0,
    vector<ushort, N> mask = 1)
{
  auto op = details::getMsgOp(msgDesc);
  assert(
          op == details::msgOp::DP_STORE);

  cm_raw_send_helper(msgVar2.template format<U3, 1, N3>(),
                     msgVar.template format<U2, 1, N2>(),
                     msgVar2.template format<U3, 1, N3>(),
                     exDesc, msgDesc, execSize, sfid,
                     numSrc0, numSrc1, numDst,
                     isEOT, isSendc, mask);
}

template <typename U1, unsigned N1, typename U2, unsigned N2, unsigned N = 16,
          template<typename ElmTy, uint U> typename MsgTy1,
          template<typename ElmTy, uint U> typename MsgTy2>
CM_INLINE
typename std::enable_if<sizeof(U1) * N1 % 32 == 0 && sizeof(U2) * N2 % 32 == 0>::type
cm_raw_send(MsgTy1<U1, N1> rspVar,
    MsgTy2<U2, N2> msgVar,
    int dummySrc1,
    uint exDesc, uint msgDesc,
    uchar execSize, uchar sfid,
    uchar numSrc0, uchar numSrc1, uchar numDst,
    uchar isEOT = 0, uchar isSendc = 0,
    vector<ushort, N> mask = 1)
{
  auto op = details::getMsgOp(msgDesc);
  assert(
          op == details::msgOp::DP_LOAD);

  cm_raw_send_helper(rspVar.template format<U1, 1, N1>(),
                     msgVar.template format<U2, 1, N2>(),
                     msgVar.template format<U2, 1, N2>(),
                     exDesc, msgDesc, execSize, sfid,
                     numSrc0, numSrc1, numDst,
                     isEOT, isSendc, mask);
}

#endif // _CM_LSC_H_
