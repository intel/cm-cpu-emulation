/*========================== begin_copyright_notice ============================

Copyright (C) 2020 Intel Corporation

SPDX-License-Identifier: MIT

============================= end_copyright_notice ===========================*/

#ifndef GUARD_common_type_large_integer_h
#define GUARD_common_type_large_integer_h

union LARGE_INTEGER
{
    struct
    {
        unsigned int LowPart;
        int HighPart;
    } u;
    long long int QuadPart;
};

#endif // GUARD_common_type_large_integer_h
