/*========================== begin_copyright_notice ============================

Copyright (C) 2017 Intel Corporation

SPDX-License-Identifier: MIT

============================= end_copyright_notice ===========================*/

#pragma once

#define PRINT_HEADER_SIZE 32

#define CM_PRINT_OBJECT_TYPE_ENTRY_INDEX 0
#define CM_PRINT_DATA___TYPE_ENTRY_INDEX 1
#define CM_PRINT_LOWER32BITS_ENTRY_INDEX 6
#define CM_PRINT_UPPER32BITS_ENTRY_INDEX 7

#define CM_PRINT_DATA_TYPE_CHAR   0
#define CM_PRINT_DATA_TYPE_UCHAR  1
#define CM_PRINT_DATA_TYPE_FLOAT  2
#define CM_PRINT_DATA_TYPE_INT    3
#define CM_PRINT_DATA_TYPE_UINT   4
#define CM_PRINT_DATA_TYPE_SHORT  5
#define CM_PRINT_DATA_TYPE_USHORT 6
#define CM_PRINT_DATA_TYPE_QWORD  7
#define CM_PRINT_DATA_TYPE_UQWORD 8
#define CM_PRINT_DATA_TYPE_DOUBLE 9

#define CM_PRINT_OBJECT_TYPE_UNKNOWN 0
#define CM_PRINT_OBJECT_TYPE_MATRIX  1
#define CM_PRINT_OBJECT_TYPE_VECTOR  2
#define CM_PRINT_OBJECT_TYPE_SCALAR  3
#define CM_PRINT_OBJECT_TYPE_STRING  4
#define CM_PRINT_OBJECT_TYPE_FORMAT  5

/// If you want to change the static
/// buffer change these two macros.
#define CM_PRINTF_STATIC_BUFFER_ID 1
#define CM_PRINT_BUFFER CM_STATIC_BUFFER_1
