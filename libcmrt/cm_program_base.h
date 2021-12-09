/*========================== begin_copyright_notice ============================

Copyright (C) 2017 Intel Corporation

SPDX-License-Identifier: MIT

============================= end_copyright_notice ===========================*/

#ifndef CMRTLIB____SHARE_CM_PROGRAM_BASE_H_
#define CMRTLIB____SHARE_CM_PROGRAM_BASE_H_

#include "cm_include.h"

//!
//! CM Program
//!
class CmProgram
{
public:
    virtual int32_t GetCommonISACode(void* & pCommonISACode, uint32_t & size) = 0;

    virtual ~CmProgram () = default;
};

#endif  // #ifndef CMRTLIB____SHARE_CM_PROGRAM_BASE_H_
