/*===================== begin_copyright_notice ==================================

 Copyright (c) 2020, Intel Corporation


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

#ifndef CMRTLIB_AGNOSTIC_SHARE_CMDEBUG_H_
#define CMRTLIB_AGNOSTIC_SHARE_CMDEBUG_H_

#include <cstdio>
#include <iostream>
#include <string>

#include "cm_def_os.h"
#include "cm_def.h"

#define CM_DBG_FAIL_ON_NOT_IMPLEMENTED false

template <class ... T>
inline void CmPrintMessage(T ... args ) {
    std::printf(std::forward<T>(args)...);
}

template <class T>
inline void CmPrintMessage(T arg) {
    std::cout << arg << std::endl;
}

template <class ... T>
inline void CmDebugMessage(T ... args) {
#if defined(_DEBUG) || !defined(NDEBUG)
    CmPrintMessage(std::forward<T>(args)...);
#endif
}

template<class ... T>
inline void CmAssertMessage(bool cond, T ... args) {
#if defined(_DEBUG) || !defined(NDEBUG)
    if (cond) return;
    CmDebugMessage(std::forward<T>(args)...);
    CmAssert(false);
#endif
}

template <class S, class ... T>
inline void CmErrorMessage(S msg, T ... args) {
    CmPrintMessage(("*** Error " + std::string(msg)).c_str (), std::forward<T>(args)...);
}

template<class ... T>
[[ noreturn ]] inline void CmFailWithMessage(T ... args) {
    CmErrorMessage(std::forward<T>(args)...);
    std::exit(EXIT_FAILURE);
}

template <class ... T>
inline void CmReleaseMessage(T ... args) {
    CmPrintMessage(std::forward<T>(args)...);
}

inline CM_RETURN_CODE CmNotImplemented (std::string msg) {
    msg = "NOT IMPLEMENTED: " + std::string(msg);

#if CM_DBG_FAIL_ON_NOT_IMPLEMENTED
    CmFailWithMessage(msg);
#else
    CmDebugMessage(msg);
    return CM_NOT_IMPLEMENTED;
#endif
}

//*-----------------------------------------------------------------------------
//| Macro checks the COM Results
//*-----------------------------------------------------------------------------
#ifndef CHK_RET
#define CHK_RET(stmt)                                                           \
{                                                                               \
    result = (stmt);                                                            \
    if (result != CM_SUCCESS)                                                   \
    {                                                                           \
        CmPrintMessage("%s: hr check failed\n", __FUNCTION__);                  \
        CmAssert(0);                                                            \
        goto finish;                                                            \
    }                                                                           \
}
#endif // CHK_HR

#ifndef CHK_NULL
#define CHK_NULL(p)                                                           \
{                                                                               \
    if ( (p) == nullptr)                                                   \
    {                                                                           \
        CmPrintMessage("%s: nullptr check failed\n", __FUNCTION__);                  \
        CmAssert(0);                                                            \
        result = CM_NULL_POINTER;                                               \
        goto finish;                                                            \
    }                                                                           \
}
#endif

#ifndef CHK_NULL_RETURN
#define CHK_NULL_RETURN(p)                                                           \
{                                                                               \
    if ( (p) == nullptr)                                                   \
    {                                                                           \
        CmPrintMessage("%s: nullptr check failed\n", __FUNCTION__);                  \
        CmAssert(0);                                                            \
        return CM_NULL_POINTER;                                               \
    }                                                                           \
}
#endif

#ifndef CHK_FAILURE_RETURN
#define CHK_FAILURE_RETURN(ret)                                                           \
{                                                                               \
    if ( (ret) != CM_SUCCESS)                                                   \
    {                                                                           \
        CmPrintMessage("%s:%d: return check failed\n", __FUNCTION__, __LINE__);                  \
        return ret;                                                            \
    }                                                                           \
}
#endif

#endif  // #ifndef CMRTLIB_AGNOSTIC_SHARE_CMDEBUG_H_
