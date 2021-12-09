/*========================== begin_copyright_notice ============================

Copyright (C) 2017 Intel Corporation

SPDX-License-Identifier: MIT

============================= end_copyright_notice ===========================*/

#ifndef CMRTLIB_LINUX_SHARE_CM_DEF_OS_H_
#define CMRTLIB_LINUX_SHARE_CM_DEF_OS_H_

#include "cm_include.h"
#include "cm_common.h"

#include "cm_rt_def_os.h"

#ifndef ANDROID
#include <va/va.h>
#else
#include <va/va_android.h>
#define Display unsigned int
#endif

#include <cstring>
#include "pthread.h"
#include <malloc.h>

////////////////////////////////////////////////////////////////////////////////////
// MS-specific defines/typedefs, which are absent under Linux but still used
////////////////////////////////////////////////////////////////////////////////////
#define _aligned_malloc(size, alignment) memalign(alignment, size)
#define _aligned_free(ptr) free(ptr)
typedef uint8_t BOOLEAN, *PBOOLEAN;

////////////////////////////////////////////////////////////////////////////////////
//      Platform dependent macros (Start)
////////////////////////////////////////////////////////////////////////////////////
#define CM_STRCPY(dst, sizeInBytes, src)       strcpy(dst, src)
#define CM_STRNCPY(dst, sizeOfDst, src, count) strncpy(dst, src, count)
#define CM_STRCAT(dst, sizeOfDst, src)       strcat(dst, src)
#define CM_GETENV(dst, name) dst = getenv(name)
#define CM_GETENV_FREE(dst)
#define CM_FOPEN(pFile, filename, mode) pFile = fopen(filename, mode)

#ifdef __cplusplus
#   define EXTERN_C     extern "C"
#else
#   define EXTERN_C
#endif

#define SUCCEEDED(hr)   (hr == VA_STATUS_SUCCESS)
#define FAILED(hr)      (hr != VA_STATUS_SUCCESS)

////////////////////////////////////////////////////////////////////////////////////
//      Platform dependent macros (End)
////////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////////
//      Platform dependent definitions (Start)
////////////////////////////////////////////////////////////////////////////////////

#define VAExtModuleCMRT 2
#define CM_MAX_SURFACE2D_FORMAT_COUNT 47

// max resolution for surface 2D
#define CM_MAX_2D_SURF_WIDTH  16384
#define CM_MAX_2D_SURF_HEIGHT 16384

////////////////////////////////////////////////////////////////////////////////////
//      Platform dependent definitions (End)
////////////////////////////////////////////////////////////////////////////////////

typedef enum _REGISTRATION_OP
{
    REG_IGNORE          = 0,
    REG_REGISTER        = 1,
    REG_UNREGISTER      = 2,
    REG_REGISTER_INDEX  = 3     // Register surface for Cm
} REGISTRATION_OP;

class CSync
{
public:
    CSync() { pthread_mutex_init(&m_criticalSection, nullptr); }
    ~CSync() { pthread_mutex_destroy(&m_criticalSection); }
    void Acquire() {  pthread_mutex_lock(&m_criticalSection); }
    void Release() {pthread_mutex_unlock(&m_criticalSection); }

private:
    pthread_mutex_t m_criticalSection;
};

//The communication function for CM to call into UMD,  get function pointer by libVA::vaGetLibFunc()
typedef VAStatus (__cdecl *pvaCmExtSendReqMsg)(VADisplay dpy, void *moduleType,
                                             uint32_t *inputFunId,  void *inputData,  uint32_t *inputDataLen,
                         uint32_t *outputFunId, void *outputData, uint32_t *outputDataLen);

typedef struct _CM_CREATESURFACE2D_PARAM
{
    uint32_t    width;                     // [in] width of 2D texture in pixel
    uint32_t    height;                    // [in] height of 2D texture in pixel
    CM_SURFACE_FORMAT   format;             // [in] DXGI format of 2D texture
    union
    {
        uint32_t index2DinLookupTable;       // [in] surface 2d's index in look up table.
        uint32_t vaSurfaceID;              // [in] libva-surface 2d's index in media driver
    };
    VASurfaceID *vaSurface;                  // [in] Pointer to a Libva Surface.
    void        *cmSurface2DHandle;         // [out] pointer of CmSurface2D used in driver
    bool        isCmCreated;
    int32_t     returnValue;               // [out] the return value from driver
    bool        isLibvaCreated;            // [in] if the surface created via libva
    void        *vaDpy;                     // [in] VaDisplay used to free va sruface
}CM_CREATESURFACE2D_PARAM, *PCM_CREATESURFACE2D_PARAM;

//The communication function for CM to call into UMD,  get function pointer by libVA::vaGetLibFunc()
typedef VAStatus (__cdecl *pvaCmExtSendReqMsg)(
                            VADisplay dpy,
                            void *moduleType,
                            uint32_t *inputFunId,
                            void *inputData,
                            uint32_t *inputDataLen,
                            uint32_t *outputFunId,
                            void *outputData,
                            uint32_t *outputDataLen);

typedef VADisplay (*pfVAGetDisplayDRM) (int32_t fd);    //vaGetDisplayDRM from libva-drm.so

#ifndef CMRT_NOINLINE
#define CMRT_NOINLINE __attribute__((noinline))
#endif

typedef void *HMODULE;

#endif  // #ifndef CMRTLIB_LINUX_SHARE_CM_DEF_OS_H_
