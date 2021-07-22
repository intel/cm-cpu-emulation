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

#define CM_EMU

#include <assert.h>

#include "cm_lib.h"
#include "genx_lib.h"
#include "cm_list.h"

#include "emu_cfg.h"
#include "emu_platform.h"

using namespace std;

#define CM_GETENV(dst, name) dst = getenv(name)

#define GLOBAL_SURFACE_INDEX_NUMBER 4
 const int maxbsize = 255;
CMRT_LIBCM_API cm_list<CmEmulSys::iobuffer> CmEmulSys::iobuffers; /* List of allocated (loaded) genx i/o buffers */

CMRT_LIBCM_API SurfaceIndex* globalSurfaceIndex[4];
ushort thread_origin_x = 0;
ushort thread_origin_y = 0;
ushort color           = 0;

/* This function searches data i/o buffer by its ID */
extern cm_list<CmEmulSys::iobuffer>::iterator
CM_API CmEmulSys::search_buffer(int id)
{
    cm_list<CmEmulSys::iobuffer>::iterator it;

    CmEmulSys::enter_dataport_cs();
    for (it = CmEmulSys::iobuffers.begin(); it != CmEmulSys::iobuffers.end(); ++it) {
        if (it->id == id) {
            break;
        }
    }
    CmEmulSys::leave_dataport_cs();
    return it;
}

/* This function searches data i/o buffer by its source and bclass */
extern cm_list<CmEmulSys::iobuffer>::iterator
CmEmulSys::search_buffer(void *src, CmBufferType bclass)
{
    cm_list<CmEmulSys::iobuffer>::iterator it;

    CmEmulSys::enter_dataport_cs();
    for (it = CmEmulSys::iobuffers.begin(); it != CmEmulSys::iobuffers.end(); ++it) {
        if ((it->p == src) && (it->bclass == bclass)) {
            break;
        }
    }
    CmEmulSys::leave_dataport_cs();
    return it;
}

CM_API void initialize_global_surface_index()
{
    for (int i = 0; i < GLOBAL_SURFACE_INDEX_NUMBER; i ++)
        globalSurfaceIndex[i] = NULL;

    return;
}

CM_API void set_global_surface_index(int index, SurfaceIndex *si)
{
    globalSurfaceIndex[index] = si;
}

CM_API SurfaceIndex * get_global_surface_index(int index)
{
    return globalSurfaceIndex[index];
}

CM_API void set_thread_origin_x(ushort x)
{
    thread_origin_x = x;
}

CM_API void set_thread_origin_y(ushort y)
{
    thread_origin_y = y;
}

CM_API void set_color(ushort c)
{
    color = c;
}

CM_API ushort get_color()
{
    return color;
}

CM_API ushort get_thread_origin_x()
{
    if(GfxEmu::Cfg ().Platform.getInt () >= GfxEmu::Platform::XEHP_SDV)
        return cm_group_id(0);
    return thread_origin_x;
}

CM_API ushort get_thread_origin_y()
{
    if(GfxEmu::Cfg ().Platform.getInt () >= GfxEmu::Platform::XEHP_SDV)
        return cm_group_id(1);
    return thread_origin_y;
}

/* This function releases allocated genx i/o buffer */
extern void
CM_unregister_buffer_emu(int buf_id)
{
    CmEmulSys::enter_dataport_cs();
    cm_list<CmEmulSys::iobuffer>::iterator buff_iter = CmEmulSys::search_buffer(buf_id);
    if (buff_iter != CmEmulSys::iobuffers.end()) {
        // Let's consider the pre/post execution copy as legacy
        // and comment it out for now.

        //if(buff_iter->bclass == GEN4_INPUT_OUTPUT_BUFFER) {
        //    memcpy(buff_iter->p, buff_iter->p_volatile, (buff_iter->width)*(buff_iter->height)*(buff_iter->depth));
        //    free(buff_iter->p_volatile);
        //}
        CmEmulSys::iobuffers.remove(buff_iter);
    }
    CmEmulSys::leave_dataport_cs();
}

