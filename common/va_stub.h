/*========================== begin_copyright_notice ============================

Copyright (C) 2021 Intel Corporation

SPDX-License-Identifier: MIT

============================= end_copyright_notice ===========================*/

#ifndef _VA_STUB_H_
#define _VA_STUB_H_

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef unsigned int VAGenericID;
typedef VAGenericID VASurfaceID;

typedef void* VADisplay;
typedef int VAStatus;
#define VA_STATUS_SUCCESS			0x00000000
#define VA_FOURCC(ch0, ch1, ch2, ch3) \
    ((unsigned long)(unsigned char) (ch0) | ((unsigned long)(unsigned char) (ch1) << 8) | \
    ((unsigned long)(unsigned char) (ch2) << 16) | ((unsigned long)(unsigned char) (ch3) << 24 ))

#define VA_FOURCC_NV12		0x3231564E
#define VA_FOURCC_NV21		0x3132564E
#define VA_FOURCC_AI44		0x34344149
#define VA_FOURCC_RGBA		0x41424752
#define VA_FOURCC_RGBX		0x58424752
#define VA_FOURCC_BGRA		0x41524742
#define VA_FOURCC_BGRX		0x58524742
#define VA_FOURCC_ARGB		0x42475241
#define VA_FOURCC_XRGB		0x42475258
#define VA_FOURCC_ABGR          0x52474241
#define VA_FOURCC_XBGR          0x52474258
#define VA_FOURCC_UYVY          0x59565955
#define VA_FOURCC_YUY2          0x32595559
#define VA_FOURCC_AYUV          0x56555941
#define VA_FOURCC_NV11          0x3131564e
#define VA_FOURCC_YV12          0x32315659
#define VA_FOURCC_P208          0x38303250
#define VA_FOURCC_I420          0x30323449
#define VA_FOURCC_YV24          0x34325659
#define VA_FOURCC_YV32          0x32335659
#define VA_FOURCC_Y800          0x30303859
#define VA_FOURCC_IMC3          0x33434D49
#define VA_FOURCC_411P          0x50313134
#define VA_FOURCC_411R          0x52313134
#define VA_FOURCC_422H          0x48323234
#define VA_FOURCC_422V          0x56323234
#define VA_FOURCC_444P          0x50343434
#define VA_FOURCC_RGBP          0x50424752
#define VA_FOURCC_BGRP          0x50524742
#define VA_FOURCC_RGB565        0x36314752
#define VA_FOURCC_BGR565        0x36314742
#define VA_FOURCC_Y210          0x30313259
#define VA_FOURCC_Y216          0x36313259
#define VA_FOURCC_Y410          0x30313459
#define VA_FOURCC_Y416          0x36313459
#define VA_FOURCC_YV16          0x36315659
#define VA_FOURCC_P010          0x30313050
#define VA_FOURCC_P016          0x36313050
#define VA_FOURCC_I010          0x30313049
#define VA_FOURCC_IYUV          0x56555949
#define VA_FOURCC_A2R10G10B10   0x30335241 /* VA_FOURCC('A','R','3','0') */
#define VA_FOURCC_A2B10G10R10   0x30334241 /* VA_FOURCC('A','B','3','0') */
#define VA_FOURCC_X2R10G10B10   0x30335258 /* VA_FOURCC('X','R','3','0') */
#define VA_FOURCC_X2B10G10R10   0x30334258 /* VA_FOURCC('X','B','3','0') */
#define VA_FOURCC_Y8            0x20203859
#define VA_FOURCC_Y16           0x20363159
#define VA_FOURCC_VYUY          0x59555956
#define VA_FOURCC_YVYU          0x55595659
#define VA_FOURCC_ARGB64        0x34475241
#define VA_FOURCC_ABGR64        0x34474241

#ifdef __cplusplus
}
#endif

#endif /* _VA_STUB_H_ */
