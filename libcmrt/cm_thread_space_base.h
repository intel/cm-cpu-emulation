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