extern void
CM_register_buffer_emu(int buf_id, CmBufferType bclass, void *src, uint width, uint height,
                       CmSurfaceFormatID surfFormat, uint depth, uint pitch)
{
    cm_list<CmEmulSys::iobuffer>::iterator buff_iter;
    assert((buf_id < maxbsize) && ((int)CmEmulSys::iobuffers.size() < maxbsize));

    CmEmulSys::enter_dataport_cs();

    CM_unregister_buffer_emu(buf_id);
    buff_iter = CmEmulSys::search_buffer(src, bclass);
    if (buff_iter != CmEmulSys::iobuffers.end()) {
        if(buff_iter->id != buf_id) {
            GFX_EMU_ERROR_MESSAGE("the registration of buffer %d conflicts with the registration of buffer %d!\n", buf_id, buff_iter->id);
            exit(EXIT_FAILURE);
        }
    }

    CmEmulSys::iobuffer new_buff;

    new_buff.id = buf_id;
    new_buff.bclass = bclass;
    new_buff.pixelFormat = surfFormat;
    new_buff.p = src;
    new_buff.width = width;
    new_buff.height = height;
    new_buff.depth = depth;
    new_buff.pitch = pitch;

    // 3D surface is only used for sampler, which can only have
    // INPUT type buffer. Therefore no need to consider depth
    // in the buffer size calculation "width*height".
    if(bclass == GEN4_INPUT_OUTPUT_BUFFER) {
        // Let's consider the pre/post execution copy as legacy
        // and comment it out for now.

        //new_buff.p_volatile = (unsigned char*) malloc(width*height*depth);
        //if(new_buff.p_volatile != NULL)
        //{
        //    memcpy(new_buff.p_volatile, src, width*height*depth);
        //}
        new_buff.p_volatile = src;
    } else {
        new_buff.p_volatile = NULL;
    }

    CmEmulSys::iobuffers.add(new_buff);
    CmEmulSys::leave_dataport_cs();
}

extern void
CM_register_buffer_emu(int buf_id, CmBufferType bclass, void *src, uint width)
{
    CM_register_buffer_emu(buf_id,bclass,src,width,1,R8G8B8A8_UINT,1,0);
}

extern void
CM_modify_buffer_emu(int buf_id, CmBufferDescField field, int value)
{

    CmEmulSys::enter_dataport_cs();
    cm_list<CmEmulSys::iobuffer>::iterator buff_iter = CmEmulSys::search_buffer(buf_id);
    if (buff_iter != CmEmulSys::iobuffers.end()) {
        switch (field)
        {
        case GEN4_FIELD_SURFACE_FORMAT:
            buff_iter->pixelFormat = (CmSurfaceFormatID)value;
            break;
        case GEN4_FIELD_SURFACE_TYPE:
        case GEN4_FIELD_TILE_FORMAT:
        case GEN4_FIELD_SURFACE_PITCH:
        case GEN4_FIELD_SURFACE_SIZE:
        case GEN4_FIELD_SAMPLING_MIN_MODE:
        case GEN4_FIELD_SAMPLING_MIP_MODE:
        case GEN4_FIELD_SAMPLING_MAP_MODE:
            break;
            default:
                 break;
        }
    }
    CmEmulSys::leave_dataport_cs();
}

/* This function releases allocated genx i/o buffer */
CM_API void
CM_unregister_buffer_emu(SurfaceIndex buf_id, bool copy)
{
    CmEmulSys::enter_dataport_cs();
    cm_list<CmEmulSys::iobuffer>::iterator buff_iter = CmEmulSys::search_buffer(buf_id.get_data());
    if (buff_iter != CmEmulSys::iobuffers.end()) {
        if(buff_iter->bclass == GEN4_INPUT_OUTPUT_BUFFER) {
            // Let's consider the pre/post execution copy as legacy
            // and comment it out for now.

            //if(copy)
            //{
            //    memcpy(buff_iter->p, buff_iter->p_volatile, (buff_iter->width)*(buff_iter->height)*(buff_iter->depth));
            //}
            //free(buff_iter->p_volatile);
        }
        CmEmulSys::iobuffers.remove(buff_iter);
    }
    CmEmulSys::leave_dataport_cs();
}

