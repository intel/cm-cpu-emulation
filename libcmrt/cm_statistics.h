/*========================== begin_copyright_notice ============================

Copyright (C) 2017 Intel Corporation

SPDX-License-Identifier: MIT

============================= end_copyright_notice ===========================*/

//////////////////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////////////////
#pragma once
#ifndef __CM_STATISTICS_H__
#define __CM_STATISTICS_H__

/////////////////////////////////////////////////////////////////////////////////////////
#include "cm_def.h"

class CmTask;

////////////////////////////////////////////////////////////////////////////////////
typedef struct __tag_CmRunnedKernels_t
{
    char            m_strName[CM_MAX_KERNEL_NAME_SIZE_IN_BYTE];
    unsigned int    m_nLen;
    unsigned int    m_nTimes;
    struct __tag_CmRunnedKernels_t *m_next;
}CmRunnedKernels_t;

////////////////////////////////////////////////////////////////////////////////
class CmStatistics
{
public:
    static CmStatistics *Create( void );
    static CmStatistics *Get(void) { return m_pTracker; }
    static int Destroy(void);

    ////////////////////////////////////////////////////////////////////////
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
