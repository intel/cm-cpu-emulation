/*========================== begin_copyright_notice ============================

Copyright (C) 2017 Intel Corporation

SPDX-License-Identifier: MIT

============================= end_copyright_notice ===========================*/

#pragma once

#define CM_EMU

#ifdef CM_NOINLINE
    #undef CM_NOINLINE
#endif
#define CM_NOINLINE

#if defined(_WIN32)
#define _NAME(...) #__VA_ARGS__, (void (__cdecl *)(void))__VA_ARGS__
#else
#define _NAME(...) #__VA_ARGS__, (const void *)(void (*)(void))__VA_ARGS__
#endif