CM_API void
CM_register_buffer_emu(SurfaceIndex buf_id, CmBufferType bclass, void *src, uint width, uint height,
                       CmSurfaceFormatID surfFormat, uint depth, uint pitch)
{
    cm_list<CmEmulSys::iobuffer>::iterator buff_iter;

    assert((buf_id.get_data() < maxbsize) && ((int)CmEmulSys::iobuffers.size() < maxbsize));
    CmEmulSys::enter_dataport_cs();

    CM_unregister_buffer_emu(buf_id.get_data());

    CmEmulSys::iobuffer new_buff;

    new_buff.id = buf_id.get_data();
    new_buff.bclass = bclass;
    new_buff.pixelFormat = surfFormat;
    new_buff.p = src;
    new_buff.width = width;
    new_buff.height = height;
    new_buff.depth = depth;
    new_buff.pitch = pitch;

    // 3D surface is only used for sampler, which can only have
    // INPUT type buffer. Therefore no need to consider depth
    // in the buffer size calculation "width*height".
    if(bclass == GEN4_INPUT_OUTPUT_BUFFER) {
        // Let's consider the pre/post execution copy as legacy
        // and comment it out for now.

        //new_buff.p_volatile = (unsigned char*) malloc(width*height*depth);
        //if(new_buff.p_volatile != NULL)
        //{
        //    memcpy(new_buff.p_volatile, src, width*height*depth);
        //}
        new_buff.p_volatile = src;
    } else {
        new_buff.p_volatile = NULL;
    }
    CmEmulSys::iobuffers.add(new_buff);
    CmEmulSys::leave_dataport_cs();
}

CM_API void
CM_register_buffer_emu(SurfaceIndex buf_id, CmBufferType bclass, void *src, uint width)
{
    CM_register_buffer_emu(buf_id,bclass,src,width,1,R8G8B8A8_UINT,1,0);
}

CM_API void
CM_modify_buffer_emu(SurfaceIndex buf_id, CmBufferDescField field, int value)
{
    CmEmulSys::enter_dataport_cs();
    cm_list<CmEmulSys::iobuffer>::iterator buff_iter = CmEmulSys::search_buffer(buf_id.get_data());
    if (buff_iter != CmEmulSys::iobuffers.end()) {
        switch (field)
        {
        case GEN4_FIELD_SURFACE_FORMAT:
            buff_iter->pixelFormat = (CmSurfaceFormatID)value;
            break;
        case GEN4_FIELD_SURFACE_ID:
            buff_iter->id = value;
            break;
        case GEN4_FIELD_SURFACE_TYPE:
        case GEN4_FIELD_TILE_FORMAT:
        case GEN4_FIELD_SURFACE_PITCH:
        case GEN4_FIELD_SURFACE_SIZE:
        case GEN4_FIELD_SAMPLING_MIN_MODE:
        case GEN4_FIELD_SAMPLING_MIP_MODE:
        case GEN4_FIELD_SAMPLING_MAP_MODE:
            break;
        default:
             break;
        }
    }
    CmEmulSys::leave_dataport_cs();
}

CM_API void
cm_fence()
{
    for (auto it = CmEmulSys::iobuffers.begin();
              it != CmEmulSys::iobuffers.end(); ++it)
    {
        // Let's consider the pre/post execution copy as legacy
        // and comment it out for now.

        //if (it->p_volatile != 0)
        //{
        //    memcpy(it->p, it->p_volatile, it->width * it->height * it->depth);
        //}
    }
    // Add global fence if enabling threading in emulation mode.
}

CM_API void
cm_fence(unsigned char bit_mask)
{
    for (auto it = CmEmulSys::iobuffers.begin();
              it != CmEmulSys::iobuffers.end(); ++it)
    {
        // Let's consider the pre/post execution copy as legacy
        // and comment it out for now.

        //if (it->p_volatile != 0)
        //{
        //    memcpy(it->p, it->p_volatile, it->width * it->height * it->depth);
        //}
    }
    // Add global fence if enabling threading in emulation mode.
}

CM_API void
cm_wait(uchar mask)
{
}

