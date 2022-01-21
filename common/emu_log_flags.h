/*========================== begin_copyright_notice ============================

Copyright (C) 2017 Intel Corporation

SPDX-License-Identifier: MIT

============================= end_copyright_notice ===========================*/

// -- See below for GFX_EMU_DEBUG_LEVEL-s.
GFX_EMU_DEBUG_MINIMAL_LEVEL(                          3) // Set for minimal level of debug.

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
GFX_EMU_DEBUG_FLAG("warn",          fWarn,            13, false)

// -- Importance level --- NB: num must increase with importance semantics.
//GFX_EMU_DEBUG_LEVEL(msg,constant,num)
// --
GFX_EMU_DEBUG_LEVEL("extra",         fExtraDetail,    1)
GFX_EMU_DEBUG_LEVEL("detail",        fDetail,         2)
GFX_EMU_DEBUG_LEVEL("info",          fInfo,           3)
