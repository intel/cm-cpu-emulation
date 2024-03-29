/*========================== begin_copyright_notice ============================

Copyright (C) 2017 Intel Corporation

SPDX-License-Identifier: MIT

============================= end_copyright_notice ===========================*/

#pragma once

#include <mmintrin.h>
#include <xmmintrin.h>
#include <emmintrin.h>
#include <smmintrin.h>
#include "cm_mem.h"

enum CPU_INSTRUCTION_LEVEL
{
    CPU_INSTRUCTION_LEVEL_UNKNOWN,
    CPU_INSTRUCTION_LEVEL_MMX,
    CPU_INSTRUCTION_LEVEL_SSE,
    CPU_INSTRUCTION_LEVEL_SSE2,
    CPU_INSTRUCTION_LEVEL_SSE3,
    CPU_INSTRUCTION_LEVEL_SSE4,
    CPU_INSTRUCTION_LEVEL_SSE4_1,
    NUM_CPU_INSTRUCTION_LEVELS
};

typedef __m128              DQWORD;         // 128-bits,   16-bytes
typedef uint32_t            PREFETCH[8];    //             32-bytes
typedef uint32_t            CACHELINE[8];   //             32-bytes
typedef uint16_t            DHWORD[32];     // 512-bits,   64-bytes

inline bool IsPowerOfTwo(const size_t number);
inline void* Align(void* const ptr, const size_t alignment);
inline size_t GetAlignmentOffset(void* const ptr, const size_t alignSize);
inline bool IsAligned(void * ptr, const size_t alignSize);

inline void CmFastMemCopy(void* dst, const void* src, const size_t bytes);
inline void CmFastMemCopyFromWC(void* dst, const void* src, const size_t bytes, CPU_INSTRUCTION_LEVEL cpuInstructionLevel);

inline void Prefetch(const void* ptr);
inline void FastMemCopy_SSE2(void* dst, void* src, const size_t doubleQuadWords);

inline void FastMemCopy_SSE2_movntdq_movdqa(void* dst, void* src, const size_t doubleQuadWords);
inline void FastMemCopy_SSE2_movdqu_movdqa(void* dst, void* src, const size_t doubleQuadWords);
inline void FastMemCopy_SSE2_movntdq_movdqu(void* dst, const void* src, const size_t doubleQuadWords);

#if !defined (_WIN64) && !defined (__GNUC__)
inline void __fastcall FastBlockCopyFromUSWC_SSE4_1_movntdqa_movdqa(void* dst, const void* src);
inline void __fastcall FastBlockCopyFromUSWC_SSE4_1_movntdqa_movdqu(void* dst, const void* src);
#endif

/*****************************************************************************\
MACROS:
EMIT_R_MR
Example:  movntdqa xmm1, xmmword ptr [eax]

EMIT_R_MR_OFFSET
Example: movntdqa xmm1, xmmword ptr [eax + 0x10]

Description:
Used to encode SSE4.1 instructions with parametrs
\*****************************************************************************/
#define EMIT_R_MR(OPCODE, X, Y )   \
    OPCODE                         \
    __asm _emit (0x00 + X*8 + Y)

#define EMIT_R_MR_OFFSET(OPCODE, X, Y, OFFSET)  \
    OPCODE                                      \
    __asm _emit (0x80 + X*8 + Y)                \
    __asm _emit (OFFSET&0xFF)                   \
    __asm _emit ((OFFSET>>8)&0xFF)              \
    __asm _emit ((OFFSET>>16)&0xFF)             \
    __asm _emit ((OFFSET>>24)&0xFF)

/*****************************************************************************\
MACROS:
MOVNTDQA_OP
MOVNTDQA_R_MR
MOVNTDQA_R_MRB

Description:
Used to emit SSE4_1 movntdqa (streaming load) instructions
SRC - XMM Register, destination data is to be stored
DST - General Purpose Register containing source address
OFFSET - Offset to be added to the source address
\*****************************************************************************/
#define MOVNTDQA_OP     \
    _asm _emit 0x66     \
    _asm _emit 0x0F     \
    _asm _emit 0x38     \
    _asm _emit 0x2A

#define MOVNTDQA_R_MR(DST, SRC)                 \
    EMIT_R_MR(MOVNTDQA_OP, DST, SRC)

