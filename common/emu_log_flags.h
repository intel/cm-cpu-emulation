/*===================== begin_copyright_notice ==================================

 Copyright (c) 2021, Intel Corporation


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

// -- See below for GFX_EMU_DEBUG_LEVEL-s.
GFX_EMU_DEBUG_MINIMAL_LEVEL(                          2) // Set for minimal level of debug.

// -- Debug channels ---
//GFX_EMU_DEBUG_FLAG(msg,constant,flag_shift,enabled by default)
GFX_EMU_DEBUG_FLAG("kernel launch", fKernelLaunch,    1, false)
GFX_EMU_DEBUG_FLAG("kernel support", fKernelSupport,  2, false)
GFX_EMU_DEBUG_FLAG("dbg symb",      fDbgSymb,         3, false)
GFX_EMU_DEBUG_FLAG("CM API",        fCmAPI,           4, false)
GFX_EMU_DEBUG_FLAG("L0 API",        fL0API,           5, false)
GFX_EMU_DEBUG_FLAG("OpenCL API",    fOpenClAPI,       6, false)
GFX_EMU_DEBUG_FLAG("libcm",         fLibCm,           7, false)
GFX_EMU_DEBUG_FLAG("cmrt",          fLibCmrt,         8, false)
GFX_EMU_DEBUG_FLAG("shim",          fShim,            9, false)
GFX_EMU_DEBUG_FLAG("cm intrin",     fCmIntrin,        10, false)
GFX_EMU_DEBUG_FLAG("config",        fCfg,             11, false)
GFX_EMU_DEBUG_FLAG("sched",         fSched,           12, false)

// Flag used to denote messages display despite user-set filtering.
GFX_EMU_DEBUG_FLAG("",         fSticky,          13, false /*this one is always enabled this param is ignored*/)

// -- Importance level --- NB: num must increase with importance semantics.
//GFX_EMU_DEBUG_LEVEL(msg,constant,num)
// --
GFX_EMU_DEBUG_LEVEL("extra",         fExtraDetail,    1)
GFX_EMU_DEBUG_LEVEL("detail",        fDetail,         2)
GFX_EMU_DEBUG_LEVEL("info",          fInfo,           3)
GFX_EMU_DEBUG_LEVEL("critical",      fCritical,       4)
