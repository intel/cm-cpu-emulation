/*========================== begin_copyright_notice ============================

Copyright (C) 2020 Intel Corporation

SPDX-License-Identifier: MIT

============================= end_copyright_notice ===========================*/

#ifndef GUARD_common_type_type_mw_group_select_h
#define GUARD_common_type_type_mw_group_select_h
//| CM MEDIA_OBJECT_WALKER GroupID Select
typedef enum _CM_MW_GROUP_SELECT
{
    CM_MW_GROUP_NONE        = 0,
    CM_MW_GROUP_COLORLOOP   = 1,
    CM_MW_GROUP_INNERLOCAL  = 2,
    CM_MW_GROUP_MIDLOCAL    = 3,
    CM_MW_GROUP_OUTERLOCAL  = 4,
    CM_MW_GROUP_INNERGLOBAL = 5,
} CM_MW_GROUP_SELECT;

#endif // GUARD_common_type_type_mw_group_select_h
