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

#ifndef __TYPE_DATAPORT_H_INCLUDED__
#define __TYPE_DATAPORT_H_INCLUDED__

typedef enum _CmBufferType_
{
    DPE_INPUT_BUFFER         = 2,
    DPE_OUTPUT_BUFFER        = 3,
    GEN4_INPUT_BUFFER        = 4,
    GEN4_OUTPUT_BUFFER       = 5,
    GEN4_INPUT_OUTPUT_BUFFER = 6,
    GEN4_BUFFER              = 8,
    GEN4_SAMPLING_BUFFER     = 9
} CmBufferType;

typedef enum _CmSurfaceFormatID
{
    R32G32B32A32_FLOAT                  = 0x000,
    R32G32B32A32_SINT                   = 0x001,
    R32G32B32A32_UINT                   = 0x002,
    R32G32B32A32_UNORM                  = 0x003,
    R32G32B32A32_SNORM                  = 0x004,
    R64G64_FLOAT                        = 0x005,
    R32G32B32X32_FLOAT                  = 0x006,
    R32G32B32A32_SSCALED                = 0x007,
    R32G32B32A32_USCALED                = 0x008,
    PLANAR_420_8                        = 0x00D, //13
    R32G32B32_FLOAT                     = 0x040,
    R32G32B32_SINT                      = 0x041,
    R32G32B32_UINT                      = 0x042,
    R32G32B32_UNORM                     = 0x043,
    R32G32B32_SNORM                     = 0x044,
    R32G32B32_SSCALED                   = 0x045,
    R32G32B32_USCALED                   = 0x046,
    R16G16B16A16_UNORM                  = 0x080,
    R16G16B16A16_SNORM                  = 0x081,
    R16G16B16A16_SINT                   = 0x082,
    R16G16B16A16_UINT                   = 0x083,
    R16G16B16A16_FLOAT                  = 0x084,
    R32G32_FLOAT                        = 0x085,
    R32G32_SINT                         = 0x086,
    R32G32_UINT                         = 0x087,
    R32_FLOAT_X8X24_TYPELESS            = 0x088,
    X32_TYPELESS_G8X24_UINT             = 0x089,
    L32A32_FLOAT                        = 0x08a,
    R32G32_UNORM                        = 0x08b,
    R32G32_SNORM                        = 0x08c,
    R64_FLOAT                           = 0x08d,
    R16G16B16X16_UNORM                  = 0x08e,
    R16G16B16X16_FLOAT                  = 0x08f,
    A32X32_FLOAT                        = 0x090,
    L32X32_FLOAT                        = 0x091,
    I32X32_FLOAT                        = 0x092,
    R16G16B16A16_SSCALED                = 0x093,
    R16G16B16A16_USCALED                = 0x094,
    R32G32_SSCALED                      = 0x095,
    R32G32_USCALED                      = 0x096,
    B8G8R8A8_UNORM                      = 0x0c0,
    B8G8R8A8_UNORM_SRGB                 = 0x0c1,
    R10G10B10A2_UNORM                   = 0x0c2,
    R10G10B10A2_UNORM_SRGB              = 0x0c3,
    R10G10B10A2_UINT                    = 0x0c4,
    R10G10B10_SNORM_A2_UNORM            = 0x0c5,
    R10G10B10_SINT_A2_UINT              = 0x0c6,
    R8G8B8A8_UNORM                      = 0x0c7,
    R8G8B8A8_UNORM_SRGB                 = 0x0c8,
    R8G8B8A8_SNORM                      = 0x0c9,
    R8G8B8A8_SINT                       = 0x0ca,
    R8G8B8A8_UINT                       = 0x0cb,
    R16G16_UNORM                        = 0x0cc,
    R16G16_SNORM                        = 0x0cd,
    R16G16_SINT                         = 0x0ce,
    R16G16_UINT                         = 0x0cf,
    R16G16_FLOAT                        = 0x0d0,
    B10G10R10A2_UNORM                   = 0x0d1,
    B10G10R10A2_UNORM_SRGB              = 0x0d2,
    R11G11B10_FLOAT                     = 0x0d3,
    R32_SINT                            = 0x0d6,
    R32_UINT                            = 0x0d7,
    R32_FLOAT                           = 0x0d8,
    R24_UNORM_X8_TYPELESS               = 0x0d9,
    X24_TYPELESS_G8_UINT                = 0x0da,
    //R24X8_UNORM                       = 0x0db,
    L16A16_UNORM                        = 0x0df,
    I24X8_UNORM                         = 0x0e0,
    L24X8_UNORM                         = 0x0e1,
    A24X8_UNORM                         = 0x0e2,
    I32_FLOAT                           = 0x0e3,
    L32_FLOAT                           = 0x0e4,
    A32_FLOAT                           = 0x0e5,
    B8G8R8X8_UNORM                      = 0x0e9,
    B8G8R8X8_UNORM_SRGB                 = 0x0ea,
    R8G8B8X8_UNORM                      = 0x0eb,
    R8G8B8X8_UNORM_SRGB                 = 0x0ec,
    R9G9B9E5_SHAREDEXP                  = 0x0ed,
    B10G10R10X2_UNORM                   = 0x0ee,
    L16A16_FLOAT                        = 0x0f0,
    R32_UNORM                           = 0x0f1,
    R32_SNORM                           = 0x0f2,
    //B8G8R8A8_SNORM                    = 0x0f3,
    R10G10B10X2_USCALED                 = 0x0f3,
    R8G8B8A8_SSCALED                    = 0x0f4,
    R8G8B8A8_USCALED                    = 0x0f5,
    R16G16_SSCALED                      = 0x0f6,
    R16G16_USCALED                      = 0x0f7,
    R32_SSCALED                         = 0x0f8,
    R32_USCALED                         = 0x0f9,
    R8G8B8A8_UNORM_YUV                  = 0x0fd,
    R8G8B8A8_UNORM_SNCK                 = 0x0fe,
    B5G6R5_UNORM                        = 0x100,
    B5G6R5_UNORM_SRGB                   = 0x101,
    B5G5R5A1_UNORM                      = 0x102,
    B5G5R5A1_UNORM_SRGB                 = 0x103,
    B4G4R4A4_UNORM                      = 0x104,
    B4G4R4A4_UNORM_SRGB                 = 0x105,
    R8G8_UNORM                          = 0x106,
    R8G8_SNORM                          = 0x107,
    R8G8_SINT                           = 0x108,
    R8G8_UINT                           = 0x109,
    R16_UNORM                           = 0x10a,
    R16_SNORM                           = 0x10b,
    R16_SINT                            = 0x10c,
    R16_UINT                            = 0x10d,
    R16_FLOAT                           = 0x10e,
    I16_UNORM                           = 0x111,
    L16_UNORM                           = 0x112,
    A16_UNORM                           = 0x113,
    L8A8_UNORM                          = 0x114,
    I16_FLOAT                           = 0x115,
    L16_FLOAT                           = 0x116,
    A16_FLOAT                           = 0x117,
    R5G5_SNORM_B6_UNORM                 = 0x119,
    B5G5R5X1_UNORM                      = 0x11a,
    B5G5R5X1_UNORM_SRGB                 = 0x11b,
    R8G8_SSCALED                        = 0x11c,
    R8G8_USCALED                        = 0x11d,
    R16_SSCALED                         = 0x11e,
    R16_USCALED                         = 0x11f,
    R8_UNORM                            = 0x140,
    R8_SNORM                            = 0x141,
    R8_SINT                             = 0x142,
    R8_UINT                             = 0x143,
    A8_UNORM                            = 0x144,
    I8_UNORM                            = 0x145,
    L8_UNORM                            = 0x146,
    P4A4_UNORM                          = 0x147,
    A4P4_UNORM                          = 0x148,
    R8_SSCALED                          = 0x149,
    R8_USCALED                          = 0x14A,
    R1_UNORM                            = 0x181,
    R1_UINT                             = 0x181,
    YCRCB_NORMAL                        = 0x182,
    R8G8_B8G8_UNORM                     = 0x182,
    YCRCB_SWAPUVY                       = 0x183,
    G8R8_G8B8_UNORM                     = 0x183,
    BC1_UNORM_DXT1                      = 0x186,
    BC2_UNORM_DXT2_3                    = 0x187,
    BC3_UNORM_DXT4_5                    = 0x188,
    BC4_UNORM_DXN1                      = 0x189,
    BC5_UNORM_DXN2                      = 0x18a,
    BC1_UNORM_SRGB_DXT1_SRGB            = 0x18b,
    BC2_UNORM_SRGB_DXT2_3_SRGB          = 0x18c,
    BC3_UNORM_SRGB_DXT4_5_SRGB          = 0x18d,
    MONO8                               = 0x18e,
    YCRCB_SWAPUV                        = 0x18f,
    YCRCB_SWAPY                         = 0x190,
    DXT1_RGB                            = 0x191,
    FXT1                                = 0x192,
    R8G8B8_UNORM                        = 0x193,
    R8G8B8_SNORM                        = 0x194,
    //R8G8B8_UINT                       = 0x195,
    //R8G8B8_SINT                       = 0x196,
    R8G8B8_SSCALED                      = 0x195,
    R8G8B8_USCALED                      = 0x196,
    R64G64B64A64_FLOAT                  = 0x197,
    R64G64B64_FLOAT                     = 0x198,
    BC4_SNORM                           = 0x199,
    BC5_SNORM                           = 0x19a,
    //BC5_SNORM_NONLINEAR               = 0x19b,
    R16G16B16_FLOAT                     = 0x19b,
    R16G16B16_UNORM                     = 0x19c,
    R16G16B16_SNORM                     = 0x19d,
    R16G16B16_SSCALED                   = 0x19e,
    R16G16B16_USCALED                   = 0x19f,
    //R16G16B16_SINT                    = 0x19e, // REV4 remove
    //R16G16B16_UINT                    = 0x19f, // REV4 remove
    INVALID_SURF_FORMAT                 = 0xfff
} CmSurfaceFormatID;

