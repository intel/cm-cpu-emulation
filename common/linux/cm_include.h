/*========================== begin_copyright_notice ============================

Copyright (C) 2017 Intel Corporation

SPDX-License-Identifier: MIT

============================= end_copyright_notice ===========================*/

// cm_include.h : include file for standard system include files,
// or project specific include files that are used frequently, but
// are changed infrequently
//

#ifndef CMRTLIB_LINUX_SHARE_CM_INCLUDE_H_
#define CMRTLIB_LINUX_SHARE_CM_INCLUDE_H_

#include <va_stub.h>
#include <dlfcn.h>

#include "type_large_integer.h"

#define sprintf_s snprintf

#ifdef CM_RT_EXPORTS
#define CM_RT_API __attribute__((visibility("default")))
#else
#define CM_RT_API
#endif

#ifndef CM_NOINLINE
  #define CM_NOINLINE __attribute__((noinline))
#endif

#define __cdecl
#define __stdcall  __attribute__((__stdcall__))

#ifdef __try
#undef __try
#endif
#define __try try

#ifdef __except
#undef __except
#endif
// clang-format off
#define __except(e)  catch(e)
// clang-format on

#define  EXCEPTION_EXECUTE_HANDLER std::exception const& e

#if NO_EXCEPTION_HANDLING || ANDROID
    #define try         if (true)
    #define catch(x)    if (false)
    #define throw(...)
#endif

typedef int HANDLE;

extern "C" int32_t QueryPerformanceFrequency(LARGE_INTEGER *frequency);
extern "C" int32_t QueryPerformanceCounter(LARGE_INTEGER *performanceCount);

#endif  // #ifndef CMRTLIB_LINUX_SHARE_CM_INCLUDE_H_
