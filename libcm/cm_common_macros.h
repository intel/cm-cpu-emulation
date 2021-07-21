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

#pragma once

#include <iostream>
#include <type_traits>

#define S__LINE__3(num) #num
#define S__LINE__2(num) S__LINE__3(num)
#define S__LINE__       S__LINE__2(__LINE__)

#define CM_STATIC_WARNING__(cond, msg) { \
struct WarnMe{ \
    [[deprecated( msg )]] void DoWarn(std::false_type) { \
        static int once = 0; if(!once) { \
            std::cout << msg << std::endl; std::cerr << msg << std::endl; once++; }; \
    } \
    void DoWarn(std::true_type) {}; \
    WarnMe() {DoWarn(typename std::conditional<(cond),std::true_type,std::false_type>::type {});} \
} warn_me; };

#define CM_STATIC_WARNING(cond, msg) CM_STATIC_WARNING__(cond, "WARNING: " msg " at " __FILE__ ":" S__LINE__);
#define CM_STATIC_ERROR(C,M) static_assert((C), " *** Error: " M)

#ifndef NEW_CM_RT
#define NEW_CM_RT  // Defined for new CM Runtime APIs
#endif

#ifndef AUTO_CM_MODE_SET
    /// Defined these macros for MSVC and GCC.
#if !defined(CM_GENX)
    #define CM_GENX
#endif // !defined(CM_GENX)
    #define CM_EMU
    #define _GENX_MAIN_
    #define _GENX_
    #define _GENX_STACKCALL_
    #define _CM_INPUT_
    #define _CM_OUTPUT_
    #define _CM_INPUT_OUTPUT_
    #define _CM_OUTPUT_INPUT_
    #define _CM_CALLABLE_
#define AUTO_CM_MODE_SET
#endif /* AUTO_CM_MODE_SET */

#ifndef CM_NOINLINE
    #define CM_NOINLINE
#endif

#ifndef CM_INLINE
    #define CM_INLINE inline __attribute__((always_inline))
#endif

#ifndef CM_API
    #if defined(NEW_CM_RT) && defined(LIBCM_TEST_EXPORTS)
    #define CM_API __attribute__((visibility("default")))
    #elif defined(NEW_CM_RT)
    #define CM_API
    #else
    #define CM_API
    #endif /* CM_EMU */
#endif /* CM_API */

#ifndef CMRT_LIBCM_API
    #if defined(NEW_CM_RT) && defined(LIBCM_TEST_EXPORTS)
    #define CMRT_LIBCM_API __attribute__((visibility("default")))
    #elif defined(NEW_CM_RT)
    #define CMRT_LIBCM_API __attribute__((visibility("default")))
    #else
    #define CMRT_LIBCM_API
    #endif /* CM_EMU */
#endif /* CMRT_LIBCM_API */

#define CM_CHK_RESULT(cm_call)                                  \
do {                                                            \
    int result = cm_call;                                       \
    if (result != CM_SUCCESS) {                                 \
        GFX_EMU_ERROR_MESSAGE("Invalid CM call at %s:%d. Error %d: \n", \
        __FILE__, __LINE__, result);                            \
        exit(EXIT_FAILURE);                                     \
    }                                                           \
} while(false)

// For non-emu modes - when jit target is specified a CM_GENn define is created
// We rely on this in the front end in some header files for gen specific support
// Use the standard /D geni define to create this define for EMU mode as well
#if defined(gen7)
#define CM_GEN7
#elif defined(gen7_5)
#define CM_GEN7_5
#elif defined(gen8)
#define CM_GEN8
#elif defined(gen8_5)
#define CM_GEN8_5
#elif defined(gen9)
#define CM_GEN9
#endif