#define MOVNTDQA_R_MR_OFFSET(DST, SRC, OFFSET)  \
    EMIT_R_MR_OFFSET(MOVNTDQA_OP, DST, SRC, OFFSET)

#ifndef BIT
#define BIT( n )    ( 1 << (n) )
#endif

extern "C" void __cpuid(int cpuInfo[4], int infoType);

/*****************************************************************************\
MACROS:
REG_XXX

Description:
Define CPU General Purpose and XMM Register Indices
These MACROS are to be replaced with instrinics available with .NET 2008
\*****************************************************************************/
#define REG_EAX  0x00
#define REG_ECX  0x01
#define REG_EDX  0x02
#define REG_EBX  0x03
#define REG_ESP  0x04
#define REG_EBP  0x05
#define REG_ESI  0x06
#define REG_EDI  0x07
#define REG_XMM0 0x00
#define REG_XMM1 0x01
#define REG_XMM2 0x02
#define REG_XMM3 0x03
#define REG_XMM4 0x04
#define REG_XMM5 0x05
#define REG_XMM6 0x06
#define REG_XMM7 0x07

/*****************************************************************************\
Inline Function:
Prefetch

Description:
executes __asm prefetchnta
\*****************************************************************************/
inline void Prefetch(const void* ptr)
{
    _mm_prefetch((const char*)ptr, _MM_HINT_NTA);
}

inline bool TestSSE4_1(void)
{
    bool success = true;

#ifndef NO_EXCEPTION_HANDLING
    __try
    {
#endif //NO_EXCEPTION_HANDLING

#if !defined(_WIN64) && defined(_WIN32)
    //Execute SSE4.1 PTEST instruction
    __asm _emit 0x66
    __asm _emit 0x0f
    __asm _emit 0x38
    __asm _emit 0x17
    __asm _emit 0xc1
#elif defined (_WIN32) && defined (_In_)
    static __m128i m128Space = { 0 };
    __m128i* pMMSrc = &m128Space;
    __m128i  xmm0;
    xmm0 = _mm_stream_load_si128(pMMSrc);
#else
    success = false;
#endif // #ifndef _WIN64

#ifndef NO_EXCEPTION_HANDLING
    }
    __except (EXCEPTION_EXECUTE_HANDLER)
    {
        success = false;
    }
#endif // NO_EXCEPTION_HANDLING
    return success;
}

/*****************************************************************************\
Inline Function:
GetCPUID

Description:
Retrieves cpu information and capabilities supported

for 32 bit XP compiling with Visual 2003, the DDK does not support __cpuid intrinsic
for 64 bit the __cpuid intrinsic is not supported

Input:
int InfoType - type of information requested
Output:
int CPUInfo[4] - requested info
\*****************************************************************************/
inline void GetCPUID(int CPUInfo[4], int InfoType)
{
#ifndef NO_EXCEPTION_HANDLING
    __try
    {
#endif //NO_EXCEPTION_HANDLING

#if defined(_WIN32) && !defined(_WIN64)
    #ifdef _MSC_VER
        __asm mov eax, InfoType
        __asm cpuid
        __asm mov esi, CPUInfo
        __asm mov[esi], eax
        __asm mov[esi + 4], ebx
        __asm mov[esi + 8], ecx
        __asm mov[esi + 12], edx
    #endif
#elif defined(_WIN64)
        __cpuid(CPUInfo, InfoType);
#elif defined(__GNUC__)
#endif

#ifndef NO_EXCEPTION_HANDLING
    }
    __except (EXCEPTION_EXECUTE_HANDLER)
    {
        // cpuid failed!
        return;
    }
#endif  //NO_EXCEPTION_HANDLING
}

