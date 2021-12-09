/*========================== begin_copyright_notice ============================

Copyright (C) 2017 Intel Corporation

SPDX-License-Identifier: MIT

============================= end_copyright_notice ===========================*/

#ifndef CMRTLIB_AGNOSTIC_SHARE_CM_MEM_H_
#define CMRTLIB_AGNOSTIC_SHARE_CM_MEM_H_

#include <cstring>
#include <new>
#include "emu_log.h"

#define CmSafeDeleteArray(_ptr) {if(_ptr) {delete[] (_ptr); (_ptr)=0;}}
#define CmSafeRelease(_ptr)     {if(_ptr) {delete (_ptr); (_ptr)=0;}}

/*****************************************************************************\
Inline Function:
    CmSafeMemCopy

Description:
    Exception Handler Memory Copy function
\*****************************************************************************/
inline void CmSafeMemCopy( void* dst, const void* src, const size_t bytes )
{
#ifdef GFX_EMU_DEBUG_ENABLED
    __try
#endif
    {
        memcpy( dst, src, bytes );
    }
#ifdef GFX_EMU_DEBUG_ENABLED
    // catch exceptions here so they are easily debugged
    __except(EXCEPTION_EXECUTE_HANDLER)
    {
        GFX_EMU_ASSERT(0);
    }
#endif
}

/*****************************************************************************\
Inline Function:
    CmSafeMemSet

Description:
    Memory set
\*****************************************************************************/
inline void CmSafeMemSet( void* dst, const int data, const size_t bytes )
{
#ifdef GFX_EMU_DEBUG_ENABLED
    __try
#endif
    {
        ::memset( dst, data, bytes );
    }
#ifdef GFX_EMU_DEBUG_ENABLED
    // catch exceptions here so they are easily debugged
    __except(EXCEPTION_EXECUTE_HANDLER)
    {
        GFX_EMU_ASSERT(0);
    }
#endif
}

inline void CmDwordMemSet( void* dst, const uint32_t data, const size_t bytes )
{
    uint32_t *ptr = reinterpret_cast<uint32_t*>( dst );
    uint32_t size = (uint32_t)(bytes >> 2); // divide by 4 byte to dword
    uint32_t *maxPtr = ptr + size;
    while(ptr < maxPtr)
        *ptr++ = data;
}

/*****************************************************************************\
Inline Function:
CmSafeMemCompare

Description:
Exception Handler Memory Compare function
\*****************************************************************************/
inline int CmSafeMemCompare(const void* dst, const void* src, const size_t bytes)
{
#ifdef GFX_EMU_DEBUG_ENABLED
    __try
#endif
    {
        return ::memcmp(dst, src, bytes);
    }
#ifdef GFX_EMU_DEBUG_ENABLED
    // catch exceptions here so they are easily debugged
    __except (EXCEPTION_EXECUTE_HANDLER)
    {
        GFX_EMU_ASSERT(0);
        return 0x7FFFFFFF;  //  An unreasonably large value indicating errors.
    }
#endif
}

#endif  // #ifndef CMRTLIB_AGNOSTIC_SHARE_CM_MEM_H_
