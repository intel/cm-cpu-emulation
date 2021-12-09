/*========================== begin_copyright_notice ============================

Copyright (C) 2017 Intel Corporation

SPDX-License-Identifier: MIT

============================= end_copyright_notice ===========================*/

// cm_include.h : include file for standard system include files,
// or project specific include files that are used frequently, but
// are changed infrequently
//

#ifndef CMRTLIB_WINDOWS_SHARE_CM_INCLUDE_H_
#define CMRTLIB_WINDOWS_SHARE_CM_INCLUDE_H_

#include <cstdint>
#include <cstddef>

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN             // Exclude rarely-used stuff from Windows headers
#endif

#ifdef CM_RT_EXPORTS
  #define CM_RT_API __declspec(dllexport)
#else
  #define CM_RT_API
#endif

#define STDCALL __stdcall

#define NO_VTABLE       __declspec(novtable)
#define ALIGN( size )   __declspec(align(size))

#ifndef CM_NOINLINE
    #define CM_NOINLINE __declspec(noinline)
#endif

#ifdef CM_DX9
#include <d3d9.h>
#include <dxva2api.h>
#elif defined(CM_DX11)
#include <d3d11.h>
#endif

#endif  // #ifndef CMRTLIB_WINDOWS_SHARE_CM_INCLUDE_H_