/*****************************************************************************\
Inline Function:
CmFastMemCopyFromWC
Description:
Intel C++ Compiler Memory Copy function for large amounts of data
Input:
dst - pointer to write-combined destination buffer
src - pointer to source buffer
bytes - number of bytes to copy
\*****************************************************************************/
inline void CmFastMemCopyFromWC(void* dst, const void* src, const size_t bytes, CPU_INSTRUCTION_LEVEL cpuInstructionLevel)
{
    if (cpuInstructionLevel >= CPU_INSTRUCTION_LEVEL_SSE4_1)
    {
        // Cache pointers to memory
        uint8_t *p_dst = (uint8_t*)dst;
        uint8_t *p_src = (uint8_t*)src;

        size_t count = bytes;

        if (count >= sizeof(DHWORD))
        {
            //Streaming Load must be 16-byte aligned but should
            //be 64-byte aligned for optimal performance
            const size_t doubleHexWordAlignBytes =
                GetAlignmentOffset(p_src, sizeof(DHWORD));

            // Copy portion of the source memory that is not aligned
            if (doubleHexWordAlignBytes)
            {
                CmSafeMemCopy(p_dst, p_src, doubleHexWordAlignBytes);

                p_dst += doubleHexWordAlignBytes;
                p_src += doubleHexWordAlignBytes;
                count -= doubleHexWordAlignBytes;
            }

            GFX_EMU_ASSERT(IsAligned(p_src, sizeof(DHWORD)) == true);

            // Get the number of bytes to be copied (rounded down to nearets DHWORD)
            const size_t DoubleHexWordsToCopy = count / sizeof(DHWORD);

            if (DoubleHexWordsToCopy)
            {
                // Determine if the destination address is aligned
                const bool isDstDoubleQuadWordAligned =
                    IsAligned(p_dst, sizeof(DQWORD));

#if defined(_WIN64)
                __m128i* pMMSrc = (__m128i*)(p_src);
                __m128i* pMMDest = reinterpret_cast<__m128i*>(p_dst);
                __m128i  xmm0, xmm1, xmm2, xmm3;
#endif
                if (isDstDoubleQuadWordAligned)
                {
                    for (size_t i = 0; i<DoubleHexWordsToCopy; i++)
                    {
#if defined(_WIN64)
                        xmm0 = _mm_stream_load_si128(pMMSrc);
                        xmm1 = _mm_stream_load_si128(pMMSrc + 1);
                        xmm2 = _mm_stream_load_si128(pMMSrc + 2);
                        xmm3 = _mm_stream_load_si128(pMMSrc + 3);
                        pMMSrc += 4;

                        _mm_store_si128(pMMDest, xmm0);
                        _mm_store_si128(pMMDest + 1, xmm1);
                        _mm_store_si128(pMMDest + 2, xmm2);
                        _mm_store_si128(pMMDest + 3, xmm3);
                        pMMDest += 4;
#elif defined(_WIN32)
                        FastBlockCopyFromUSWC_SSE4_1_movntdqa_movdqa(p_dst, p_src);
#endif

                        p_dst += sizeof(DHWORD);
                        p_src += sizeof(DHWORD);
                        count -= sizeof(DHWORD);
                    }
                }
                else
                {
                    for (size_t i = 0; i<DoubleHexWordsToCopy; i++)
                    {
#if defined(_WIN64)
                        xmm0 = _mm_stream_load_si128(pMMSrc);
                        xmm1 = _mm_stream_load_si128(pMMSrc + 1);
                        xmm2 = _mm_stream_load_si128(pMMSrc + 2);
                        xmm3 = _mm_stream_load_si128(pMMSrc + 3);
                        pMMSrc += 4;

                        _mm_storeu_si128(pMMDest, xmm0);
                        _mm_storeu_si128(pMMDest + 1, xmm1);
                        _mm_storeu_si128(pMMDest + 2, xmm2);
                        _mm_storeu_si128(pMMDest + 3, xmm3);
                        pMMDest += 4;
#elif defined(_WIN32)
                        FastBlockCopyFromUSWC_SSE4_1_movntdqa_movdqu(p_dst, p_src);
#endif
                        p_dst += sizeof(DHWORD);
                        p_src += sizeof(DHWORD);
                        count -= sizeof(DHWORD);
                    }
                }
            }
        }

        // Copy remaining uint8_t(s)
        if (count)
        {
            CmSafeMemCopy(p_dst, p_src, count);
        }
    }
    else
    {
        CmFastMemCopy(dst, src, bytes);
    }
}

