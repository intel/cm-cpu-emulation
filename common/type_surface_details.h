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

#ifndef GUARD_common_type_surface_details_h
#define GUARD_common_type_surface_details_h

//|GT-PIN
struct CM_SURFACE_DETAILS
{
    uint32_t width;            //width of surface
    uint32_t height;           //height of surface, 0 if surface is CmBuffer
    uint32_t depth;            //depth of surface, 0 if surface is CmBuffer or CmSurface2D
    CM_SURFACE_FORMAT format;  //format of surface, if surface is CmBuffer
    uint32_t planeIndex;       //plane Index for this BTI, 0 if surface is not planar surface
    uint32_t pitch;            //pitch of surface, 0 if surface is CmBuffer
    uint32_t slicePitch;       // pitch of a slice in CmSurface3D, 0 if surface is CmBuffer or CmSurface2D
    uint32_t surfaceBaseAddress;
    uint8_t tiledSurface;
    uint8_t tileWalk;
    uint32_t xOffset;
    uint32_t yOffset;
};
#endif // GUARD_common_type_surface_details_h
