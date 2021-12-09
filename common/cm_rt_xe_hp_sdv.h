/*========================== begin_copyright_notice ============================

Copyright (C) 2019 Intel Corporation

SPDX-License-Identifier: MIT

============================= end_copyright_notice ===========================*/

//!
//! \file      cm_rt_xe_hp_sdv.h
//! \brief     Contains Definitions for CM on XE_HP_SDV
//!

#ifndef __CM_RT_XE_HP_SDV_H__
#define __CM_RT_XE_HP_SDV_H__

#define XE_HP_SDV_L3_PLANE_DEFAULT    CM_L3_PLANE_DEFAULT
#define XE_HP_SDV_L3_PLANE_1          CM_L3_PLANE_1
#define XE_HP_SDV_L3_PLANE_2          CM_L3_PLANE_2
#define XE_HP_SDV_L3_PLANE_3          CM_L3_PLANE_3
#define XE_HP_SDV_L3_PLANE_4          CM_L3_PLANE_4
#define XE_HP_SDV_L3_PLANE_5          CM_L3_PLANE_5
#define XE_HP_SDV_L3_PLANE_6          CM_L3_PLANE_6
#define XE_HP_SDV_L3_CONFIG_COUNT     7

// 4KB per Way for XE_HP_SDV, two Way per section
static const L3ConfigRegisterValues XE_HP_SDV_L3_PLANES[XE_HP_SDV_L3_CONFIG_COUNT] =
{                                    //  Rest  DC  RO   Z    Color  UTC  CB  Sum (in KB)
    {0xA0000000, 0, 0, 0},           //  320   0   0    0    0      0    0   320
    {0x70000000, 0x30000000, 0, 0},  //  224   0   0    0    0      96   0   320
    {0x0040A000, 0x58000000, 0, 0},  //  0     64  80   0    0      176  0   320
    {0x80000000, 0x00000100, 0, 0},  //  256   0   0    0    0      0    64  320
    {0x40000000, 0x60000000, 0, 0},  //  128   0   0    0    0      192  0   320
    {0x00810000, 0x00000100, 0, 0},  //  0     128 128  0    0      0    64  320
    {0x40000000, 0x40000100, 0, 0},  //  128   0   0    0    0      128  64  320
};

#endif //__CM_RT_XE_HP_SDV_H__