/*****************************************************************************\
Inline Function:
GetCpuInstructionLevel

Description:
Returns the highest level of IA32 intruction extensions supported by the CPU
( i.e. SSE, SSE2, SSE4, etc )

Output:
CPU_INSTRUCTION_LEVEL - highest level of IA32 instruction extension(s) supported
by CPU
\*****************************************************************************/
inline CPU_INSTRUCTION_LEVEL GetCpuInstructionLevel(void)
{
    int CpuInfo[4];
    memset(CpuInfo, 0, 4 * sizeof(int));

    GetCPUID(CpuInfo, 1);

    CPU_INSTRUCTION_LEVEL CpuInstructionLevel = CPU_INSTRUCTION_LEVEL_UNKNOWN;
    if ((CpuInfo[2] & BIT(19)) && TestSSE4_1())
    {
        CpuInstructionLevel = CPU_INSTRUCTION_LEVEL_SSE4_1;
    }
    else if (CpuInfo[2] & BIT(1))
    {
        CpuInstructionLevel = CPU_INSTRUCTION_LEVEL_SSE3;
    }
    else if (CpuInfo[3] & BIT(26))
    {
        CpuInstructionLevel = CPU_INSTRUCTION_LEVEL_SSE2;
    }
    else if (CpuInfo[3] & BIT(25))
    {
        CpuInstructionLevel = CPU_INSTRUCTION_LEVEL_SSE;
    }
    else if (CpuInfo[3] & BIT(23))
    {
        CpuInstructionLevel = CPU_INSTRUCTION_LEVEL_MMX;
    }

    return CpuInstructionLevel;
}

/*****************************************************************************\
Inline Template Function:
Max

Description:
Returns the max of the two values
\*****************************************************************************/
__inline size_t Max(size_t var0, size_t var1)
{
    return (var0 >= var1) ? var0 : var1;
}
/*****************************************************************************\
Inline Function:
IsAligned

Description:
Determines if the given size is aligned to the given size
\*****************************************************************************/
inline bool IsAligned(void * ptr, const size_t alignSize)
{
    return (((size_t)ptr % alignSize) == 0);
}

/*****************************************************************************\
Inline Function:
IsPowerOfTwo

Description:
Determines if the given value is a power of two.
\*****************************************************************************/
inline bool IsPowerOfTwo(const size_t number)
{
    return ((number & (number - 1)) == 0);
}

/*****************************************************************************\
Inline Function:
GetAlignmentOffset

Description:
Returns the size in bytes needed to align the given size to the
given alignment size
\*****************************************************************************/
inline size_t GetAlignmentOffset(void* const ptr, const size_t alignSize)
{
    GFX_EMU_ASSERT(alignSize);

    uint32_t offset = 0;

    if (IsPowerOfTwo(alignSize))
    {   // can recast 'ptr' to uint32_t, since offset is uint32_t
        offset = uint32_t(uintptr_t(Align(ptr, alignSize)) - (uintptr_t)(ptr));
    }
    else
    {
        const uint32_t modulo = (uint32_t)(uintptr_t(ptr) % alignSize);

        if (modulo)
        {
            offset = (uint32_t)alignSize - modulo;
        }
    }

    return offset;
}

/*****************************************************************************\
Inline Function:
Align

Description:
Type-safe (power-2) alignment of a pointer.
\*****************************************************************************/
inline void* Align(void* const ptr, const size_t alignment)
{
    GFX_EMU_ASSERT(IsPowerOfTwo(alignment));

    return (void*)((((size_t)ptr) + alignment - 1) & ~(alignment - 1));
}