CM_API void cm_fence();
CM_API void cm_fence(unsigned char bit_mask);

namespace CmEmulSys{
struct iobuffer {
    int id;
    CmBufferType bclass;
    CmSurfaceFormatID pixelFormat;
    void *p;    /* i/o datas */
    void *p_volatile;
    int width;
    int height;
    int depth;
    int pitch;
};
CMRT_LIBCM_API extern cm_list<iobuffer> iobuffers;
CM_API cm_list<CmEmulSys::iobuffer>::iterator search_buffer(int id);
extern cm_list<CmEmulSys::iobuffer>::iterator search_buffer(void *src, CmBufferType bclass);
} // namespace CmEmuSys

/*
 * Returns bytes per pixel value for specified surface format ID.
 */
inline uint CM_genx_bytes_per_pixel(CmSurfaceFormatID pixelFormat)
{
    switch(pixelFormat >> 6) {
        case 0:
            return 16;
        case 1:
            return 12;
        case 2:
            return 8;
        case 3:
            return 4;
        case 4:
            return 2;
        case 5:
            return 1;
        default:
            if( pixelFormat == YCRCB_NORMAL ||
                pixelFormat == YCRCB_SWAPY)
            {
                return 2;
            }
            return 4;
    }
}

#endif // __TYPE_DATAPORT_H_INCLUDED__

