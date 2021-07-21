/*===================== begin_copyright_notice ==================================

 Copyright (c) 2021, Intel Corporation


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

#ifndef GUARD_common_type_cond_end_operator_code_h
#define GUARD_common_type_cond_end_operator_code_h

typedef enum _CM_CONDITIONAL_END_OPERATOR_CODE {
    MAD_GREATER_THAN_IDD = 0,
    MAD_GREATER_THAN_OR_EQUAL_IDD,
    MAD_LESS_THAN_IDD,
    MAD_LESS_THAN_OR_EQUAL_IDD,
    MAD_EQUAL_IDD,
    MAD_NOT_EQUAL_IDD
} CM_CONDITIONAL_END_OPERATOR_CODE;

#endif // GUARD_common_type_cond_end_operator_code_h