/*****************************************************************************\
Inline Function:
FastMemCopy_SSE2_movntdq_movdqa

Description:
Intel C++ Compiler Memory Copy function using Streaming SIMD Extensions 2

Input:
dst - 16-byte aligned pointer to destination buffer
src - 16-byte aligned pointer to source buffer
doubleQuadWords - number of DoubleQuadWords to copy
\*****************************************************************************/
inline void FastMemCopy_SSE2_movntdq_movdqa(
    void* dst,
    void* src,
    const size_t doubleQuadWords)
{
    GFX_EMU_ASSERT(IsAligned(dst, sizeof(DQWORD)));
    GFX_EMU_ASSERT(IsAligned(src, sizeof(DQWORD)));

    const size_t DoubleQuadWordsPerPrefetch = sizeof(PREFETCH) / sizeof(DQWORD);

    // Prefetch the src data
    Prefetch((uint8_t*)src);
    Prefetch((uint8_t*)src + sizeof(PREFETCH));

    // Convert to SSE2 registers
    __m128i* dst128i = (__m128i*)dst;
    __m128i* src128i = (__m128i*)src;

    size_t count = doubleQuadWords;

    // Copies a cacheline per loop iteration
    while (count >= DoubleQuadWordsPerPrefetch)
    {
        Prefetch((uint8_t*)src128i + 2 * sizeof(PREFETCH));

        count -= DoubleQuadWordsPerPrefetch;

        // Copy cacheline of data
        for (size_t i = 0; i < DoubleQuadWordsPerPrefetch; i++)
        {
            _mm_stream_si128(dst128i++,
                _mm_load_si128(src128i++));
        }
    }

    // Copy DQWORD if not cacheline multiple
    while (count--)
    {
        _mm_stream_si128(dst128i++,
            _mm_load_si128(src128i++));
    }
}

/*****************************************************************************\
Inline Function:
FastMemCopy_SSE2_movdqu_movdqa

Description:
Intel C++ Compiler Memory Copy function using Streaming SIMD Extensions 2

Input:
dst - pointer to destination buffer
src - 16-byte aligned pointer to source buffer
doubleQuadWords - number of DoubleQuadWords to copy
\*****************************************************************************/
inline void FastMemCopy_SSE2_movdqu_movdqa(
    void* dst,
    void* src,
    const size_t doubleQuadWords)
{
    GFX_EMU_ASSERT(IsAligned(src, sizeof(DQWORD)));

    const size_t DoubleQuadWordsPerPrefetch = sizeof(PREFETCH) / sizeof(DQWORD);

    // Prefetch the src data
    Prefetch((uint8_t*)src);
    Prefetch((uint8_t*)src + sizeof(PREFETCH));

    // Convert to SSE2 registers
    __m128i* dst128i = (__m128i*)dst;
    __m128i* src128i = (__m128i*)src;

    size_t count = doubleQuadWords;

    // Copies a cacheline per loop iteration
    while (count >= DoubleQuadWordsPerPrefetch)
    {
        Prefetch((uint8_t*)src128i + 2 * sizeof(PREFETCH));

        count -= DoubleQuadWordsPerPrefetch;

        // Copy cacheline of data
        for (size_t i = 0; i < DoubleQuadWordsPerPrefetch; i++)
        {
            _mm_storeu_si128(dst128i++,
                _mm_load_si128(src128i++));
        }
    }

    // Copy DQWORD if not cacheline multiple
    while (count--)
    {
        _mm_storeu_si128(dst128i++,
            _mm_load_si128(src128i++));
    }
}

/*****************************************************************************\
Inline Function:
FastMemCopy_SSE2_movntdq_movdqu

Description:
Intel C++ Compiler Memory Copy function using Streaming SIMD Extensions 2

Input:
dst - 16-byte aligned pointer to destination buffer
src - pointer to source buffer
doubleQuadWords - number of DoubleQuadWords to copy
\*****************************************************************************/
inline void FastMemCopy_SSE2_movntdq_movdqu(
    void* dst,
    const void* src,
    const size_t doubleQuadWords)
{
    GFX_EMU_ASSERT(IsAligned(dst, sizeof(DQWORD)));

    const size_t DoubleQuadWordsPerPrefetch = sizeof(PREFETCH) / sizeof(DQWORD);

    // Prefetch the src data
    Prefetch((uint8_t*)src);
    Prefetch((uint8_t*)src + sizeof(PREFETCH));

    // Convert to SSE2 registers
    __m128i* dst128i = (__m128i*)dst;
    __m128i* src128i = (__m128i*)src;

    size_t count = doubleQuadWords;

    // Copies a cacheline per loop iteration
    while (count >= DoubleQuadWordsPerPrefetch)
    {
        Prefetch((uint8_t*)src128i + 2 * sizeof(PREFETCH));

        count -= DoubleQuadWordsPerPrefetch;

        // Copy cacheline of data
        for (size_t i = 0; i < DoubleQuadWordsPerPrefetch; i++)
        {
            _mm_stream_si128(dst128i++,
                _mm_loadu_si128(src128i++));
        }
    }

    // Copy DQWORD if not cacheline multiple
    while (count--)
    {
        _mm_stream_si128(dst128i++,
            _mm_loadu_si128(src128i++));
    }
}

