/*========================== begin_copyright_notice ============================

Copyright (C) 2017 Intel Corporation

SPDX-License-Identifier: MIT

============================= end_copyright_notice ===========================*/

#ifndef CMRTLIB____SHARE_CM_THREAD_SPACE_BASE_H_
#define CMRTLIB____SHARE_CM_THREAD_SPACE_BASE_H_

#include "cm_include.h"

class CmKernel;

#define CM_NUM_DWORD_FOR_MW_PARAM 16
#define CM_MAX_DEPENDENCY_COUNT 8
#define CM_INVALID_COLOR_COUNT 0
#define CM_26ZI_BLOCK_WIDTH 16
#define CM_26ZI_BLOCK_HEIGHT 8

enum CM_TS_FLAG
{
    WHITE = 0,
    GRAY  = 1,
    BLACK = 2
};

#include "type_dependency_pattern.h"
#include "type_walking_pattern.h"
#include "type_26zi_dispatch_pattern.h"
#include "type_walking_patterns.h"
#include "type_dependency.h"
#include "type_coord.h"
#include "type_thread_param.h"

typedef struct _CM_THREAD_SPACE_UNIT {
    void    *pKernel;
    uint32_t threadId;
    int32_t numEdges; //For Emulation mode
    CM_COORDINATE   scoreboardCoordinates;
    uint32_t dependencyMask;
    uint32_t scoreboardColor;
    uint32_t sliceDestinationSelect;
    uint32_t subSliceDestinationSelect;
} CM_THREAD_SPACE_UNIT;

#include "type_mw_group_select.h"
#include "type_thread_space_base.h"

#endif  // #ifndef CMRTLIB____SHARE_CM_THREAD_SPACE_BASE_H_
