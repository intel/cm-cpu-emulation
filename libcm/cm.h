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

#ifndef CM_H
#define CM_H

#include "cm_version_defs.h"

//Define MDF version for new feature
#ifndef __INTEL_MDF
#define __INTEL_MDF     (CM_6_0 + 1)
#endif

#ifndef __INTEL_CM
#define __INTEL_CM      (CM_6_0 + 1)
#endif

#define CM_V2
//#define CM_V1
//#define NEW_CM_RT

#include "cm_common_macros.h"

#if defined(CM_GEN5)    || \
    defined(CM_GEN6)    || \
    defined(CM_GEN7)    || \
    defined (CM_GEN7_5) || \
    defined(CM_GEN8)    || \
    defined(CM_GEN8_5)  || \
    defined(CM_GEN9)    || \
    defined(CM_JIT)
#if !defined(CM_GENX)
#define CM_GENX
#endif // !defined(CM_GENX)
#endif

 #define CM_execute_kernels CM_execute_kernels_emu
 #define CM_set_thread_count CM_set_thread_count_emu

 #define CM_register_buffer CM_register_buffer_emu
 #define CM_unregister_buffer CM_unregister_buffer_emu
 #define CM_modify_buffer CM_modify_buffer_emu

//__declspec(dllimport)

#ifndef CM_RT_EXPORTS
#include "half_type.h"
#endif

#include "libcm_def.h"
#include "cm_vm.h"
#include "cm_intrin.h"
#include "cm_internal.h"

// Dummy CM_SW_BARRIER as it is not supported under emulation
#define CM_SW_BARRIER

#ifdef CM_GENX
 #include "genx_dataport.h"
 #include "genx_threading.h"
 #include "genx_simdcontrolflow.h"
#endif /* CM_GENX */

#include "cm_slm_user.h"
#include "cm_printf_device.h"
#include "cm_color.h"
#endif /* CM_H */