/*****************************************************************************\
Inline Function:
FastMemCopy_SSE2_movdqu_movdqu

Description:
Intel C++ Compiler Memory Copy function using Streaming SIMD Extensions 2

Input:
dst - pointer to destination buffer
src - pointer to source buffer
doubleQuadWords - number of DoubleQuadWords to copy
\*****************************************************************************/
inline void FastMemCopy_SSE2_movdqu_movdqu(
    void* dst,
    const void* src,
    const size_t doubleQuadWords)
{
    const size_t DoubleQuadWordsPerPrefetch = sizeof(PREFETCH) / sizeof(DQWORD);

    // Prefetch the src data
    Prefetch((uint8_t*)src);
    Prefetch((uint8_t*)src + sizeof(PREFETCH));

    // Convert to SSE2 registers
    __m128i* dst128i = (__m128i*)dst;
    __m128i* src128i = (__m128i*)src;

    size_t count = doubleQuadWords;

    // Copies a cacheline per loop iteration
    while (count >= DoubleQuadWordsPerPrefetch)
    {
        Prefetch((uint8_t*)src128i + 2 * sizeof(PREFETCH));

        count -= DoubleQuadWordsPerPrefetch;

        // Copy cacheline of data
        for (size_t i = 0; i < DoubleQuadWordsPerPrefetch; i++)
        {
            _mm_storeu_si128(dst128i++,
                _mm_loadu_si128(src128i++));
        }
    }

    // Copy DQWORD if not cacheline multiple
    while (count--)
    {
        _mm_storeu_si128(dst128i++,
            _mm_loadu_si128(src128i++));
    }
}

/*****************************************************************************\
Inline Function:
FastMemCopy_SSE2

Description:
Intel C++ Compiler Memory Copy function using Streaming SIMD Extensions 2

Input:
dst - pointer to destination buffer
src - pointer to source buffer
doubleQuadWords - number of DoubleQuadWords to copy
\*****************************************************************************/
inline void FastMemCopy_SSE2(
    void* dst,
    void* src,
    const size_t doubleQuadWords)
{
    // Determine if the source and destination addresses are 128-bit aligned
    const bool isDstDoubleQuadWordAligned = IsAligned(dst, sizeof(DQWORD));
    const bool isSrcDoubleQuadWordAligned = IsAligned(src, sizeof(DQWORD));

    if (isSrcDoubleQuadWordAligned && isDstDoubleQuadWordAligned)
    {
        FastMemCopy_SSE2_movntdq_movdqa(dst, src, doubleQuadWords);
    }
    else if (isDstDoubleQuadWordAligned)
    {
        FastMemCopy_SSE2_movntdq_movdqu(dst, src, doubleQuadWords);
    }
    else if (isSrcDoubleQuadWordAligned)
    {
        FastMemCopy_SSE2_movdqu_movdqa(dst, src, doubleQuadWords);
    }
    else // if( !isSrcDoubleQuadWordAligned && !isDstDoubleQuadWordAligned )
    {
        FastMemCopy_SSE2_movdqu_movdqu(dst, src, doubleQuadWords);
    }
}

/*****************************************************************************\
Inline Function:
CmFastMemCopy

Description:
Intel C++ Compiler Memory Copy function for large amounts of data

Input:
dst - pointer to destination buffer
src - pointer to source buffer
bytes - number of bytes to copy
\*****************************************************************************/
inline void CmFastMemCopy(void* dst, const   void* src, const size_t bytes)
{

    // Cache pointers to memory
    uint8_t *p_dst = (uint8_t*)dst;
    uint8_t *p_src = (uint8_t*)src;

    size_t count = bytes;

    // Get the number of DQWORDs to be copied
    const size_t doubleQuadWords = count / sizeof(DQWORD);

    if (doubleQuadWords)
    {
        FastMemCopy_SSE2(p_dst, p_src, doubleQuadWords);

        p_dst += doubleQuadWords * sizeof(DQWORD);
        p_src += doubleQuadWords * sizeof(DQWORD);
        count -= doubleQuadWords * sizeof(DQWORD);
    }

    // Copy remaining uint8_t(s)
    if (count)
    {
        CmSafeMemCopy(p_dst, p_src, count);
    }
}

