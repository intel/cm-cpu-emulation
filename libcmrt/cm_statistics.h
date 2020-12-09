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

#pragma once
#ifndef __CM_STATISTICS_H__
#define __CM_STATISTICS_H__

#include "cm_def.h"

class CmTask;

typedef struct __tag_CmRunnedKernels_t
{
    char            m_strName[CM_MAX_KERNEL_NAME_SIZE_IN_BYTE];
    unsigned int    m_nLen;
    unsigned int    m_nTimes;
    struct __tag_CmRunnedKernels_t *m_next;
}CmRunnedKernels_t;

class CmStatistics
{
public:
    static CmStatistics *Create( void );
    static CmStatistics *Get(void) { return m_pTracker; }
    static int Destroy(void);

    int TrackRunnedKernels( const CmTask * const pTask );

protected:
    CmStatistics(void);
    ~CmStatistics(void);

    static bool IsEnabled( void );

private:
    static CmStatistics *m_pTracker;
    CmRunnedKernels_t   *m_lsRunnedKernel;
};
#endif //__CM_STATISTICS_H__
