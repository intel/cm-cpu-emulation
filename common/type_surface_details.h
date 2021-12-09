/*========================== begin_copyright_notice ============================

Copyright (C) 2020 Intel Corporation

SPDX-License-Identifier: MIT

============================= end_copyright_notice ===========================*/

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