/*****************************************************************************\
Inline Function:
FastBlockCopyFromUSWC_SSE4_1_movntdqa_movdqa

Description: Fast copy from USWC memory to cacheable system memory

Input:
dst - 16-byte aligned pointer to (cacheable) destination buffer
src - 16-byte(req)/64-byte(optimal) aligned pointer to (USWC) source buffer
\*****************************************************************************/
#if !defined (_WIN64) && !defined (__GNUC__)
inline void __fastcall FastBlockCopyFromUSWC_SSE4_1_movntdqa_movdqa(void* dst, const void* src)
{
    __asm
    {
        ; Store the orginal source start address
        mov edx, src

        ; Store the dest address
        mov ecx, dst

        align 16

        ; Load data from source buffer
        ; Streaming loads from the same cache line should be grouped together
        ; and not be interleaved with : a) Writes or non - streaming loads or
        ; b) Streaming loads from other cache lines(strided accesses)

        ; movntdqa xmm0, xmmword ptr[edx]
        MOVNTDQA_R_MR(REG_XMM0, REG_EDX)

        ; movntdqa xmm1, xmmword ptr[edx + 16]
        MOVNTDQA_R_MR_OFFSET(REG_XMM1, REG_EDX, 16)

        ; movntdqa xmm2, xmmword ptr[edx + 32]
        MOVNTDQA_R_MR_OFFSET(REG_XMM2, REG_EDX, 32)

        ; movntdqa xmm3, xmmword ptr[edx + 48]
        MOVNTDQA_R_MR_OFFSET(REG_XMM3, REG_EDX, 48)

        ; Save data in destination buffer.
        movdqa xmmword ptr[ecx], xmm0
        movdqa xmmword ptr[ecx + 16], xmm1
        movdqa xmmword ptr[ecx + 32], xmm2
        movdqa xmmword ptr[ecx + 48], xmm3
    }
} // FastMemCopy_SSE4_1_movntdqa_movdqa()
#endif //!defined (_WIN64)

  /*****************************************************************************\
  Inline Function:
  FastBlockCopyFromUSWC_SSE4_1_movntdqa_movdqu

  Description: Fast copy from USWC memory (DHWORD in size) to cacheable system memory

  Input:
  dst - 16-byte (unaligned) pointer to (cacheable) destination buffer
  src - 16-byte(req)/64-byte(optimal) aligned pointer to (USWC) source buffer
  \*****************************************************************************/
#if !defined(_WIN64) && defined(_WIN32)
inline void  __fastcall FastBlockCopyFromUSWC_SSE4_1_movntdqa_movdqu(void* dst, const void* src)
{
    __asm
    {
        ; Store the orginal source start address
        mov edx, src

        ; Store the dest address
        mov ecx, dst

        align 16

        ; Load data from source buffer
        ; Streaming loads from the same cache line should be grouped together
        ; and not be interleaved with : a) Writes or non - streaming loads or
        ; b) Streaming loads from other cache lines(strided accesses)

        ; movntdqa xmm0, xmmword ptr[edx]
        MOVNTDQA_R_MR(REG_XMM0, REG_EDX)

        ; movntdqa xmm1, xmmword ptr[edx + 16]
        MOVNTDQA_R_MR_OFFSET(REG_XMM1, REG_EDX, 16)

        ; movntdqa xmm2, xmmword ptr[edx + 32]
        MOVNTDQA_R_MR_OFFSET(REG_XMM2, REG_EDX, 32)

        ; movntdqa xmm3, xmmword ptr[edx + 48]
        MOVNTDQA_R_MR_OFFSET(REG_XMM3, REG_EDX, 48)

        ; Copy data in destination buffer.
        movdqu xmmword ptr[ecx], xmm0
        movdqu xmmword ptr[ecx + 16], xmm1
        movdqu xmmword ptr[ecx + 32], xmm2
        movdqu xmmword ptr[ecx + 48], xmm3
    }
} // FastMemCopy_SSE4_1_movntdqa_movdqu()
#endif // !defined (_WIN64)
