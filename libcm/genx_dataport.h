/*===================== begin_copyright_notice ==================================

 Copyright (c) 2020, Intel Corporation


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

#ifndef GENX_DATAPORT_H
#define GENX_DATAPORT_H

class CmThreadSpace;

#ifndef NEW_CM_RT
#define NEW_CM_RT  // Defined for new CM Runtime APIs
#endif

#include "cm_list.h"
#include "libcm_def.h"
#include "cm_vm.h"
#include "libcm_common.h"
#include "dataport_common.h"
#include <mutex>

#pragma warning(disable : 4244)

static std::mutex mutexForWrite;

typedef enum _CmBufferDescField_
{
    GEN4_FIELD_TILE_FORMAT     = 15,
    GEN4_FIELD_SURFACE_TYPE    = 16,
    GEN4_FIELD_SURFACE_FORMAT  = 17,
    GEN4_FIELD_SURFACE_PITCH   = 18,
    GEN4_FIELD_SURFACE_SIZE    = 19,
    GEN4_FIELD_SAMPLING_MIN_MODE = 20,
    GEN4_FIELD_SAMPLING_MIP_MODE = 21,
    GEN4_FIELD_SAMPLING_MAP_MODE = 22,
    GEN4_FIELD_PERSISTENT_MAP = 23,
    GEN4_FIELD_REGISTER_BUFFER = 24,
    GEN4_FIELD_SURFACE_ID = 25
} CmBufferDescField;

typedef enum _CmBufferAttrib_
{
    GENX_TOP_FIELD              = 1,
    GENX_BOTTOM_FIELD           = 2,
    GENX_DWALIGNED              = 3,
    GENX_MODIFIED               = 4,
    GENX_MODIFIED_TOP_FIELD     = 5,
    GENX_MODIFIED_BOTTOM_FIELD  = 6,
    GENX_MODIFIED_DWALIGNED     = 7,
    GENX_CONSTANT               = 8,
    GENX_CONSTANT_DWALIGNED     = 9,
    GENX_NUM_BUFFER_ATTRIB      = 10
} CmBufferAttrib;

typedef enum _CmSurfacePlaneIndex_
{
    GENX_SURFACE_Y_PLANE     = 0,
    GENX_SURFACE_U_PLANE     = 1,
    GENX_SURFACE_UV_PLANE    = 1,
    GENX_SURFACE_V_PLANE     = 2
} CmSurfacePlaneIndex;

typedef enum _CM_READ_SIZE_
{
    CM_READ_1 = 0,
    CM_READ_2 = 1,
    CM_READ_4 = 2,
    CM_READ_8 = 3
} CM_READ_SIZE;

/* Note: width and height for CM_register_buffer() in bytes and
 * for read()/write() in elements! */
typedef struct CM_REG_BUFF_PARM_S
{
    int esize;
    int length;
    int offset;

    union {
        int     d32bit;
        short   d16bit;
        char    d8bit;
        void *  dptr;
    } data;
}
CM_REG_BUFF_PARM_S;

#define CM_GLOBAL_COHERENT_FENCE 1
#define CM_L3_FLUSH_INSTRUCTIONS 2
#define CM_L3_FLUSH_TEXTURE_DATA 4
#define CM_L3_FLUSH_CONSTANT_DATA 8
#define CM_L3_FLUSH_RW_DATA 16
#define CM_COMMIT_ENABLE 32

CM_API void cm_wait(uchar mask = 0);

CM_API inline _GENX_ void cm_signal(void){}

CM_API inline _GENX_ void monitor_event(unsigned int event_id){}
CM_API inline _GENX_ void monitor_no_event(void){}
CM_API inline _GENX_ void signal_event(unsigned int event_id){}
CM_API inline _GENX_ unsigned int wait_event(unsigned short timer_value){ return 0; }

CM_API void cm_sampler_cache_flush();

#if !defined(SIMDCF_ELEMENT_SKIP)
#       ifdef CM_GENX
#           include "cm_internal_emu.h"    //using namespace __CMInternal__
#           define SIMDCF_ELEMENT_SKIP(i) \
        if (__CMInternal__::getWorkingStack() && !__CMInternal__::getWorkingStack()->isEmpty()) \
            if ((short)(__CMInternal__::getSIMDMarker() << (i)) >= 0) \
                continue; \

#       else
#           define SIMDCF_ELEMENT_SKIP(i)
#       endif
#endif

CM_API ushort get_thread_origin_x(void);
CM_API ushort get_thread_origin_y(void);
CM_API ushort get_color(void);
CM_API void set_thread_origin_x(ushort);
CM_API void set_thread_origin_y(ushort);
CM_API void set_color(ushort);
CM_API void initialize_global_surface_index();
CM_API void set_global_surface_index(int index, SurfaceIndex *si);
CM_API SurfaceIndex * get_global_surface_index(int index);

#ifndef NEW_CM_RT
CM_API void CM_register_buffer(int buf_id, CmBufferType bclass, void *src, uint width);
CM_API void CM_register_buffer(int buf_id, CmBufferType bclass, void *src, uint width, uint height,
                                      CmSurfaceFormatID surfFormat = R8G8B8A8_UINT, uint depth = 1, uint pitch = 0);
//CM_API void _CM_typed_register_buffer(int num, CM_REG_BUFF_PARM_S * dat);
CM_API void CM_unregister_buffer(int buf_id); /* Note: Cm spec 1.0 extension */
CM_API void CM_modify_buffer(int buf_id, CmBufferDescField field,
                                    int value);
CM_API void CM_modify_buffer_emu(SurfaceIndex buf_id, CmBufferDescField field, int value);
#else /* NEW_CM_RT */
CM_API void CM_register_buffer(SurfaceIndex buf_id, CmBufferType bclass, void *src, uint width);
CM_API void CM_register_buffer(SurfaceIndex buf_id, CmBufferType bclass, void *src, uint width, uint height,
                                      CmSurfaceFormatID surfFormat = R8G8B8A8_UINT, uint depth = 1, uint pitch = 0);
//CM_API void _CM_typed_register_buffer(SurfaceIndex num, CM_REG_BUFF_PARM_S * dat);
CM_API void CM_unregister_buffer(SurfaceIndex buf_id, bool copy=true); /* Note: Cm spec 1.0 extension */
CM_API void CM_unregister_buffer_emu(SurfaceIndex buf_id, bool copy);
CM_API void CM_modify_buffer(SurfaceIndex buf_id, CmBufferDescField field,
                                    int value);
CM_API void CM_modify_buffer_emu(SurfaceIndex buf_id, CmBufferDescField field, int value);
#endif /* NEW_CM_RT */

#define MODIFIED(A)                A, GENX_MODIFIED
#define TOP_FIELD(A)               A, GENX_TOP_FIELD
#define BOTTOM_FIELD(A)            A, GENX_BOTTOM_FIELD
#define MODIFIED_TOP_FIELD(A)      A, GENX_MODIFIED_TOP_FIELD
#define MODIFIED_BOTTOM_FIELD(A)   A, GENX_MODIFIED_BOTTOM_FIELD
#define DWALIGNED(A)               A, GENX_DWALIGNED
#define MODIFIED_DWALIGNED(A)      A, GENX_MODIFIED_DWALIGNED
#define CONSTANT(A)                A, GENX_CONSTANT
#define CONSTANT_DWALIGNED(A)      A, GENX_CONSTANT_DWALIGNED

#ifndef NEW_CM_RT

/* This funtion reads Media Block data from genx dataport */
template <typename T, uint R, uint C>
CM_API bool
read(uint buf_id, int x_pos, int y_pos, matrix<T,R,C> &in)
{
    int i,j;
    uint offset;
    cm_list<CmEmulSys::iobuffer>::iterator buff_iter =
                                    CmEmulSys::search_buffer(buf_id&0xFF);

    if(buff_iter == CmEmulSys::iobuffers.end()) {
        printf("Error reading buffer %d: buffer %d is not registered!\n", buf_id&0xFF, buf_id&0xFF);
        exit(EXIT_FAILURE);
    }

    int width = buff_iter->width;
    int height = buff_iter->height;

    {
        if((buff_iter->bclass != GEN4_INPUT_BUFFER) &&
            (buff_iter->bclass != GEN4_INPUT_OUTPUT_BUFFER)) {
                printf("Error reading buffer %d: the registered buffer type is not INPUT_BUFFER!\n", buf_id&0xFF);
                exit(EXIT_FAILURE);
        }
    }

    int x_pos_a, y_pos_a;  /* Actual positions */
    int sizeofT = sizeof(T); /* Make this into a signed integer */
    uint bpp = CM_genx_bytes_per_pixel(buff_iter->pixelFormat);
    assert(((bpp - 1) & bpp) == 0); // Is power-of-2 number

    for (i = 0; i < R; i++) {
        for (j = 0; j < C; j++) {
            x_pos_a = x_pos + j * sizeof(T);
            {
                y_pos_a = y_pos + i;
            }
            // We should check the boundary condition based on sizeof(T), x_pos_a is 0-based
            // Note: Use a signed variable; otherwise sizeof(T) is unsigned
            if ((x_pos_a + sizeofT) > width) {
                // If we're trying to read outside the boundary, limit the value of x_pos_a
                // Assumption -- We don't this situation:
                //         x_pos_a  width's boundary
                //           |      |
                //           <---type(T)--->
                // At most x_pos_a+sizeof(T) is exactly at the boundary.
                x_pos_a = width;
            }
            if (y_pos_a > height - 1) {
                y_pos_a = height - 1;
            }
            // Surface width can be less than bpp and coordinates can be negative

            if (y_pos_a < 0) {
                y_pos_a = 0;
            }

            // Surface width can be less than bpp and coordinates can be negative
            if((buff_iter->pixelFormat == YCRCB_NORMAL ||
                buff_iter->pixelFormat == YCRCB_SWAPY) &&
                x_pos_a < 0)
            {
                // If we're trying to read outside the left boundary, increase x_pos_a
                //sizeofT
                /*
                case 1 matrix 1 byte per element
                case 2 matrix 2 byte per element
                case 3 matrix 4 byte per element
                */
                if((j + (4/sizeofT)) > C) {
                    printf("Invalid matrix width for Packed format!\n", buf_id.get_data()&0xFF);
                    exit(EXIT_FAILURE);
                }
                offset = y_pos_a * width;
                if(buff_iter->pixelFormat == YCRCB_NORMAL)
                {
                    ((unsigned char*)in.get_addr(i*C+j))[0] = *((unsigned char*)((char*)buff_iter->p + offset + 0));
                    ((unsigned char*)in.get_addr(i*C+j))[1] = *((unsigned char*)((char*)buff_iter->p + offset + 1));
                    ((unsigned char*)in.get_addr(i*C+j))[2] = *((unsigned char*)((char*)buff_iter->p + offset + 0));
                    ((unsigned char*)in.get_addr(i*C+j))[3] = *((unsigned char*)((char*)buff_iter->p + offset + 3));
                }else if(buff_iter->pixelFormat == YCRCB_SWAPY)
                {
                    ((unsigned char*)in.get_addr(i*C+j))[0] = *((unsigned char*)((char*)buff_iter->p + offset + 0));
                    ((unsigned char*)in.get_addr(i*C+j))[1] = *((unsigned char*)((char*)buff_iter->p + offset + 1));
                    ((unsigned char*)in.get_addr(i*C+j))[2] = *((unsigned char*)((char*)buff_iter->p + offset + 2));
                    ((unsigned char*)in.get_addr(i*C+j))[3] = *((unsigned char*)((char*)buff_iter->p + offset + 1));
                }

                j+= (4/sizeofT);
                j--;
                continue;

            }else
            {
                if (x_pos_a < 0) {
                    // Need to align x position to bbp
                    int offset = x_pos % bpp;
                    x_pos_a -= offset;
                }
                while (x_pos_a < 0) {
                    // If we're trying to read outside the left boundary, increase x_pos_a
                    x_pos_a += bpp;
                }
            }

            if (x_pos_a >= width) {

                if((buff_iter->pixelFormat == YCRCB_NORMAL ||
                    buff_iter->pixelFormat == YCRCB_SWAPY))
                {
                    // If we're trying to read outside the left boundary, increase x_pos_a
                    //sizeofT
                    /*
                    case 1 matrix 1 byte per element
                    case 2 matrix 2 byte per element
                    case 3 matrix 4 byte per element
                    */
                    if((j + (4/sizeofT)) > C) {
                        printf("Invalid matrix width for Packed format!\n", buf_id.get_data()&0xFF);
                        exit(EXIT_FAILURE);
                    }
                    //setting offset to width - 4 for row we are processing
                    offset = y_pos_a * width + width - 4;
                    if(buff_iter->pixelFormat == YCRCB_NORMAL)
                    {
                        ((unsigned char*)in.get_addr(i*C+j))[0] = *((unsigned char*)((char*)buff_iter->p + offset + 2));
                        ((unsigned char*)in.get_addr(i*C+j))[1] = *((unsigned char*)((char*)buff_iter->p + offset + 1));
                        ((unsigned char*)in.get_addr(i*C+j))[2] = *((unsigned char*)((char*)buff_iter->p + offset + 2));
                        ((unsigned char*)in.get_addr(i*C+j))[3] = *((unsigned char*)((char*)buff_iter->p + offset + 3));
                    }else if(buff_iter->pixelFormat == YCRCB_SWAPY)
                    {
                        ((unsigned char*)in.get_addr(i*C+j))[0] = *((unsigned char*)((char*)buff_iter->p + offset + 0));
                        ((unsigned char*)in.get_addr(i*C+j))[1] = *((unsigned char*)((char*)buff_iter->p + offset + 3));
                        ((unsigned char*)in.get_addr(i*C+j))[2] = *((unsigned char*)((char*)buff_iter->p + offset + 2));
                        ((unsigned char*)in.get_addr(i*C+j))[3] = *((unsigned char*)((char*)buff_iter->p + offset + 3));
                    }

                    j+= (4/sizeofT);
                    j--;
                    continue;

                }else
                {

                    x_pos_a = x_pos_a - bpp;
                    for( int byte_count =0; byte_count < sizeof(T); byte_count++)
                    {
                        if (x_pos_a >= width) {
                            x_pos_a = x_pos_a - bpp;
                        }
                        offset = y_pos_a * width + x_pos_a;

                        /*
                        If destination size per element is less then or equal pixel size of the surface
                        move the pixel value accross the destination elements.
                        If destination size per element is greater then pixel size of the surface
                        replicate pixel value in the destination element.
                        */
                        if(sizeof(T) <= bpp)
                        {
                            for(int bpp_count = 0; j<C&&bpp_count<bpp ;j++, bpp_count+=sizeof(T))
                            {
                                in(i,j) = *((T*)((char*)buff_iter->p + offset + bpp_count));
                            }
                            j--;
                            break;
                        }
                        else
                        {
                            ((unsigned char*)in.get_addr(i*C+j))[byte_count] = *((unsigned char*)((char*)buff_iter->p + offset));
                        }

                        x_pos_a = x_pos_a + 1;
                    }
                    x_pos_a = width;
                }
            }
            else {
                offset = y_pos_a * width + x_pos_a;
                {
                    in(i,j) = *((T*)((char*)buff_iter->p + offset));
                }
            }
        }
    }

    return true;
}

/* This funtion reads Media Block data from genx dataport */
template <typename T, uint R, uint C>
CM_API bool
read(uint buf_id, int x_pos, int y_pos, matrix_ref<T,R,C> in)
{
    int i,j;
    uint offset;
    cm_list<CmEmulSys::iobuffer>::iterator buff_iter =
                                    CmEmulSys::search_buffer(buf_id&0xFF);

    if(buff_iter == CmEmulSys::iobuffers.end()) {
        printf("Error reading buffer %d: buffer %d is not registered!\n", buf_id&0xFF, buf_id&0xFF);
        exit(EXIT_FAILURE);
    }

    int width = buff_iter->width;
    int height = buff_iter->height;

    {
        if((buff_iter->bclass != GEN4_INPUT_BUFFER) &&
            (buff_iter->bclass != GEN4_INPUT_OUTPUT_BUFFER)) {
                printf("Error reading buffer %d: the registered buffer type is not INPUT_BUFFER!\n", buf_id&0xFF);
                exit(EXIT_FAILURE);
        }
    }

    int x_pos_a, y_pos_a;  /* Actual positions */
    int sizeofT = sizeof(T); /* Make this into a signed integer */
    uint bpp = CM_genx_bytes_per_pixel(buff_iter->pixelFormat);
    assert(((bpp - 1) & bpp) == 0); // Is power-of-2 number

    for (i = 0; i < R; i++) {
        for (j = 0; j < C; j++) {
            x_pos_a = x_pos + j * sizeof(T);
            {
                y_pos_a = y_pos + i;
            }
            // We should check the boundary condition based on sizeof(T), x_pos_a is 0-based
            // Note: Use a signed variable; otherwise sizeof(T) is unsigned
            if ((x_pos_a + sizeofT) > width) {
                // If we're trying to read outside the boundary, limit the value of x_pos_a
                // Assumption -- We don't this situation:
                //         x_pos_a  width's boundary
                //           |      |
                //           <---type(T)--->
                // At most x_pos_a+sizeof(T) is exactly at the boundary.
                x_pos_a = width;
            }
            if (y_pos_a > height - 1) {
                y_pos_a = height - 1;
            }
            // Surface width can be less than bpp and coordinates can be negative

            if (y_pos_a < 0) {
                y_pos_a = 0;
            }

            // Surface width can be less than bpp and coordinates can be negative
            if((buff_iter->pixelFormat == YCRCB_NORMAL ||
                buff_iter->pixelFormat == YCRCB_SWAPY) &&
                x_pos_a < 0)
            {
                // If we're trying to read outside the left boundary, increase x_pos_a
                //sizeofT
                /*
                case 1 matrix 1 byte per element
                case 2 matrix 2 byte per element
                case 3 matrix 4 byte per element
                */
                if((j + (4/sizeofT)) > C) {
                    printf("Invalid matrix width for Packed format!\n", buf_id.get_data()&0xFF);
                    exit(EXIT_FAILURE);
                }
                offset = y_pos_a * width;
                if(buff_iter->pixelFormat == YCRCB_NORMAL)
                {
                    ((unsigned char*)in.get_addr(i*C+j))[0] = *((unsigned char*)((char*)buff_iter->p + offset + 0));
                    ((unsigned char*)in.get_addr(i*C+j))[1] = *((unsigned char*)((char*)buff_iter->p + offset + 1));
                    ((unsigned char*)in.get_addr(i*C+j))[2] = *((unsigned char*)((char*)buff_iter->p + offset + 0));
                    ((unsigned char*)in.get_addr(i*C+j))[3] = *((unsigned char*)((char*)buff_iter->p + offset + 3));
                }else if(buff_iter->pixelFormat == YCRCB_SWAPY)
                {
                    ((unsigned char*)in.get_addr(i*C+j))[0] = *((unsigned char*)((char*)buff_iter->p + offset + 0));
                    ((unsigned char*)in.get_addr(i*C+j))[1] = *((unsigned char*)((char*)buff_iter->p + offset + 1));
                    ((unsigned char*)in.get_addr(i*C+j))[2] = *((unsigned char*)((char*)buff_iter->p + offset + 2));
                    ((unsigned char*)in.get_addr(i*C+j))[3] = *((unsigned char*)((char*)buff_iter->p + offset + 1));
                }

                j+= (4/sizeofT);
                j--;
                continue;

            }else
            {
                if (x_pos_a < 0) {
                    // Need to align x position to bbp
                    int offset = x_pos % bpp;
                    x_pos_a -= offset;
                }
                while (x_pos_a < 0) {
                    // If we're trying to read outside the left boundary, increase x_pos_a
                    x_pos_a += bpp;
                }
            }

            if (x_pos_a >= width) {

                if((buff_iter->pixelFormat == YCRCB_NORMAL ||
                    buff_iter->pixelFormat == YCRCB_SWAPY))
                {
                    // If we're trying to read outside the left boundary, increase x_pos_a
                    //sizeofT
                    /*
                    case 1 matrix 1 byte per element
                    case 2 matrix 2 byte per element
                    case 3 matrix 4 byte per element
                    */
                    if((j + (4/sizeofT)) > C) {
                        printf("Invalid matrix width for Packed format!\n", buf_id.get_data()&0xFF);
                        exit(EXIT_FAILURE);
                    }
                    //setting offset to width - 4 for row we are processing
                    offset = y_pos_a * width + width - 4;
                    if(buff_iter->pixelFormat == YCRCB_NORMAL)
                    {
                        ((unsigned char*)in.get_addr(i*C+j))[0] = *((unsigned char*)((char*)buff_iter->p + offset + 2));
                        ((unsigned char*)in.get_addr(i*C+j))[1] = *((unsigned char*)((char*)buff_iter->p + offset + 1));
                        ((unsigned char*)in.get_addr(i*C+j))[2] = *((unsigned char*)((char*)buff_iter->p + offset + 2));
                        ((unsigned char*)in.get_addr(i*C+j))[3] = *((unsigned char*)((char*)buff_iter->p + offset + 3));
                    }else if(buff_iter->pixelFormat == YCRCB_SWAPY)
                    {
                        ((unsigned char*)in.get_addr(i*C+j))[0] = *((unsigned char*)((char*)buff_iter->p + offset + 0));
                        ((unsigned char*)in.get_addr(i*C+j))[1] = *((unsigned char*)((char*)buff_iter->p + offset + 3));
                        ((unsigned char*)in.get_addr(i*C+j))[2] = *((unsigned char*)((char*)buff_iter->p + offset + 2));
                        ((unsigned char*)in.get_addr(i*C+j))[3] = *((unsigned char*)((char*)buff_iter->p + offset + 3));
                    }

                    j+= (4/sizeofT);
                    j--;
                    continue;

                }else
                {

                    x_pos_a = x_pos_a - bpp;
                    for( int byte_count =0; byte_count < sizeof(T); byte_count++)
                    {
                        if (x_pos_a >= width) {
                            x_pos_a = x_pos_a - bpp;
                        }
                        offset = y_pos_a * width + x_pos_a;

                        /*
                        If destination size per element is less then or equal pixel size of the surface
                        move the pixel value accross the destination elements.
                        If destination size per element is greater then pixel size of the surface
                        replicate pixel value in the destination element.
                        */
                        if(sizeof(T) <= bpp)
                        {
                            for(int bpp_count = 0; j<C&&bpp_count<bpp ;j++, bpp_count+=sizeof(T))
                            {
                                in(i,j) = *((T*)((char*)buff_iter->p + offset + bpp_count));
                            }
                            j--;
                            break;
                        }
                        else
                        {
                            ((unsigned char*)in.get_addr(i*C+j))[byte_count] = *((unsigned char*)((char*)buff_iter->p + offset));
                        }

                        x_pos_a = x_pos_a + 1;
                    }
                    x_pos_a = width;
                }
            }
            else {
                offset = y_pos_a * width + x_pos_a;
                {
                    in(i,j) = *((T*)((char*)buff_iter->p + offset));
                }
            }
        }
    }

    return true;
}

template <typename T, uint R, uint C>
CM_API bool
read(uint buf_id, CmBufferAttrib buf_attrib, int x_pos, int y_pos, matrix<T,R,C> &in)
{
    int i,j;
    uint offset;
    cm_list<CmEmulSys::iobuffer>::iterator buff_iter =
                                    CmEmulSys::search_buffer(buf_id&0xFF);

    if(buff_iter == CmEmulSys::iobuffers.end()) {
        printf("Error reading buffer %d: buffer %d is not registered!\n", buf_id&0xFF, buf_id&0xFF);
        exit(EXIT_FAILURE);
    }

    if((buf_attrib < GENX_TOP_FIELD) || (buf_attrib > GENX_MODIFIED_BOTTOM_FIELD)) {
        printf("Error reading buffer %d: incorrect buffer attribute %d!\n", buf_id&0xFF, buf_attrib);
        exit(EXIT_FAILURE);
    }

    int width = buff_iter->width;
    int height = buff_iter->height;

    if(buf_attrib >= GENX_MODIFIED) {
        if(buff_iter->bclass != GEN4_INPUT_OUTPUT_BUFFER) {
            printf("Error reading buffer MODIFIED(%d): the registered buffer type is not INPUT_OUTPUT_BUFFER!\n", buf_id&0xFF);
            exit(EXIT_FAILURE);
        }
    } else {
        if((buff_iter->bclass != GEN4_INPUT_BUFFER) &&
            (buff_iter->bclass != GEN4_INPUT_OUTPUT_BUFFER)) {
                printf("Error reading buffer %d: the registered buffer type is not INPUT_BUFFER!\n", buf_id&0xFF);
                exit(EXIT_FAILURE);
        }
    }

    int x_pos_a, y_pos_a;  /* Actual positions */
    int sizeofT = sizeof(T); /* Make this into a signed integer */
    uint bpp = CM_genx_bytes_per_pixel(buff_iter->pixelFormat);
    assert(((bpp - 1) & bpp) == 0); // Is power-of-2 number

    for (i = 0; i < R; i++) {
        for (j = 0; j < C; j++) {
            x_pos_a = x_pos + j * sizeof(T);
            if ((buf_attrib == GENX_TOP_FIELD) ||
                (buf_attrib == GENX_MODIFIED_TOP_FIELD))
            {
                //y_pos_a = (y_pos + i) * 2;
                y_pos_a = (y_pos + i) << 1;
            }
            else
                if ((buf_attrib == GENX_BOTTOM_FIELD) ||
                    (buf_attrib == GENX_MODIFIED_BOTTOM_FIELD))
                {
                    //y_pos_a = (y_pos + i) * 2  + 1;
                    y_pos_a = 1 + ((y_pos + i) << 1);
                }
                else
                {
                    y_pos_a = y_pos + i;
                }
            // We should check the boundary condition based on sizeof(T), x_pos_a is 0-based
            // Note: Use a signed variable; otherwise sizeof(T) is unsigned
            if ((x_pos_a + sizeofT) > width) {
                // If we're trying to read outside the boundary, limit the value of x_pos_a
                // Assumption -- We don't this situation:
                //         x_pos_a  width's boundary
                //           |      |
                //           <---type(T)--->
                // At most x_pos_a+sizeof(T) is exactly at the boundary.
                x_pos_a = width;
            }
            if (y_pos_a > height - 1) {
                y_pos_a = height - 1;
            }
            // Surface width can be less than bpp and coordinates can be negative
            if (y_pos_a < 0) {
                y_pos_a = 0;
            }

            // Surface width can be less than bpp and coordinates can be negative
            if((buff_iter->pixelFormat == YCRCB_NORMAL ||
                buff_iter->pixelFormat == YCRCB_SWAPY) &&
                x_pos_a < 0)
            {
                void * temp_buffer_pointer = NULL;
                if(buf_attrib >= GENX_MODIFIED)
                {
                    temp_buffer_pointer = buff_iter->p_volatile;
                }else
                {
                    temp_buffer_pointer = buff_iter->p;
                }
                // If we're trying to read outside the left boundary, increase x_pos_a
                //sizeofT
                /*
                case 1 matrix 1 byte per element
                case 2 matrix 2 byte per element
                case 3 matrix 4 byte per element
                */
                if((j + (4/sizeofT)) > C) {
                    printf("Invalid matrix width for Packed format!\n", buf_id.get_data()&0xFF);
                    exit(EXIT_FAILURE);
                }
                offset = y_pos_a * width;

                if(buff_iter->pixelFormat == YCRCB_NORMAL)
                {
                    ((unsigned char*)in.get_addr(i*C+j))[0] = *((unsigned char*)((char*)temp_buffer_pointer + offset + 0));
                    ((unsigned char*)in.get_addr(i*C+j))[1] = *((unsigned char*)((char*)temp_buffer_pointer + offset + 1));
                    ((unsigned char*)in.get_addr(i*C+j))[2] = *((unsigned char*)((char*)temp_buffer_pointer + offset + 0));
                    ((unsigned char*)in.get_addr(i*C+j))[3] = *((unsigned char*)((char*)temp_buffer_pointer + offset + 3));
                }else if(buff_iter->pixelFormat == YCRCB_SWAPY)
                {
                    ((unsigned char*)in.get_addr(i*C+j))[0] = *((unsigned char*)((char*)temp_buffer_pointer + offset + 0));
                    ((unsigned char*)in.get_addr(i*C+j))[1] = *((unsigned char*)((char*)temp_buffer_pointer + offset + 1));
                    ((unsigned char*)in.get_addr(i*C+j))[2] = *((unsigned char*)((char*)temp_buffer_pointer + offset + 2));
                    ((unsigned char*)in.get_addr(i*C+j))[3] = *((unsigned char*)((char*)temp_buffer_pointer + offset + 1));
                }

                j+= (4/sizeofT);
                j--;
                continue;

            }else
            {
                if (x_pos_a < 0) {
                    // Need to align x position to bbp
                    int offset = x_pos % bpp;
                    x_pos_a -= offset;
                }
                while (x_pos_a < 0) {
                    // If we're trying to read outside the left boundary, increase x_pos_a
                    x_pos_a += bpp;
                }
            }

            if (x_pos_a >= width) {

                void * temp_buffer_pointer = NULL;
                if(buf_attrib >= GENX_MODIFIED)
                {
                    temp_buffer_pointer = buff_iter->p_volatile;
                }else
                {
                    temp_buffer_pointer = buff_iter->p;
                }
                if((buff_iter->pixelFormat == YCRCB_NORMAL ||
                    buff_iter->pixelFormat == YCRCB_SWAPY))
                {
                    // If we're trying to read outside the left boundary, increase x_pos_a
                    //sizeofT
                    /*
                    case 1 matrix 1 byte per element
                    case 2 matrix 2 byte per element
                    case 3 matrix 4 byte per element
                    */
                    if((j + (4/sizeofT)) > C) {
                        printf("Invalid matrix width for Packed format!\n", buf_id.get_data()&0xFF);
                        exit(EXIT_FAILURE);
                    }
                    //setting offset to width - 4 for row we are processing
                    offset = y_pos_a * width + width - 4;
                    if(buff_iter->pixelFormat == YCRCB_NORMAL)
                    {
                        ((unsigned char*)in.get_addr(i*C+j))[0] = *((unsigned char*)((char*)temp_buffer_pointer + offset + 2));
                        ((unsigned char*)in.get_addr(i*C+j))[1] = *((unsigned char*)((char*)temp_buffer_pointer + offset + 1));
                        ((unsigned char*)in.get_addr(i*C+j))[2] = *((unsigned char*)((char*)temp_buffer_pointer + offset + 2));
                        ((unsigned char*)in.get_addr(i*C+j))[3] = *((unsigned char*)((char*)temp_buffer_pointer + offset + 3));
                    }else if(buff_iter->pixelFormat == YCRCB_SWAPY)
                    {
                        ((unsigned char*)in.get_addr(i*C+j))[0] = *((unsigned char*)((char*)temp_buffer_pointer + offset + 0));
                        ((unsigned char*)in.get_addr(i*C+j))[1] = *((unsigned char*)((char*)temp_buffer_pointer + offset + 3));
                        ((unsigned char*)in.get_addr(i*C+j))[2] = *((unsigned char*)((char*)temp_buffer_pointer + offset + 2));
                        ((unsigned char*)in.get_addr(i*C+j))[3] = *((unsigned char*)((char*)temp_buffer_pointer + offset + 3));
                    }

                    j+= (4/sizeofT);
                    j--;
                    continue;

                }else
                {

                    x_pos_a = x_pos_a - bpp;
                    for( int byte_count =0; byte_count < sizeof(T); byte_count++)
                    {
                        if (x_pos_a >= width) {
                            x_pos_a = x_pos_a - bpp;
                        }
                        offset = y_pos_a * width + x_pos_a;

                        /*
                        If destination size per element is less then or equal pixel size of the surface
                        move the pixel value accross the destination elements.
                        If destination size per element is greater then pixel size of the surface
                        replicate pixel value in the destination element.
                        */
                        if(sizeof(T) <= bpp)
                        {
                            for(int bpp_count = 0; j<C&&bpp_count<bpp ;j++, bpp_count+=sizeof(T))
                            {
                                in(i,j) = *((T*)((char*)temp_buffer_pointer + offset + bpp_count));
                            }
                            j--;
                            break;
                        }
                        else
                        {
                            ((unsigned char*)in.get_addr(i*C+j))[byte_count] = *((unsigned char*)((char*)temp_buffer_pointer + offset));
                        }

                        x_pos_a = x_pos_a + 1;
                    }
                    x_pos_a = width;
                }
            }
            else {
                offset = y_pos_a * width + x_pos_a;
                if(buf_attrib >= GENX_MODIFIED) {
                    in(i,j) = *((T*)((char*)buff_iter->p_volatile + offset));
                } else {
                    in(i,j) = *((T*)((char*)buff_iter->p + offset));
                }
            }
        }
    }

    return true;
}

/* This funtion reads Media Block data from genx dataport */
template <typename T, uint R, uint C>
CM_API bool
read(uint buf_id, CmBufferAttrib buf_attrib, int x_pos, int y_pos, matrix_ref<T,R,C> in)
{
    int i,j;
    uint offset;
    cm_list<CmEmulSys::iobuffer>::iterator buff_iter =
                                    CmEmulSys::search_buffer(buf_id&0xFF);

    if(buff_iter == CmEmulSys::iobuffers.end()) {
        printf("Error reading buffer %d: buffer %d is not registered!\n", buf_id&0xFF, buf_id&0xFF);
        exit(EXIT_FAILURE);
    }

    if((buf_attrib < GENX_TOP_FIELD) || (buf_attrib > GENX_MODIFIED_BOTTOM_FIELD)) {
        printf("Error reading buffer %d: incorrect buffer attribute %d!\n", buf_id&0xFF, buf_attrib);
        exit(EXIT_FAILURE);
    }

    int width = buff_iter->width;
    int height = buff_iter->height;

    if(buf_attrib >= GENX_MODIFIED) {
        if(buff_iter->bclass != GEN4_INPUT_OUTPUT_BUFFER) {
            printf("Error reading buffer MODIFIED(%d): the registered buffer type is not INPUT_OUTPUT_BUFFER!\n", buf_id&0xFF);
            exit(EXIT_FAILURE);
        }
    } else {
        if((buff_iter->bclass != GEN4_INPUT_BUFFER) &&
            (buff_iter->bclass != GEN4_INPUT_OUTPUT_BUFFER)) {
                printf("Error reading buffer %d: the registered buffer type is not INPUT_BUFFER!\n", buf_id&0xFF);
                exit(EXIT_FAILURE);
        }
    }

    int x_pos_a, y_pos_a;  /* Actual positions */
    int sizeofT = sizeof(T); /* Make this into a signed integer */
    uint bpp = CM_genx_bytes_per_pixel(buff_iter->pixelFormat);
    assert(((bpp - 1) & bpp) == 0); // Is power-of-2 number

    for (i = 0; i < R; i++) {
        for (j = 0; j < C; j++) {
            x_pos_a = x_pos + j * sizeof(T);
            if ((buf_attrib == GENX_TOP_FIELD) ||
                (buf_attrib == GENX_MODIFIED_TOP_FIELD))
            {
                //y_pos_a = (y_pos + i) * 2;
                y_pos_a = (y_pos + i) << 1;
            }
            else
                if ((buf_attrib == GENX_BOTTOM_FIELD) ||
                    (buf_attrib == GENX_MODIFIED_BOTTOM_FIELD))
                {
                    //y_pos_a = (y_pos + i) * 2  + 1;
                    y_pos_a = 1 + ((y_pos + i) << 1);
                }
                else
                {
                    y_pos_a = y_pos + i;
                }
            // We should check the boundary condition based on sizeof(T), x_pos_a is 0-based
            // Note: Use a signed variable; otherwise sizeof(T) is unsigned
            if ((x_pos_a + sizeofT) > width) {
                // If we're trying to read outside the boundary, limit the value of x_pos_a
                // Assumption -- We don't this situation:
                //         x_pos_a  width's boundary
                //           |      |
                //           <---type(T)--->
                // At most x_pos_a+sizeof(T) is exactly at the boundary.
                x_pos_a = width;
            }
            if (y_pos_a > height - 1) {
                y_pos_a = height - 1;
            }
            // Surface width can be less than bpp and coordinates can be negative
            if (y_pos_a < 0) {
                y_pos_a = 0;
            }

            // Surface width can be less than bpp and coordinates can be negative
            if((buff_iter->pixelFormat == YCRCB_NORMAL ||
                buff_iter->pixelFormat == YCRCB_SWAPY) &&
                x_pos_a < 0)
            {
                void * temp_buffer_pointer = NULL;
                if(buf_attrib >= GENX_MODIFIED)
                {
                    temp_buffer_pointer = buff_iter->p_volatile;
                }else
                {
                    temp_buffer_pointer = buff_iter->p;
                }
                // If we're trying to read outside the left boundary, increase x_pos_a
                //sizeofT
                /*
                case 1 matrix 1 byte per element
                case 2 matrix 2 byte per element
                case 3 matrix 4 byte per element
                */
                if((j + (4/sizeofT)) > C) {
                    printf("Invalid matrix width for Packed format!\n", buf_id.get_data()&0xFF);
                    exit(EXIT_FAILURE);
                }
                offset = y_pos_a * width;

                if(buff_iter->pixelFormat == YCRCB_NORMAL)
                {
                    ((unsigned char*)in.get_addr(i*C+j))[0] = *((unsigned char*)((char*)temp_buffer_pointer + offset + 0));
                    ((unsigned char*)in.get_addr(i*C+j))[1] = *((unsigned char*)((char*)temp_buffer_pointer + offset + 1));
                    ((unsigned char*)in.get_addr(i*C+j))[2] = *((unsigned char*)((char*)temp_buffer_pointer + offset + 0));
                    ((unsigned char*)in.get_addr(i*C+j))[3] = *((unsigned char*)((char*)temp_buffer_pointer + offset + 3));
                }else if(buff_iter->pixelFormat == YCRCB_SWAPY)
                {
                    ((unsigned char*)in.get_addr(i*C+j))[0] = *((unsigned char*)((char*)temp_buffer_pointer + offset + 0));
                    ((unsigned char*)in.get_addr(i*C+j))[1] = *((unsigned char*)((char*)temp_buffer_pointer + offset + 1));
                    ((unsigned char*)in.get_addr(i*C+j))[2] = *((unsigned char*)((char*)temp_buffer_pointer + offset + 2));
                    ((unsigned char*)in.get_addr(i*C+j))[3] = *((unsigned char*)((char*)temp_buffer_pointer + offset + 1));
                }

                j+= (4/sizeofT);
                j--;
                continue;

            }else
            {
                if (x_pos_a < 0) {
                    // Need to align x position to bbp
                    int offset = x_pos % bpp;
                    x_pos_a -= offset;
                }
                while (x_pos_a < 0) {
                    // If we're trying to read outside the left boundary, increase x_pos_a
                    x_pos_a += bpp;
                }
            }

            if (x_pos_a >= width) {

                void * temp_buffer_pointer = NULL;
                if(buf_attrib >= GENX_MODIFIED)
                {
                    temp_buffer_pointer = buff_iter->p_volatile;
                }else
                {
                    temp_buffer_pointer = buff_iter->p;
                }
                if((buff_iter->pixelFormat == YCRCB_NORMAL ||
                    buff_iter->pixelFormat == YCRCB_SWAPY))
                {
                    // If we're trying to read outside the left boundary, increase x_pos_a
                    //sizeofT
                    /*
                    case 1 matrix 1 byte per element
                    case 2 matrix 2 byte per element
                    case 3 matrix 4 byte per element
                    */
                    if((j + (4/sizeofT)) > C) {
                        printf("Invalid matrix width for Packed format!\n", buf_id.get_data()&0xFF);
                        exit(EXIT_FAILURE);
                    }
                    //setting offset to width - 4 for row we are processing
                    offset = y_pos_a * width + width - 4;
                    if(buff_iter->pixelFormat == YCRCB_NORMAL)
                    {
                        ((unsigned char*)in.get_addr(i*C+j))[0] = *((unsigned char*)((char*)temp_buffer_pointer + offset + 2));
                        ((unsigned char*)in.get_addr(i*C+j))[1] = *((unsigned char*)((char*)temp_buffer_pointer + offset + 1));
                        ((unsigned char*)in.get_addr(i*C+j))[2] = *((unsigned char*)((char*)temp_buffer_pointer + offset + 2));
                        ((unsigned char*)in.get_addr(i*C+j))[3] = *((unsigned char*)((char*)temp_buffer_pointer + offset + 3));
                    }else if(buff_iter->pixelFormat == YCRCB_SWAPY)
                    {
                        ((unsigned char*)in.get_addr(i*C+j))[0] = *((unsigned char*)((char*)temp_buffer_pointer + offset + 0));
                        ((unsigned char*)in.get_addr(i*C+j))[1] = *((unsigned char*)((char*)temp_buffer_pointer + offset + 3));
                        ((unsigned char*)in.get_addr(i*C+j))[2] = *((unsigned char*)((char*)temp_buffer_pointer + offset + 2));
                        ((unsigned char*)in.get_addr(i*C+j))[3] = *((unsigned char*)((char*)temp_buffer_pointer + offset + 3));
                    }

                    j+= (4/sizeofT);
                    j--;
                    continue;

                }else
                {

                    x_pos_a = x_pos_a - bpp;
                    for( int byte_count =0; byte_count < sizeof(T); byte_count++)
                    {
                        if (x_pos_a >= width) {
                            x_pos_a = x_pos_a - bpp;
                        }
                        offset = y_pos_a * width + x_pos_a;

                        /*
                        If destination size per element is less then or equal pixel size of the surface
                        move the pixel value accross the destination elements.
                        If destination size per element is greater then pixel size of the surface
                        replicate pixel value in the destination element.
                        */
                        if(sizeof(T) <= bpp)
                        {
                            for(int bpp_count = 0; j<C&&bpp_count<bpp ;j++, bpp_count+=sizeof(T))
                            {
                                in(i,j) = *((T*)((char*)temp_buffer_pointer + offset + bpp_count));
                            }
                            j--;
                            break;
                        }
                        else
                        {
                            ((unsigned char*)in.get_addr(i*C+j))[byte_count] = *((unsigned char*)((char*)temp_buffer_pointer + offset));
                        }

                        x_pos_a = x_pos_a + 1;
                    }
                    x_pos_a = width;
                }
            }
            else {
                offset = y_pos_a * width + x_pos_a;
                if(buf_attrib >= GENX_MODIFIED) {
                    in(i,j) = *((T*)((char*)buff_iter->p_volatile + offset));
                } else {
                    in(i,j) = *((T*)((char*)buff_iter->p + offset));
                }
            }
        }
    }

    return true;
}

/* This funtion writes Media Block data to genx dataport */
template <typename T, uint R, uint C>
CM_API bool
write(uint buf_id, int x_pos, int y_pos, const matrix<T,R,C> &out)
{
    int i,j;
    uint offset;
    cm_list<CmEmulSys::iobuffer>::iterator buff_iter =
                                    CmEmulSys::search_buffer(buf_id&0xFF);

    if(buff_iter == CmEmulSys::iobuffers.end()) {
        printf("Error writing buffer %d: buffer %d is not registered!\n", buf_id&0xFF, buf_id&0xFF);
        exit(EXIT_FAILURE);
    }

    int width = buff_iter->width;
    int height = buff_iter->height;

    if((x_pos % 4) != 0) {
        printf("Error writing buffer %d: X-coordinate must be 4-byte aligned!\n", buf_id&0xFF);
        exit(EXIT_FAILURE);
    }
    int sizeofT = sizeof(T); /* Make this into a signed integer */
    if(((C * sizeofT) % 4) != 0) {
        printf("Error writing buffer %d: input matrix width must be 4-byte aligned!\n", buf_id&0xFF);
        exit(EXIT_FAILURE);
    }

    if((buff_iter->bclass != GEN4_OUTPUT_BUFFER) &&
        (buff_iter->bclass != GEN4_INPUT_OUTPUT_BUFFER)) {
        printf("Error writing buffer %d: incorrect buffer type!\n", buf_id&0xFF);
    }

    uint x_pos_a, y_pos_a;  /* Actual positions */
    uint bpp = CM_genx_bytes_per_pixel(buff_iter->pixelFormat);
    assert(((bpp - 1) & bpp) == 0); // Is power-of-2 number

    for (i = 0; i < R; i++) {
        for (j = 0; j < C; j++) {
            x_pos_a = x_pos + j * sizeofT;
            {
                y_pos_a = y_pos + i;
            }
            if ((x_pos_a + sizeofT) > width) {
                continue;
            }
            if (y_pos_a > height - 1) {
                continue;
            }
            offset = y_pos_a * width + x_pos_a;
            if(buff_iter->bclass == GEN4_INPUT_OUTPUT_BUFFER) {
                *((T*)( (char*)buff_iter->p_volatile + offset )) = out(i,j);
            } else {
                *((T*)( (char*)buff_iter->p + offset )) = out(i,j);
            }
        }
    }

    return true;
}

/* This funtion writes Media Block data to genx dataport */
template <typename T, uint R, uint C>
CM_API bool
write(uint buf_id, int x_pos, int y_pos, const matrix_ref<T,R,C> out)
{
    int i,j;
    uint offset;
    cm_list<CmEmulSys::iobuffer>::iterator buff_iter =
                                    CmEmulSys::search_buffer(buf_id&0xFF);

    if(buff_iter == CmEmulSys::iobuffers.end()) {
        printf("Error writing buffer %d: buffer %d is not registered!\n", buf_id&0xFF, buf_id&0xFF);
        exit(EXIT_FAILURE);
    }

    int width = buff_iter->width;
    int height = buff_iter->height;

    if((x_pos % 4) != 0) {
        printf("Error writing buffer %d: X-coordinate must be 4-byte aligned!\n", buf_id&0xFF);
        exit(EXIT_FAILURE);
    }
    int sizeofT = sizeof(T); /* Make this into a signed integer */
    if(((C * sizeofT) % 4) != 0) {
        printf("Error writing buffer %d: input matrix width must be 4-byte aligned!\n", buf_id&0xFF);
        exit(EXIT_FAILURE);
    }

    if((buff_iter->bclass != GEN4_OUTPUT_BUFFER) &&
        (buff_iter->bclass != GEN4_INPUT_OUTPUT_BUFFER)) {
            printf("Error writing buffer %d: incorrect buffer type!\n", buf_id&0xFF);
    }

    uint x_pos_a, y_pos_a;  /* Actual positions */
    uint bpp = CM_genx_bytes_per_pixel(buff_iter->pixelFormat);
    assert(((bpp - 1) & bpp) == 0); // Is power-of-2 number

    for (i = 0; i < R; i++) {
        for (j = 0; j < C; j++) {
            x_pos_a = x_pos + j * sizeofT;
            {
                y_pos_a = y_pos + i;
            }
            if ((x_pos_a + sizeofT) > width) {
                continue;
            }
            if (y_pos_a > height - 1) {
                continue;
            }
            offset = y_pos_a * width + x_pos_a;
            if(buff_iter->bclass == GEN4_INPUT_OUTPUT_BUFFER) {
                *((T*)( (char*)buff_iter->p_volatile + offset )) = out(i,j);
            } else {
                *((T*)( (char*)buff_iter->p + offset )) = out(i,j);
            }
        }
    }

    return true;
}

/* This funtion writes Media Block data to genx dataport */
template <typename T, uint R, uint C>
CM_API bool
write(uint buf_id, CmBufferAttrib buf_attrib, int x_pos, int y_pos, const matrix<T,R,C> &out)
{
    int i,j;
    uint offset;
    cm_list<CmEmulSys::iobuffer>::iterator buff_iter =
                                    CmEmulSys::search_buffer(buf_id&0xFF);

    if(buff_iter == CmEmulSys::iobuffers.end()) {
        printf("Error writing buffer %d: buffer %d is not registered!\n", buf_id&0xFF, buf_id&0xFF);
        exit(EXIT_FAILURE);
    }

    if((buf_attrib != GENX_TOP_FIELD) && (buf_attrib != GENX_BOTTOM_FIELD)) {
        printf("Error writing buffer %d: incorrect buffer attribute %d!\n", buf_id&0xFF, buf_attrib);
        exit(EXIT_FAILURE);
    }

    int width = buff_iter->width;
    int height = buff_iter->height;

    if((x_pos % 4) != 0) {
        printf("Error writing buffer %d: X-coordinate must be 4-byte aligned!\n", buf_id&0xFF);
        exit(EXIT_FAILURE);
    }
    int sizeofT = sizeof(T); /* Make this into a signed integer */
    if(((C * sizeofT) % 4) != 0) {
        printf("Error writing buffer %d: input matrix width must be 4-byte aligned!\n", buf_id&0xFF);
        exit(EXIT_FAILURE);
    }

    if((buff_iter->bclass != GEN4_OUTPUT_BUFFER) &&
        (buff_iter->bclass != GEN4_INPUT_OUTPUT_BUFFER)) {
        printf("Error writing buffer %d: incorrect buffer type!\n", buf_id&0xFF);
    }

    uint x_pos_a, y_pos_a;  /* Actual positions */
    uint bpp = CM_genx_bytes_per_pixel(buff_iter->pixelFormat);
    assert(((bpp - 1) & bpp) == 0); // Is power-of-2 number

    for (i = 0; i < R; i++) {
        for (j = 0; j < C; j++) {
            x_pos_a = x_pos + j * sizeof(T);
            if ((buf_attrib == GENX_TOP_FIELD) ||
                (buf_attrib == GENX_MODIFIED_TOP_FIELD))
            {
                //y_pos_a = (y_pos + i) * 2;
                y_pos_a = (y_pos + i) << 1;
            }
            else
                if ((buf_attrib == GENX_BOTTOM_FIELD) ||
                    (buf_attrib == GENX_MODIFIED_BOTTOM_FIELD))
                {
                    //y_pos_a = (y_pos + i) * 2  + 1;
                    y_pos_a = 1 + ((y_pos + i) << 1);
                }
                else
                {
                    y_pos_a = y_pos + i;
                }

            if ((x_pos_a + sizeofT) > width) {
                continue;
            }
            if (y_pos_a > height - 1) {
                continue;
            }
            offset = y_pos_a * width + x_pos_a;
            if(buff_iter->bclass == GEN4_INPUT_OUTPUT_BUFFER) {
                *((T*)( (char*)buff_iter->p_volatile + offset )) = out(i,j);
            } else {
                *((T*)( (char*)buff_iter->p + offset )) = out(i,j);
            }
        }
    }

    return true;
}

/* This funtion writes Media Block data to genx dataport */
template <typename T, uint R, uint C>
CM_API bool
write(uint buf_id, CmBufferAttrib buf_attrib, int x_pos, int y_pos, const matrix_ref<T,R,C> out)
{
    int i,j;
    uint offset;
    cm_list<CmEmulSys::iobuffer>::iterator buff_iter =
                                    CmEmulSys::search_buffer(buf_id&0xFF);

    if(buff_iter == CmEmulSys::iobuffers.end()) {
        printf("Error writing buffer %d: buffer %d is not registered!\n", buf_id&0xFF, buf_id&0xFF);
        exit(EXIT_FAILURE);
    }

    if((buf_attrib != GENX_TOP_FIELD) && (buf_attrib != GENX_BOTTOM_FIELD)) {
        printf("Error writing buffer %d: incorrect buffer attribute %d!\n", buf_id&0xFF, buf_attrib);
        exit(EXIT_FAILURE);
    }

    int width = buff_iter->width;
    int height = buff_iter->height;

    if((x_pos % 4) != 0) {
        printf("Error writing buffer %d: X-coordinate must be 4-byte aligned!\n", buf_id&0xFF);
        exit(EXIT_FAILURE);
    }
    int sizeofT = sizeof(T); /* Make this into a signed integer */
    if(((C * sizeofT) % 4) != 0) {
        printf("Error writing buffer %d: input matrix width must be 4-byte aligned!\n", buf_id&0xFF);
        exit(EXIT_FAILURE);
    }

    if((buff_iter->bclass != GEN4_OUTPUT_BUFFER) &&
        (buff_iter->bclass != GEN4_INPUT_OUTPUT_BUFFER)) {
        printf("Error writing buffer %d: incorrect buffer type!\n", buf_id&0xFF);
    }

    uint x_pos_a, y_pos_a;  /* Actual positions */
    uint bpp = CM_genx_bytes_per_pixel(buff_iter->pixelFormat);
    assert(((bpp - 1) & bpp) == 0); // Is power-of-2 number

    for (i = 0; i < R; i++) {
        for (j = 0; j < C; j++) {
            x_pos_a = x_pos + j * sizeof(T);
            if ((buf_attrib == GENX_TOP_FIELD) ||
                (buf_attrib == GENX_MODIFIED_TOP_FIELD))
            {
                //y_pos_a = (y_pos + i) * 2;
                y_pos_a = (y_pos + i) << 1;
            }
            else
                if ((buf_attrib == GENX_BOTTOM_FIELD) ||
                    (buf_attrib == GENX_MODIFIED_BOTTOM_FIELD))
                {
                    //y_pos_a = (y_pos + i) * 2  + 1;
                    y_pos_a = 1 + ((y_pos + i) << 1);
                }
                else
                {
                    y_pos_a = y_pos + i;
                }

            if ((x_pos_a + sizeofT) > width) {
                continue;
            }
            if (y_pos_a > height - 1) {
                continue;
            }
            offset = y_pos_a * width + x_pos_a;
            if(buff_iter->bclass == GEN4_INPUT_OUTPUT_BUFFER) {
                *((T*)( (char*)buff_iter->p_volatile + offset )) = out(i,j);
            } else {
                *((T*)( (char*)buff_iter->p + offset )) = out(i,j);
            }
        }
    }

    return true;
}

/* This funtion reads OWord Block data from genx dataport */
template <typename T, uint S>
CM_API bool
read(uint buf_id, int offset, vector<T,S> &in)
{
    int i;
    uint pos;
    cm_list<CmEmulSys::iobuffer>::iterator buff_iter =
                                    CmEmulSys::search_buffer(buf_id&0xFF);

    if(buff_iter == CmEmulSys::iobuffers.end()) {
        printf("Error reading buffer %d: buffer %d is not registered!\n", buf_id&0xFF, buf_id&0xFF);
        exit(EXIT_FAILURE);
    }

    int width = buff_iter->width;
    int height = buff_iter->height;

    if((offset % 16) != 0) {
        printf("Error reading buffer %d: offset must be 16-byte aligned!\n", buf_id&0xFF);
        exit(EXIT_FAILURE);
    }
    assert(height == 1);

    {
        if((buff_iter->bclass != GEN4_INPUT_BUFFER) &&
            (buff_iter->bclass != GEN4_INPUT_OUTPUT_BUFFER)) {
                printf("Error reading buffer %d: the registered buffer type is not INPUT_BUFFER!\n", buf_id&0xFF);
                exit(EXIT_FAILURE);
        }
    }

    int sizeofT = sizeof(T); /* Make this into a signed integer */
    for (i = 0; i < S; i++) {
        pos = offset + i * sizeofT;
        if (pos >= width) {
            in(i) = 0;
        } else {
            {
                in(i) = *((T*)((char*)buff_iter->p + pos));
            }
        }
    }

    return true;
}

/* This funtion reads OWord Block data from genx dataport */
template <typename T, uint S>
CM_API bool
read(uint buf_id, int offset, vector_ref<T,S> in)
{
    int i;
    uint pos;
    cm_list<CmEmulSys::iobuffer>::iterator buff_iter =
                                    CmEmulSys::search_buffer(buf_id&0xFF);

    if(buff_iter == CmEmulSys::iobuffers.end()) {
        printf("Error reading buffer %d: buffer %d is not registered!\n", buf_id&0xFF, buf_id&0xFF);
        exit(EXIT_FAILURE);
    }

    int width = buff_iter->width;
    int height = buff_iter->height;

    if((offset % 16) != 0) {
        printf("Error reading buffer %d: offset must be 16-byte aligned!\n", buf_id&0xFF);
        exit(EXIT_FAILURE);
    }
    assert(height == 1);

    {
        if((buff_iter->bclass != GEN4_INPUT_BUFFER) &&
            (buff_iter->bclass != GEN4_INPUT_OUTPUT_BUFFER)) {
                printf("Error reading buffer %d: the registered buffer type is not INPUT_BUFFER!\n", buf_id&0xFF);
                exit(EXIT_FAILURE);
        }
    }

    int sizeofT = sizeof(T); /* Make this into a signed integer */
    for (i = 0; i < S; i++) {
        pos = offset + i * sizeofT;
        if (pos >= width) {
            in(i) = 0;
        } else {
            {
                in(i) = *((T*)((char*)buff_iter->p + pos));
            }
        }
    }

    return true;
}

/* This funtion reads OWord Block data from genx dataport */
template <typename T, uint S>
CM_API bool
read(uint buf_id, CmBufferAttrib buf_attrib, int offset, vector<T,S> &in)
{
    int i;
    uint pos;
    cm_list<CmEmulSys::iobuffer>::iterator buff_iter =
                                    CmEmulSys::search_buffer(buf_id&0xFF);

    if(buff_iter == CmEmulSys::iobuffers.end()) {
        printf("Error reading buffer %d: buffer %d is not registered!\n", buf_id&0xFF, buf_id&0xFF);
        exit(EXIT_FAILURE);
    }

    if(buf_attrib != GENX_MODIFIED && buf_attrib != GENX_DWALIGNED &&
       buf_attrib != GENX_MODIFIED_DWALIGNED && buf_attrib != GENX_CONSTANT &&
       buf_attrib != GENX_CONSTANT_DWALIGNED) {
        printf("Error reading buffer %d: incorrect buffer attribute %d!\n", buf_id&0xFF, buf_attrib);
        exit(EXIT_FAILURE);
    }

    int width = buff_iter->width;
    int height = buff_iter->height;

    if(buf_attrib != GENX_DWALIGNED && buf_attrib != GENX_MODIFIED_DWALIGNED &&
       buf_attrib != GENX_CONSTANT_DWALIGNED) {
        if((offset % 16) != 0) {
            printf("Error reading buffer %d: offset must be 16-byte aligned!\n", buf_id&0xFF);
            exit(EXIT_FAILURE);
        }
    }
    else {
        if((offset % 4) != 0) {
            printf("Error reading buffer %d: offset must be 4-byte aligned!\n", buf_id&0xFF);
            exit(EXIT_FAILURE);
        }
    }

    assert(height == 1);

    char* buff;

    if(buf_attrib == GENX_MODIFIED || buf_attrib == GENX_MODIFIED_DWALIGNED)
    {
        if(buff_iter->bclass != GEN4_INPUT_OUTPUT_BUFFER) {
            printf("Error reading buffer MODIFIED(%d): the registered buffer type is not INPUT_OUTPUT_BUFFER!\n", buf_id&0xFF);
            exit(EXIT_FAILURE);
        }

        buff = (char*) buff_iter->p_volatile;
    }
    else
    {
        buff = (char*) buff_iter->p;
    }

    int sizeofT = sizeof(T); /* Make this into a signed integer */
    for (i = 0; i < S; i++) {
        pos = offset + i * sizeofT;
        if (pos >= width) {
            in(i) = 0;
        } else {
            {
                in(i) = *((T*)(buff + pos));
            }
        }
    }

    return true;
}

/* This funtion reads OWord Block data from genx dataport */
template <typename T, uint S>
CM_API bool
read(uint buf_id, CmBufferAttrib buf_attrib, int offset, vector_ref<T,S> in)
{
    int i;
    uint pos;
    cm_list<CmEmulSys::iobuffer>::iterator buff_iter =
                                    CmEmulSys::search_buffer(buf_id&0xFF);

    if(buff_iter == CmEmulSys::iobuffers.end()) {
        printf("Error reading buffer %d: buffer %d is not registered!\n", buf_id&0xFF, buf_id&0xFF);
        exit(EXIT_FAILURE);
    }

    if(buf_attrib != GENX_MODIFIED && buf_attrib != GENX_DWALIGNED &&
       buf_attrib != GENX_MODIFIED_DWALIGNED && buf_attrib != GENX_CONSTANT &&
       buf_attrib != GENX_CONSTANT_DWALIGNED) {
        printf("Error reading buffer %d: incorrect buffer attribute %d!\n", buf_id&0xFF, buf_attrib);
        exit(EXIT_FAILURE);
    }

    int width = buff_iter->width;
    int height = buff_iter->height;

    if(buf_attrib != GENX_DWALIGNED && buf_attrib != GENX_MODIFIED_DWALIGNED &&
       buf_attrib != GENX_CONSTANT_DWALIGNED) {
        if((offset % 16) != 0) {
            printf("Error reading buffer %d: offset must be 16-byte aligned!\n", buf_id&0xFF);
            exit(EXIT_FAILURE);
        }
    }
    else {
        if((offset % 4) != 0) {
            printf("Error reading buffer %d: offset must be 4-byte aligned!\n", buf_id&0xFF);
            exit(EXIT_FAILURE);
        }
    }

    assert(height == 1);

    char* buff;

    if(buf_attrib == GENX_MODIFIED || buf_attrib == GENX_MODIFIED_DWALIGNED)
    {
        if(buff_iter->bclass != GEN4_INPUT_OUTPUT_BUFFER) {
            printf("Error reading buffer MODIFIED(%d): the registered buffer type is not INPUT_OUTPUT_BUFFER!\n", buf_id&0xFF);
            exit(EXIT_FAILURE);
        }
        buff = (char*) buff_iter->p_volatile;
    }
    else
    {
        buff = (char*) buff_iter->p;
    }

    int sizeofT = sizeof(T); /* Make this into a signed integer */
    for (i = 0; i < S; i++) {
        pos = offset + i * sizeofT;
        if (pos >= width) {
            in(i) = 0;
        } else {
            {
                in(i) = *((T*)(buff + pos));
            }
        }
    }

    return true;
}

/* This funtion writes OWord Block data to genx dataport */
template <typename T, uint S>
CM_API bool
write(uint buf_id, int offset, const vector<T,S> &out)
{
    int i;
    uint pos;
    cm_list<CmEmulSys::iobuffer>::iterator buff_iter =
                                    CmEmulSys::search_buffer(buf_id&0xFF);

    if(buff_iter == CmEmulSys::iobuffers.end()) {
        printf("Error reading buffer %d: buffer %d is not registered!\n", buf_id&0xFF, buf_id&0xFF);
        exit(EXIT_FAILURE);
    }

    int width = buff_iter->width;
    int height = buff_iter->height;

    if((offset % 16) != 0) {
        printf("Error writing buffer %d: offset must be 16-byte aligned!\n", buf_id&0xFF);
        exit(EXIT_FAILURE);
    }
    int sizeofT = sizeof(T); /* Make this into a signed integer */
    if(((S * sizeofT) % 16) != 0) {
        printf("Error writing buffer %d: input vector size must be 16-byte aligned!\n", buf_id&0xFF);
        exit(EXIT_FAILURE);
    }
    assert(height == 1);

    if((buff_iter->bclass != GEN4_OUTPUT_BUFFER) &&
        (buff_iter->bclass != GEN4_INPUT_OUTPUT_BUFFER)) {
            printf("Error writing buffer %d: incorrect buffer type!\n", buf_id&0xFF);
            exit(EXIT_FAILURE);
    }

    for (i = 0; i < S; i++) {
        pos = offset + i * sizeofT;
        if (pos >= width) {
            break;
        }
        if(buff_iter->bclass == GEN4_INPUT_OUTPUT_BUFFER) {
            *((T*)( (char*)buff_iter->p_volatile + pos )) = out(i);
        } else {
            *((T*)( (char*)buff_iter->p + pos )) = out(i);
        }
    }

    return true;
}

/* This funtion writes OWord Block data to genx dataport */
template <typename T, uint S>
CM_API bool
write(uint buf_id, int offset, const vector_ref<T, S> out)
{
    int i;
    uint pos;
    cm_list<CmEmulSys::iobuffer>::iterator buff_iter =
                                    CmEmulSys::search_buffer(buf_id&0xFF);

    if(buff_iter == CmEmulSys::iobuffers.end()) {
        printf("Error reading buffer %d: buffer %d is not registered!\n", buf_id&0xFF, buf_id&0xFF);
        exit(EXIT_FAILURE);
    }

    int width = buff_iter->width;
    int height = buff_iter->height;

    if((offset % 16) != 0) {
        printf("Error writing buffer %d: offset must be 16-byte aligned!\n", buf_id&0xFF);
        exit(EXIT_FAILURE);
    }
    int sizeofT = sizeof(T); /* Make this into a signed integer */
    if(((S * sizeofT) % 16) != 0) {
        printf("Error writing buffer %d: input vector size must be 16-byte aligned!\n", buf_id&0xFF);
        exit(EXIT_FAILURE);
    }
    assert(height == 1);

    if((buff_iter->bclass != GEN4_OUTPUT_BUFFER) &&
        (buff_iter->bclass != GEN4_INPUT_OUTPUT_BUFFER)) {
            printf("Error writing buffer %d: incorrect buffer type!\n", buf_id&0xFF);
            exit(EXIT_FAILURE);
    }

    for (i = 0; i < S; i++) {
        pos = offset + i * sizeofT;
        if (pos >= width) {
            break;
        }
        if(buff_iter->bclass == GEN4_INPUT_OUTPUT_BUFFER) {
            *((T*)( (char*)buff_iter->p_volatile + pos )) = out(i);
        } else {
            *((T*)( (char*)buff_iter->p + pos )) = out(i);
        }
    }

    return true;
}

/* This funtion reads scattered DWords from genx dataport */
template <typename T, uint N>
CM_API bool
read(uint buf_id, uint global_offset, vector<uint, N> &element_offset, vector<T, N> &in)
{
    static const bool conformable1 = is_fp_or_dword_type<T>::value;
    int i;
    uint pos;
    cm_list<CmEmulSys::iobuffer>::iterator buff_iter =
                                    CmEmulSys::search_buffer(buf_id&0xFF);

    if(buff_iter == CmEmulSys::iobuffers.end()) {
        printf("Error reading buffer %d: buffer %d is not registered!\n", buf_id&0xFF, buf_id&0xFF);
        exit(EXIT_FAILURE);
    }

    int width = buff_iter->width;
    int height = buff_iter->height;

    //assert(height == 1);

    {
        if((buff_iter->bclass != GEN4_INPUT_BUFFER) &&
            (buff_iter->bclass != GEN4_INPUT_OUTPUT_BUFFER)) {
                printf("Error reading buffer %d: the registered buffer type is not INPUT_BUFFER!\n", buf_id&0xFF);
                exit(EXIT_FAILURE);
        }
    }

    if((N != 8) && (N != 16)) {
        printf("Error reading buffer %d: the vector parameter length must be 8 or 16 for DWord scattered read!\n", buf_id&0xFF);
        exit(EXIT_FAILURE);
    }

    for (i = 0; i < N; i++) {
        SIMDCF_ELEMENT_SKIP(i);
        pos = (global_offset + element_offset(i))*4;
        if (pos >= width*height) {
            in(i) = width*height - 1;
        } else {
            {
                in(i) = *((T*)((char*)buff_iter->p + pos));
            }
        }
    }

    return true;
}

template <typename T, uint N>
CM_API bool
read(uint buf_id, uint global_offset, vector<uint, N> &element_offset, vector_ref<T, N> in)
{
    static const bool conformable1 = is_fp_or_dword_type<T>::value;
    int i;
    uint pos;
    cm_list<CmEmulSys::iobuffer>::iterator buff_iter =
                                    CmEmulSys::search_buffer(buf_id&0xFF);

    if(buff_iter == CmEmulSys::iobuffers.end()) {
        printf("Error reading buffer %d: buffer %d is not registered!\n", buf_id&0xFF, buf_id&0xFF);
        exit(EXIT_FAILURE);
    }

    int width = buff_iter->width;
    int height = buff_iter->height;

    //assert(height == 1);

    {
        if((buff_iter->bclass != GEN4_INPUT_BUFFER) &&
            (buff_iter->bclass != GEN4_INPUT_OUTPUT_BUFFER)) {
                printf("Error reading buffer %d: the registered buffer type is not INPUT_BUFFER!\n", buf_id&0xFF);
                exit(EXIT_FAILURE);
        }
    }

    if((N != 8) && (N != 16)) {
        printf("Error reading buffer %d: the vector parameter length must be 8 or 16 for DWord scattered read!\n", buf_id&0xFF);
        exit(EXIT_FAILURE);
    }

    for (i = 0; i < N; i++) {
        SIMDCF_ELEMENT_SKIP(i);
        pos = (global_offset + element_offset(i))*4;
        if (pos >= width*height) {
            in(i) = width*height - 1;
        } else {
            {
                in(i) = *((T*)((char*)buff_iter->p + pos));
            }
        }
    }

    return true;
}

template <typename T, uint N>
CM_API bool
read(uint buf_id, uint global_offset, vector_ref<uint, N> element_offset, vector<T, N> &in)
{
    static const bool conformable1 = is_fp_or_dword_type<T>::value;
    int i;
    uint pos;
    cm_list<CmEmulSys::iobuffer>::iterator buff_iter =
                                    CmEmulSys::search_buffer(buf_id&0xFF);

    if(buff_iter == CmEmulSys::iobuffers.end()) {
        printf("Error reading buffer %d: buffer %d is not registered!\n", buf_id&0xFF, buf_id&0xFF);
        exit(EXIT_FAILURE);
    }

    int width = buff_iter->width;
    int height = buff_iter->height;

    //assert(height == 1);

    {
        if((buff_iter->bclass != GEN4_INPUT_BUFFER) &&
            (buff_iter->bclass != GEN4_INPUT_OUTPUT_BUFFER)) {
                printf("Error reading buffer %d: the registered buffer type is not INPUT_BUFFER!\n", buf_id&0xFF);
                exit(EXIT_FAILURE);
        }
    }

    if((N != 8) && (N != 16)) {
        printf("Error reading buffer %d: the vector parameter length must be 8 or 16 for DWord scattered read!\n", buf_id&0xFF);
        exit(EXIT_FAILURE);
    }

    for (i = 0; i < N; i++) {
        SIMDCF_ELEMENT_SKIP(i);
        pos = (global_offset + element_offset(i))*4;
        if (pos >= width*height) {
            in(i) = width*height - 1;
        } else {
            {
                in(i) = *((T*)((char*)buff_iter->p + pos));
            }
        }
    }

    return true;
}

template <typename T, uint N>
CM_API bool
read(uint buf_id, uint global_offset, vector_ref<uint, N> element_offset, vector_ref<T, N> in)
{
    static const bool conformable1 = is_fp_or_dword_type<T>::value;
    int i;
    uint pos;
    cm_list<CmEmulSys::iobuffer>::iterator buff_iter =
                                    CmEmulSys::search_buffer(buf_id&0xFF);

    if(buff_iter == CmEmulSys::iobuffers.end()) {
        printf("Error reading buffer %d: buffer %d is not registered!\n", buf_id&0xFF, buf_id&0xFF);
        exit(EXIT_FAILURE);
    }

    int width = buff_iter->width;
    int height = buff_iter->height;

    //assert(height == 1);

    {
        if((buff_iter->bclass != GEN4_INPUT_BUFFER) &&
            (buff_iter->bclass != GEN4_INPUT_OUTPUT_BUFFER)) {
                printf("Error reading buffer %d: the registered buffer type is not INPUT_BUFFER!\n", buf_id&0xFF);
                exit(EXIT_FAILURE);
        }
    }

    if((N != 8) && (N != 16)) {
        printf("Error reading buffer %d: the vector parameter length must be 8 or 16 for DWord scattered read!\n", buf_id&0xFF);
        exit(EXIT_FAILURE);
    }

    for (i = 0; i < N; i++) {
        SIMDCF_ELEMENT_SKIP(i);
        pos = (global_offset + element_offset(i))*4;
        if (pos >= width*height) {
            in(i) = width*height - 1;
        } else {
            {
                in(i) = *((T*)((char*)buff_iter->p + pos));
            }
        }
    }

    return true;
}

/* This funtion reads scattered DWords from genx dataport */
template <typename T, uint N>
CM_API bool
read(uint buf_id, CmBufferAttrib buf_attrib, uint global_offset, vector<uint, N> &element_offset, vector<T, N> &in)
{
    static const bool conformable1 = is_fp_or_dword_type<T>::value;
    int i;
    uint pos;
    cm_list<CmEmulSys::iobuffer>::iterator buff_iter =
                                    CmEmulSys::search_buffer(buf_id&0xFF);

    if(buff_iter == CmEmulSys::iobuffers.end()) {
        printf("Error reading buffer %d: buffer %d is not registered!\n", buf_id&0xFF, buf_id&0xFF);
        exit(EXIT_FAILURE);
    }

    if(buf_attrib != GENX_MODIFIED && buf_attrib != GENX_CONSTANT) {
        printf("Error reading buffer %d: incorrect buffer attribute %d!\n", buf_id&0xFF, buf_attrib);
        exit(EXIT_FAILURE);
    }

    int width = buff_iter->width;
    int height = buff_iter->height;

    //assert(height == 1);

    {
        if(buff_iter->bclass != GEN4_INPUT_OUTPUT_BUFFER) {
            printf("Error reading buffer MODIFIED(%d): the registered buffer type is not INPUT_OUTPUT_BUFFER!\n", buf_id&0xFF);
            exit(EXIT_FAILURE);
        }
    }

    if((N != 8) && (N != 16)) {
        printf("Error reading buffer %d: the vector parameter length must be 8 or 16 for DWord scattered read!\n", buf_id&0xFF);
        exit(EXIT_FAILURE);
    }

    for (i = 0; i < N; i++) {
        SIMDCF_ELEMENT_SKIP(i);
        pos = (global_offset + element_offset(i))*4;
        if (pos >= width*height) {
            in(i) = width*height - 1;
        } else {
            {
                in(i) = *((T*)((char*)buff_iter->p_volatile + pos));
            }
        }
    }

    return true;
}

template <typename T, uint N>
CM_API bool
read(uint buf_id, CmBufferAttrib buf_attrib, uint global_offset, vector<uint, N> &element_offset, vector_ref<T, N> in)
{
    static const bool conformable1 = is_fp_or_dword_type<T>::value;
    int i;
    uint pos;
    cm_list<CmEmulSys::iobuffer>::iterator buff_iter =
                                    CmEmulSys::search_buffer(buf_id&0xFF);

    if(buff_iter == CmEmulSys::iobuffers.end()) {
        printf("Error reading buffer %d: buffer %d is not registered!\n", buf_id&0xFF, buf_id&0xFF);
        exit(EXIT_FAILURE);
    }

    if(buf_attrib != GENX_MODIFIED && buf_attrib != GENX_CONSTANT) {
        printf("Error reading buffer %d: incorrect buffer attribute %d!\n", buf_id&0xFF, buf_attrib);
        exit(EXIT_FAILURE);
    }

    int width = buff_iter->width;
    int height = buff_iter->height;

    //assert(height == 1);

    {
        if(buff_iter->bclass != GEN4_INPUT_OUTPUT_BUFFER) {
            printf("Error reading buffer MODIFIED(%d): the registered buffer type is not INPUT_OUTPUT_BUFFER!\n", buf_id&0xFF);
            exit(EXIT_FAILURE);
        }
    }

    if((N != 8) && (N != 16)) {
        printf("Error reading buffer %d: the vector parameter length must be 8 or 16 for DWord scattered read!\n", buf_id&0xFF);
        exit(EXIT_FAILURE);
    }

    for (i = 0; i < N; i++) {
        SIMDCF_ELEMENT_SKIP(i);
        pos = (global_offset + element_offset(i))*4;
        if (pos >= width*height) {
            in(i) = width*height - 1;
        } else {
            {
                in(i) = *((T*)((char*)buff_iter->p_volatile + pos));
            }
        }
    }

    return true;
}

template <typename T, uint N>
CM_API bool
read(uint buf_id, CmBufferAttrib buf_attrib, uint global_offset, vector_ref<uint, N> element_offset, vector<T, N> &in)
{
    static const bool conformable1 = is_fp_or_dword_type<T>::value;
    int i;
    uint pos;
    cm_list<CmEmulSys::iobuffer>::iterator buff_iter =
                                    CmEmulSys::search_buffer(buf_id&0xFF);

    if(buff_iter == CmEmulSys::iobuffers.end()) {
        printf("Error reading buffer %d: buffer %d is not registered!\n", buf_id&0xFF, buf_id&0xFF);
        exit(EXIT_FAILURE);
    }

    if(buf_attrib != GENX_MODIFIED && buf_attrib != GENX_CONSTANT) {
        printf("Error reading buffer %d: incorrect buffer attribute %d!\n", buf_id&0xFF, buf_attrib);
        exit(EXIT_FAILURE);
    }

    int width = buff_iter->width;
    int height = buff_iter->height;

    //assert(height == 1);

    {
        if(buff_iter->bclass != GEN4_INPUT_OUTPUT_BUFFER) {
            printf("Error reading buffer MODIFIED(%d): the registered buffer type is not INPUT_OUTPUT_BUFFER!\n", buf_id&0xFF);
            exit(EXIT_FAILURE);
        }
    }

    if((N != 8) && (N != 16)) {
        printf("Error reading buffer %d: the vector parameter length must be 8 or 16 for DWord scattered read!\n", buf_id&0xFF);
        exit(EXIT_FAILURE);
    }

    for (i = 0; i < N; i++) {
        SIMDCF_ELEMENT_SKIP(i);
        pos = (global_offset + element_offset(i))*4;
        if (pos >= width*height) {
            in(i) = width*height - 1;
        } else {
            {
                in(i) = *((T*)((char*)buff_iter->p_volatile + pos));
            }
        }
    }

    return true;
}

template <typename T, uint N>
CM_API bool
read(uint buf_id, CmBufferAttrib buf_attrib, uint global_offset, vector_ref<uint, N> element_offset, vector_ref<T, N> in)
{
    static const bool conformable1 = is_fp_or_dword_type<T>::value;
    int i;
    uint pos;
    cm_list<CmEmulSys::iobuffer>::iterator buff_iter =
                                    CmEmulSys::search_buffer(buf_id&0xFF);

    if(buff_iter == CmEmulSys::iobuffers.end()) {
        printf("Error reading buffer %d: buffer %d is not registered!\n", buf_id&0xFF, buf_id&0xFF);
        exit(EXIT_FAILURE);
    }

    if(buf_attrib != GENX_MODIFIED && buf_attrib != GENX_CONSTANT) {
        printf("Error reading buffer %d: incorrect buffer attribute %d!\n", buf_id&0xFF, buf_attrib);
        exit(EXIT_FAILURE);
    }

    int width = buff_iter->width;
    int height = buff_iter->height;

    //assert(height == 1);

    {
        if(buff_iter->bclass != GEN4_INPUT_OUTPUT_BUFFER) {
            printf("Error reading buffer MODIFIED(%d): the registered buffer type is not INPUT_OUTPUT_BUFFER!\n", buf_id&0xFF);
            exit(EXIT_FAILURE);
        }
    }

    if((N != 8) && (N != 16)) {
        printf("Error reading buffer %d: the vector parameter length must be 8 or 16 for DWord scattered read!\n", buf_id&0xFF);
        exit(EXIT_FAILURE);
    }

    for (i = 0; i < N; i++) {
        SIMDCF_ELEMENT_SKIP(i);
        pos = (global_offset + element_offset(i))*4;
        if (pos >= width*height) {
            in(i) = width*height - 1;
        } else {
            {
                in(i) = *((T*)((char*)buff_iter->p_volatile + pos));
            }
        }
    }

    return true;
}

/* This funtion writes scattered DWords to genx dataport */
template <typename T, uint N>
CM_API bool
write(uint buf_id, uint global_offset, vector<uint, N> &element_offset, vector<T, N> &out)
{
    static const bool conformable1 = is_fp_or_dword_type<T>::value;
    int i;
    uint pos;
    cm_list<CmEmulSys::iobuffer>::iterator buff_iter =
                                    CmEmulSys::search_buffer(buf_id&0xFF);

    if(buff_iter == CmEmulSys::iobuffers.end()) {
        printf("Error reading buffer %d: buffer %d is not registered!\n", buf_id&0xFF, buf_id&0xFF);
        exit(EXIT_FAILURE);
    }

    int width = buff_iter->width;
    int height = buff_iter->height;

    //assert(height == 1);

    if((buff_iter->bclass != GEN4_OUTPUT_BUFFER) &&
        (buff_iter->bclass != GEN4_INPUT_OUTPUT_BUFFER)) {
            printf("Error writing buffer %d: incorrect buffer type!\n", buf_id&0xFF);
            exit(EXIT_FAILURE);
    }

    if((N != 8) && (N != 16)) {
        printf("Error reading buffer %d: the vector parameter length must be 8 or 16 for DWord scattered write!\n", buf_id&0xFF);
        exit(EXIT_FAILURE);
    }

    for (i = 0; i < N; i++) {
        SIMDCF_ELEMENT_SKIP(i);
        pos = (global_offset + element_offset(i))*4;
        if (pos >= width*height) {
            break;
        }
        if(buff_iter->bclass == GEN4_INPUT_OUTPUT_BUFFER) {
            *((T*)( (char*)buff_iter->p_volatile + pos )) = out(i);
        } else {
            *((T*)( (char*)buff_iter->p + pos )) = out(i);
        }
    }

    return true;
}

template <typename T, uint N>
CM_API bool
write(uint buf_id, uint global_offset, vector<uint, N> &element_offset, vector_ref<T, N> out)
{
    static const bool conformable1 = is_fp_or_dword_type<T>::value;
    int i;
    uint pos;
    cm_list<CmEmulSys::iobuffer>::iterator buff_iter =
                                    CmEmulSys::search_buffer(buf_id&0xFF);

    if(buff_iter == CmEmulSys::iobuffers.end()) {
        printf("Error reading buffer %d: buffer %d is not registered!\n", buf_id&0xFF, buf_id&0xFF);
        exit(EXIT_FAILURE);
    }

    int width = buff_iter->width;
    int height = buff_iter->height;

    //assert(height == 1);

    if((buff_iter->bclass != GEN4_OUTPUT_BUFFER) &&
        (buff_iter->bclass != GEN4_INPUT_OUTPUT_BUFFER)) {
            printf("Error writing buffer %d: incorrect buffer type!\n", buf_id&0xFF);
            exit(EXIT_FAILURE);
    }

    if((N != 8) && (N != 16)) {
        printf("Error reading buffer %d: the vector parameter length must be 8 or 16 for DWord scattered write!\n", buf_id&0xFF);
        exit(EXIT_FAILURE);
    }

    for (i = 0; i < N; i++) {
        SIMDCF_ELEMENT_SKIP(i);
        pos = (global_offset + element_offset(i))*4;
        if (pos >= width*height) {
            break;
        }
        if(buff_iter->bclass == GEN4_INPUT_OUTPUT_BUFFER) {
            *((T*)( (char*)buff_iter->p_volatile + pos )) = out(i);
        } else {
            *((T*)( (char*)buff_iter->p + pos )) = out(i);
        }
    }

    return true;
}

template <typename T, uint N>
CM_API bool
write(uint buf_id, uint global_offset, vector_ref<uint, N> element_offset, vector<T, N> &out)
{
    static const bool conformable1 = is_fp_or_dword_type<T>::value;
    int i;
    uint pos;
    cm_list<CmEmulSys::iobuffer>::iterator buff_iter =
                                    CmEmulSys::search_buffer(buf_id&0xFF);

    if(buff_iter == CmEmulSys::iobuffers.end()) {
        printf("Error reading buffer %d: buffer %d is not registered!\n", buf_id&0xFF, buf_id&0xFF);
        exit(EXIT_FAILURE);
    }

    int width = buff_iter->width;
    int height = buff_iter->height;

    //assert(height == 1);

    if((buff_iter->bclass != GEN4_OUTPUT_BUFFER) &&
        (buff_iter->bclass != GEN4_INPUT_OUTPUT_BUFFER)) {
            printf("Error writing buffer %d: incorrect buffer type!\n", buf_id&0xFF);
            exit(EXIT_FAILURE);
    }

    if((N != 8) && (N != 16)) {
        printf("Error reading buffer %d: the vector parameter length must be 8 or 16 for DWord scattered write!\n", buf_id&0xFF);
        exit(EXIT_FAILURE);
    }

    for (i = 0; i < N; i++) {
        SIMDCF_ELEMENT_SKIP(i);
        pos = (global_offset + element_offset(i))*4;
        if (pos >= width*height) {
            break;
        }
        if(buff_iter->bclass == GEN4_INPUT_OUTPUT_BUFFER) {
            *((T*)( (char*)buff_iter->p_volatile + pos )) = out(i);
        } else {
            *((T*)( (char*)buff_iter->p + pos )) = out(i);
        }
    }

    return true;
}

template <typename T, uint N>
CM_API bool
write(uint buf_id, uint global_offset, vector_ref<uint, N> element_offset, vector_ref<T, N> out)
{
    static const bool conformable1 = is_fp_or_dword_type<T>::value;
    int i;
    uint pos;
    cm_list<CmEmulSys::iobuffer>::iterator buff_iter =
                                    CmEmulSys::search_buffer(buf_id&0xFF);

    if(buff_iter == CmEmulSys::iobuffers.end()) {
        printf("Error reading buffer %d: buffer %d is not registered!\n", buf_id&0xFF, buf_id&0xFF);
        exit(EXIT_FAILURE);
    }

    int width = buff_iter->width;
    int height = buff_iter->height;

    //assert(height == 1);

    if((buff_iter->bclass != GEN4_OUTPUT_BUFFER) &&
        (buff_iter->bclass != GEN4_INPUT_OUTPUT_BUFFER)) {
            printf("Error writing buffer %d: incorrect buffer type!\n", buf_id&0xFF);
            exit(EXIT_FAILURE);
    }

    if((N != 8) && (N != 16)) {
        printf("Error reading buffer %d: the vector parameter length must be 8 or 16 for DWord scattered write!\n", buf_id&0xFF);
        exit(EXIT_FAILURE);
    }

    for (i = 0; i < N; i++) {
        SIMDCF_ELEMENT_SKIP(i);
        pos = (global_offset + element_offset(i))*4;
        if (pos >= width*height) {
            break;
        }
        if(buff_iter->bclass == GEN4_INPUT_OUTPUT_BUFFER) {
            *((T*)( (char*)buff_iter->p_volatile + pos )) = out(i);
        } else {
            *((T*)( (char*)buff_iter->p + pos )) = out(i);
        }
    }

    return true;
}

/* This funtion performs atomic scattered DWord write to genx dataport */
template <typename T, uint N>
CM_API bool
write(uint buf_id, CmAtomicOpType op, uint global_offset, vector<uint, 8> &element_offset, vector <T, N> &src, vector<T, 8> &v)
{
    static const bool conformable1 = is_dword_type<T>::value;
    int i;
    uint pos;
    cm_list<CmEmulSys::iobuffer>::iterator buff_iter =
                                    CmEmulSys::search_buffer(buf_id&0xFF);

    if(buff_iter == CmEmulSys::iobuffers.end()) {
        printf("Error reading buffer %d: buffer %d is not registered!\n", buf_id&0xFF, buf_id&0xFF);
        exit(EXIT_FAILURE);
    }

    int width = buff_iter->width;
    int height = buff_iter->height;

    //assert(height == 1);

    if(buff_iter->bclass != GEN4_INPUT_OUTPUT_BUFFER) {
            printf("Error writing buffer %d: the buffer type must be GEN4_INPUT_OUTPUT_BUFFER for DWord atomic write!\n", buf_id&0xFF);
            exit(EXIT_FAILURE);
    }

    if(op > ATOMIC_MAXSINT) {
        printf("Error writing buffer %d: invalid opcode for DWord atomic write!\n", buf_id&0xFF);
        exit(EXIT_FAILURE);
    }

    if(op == ATOMIC_CMPXCHG) {
        if(N != 16) {
            printf("Error writing buffer %d: src length must be 16 for DWord atomic write when op is ATOMIC_CMPXCHG!\n", buf_id&0xFF);
            exit(EXIT_FAILURE);
        }
    } else {
        if(N != 8) {
            printf("Error writing buffer %d: src length must be 8 for DWord atomic write when op is not ATOMIC_CMPXCHG!\n", buf_id&0xFF);
            exit(EXIT_FAILURE);
        }
    }

    // InitializeCriticalSection(&dataport_cs);

    for (i = 0; i < 8; i++) {
        SIMDCF_ELEMENT_SKIP(i);
        pos = (global_offset + element_offset(i))*4;
        if (pos >= width*height) {
            break;
        }
        if(buff_iter->bclass == GEN4_INPUT_OUTPUT_BUFFER) {
            switch (op) {
                case ATOMIC_ADD:
                    v(i) = *((T*)((char*)buff_iter->p_volatile + pos));
                    *((T*)((char*)buff_iter->p_volatile + pos)) = v(i) + src(i);
                    break;
                case ATOMIC_SUB:
                    v(i) = *((T*)((char*)buff_iter->p_volatile + pos));
                    *((T*)((char*)buff_iter->p_volatile + pos)) = v(i) - src(i);
                    break;
                case ATOMIC_INC:
                    v(i) = *((T*)((char*)buff_iter->p_volatile + pos));
                    *((T*)((char*)buff_iter->p_volatile + pos)) = v(i) + 1;
                    break;
                case ATOMIC_DEC:
                    v(i) = *((T*)((char*)buff_iter->p_volatile + pos));
                    *((T*)((char*)buff_iter->p_volatile + pos)) = v(i) - 1;
                    break;
                case ATOMIC_MIN:
                    v(i) = *((T*)((char*)buff_iter->p_volatile + pos));
                    if(!is_fp_type<T>::value) {
                        *((T*)((char*)buff_iter->p_volatile + pos)) = (((uint) v(i) <= (uint) src(i))? v(i): src(i));
                    }
                    break;
                case ATOMIC_MAX:
                    v(i) = *((T*)((char*)buff_iter->p_volatile + pos));
                    if(!is_fp_type<T>::value) {
                        *((T*)((char*)buff_iter->p_volatile + pos)) = (((uint) v(i) >= (uint) src(i))? v(i): src(i));
                    }
                    break;
                case ATOMIC_XCHG:
                    v(i) = *((T*)((char*)buff_iter->p_volatile + pos));
                    *((T*)((char*)buff_iter->p_volatile + pos)) = src(i);
                    break;
                case ATOMIC_CMPXCHG:
                    v(i) = *((T*)((char*)buff_iter->p_volatile + pos));
                    *((T*)((char*)buff_iter->p_volatile + pos)) = ((v(i) == src(i+8))? src(i): v(i));
                    break;
                case ATOMIC_AND:
                    v(i) = *((T*)((char*)buff_iter->p_volatile + pos));
                    *((T*)((char*)buff_iter->p_volatile + pos)) = v(i) & src(i);
                    break;
                case ATOMIC_OR:
                    v(i) = *((T*)((char*)buff_iter->p_volatile + pos));
                    *((T*)((char*)buff_iter->p_volatile + pos)) = v(i) | src(i);
                    break;
                case ATOMIC_XOR:
                    v(i) = *((T*)((char*)buff_iter->p_volatile + pos));
                    *((T*)((char*)buff_iter->p_volatile + pos)) = v(i) ^ src(i);
                    break;
                case ATOMIC_MINSINT:
                    v(i) = *((T*)((char*)buff_iter->p_volatile + pos));
                    *((T*)((char*)buff_iter->p_volatile + pos)) = (((int) v(i) <= (int) src(i))? v(i): src(i));
                    break;
                case ATOMIC_MAXSINT:
                    v(i) = *((T*)((char*)buff_iter->p_volatile + pos));
                    *((T*)((char*)buff_iter->p_volatile + pos)) = (((int) v(i) >= (int) src(i))? v(i): src(i));
                    break;
                default:
                    printf("Error writing buffer %d: invalid opcode for DWord atomic write!\n", buf_id&0xFF);
                    exit(EXIT_FAILURE);
            }
        } else {
            printf("Error writing buffer %d: the buffer type must be GEN4_INPUT_OUTPUT_BUFFER for DWord atomic write!\n", buf_id&0xFF);
            exit(EXIT_FAILURE);
        }
    }

    // DeleteCriticalSection(&dataport_cs);

    return true;
}

template <typename T, uint N>
CM_API bool
write(uint buf_id, CmAtomicOpType op, uint global_offset, vector<uint, 8> &element_offset, vector <T, N> &src, vector_ref<T, 8> v)
{
    static const bool conformable1 = is_dword_type<T>::value;
    int i;
    uint pos;
    cm_list<CmEmulSys::iobuffer>::iterator buff_iter =
                                    CmEmulSys::search_buffer(buf_id&0xFF);

    if(buff_iter == CmEmulSys::iobuffers.end()) {
        printf("Error reading buffer %d: buffer %d is not registered!\n", buf_id&0xFF, buf_id&0xFF);
        exit(EXIT_FAILURE);
    }

    int width = buff_iter->width;
    int height = buff_iter->height;

    //assert(height == 1);

    if(buff_iter->bclass != GEN4_INPUT_OUTPUT_BUFFER) {
            printf("Error writing buffer %d: the buffer type must be GEN4_INPUT_OUTPUT_BUFFER for DWord atomic write!\n", buf_id&0xFF);
            exit(EXIT_FAILURE);
    }

    if(op > ATOMIC_MAXSINT) {
        printf("Error writing buffer %d: invalid opcode for DWord atomic write!\n", buf_id&0xFF);
        exit(EXIT_FAILURE);
    }

    if(op == ATOMIC_CMPXCHG) {
        if(N != 16) {
            printf("Error writing buffer %d: src length must be 16 for DWord atomic write when op is ATOMIC_CMPXCHG!\n", buf_id&0xFF);
            exit(EXIT_FAILURE);
        }
    } else {
        if(N != 8) {
            printf("Error writing buffer %d: src length must be 8 for DWord atomic write when op is not ATOMIC_CMPXCHG!\n", buf_id&0xFF);
            exit(EXIT_FAILURE);
        }
    }

    // InitializeCriticalSection(&dataport_cs);

    for (i = 0; i < 8; i++) {
        SIMDCF_ELEMENT_SKIP(i);
        pos = (global_offset + element_offset(i))*4;
        if (pos >= width*height) {
            break;
        }
        if(buff_iter->bclass == GEN4_INPUT_OUTPUT_BUFFER) {
            switch (op) {
                case ATOMIC_ADD:
                    v(i) = *((T*)((char*)buff_iter->p_volatile + pos));
                    *((T*)((char*)buff_iter->p_volatile + pos)) = v(i) + src(i);
                    break;
                case ATOMIC_SUB:
                    v(i) = *((T*)((char*)buff_iter->p_volatile + pos));
                    *((T*)((char*)buff_iter->p_volatile + pos)) = v(i) - src(i);
                    break;
                case ATOMIC_INC:
                    v(i) = *((T*)((char*)buff_iter->p_volatile + pos));
                    *((T*)((char*)buff_iter->p_volatile + pos)) = v(i) + 1;
                    break;
                case ATOMIC_DEC:
                    v(i) = *((T*)((char*)buff_iter->p_volatile + pos));
                    *((T*)((char*)buff_iter->p_volatile + pos)) = v(i) - 1;
                    break;
                case ATOMIC_MIN:
                    v(i) = *((T*)((char*)buff_iter->p_volatile + pos));
                    if(!is_fp_type<T>::value) {
                        *((T*)((char*)buff_iter->p_volatile + pos)) = (((uint) v(i) <= (uint) src(i))? v(i): src(i));
                    }
                    break;
                case ATOMIC_MAX:
                    v(i) = *((T*)((char*)buff_iter->p_volatile + pos));
                    if(!is_fp_type<T>::value) {
                        *((T*)((char*)buff_iter->p_volatile + pos)) = (((uint) v(i) >= (uint) src(i))? v(i): src(i));
                    }
                    break;
                case ATOMIC_XCHG:
                    v(i) = *((T*)((char*)buff_iter->p_volatile + pos));
                    *((T*)((char*)buff_iter->p_volatile + pos)) = src(i);
                    break;
                case ATOMIC_CMPXCHG:
                    v(i) = *((T*)((char*)buff_iter->p_volatile + pos));
                    *((T*)((char*)buff_iter->p_volatile + pos)) = ((v(i) == src(i+8))? src(i): v(i));
                    break;
                case ATOMIC_AND:
                    v(i) = *((T*)((char*)buff_iter->p_volatile + pos));
                    *((T*)((char*)buff_iter->p_volatile + pos)) = v(i) & src(i);
                    break;
                case ATOMIC_OR:
                    v(i) = *((T*)((char*)buff_iter->p_volatile + pos));
                    *((T*)((char*)buff_iter->p_volatile + pos)) = v(i) | src(i);
                    break;
                case ATOMIC_XOR:
                    v(i) = *((T*)((char*)buff_iter->p_volatile + pos));
                    *((T*)((char*)buff_iter->p_volatile + pos)) = v(i) ^ src(i);
                    break;
                case ATOMIC_MINSINT:
                    v(i) = *((T*)((char*)buff_iter->p_volatile + pos));
                    *((T*)((char*)buff_iter->p_volatile + pos)) = (((int) v(i) <= (int) src(i))? v(i): src(i));
                    break;
                case ATOMIC_MAXSINT:
                    v(i) = *((T*)((char*)buff_iter->p_volatile + pos));
                    *((T*)((char*)buff_iter->p_volatile + pos)) = (((int) v(i) >= (int) src(i))? v(i): src(i));
                    break;
                default:
                    printf("Error writing buffer %d: invalid opcode for DWord atomic write!\n", buf_id&0xFF);
                    exit(EXIT_FAILURE);
            }
        } else {
            printf("Error writing buffer %d: the buffer type must be GEN4_INPUT_OUTPUT_BUFFER for DWord atomic write!\n", buf_id&0xFF);
            exit(EXIT_FAILURE);
        }
    }

    // DeleteCriticalSection(&dataport_cs);

    return true;
}

template <typename T, uint N>
CM_API bool
write(uint buf_id, CmAtomicOpType op, uint global_offset, vector<uint, 8> &element_offset, vector <T, N> &src, int v)
{
    static const bool conformable1 = is_dword_type<T>::value;
    int i;
    uint pos;
    if (v != NULL) {
        printf("write atomic passed destination vec as int but not NULL %x\n", v);
        exit(EXIT_FAILURE);
    }
    cm_list<CmEmulSys::iobuffer>::iterator buff_iter =
                                    CmEmulSys::search_buffer(buf_id&0xFF);

    if(buff_iter == CmEmulSys::iobuffers.end()) {
        printf("Error reading buffer %d: buffer %d is not registered!\n", buf_id&0xFF, buf_id&0xFF);
        exit(EXIT_FAILURE);
    }

    int width = buff_iter->width;
    int height = buff_iter->height;

    //assert(height == 1);

    if(buff_iter->bclass != GEN4_INPUT_OUTPUT_BUFFER) {
            printf("Error writing buffer %d: the buffer type must be GEN4_INPUT_OUTPUT_BUFFER for DWord atomic write!\n", buf_id&0xFF);
            exit(EXIT_FAILURE);
    }

    if(op > ATOMIC_MAXSINT) {
        printf("Error writing buffer %d: invalid opcode for DWord atomic write!\n", buf_id&0xFF);
        exit(EXIT_FAILURE);
    }

    if(op == ATOMIC_CMPXCHG) {
        if(N != 16) {
            printf("Error writing buffer %d: src length must be 16 for DWord atomic write when op is ATOMIC_CMPXCHG!\n", buf_id&0xFF);
            exit(EXIT_FAILURE);
        }
    } else {
        if(N != 8) {
            printf("Error writing buffer %d: src length must be 8 for DWord atomic write when op is not ATOMIC_CMPXCHG!\n", buf_id&0xFF);
            exit(EXIT_FAILURE);
        }
    }

    // InitializeCriticalSection(&dataport_cs);

    for (i = 0; i < 8; i++) {
        SIMDCF_ELEMENT_SKIP(i);
        pos = (global_offset + element_offset(i))*4;
        if (pos >= width*height) {
            break;
        }
        if(buff_iter->bclass == GEN4_INPUT_OUTPUT_BUFFER) {
            switch (op) {
                case ATOMIC_ADD:
                    *((T*)((char*)buff_iter->p_volatile + pos)) = *((T*)((char*)buff_iter->p_volatile + pos)) + src(i);
                    break;
                case ATOMIC_SUB:
                    *((T*)((char*)buff_iter->p_volatile + pos)) = *((T*)((char*)buff_iter->p_volatile + pos)) - src(i);
                    break;
                case ATOMIC_INC:
                    *((T*)((char*)buff_iter->p_volatile + pos)) = *((T*)((char*)buff_iter->p_volatile + pos)) + 1;
                    break;
                case ATOMIC_DEC:
                    *((T*)((char*)buff_iter->p_volatile + pos)) = *((T*)((char*)buff_iter->p_volatile + pos)) - 1;
                    break;
                case ATOMIC_MIN:
                    if(!is_fp_type<T>::value) {
                        *((T*)((char*)buff_iter->p_volatile + pos)) = (((uint) *((T*)((char*)buff_iter->p_volatile + pos)) <= (uint) src(i))? v(i): src(i));
                    }
                    break;
                case ATOMIC_MAX:
                    if(!is_fp_type<T>::value) {
                        *((T*)((char*)buff_iter->p_volatile + pos)) = (((uint) *((T*)((char*)buff_iter->p_volatile + pos)) >= (uint) src(i))? v(i): src(i));
                    }
                    break;
                case ATOMIC_XCHG:
                    *((T*)((char*)buff_iter->p_volatile + pos)) = src(i);
                    break;
                case ATOMIC_CMPXCHG:
                    *((T*)((char*)buff_iter->p_volatile + pos)) = ((*((T*)((char*)buff_iter->p_volatile + pos)) == src(i+8))? src(i): *((T*)((char*)buff_iter->p_volatile + pos)));
                    break;
                case ATOMIC_AND:
                    *((T*)((char*)buff_iter->p_volatile + pos)) = *((T*)((char*)buff_iter->p_volatile + pos)) & src(i);
                    break;
                case ATOMIC_OR:
                    *((T*)((char*)buff_iter->p_volatile + pos)) = *((T*)((char*)buff_iter->p_volatile + pos)) | src(i);
                    break;
                case ATOMIC_XOR:
                    *((T*)((char*)buff_iter->p_volatile + pos)) = *((T*)((char*)buff_iter->p_volatile + pos)) ^ src(i);
                    break;
                case ATOMIC_MINSINT:
                    *((T*)((char*)buff_iter->p_volatile + pos)) = (((int) *((T*)((char*)buff_iter->p_volatile + pos)) <= (int) src(i))? v(i): src(i));
                    break;
                case ATOMIC_MAXSINT:
                    *((T*)((char*)buff_iter->p_volatile + pos)) = (((int) *((T*)((char*)buff_iter->p_volatile + pos)) >= (int) src(i))? v(i): src(i));
                    break;
                default:
                    printf("Error writing buffer %d: invalid opcode for DWord atomic write!\n", buf_id&0xFF);
                    exit(EXIT_FAILURE);
            }
        } else {
            printf("Error writing buffer %d: the buffer type must be GEN4_INPUT_OUTPUT_BUFFER for DWord atomic write!\n", buf_id&0xFF);
            exit(EXIT_FAILURE);
        }
    }

    // DeleteCriticalSection(&dataport_cs);

    return true;
}

template <typename T, uint N>
CM_API bool
write(uint buf_id, CmAtomicOpType op, uint global_offset, vector<uint, 8> &element_offset, vector_ref <T, N> src, vector<T, 8> &v)
{
    static const bool conformable1 = is_dword_type<T>::value;
    int i;
    uint pos;
    cm_list<CmEmulSys::iobuffer>::iterator buff_iter =
                                    CmEmulSys::search_buffer(buf_id&0xFF);

    if(buff_iter == CmEmulSys::iobuffers.end()) {
        printf("Error reading buffer %d: buffer %d is not registered!\n", buf_id&0xFF, buf_id&0xFF);
        exit(EXIT_FAILURE);
    }

    int width = buff_iter->width;
    int height = buff_iter->height;

    //assert(height == 1);

    if(buff_iter->bclass != GEN4_INPUT_OUTPUT_BUFFER) {
            printf("Error writing buffer %d: the buffer type must be GEN4_INPUT_OUTPUT_BUFFER for DWord atomic write!\n", buf_id&0xFF);
            exit(EXIT_FAILURE);
    }

    if(op > ATOMIC_MAXSINT) {
        printf("Error writing buffer %d: invalid opcode for DWord atomic write!\n", buf_id&0xFF);
        exit(EXIT_FAILURE);
    }

    if(op == ATOMIC_CMPXCHG) {
        if(N != 16) {
            printf("Error writing buffer %d: src length must be 16 for DWord atomic write when op is ATOMIC_CMPXCHG!\n", buf_id&0xFF);
            exit(EXIT_FAILURE);
        }
    } else {
        if(N != 8) {
            printf("Error writing buffer %d: src length must be 8 for DWord atomic write when op is not ATOMIC_CMPXCHG!\n", buf_id&0xFF);
            exit(EXIT_FAILURE);
        }
    }

    // InitializeCriticalSection(&dataport_cs);

    for (i = 0; i < 8; i++) {
        SIMDCF_ELEMENT_SKIP(i);
        pos = (global_offset + element_offset(i))*4;
        if (pos >= width*height) {
            break;
        }
        if(buff_iter->bclass == GEN4_INPUT_OUTPUT_BUFFER) {
            switch (op) {
                case ATOMIC_ADD:
                    v(i) = *((T*)((char*)buff_iter->p_volatile + pos));
                    *((T*)((char*)buff_iter->p_volatile + pos)) = v(i) + src(i);
                    break;
                case ATOMIC_SUB:
                    v(i) = *((T*)((char*)buff_iter->p_volatile + pos));
                    *((T*)((char*)buff_iter->p_volatile + pos)) = v(i) - src(i);
                    break;
                case ATOMIC_INC:
                    v(i) = *((T*)((char*)buff_iter->p_volatile + pos));
                    *((T*)((char*)buff_iter->p_volatile + pos)) = v(i) + 1;
                    break;
                case ATOMIC_DEC:
                    v(i) = *((T*)((char*)buff_iter->p_volatile + pos));
                    *((T*)((char*)buff_iter->p_volatile + pos)) = v(i) - 1;
                    break;
                case ATOMIC_MIN:
                    v(i) = *((T*)((char*)buff_iter->p_volatile + pos));
                    if(!is_fp_type<T>::value) {
                        *((T*)((char*)buff_iter->p_volatile + pos)) = (((uint) v(i) <= (uint) src(i))? v(i): src(i));
                    }
                    break;
                case ATOMIC_MAX:
                    v(i) = *((T*)((char*)buff_iter->p_volatile + pos));
                    if(!is_fp_type<T>::value) {
                        *((T*)((char*)buff_iter->p_volatile + pos)) = (((uint) v(i) >= (uint) src(i))? v(i): src(i));
                    }
                    break;
                case ATOMIC_XCHG:
                    v(i) = *((T*)((char*)buff_iter->p_volatile + pos));
                    *((T*)((char*)buff_iter->p_volatile + pos)) = src(i);
                    break;
                case ATOMIC_CMPXCHG:
                    v(i) = *((T*)((char*)buff_iter->p_volatile + pos));
                    *((T*)((char*)buff_iter->p_volatile + pos)) = ((v(i) == src(i+8))? src(i): v(i));
                    break;
                case ATOMIC_AND:
                    v(i) = *((T*)((char*)buff_iter->p_volatile + pos));
                    *((T*)((char*)buff_iter->p_volatile + pos)) = v(i) & src(i);
                    break;
                case ATOMIC_OR:
                    v(i) = *((T*)((char*)buff_iter->p_volatile + pos));
                    *((T*)((char*)buff_iter->p_volatile + pos)) = v(i) | src(i);
                    break;
                case ATOMIC_XOR:
                    v(i) = *((T*)((char*)buff_iter->p_volatile + pos));
                    *((T*)((char*)buff_iter->p_volatile + pos)) = v(i) ^ src(i);
                    break;
                case ATOMIC_MINSINT:
                    v(i) = *((T*)((char*)buff_iter->p_volatile + pos));
                    *((T*)((char*)buff_iter->p_volatile + pos)) = (((int) v(i) <= (int) src(i))? v(i): src(i));
                    break;
                case ATOMIC_MAXSINT:
                    v(i) = *((T*)((char*)buff_iter->p_volatile + pos));
                    *((T*)((char*)buff_iter->p_volatile + pos)) = (((int) v(i) >= (int) src(i))? v(i): src(i));
                    break;
                default:
                    printf("Error writing buffer %d: invalid opcode for DWord atomic write!\n", buf_id&0xFF);
                    exit(EXIT_FAILURE);
            }
        } else {
            printf("Error writing buffer %d: the buffer type must be GEN4_INPUT_OUTPUT_BUFFER for DWord atomic write!\n", buf_id&0xFF);
            exit(EXIT_FAILURE);
        }
    }

    // DeleteCriticalSection(&dataport_cs);

    return true;
}

template <typename T, uint N>
CM_API bool
write(uint buf_id, CmAtomicOpType op, uint global_offset, vector<uint, 8> &element_offset, vector_ref <T, N> src, vector_ref<T, 8> v)
{
    static const bool conformable1 = is_dword_type<T>::value;
    int i;
    uint pos;
    cm_list<CmEmulSys::iobuffer>::iterator buff_iter =
                                    CmEmulSys::search_buffer(buf_id&0xFF);

    if(buff_iter == CmEmulSys::iobuffers.end()) {
        printf("Error reading buffer %d: buffer %d is not registered!\n", buf_id&0xFF, buf_id&0xFF);
        exit(EXIT_FAILURE);
    }

    int width = buff_iter->width;
    int height = buff_iter->height;

    //assert(height == 1);

    if(buff_iter->bclass != GEN4_INPUT_OUTPUT_BUFFER) {
            printf("Error writing buffer %d: the buffer type must be GEN4_INPUT_OUTPUT_BUFFER for DWord atomic write!\n", buf_id&0xFF);
            exit(EXIT_FAILURE);
    }

    if(op > ATOMIC_MAXSINT) {
        printf("Error writing buffer %d: invalid opcode for DWord atomic write!\n", buf_id&0xFF);
        exit(EXIT_FAILURE);
    }

    if(op == ATOMIC_CMPXCHG) {
        if(N != 16) {
            printf("Error writing buffer %d: src length must be 16 for DWord atomic write when op is ATOMIC_CMPXCHG!\n", buf_id&0xFF);
            exit(EXIT_FAILURE);
        }
    } else {
        if(N != 8) {
            printf("Error writing buffer %d: src length must be 8 for DWord atomic write when op is not ATOMIC_CMPXCHG!\n", buf_id&0xFF);
            exit(EXIT_FAILURE);
        }
    }

    // InitializeCriticalSection(&dataport_cs);

    for (i = 0; i < 8; i++) {
        SIMDCF_ELEMENT_SKIP(i);
        pos = (global_offset + element_offset(i))*4;
        if (pos >= width*height) {
            break;
        }
        if(buff_iter->bclass == GEN4_INPUT_OUTPUT_BUFFER) {
            switch (op) {
                case ATOMIC_ADD:
                    v(i) = *((T*)((char*)buff_iter->p_volatile + pos));
                    *((T*)((char*)buff_iter->p_volatile + pos)) = v(i) + src(i);
                    break;
                case ATOMIC_SUB:
                    v(i) = *((T*)((char*)buff_iter->p_volatile + pos));
                    *((T*)((char*)buff_iter->p_volatile + pos)) = v(i) - src(i);
                    break;
                case ATOMIC_INC:
                    v(i) = *((T*)((char*)buff_iter->p_volatile + pos));
                    *((T*)((char*)buff_iter->p_volatile + pos)) = v(i) + 1;
                    break;
                case ATOMIC_DEC:
                    v(i) = *((T*)((char*)buff_iter->p_volatile + pos));
                    *((T*)((char*)buff_iter->p_volatile + pos)) = v(i) - 1;
                    break;
                case ATOMIC_MIN:
                    v(i) = *((T*)((char*)buff_iter->p_volatile + pos));
                    if(!is_fp_type<T>::value) {
                        *((T*)((char*)buff_iter->p_volatile + pos)) = (((uint) v(i) <= (uint) src(i))? v(i): src(i));
                    }
                    break;
                case ATOMIC_MAX:
                    v(i) = *((T*)((char*)buff_iter->p_volatile + pos));
                    if(!is_fp_type<T>::value) {
                        *((T*)((char*)buff_iter->p_volatile + pos)) = (((uint) v(i) >= (uint) src(i))? v(i): src(i));
                    }
                    break;
                case ATOMIC_XCHG:
                    v(i) = *((T*)((char*)buff_iter->p_volatile + pos));
                    *((T*)((char*)buff_iter->p_volatile + pos)) = src(i);
                    break;
                case ATOMIC_CMPXCHG:
                    v(i) = *((T*)((char*)buff_iter->p_volatile + pos));
                    *((T*)((char*)buff_iter->p_volatile + pos)) = ((v(i) == src(i+8))? src(i): v(i));
                    break;
                case ATOMIC_AND:
                    v(i) = *((T*)((char*)buff_iter->p_volatile + pos));
                    *((T*)((char*)buff_iter->p_volatile + pos)) = v(i) & src(i);
                    break;
                case ATOMIC_OR:
                    v(i) = *((T*)((char*)buff_iter->p_volatile + pos));
                    *((T*)((char*)buff_iter->p_volatile + pos)) = v(i) | src(i);
                    break;
                case ATOMIC_XOR:
                    v(i) = *((T*)((char*)buff_iter->p_volatile + pos));
                    *((T*)((char*)buff_iter->p_volatile + pos)) = v(i) ^ src(i);
                    break;
                case ATOMIC_MINSINT:
                    v(i) = *((T*)((char*)buff_iter->p_volatile + pos));
                    *((T*)((char*)buff_iter->p_volatile + pos)) = (((int) v(i) <= (int) src(i))? v(i): src(i));
                    break;
                case ATOMIC_MAXSINT:
                    v(i) = *((T*)((char*)buff_iter->p_volatile + pos));
                    *((T*)((char*)buff_iter->p_volatile + pos)) = (((int) v(i) >= (int) src(i))? v(i): src(i));
                    break;
                default:
                    printf("Error writing buffer %d: invalid opcode for DWord atomic write!\n", buf_id&0xFF);
                    exit(EXIT_FAILURE);
            }
        } else {
            printf("Error writing buffer %d: the buffer type must be GEN4_INPUT_OUTPUT_BUFFER for DWord atomic write!\n", buf_id&0xFF);
            exit(EXIT_FAILURE);
        }
    }

    // DeleteCriticalSection(&dataport_cs);

    return true;
}

template <typename T, uint N>
CM_API bool
write(uint buf_id, CmAtomicOpType op, uint global_offset, vector<uint, 8> &element_offset, vector_ref <T, N> src, int v)
{
    static const bool conformable1 = is_dword_type<T>::value;
    int i;
    uint pos;
    if (v != NULL) {
        printf("write atomic passed destination vec as int but not NULL %x\n", v);
        exit(EXIT_FAILURE);
    }
    cm_list<CmEmulSys::iobuffer>::iterator buff_iter =
                                    CmEmulSys::search_buffer(buf_id&0xFF);

    if(buff_iter == CmEmulSys::iobuffers.end()) {
        printf("Error reading buffer %d: buffer %d is not registered!\n", buf_id&0xFF, buf_id&0xFF);
        exit(EXIT_FAILURE);
    }

    int width = buff_iter->width;
    int height = buff_iter->height;

    //assert(height == 1);

    if(buff_iter->bclass != GEN4_INPUT_OUTPUT_BUFFER) {
            printf("Error writing buffer %d: the buffer type must be GEN4_INPUT_OUTPUT_BUFFER for DWord atomic write!\n", buf_id&0xFF);
            exit(EXIT_FAILURE);
    }

    if(op > ATOMIC_MAXSINT) {
        printf("Error writing buffer %d: invalid opcode for DWord atomic write!\n", buf_id&0xFF);
        exit(EXIT_FAILURE);
    }

    if(op == ATOMIC_CMPXCHG) {
        if(N != 16) {
            printf("Error writing buffer %d: src length must be 16 for DWord atomic write when op is ATOMIC_CMPXCHG!\n", buf_id&0xFF);
            exit(EXIT_FAILURE);
        }
    } else {
        if(N != 8) {
            printf("Error writing buffer %d: src length must be 8 for DWord atomic write when op is not ATOMIC_CMPXCHG!\n", buf_id&0xFF);
            exit(EXIT_FAILURE);
        }
    }

    // InitializeCriticalSection(&dataport_cs);

    for (i = 0; i < 8; i++) {
        SIMDCF_ELEMENT_SKIP(i);
        pos = (global_offset + element_offset(i))*4;
        if (pos >= width*height) {
            break;
        }
        if(buff_iter->bclass == GEN4_INPUT_OUTPUT_BUFFER) {
            switch (op) {
                case ATOMIC_ADD:
                    *((T*)((char*)buff_iter->p_volatile + pos)) = *((T*)((char*)buff_iter->p_volatile + pos)) + src(i);
                    break;
                case ATOMIC_SUB:
                    *((T*)((char*)buff_iter->p_volatile + pos)) = *((T*)((char*)buff_iter->p_volatile + pos)) - src(i);
                    break;
                case ATOMIC_INC:
                    *((T*)((char*)buff_iter->p_volatile + pos)) = *((T*)((char*)buff_iter->p_volatile + pos)) + 1;
                    break;
                case ATOMIC_DEC:
                    *((T*)((char*)buff_iter->p_volatile + pos)) = *((T*)((char*)buff_iter->p_volatile + pos)) - 1;
                    break;
                case ATOMIC_MIN:
                    if(!is_fp_type<T>::value) {
                        *((T*)((char*)buff_iter->p_volatile + pos)) = (((uint) *((T*)((char*)buff_iter->p_volatile + pos)) <= (uint) src(i))? *((T*)((char*)buff_iter->p_volatile + pos)): src(i));
                    }
                    break;
                case ATOMIC_MAX:
                    if(!is_fp_type<T>::value) {
                        *((T*)((char*)buff_iter->p_volatile + pos)) = (((uint) *((T*)((char*)buff_iter->p_volatile + pos)) >= (uint) src(i))? *((T*)((char*)buff_iter->p_volatile + pos)): src(i));
                    }
                    break;
                case ATOMIC_XCHG:
                    *((T*)((char*)buff_iter->p_volatile + pos)) = src(i);
                    break;
                case ATOMIC_CMPXCHG:
                    *((T*)((char*)buff_iter->p_volatile + pos)) = ((*((T*)((char*)buff_iter->p_volatile + pos)) == src(i+8))? src(i): *((T*)((char*)buff_iter->p_volatile + pos)));
                    break;
                case ATOMIC_AND:
                    *((T*)((char*)buff_iter->p_volatile + pos)) = *((T*)((char*)buff_iter->p_volatile + pos)) & src(i);
                    break;
                case ATOMIC_OR:
                    *((T*)((char*)buff_iter->p_volatile + pos)) = *((T*)((char*)buff_iter->p_volatile + pos)) | src(i);
                    break;
                case ATOMIC_XOR:
                    *((T*)((char*)buff_iter->p_volatile + pos)) = *((T*)((char*)buff_iter->p_volatile + pos)) ^ src(i);
                    break;
                case ATOMIC_MINSINT:
                    *((T*)((char*)buff_iter->p_volatile + pos)) = (((int) *((T*)((char*)buff_iter->p_volatile + pos)) <= (int) src(i))? v(i): src(i));
                    break;
                case ATOMIC_MAXSINT:
                    *((T*)((char*)buff_iter->p_volatile + pos)) = (((int) *((T*)((char*)buff_iter->p_volatile + pos)) >= (int) src(i))? v(i): src(i));
                    break;
                default:
                    printf("Error writing buffer %d: invalid opcode for DWord atomic write!\n", buf_id&0xFF);
                    exit(EXIT_FAILURE);
            }
        } else {
            printf("Error writing buffer %d: the buffer type must be GEN4_INPUT_OUTPUT_BUFFER for DWord atomic write!\n", buf_id&0xFF);
            exit(EXIT_FAILURE);
        }
    }

    // DeleteCriticalSection(&dataport_cs);

    return true;
}

template <typename T, uint N>
CM_API bool
write(uint buf_id, CmAtomicOpType op, uint global_offset, vector_ref<uint, 8> element_offset, vector <T, N> &src, vector<T, 8> &v)
{
    static const bool conformable1 = is_dword_type<T>::value;
    int i;
    uint pos;
    cm_list<CmEmulSys::iobuffer>::iterator buff_iter =
                                    CmEmulSys::search_buffer(buf_id&0xFF);

    if(buff_iter == CmEmulSys::iobuffers.end()) {
        printf("Error reading buffer %d: buffer %d is not registered!\n", buf_id&0xFF, buf_id&0xFF);
        exit(EXIT_FAILURE);
    }

    int width = buff_iter->width;
    int height = buff_iter->height;

    //assert(height == 1);

    if(buff_iter->bclass != GEN4_INPUT_OUTPUT_BUFFER) {
            printf("Error writing buffer %d: the buffer type must be GEN4_INPUT_OUTPUT_BUFFER for DWord atomic write!\n", buf_id&0xFF);
            exit(EXIT_FAILURE);
    }

    if(op > ATOMIC_MAXSINT) {
        printf("Error writing buffer %d: invalid opcode for DWord atomic write!\n", buf_id&0xFF);
        exit(EXIT_FAILURE);
    }

    if(op == ATOMIC_CMPXCHG) {
        if(N != 16) {
            printf("Error writing buffer %d: src length must be 16 for DWord atomic write when op is ATOMIC_CMPXCHG!\n", buf_id&0xFF);
            exit(EXIT_FAILURE);
        }
    } else {
        if(N != 8) {
            printf("Error writing buffer %d: src length must be 8 for DWord atomic write when op is not ATOMIC_CMPXCHG!\n", buf_id&0xFF);
            exit(EXIT_FAILURE);
        }
    }

    // InitializeCriticalSection(&dataport_cs);

    for (i = 0; i < 8; i++) {
        SIMDCF_ELEMENT_SKIP(i);
        pos = (global_offset + element_offset(i))*4;
        if (pos >= width*height) {
            break;
        }
        if(buff_iter->bclass == GEN4_INPUT_OUTPUT_BUFFER) {
            switch (op) {
                case ATOMIC_ADD:
                    v(i) = *((T*)((char*)buff_iter->p_volatile + pos));
                    *((T*)((char*)buff_iter->p_volatile + pos)) = v(i) + src(i);
                    break;
                case ATOMIC_SUB:
                    v(i) = *((T*)((char*)buff_iter->p_volatile + pos));
                    *((T*)((char*)buff_iter->p_volatile + pos)) = v(i) - src(i);
                    break;
                case ATOMIC_INC:
                    v(i) = *((T*)((char*)buff_iter->p_volatile + pos));
                    *((T*)((char*)buff_iter->p_volatile + pos)) = v(i) + 1;
                    break;
                case ATOMIC_DEC:
                    v(i) = *((T*)((char*)buff_iter->p_volatile + pos));
                    *((T*)((char*)buff_iter->p_volatile + pos)) = v(i) - 1;
                    break;
                case ATOMIC_MIN:
                    v(i) = *((T*)((char*)buff_iter->p_volatile + pos));
                    if(!is_fp_type<T>::value) {
                        *((T*)((char*)buff_iter->p_volatile + pos)) = (((uint) v(i) <= (uint) src(i))? v(i): src(i));
                    }
                    break;
                case ATOMIC_MAX:
                    v(i) = *((T*)((char*)buff_iter->p_volatile + pos));
                    if(!is_fp_type<T>::value) {
                        *((T*)((char*)buff_iter->p_volatile + pos)) = (((uint) v(i) >= (uint) src(i))? v(i): src(i));
                    }
                    break;
                case ATOMIC_XCHG:
                    v(i) = *((T*)((char*)buff_iter->p_volatile + pos));
                    *((T*)((char*)buff_iter->p_volatile + pos)) = src(i);
                    break;
                case ATOMIC_CMPXCHG:
                    v(i) = *((T*)((char*)buff_iter->p_volatile + pos));
                    *((T*)((char*)buff_iter->p_volatile + pos)) = ((v(i) == src(i+8))? src(i): v(i));
                    break;
                case ATOMIC_AND:
                    v(i) = *((T*)((char*)buff_iter->p_volatile + pos));
                    *((T*)((char*)buff_iter->p_volatile + pos)) = v(i) & src(i);
                    break;
                case ATOMIC_OR:
                    v(i) = *((T*)((char*)buff_iter->p_volatile + pos));
                    *((T*)((char*)buff_iter->p_volatile + pos)) = v(i) | src(i);
                    break;
                case ATOMIC_XOR:
                    v(i) = *((T*)((char*)buff_iter->p_volatile + pos));
                    *((T*)((char*)buff_iter->p_volatile + pos)) = v(i) ^ src(i);
                    break;
                case ATOMIC_MINSINT:
                    v(i) = *((T*)((char*)buff_iter->p_volatile + pos));
                    *((T*)((char*)buff_iter->p_volatile + pos)) = (((int) v(i) <= (int) src(i))? v(i): src(i));
                    break;
                case ATOMIC_MAXSINT:
                    v(i) = *((T*)((char*)buff_iter->p_volatile + pos));
                    *((T*)((char*)buff_iter->p_volatile + pos)) = (((int) v(i) >= (int) src(i))? v(i): src(i));
                    break;
                default:
                    printf("Error writing buffer %d: invalid opcode for DWord atomic write!\n", buf_id&0xFF);
                    exit(EXIT_FAILURE);
            }
        } else {
            printf("Error writing buffer %d: the buffer type must be GEN4_INPUT_OUTPUT_BUFFER for DWord atomic write!\n", buf_id&0xFF);
            exit(EXIT_FAILURE);
        }
    }

    // DeleteCriticalSection(&dataport_cs);

    return true;
}

template <typename T, uint N>
CM_API bool
write(uint buf_id, CmAtomicOpType op, uint global_offset, vector_ref<uint, 8> element_offset, vector <T, N> &src, vector_ref<T, 8> v)
{
    static const bool conformable1 = is_dword_type<T>::value;
    int i;
    uint pos;
    cm_list<CmEmulSys::iobuffer>::iterator buff_iter =
                                    CmEmulSys::search_buffer(buf_id&0xFF);

    if(buff_iter == CmEmulSys::iobuffers.end()) {
        printf("Error reading buffer %d: buffer %d is not registered!\n", buf_id&0xFF, buf_id&0xFF);
        exit(EXIT_FAILURE);
    }

    int width = buff_iter->width;
    int height = buff_iter->height;

    //assert(height == 1);

    if(buff_iter->bclass != GEN4_INPUT_OUTPUT_BUFFER) {
            printf("Error writing buffer %d: the buffer type must be GEN4_INPUT_OUTPUT_BUFFER for DWord atomic write!\n", buf_id&0xFF);
            exit(EXIT_FAILURE);
    }

    if(op > ATOMIC_MAXSINT) {
        printf("Error writing buffer %d: invalid opcode for DWord atomic write!\n", buf_id&0xFF);
        exit(EXIT_FAILURE);
    }

    if(op == ATOMIC_CMPXCHG) {
        if(N != 16) {
            printf("Error writing buffer %d: src length must be 16 for DWord atomic write when op is ATOMIC_CMPXCHG!\n", buf_id&0xFF);
            exit(EXIT_FAILURE);
        }
    } else {
        if(N != 8) {
            printf("Error writing buffer %d: src length must be 8 for DWord atomic write when op is not ATOMIC_CMPXCHG!\n", buf_id&0xFF);
            exit(EXIT_FAILURE);
        }
    }

    // InitializeCriticalSection(&dataport_cs);

    for (i = 0; i < 8; i++) {
        SIMDCF_ELEMENT_SKIP(i);
        pos = (global_offset + element_offset(i))*4;
        if (pos >= width*height) {
            break;
        }
        if(buff_iter->bclass == GEN4_INPUT_OUTPUT_BUFFER) {
            switch (op) {
                case ATOMIC_ADD:
                    v(i) = *((T*)((char*)buff_iter->p_volatile + pos));
                    *((T*)((char*)buff_iter->p_volatile + pos)) = v(i) + src(i);
                    break;
                case ATOMIC_SUB:
                    v(i) = *((T*)((char*)buff_iter->p_volatile + pos));
                    *((T*)((char*)buff_iter->p_volatile + pos)) = v(i) - src(i);
                    break;
                case ATOMIC_INC:
                    v(i) = *((T*)((char*)buff_iter->p_volatile + pos));
                    *((T*)((char*)buff_iter->p_volatile + pos)) = v(i) + 1;
                    break;
                case ATOMIC_DEC:
                    v(i) = *((T*)((char*)buff_iter->p_volatile + pos));
                    *((T*)((char*)buff_iter->p_volatile + pos)) = v(i) - 1;
                    break;
                case ATOMIC_MIN:
                    v(i) = *((T*)((char*)buff_iter->p_volatile + pos));
                    if(!is_fp_type<T>::value) {
                        *((T*)((char*)buff_iter->p_volatile + pos)) = (((uint) v(i) <= (uint) src(i))? v(i): src(i));
                    }
                    break;
                case ATOMIC_MAX:
                    v(i) = *((T*)((char*)buff_iter->p_volatile + pos));
                    if(!is_fp_type<T>::value) {
                        *((T*)((char*)buff_iter->p_volatile + pos)) = (((uint) v(i) >= (uint) src(i))? v(i): src(i));
                    }
                    break;
                case ATOMIC_XCHG:
                    v(i) = *((T*)((char*)buff_iter->p_volatile + pos));
                    *((T*)((char*)buff_iter->p_volatile + pos)) = src(i);
                    break;
                case ATOMIC_CMPXCHG:
                    v(i) = *((T*)((char*)buff_iter->p_volatile + pos));
                    *((T*)((char*)buff_iter->p_volatile + pos)) = ((v(i) == src(i+8))? src(i): v(i));
                    break;
                case ATOMIC_AND:
                    v(i) = *((T*)((char*)buff_iter->p_volatile + pos));
                    *((T*)((char*)buff_iter->p_volatile + pos)) = v(i) & src(i);
                    break;
                case ATOMIC_OR:
                    v(i) = *((T*)((char*)buff_iter->p_volatile + pos));
                    *((T*)((char*)buff_iter->p_volatile + pos)) = v(i) | src(i);
                    break;
                case ATOMIC_XOR:
                    v(i) = *((T*)((char*)buff_iter->p_volatile + pos));
                    *((T*)((char*)buff_iter->p_volatile + pos)) = v(i) ^ src(i);
                    break;
                case ATOMIC_MINSINT:
                    v(i) = *((T*)((char*)buff_iter->p_volatile + pos));
                    *((T*)((char*)buff_iter->p_volatile + pos)) = (((int) v(i) <= (int) src(i))? v(i): src(i));
                    break;
                case ATOMIC_MAXSINT:
                    v(i) = *((T*)((char*)buff_iter->p_volatile + pos));
                    *((T*)((char*)buff_iter->p_volatile + pos)) = (((int) v(i) >= (int) src(i))? v(i): src(i));
                    break;
                default:
                    printf("Error writing buffer %d: invalid opcode for DWord atomic write!\n", buf_id&0xFF);
                    exit(EXIT_FAILURE);
            }
        } else {
            printf("Error writing buffer %d: the buffer type must be GEN4_INPUT_OUTPUT_BUFFER for DWord atomic write!\n", buf_id&0xFF);
            exit(EXIT_FAILURE);
        }
    }

    // DeleteCriticalSection(&dataport_cs);

    return true;
}

template <typename T, uint N>
CM_API bool
write(uint buf_id, CmAtomicOpType op, uint global_offset, vector_ref<uint, 8> element_offset, vector <T, N> &src, int v)
{
    static const bool conformable1 = is_dword_type<T>::value;
    int i;
    uint pos;
    cm_list<CmEmulSys::iobuffer>::iterator buff_iter =
                                    CmEmulSys::search_buffer(buf_id&0xFF);

    if (v != NULL) {
        printf("write atomic passed destination vec as int but not NULL %x\n", v);
        exit(EXIT_FAILURE);
    }
    if(buff_iter == CmEmulSys::iobuffers.end()) {
        printf("Error reading buffer %d: buffer %d is not registered!\n", buf_id&0xFF, buf_id&0xFF);
        exit(EXIT_FAILURE);
    }

    int width = buff_iter->width;
    int height = buff_iter->height;

    //assert(height == 1);

    if(buff_iter->bclass != GEN4_INPUT_OUTPUT_BUFFER) {
            printf("Error writing buffer %d: the buffer type must be GEN4_INPUT_OUTPUT_BUFFER for DWord atomic write!\n", buf_id&0xFF);
            exit(EXIT_FAILURE);
    }

    if(op > ATOMIC_MAXSINT) {
        printf("Error writing buffer %d: invalid opcode for DWord atomic write!\n", buf_id&0xFF);
        exit(EXIT_FAILURE);
    }

    if(op == ATOMIC_CMPXCHG) {
        if(N != 16) {
            printf("Error writing buffer %d: src length must be 16 for DWord atomic write when op is ATOMIC_CMPXCHG!\n", buf_id&0xFF);
            exit(EXIT_FAILURE);
        }
    } else {
        if(N != 8) {
            printf("Error writing buffer %d: src length must be 8 for DWord atomic write when op is not ATOMIC_CMPXCHG!\n", buf_id&0xFF);
            exit(EXIT_FAILURE);
        }
    }

    // InitializeCriticalSection(&dataport_cs);

    for (i = 0; i < 8; i++) {
        SIMDCF_ELEMENT_SKIP(i);
        pos = (global_offset + element_offset(i))*4;
        if (pos >= width*height) {
            break;
        }
        if(buff_iter->bclass == GEN4_INPUT_OUTPUT_BUFFER) {
            switch (op) {
                case ATOMIC_ADD:
                    *((T*)((char*)buff_iter->p_volatile + pos)) = *((T*)((char*)buff_iter->p_volatile + pos)) + src(i);
                    break;
                case ATOMIC_SUB:
                    *((T*)((char*)buff_iter->p_volatile + pos)) = *((T*)((char*)buff_iter->p_volatile + pos)) - src(i);
                    break;
                case ATOMIC_INC:
                    *((T*)((char*)buff_iter->p_volatile + pos)) = *((T*)((char*)buff_iter->p_volatile + pos)) + 1;
                    break;
                case ATOMIC_DEC:
                    *((T*)((char*)buff_iter->p_volatile + pos)) = *((T*)((char*)buff_iter->p_volatile + pos)) - 1;
                    break;
                case ATOMIC_MIN:
                    if(!is_fp_type<T>::value) {
                        *((T*)((char*)buff_iter->p_volatile + pos)) = (((uint) *((T*)((char*)buff_iter->p_volatile + pos)) <= (uint) src(i))? *((T*)((char*)buff_iter->p_volatile + pos)): src(i));
                    }
                    break;
                case ATOMIC_MAX:
                    if(!is_fp_type<T>::value) {
                        *((T*)((char*)buff_iter->p_volatile + pos)) = (((uint) *((T*)((char*)buff_iter->p_volatile + pos)) >= (uint) src(i))? *((T*)((char*)buff_iter->p_volatile + pos)): src(i));
                    }
                    break;
                case ATOMIC_XCHG:
                    *((T*)((char*)buff_iter->p_volatile + pos)) = src(i);
                    break;
                case ATOMIC_CMPXCHG:
                    *((T*)((char*)buff_iter->p_volatile + pos)) = ((*((T*)((char*)buff_iter->p_volatile + pos)) == src(i+8))? src(i): v(i));
                    break;
                case ATOMIC_AND:
                    *((T*)((char*)buff_iter->p_volatile + pos)) = *((T*)((char*)buff_iter->p_volatile + pos)) & src(i);
                    break;
                case ATOMIC_OR:
                    *((T*)((char*)buff_iter->p_volatile + pos)) = *((T*)((char*)buff_iter->p_volatile + pos)) | src(i);
                    break;
                case ATOMIC_XOR:
                    *((T*)((char*)buff_iter->p_volatile + pos)) = *((T*)((char*)buff_iter->p_volatile + pos)) ^ src(i);
                    break;
                case ATOMIC_MINSINT:
                    *((T*)((char*)buff_iter->p_volatile + pos)) = (((int) *((T*)((char*)buff_iter->p_volatile + pos)) <= (int) src(i))? v(i): src(i));
                    break;
                case ATOMIC_MAXSINT:
                    *((T*)((char*)buff_iter->p_volatile + pos)) = (((int) *((T*)((char*)buff_iter->p_volatile + pos)) >= (int) src(i))? v(i): src(i));
                    break;
                default:
                    printf("Error writing buffer %d: invalid opcode for DWord atomic write!\n", buf_id&0xFF);
                    exit(EXIT_FAILURE);
            }
        } else {
            printf("Error writing buffer %d: the buffer type must be GEN4_INPUT_OUTPUT_BUFFER for DWord atomic write!\n", buf_id&0xFF);
            exit(EXIT_FAILURE);
        }
    }

    // DeleteCriticalSection(&dataport_cs);

    return true;
}

template <typename T, uint N>
CM_API bool
write(uint buf_id, CmAtomicOpType op, uint global_offset, vector_ref<uint, 8> element_offset, vector_ref <T, N> src, vector<T, 8> &v)
{
    static const bool conformable1 = is_dword_type<T>::value;
    int i;
    uint pos;
    cm_list<CmEmulSys::iobuffer>::iterator buff_iter =
                                    CmEmulSys::search_buffer(buf_id&0xFF);

    if(buff_iter == CmEmulSys::iobuffers.end()) {
        printf("Error reading buffer %d: buffer %d is not registered!\n", buf_id&0xFF, buf_id&0xFF);
        exit(EXIT_FAILURE);
    }

    int width = buff_iter->width;
    int height = buff_iter->height;

    //assert(height == 1);

    if(buff_iter->bclass != GEN4_INPUT_OUTPUT_BUFFER) {
            printf("Error writing buffer %d: the buffer type must be GEN4_INPUT_OUTPUT_BUFFER for DWord atomic write!\n", buf_id&0xFF);
            exit(EXIT_FAILURE);
    }

    if(op > ATOMIC_MAXSINT) {
        printf("Error writing buffer %d: invalid opcode for DWord atomic write!\n", buf_id&0xFF);
        exit(EXIT_FAILURE);
    }

    if(op == ATOMIC_CMPXCHG) {
        if(N != 16) {
            printf("Error writing buffer %d: src length must be 16 for DWord atomic write when op is ATOMIC_CMPXCHG!\n", buf_id&0xFF);
            exit(EXIT_FAILURE);
        }
    } else {
        if(N != 8) {
            printf("Error writing buffer %d: src length must be 8 for DWord atomic write when op is not ATOMIC_CMPXCHG!\n", buf_id&0xFF);
            exit(EXIT_FAILURE);
        }
    }

    // InitializeCriticalSection(&dataport_cs);

    for (i = 0; i < 8; i++) {
        SIMDCF_ELEMENT_SKIP(i);
        pos = (global_offset + element_offset(i))*4;
        if (pos >= width*height) {
            break;
        }
        if(buff_iter->bclass == GEN4_INPUT_OUTPUT_BUFFER) {
            switch (op) {
                case ATOMIC_ADD:
                    v(i) = *((T*)((char*)buff_iter->p_volatile + pos));
                    *((T*)((char*)buff_iter->p_volatile + pos)) = v(i) + src(i);
                    break;
                case ATOMIC_SUB:
                    v(i) = *((T*)((char*)buff_iter->p_volatile + pos));
                    *((T*)((char*)buff_iter->p_volatile + pos)) = v(i) - src(i);
                    break;
                case ATOMIC_INC:
                    v(i) = *((T*)((char*)buff_iter->p_volatile + pos));
                    *((T*)((char*)buff_iter->p_volatile + pos)) = v(i) + 1;
                    break;
                case ATOMIC_DEC:
                    v(i) = *((T*)((char*)buff_iter->p_volatile + pos));
                    *((T*)((char*)buff_iter->p_volatile + pos)) = v(i) - 1;
                    break;
                case ATOMIC_MIN:
                    v(i) = *((T*)((char*)buff_iter->p_volatile + pos));
                    if(!is_fp_type<T>::value) {
                        *((T*)((char*)buff_iter->p_volatile + pos)) = (((uint) v(i) <= (uint) src(i))? v(i): src(i));
                    }
                    break;
                case ATOMIC_MAX:
                    v(i) = *((T*)((char*)buff_iter->p_volatile + pos));
                    if(!is_fp_type<T>::value) {
                        *((T*)((char*)buff_iter->p_volatile + pos)) = (((uint) v(i) >= (uint) src(i))? v(i): src(i));
                    }
                    break;
                case ATOMIC_XCHG:
                    v(i) = *((T*)((char*)buff_iter->p_volatile + pos));
                    *((T*)((char*)buff_iter->p_volatile + pos)) = src(i);
                    break;
                case ATOMIC_CMPXCHG:
                    v(i) = *((T*)((char*)buff_iter->p_volatile + pos));
                    *((T*)((char*)buff_iter->p_volatile + pos)) = ((v(i) == src(i+8))? src(i): v(i));
                    break;
                case ATOMIC_AND:
                    v(i) = *((T*)((char*)buff_iter->p_volatile + pos));
                    *((T*)((char*)buff_iter->p_volatile + pos)) = v(i) & src(i);
                    break;
                case ATOMIC_OR:
                    v(i) = *((T*)((char*)buff_iter->p_volatile + pos));
                    *((T*)((char*)buff_iter->p_volatile + pos)) = v(i) | src(i);
                    break;
                case ATOMIC_XOR:
                    v(i) = *((T*)((char*)buff_iter->p_volatile + pos));
                    *((T*)((char*)buff_iter->p_volatile + pos)) = v(i) ^ src(i);
                    break;
                case ATOMIC_MINSINT:
                    v(i) = *((T*)((char*)buff_iter->p_volatile + pos));
                    *((T*)((char*)buff_iter->p_volatile + pos)) = (((int) v(i) <= (int) src(i))? v(i): src(i));
                    break;
                case ATOMIC_MAXSINT:
                    v(i) = *((T*)((char*)buff_iter->p_volatile + pos));
                    *((T*)((char*)buff_iter->p_volatile + pos)) = (((int) v(i) >= (int) src(i))? v(i): src(i));
                    break;
                default:
                    printf("Error writing buffer %d: invalid opcode for DWord atomic write!\n", buf_id&0xFF);
                    exit(EXIT_FAILURE);
            }
        } else {
            printf("Error writing buffer %d: the buffer type must be GEN4_INPUT_OUTPUT_BUFFER for DWord atomic write!\n", buf_id&0xFF);
            exit(EXIT_FAILURE);
        }
    }

    // DeleteCriticalSection(&dataport_cs);

    return true;
}

template <typename T, uint N>
CM_API bool
write(uint buf_id, CmAtomicOpType op, uint global_offset, vector_ref<uint, 8> element_offset, vector_ref <T, N> src, vector_ref<T, 8> v)
{
    static const bool conformable1 = is_dword_type<T>::value;
    int i;
    uint pos;
    cm_list<CmEmulSys::iobuffer>::iterator buff_iter =
                                    CmEmulSys::search_buffer(buf_id&0xFF);

    if(buff_iter == CmEmulSys::iobuffers.end()) {
        printf("Error reading buffer %d: buffer %d is not registered!\n", buf_id&0xFF, buf_id&0xFF);
        exit(EXIT_FAILURE);
    }

    int width = buff_iter->width;
    int height = buff_iter->height;

    //assert(height == 1);

    if(buff_iter->bclass != GEN4_INPUT_OUTPUT_BUFFER) {
            printf("Error writing buffer %d: the buffer type must be GEN4_INPUT_OUTPUT_BUFFER for DWord atomic write!\n", buf_id&0xFF);
            exit(EXIT_FAILURE);
    }

    if(op > ATOMIC_MAXSINT) {
        printf("Error writing buffer %d: invalid opcode for DWord atomic write!\n", buf_id&0xFF);
        exit(EXIT_FAILURE);
    }

    if(op == ATOMIC_CMPXCHG) {
        if(N != 16) {
            printf("Error writing buffer %d: src length must be 16 for DWord atomic write when op is ATOMIC_CMPXCHG!\n", buf_id&0xFF);
            exit(EXIT_FAILURE);
        }
    } else {
        if(N != 8) {
            printf("Error writing buffer %d: src length must be 8 for DWord atomic write when op is not ATOMIC_CMPXCHG!\n", buf_id&0xFF);
            exit(EXIT_FAILURE);
        }
    }

    // InitializeCriticalSection(&dataport_cs);

    for (i = 0; i < 8; i++) {
        SIMDCF_ELEMENT_SKIP(i);
        pos = (global_offset + element_offset(i))*4;
        if (pos >= width*height) {
            break;
        }
        if(buff_iter->bclass == GEN4_INPUT_OUTPUT_BUFFER) {
            switch (op) {
                case ATOMIC_ADD:
                    v(i) = *((T*)((char*)buff_iter->p_volatile + pos));
                    *((T*)((char*)buff_iter->p_volatile + pos)) = v(i) + src(i);
                    break;
                case ATOMIC_SUB:
                    v(i) = *((T*)((char*)buff_iter->p_volatile + pos));
                    *((T*)((char*)buff_iter->p_volatile + pos)) = v(i) - src(i);
                    break;
                case ATOMIC_INC:
                    v(i) = *((T*)((char*)buff_iter->p_volatile + pos));
                    *((T*)((char*)buff_iter->p_volatile + pos)) = v(i) + 1;
                    break;
                case ATOMIC_DEC:
                    v(i) = *((T*)((char*)buff_iter->p_volatile + pos));
                    *((T*)((char*)buff_iter->p_volatile + pos)) = v(i) - 1;
                    break;
                case ATOMIC_MIN:
                    v(i) = *((T*)((char*)buff_iter->p_volatile + pos));
                    if(!is_fp_type<T>::value) {
                        *((T*)((char*)buff_iter->p_volatile + pos)) = (((uint) v(i) <= (uint) src(i))? v(i): src(i));
                    }
                    break;
                case ATOMIC_MAX:
                    v(i) = *((T*)((char*)buff_iter->p_volatile + pos));
                    if(!is_fp_type<T>::value) {
                        *((T*)((char*)buff_iter->p_volatile + pos)) = (((uint) v(i) >= (uint) src(i))? v(i): src(i));
                    }
                    break;
                case ATOMIC_XCHG:
                    v(i) = *((T*)((char*)buff_iter->p_volatile + pos));
                    *((T*)((char*)buff_iter->p_volatile + pos)) = src(i);
                    break;
                case ATOMIC_CMPXCHG:
                    v(i) = *((T*)((char*)buff_iter->p_volatile + pos));
                    *((T*)((char*)buff_iter->p_volatile + pos)) = ((v(i) == src(i+8))? src(i): v(i));
                    break;
                case ATOMIC_AND:
                    v(i) = *((T*)((char*)buff_iter->p_volatile + pos));
                    *((T*)((char*)buff_iter->p_volatile + pos)) = v(i) & src(i);
                    break;
                case ATOMIC_OR:
                    v(i) = *((T*)((char*)buff_iter->p_volatile + pos));
                    *((T*)((char*)buff_iter->p_volatile + pos)) = v(i) | src(i);
                    break;
                case ATOMIC_XOR:
                    v(i) = *((T*)((char*)buff_iter->p_volatile + pos));
                    *((T*)((char*)buff_iter->p_volatile + pos)) = v(i) ^ src(i);
                    break;
                case ATOMIC_MINSINT:
                    v(i) = *((T*)((char*)buff_iter->p_volatile + pos));
                    *((T*)((char*)buff_iter->p_volatile + pos)) = (((int) v(i) <= (int) src(i))? v(i): src(i));
                    break;
                case ATOMIC_MAXSINT:
                    v(i) = *((T*)((char*)buff_iter->p_volatile + pos));
                    *((T*)((char*)buff_iter->p_volatile + pos)) = (((int) v(i) >= (int) src(i))? v(i): src(i));
                    break;
                default:
                    printf("Error writing buffer %d: invalid opcode for DWord atomic write!\n", buf_id&0xFF);
                    exit(EXIT_FAILURE);
            }
        } else {
            printf("Error writing buffer %d: the buffer type must be GEN4_INPUT_OUTPUT_BUFFER for DWord atomic write!\n", buf_id&0xFF);
            exit(EXIT_FAILURE);
        }
    }

    // DeleteCriticalSection(&dataport_cs);

    return true;
}

template <typename T, uint N>
CM_API bool
write(uint buf_id, CmAtomicOpType op, uint global_offset, vector_ref<uint, 8> element_offset, vector_ref <T, N> src, int v)
{
    static const bool conformable1 = is_dword_type<T>::value;
    int i;
    uint pos;
    cm_list<CmEmulSys::iobuffer>::iterator buff_iter =
                                    CmEmulSys::search_buffer(buf_id&0xFF);

    if (v != NULL) {
        printf("write atomic passed destination vec as int but not NULL %x\n", v);
        exit(EXIT_FAILURE);
    }
    if(buff_iter == CmEmulSys::iobuffers.end()) {
        printf("Error reading buffer %d: buffer %d is not registered!\n", buf_id&0xFF, buf_id&0xFF);
        exit(EXIT_FAILURE);
    }

    int width = buff_iter->width;
    int height = buff_iter->height;

    //assert(height == 1);

    if(buff_iter->bclass != GEN4_INPUT_OUTPUT_BUFFER) {
            printf("Error writing buffer %d: the buffer type must be GEN4_INPUT_OUTPUT_BUFFER for DWord atomic write!\n", buf_id&0xFF);
            exit(EXIT_FAILURE);
    }

    if(op > ATOMIC_MAXSINT) {
        printf("Error writing buffer %d: invalid opcode for DWord atomic write!\n", buf_id&0xFF);
        exit(EXIT_FAILURE);
    }

    if(op == ATOMIC_CMPXCHG) {
        if(N != 16) {
            printf("Error writing buffer %d: src length must be 16 for DWord atomic write when op is ATOMIC_CMPXCHG!\n", buf_id&0xFF);
            exit(EXIT_FAILURE);
        }
    } else {
        if(N != 8) {
            printf("Error writing buffer %d: src length must be 8 for DWord atomic write when op is not ATOMIC_CMPXCHG!\n", buf_id&0xFF);
            exit(EXIT_FAILURE);
        }
    }

    // InitializeCriticalSection(&dataport_cs);

    for (i = 0; i < 8; i++) {
        SIMDCF_ELEMENT_SKIP(i);
        pos = (global_offset + element_offset(i))*4;
        if (pos >= width*height) {
            break;
        }
        if(buff_iter->bclass == GEN4_INPUT_OUTPUT_BUFFER) {
            switch (op) {
                case ATOMIC_ADD:
                    *((T*)((char*)buff_iter->p_volatile + pos)) = *((T*)((char*)buff_iter->p_volatile + pos)) + src(i);
                    break;
                case ATOMIC_SUB:
                    *((T*)((char*)buff_iter->p_volatile + pos)) = *((T*)((char*)buff_iter->p_volatile + pos)) - src(i);
                    break;
                case ATOMIC_INC:
                    *((T*)((char*)buff_iter->p_volatile + pos)) = *((T*)((char*)buff_iter->p_volatile + pos)) + 1;
                    break;
                case ATOMIC_DEC:
                    *((T*)((char*)buff_iter->p_volatile + pos)) = *((T*)((char*)buff_iter->p_volatile + pos)) - 1;
                    break;
                case ATOMIC_MIN:
                    if(!is_fp_type<T>::value) {
                        *((T*)((char*)buff_iter->p_volatile + pos)) = (((uint) *((T*)((char*)buff_iter->p_volatile + pos)) <= (uint) src(i))? *((T*)((char*)buff_iter->p_volatile + pos)): src(i));
                    }
                    break;
                case ATOMIC_MAX:
                    if(!is_fp_type<T>::value) {
                        *((T*)((char*)buff_iter->p_volatile + pos)) = (((uint) *((T*)((char*)buff_iter->p_volatile + pos)) >= (uint) src(i))? *((T*)((char*)buff_iter->p_volatile + pos)): src(i));
                    }
                    break;
                case ATOMIC_XCHG:
                    *((T*)((char*)buff_iter->p_volatile + pos)) = src(i);
                    break;
                case ATOMIC_CMPXCHG:
                    *((T*)((char*)buff_iter->p_volatile + pos)) = ((*((T*)((char*)buff_iter->p_volatile + pos)) == src(i+8))? src(i): v(i));
                    break;
                case ATOMIC_AND:
                    *((T*)((char*)buff_iter->p_volatile + pos)) = *((T*)((char*)buff_iter->p_volatile + pos)) & src(i);
                    break;
                case ATOMIC_OR:
                    *((T*)((char*)buff_iter->p_volatile + pos)) = *((T*)((char*)buff_iter->p_volatile + pos)) | src(i);
                    break;
                case ATOMIC_XOR:
                    *((T*)((char*)buff_iter->p_volatile + pos)) = *((T*)((char*)buff_iter->p_volatile + pos)) ^ src(i);
                    break;
                case ATOMIC_MINSINT:
                    *((T*)((char*)buff_iter->p_volatile + pos)) = (((int) *((T*)((char*)buff_iter->p_volatile + pos)) <= (int) src(i))? v(i): src(i));
                    break;
                case ATOMIC_MAXSINT:
                    *((T*)((char*)buff_iter->p_volatile + pos)) = (((int) *((T*)((char*)buff_iter->p_volatile + pos)) >= (int) src(i))? v(i): src(i));
                    break;
                default:
                    printf("Error writing buffer %d: invalid opcode for DWord atomic write!\n", buf_id&0xFF);
                    exit(EXIT_FAILURE);
            }
        } else {
            printf("Error writing buffer %d: the buffer type must be GEN4_INPUT_OUTPUT_BUFFER for DWord atomic write!\n", buf_id&0xFF);
            exit(EXIT_FAILURE);
        }
    }

    // DeleteCriticalSection(&dataport_cs);

    return true;
}

template <typename T>
CM_API bool
write(uint buf_id, CmAtomicOpType op, uint global_offset, vector<uint, 8> &element_offset, int src, vector<T, 8> &v)
{
    static const bool conformable1 = is_dword_type<T>::value;
    int i;
    uint pos;
    cm_list<CmEmulSys::iobuffer>::iterator buff_iter =
                                    CmEmulSys::search_buffer(buf_id&0xFF);

    if(buff_iter == CmEmulSys::iobuffers.end()) {
        printf("Error reading buffer %d: buffer %d is not registered!\n", buf_id&0xFF, buf_id&0xFF);
        exit(EXIT_FAILURE);
    }

    int width = buff_iter->width;
    int height = buff_iter->height;

    //assert(height == 1);

    if(buff_iter->bclass != GEN4_INPUT_OUTPUT_BUFFER) {
            printf("Error writing buffer %d: the buffer type must be GEN4_INPUT_OUTPUT_BUFFER for DWord atomic write!\n", buf_id&0xFF);
            exit(EXIT_FAILURE);
    }

    if((op != ATOMIC_INC) && (op != ATOMIC_DEC)) {
        printf("Error writing buffer %d: NULL src is only allowed for ATOMIC_INC or ATOMIC_DEC!\n", buf_id&0xFF);
        exit(EXIT_FAILURE);
    }

    // InitializeCriticalSection(&dataport_cs);

    for (i = 0; i < 8; i++) {
        SIMDCF_ELEMENT_SKIP(i);
        pos = (global_offset + element_offset(i))*4;
        if (pos >= width*height) {
            break;
        }
        if(buff_iter->bclass == GEN4_INPUT_OUTPUT_BUFFER) {
            switch (op) {
                case ATOMIC_INC:
                    v(i) = *((T*)((char*)buff_iter->p_volatile + pos));
                    *((T*)((char*)buff_iter->p_volatile + pos)) = v(i) + 1;
                    break;
                case ATOMIC_DEC:
                    v(i) = *((T*)((char*)buff_iter->p_volatile + pos));
                    *((T*)((char*)buff_iter->p_volatile + pos)) = v(i) - 1;
                    break;
                default:
                    printf("Error writing buffer %d: invalid opcode for DWord atomic write!\n", buf_id&0xFF);
                    exit(EXIT_FAILURE);
            }
        } else {
            printf("Error writing buffer %d: the buffer type must be GEN4_INPUT_OUTPUT_BUFFER for DWord atomic write!\n", buf_id&0xFF);
            exit(EXIT_FAILURE);
        }
    }

    // DeleteCriticalSection(&dataport_cs);

    return true;
}

template <typename T>
CM_API bool
write(uint buf_id, CmAtomicOpType op, uint global_offset, vector<uint, 8> &element_offset, int src, vector_ref<T, 8> v)
{
    static const bool conformable1 = is_dword_type<T>::value;
    int i;
    uint pos;
    cm_list<CmEmulSys::iobuffer>::iterator buff_iter =
                                    CmEmulSys::search_buffer(buf_id&0xFF);

    if(buff_iter == CmEmulSys::iobuffers.end()) {
        printf("Error reading buffer %d: buffer %d is not registered!\n", buf_id&0xFF, buf_id&0xFF);
        exit(EXIT_FAILURE);
    }

    int width = buff_iter->width;
    int height = buff_iter->height;

    //assert(height == 1);

    if(buff_iter->bclass != GEN4_INPUT_OUTPUT_BUFFER) {
            printf("Error writing buffer %d: the buffer type must be GEN4_INPUT_OUTPUT_BUFFER for DWord atomic write!\n", buf_id&0xFF);
            exit(EXIT_FAILURE);
    }

    if((op != ATOMIC_INC) && (op != ATOMIC_DEC)) {
        printf("Error writing buffer %d: NULL src is only allowed for ATOMIC_INC or ATOMIC_DEC!\n", buf_id&0xFF);
        exit(EXIT_FAILURE);
    }

    // InitializeCriticalSection(&dataport_cs);

    for (i = 0; i < 8; i++) {
        SIMDCF_ELEMENT_SKIP(i);
        pos = (global_offset + element_offset(i))*4;
        if (pos >= width*height) {
            break;
        }
        if(buff_iter->bclass == GEN4_INPUT_OUTPUT_BUFFER) {
            switch (op) {
                case ATOMIC_INC:
                    v(i) = *((T*)((char*)buff_iter->p_volatile + pos));
                    *((T*)((char*)buff_iter->p_volatile + pos)) = v(i) + 1;
                    break;
                case ATOMIC_DEC:
                    v(i) = *((T*)((char*)buff_iter->p_volatile + pos));
                    *((T*)((char*)buff_iter->p_volatile + pos)) = v(i) - 1;
                    break;
                default:
                    printf("Error writing buffer %d: invalid opcode for DWord atomic write!\n", buf_id&0xFF);
                    exit(EXIT_FAILURE);
            }
        } else {
            printf("Error writing buffer %d: the buffer type must be GEN4_INPUT_OUTPUT_BUFFER for DWord atomic write!\n", buf_id&0xFF);
            exit(EXIT_FAILURE);
        }
    }

    // DeleteCriticalSection(&dataport_cs);

    return true;
}

template <typename T>
CM_API bool
write(uint buf_id, CmAtomicOpType op, uint global_offset, vector<uint, 8> &element_offset, int src, int v)
{
    static const bool conformable1 = is_dword_type<T>::value;
    int i;
    uint pos;
    cm_list<CmEmulSys::iobuffer>::iterator buff_iter =
                                    CmEmulSys::search_buffer(buf_id&0xFF);

    if (v != NULL) {
        printf("write atomic passed destination vec as int but not NULL %x\n", v);
        exit(EXIT_FAILURE);
    }
    if(buff_iter == CmEmulSys::iobuffers.end()) {
        printf("Error reading buffer %d: buffer %d is not registered!\n", buf_id&0xFF, buf_id&0xFF);
        exit(EXIT_FAILURE);
    }

    int width = buff_iter->width;
    int height = buff_iter->height;

    //assert(height == 1);

    if(buff_iter->bclass != GEN4_INPUT_OUTPUT_BUFFER) {
            printf("Error writing buffer %d: the buffer type must be GEN4_INPUT_OUTPUT_BUFFER for DWord atomic write!\n", buf_id&0xFF);
            exit(EXIT_FAILURE);
    }

    if((op != ATOMIC_INC) && (op != ATOMIC_DEC)) {
        printf("Error writing buffer %d: NULL src is only allowed for ATOMIC_INC or ATOMIC_DEC!\n", buf_id&0xFF);
        exit(EXIT_FAILURE);
    }

    // InitializeCriticalSection(&dataport_cs);

    for (i = 0; i < 8; i++) {
        SIMDCF_ELEMENT_SKIP(i);
        pos = (global_offset + element_offset(i))*4;
        if (pos >= width*height) {
            break;
        }
        if(buff_iter->bclass == GEN4_INPUT_OUTPUT_BUFFER) {
            switch (op) {
                case ATOMIC_INC:
                    *((T*)((char*)buff_iter->p_volatile + pos)) = *((T*)((char*)buff_iter->p_volatile + pos)) + 1;
                    break;
                case ATOMIC_DEC:
                    *((T*)((char*)buff_iter->p_volatile + pos)) = *((T*)((char*)buff_iter->p_volatile + pos)) - 1;
                    break;
                default:
                    printf("Error writing buffer %d: invalid opcode for DWord atomic write!\n", buf_id&0xFF);
                    exit(EXIT_FAILURE);
            }
        } else {
            printf("Error writing buffer %d: the buffer type must be GEN4_INPUT_OUTPUT_BUFFER for DWord atomic write!\n", buf_id&0xFF);
            exit(EXIT_FAILURE);
        }
    }

    // DeleteCriticalSection(&dataport_cs);

    return true;
}

template <typename T>
CM_API bool
write(uint buf_id, CmAtomicOpType op, uint global_offset, vector_ref<uint, 8> element_offset, int src, vector<T, 8> &v)
{
    static const bool conformable1 = is_dword_type<T>::value;
    int i;
    uint pos;
    cm_list<CmEmulSys::iobuffer>::iterator buff_iter =
                                    CmEmulSys::search_buffer(buf_id&0xFF);

    if(buff_iter == CmEmulSys::iobuffers.end()) {
        printf("Error reading buffer %d: buffer %d is not registered!\n", buf_id&0xFF, buf_id&0xFF);
        exit(EXIT_FAILURE);
    }

    int width = buff_iter->width;
    int height = buff_iter->height;

    //assert(height == 1);

    if(buff_iter->bclass != GEN4_INPUT_OUTPUT_BUFFER) {
            printf("Error writing buffer %d: the buffer type must be GEN4_INPUT_OUTPUT_BUFFER for DWord atomic write!\n", buf_id&0xFF);
            exit(EXIT_FAILURE);
    }

    if((op != ATOMIC_INC) && (op != ATOMIC_DEC)) {
        printf("Error writing buffer %d: NULL src is only allowed for ATOMIC_INC or ATOMIC_DEC!\n", buf_id&0xFF);
        exit(EXIT_FAILURE);
    }

    // InitializeCriticalSection(&dataport_cs);

    for (i = 0; i < 8; i++) {
        SIMDCF_ELEMENT_SKIP(i);
        pos = (global_offset + element_offset(i))*4;
        if (pos >= width*height) {
            break;
        }
        if(buff_iter->bclass == GEN4_INPUT_OUTPUT_BUFFER) {
            switch (op) {
                case ATOMIC_INC:
                    v(i) = *((T*)((char*)buff_iter->p_volatile + pos));
                    *((T*)((char*)buff_iter->p_volatile + pos)) = v(i) + 1;
                    break;
                case ATOMIC_DEC:
                    v(i) = *((T*)((char*)buff_iter->p_volatile + pos));
                    *((T*)((char*)buff_iter->p_volatile + pos)) = v(i) - 1;
                    break;
                default:
                    printf("Error writing buffer %d: invalid opcode for DWord atomic write!\n", buf_id&0xFF);
                    exit(EXIT_FAILURE);
            }
        } else {
            printf("Error writing buffer %d: the buffer type must be GEN4_INPUT_OUTPUT_BUFFER for DWord atomic write!\n", buf_id&0xFF);
            exit(EXIT_FAILURE);
        }
    }

    // DeleteCriticalSection(&dataport_cs);

    return true;
}

template <typename T>
CM_API bool
write(uint buf_id, CmAtomicOpType op, uint global_offset, vector_ref<uint, 8> element_offset, int src, vector_ref<T, 8> v)
{
    static const bool conformable1 = is_dword_type<T>::value;
    int i;
    uint pos;
    cm_list<CmEmulSys::iobuffer>::iterator buff_iter =
                                    CmEmulSys::search_buffer(buf_id&0xFF);

    if(buff_iter == CmEmulSys::iobuffers.end()) {
        printf("Error reading buffer %d: buffer %d is not registered!\n", buf_id&0xFF, buf_id&0xFF);
        exit(EXIT_FAILURE);
    }

    int width = buff_iter->width;
    int height = buff_iter->height;

    //assert(height == 1);

    if(buff_iter->bclass != GEN4_INPUT_OUTPUT_BUFFER) {
            printf("Error writing buffer %d: the buffer type must be GEN4_INPUT_OUTPUT_BUFFER for DWord atomic write!\n", buf_id&0xFF);
            exit(EXIT_FAILURE);
    }

    if((op != ATOMIC_INC) && (op != ATOMIC_DEC)) {
        printf("Error writing buffer %d: NULL src is only allowed for ATOMIC_INC or ATOMIC_DEC!\n", buf_id&0xFF);
        exit(EXIT_FAILURE);
    }

    // InitializeCriticalSection(&dataport_cs);

    for (i = 0; i < 8; i++) {
        SIMDCF_ELEMENT_SKIP(i);
        pos = (global_offset + element_offset(i))*4;
        if (pos >= width*height) {
            break;
        }
        if(buff_iter->bclass == GEN4_INPUT_OUTPUT_BUFFER) {
            switch (op) {
                case ATOMIC_INC:
                    v(i) = *((T*)((char*)buff_iter->p_volatile + pos));
                    *((T*)((char*)buff_iter->p_volatile + pos)) = v(i) + 1;
                    break;
                case ATOMIC_DEC:
                    v(i) = *((T*)((char*)buff_iter->p_volatile + pos));
                    *((T*)((char*)buff_iter->p_volatile + pos)) = v(i) - 1;
                    break;
                default:
                    printf("Error writing buffer %d: invalid opcode for DWord atomic write!\n", buf_id&0xFF);
                    exit(EXIT_FAILURE);
            }
        } else {
            printf("Error writing buffer %d: the buffer type must be GEN4_INPUT_OUTPUT_BUFFER for DWord atomic write!\n", buf_id&0xFF);
            exit(EXIT_FAILURE);
        }
    }

    // DeleteCriticalSection(&dataport_cs);

    return true;
}

template <typename T>
CM_API bool
write(uint buf_id, CmAtomicOpType op, uint global_offset, vector_ref<uint, 8> element_offset, int src, int v)
{
    static const bool conformable1 = is_dword_type<T>::value;
    int i;
    uint pos;
    cm_list<CmEmulSys::iobuffer>::iterator buff_iter =
                                    CmEmulSys::search_buffer(buf_id&0xFF);

    if (v != NULL) {
        printf("write atomic passed destination vec as int but not NULL %x\n", v);
        exit(EXIT_FAILURE);
    }
    if(buff_iter == CmEmulSys::iobuffers.end()) {
        printf("Error reading buffer %d: buffer %d is not registered!\n", buf_id&0xFF, buf_id&0xFF);
        exit(EXIT_FAILURE);
    }

    int width = buff_iter->width;
    int height = buff_iter->height;

    //assert(height == 1);

    if(buff_iter->bclass != GEN4_INPUT_OUTPUT_BUFFER) {
            printf("Error writing buffer %d: the buffer type must be GEN4_INPUT_OUTPUT_BUFFER for DWord atomic write!\n", buf_id&0xFF);
            exit(EXIT_FAILURE);
    }

    if((op != ATOMIC_INC) && (op != ATOMIC_DEC)) {
        printf("Error writing buffer %d: NULL src is only allowed for ATOMIC_INC or ATOMIC_DEC!\n", buf_id&0xFF);
        exit(EXIT_FAILURE);
    }

    // InitializeCriticalSection(&dataport_cs);

    for (i = 0; i < 8; i++) {
        SIMDCF_ELEMENT_SKIP(i);
        pos = (global_offset + element_offset(i))*4;
        if (pos >= width*height) {
            break;
        }
        if(buff_iter->bclass == GEN4_INPUT_OUTPUT_BUFFER) {
            switch (op) {
                case ATOMIC_INC:
                    *((T*)((char*)buff_iter->p_volatile + pos)) = *((T*)((char*)buff_iter->p_volatile + pos)) + 1;
                    break;
                case ATOMIC_DEC:
                    *((T*)((char*)buff_iter->p_volatile + pos)) = *((T*)((char*)buff_iter->p_volatile + pos)) - 1;
                    break;
                default:
                    printf("Error writing buffer %d: invalid opcode for DWord atomic write!\n", buf_id&0xFF);
                    exit(EXIT_FAILURE);
            }
        } else {
            printf("Error writing buffer %d: the buffer type must be GEN4_INPUT_OUTPUT_BUFFER for DWord atomic write!\n", buf_id&0xFF);
            exit(EXIT_FAILURE);
        }
    }

    // DeleteCriticalSection(&dataport_cs);

    return true;
}

#else /* NEW_CM_RT */

/* This funtion reads Media Block data from genx dataport */
template <typename T, uint R, uint C>
CM_API bool
read(SurfaceIndex & buf_id, int x_pos, int y_pos, matrix<T,R,C> &in)
{
    std::unique_lock<std::mutex> lock(mutexForWrite);

    uint i,j;
    uint offset;
    cm_list<CmEmulSys::iobuffer>::iterator buff_iter =
                                    CmEmulSys::search_buffer(buf_id.get_data()&0xFF);

    if(buff_iter == CmEmulSys::iobuffers.end()) {
        printf("Error reading buffer %d: buffer %d is not registered!\n", buf_id.get_data()&0xFF, buf_id.get_data()&0xFF);
        exit(EXIT_FAILURE);
    }

    int width = buff_iter->width;
    int height = buff_iter->height;

    {
        if((buff_iter->bclass != GEN4_INPUT_BUFFER) &&
            (buff_iter->bclass != GEN4_INPUT_OUTPUT_BUFFER)) {
                printf("Error reading buffer %d: the registered buffer type is not INPUT_BUFFER!\n", buf_id.get_data()&0xFF);
                exit(EXIT_FAILURE);
        }
    }

    int x_pos_a, y_pos_a;  /* Actual positions */
    int sizeofT = sizeof(T); /* Make this into a signed integer */
    uint bpp = CM_genx_bytes_per_pixel(buff_iter->pixelFormat);
    assert(((bpp - 1) & bpp) == 0); // Is power-of-2 number

    for (i = 0; i < R; i++) {
        for (j = 0; j < C; j++) {
            x_pos_a = x_pos + j * sizeof(T);
            {
                y_pos_a = y_pos + i;
            }
            // We should check the boundary condition based on sizeof(T), x_pos_a is 0-based
            // Note: Use a signed variable; otherwise sizeof(T) is unsigned
            if ((x_pos_a + sizeofT) > width) {
                // If we're trying to read outside the boundary, limit the value of x_pos_a
                // Assumption -- We don't this situation:
                //         x_pos_a  width's boundary
                //           |      |
                //           <---type(T)--->
                // At most x_pos_a+sizeof(T) is exactly at the boundary.
                x_pos_a = width;
            }
            if (y_pos_a > height - 1) {
                y_pos_a = height - 1;
            }

            if (y_pos_a < 0) {
                y_pos_a = 0;
            }

            // Surface width can be less than bpp and coordinates can be negative
            if((buff_iter->pixelFormat == YCRCB_NORMAL ||
                buff_iter->pixelFormat == YCRCB_SWAPY) &&
                x_pos_a < 0)
            {
                // If we're trying to read outside the left boundary, increase x_pos_a
                //sizeofT
                /*
                case 1 matrix 1 byte per element
                case 2 matrix 2 byte per element
                case 3 matrix 4 byte per element
                */
                if((j + (4/sizeofT)) > C) {
                    printf("Invalid matrix width [%d]for Packed format!\n", buf_id.get_data()&0xFF);
                    exit(EXIT_FAILURE);
                }
                offset = y_pos_a * width;
                if(buff_iter->pixelFormat == YCRCB_NORMAL)
                {
                    ((unsigned char*)in.get_addr(i*C+j))[0] = *((unsigned char*)((char*)buff_iter->p + offset + 0));
                    ((unsigned char*)in.get_addr(i*C+j))[1] = *((unsigned char*)((char*)buff_iter->p + offset + 1));
                    ((unsigned char*)in.get_addr(i*C+j))[2] = *((unsigned char*)((char*)buff_iter->p + offset + 0));
                    ((unsigned char*)in.get_addr(i*C+j))[3] = *((unsigned char*)((char*)buff_iter->p + offset + 3));
                }else if(buff_iter->pixelFormat == YCRCB_SWAPY)
                {
                    ((unsigned char*)in.get_addr(i*C+j))[0] = *((unsigned char*)((char*)buff_iter->p + offset + 0));
                    ((unsigned char*)in.get_addr(i*C+j))[1] = *((unsigned char*)((char*)buff_iter->p + offset + 1));
                    ((unsigned char*)in.get_addr(i*C+j))[2] = *((unsigned char*)((char*)buff_iter->p + offset + 2));
                    ((unsigned char*)in.get_addr(i*C+j))[3] = *((unsigned char*)((char*)buff_iter->p + offset + 1));
                }

                j+= (4/sizeofT);
                j--;
                continue;

            }else
            {
                if (x_pos_a < 0) {
                    // Need to align x position to bbp
                    int offset = x_pos % bpp;
                    x_pos_a -= offset;
                }
                while (x_pos_a < 0) {
                    // If we're trying to read outside the left boundary, increase x_pos_a
                    x_pos_a += bpp;
                }
            }

            if (x_pos_a >= width) {

                if((buff_iter->pixelFormat == YCRCB_NORMAL ||
                    buff_iter->pixelFormat == YCRCB_SWAPY))
                {
                    // If we're trying to read outside the left boundary, increase x_pos_a
                    //sizeofT
                    /*
                    case 1 matrix 1 byte per element
                    case 2 matrix 2 byte per element
                    case 3 matrix 4 byte per element
                    */
                    if((j + (4/sizeofT)) > C) {
                        printf("Invalid matrix width [%d] for Packed format!\n", buf_id.get_data()&0xFF);
                        exit(EXIT_FAILURE);
                    }
                    //setting offset to width - 4 for row we are processing
                    offset = y_pos_a * width + width - 4;
                    if(buff_iter->pixelFormat == YCRCB_NORMAL)
                    {
                        ((unsigned char*)in.get_addr(i*C+j))[0] = *((unsigned char*)((char*)buff_iter->p + offset + 2));
                        ((unsigned char*)in.get_addr(i*C+j))[1] = *((unsigned char*)((char*)buff_iter->p + offset + 1));
                        ((unsigned char*)in.get_addr(i*C+j))[2] = *((unsigned char*)((char*)buff_iter->p + offset + 2));
                        ((unsigned char*)in.get_addr(i*C+j))[3] = *((unsigned char*)((char*)buff_iter->p + offset + 3));
                    }else if(buff_iter->pixelFormat == YCRCB_SWAPY)
                    {
                        ((unsigned char*)in.get_addr(i*C+j))[0] = *((unsigned char*)((char*)buff_iter->p + offset + 0));
                        ((unsigned char*)in.get_addr(i*C+j))[1] = *((unsigned char*)((char*)buff_iter->p + offset + 3));
                        ((unsigned char*)in.get_addr(i*C+j))[2] = *((unsigned char*)((char*)buff_iter->p + offset + 2));
                        ((unsigned char*)in.get_addr(i*C+j))[3] = *((unsigned char*)((char*)buff_iter->p + offset + 3));
                    }

                    j+= (4/sizeofT);
                    j--;
                    continue;

                }else
                {
                    x_pos_a = x_pos_a - bpp;
                    for( uint byte_count =0; byte_count < sizeof(T); byte_count++)
                    {
                        if (x_pos_a >= width) {
                            x_pos_a = x_pos_a - bpp;
                        }
                        offset = y_pos_a * width + x_pos_a;

                        /*
                        If destination size per element is less then or equal pixel size of the surface
                        move the pixel value accross the destination elements.
                        If destination size per element is greater then pixel size of the surface
                        replicate pixel value in the destination element.
                        */
                        if(sizeof(T) <= bpp)
                        {
                            for(uint bpp_count = 0; j<C&&bpp_count<bpp ;j++, bpp_count+=sizeof(T))
                            {
                                in(i,j) = *((T*)((char*)buff_iter->p + offset + bpp_count));
                            }
                            j--;
                            break;
                        }
                        else
                        {
                            ((unsigned char*)in.get_addr(i*C+j))[byte_count] = *((unsigned char*)((char*)buff_iter->p + offset));
                        }

                        x_pos_a = x_pos_a + 1;
                    }
                    x_pos_a = width;
                }
            }
            else {
                offset = y_pos_a * width + x_pos_a;
                {
                    in(i,j) = *((T*)((char*)buff_iter->p + offset));
                }
            }
        }
    }

    return true;
}

/* This funtion reads Media Block data from genx dataport */
template <typename T, uint R, uint C>
CM_API bool
read(SurfaceIndex & buf_id, int x_pos, int y_pos, matrix_ref<T,R,C> in)
{
    return read(buf_id, GENX_MODIFIED, x_pos, y_pos, in);
}

template <typename T, uint R, uint C>
CM_API bool
read(SurfaceIndex & buf_id, CmBufferAttrib buf_attrib, int x_pos, int y_pos, matrix<T,R,C> &in)
{
    std::unique_lock<std::mutex> lock(mutexForWrite);

    int i,j;
    uint offset;
    cm_list<CmEmulSys::iobuffer>::iterator buff_iter =
                                    CmEmulSys::search_buffer(buf_id.get_data()&0xFF);

    if(buff_iter == CmEmulSys::iobuffers.end()) {
        printf("Error reading buffer %d: buffer %d is not registered!\n", buf_id.get_data()&0xFF, buf_id.get_data()&0xFF);
        exit(EXIT_FAILURE);
    }

    if((buf_attrib < GENX_TOP_FIELD) || (buf_attrib > GENX_MODIFIED_BOTTOM_FIELD)) {
        printf("Error reading buffer %d: incorrect buffer attribute %d!\n", buf_id.get_data()&0xFF, buf_attrib);
        exit(EXIT_FAILURE);
    }

    int width = buff_iter->width;
    int height = buff_iter->height;

    if(buf_attrib >= GENX_MODIFIED) {
        if(buff_iter->bclass != GEN4_INPUT_OUTPUT_BUFFER) {
            printf("Error reading buffer MODIFIED(%d): the registered buffer type is not INPUT_OUTPUT_BUFFER!\n", buf_id.get_data()&0xFF);
            exit(EXIT_FAILURE);
        }
    } else {
        if((buff_iter->bclass != GEN4_INPUT_BUFFER) &&
            (buff_iter->bclass != GEN4_INPUT_OUTPUT_BUFFER)) {
                printf("Error reading buffer %d: the registered buffer type is not INPUT_BUFFER!\n", buf_id.get_data()&0xFF);
                exit(EXIT_FAILURE);
        }
    }

    int x_pos_a, y_pos_a;  /* Actual positions */
    int sizeofT = sizeof(T); /* Make this into a signed integer */
    uint bpp = CM_genx_bytes_per_pixel(buff_iter->pixelFormat);
    assert(((bpp - 1) & bpp) == 0); // Is power-of-2 number

    for (i = 0; i < R; i++) {
        for (j = 0; j < C; j++) {
            x_pos_a = x_pos + j * sizeof(T);
            if ((buf_attrib == GENX_TOP_FIELD) ||
                (buf_attrib == GENX_MODIFIED_TOP_FIELD))
            {
                //y_pos_a = (y_pos + i) * 2;
                y_pos_a = (y_pos + i) << 1;
            }
            else
                if ((buf_attrib == GENX_BOTTOM_FIELD) ||
                    (buf_attrib == GENX_MODIFIED_BOTTOM_FIELD))
                {
                    //y_pos_a = (y_pos + i) * 2  + 1;
                    y_pos_a = 1 + ((y_pos + i) << 1);
                }
                else
                {
                    y_pos_a = y_pos + i;
                }
            // We should check the boundary condition based on sizeof(T), x_pos_a is 0-based
            // Note: Use a signed variable; otherwise sizeof(T) is unsigned
            if ((x_pos_a + sizeofT) > width) {
                // If we're trying to read outside the boundary, limit the value of x_pos_a
                // Assumption -- We don't this situation:
                //         x_pos_a  width's boundary
                //           |      |
                //           <---type(T)--->
                // At most x_pos_a+sizeof(T) is exactly at the boundary.
                x_pos_a = width;
            }
            if (y_pos_a > height - 1) {
                y_pos_a = height - 1;
            }
            // Surface width can be less than bpp and coordinates can be negative
            if (y_pos_a < 0) {
                y_pos_a = 0;
            }

            // Surface width can be less than bpp and coordinates can be negative
            if((buff_iter->pixelFormat == YCRCB_NORMAL ||
                buff_iter->pixelFormat == YCRCB_SWAPY) &&
                x_pos_a < 0)
            {
                void * temp_buffer_pointer = NULL;
                if(buf_attrib >= GENX_MODIFIED)
                {
                    temp_buffer_pointer = buff_iter->p_volatile;
                }else
                {
                    temp_buffer_pointer = buff_iter->p;
                }
                // If we're trying to read outside the left boundary, increase x_pos_a
                //sizeofT
                /*
                case 1 matrix 1 byte per element
                case 2 matrix 2 byte per element
                case 3 matrix 4 byte per element
                */
                if((j + (4/sizeofT)) > C) {
                    printf("Invalid matrix width[%d] for Packed format!\n", buf_id.get_data()&0xFF);
                    exit(EXIT_FAILURE);
                }
                offset = y_pos_a * width;
                if(buff_iter->pixelFormat == YCRCB_NORMAL)
                {
                    ((unsigned char*)in.get_addr(i*C+j))[0] = *((unsigned char*)((char*)temp_buffer_pointer + offset + 0));
                    ((unsigned char*)in.get_addr(i*C+j))[1] = *((unsigned char*)((char*)temp_buffer_pointer + offset + 1));
                    ((unsigned char*)in.get_addr(i*C+j))[2] = *((unsigned char*)((char*)temp_buffer_pointer + offset + 0));
                    ((unsigned char*)in.get_addr(i*C+j))[3] = *((unsigned char*)((char*)temp_buffer_pointer + offset + 3));
                }else if(buff_iter->pixelFormat == YCRCB_SWAPY)
                {
                    ((unsigned char*)in.get_addr(i*C+j))[0] = *((unsigned char*)((char*)temp_buffer_pointer + offset + 0));
                    ((unsigned char*)in.get_addr(i*C+j))[1] = *((unsigned char*)((char*)temp_buffer_pointer + offset + 1));
                    ((unsigned char*)in.get_addr(i*C+j))[2] = *((unsigned char*)((char*)temp_buffer_pointer + offset + 2));
                    ((unsigned char*)in.get_addr(i*C+j))[3] = *((unsigned char*)((char*)temp_buffer_pointer + offset + 1));
                }

                j+= (4/sizeofT);
                j--;
                continue;

            }else
            {
                if (x_pos_a < 0) {
                    // Need to align x position to bbp
                    int offset = x_pos % bpp;
                    x_pos_a -= offset;
                }
                while (x_pos_a < 0) {
                    // If we're trying to read outside the left boundary, increase x_pos_a
                    x_pos_a += bpp;
                }
            }

            if (x_pos_a >= width) {

                void * temp_buffer_pointer = NULL;
                if(buf_attrib >= GENX_MODIFIED)
                {
                    temp_buffer_pointer = buff_iter->p_volatile;
                }else
                {
                    temp_buffer_pointer = buff_iter->p;
                }
                if((buff_iter->pixelFormat == YCRCB_NORMAL ||
                    buff_iter->pixelFormat == YCRCB_SWAPY))
                {

                    // If we're trying to read outside the left boundary, increase x_pos_a
                    //sizeofT
                    /*
                    case 1 matrix 1 byte per element
                    case 2 matrix 2 byte per element
                    case 3 matrix 4 byte per element
                    */
                    if((j + (4/sizeofT)) > C) {
                        printf("Invalid matrix width[%d] for Packed format!\n", buf_id.get_data()&0xFF);
                        exit(EXIT_FAILURE);
                    }
                    //setting offset to width - 4 for row we are processing
                    offset = y_pos_a * width + width - 4;
                    if(buff_iter->pixelFormat == YCRCB_NORMAL)
                    {
                        ((unsigned char*)in.get_addr(i*C+j))[0] = *((unsigned char*)((char*)temp_buffer_pointer + offset + 2));
                        ((unsigned char*)in.get_addr(i*C+j))[1] = *((unsigned char*)((char*)temp_buffer_pointer + offset + 1));
                        ((unsigned char*)in.get_addr(i*C+j))[2] = *((unsigned char*)((char*)temp_buffer_pointer + offset + 2));
                        ((unsigned char*)in.get_addr(i*C+j))[3] = *((unsigned char*)((char*)temp_buffer_pointer + offset + 3));
                    }else if(buff_iter->pixelFormat == YCRCB_SWAPY)
                    {
                        ((unsigned char*)in.get_addr(i*C+j))[0] = *((unsigned char*)((char*)temp_buffer_pointer + offset + 0));
                        ((unsigned char*)in.get_addr(i*C+j))[1] = *((unsigned char*)((char*)temp_buffer_pointer + offset + 3));
                        ((unsigned char*)in.get_addr(i*C+j))[2] = *((unsigned char*)((char*)temp_buffer_pointer + offset + 2));
                        ((unsigned char*)in.get_addr(i*C+j))[3] = *((unsigned char*)((char*)temp_buffer_pointer + offset + 3));
                    }

                    j+= (4/sizeofT);
                    j--;
                    continue;

                }else
                {

                    x_pos_a = x_pos_a - bpp;
                    for( int byte_count =0; byte_count < sizeof(T); byte_count++)
                    {
                        if (x_pos_a >= width) {
                            x_pos_a = x_pos_a - bpp;
                        }
                        offset = y_pos_a * width + x_pos_a;

                        /*
                        If destination size per element is less then or equal pixel size of the surface
                        move the pixel value accross the destination elements.
                        If destination size per element is greater then pixel size of the surface
                        replicate pixel value in the destination element.
                        */
                        if(sizeof(T) <= bpp)
                        {
                            for(int bpp_count = 0; j<C&&bpp_count<bpp ;j++, bpp_count+=sizeof(T))
                            {
                                in(i,j) = *((T*)((char*)temp_buffer_pointer + offset + bpp_count));
                            }
                            j--;
                            break;
                        }
                        else
                        {
                            ((unsigned char*)in.get_addr(i*C+j))[byte_count] = *((unsigned char*)((char*)temp_buffer_pointer + offset));
                        }

                        x_pos_a = x_pos_a + 1;
                    }
                    x_pos_a = width;
                }
            }
            else {
                offset = y_pos_a * width + x_pos_a;
                if(buf_attrib >= GENX_MODIFIED) {
                    in(i,j) = *((T*)((char*)buff_iter->p_volatile + offset));
                } else {
                    in(i,j) = *((T*)((char*)buff_iter->p + offset));
                }
            }
        }
    }

    return true;
}

/* This funtion reads Media Block data from genx dataport */
template <typename T, uint R, uint C>
CM_API bool
read(SurfaceIndex & buf_id, CmBufferAttrib buf_attrib, int x_pos, int y_pos, matrix_ref<T,R,C> in)
{
    std::unique_lock<std::mutex> lock(mutexForWrite);

    int i,j;
    uint offset;
    cm_list<CmEmulSys::iobuffer>::iterator buff_iter =
                                    CmEmulSys::search_buffer(buf_id.get_data()&0xFF);

    if(buff_iter == CmEmulSys::iobuffers.end()) {
        printf("Error reading buffer %d: buffer %d is not registered!\n", buf_id.get_data()&0xFF, buf_id.get_data()&0xFF);
        exit(EXIT_FAILURE);
    }

    if((buf_attrib < GENX_TOP_FIELD) || (buf_attrib > GENX_MODIFIED_BOTTOM_FIELD)) {
        printf("Error reading buffer %d: incorrect buffer attribute %d!\n", buf_id.get_data()&0xFF, buf_attrib);
        exit(EXIT_FAILURE);
    }

    int width = buff_iter->width;
    int height = buff_iter->height;

    if(buf_attrib >= GENX_MODIFIED) {
        if(buff_iter->bclass != GEN4_INPUT_OUTPUT_BUFFER) {
            printf("Error reading buffer MODIFIED(%d): the registered buffer type is not INPUT_OUTPUT_BUFFER!\n", buf_id.get_data()&0xFF);
            exit(EXIT_FAILURE);
        }
    } else {
        if((buff_iter->bclass != GEN4_INPUT_BUFFER) &&
            (buff_iter->bclass != GEN4_INPUT_OUTPUT_BUFFER)) {
                printf("Error reading buffer %d: the registered buffer type is not INPUT_BUFFER!\n", buf_id.get_data()&0xFF);
                exit(EXIT_FAILURE);
        }
    }

    int x_pos_a, y_pos_a;  /* Actual positions */
    int sizeofT = sizeof(T); /* Make this into a signed integer */
    uint bpp = CM_genx_bytes_per_pixel(buff_iter->pixelFormat);
    assert(((bpp - 1) & bpp) == 0); // Is power-of-2 number

    for (i = 0; i < R; i++) {
        for (j = 0; j < C; j++) {
            x_pos_a = x_pos + j * sizeof(T);
            if ((buf_attrib == GENX_TOP_FIELD) ||
                (buf_attrib == GENX_MODIFIED_TOP_FIELD))
            {
                //y_pos_a = (y_pos + i) * 2;
                y_pos_a = (y_pos + i) << 1;
            }
            else
                if ((buf_attrib == GENX_BOTTOM_FIELD) ||
                    (buf_attrib == GENX_MODIFIED_BOTTOM_FIELD))
                {
                    //y_pos_a = (y_pos + i) * 2  + 1;
                    y_pos_a = 1 + ((y_pos + i) << 1);
                }
                else
                {
                    y_pos_a = y_pos + i;
                }

            // We should check the boundary condition based on sizeof(T), x_pos_a is 0-based
            // Note: Use a signed variable; otherwise sizeof(T) is unsigned
            if ((x_pos_a + sizeofT) > width) {
                // If we're trying to read outside the boundary, limit the value of x_pos_a
                // Assumption -- We don't this situation:
                //         x_pos_a  width's boundary
                //           |      |
                //           <---type(T)--->
                // At most x_pos_a+sizeof(T) is exactly at the boundary.
                x_pos_a = width;
            }
            if (y_pos_a > height - 1) {
                y_pos_a = height - 1;
            }
            // Surface width can be less than bpp and coordinates can be negative
            if (y_pos_a < 0) {
                y_pos_a = 0;
            }

            // Surface width can be less than bpp and coordinates can be negative
            if((buff_iter->pixelFormat == YCRCB_NORMAL ||
                buff_iter->pixelFormat == YCRCB_SWAPY) &&
                x_pos_a < 0)
            {

                void * temp_buffer_pointer = NULL;
                if(buf_attrib >= GENX_MODIFIED)
                {
                    temp_buffer_pointer = buff_iter->p_volatile;
                }else
                {
                    temp_buffer_pointer = buff_iter->p;
                }
                // If we're trying to read outside the left boundary, increase x_pos_a
                //sizeofT
                /*
                case 1 matrix 1 byte per element
                case 2 matrix 2 byte per element
                case 3 matrix 4 byte per element
                */
                if((j + (4/sizeofT)) > C) {
                    printf("Invalid matrix width[%d] for Packed format!\n", buf_id.get_data()&0xFF);
                    exit(EXIT_FAILURE);
                }
                offset = y_pos_a * width;

                if(buff_iter->pixelFormat == YCRCB_NORMAL)
                {
                    ((unsigned char*)in.get_addr(i*C+j))[0] = *((unsigned char*)((char*)temp_buffer_pointer + offset + 0));
                    ((unsigned char*)in.get_addr(i*C+j))[1] = *((unsigned char*)((char*)temp_buffer_pointer + offset + 1));
                    ((unsigned char*)in.get_addr(i*C+j))[2] = *((unsigned char*)((char*)temp_buffer_pointer + offset + 0));
                    ((unsigned char*)in.get_addr(i*C+j))[3] = *((unsigned char*)((char*)temp_buffer_pointer + offset + 3));
                }else if(buff_iter->pixelFormat == YCRCB_SWAPY)
                {
                    ((unsigned char*)in.get_addr(i*C+j))[0] = *((unsigned char*)((char*)temp_buffer_pointer + offset + 0));
                    ((unsigned char*)in.get_addr(i*C+j))[1] = *((unsigned char*)((char*)temp_buffer_pointer + offset + 1));
                    ((unsigned char*)in.get_addr(i*C+j))[2] = *((unsigned char*)((char*)temp_buffer_pointer + offset + 2));
                    ((unsigned char*)in.get_addr(i*C+j))[3] = *((unsigned char*)((char*)temp_buffer_pointer + offset + 1));
                }

                j+= (4/sizeofT);
                j--;
                continue;

            }else
            {
                if (x_pos_a < 0) {
                    // Need to align x position to bbp
                    int offset = x_pos % bpp;
                    x_pos_a -= offset;
                }
                while (x_pos_a < 0) {
                    // If we're trying to read outside the left boundary, increase x_pos_a
                    x_pos_a += bpp;
                }
            }

            if (x_pos_a >= width) {

                void * temp_buffer_pointer = NULL;
                if(buf_attrib >= GENX_MODIFIED)
                {
                    temp_buffer_pointer = buff_iter->p_volatile;
                }else
                {
                    temp_buffer_pointer = buff_iter->p;
                }
                if((buff_iter->pixelFormat == YCRCB_NORMAL ||
                    buff_iter->pixelFormat == YCRCB_SWAPY))
                {

                    // If we're trying to read outside the left boundary, increase x_pos_a
                    //sizeofT
                    /*
                    case 1 matrix 1 byte per element
                    case 2 matrix 2 byte per element
                    case 3 matrix 4 byte per element
                    */
                    if((j + (4/sizeofT)) > C) {
                        printf("Invalid matrix width[%d] for Packed format!\n", buf_id.get_data()&0xFF);
                        exit(EXIT_FAILURE);
                    }
                    //setting offset to width - 4 for row we are processing
                    offset = y_pos_a * width + width - 4;
                    if(buff_iter->pixelFormat == YCRCB_NORMAL)
                    {
                        ((unsigned char*)in.get_addr(i*C+j))[0] = *((unsigned char*)((char*)temp_buffer_pointer + offset + 2));
                        ((unsigned char*)in.get_addr(i*C+j))[1] = *((unsigned char*)((char*)temp_buffer_pointer + offset + 1));
                        ((unsigned char*)in.get_addr(i*C+j))[2] = *((unsigned char*)((char*)temp_buffer_pointer + offset + 2));
                        ((unsigned char*)in.get_addr(i*C+j))[3] = *((unsigned char*)((char*)temp_buffer_pointer + offset + 3));
                    }else if(buff_iter->pixelFormat == YCRCB_SWAPY)
                    {
                        ((unsigned char*)in.get_addr(i*C+j))[0] = *((unsigned char*)((char*)temp_buffer_pointer + offset + 0));
                        ((unsigned char*)in.get_addr(i*C+j))[1] = *((unsigned char*)((char*)temp_buffer_pointer + offset + 3));
                        ((unsigned char*)in.get_addr(i*C+j))[2] = *((unsigned char*)((char*)temp_buffer_pointer + offset + 2));
                        ((unsigned char*)in.get_addr(i*C+j))[3] = *((unsigned char*)((char*)temp_buffer_pointer + offset + 3));
                    }

                    j+= (4/sizeofT);
                    j--;
                    continue;

                }else
                {

                    x_pos_a = x_pos_a - bpp;
                    for( int byte_count =0; byte_count < sizeof(T); byte_count++)
                    {
                        if (x_pos_a >= width) {
                            x_pos_a = x_pos_a - bpp;
                        }
                        offset = y_pos_a * width + x_pos_a;

                        /*
                        If destination size per element is less then or equal pixel size of the surface
                        move the pixel value accross the destination elements.
                        If destination size per element is greater then pixel size of the surface
                        replicate pixel value in the destination element.
                        */
                        if(sizeof(T) <= bpp)
                        {
                            for(int bpp_count = 0; j<C&&bpp_count<bpp ;j++, bpp_count+=sizeof(T))
                            {
                                in(i,j) = *((T*)((char*)temp_buffer_pointer + offset + bpp_count));
                            }
                            j--;
                            break;
                        }
                        else
                        {
                            ((unsigned char*)in.get_addr(i*C+j))[byte_count] = *((unsigned char*)((char*)temp_buffer_pointer + offset));
                        }

                        x_pos_a = x_pos_a + 1;
                    }
                    x_pos_a = width;
                }
            }
            else {
                offset = y_pos_a * width + x_pos_a;
                if(buf_attrib >= GENX_MODIFIED) {
                    in(i,j) = *((T*)((char*)buff_iter->p_volatile + offset));
                } else {
                    in(i,j) = *((T*)((char*)buff_iter->p + offset));
                }
            }
        }
    }

    return true;
}

/* This funtion writes Media Block data to genx dataport */
template <typename T, uint R, uint C>
CM_API bool
write(SurfaceIndex & buf_id, int x_pos, int y_pos, const matrix<T,R,C> &out)
{
    std::unique_lock<std::mutex> lk(mutexForWrite);

    uint i,j;
    uint offset;
    cm_list<CmEmulSys::iobuffer>::iterator buff_iter =
                                    CmEmulSys::search_buffer(buf_id.get_data()&0xFF);

    if(buff_iter == CmEmulSys::iobuffers.end()) {
        printf("Error writing buffer %d: buffer %d is not registered!\n", buf_id.get_data()&0xFF, buf_id.get_data()&0xFF);
        exit(EXIT_FAILURE);
    }

    int width = buff_iter->width;
    int height = buff_iter->height;

    if((x_pos % 4) != 0) {
        printf("Error writing buffer %d: X-coordinate must be 4-byte aligned!\n", buf_id.get_data()&0xFF);
        exit(EXIT_FAILURE);
    }
    int sizeofT = sizeof(T); /* Make this into a signed integer */
    if(((C * sizeofT) % 4) != 0) {
        printf("Error writing buffer %d: input matrix width must be 4-byte aligned!\n", buf_id.get_data()&0xFF);
        exit(EXIT_FAILURE);
    }

    if((buff_iter->bclass != GEN4_OUTPUT_BUFFER) &&
        (buff_iter->bclass != GEN4_INPUT_OUTPUT_BUFFER)) {
        printf("Error writing buffer %d: incorrect buffer type!\n", buf_id.get_data()&0xFF);
    }

    uint x_pos_a, y_pos_a;  /* Actual positions */
    uint bpp = CM_genx_bytes_per_pixel(buff_iter->pixelFormat);
    assert(((bpp - 1) & bpp) == 0); // Is power-of-2 number

    for (i = 0; i < R; i++) {
        for (j = 0; j < C; j++) {
            x_pos_a = x_pos + j * sizeofT;
            {
                y_pos_a = y_pos + i;
            }
            if ((int)x_pos_a < 0) {
                continue;
            }
            if ((int)y_pos_a < 0) {
                continue;
            }
            if ((int)(x_pos_a + sizeofT) > width) {
                continue;
            }
            if ((int)y_pos_a > height - 1) {
                continue;
            }
            offset = y_pos_a * width + x_pos_a;
            if(buff_iter->bclass == GEN4_INPUT_OUTPUT_BUFFER) {
                *((T*)( (char*)buff_iter->p_volatile + offset )) = out(i,j);
            } else {
                *((T*)( (char*)buff_iter->p + offset )) = out(i,j);
            }
        }
    }

    return true;
}

/* This funtion writes Media Block data to genx dataport */
template <typename T, uint R, uint C>
CM_API bool
write(SurfaceIndex & buf_id, int x_pos, int y_pos, const matrix_ref<T,R,C> out)
{
    std::unique_lock<std::mutex> lk(mutexForWrite);

    int i,j;
    uint offset;
    cm_list<CmEmulSys::iobuffer>::iterator buff_iter =
                                    CmEmulSys::search_buffer(buf_id.get_data()&0xFF);

    if(buff_iter == CmEmulSys::iobuffers.end()) {
        printf("Error writing buffer %d: buffer %d is not registered!\n", buf_id.get_data()&0xFF, buf_id.get_data()&0xFF);
        exit(EXIT_FAILURE);
    }

    int width = buff_iter->width;
    int height = buff_iter->height;

    if((x_pos % 4) != 0) {
        printf("Error writing buffer %d: X-coordinate must be 4-byte aligned!\n", buf_id.get_data()&0xFF);
        exit(EXIT_FAILURE);
    }
    int sizeofT = sizeof(T); /* Make this into a signed integer */
    if(((C * sizeofT) % 4) != 0) {
        printf("Error writing buffer %d: input matrix width must be 4-byte aligned!\n", buf_id.get_data()&0xFF);
        exit(EXIT_FAILURE);
    }

    if((buff_iter->bclass != GEN4_OUTPUT_BUFFER) &&
        (buff_iter->bclass != GEN4_INPUT_OUTPUT_BUFFER)) {
            printf("Error writing buffer %d: incorrect buffer type!\n", buf_id.get_data()&0xFF);
    }

    uint x_pos_a, y_pos_a;  /* Actual positions */
    uint bpp = CM_genx_bytes_per_pixel(buff_iter->pixelFormat);
    assert(((bpp - 1) & bpp) == 0); // Is power-of-2 number

    for (i = 0; i < R; i++) {
        for (j = 0; j < C; j++) {
            x_pos_a = x_pos + j * sizeofT;
            {
                y_pos_a = y_pos + i;
            }
            if ((int)x_pos_a < 0) {
                continue;
            }
            if ((int)y_pos_a < 0) {
                continue;
            }
            if ((x_pos_a + sizeofT) > width) {
                continue;
            }
            if (y_pos_a > height - 1) {
                continue;
            }
            offset = y_pos_a * width + x_pos_a;
            if(buff_iter->bclass == GEN4_INPUT_OUTPUT_BUFFER) {
                *((T*)( (char*)buff_iter->p_volatile + offset )) = out(i,j);
            } else {
                *((T*)( (char*)buff_iter->p + offset )) = out(i,j);
            }
        }
    }

    return true;
}

/* This funtion writes Media Block data to genx dataport */
template <typename T, uint R, uint C>
CM_API bool
write(SurfaceIndex & buf_id, CmBufferAttrib buf_attrib, int x_pos, int y_pos, const matrix<T,R,C> &out)
{
    std::unique_lock<std::mutex> lk(mutexForWrite);

    int i,j;
    uint offset;
    cm_list<CmEmulSys::iobuffer>::iterator buff_iter =
                                    CmEmulSys::search_buffer(buf_id.get_data()&0xFF);

    if(buff_iter == CmEmulSys::iobuffers.end()) {
        printf("Error writing buffer %d: buffer %d is not registered!\n", buf_id.get_data()&0xFF, buf_id.get_data()&0xFF);
        exit(EXIT_FAILURE);
    }

    if((buf_attrib != GENX_TOP_FIELD) && (buf_attrib != GENX_BOTTOM_FIELD)) {
        printf("Error writing buffer %d: incorrect buffer attribute %d!\n", buf_id.get_data()&0xFF, buf_attrib);
        exit(EXIT_FAILURE);
    }

    int width = buff_iter->width;
    int height = buff_iter->height;

    if((x_pos % 4) != 0) {
        printf("Error writing buffer %d: X-coordinate must be 4-byte aligned!\n", buf_id.get_data()&0xFF);
        exit(EXIT_FAILURE);
    }
    int sizeofT = sizeof(T); /* Make this into a signed integer */
    if(((C * sizeofT) % 4) != 0) {
        printf("Error writing buffer %d: input matrix width must be 4-byte aligned!\n", buf_id.get_data()&0xFF);
        exit(EXIT_FAILURE);
    }

    if((buff_iter->bclass != GEN4_OUTPUT_BUFFER) &&
        (buff_iter->bclass != GEN4_INPUT_OUTPUT_BUFFER)) {
        printf("Error writing buffer %d: incorrect buffer type!\n", buf_id.get_data()&0xFF);
    }

    uint x_pos_a, y_pos_a;  /* Actual positions */
    uint bpp = CM_genx_bytes_per_pixel(buff_iter->pixelFormat);
    assert(((bpp - 1) & bpp) == 0); // Is power-of-2 number

    for (i = 0; i < R; i++) {
        for (j = 0; j < C; j++) {
            x_pos_a = x_pos + j * sizeof(T);
            if ((buf_attrib == GENX_TOP_FIELD) ||
                (buf_attrib == GENX_MODIFIED_TOP_FIELD))
            {
                //y_pos_a = (y_pos + i) * 2;
                y_pos_a = (y_pos + i) << 1;
            }
            else
                if ((buf_attrib == GENX_BOTTOM_FIELD) ||
                    (buf_attrib == GENX_MODIFIED_BOTTOM_FIELD))
                {
                    //y_pos_a = (y_pos + i) * 2  + 1;
                    y_pos_a = 1 + ((y_pos + i) << 1);
                }
                else
                {
                    y_pos_a = y_pos + i;
                }

            if ((int)x_pos_a < 0) {
                continue;
            }
            if ((int)y_pos_a < 0) {
                continue;
            }
            if ((x_pos_a + sizeofT) > width) {
                continue;
            }
            if (y_pos_a > height - 1) {
                continue;
            }
            offset = y_pos_a * width + x_pos_a;
            if(buff_iter->bclass == GEN4_INPUT_OUTPUT_BUFFER) {
                *((T*)( (char*)buff_iter->p_volatile + offset )) = out(i,j);
            } else {
                *((T*)( (char*)buff_iter->p + offset )) = out(i,j);
            }
        }
    }

    return true;
}

/* This funtion writes Media Block data to genx dataport */
template <typename T, uint R, uint C>
CM_API bool
write(SurfaceIndex & buf_id, CmBufferAttrib buf_attrib, int x_pos, int y_pos, const matrix_ref<T,R,C> out)
{
    std::unique_lock<std::mutex> lk(mutexForWrite);

    int i,j;
    uint offset;
    cm_list<CmEmulSys::iobuffer>::iterator buff_iter =
                                    CmEmulSys::search_buffer(buf_id.get_data()&0xFF);

    if(buff_iter == CmEmulSys::iobuffers.end()) {
        printf("Error writing buffer %d: buffer %d is not registered!\n", buf_id.get_data()&0xFF, buf_id.get_data()&0xFF);
        exit(EXIT_FAILURE);
    }

    if((buf_attrib != GENX_TOP_FIELD) && (buf_attrib != GENX_BOTTOM_FIELD)) {
        printf("Error writing buffer %d: incorrect buffer attribute %d!\n", buf_id.get_data()&0xFF, buf_attrib);
        exit(EXIT_FAILURE);
    }

    int width = buff_iter->width;
    int height = buff_iter->height;

    if((x_pos % 4) != 0) {
        printf("Error writing buffer %d: X-coordinate must be 4-byte aligned!\n", buf_id.get_data()&0xFF);
        exit(EXIT_FAILURE);
    }
    int sizeofT = sizeof(T); /* Make this into a signed integer */
    if(((C * sizeofT) % 4) != 0) {
        printf("Error writing buffer %d: input matrix width must be 4-byte aligned!\n", buf_id.get_data()&0xFF);
        exit(EXIT_FAILURE);
    }

    if((buff_iter->bclass != GEN4_OUTPUT_BUFFER) &&
        (buff_iter->bclass != GEN4_INPUT_OUTPUT_BUFFER)) {
        printf("Error writing buffer %d: incorrect buffer type!\n", buf_id.get_data()&0xFF);
    }

    uint x_pos_a, y_pos_a;  /* Actual positions */
    uint bpp = CM_genx_bytes_per_pixel(buff_iter->pixelFormat);
    assert(((bpp - 1) & bpp) == 0); // Is power-of-2 number

    for (i = 0; i < R; i++) {
        for (j = 0; j < C; j++) {
            x_pos_a = x_pos + j * sizeof(T);
            if ((buf_attrib == GENX_TOP_FIELD) ||
                (buf_attrib == GENX_MODIFIED_TOP_FIELD))
            {
                //y_pos_a = (y_pos + i) * 2;
                y_pos_a = (y_pos + i) << 1;
            }
            else
                if ((buf_attrib == GENX_BOTTOM_FIELD) ||
                    (buf_attrib == GENX_MODIFIED_BOTTOM_FIELD))
                {
                    //y_pos_a = (y_pos + i) * 2  + 1;
                    y_pos_a = 1 + ((y_pos + i) << 1);
                }
                else
                {
                    y_pos_a = y_pos + i;
                }

            if ((int)x_pos_a < 0) {
                continue;
            }
            if ((int)y_pos_a < 0) {
                continue;
            }
            if ((x_pos_a + sizeofT) > width) {
                continue;
            }
            if (y_pos_a > height - 1) {
                continue;
            }
            offset = y_pos_a * width + x_pos_a;
            if(buff_iter->bclass == GEN4_INPUT_OUTPUT_BUFFER) {
                *((T*)( (char*)buff_iter->p_volatile + offset )) = out(i,j);
            } else {
                *((T*)( (char*)buff_iter->p + offset )) = out(i,j);
            }
        }
    }

    return true;
}

template <typename T, uint R, uint C>
CM_API bool
read_plane(SurfaceIndex  buf_id, CmSurfacePlaneIndex plane_id, int x_pos, int y_pos, matrix<T,R,C> &in)
{
    return read_plane(buf_id, GENX_MODIFIED, plane_id, x_pos, y_pos, in);
}

template <typename T, uint R, uint C>
CM_API bool
read_plane(SurfaceIndex  buf_id, CmSurfacePlaneIndex plane_id, int x_pos, int y_pos, matrix_ref<T,R,C> in)
{
    return read_plane(buf_id, GENX_MODIFIED, plane_id, x_pos, y_pos, in);
}

template <typename T, uint R, uint C>
CM_API bool
read_plane(SurfaceIndex  buf_id, CmBufferAttrib buf_attrib, CmSurfacePlaneIndex plane_id, int x_pos, int y_pos, matrix<T,R,C> &in)
{
    if(plane_id < GENX_SURFACE_Y_PLANE || plane_id > GENX_SURFACE_V_PLANE) {
        printf("Invalid plane index for surface %d!\n", buf_id.get_data()&0xFF);
        exit(EXIT_FAILURE);
    }
    return read<T, R, C>(buf_id + plane_id, buf_attrib, x_pos, y_pos, in);
}

template <typename T, uint R, uint C>
CM_API bool
read_plane(SurfaceIndex  buf_id, CmBufferAttrib buf_attrib, CmSurfacePlaneIndex plane_id, int x_pos, int y_pos, matrix_ref<T,R,C> in)
{
    if(plane_id < GENX_SURFACE_Y_PLANE || plane_id > GENX_SURFACE_V_PLANE) {
        printf("Invalid plane index for surface %d!\n", buf_id.get_data()&0xFF);
        exit(EXIT_FAILURE);
    }
    return read<T, R, C>(buf_id + plane_id, buf_attrib, x_pos, y_pos, in);
}

template <typename T, uint R, uint C>
CM_API bool
write_plane(SurfaceIndex  buf_id, CmSurfacePlaneIndex plane_id, int x_pos, int y_pos, const matrix<T,R,C> &out)
{
    if(plane_id < GENX_SURFACE_Y_PLANE || plane_id > GENX_SURFACE_V_PLANE) {
        printf("Invalid plane index for surface %d!\n", buf_id.get_data()&0xFF);
        exit(EXIT_FAILURE);
    }
    return write<T, R, C>(buf_id + plane_id, x_pos, y_pos, out);
}

template <typename T, uint R, uint C>
CM_API bool
write_plane(SurfaceIndex  buf_id, CmSurfacePlaneIndex plane_id, int x_pos, int y_pos, const matrix_ref<T,R,C> out)
{
    if(plane_id < GENX_SURFACE_Y_PLANE || plane_id > GENX_SURFACE_V_PLANE) {
        printf("Invalid plane index for surface %d!\n", buf_id.get_data()&0xFF);
        exit(EXIT_FAILURE);
    }
    return write<T, R, C>(buf_id + plane_id, x_pos, y_pos, out);
}

template <typename T, uint R, uint C>
CM_API bool
write_plane(SurfaceIndex  buf_id, CmBufferAttrib buf_attrib, CmSurfacePlaneIndex plane_id, int x_pos, int y_pos, const matrix<T,R,C> &out)
{
    if(plane_id < GENX_SURFACE_Y_PLANE || plane_id > GENX_SURFACE_V_PLANE) {
        printf("Invalid plane index for surface %d!\n", buf_id.get_data()&0xFF);
        exit(EXIT_FAILURE);
    }
    return write<T, R, C>(buf_id + plane_id, buf_attrib, x_pos, y_pos, out);
}

template <typename T, uint R, uint C>
CM_API bool
write_plane(SurfaceIndex  buf_id, CmBufferAttrib buf_attrib, CmSurfacePlaneIndex plane_id, int x_pos, int y_pos, const matrix_ref<T,R,C> out)
{
    if(plane_id < GENX_SURFACE_Y_PLANE || plane_id > GENX_SURFACE_V_PLANE) {
        printf("Invalid plane index for surface %d!\n", buf_id.get_data()&0xFF);
        exit(EXIT_FAILURE);
    }
    return write<T, R, C>(buf_id + plane_id, buf_attrib, x_pos, y_pos, out);
}

/* This funtion reads OWord Block data from genx dataport */
template <typename T, uint S>
CM_API bool
read(SurfaceIndex & buf_id, int offset, vector<T,S> &in)
{
    return read(buf_id, GENX_MODIFIED, offset, in);
}

/* This funtion reads OWord Block data from genx dataport */
template <typename T, uint S>
CM_API bool
read(SurfaceIndex & buf_id, int offset, vector_ref<T,S> in)
{
    return read(buf_id, GENX_MODIFIED, offset, in);
}

/* This funtion reads OWord Block data from genx dataport */

template <typename T, uint S>
CM_API bool
read(SurfaceIndex & buf_id, CmBufferAttrib buf_attrib, int offset, vector<T,S> &in)
{
    std::unique_lock<std::mutex> lock(mutexForWrite);

    int i;
    uint pos;
    cm_list<CmEmulSys::iobuffer>::iterator buff_iter =
        CmEmulSys::search_buffer(buf_id.get_data()&0xFF);

    if(buff_iter == CmEmulSys::iobuffers.end()) {
        printf("Error reading buffer %d: buffer %d is not registered!\n", buf_id.get_data()&0xFF, buf_id.get_data()&0xFF);
        exit(EXIT_FAILURE);
    }

    if(buf_attrib != GENX_MODIFIED && buf_attrib != GENX_DWALIGNED &&
       buf_attrib != GENX_MODIFIED_DWALIGNED && buf_attrib != GENX_CONSTANT &&
       buf_attrib != GENX_CONSTANT_DWALIGNED) {
        printf("Error reading buffer %d: incorrect buffer attribute %d!\n", buf_id.get_data()&0xFF, buf_attrib);
        exit(EXIT_FAILURE);
    }

    int width = buff_iter->width;
    int height = buff_iter->height;

    if(buf_attrib != GENX_DWALIGNED && buf_attrib != GENX_MODIFIED_DWALIGNED &&
       buf_attrib != GENX_CONSTANT_DWALIGNED) {
        if((offset % 16) != 0) {
            printf("Error reading buffer %d: offset must be 16-byte aligned!\n", buf_id.get_data()&0xFF);
            exit(EXIT_FAILURE);
        }
    }
    else {
        if((offset % 4) != 0) {
            printf("Error reading buffer %d: offset must be 4-byte aligned!\n", buf_id.get_data()&0xFF);
            exit(EXIT_FAILURE);
        }
    }

    assert(height == 1);

    char* buff;

    if(buf_attrib == GENX_MODIFIED || buf_attrib == GENX_MODIFIED_DWALIGNED)
    {
        if(buff_iter->bclass != GEN4_INPUT_OUTPUT_BUFFER) {
            printf("Error reading buffer MODIFIED(%d): the registered buffer type is not INPUT_OUTPUT_BUFFER!\n", buf_id.get_data()&0xFF);
            exit(EXIT_FAILURE);
        }

        buff = (char*) buff_iter->p_volatile;
    }
    else
    {
        buff = (char*) buff_iter->p;
    }

    int sizeofT = sizeof(T); /* Make this into a signed integer */
    for (i = 0; i < S; i++) {
        pos = offset + i * sizeofT;
        if (pos >= width) {
            in(i) = 0;
        } else {
            {
                in(i) = *((T*)(buff + pos));
            }
        }
    }

    return true;
}

/* This funtion reads OWord Block data from genx dataport */
template <typename T, uint S>
CM_API bool
read(SurfaceIndex & buf_id, CmBufferAttrib buf_attrib, int offset, vector_ref<T,S> in)
{
    std::unique_lock<std::mutex> lock(mutexForWrite);

    int i;
    uint pos;
    cm_list<CmEmulSys::iobuffer>::iterator buff_iter =
                                    CmEmulSys::search_buffer(buf_id.get_data()&0xFF);

    if(buff_iter == CmEmulSys::iobuffers.end()) {
        printf("Error reading buffer %d: buffer %d is not registered!\n", buf_id.get_data()&0xFF, buf_id.get_data()&0xFF);
        exit(EXIT_FAILURE);
    }

    if(buf_attrib != GENX_MODIFIED && buf_attrib != GENX_DWALIGNED &&
       buf_attrib != GENX_MODIFIED_DWALIGNED && buf_attrib != GENX_CONSTANT &&
       buf_attrib != GENX_CONSTANT_DWALIGNED) {
        printf("Error reading buffer %d: incorrect buffer attribute %d!\n", buf_id.get_data()&0xFF, buf_attrib);
        exit(EXIT_FAILURE);
    }

    int width = buff_iter->width;
    int height = buff_iter->height;

    if(buf_attrib != GENX_DWALIGNED && buf_attrib != GENX_MODIFIED_DWALIGNED &&
       buf_attrib != GENX_CONSTANT_DWALIGNED) {
        if((offset % 16) != 0) {
            printf("Error reading buffer %d: offset must be 16-byte aligned!\n", buf_id.get_data()&0xFF);
            exit(EXIT_FAILURE);
        }
    }
    else {
        if((offset % 4) != 0) {
            printf("Error reading buffer %d: offset must be 4-byte aligned!\n", buf_id.get_data()&0xFF);
            exit(EXIT_FAILURE);
        }
    }

    assert(height == 1);

    char* buff;

    if(buf_attrib == GENX_MODIFIED || buf_attrib == GENX_MODIFIED_DWALIGNED)
    {
        if(buff_iter->bclass != GEN4_INPUT_OUTPUT_BUFFER) {
            printf("Error reading buffer MODIFIED(%d): the registered buffer type is not INPUT_OUTPUT_BUFFER!\n", buf_id.get_data()&0xFF);
            exit(EXIT_FAILURE);
        }
        buff = (char*) buff_iter->p_volatile;
    }
    else
    {
        buff = (char*) buff_iter->p;
    }

    int sizeofT = sizeof(T); /* Make this into a signed integer */
    for (i = 0; i < S; i++) {
        pos = offset + i * sizeofT;
        if (pos >= width) {
            in(i) = 0;
        } else {
            {
                in(i) = *((T*)(buff + pos));
            }
        }
    }

    return true;
}

/* This funtion writes OWord Block data to genx dataport */
template <typename T, uint S>
CM_API bool
write(SurfaceIndex & buf_id, int offset, const vector<T,S> &out)
{
    std::unique_lock<std::mutex> lk(mutexForWrite);

    int i;
    uint pos;
    cm_list<CmEmulSys::iobuffer>::iterator buff_iter =
                                    CmEmulSys::search_buffer(buf_id.get_data()&0xFF);

    if(buff_iter == CmEmulSys::iobuffers.end()) {
        printf("Error reading buffer %d: buffer %d is not registered!\n", buf_id.get_data()&0xFF, buf_id.get_data()&0xFF);
        exit(EXIT_FAILURE);
    }

    int width = buff_iter->width;
    int height = buff_iter->height;

    if((offset % 16) != 0) {
        printf("Error writing buffer %d: offset must be 16-byte aligned!\n", buf_id.get_data()&0xFF);
        exit(EXIT_FAILURE);
    }
    int sizeofT = sizeof(T); /* Make this into a signed integer */
    if(((S * sizeofT) % 16) != 0) {
        printf("Error writing buffer %d: input vector size must be 16-byte aligned!\n", buf_id.get_data()&0xFF);
        exit(EXIT_FAILURE);
    }
    assert(height == 1);

    if((buff_iter->bclass != GEN4_OUTPUT_BUFFER) &&
        (buff_iter->bclass != GEN4_INPUT_OUTPUT_BUFFER)) {
            printf("Error writing buffer %d: incorrect buffer type!\n", buf_id.get_data()&0xFF);
            exit(EXIT_FAILURE);
    }

    for (i = 0; i < S; i++) {
        pos = offset + i * sizeofT;
        if (pos >= width) {
            printf("Warning writing buffer %d: there is unexpected out-of-bound access for Oword block write!\n", buf_id.get_data() & 0xFF);
            break;
        }
        if(buff_iter->bclass == GEN4_INPUT_OUTPUT_BUFFER) {
            *((T*)( (char*)buff_iter->p_volatile + pos )) = out(i);
        } else {
            *((T*)( (char*)buff_iter->p + pos )) = out(i);
        }
    }

    return true;
}

/* This funtion writes OWord Block data to genx dataport */
template <typename T, uint S>
CM_API bool
write(SurfaceIndex & buf_id, int offset, const vector_ref<T, S> out)
{
    std::unique_lock<std::mutex> lk(mutexForWrite);

    uint i;
    int pos;
    cm_list<CmEmulSys::iobuffer>::iterator buff_iter =
                                    CmEmulSys::search_buffer(buf_id.get_data()&0xFF);

    if(buff_iter == CmEmulSys::iobuffers.end()) {
        printf("Error reading buffer %d: buffer %d is not registered!\n", buf_id.get_data()&0xFF, buf_id.get_data()&0xFF);
        exit(EXIT_FAILURE);
    }

    int width = buff_iter->width;
    int height = buff_iter->height;

    if((offset % 16) != 0) {
        printf("Error writing buffer %d: offset must be 16-byte aligned!\n", buf_id.get_data()&0xFF);
        exit(EXIT_FAILURE);
    }
    int sizeofT = sizeof(T); /* Make this into a signed integer */
    if(((S * sizeofT) % 16) != 0) {
        printf("Error writing buffer %d: input vector size must be 16-byte aligned!\n", buf_id.get_data()&0xFF);
        exit(EXIT_FAILURE);
    }
    assert(height == 1);

    if((buff_iter->bclass != GEN4_OUTPUT_BUFFER) &&
        (buff_iter->bclass != GEN4_INPUT_OUTPUT_BUFFER)) {
            printf("Error writing buffer %d: incorrect buffer type!\n", buf_id.get_data()&0xFF);
            exit(EXIT_FAILURE);
    }

    for (i = 0; i < S; i++) {
        pos = offset + i * sizeofT;
        if (pos >= width) {
            break;
        }
        if(buff_iter->bclass == GEN4_INPUT_OUTPUT_BUFFER) {
            *((T*)( (char*)buff_iter->p_volatile + pos )) = out(i);
        } else {
            *((T*)( (char*)buff_iter->p + pos )) = out(i);
        }
    }

    return true;
}

/* This funtion reads scattered DWords from genx dataport */
template <typename T, uint N>
CM_API bool
read(SurfaceIndex & buf_id, CmBufferAttrib buf_attrib, uint global_offset, vector_ref<uint, N> element_offset, vector_ref<T, N> in)
{
    std::unique_lock<std::mutex> lock(mutexForWrite);

    static const bool conformable1 = is_fp_or_dword_type<T>::value;
    uint i;
    uint pos;
    cm_list<CmEmulSys::iobuffer>::iterator buff_iter =
                                    CmEmulSys::search_buffer(buf_id.get_data()&0xFF);

    if(buff_iter == CmEmulSys::iobuffers.end()) {
        printf("Error reading buffer %d: buffer %d is not registered!\n", buf_id.get_data()&0xFF, buf_id.get_data()&0xFF);
        exit(EXIT_FAILURE);
    }

    if(buf_attrib != GENX_MODIFIED && buf_attrib != GENX_CONSTANT) {
        printf("Error reading buffer %d: incorrect buffer attribute %d!\n", buf_id.get_data()&0xFF, buf_attrib);
        exit(EXIT_FAILURE);
    }

    int width = buff_iter->width;
    int height = buff_iter->height;

    {
        if(buff_iter->bclass != GEN4_INPUT_OUTPUT_BUFFER) {
            printf("Error reading buffer MODIFIED(%d): the registered buffer type is not INPUT_OUTPUT_BUFFER!\n", buf_id.get_data()&0xFF);
            exit(EXIT_FAILURE);
        }
    }

    size_t sizeOfT = sizeof(T);
    for (i = 0; i < N; i++) {
        SIMDCF_ELEMENT_SKIP(i);
        pos = (global_offset + element_offset(i))*sizeOfT;
        if (pos >= width*height) {
            in(i) = width*height - 1;
        } else {
            {
                in(i) = *((T*)((char*)buff_iter->p_volatile + pos));
            }
        }
    }

    return true;
}

template <typename T, uint N>
CM_API bool
read(SurfaceIndex & buf_id, CmBufferAttrib buf_attrib, uint global_offset, vector<uint, N> &element_offset, vector<T, N> &in)
{
    return read(buf_id, buf_attrib, global_offset, element_offset.template select<N, 1>(), in.template select<N, 1>());
}

template <typename T, uint N>
CM_API bool
read(SurfaceIndex & buf_id, CmBufferAttrib buf_attrib, uint global_offset, vector<uint, N> &element_offset, vector_ref<T, N> in)
{
    return read(buf_id, buf_attrib, global_offset, element_offset.template select<N, 1>(), in);
}

template <typename T, uint N>
CM_API bool
read(SurfaceIndex & buf_id, CmBufferAttrib buf_attrib, uint global_offset, vector_ref<uint, N> element_offset, vector<T, N> &in)
{
    return read(buf_id, buf_attrib, global_offset, element_offset, in.template select<N, 1>());
}

template <typename T, uint N>
CM_API bool
read(SurfaceIndex & buf_id, uint global_offset, vector_ref<uint, N> element_offset, vector_ref<T, N> in)
{
    return read(buf_id, GENX_MODIFIED, global_offset, element_offset, in);
}

template <typename T, uint N>
CM_API bool
read(SurfaceIndex & buf_id, uint global_offset, vector<uint, N> &element_offset, vector<T, N> &in)
{
    return read(buf_id, GENX_MODIFIED, global_offset, element_offset, in);
}

template <typename T, uint N>
CM_API bool
read(SurfaceIndex & buf_id, uint global_offset, vector<uint, N> &element_offset, vector_ref<T, N> in)
{
    return read(buf_id, GENX_MODIFIED, global_offset, element_offset, in);
}

template <typename T, uint N>
CM_API bool
read(SurfaceIndex & buf_id, uint global_offset, vector_ref<uint, N> element_offset, vector<T, N> &in)
{
    return read(buf_id, GENX_MODIFIED, global_offset, element_offset, in);
}

/* This funtion reads HWord Block data from genx dataport */
template <typename T, uint S>
CM_API bool
cm_hword_read(SurfaceIndex & buf_id, int offset, vector<T,S> &in)
{
    return cm_hword_read(buf_id, GENX_MODIFIED, offset, in);
}

/* This funtion reads HWord Block data from genx dataport */
template <typename T, uint S>
CM_API bool
cm_hword_read(SurfaceIndex & buf_id, int offset, vector_ref<T,S> in)
{
    return cm_hword_read(buf_id, GENX_MODIFIED, offset, in);
}

/* This funtion reads HWord Block data from genx dataport */
template <typename T, uint S>
CM_API bool
cm_hword_read(SurfaceIndex & buf_id, CmBufferAttrib buf_attrib, int offset, vector<T,S> &in)
{
    cm_list<CmEmulSys::iobuffer>::iterator buff_iter =
        CmEmulSys::search_buffer(buf_id.get_data()&0xFF);

    if(buff_iter == CmEmulSys::iobuffers.end())
    {
        printf("%s:%d - Error reading buffer %d: buffer %d is not registered!\n",
               __FUNCTION__,
               __LINE__,
               buf_id.get_data()&0xFF,
               buf_id.get_data()&0xFF);
        exit(EXIT_FAILURE);
    }

    if ((offset & 0x1f) != 0) /// 0x1f = 011111B = 31
    {
        printf("%s:%d - Error reading buffer %d: offset must be 32-byte aligned! offset = %d\n",
               __FUNCTION__,
               __LINE__,
               buf_id.get_data()&0xFF,
               offset);
        exit(EXIT_FAILURE);
    }

    int sizeofT = sizeof(T);
    int numbytes = sizeofT * S;

    /// VMIT-6506
    if (numbytes != 32         // 1-HWORD
        && numbytes != 32 * 2  // 2-HWORD
        && numbytes != 32 * 4  // 4-HWORD
        && numbytes != 32 * 8) // 8-HWORD
    {
        printf("%s:%d - Error reading buffer %d: read size must be 1/2/4/8-HWORD (32/64/128/256 Bytes)!! numbytes = %d\n",
               __FUNCTION__,
               __LINE__,
               buf_id.get_data()&0xFF,
               numbytes);
        exit(EXIT_FAILURE);
    }

    if (buff_iter->height != 1)
    {
        printf("%s:%d - Height of read-surface is not 1!! (height = %d)\n",
               __FUNCTION__,
               __LINE__,
               buff_iter->height);
        exit(EXIT_FAILURE);
    }

    char* buff;

    if(buf_attrib == GENX_MODIFIED
       || buf_attrib == GENX_MODIFIED_DWALIGNED
        )
    {
        if(buff_iter->bclass != GEN4_INPUT_OUTPUT_BUFFER) {
            printf("%s:%d - Error reading buffer MODIFIED(%d): the registered buffer type is not INPUT_OUTPUT_BUFFER!\n",
                   __FUNCTION__,
                   __LINE__,
                   buf_id.get_data()&0xFF);
            exit(EXIT_FAILURE);
        }
        buff = (char*) buff_iter->p_volatile;
    }
    else
    {
        buff = (char*) buff_iter->p;
    }

    uint pos = offset;

    for (int i = 0; i < S; i++)
    {
        if (pos >= buff_iter->width)
        {
            in(i) = 0;
        }
        else
        {
            in(i) = *((T*)(buff + pos));
        }

        pos += sizeofT;
        numbytes -= sizeofT;
    }

    if (numbytes != 0)
    {
        printf("%s:%d - Number of Bytes mismatch!!\n",
               __FUNCTION__,
               __LINE__);
        exit(EXIT_FAILURE);
    }

    return true;
}

/* This funtion reads scattered DWords from genx dataport */
template <typename T, uint N>
CM_API bool
read_scaled(SurfaceIndex & buf_id, CmBufferAttrib buf_attrib, uint global_offset, vector_ref<uint, N> element_offset, vector_ref<T, N> in)
{
    static const bool conformable1 = is_fp_or_dword_type<T>::value;
    uint i;
    uint pos;
    cm_list<CmEmulSys::iobuffer>::iterator buff_iter =
        CmEmulSys::search_buffer(buf_id.get_data() & 0xFF);

    if (buff_iter == CmEmulSys::iobuffers.end()) {
        printf("Error reading buffer %d: buffer %d is not registered!\n", buf_id.get_data() & 0xFF, buf_id.get_data() & 0xFF);
        exit(EXIT_FAILURE);
    }

    if (buf_attrib != GENX_MODIFIED && buf_attrib != GENX_CONSTANT) {
        printf("Error reading buffer %d: incorrect buffer attribute %d!\n", buf_id.get_data() & 0xFF, buf_attrib);
        exit(EXIT_FAILURE);
    }

    int width = buff_iter->width;
    int height = buff_iter->height;

    {
        if (buff_iter->bclass != GEN4_INPUT_OUTPUT_BUFFER) {
            printf("Error reading buffer MODIFIED(%d): the registered buffer type is not INPUT_OUTPUT_BUFFER!\n", buf_id.get_data() & 0xFF);
            exit(EXIT_FAILURE);
        }
    }

    for (i = 0; i < N; i++) {
        SIMDCF_ELEMENT_SKIP(i);
        pos = global_offset + element_offset(i);
        if (pos >= width * height) {
            in(i) = width * height - 1;
        }
        else {
            {
                in(i) = *((T*)((char*)buff_iter->p_volatile + pos));
            }
        }
    }

    return true;
}

template <typename T, uint N>
CM_API bool
read_scaled(SurfaceIndex & buf_id, CmBufferAttrib buf_attrib, uint global_offset, vector<uint, N> &element_offset, vector<T, N> &in)
{
    return read_scaled(buf_id, buf_attrib, global_offset, element_offset.template select<N, 1>(), in.template select<N, 1>());
}

template <typename T, uint N>
CM_API bool
read_scaled(SurfaceIndex & buf_id, CmBufferAttrib buf_attrib, uint global_offset, vector<uint, N> &element_offset, vector_ref<T, N> in)
{
    return read_scaled(buf_id, buf_attrib, global_offset, element_offset.template select<N, 1>(), in);
}

template <typename T, uint N>
CM_API bool
read_scaled(SurfaceIndex & buf_id, CmBufferAttrib buf_attrib, uint global_offset, vector_ref<uint, N> element_offset, vector<T, N> &in)
{
    return read_scaled(buf_id, buf_attrib, global_offset, element_offset, in.template select<N, 1>());
}

template <typename T, uint N>
CM_API bool
read_scaled(SurfaceIndex & buf_id, uint global_offset, vector_ref<uint, N> element_offset, vector_ref<T, N> in)
{
    return read_scaled(buf_id, GENX_MODIFIED, global_offset, element_offset, in);
}

template <typename T, uint N>
CM_API bool
read_scaled(SurfaceIndex & buf_id, uint global_offset, vector<uint, N> &element_offset, vector<T, N> &in)
{
    return read_scaled(buf_id, GENX_MODIFIED, global_offset, element_offset, in);
}

template <typename T, uint N>
CM_API bool
read_scaled(SurfaceIndex & buf_id, uint global_offset, vector<uint, N> &element_offset, vector_ref<T, N> in)
{
    return read_scaled(buf_id, GENX_MODIFIED, global_offset, element_offset, in);
}

template <typename T, uint N>
CM_API bool
read_scaled(SurfaceIndex & buf_id, uint global_offset, vector_ref<uint, N> element_offset, vector<T, N> &in)
{
    return read_scaled(buf_id, GENX_MODIFIED, global_offset, element_offset, in);
}

/* This funtion writes scattered DWords to genx dataport */
template <typename T, uint N>
CM_API bool
write(SurfaceIndex & buf_id, uint global_offset, vector_ref<uint, N> element_offset, vector_ref<T, N> out)
{
    std::unique_lock<std::mutex> lk(mutexForWrite);

    static const bool conformable1 = is_fp_or_dword_type<T>::value;
    uint i;
    uint pos;
    cm_list<CmEmulSys::iobuffer>::iterator buff_iter =
                                    CmEmulSys::search_buffer(buf_id.get_data()&0xFF);

    if(buff_iter == CmEmulSys::iobuffers.end()) {
        printf("Error reading buffer %d: buffer %d is not registered!\n", buf_id.get_data()&0xFF, buf_id.get_data()&0xFF);
        exit(EXIT_FAILURE);
    }

    int width = buff_iter->width;
    int height = buff_iter->height;

    if((buff_iter->bclass != GEN4_OUTPUT_BUFFER) &&
        (buff_iter->bclass != GEN4_INPUT_OUTPUT_BUFFER)) {
            printf("Error writing buffer %d: incorrect buffer type!\n", buf_id.get_data()&0xFF);
            exit(EXIT_FAILURE);
    }

    size_t sizeOfT = sizeof(T);
    for (i = 0; i < N; i++) {
        SIMDCF_ELEMENT_SKIP(i);
        pos = (global_offset + element_offset(i))*sizeOfT;
        if (pos >= width*height) {
            continue;
        }
        if(buff_iter->bclass == GEN4_INPUT_OUTPUT_BUFFER) {
            *((T*)( (char*)buff_iter->p_volatile + pos )) = out(i);
        } else {
            *((T*)( (char*)buff_iter->p + pos )) = out(i);
        }
    }

    return true;
}

template <typename T, uint N>
CM_API bool
write(SurfaceIndex & buf_id, uint global_offset, vector<uint, N> &element_offset, vector<T, N> &out)
{
    return write(buf_id, global_offset, element_offset.template select<N,1>(), out.template select<N,1>());
}

template <typename T, uint N>
CM_API bool
write(SurfaceIndex & buf_id, uint global_offset, vector<uint, N> &element_offset, vector_ref<T, N> out)
{
    return write(buf_id, global_offset, element_offset.template select<N, 1>(), out);
}

template <typename T, uint N>
CM_API bool
write(SurfaceIndex & buf_id, uint global_offset, vector_ref<uint, N> element_offset, vector<T, N> &out)
{
    return write(buf_id, global_offset, element_offset, out.template select<N, 1>());
}

template <typename T, uint N>
CM_API bool
write_scaled(SurfaceIndex & buf_id, uint global_offset, vector_ref<uint, N> element_offset, vector_ref<T, N> out)
{
    static const bool conformable1 = is_fp_or_dword_type<T>::value;
    uint i;
    uint pos;
    cm_list<CmEmulSys::iobuffer>::iterator buff_iter =
        CmEmulSys::search_buffer(buf_id.get_data() & 0xFF);

    if (buff_iter == CmEmulSys::iobuffers.end()) {
        printf("Error reading buffer %d: buffer %d is not registered!\n", buf_id.get_data() & 0xFF, buf_id.get_data() & 0xFF);
        exit(EXIT_FAILURE);
    }

    int width = buff_iter->width;
    int height = buff_iter->height;

    if ((buff_iter->bclass != GEN4_OUTPUT_BUFFER) &&
        (buff_iter->bclass != GEN4_INPUT_OUTPUT_BUFFER)) {
        printf("Error writing buffer %d: incorrect buffer type!\n", buf_id.get_data() & 0xFF);
        exit(EXIT_FAILURE);
    }

    for (i = 0; i < N; i++) {
        SIMDCF_ELEMENT_SKIP(i);
        pos = global_offset + element_offset(i);
        if (pos >= width * height) {
            continue;
        }
        if (buff_iter->bclass == GEN4_INPUT_OUTPUT_BUFFER) {
            *((T*)((char*)buff_iter->p_volatile + pos)) = out(i);
        }
        else {
            *((T*)((char*)buff_iter->p + pos)) = out(i);
        }
    }

    return true;
}

template <typename T, uint N>
CM_API bool
write_scaled(SurfaceIndex & buf_id, uint global_offset, vector<uint, N> &element_offset, vector<T, N> &out)
{
    return write_scaled(buf_id, global_offset, element_offset.template select<N, 1>(), out.template select<N, 1>());
}

template <typename T, uint N>
CM_API bool
write_scaled(SurfaceIndex & buf_id, uint global_offset, vector<uint, N> &element_offset, vector_ref<T, N> out)
{
    return write_scaled(buf_id, global_offset, element_offset.template select<N, 1>(), out);
}

template <typename T, uint N>
CM_API bool
write_scaled(SurfaceIndex & buf_id, uint global_offset, vector_ref<uint, N> element_offset, vector<T, N> &out)
{
    return write_scaled(buf_id, global_offset, element_offset, out.template select<N, 1>());
}

/* This funtion performs atomic scattered DWord write to genx dataport */
template <typename T, uint N>
CM_API bool
write(SurfaceIndex & buf_id, CmAtomicOpType op, uint global_offset, vector<uint, 8> &element_offset, vector <T, N> &src, vector<T, 8> &v)
{
    std::unique_lock<std::mutex> lk(mutexForWrite);

    static const bool conformable1 = is_dword_type<T>::value;
    uint i;
    uint pos;
    cm_list<CmEmulSys::iobuffer>::iterator buff_iter =
                                    CmEmulSys::search_buffer(buf_id.get_data()&0xFF);

    if(buff_iter == CmEmulSys::iobuffers.end()) {
        printf("Error reading buffer %d: buffer %d is not registered!\n", buf_id.get_data()&0xFF, buf_id.get_data()&0xFF);
        exit(EXIT_FAILURE);
    }

    int width = buff_iter->width;
    int height = buff_iter->height;

    //assert(height == 1);

    if(buff_iter->bclass != GEN4_INPUT_OUTPUT_BUFFER) {
            printf("Error writing buffer %d: the buffer type must be GEN4_INPUT_OUTPUT_BUFFER for DWord atomic write!\n", buf_id.get_data()&0xFF);
            exit(EXIT_FAILURE);
    }

    if(op > ATOMIC_FMAX) {
        printf("Error writing buffer %d: invalid opcode for DWord atomic write!\n", buf_id.get_data()&0xFF);
        exit(EXIT_FAILURE);
    }

    if(op == ATOMIC_CMPXCHG || op == ATOMIC_FCMPWR) {
        if(N != 16) {
            printf("Error writing buffer %d: src length must be 16 for DWord atomic write when op is ATOMIC_CMPXCHG/ATOMIC_FCMPWR!\n", buf_id.get_data()&0xFF);
            exit(EXIT_FAILURE);
        }
    } else {
        if(N != 8) {
            printf("Error writing buffer %d: src length must be 8 for DWord atomic write when op is not ATOMIC_CMPXCHG/ATOMIC_FCMPWR!\n", buf_id.get_data()&0xFF);
            exit(EXIT_FAILURE);
        }
    }

    // InitializeCriticalSection(&dataport_cs);

    for (i = 0; i < 8; i++) {
        SIMDCF_ELEMENT_SKIP(i);
        pos = (global_offset + element_offset(i))*4;
        if (pos >= width*height) {
            continue;
        }
        if(buff_iter->bclass == GEN4_INPUT_OUTPUT_BUFFER) {
            switch (op) {
                case ATOMIC_ADD:
                    v(i) = *((T*)((char*)buff_iter->p_volatile + pos));
                    *((T*)((char*)buff_iter->p_volatile + pos)) = v(i) + src(i);
                    break;
                case ATOMIC_SUB:
                    v(i) = *((T*)((char*)buff_iter->p_volatile + pos));
                    *((T*)((char*)buff_iter->p_volatile + pos)) = v(i) - src(i);
                    break;
                case ATOMIC_INC:
                    v(i) = *((T*)((char*)buff_iter->p_volatile + pos));
                    *((T*)((char*)buff_iter->p_volatile + pos)) = v(i) + 1;
                    break;
                case ATOMIC_DEC:
                    v(i) = *((T*)((char*)buff_iter->p_volatile + pos));
                    *((T*)((char*)buff_iter->p_volatile + pos)) = v(i) - 1;
                    break;
                case ATOMIC_MIN:
                    v(i) = *((T*)((char*)buff_iter->p_volatile + pos));
                    if(!is_fp_type<T>::value) {
                        *((T*)((char*)buff_iter->p_volatile + pos)) = (((uint) v(i) <= (uint) src(i))? v(i): src(i));
                    }
                    break;
                case ATOMIC_MAX:
                    v(i) = *((T*)((char*)buff_iter->p_volatile + pos));
                    if(!is_fp_type<T>::value) {
                        *((T*)((char*)buff_iter->p_volatile + pos)) = (((uint) v(i) >= (uint) src(i))? v(i): src(i));
                    }
                    break;
                case ATOMIC_XCHG:
                    v(i) = *((T*)((char*)buff_iter->p_volatile + pos));
                    *((T*)((char*)buff_iter->p_volatile + pos)) = src(i);
                    break;
                case ATOMIC_CMPXCHG:
                    v(i) = *((T*)((char*)buff_iter->p_volatile + pos));
                    *((T*)((char*)buff_iter->p_volatile + pos)) = ((v(i) == src(i+8))? src(i): v(i));
                    break;
                case ATOMIC_AND:
                    v(i) = *((T*)((char*)buff_iter->p_volatile + pos));
                    if (std::is_same_v <T, uint>)
                    {
                        *((T*)((char*)buff_iter->p_volatile + pos)) = (uint)v(i) & (uint)src(i);
                    }
                    else if (std::is_same_v <T, int>)
                    {
                        *((T*)((char*)buff_iter->p_volatile + pos)) = (int)v(i) & (int)src(i);
                    }
                    else
                    {
                        printf("Error writing buffer %d: unsupported opcode/type for DWord atomic write!\n", buf_id.get_data() & 0xFF);
                        exit(EXIT_FAILURE);
                    }
                    break;
                case ATOMIC_OR:
                    v(i) = *((T*)((char*)buff_iter->p_volatile + pos));
                    if (std::is_same_v <T, uint>)
                    {
                        *((T*)((char*)buff_iter->p_volatile + pos)) = (uint)v(i) | (uint)src(i);
                    }
                    else if (std::is_same_v <T, int>)
                    {
                        *((T*)((char*)buff_iter->p_volatile + pos)) = (int)v(i) | (int)src(i);
                    }
                    else
                    {
                        printf("Error writing buffer %d: unsupported opcode/type for DWord atomic write!\n", buf_id.get_data() & 0xFF);
                        exit(EXIT_FAILURE);
                    }
                    break;
                case ATOMIC_XOR:
                    v(i) = *((T*)((char*)buff_iter->p_volatile + pos));
                    if (std::is_same_v <T, uint>)
                    {
                        *((T*)((char*)buff_iter->p_volatile + pos)) = (uint)v(i) ^ (uint)src(i);
                    }
                    else if (std::is_same_v <T, int>)
                    {
                        *((T*)((char*)buff_iter->p_volatile + pos)) = (int)v(i) ^ (int)src(i);
                    }
                    else
                    {
                        printf("Error writing buffer %d: unsupported opcode/type for DWord atomic write!\n", buf_id.get_data() & 0xFF);
                        exit(EXIT_FAILURE);
                    }
                    break;
                case ATOMIC_MINSINT:
                    v(i) = *((T*)((char*)buff_iter->p_volatile + pos));
                    *((T*)((char*)buff_iter->p_volatile + pos)) = (((int) v(i) <= (int) src(i))? v(i): src(i));
                    break;
                case ATOMIC_MAXSINT:
                    v(i) = *((T*)((char*)buff_iter->p_volatile + pos));
                    *((T*)((char*)buff_iter->p_volatile + pos)) = (((int) v(i) >= (int) src(i))? v(i): src(i));
                    break;
                case ATOMIC_FADD:
                    v(i) = *((T*)((char*)buff_iter->p_volatile + pos));
                    if (is_fp_type<T>::value) {
                        *((T*)((char*)buff_iter->p_volatile + pos)) = (float)v(i) + (float)src(i);
                    }
                    break;
                case ATOMIC_FSUB:
                    v(i) = *((T*)((char*)buff_iter->p_volatile + pos));
                    if (is_fp_type<T>::value) {
                        *((T*)((char*)buff_iter->p_volatile + pos)) = (float)v(i) - (float)src(i);
                    }
                    break;
                case ATOMIC_FCMPWR:
                    v(i) = *((T*)((char*)buff_iter->p_volatile + pos));
                    if (is_fp_type<T>::value) {
                        *((T*)((char*)buff_iter->p_volatile + pos)) = ((v(i) == src(i)) ? src(i + 8) : v(i));
                    }
                    break;
                case ATOMIC_FMIN:
                    v(i) = *((T*)((char*)buff_iter->p_volatile + pos));
                    if (is_fp_type<T>::value) {
                        *((T*)((char*)buff_iter->p_volatile + pos)) = (((float)v(i) <= (float)src(i)) ? v(i) : src(i));
                    }
                    break;
                case ATOMIC_FMAX:
                    v(i) = *((T*)((char*)buff_iter->p_volatile + pos));
                    if (is_fp_type<T>::value) {
                        *((T*)((char*)buff_iter->p_volatile + pos)) = (((float)v(i) >= (float)src(i)) ? v(i) : src(i));
                    }
                    break;
                default:
                    printf("Error writing buffer %d: invalid opcode for DWord atomic write!\n", buf_id.get_data()&0xFF);
                    exit(EXIT_FAILURE);
            }
        } else {
            printf("Error writing buffer %d: the buffer type must be GEN4_INPUT_OUTPUT_BUFFER for DWord atomic write!\n", buf_id.get_data()&0xFF);
            exit(EXIT_FAILURE);
        }
    }

    // DeleteCriticalSection(&dataport_cs);

    return true;
}

template <typename T, uint N>
CM_API bool
write(SurfaceIndex & buf_id, CmAtomicOpType op, uint global_offset, vector<uint, 8> &element_offset, vector <T, N> &src, vector_ref<T, 8> v)
{
    std::unique_lock<std::mutex> lk(mutexForWrite);

    static const bool conformable1 = is_dword_type<T>::value;
    uint i;
    uint pos;
    cm_list<CmEmulSys::iobuffer>::iterator buff_iter =
                                    CmEmulSys::search_buffer(buf_id.get_data()&0xFF);

    if(buff_iter == CmEmulSys::iobuffers.end()) {
        printf("Error reading buffer %d: buffer %d is not registered!\n", buf_id.get_data()&0xFF, buf_id.get_data()&0xFF);
        exit(EXIT_FAILURE);
    }

    int width = buff_iter->width;
    int height = buff_iter->height;

    //assert(height == 1);

    if(buff_iter->bclass != GEN4_INPUT_OUTPUT_BUFFER) {
            printf("Error writing buffer %d: the buffer type must be GEN4_INPUT_OUTPUT_BUFFER for DWord atomic write!\n", buf_id.get_data()&0xFF);
            exit(EXIT_FAILURE);
    }

    if(op > ATOMIC_FMAX) {
        printf("Error writing buffer %d: invalid opcode for DWord atomic write!\n", buf_id.get_data()&0xFF);
        exit(EXIT_FAILURE);
    }

    if(op == ATOMIC_CMPXCHG || op == ATOMIC_FCMPWR) {
        if(N != 16) {
            printf("Error writing buffer %d: src length must be 16 for DWord atomic write when op is ATOMIC_CMPXCHG/ATOMIC_FCMPWR!\n", buf_id.get_data()&0xFF);
            exit(EXIT_FAILURE);
        }
    } else {
        if(N != 8) {
            printf("Error writing buffer %d: src length must be 8 for DWord atomic write when op is not ATOMIC_CMPXCHG/ATOMIC_FCMPWR!\n", buf_id.get_data()&0xFF);
            exit(EXIT_FAILURE);
        }
    }

    // InitializeCriticalSection(&dataport_cs);

    for (i = 0; i < 8; i++) {
        SIMDCF_ELEMENT_SKIP(i);
        pos = (global_offset + element_offset(i))*4;
        if (pos >= width*height) {
            continue;
        }
        if(buff_iter->bclass == GEN4_INPUT_OUTPUT_BUFFER) {
            switch (op) {
                case ATOMIC_ADD:
                    v(i) = *((T*)((char*)buff_iter->p_volatile + pos));
                    *((T*)((char*)buff_iter->p_volatile + pos)) = v(i) + src(i);
                    break;
                case ATOMIC_SUB:
                    v(i) = *((T*)((char*)buff_iter->p_volatile + pos));
                    *((T*)((char*)buff_iter->p_volatile + pos)) = v(i) - src(i);
                    break;
                case ATOMIC_INC:
                    v(i) = *((T*)((char*)buff_iter->p_volatile + pos));
                    *((T*)((char*)buff_iter->p_volatile + pos)) = v(i) + 1;
                    break;
                case ATOMIC_DEC:
                    v(i) = *((T*)((char*)buff_iter->p_volatile + pos));
                    *((T*)((char*)buff_iter->p_volatile + pos)) = v(i) - 1;
                    break;
                case ATOMIC_MIN:
                    v(i) = *((T*)((char*)buff_iter->p_volatile + pos));
                    if(!is_fp_type<T>::value) {
                        *((T*)((char*)buff_iter->p_volatile + pos)) = (((uint) v(i) <= (uint) src(i))? v(i): src(i));
                    }
                    break;
                case ATOMIC_MAX:
                    v(i) = *((T*)((char*)buff_iter->p_volatile + pos));
                    if(!is_fp_type<T>::value) {
                        *((T*)((char*)buff_iter->p_volatile + pos)) = (((uint) v(i) >= (uint) src(i))? v(i): src(i));
                    }
                    break;
                case ATOMIC_XCHG:
                    v(i) = *((T*)((char*)buff_iter->p_volatile + pos));
                    *((T*)((char*)buff_iter->p_volatile + pos)) = src(i);
                    break;
                case ATOMIC_CMPXCHG:
                    v(i) = *((T*)((char*)buff_iter->p_volatile + pos));
                    *((T*)((char*)buff_iter->p_volatile + pos)) = ((v(i) == src(i+8))? src(i): v(i));
                    break;
                case ATOMIC_AND:
                    v(i) = *((T*)((char*)buff_iter->p_volatile + pos));
                    if (std::is_same_v <T, uint>)
                    {
                        *((T*)((char*)buff_iter->p_volatile + pos)) = (uint)v(i) & (uint)src(i);
                    }
                    else if (std::is_same_v <T, int>)
                    {
                        *((T*)((char*)buff_iter->p_volatile + pos)) = (int)v(i) & (int)src(i);
                    }
                    else
                    {
                        printf("Error writing buffer %d: unsupported opcode/type for DWord atomic write!\n", buf_id.get_data() & 0xFF);
                        exit(EXIT_FAILURE);
                    }
                    break;
                case ATOMIC_OR:
                    v(i) = *((T*)((char*)buff_iter->p_volatile + pos));
                    if (std::is_same_v <T, uint>)
                    {
                        *((T*)((char*)buff_iter->p_volatile + pos)) = (uint)v(i) | (uint)src(i);
                    }
                    else if (std::is_same_v <T, int>)
                    {
                        *((T*)((char*)buff_iter->p_volatile + pos)) = (int)v(i) | (int)src(i);
                    }
                    else
                    {
                        printf("Error writing buffer %d: unsupported opcode/type for DWord atomic write!\n", buf_id.get_data() & 0xFF);
                        exit(EXIT_FAILURE);
                    }
                    break;
                case ATOMIC_XOR:
                    v(i) = *((T*)((char*)buff_iter->p_volatile + pos));
                    if (std::is_same_v <T, uint>)
                    {
                        *((T*)((char*)buff_iter->p_volatile + pos)) = (uint)v(i) ^ (uint)src(i);
                    }
                    else if (std::is_same_v <T, int>)
                    {
                        *((T*)((char*)buff_iter->p_volatile + pos)) = (int)v(i) ^ (int)src(i);
                    }
                    else
                    {
                        printf("Error writing buffer %d: unsupported opcode/type for DWord atomic write!\n", buf_id.get_data() & 0xFF);
                        exit(EXIT_FAILURE);
                    }
                    break;
                case ATOMIC_MINSINT:
                    v(i) = *((T*)((char*)buff_iter->p_volatile + pos));
                    *((T*)((char*)buff_iter->p_volatile + pos)) = (((int) v(i) <= (int) src(i))? v(i): src(i));
                    break;
                case ATOMIC_MAXSINT:
                    v(i) = *((T*)((char*)buff_iter->p_volatile + pos));
                    *((T*)((char*)buff_iter->p_volatile + pos)) = (((int) v(i) >= (int) src(i))? v(i): src(i));
                    break;
                case ATOMIC_FADD:
                    v(i) = *((T*)((char*)buff_iter->p_volatile + pos));
                    if (is_fp_type<T>::value) {
                        *((T*)((char*)buff_iter->p_volatile + pos)) = (float)v(i) + (float)src(i);
                    }
                    break;
                case ATOMIC_FSUB:
                    v(i) = *((T*)((char*)buff_iter->p_volatile + pos));
                    if (is_fp_type<T>::value) {
                        *((T*)((char*)buff_iter->p_volatile + pos)) = (float)v(i) - (float)src(i);
                    }
                    break;
                case ATOMIC_FCMPWR:
                    v(i) = *((T*)((char*)buff_iter->p_volatile + pos));
                    if (is_fp_type<T>::value) {
                        *((T*)((char*)buff_iter->p_volatile + pos)) = ((v(i) == src(i)) ? src(i + 8) : v(i));
                    }
                    break;
                case ATOMIC_FMIN:
                    v(i) = *((T*)((char*)buff_iter->p_volatile + pos));
                    if (is_fp_type<T>::value) {
                        *((T*)((char*)buff_iter->p_volatile + pos)) = (((float)v(i) <= (float)src(i)) ? v(i) : src(i));
                    }
                    break;
                case ATOMIC_FMAX:
                    v(i) = *((T*)((char*)buff_iter->p_volatile + pos));
                    if (is_fp_type<T>::value) {
                        *((T*)((char*)buff_iter->p_volatile + pos)) = (((float)v(i) >= (float)src(i)) ? v(i) : src(i));
                    }
                    break;
                default:
                    printf("Error writing buffer %d: invalid opcode for DWord atomic write!\n", buf_id.get_data()&0xFF);
                    exit(EXIT_FAILURE);
            }
        } else {
            printf("Error writing buffer %d: the buffer type must be GEN4_INPUT_OUTPUT_BUFFER for DWord atomic write!\n", buf_id.get_data()&0xFF);
            exit(EXIT_FAILURE);
        }
    }

    // DeleteCriticalSection(&dataport_cs);

    return true;
}

template <typename T, uint N>
CM_API bool
write(SurfaceIndex & buf_id, CmAtomicOpType op, uint global_offset, vector<uint, 8> &element_offset, vector <T, N> &src, int not_used)
{
    std::unique_lock<std::mutex> lk(mutexForWrite);

    static const bool conformable1 = is_dword_type<T>::value;
    uint i;
    uint pos;
    T v;
    cm_list<CmEmulSys::iobuffer>::iterator buff_iter =
                                    CmEmulSys::search_buffer(buf_id.get_data()&0xFF);

    if (not_used != 0) {
        printf("write atomic passed destination vec as int but not NULL %x\n", not_used);
        exit(EXIT_FAILURE);
    }
    if(buff_iter == CmEmulSys::iobuffers.end()) {
        printf("Error reading buffer %d: buffer %d is not registered!\n", buf_id.get_data()&0xFF, buf_id.get_data()&0xFF);
        exit(EXIT_FAILURE);
    }

    int width = buff_iter->width;
    int height = buff_iter->height;

    //assert(height == 1);

    if(buff_iter->bclass != GEN4_INPUT_OUTPUT_BUFFER) {
            printf("Error writing buffer %d: the buffer type must be GEN4_INPUT_OUTPUT_BUFFER for DWord atomic write!\n", buf_id.get_data()&0xFF);
            exit(EXIT_FAILURE);
    }

    if(op > ATOMIC_FMAX) {
        printf("Error writing buffer %d: invalid opcode for DWord atomic write!\n", buf_id.get_data()&0xFF);
        exit(EXIT_FAILURE);
    }

    if(op == ATOMIC_CMPXCHG || op == ATOMIC_FCMPWR) {
        if(N != 16) {
            printf("Error writing buffer %d: src length must be 16 for DWord atomic write when op is ATOMIC_CMPXCHG/ATOMIC_FCMPWR!\n", buf_id.get_data()&0xFF);
            exit(EXIT_FAILURE);
        }
    } else {
        if(N != 8) {
            printf("Error writing buffer %d: src length must be 8 for DWord atomic write when op is not ATOMIC_CMPXCHG/ATOMIC_FCMPWR!\n", buf_id.get_data()&0xFF);
            exit(EXIT_FAILURE);
        }
    }

    // InitializeCriticalSection(&dataport_cs);

    for (i = 0; i < 8; i++) {
        SIMDCF_ELEMENT_SKIP(i);
        pos = (global_offset + element_offset(i))*4;
        if (pos >= width*height) {
            continue;
        }
        if(buff_iter->bclass == GEN4_INPUT_OUTPUT_BUFFER) {
            switch (op) {
                case ATOMIC_ADD:
                    v = *((T*)((char*)buff_iter->p_volatile + pos));
                    *((T*)((char*)buff_iter->p_volatile + pos)) = v + src(i);
                    break;
                case ATOMIC_SUB:
                    v = *((T*)((char*)buff_iter->p_volatile + pos));
                    *((T*)((char*)buff_iter->p_volatile + pos)) = v - src(i);
                    break;
                case ATOMIC_INC:
                    v = *((T*)((char*)buff_iter->p_volatile + pos));
                    *((T*)((char*)buff_iter->p_volatile + pos)) = v + 1;
                    break;
                case ATOMIC_DEC:
                    v = *((T*)((char*)buff_iter->p_volatile + pos));
                    *((T*)((char*)buff_iter->p_volatile + pos)) = v - 1;
                    break;
                case ATOMIC_MIN:
                    v = *((T*)((char*)buff_iter->p_volatile + pos));
                    if(!is_fp_type<T>::value) {
                        *((T*)((char*)buff_iter->p_volatile + pos)) = (((uint) v <= (uint) src(i))? v: src(i));
                    }
                    break;
                case ATOMIC_MAX:
                    v = *((T*)((char*)buff_iter->p_volatile + pos));
                    if(!is_fp_type<T>::value) {
                        *((T*)((char*)buff_iter->p_volatile + pos)) = (((uint) v >= (uint) src(i))? v: src(i));
                    }
                    break;
                case ATOMIC_XCHG:
                    v = *((T*)((char*)buff_iter->p_volatile + pos));
                    *((T*)((char*)buff_iter->p_volatile + pos)) = src(i);
                    break;
                case ATOMIC_CMPXCHG:
                    v = *((T*)((char*)buff_iter->p_volatile + pos));
                    *((T*)((char*)buff_iter->p_volatile + pos)) = ((v == src(i+8))? src(i): v);
                    break;
                case ATOMIC_AND:
                    v = *((T*)((char*)buff_iter->p_volatile + pos));
                    if (std::is_same_v <T, uint>)
                    {
                        *((T*)((char*)buff_iter->p_volatile + pos)) = (uint) v & (uint) src(i);
                    }
                    else if (std::is_same_v <T, int>)
                    {
                        *((T*)((char*)buff_iter->p_volatile + pos)) = (int) v & (int) src(i);
                    }
                    else
                    {
                        printf("Error writing buffer %d: unsupported opcode/type for DWord atomic write!\n", buf_id.get_data() & 0xFF);
                        exit(EXIT_FAILURE);
                    }
                    break;
                case ATOMIC_OR:
                    v = *((T*)((char*)buff_iter->p_volatile + pos));
                    if (std::is_same_v <T, uint>)
                    {
                        *((T*)((char*)buff_iter->p_volatile + pos)) = (uint) v | (uint) src(i);
                    }
                    else if (std::is_same_v <T, int>)
                    {
                        *((T*)((char*)buff_iter->p_volatile + pos)) = (int) v | (int) src(i);
                    }
                    else
                    {
                        printf("Error writing buffer %d: unsupported opcode/type for DWord atomic write!\n", buf_id.get_data() & 0xFF);
                        exit(EXIT_FAILURE);
                    }
                    break;
                case ATOMIC_XOR:
                    v = *((T*)((char*)buff_iter->p_volatile + pos));
                    if (std::is_same_v <T, uint>)
                    {
                        *((T*)((char*)buff_iter->p_volatile + pos)) = (uint) v ^ (uint) src(i);
                    }
                    else if (std::is_same_v <T, int>)
                    {
                        *((T*)((char*)buff_iter->p_volatile + pos)) = (int) v ^ (int) src(i);
                    }
                    else
                    {
                        printf("Error writing buffer %d: unsupported opcode/type for DWord atomic write!\n", buf_id.get_data() & 0xFF);
                        exit(EXIT_FAILURE);
                    }
                    break;
                case ATOMIC_MINSINT:
                    v = *((T*)((char*)buff_iter->p_volatile + pos));
                    *((T*)((char*)buff_iter->p_volatile + pos)) = (((int) v <= (int) src(i))? v: src(i));
                    break;
                case ATOMIC_MAXSINT:
                    v = *((T*)((char*)buff_iter->p_volatile + pos));
                    *((T*)((char*)buff_iter->p_volatile + pos)) = (((int) v >= (int) src(i))? v: src(i));
                    break;
                case ATOMIC_FADD:
                    v = *((T*)((char*)buff_iter->p_volatile + pos));
                    if (is_fp_type<T>::value) {
                        *((T*)((char*)buff_iter->p_volatile + pos)) = (float)v + (float)src(i);
                    }
                    break;
                case ATOMIC_FSUB:
                    v = *((T*)((char*)buff_iter->p_volatile + pos));
                    if (is_fp_type<T>::value) {
                        *((T*)((char*)buff_iter->p_volatile + pos)) = (float)v - (float)src(i);
                    }
                    break;
                case ATOMIC_FCMPWR:
                    v = *((T*)((char*)buff_iter->p_volatile + pos));
                    if (is_fp_type<T>::value) {
                        *((T*)((char*)buff_iter->p_volatile + pos)) = ((v == src(i)) ? src(i + 8) : v);
                    }
                    break;
                case ATOMIC_FMIN:
                    v = *((T*)((char*)buff_iter->p_volatile + pos));
                    if (is_fp_type<T>::value) {
                        *((T*)((char*)buff_iter->p_volatile + pos)) = (((float)v <= (float)src(i)) ? v : src(i));
                    }
                    break;
                case ATOMIC_FMAX:
                    v = *((T*)((char*)buff_iter->p_volatile + pos));
                    if (is_fp_type<T>::value) {
                        *((T*)((char*)buff_iter->p_volatile + pos)) = (((float)v >= (float)src(i)) ? v : src(i));
                    }
                    break;
                default:
                    printf("Error writing buffer %d: invalid opcode for DWord atomic write!\n", buf_id.get_data()&0xFF);
                    exit(EXIT_FAILURE);
            }
        } else {
            printf("Error writing buffer %d: the buffer type must be GEN4_INPUT_OUTPUT_BUFFER for DWord atomic write!\n", buf_id.get_data()&0xFF);
            exit(EXIT_FAILURE);
        }
    }

    // DeleteCriticalSection(&dataport_cs);

    return true;
}

template <typename T, uint N>
CM_API bool
write(SurfaceIndex & buf_id, CmAtomicOpType op, uint global_offset, vector<uint, 8> &element_offset, vector_ref <T, N> src, vector<T, 8> &v)
{
    std::unique_lock<std::mutex> lk(mutexForWrite);

    static const bool conformable1 = is_dword_type<T>::value;
    uint i;
    uint pos;
    cm_list<CmEmulSys::iobuffer>::iterator buff_iter =
                                    CmEmulSys::search_buffer(buf_id.get_data()&0xFF);

    if(buff_iter == CmEmulSys::iobuffers.end()) {
        printf("Error reading buffer %d: buffer %d is not registered!\n", buf_id.get_data()&0xFF, buf_id.get_data()&0xFF);
        exit(EXIT_FAILURE);
    }

    int width = buff_iter->width;
    int height = buff_iter->height;

    //assert(height == 1);

    if(buff_iter->bclass != GEN4_INPUT_OUTPUT_BUFFER) {
            printf("Error writing buffer %d: the buffer type must be GEN4_INPUT_OUTPUT_BUFFER for DWord atomic write!\n", buf_id.get_data()&0xFF);
            exit(EXIT_FAILURE);
    }

    if(op > ATOMIC_FMAX) {
        printf("Error writing buffer %d: invalid opcode for DWord atomic write!\n", buf_id.get_data()&0xFF);
        exit(EXIT_FAILURE);
    }

    if(op == ATOMIC_CMPXCHG || op == ATOMIC_FCMPWR) {
        if(N != 16) {
            printf("Error writing buffer %d: src length must be 16 for DWord atomic write when op is ATOMIC_CMPXCHG/ATOMIC_FCMPWR!\n", buf_id.get_data()&0xFF);
            exit(EXIT_FAILURE);
        }
    } else {
        if(N != 8) {
            printf("Error writing buffer %d: src length must be 8 for DWord atomic write when op is not ATOMIC_CMPXCHG/ATOMIC_FCMPWR!\n", buf_id.get_data()&0xFF);
            exit(EXIT_FAILURE);
        }
    }

    // InitializeCriticalSection(&dataport_cs);

    for (i = 0; i < 8; i++) {
        SIMDCF_ELEMENT_SKIP(i);
        pos = (global_offset + element_offset(i))*4;
        if (pos >= width*height) {
            continue;
        }
        if(buff_iter->bclass == GEN4_INPUT_OUTPUT_BUFFER) {
            switch (op) {
                case ATOMIC_ADD:
                    v(i) = *((T*)((char*)buff_iter->p_volatile + pos));
                    *((T*)((char*)buff_iter->p_volatile + pos)) = v(i) + src(i);
                    break;
                case ATOMIC_SUB:
                    v(i) = *((T*)((char*)buff_iter->p_volatile + pos));
                    *((T*)((char*)buff_iter->p_volatile + pos)) = v(i) - src(i);
                    break;
                case ATOMIC_INC:
                    v(i) = *((T*)((char*)buff_iter->p_volatile + pos));
                    *((T*)((char*)buff_iter->p_volatile + pos)) = v(i) + 1;
                    break;
                case ATOMIC_DEC:
                    v(i) = *((T*)((char*)buff_iter->p_volatile + pos));
                    *((T*)((char*)buff_iter->p_volatile + pos)) = v(i) - 1;
                    break;
                case ATOMIC_MIN:
                    v(i) = *((T*)((char*)buff_iter->p_volatile + pos));
                    if(!is_fp_type<T>::value) {
                        *((T*)((char*)buff_iter->p_volatile + pos)) = (((uint) v(i) <= (uint) src(i))? v(i): src(i));
                    }
                    break;
                case ATOMIC_MAX:
                    v(i) = *((T*)((char*)buff_iter->p_volatile + pos));
                    if(!is_fp_type<T>::value) {
                        *((T*)((char*)buff_iter->p_volatile + pos)) = (((uint) v(i) >= (uint) src(i))? v(i): src(i));
                    }
                    break;
                case ATOMIC_XCHG:
                    v(i) = *((T*)((char*)buff_iter->p_volatile + pos));
                    *((T*)((char*)buff_iter->p_volatile + pos)) = src(i);
                    break;
                case ATOMIC_CMPXCHG:
                    v(i) = *((T*)((char*)buff_iter->p_volatile + pos));
                    *((T*)((char*)buff_iter->p_volatile + pos)) = ((v(i) == src(i+8))? src(i): v(i));
                    break;
                case ATOMIC_AND:
                    v(i) = *((T*)((char*)buff_iter->p_volatile + pos));
                    if (std::is_same_v <T, uint>)
                    {
                        *((T*)((char*)buff_iter->p_volatile + pos)) = (uint)v(i) & (uint)src(i);
                    }
                    else if (std::is_same_v <T, int>)
                    {
                        *((T*)((char*)buff_iter->p_volatile + pos)) = (int)v(i) & (int)src(i);
                    }
                    else
                    {
                        printf("Error writing buffer %d: unsupported opcode/type for DWord atomic write!\n", buf_id.get_data() & 0xFF);
                        exit(EXIT_FAILURE);
                    }
                    break;
                case ATOMIC_OR:
                    v(i) = *((T*)((char*)buff_iter->p_volatile + pos));
                    if (std::is_same_v <T, uint>)
                    {
                        *((T*)((char*)buff_iter->p_volatile + pos)) = (uint)v(i) | (uint)src(i);
                    }
                    else if (std::is_same_v <T, int>)
                    {
                        *((T*)((char*)buff_iter->p_volatile + pos)) = (int)v(i) | (int)src(i);
                    }
                    else
                    {
                        printf("Error writing buffer %d: unsupported opcode/type for DWord atomic write!\n", buf_id.get_data() & 0xFF);
                        exit(EXIT_FAILURE);
                    }
                    break;
                case ATOMIC_XOR:
                    v(i) = *((T*)((char*)buff_iter->p_volatile + pos));
                    if (std::is_same_v <T, uint>)
                    {
                        *((T*)((char*)buff_iter->p_volatile + pos)) = (uint)v(i) ^ (uint)src(i);
                    }
                    else if (std::is_same_v <T, int>)
                    {
                        *((T*)((char*)buff_iter->p_volatile + pos)) = (int)v(i) ^ (int)src(i);
                    }
                    else
                    {
                        printf("Error writing buffer %d: unsupported opcode/type for DWord atomic write!\n", buf_id.get_data() & 0xFF);
                        exit(EXIT_FAILURE);
                    }
                    break;
                case ATOMIC_MINSINT:
                    v(i) = *((T*)((char*)buff_iter->p_volatile + pos));
                    *((T*)((char*)buff_iter->p_volatile + pos)) = (((int) v(i) <= (int) src(i))? v(i): src(i));
                    break;
                case ATOMIC_MAXSINT:
                    v(i) = *((T*)((char*)buff_iter->p_volatile + pos));
                    *((T*)((char*)buff_iter->p_volatile + pos)) = (((int) v(i) >= (int) src(i))? v(i): src(i));
                    break;
                case ATOMIC_FADD:
                    v(i) = *((T*)((char*)buff_iter->p_volatile + pos));
                    if (is_fp_type<T>::value) {
                        *((T*)((char*)buff_iter->p_volatile + pos)) = (float)v(i) + (float)src(i);
                    }
                    break;
                case ATOMIC_FSUB:
                    v(i) = *((T*)((char*)buff_iter->p_volatile + pos));
                    if (is_fp_type<T>::value) {
                        *((T*)((char*)buff_iter->p_volatile + pos)) = (float)v(i) - (float)src(i);
                    }
                    break;
                case ATOMIC_FCMPWR:
                    v(i) = *((T*)((char*)buff_iter->p_volatile + pos));
                    if (is_fp_type<T>::value) {
                        *((T*)((char*)buff_iter->p_volatile + pos)) = ((v(i) == src(i)) ? src(i + 8) : v(i));
                    }
                    break;
                case ATOMIC_FMIN:
                    v(i) = *((T*)((char*)buff_iter->p_volatile + pos));
                    if (is_fp_type<T>::value) {
                        *((T*)((char*)buff_iter->p_volatile + pos)) = (((float)v(i) <= (float)src(i)) ? v(i) : src(i));
                    }
                    break;
                case ATOMIC_FMAX:
                    v(i) = *((T*)((char*)buff_iter->p_volatile + pos));
                    if (is_fp_type<T>::value) {
                        *((T*)((char*)buff_iter->p_volatile + pos)) = (((float)v(i) >= (float)src(i)) ? v(i) : src(i));
                    }
                    break;
                default:
                    printf("Error writing buffer %d: invalid opcode for DWord atomic write!\n", buf_id.get_data()&0xFF);
                    exit(EXIT_FAILURE);
            }
        } else {
            printf("Error writing buffer %d: the buffer type must be GEN4_INPUT_OUTPUT_BUFFER for DWord atomic write!\n", buf_id.get_data()&0xFF);
            exit(EXIT_FAILURE);
        }
    }

    // DeleteCriticalSection(&dataport_cs);

    return true;
}

template <typename T, uint N>
CM_API bool
write(SurfaceIndex & buf_id, CmAtomicOpType op, uint global_offset, vector<uint, 8> &element_offset, vector_ref <T, N> src, vector_ref<T, 8> v)
{
    std::unique_lock<std::mutex> lk(mutexForWrite);

    static const bool conformable1 = is_dword_type<T>::value;
    uint i;
    uint pos;
    cm_list<CmEmulSys::iobuffer>::iterator buff_iter =
                                    CmEmulSys::search_buffer(buf_id.get_data()&0xFF);

    if(buff_iter == CmEmulSys::iobuffers.end()) {
        printf("Error reading buffer %d: buffer %d is not registered!\n", buf_id.get_data()&0xFF, buf_id.get_data()&0xFF);
        exit(EXIT_FAILURE);
    }

    int width = buff_iter->width;
    int height = buff_iter->height;

    //assert(height == 1);

    if(buff_iter->bclass != GEN4_INPUT_OUTPUT_BUFFER) {
            printf("Error writing buffer %d: the buffer type must be GEN4_INPUT_OUTPUT_BUFFER for DWord atomic write!\n", buf_id.get_data()&0xFF);
            exit(EXIT_FAILURE);
    }

    if(op > ATOMIC_FMAX) {
        printf("Error writing buffer %d: invalid opcode for DWord atomic write!\n", buf_id.get_data()&0xFF);
        exit(EXIT_FAILURE);
    }

    if(op == ATOMIC_CMPXCHG || op == ATOMIC_FCMPWR) {
        if(N != 16) {
            printf("Error writing buffer %d: src length must be 16 for DWord atomic write when op is ATOMIC_CMPXCHG/ATOMIC_FCMPWR!\n", buf_id.get_data()&0xFF);
            exit(EXIT_FAILURE);
        }
    } else {
        if(N != 8) {
            printf("Error writing buffer %d: src length must be 8 for DWord atomic write when op is not ATOMIC_CMPXCHG/ATOMIC_FCMPWR!\n", buf_id.get_data()&0xFF);
            exit(EXIT_FAILURE);
        }
    }

    // InitializeCriticalSection(&dataport_cs);

    for (i = 0; i < 8; i++) {
        SIMDCF_ELEMENT_SKIP(i);
        pos = (global_offset + element_offset(i))*4;
        if (pos >= width*height) {
            continue;
        }
        if(buff_iter->bclass == GEN4_INPUT_OUTPUT_BUFFER) {
            switch (op) {
                case ATOMIC_ADD:
                    v(i) = *((T*)((char*)buff_iter->p_volatile + pos));
                    *((T*)((char*)buff_iter->p_volatile + pos)) = v(i) + src(i);
                    break;
                case ATOMIC_SUB:
                    v(i) = *((T*)((char*)buff_iter->p_volatile + pos));
                    *((T*)((char*)buff_iter->p_volatile + pos)) = v(i) - src(i);
                    break;
                case ATOMIC_INC:
                    v(i) = *((T*)((char*)buff_iter->p_volatile + pos));
                    *((T*)((char*)buff_iter->p_volatile + pos)) = v(i) + 1;
                    break;
                case ATOMIC_DEC:
                    v(i) = *((T*)((char*)buff_iter->p_volatile + pos));
                    *((T*)((char*)buff_iter->p_volatile + pos)) = v(i) - 1;
                    break;
                case ATOMIC_MIN:
                    v(i) = *((T*)((char*)buff_iter->p_volatile + pos));
                    if(!is_fp_type<T>::value) {
                        *((T*)((char*)buff_iter->p_volatile + pos)) = (((uint) v(i) <= (uint) src(i))? v(i): src(i));
                    }
                    break;
                case ATOMIC_MAX:
                    v(i) = *((T*)((char*)buff_iter->p_volatile + pos));
                    if(!is_fp_type<T>::value) {
                        *((T*)((char*)buff_iter->p_volatile + pos)) = (((uint) v(i) >= (uint) src(i))? v(i): src(i));
                    }
                    break;
                case ATOMIC_XCHG:
                    v(i) = *((T*)((char*)buff_iter->p_volatile + pos));
                    *((T*)((char*)buff_iter->p_volatile + pos)) = src(i);
                    break;
                case ATOMIC_CMPXCHG:
                    v(i) = *((T*)((char*)buff_iter->p_volatile + pos));
                    *((T*)((char*)buff_iter->p_volatile + pos)) = ((v(i) == src(i+8))? src(i): v(i));
                    break;
                case ATOMIC_AND:
                    v(i) = *((T*)((char*)buff_iter->p_volatile + pos));
                    if (std::is_same_v <T, uint>)
                    {
                        *((T*)((char*)buff_iter->p_volatile + pos)) = (uint)v(i) & (uint)src(i);
                    }
                    else if (std::is_same_v <T, int>)
                    {
                        *((T*)((char*)buff_iter->p_volatile + pos)) = (int)v(i) & (int)src(i);
                    }
                    else
                    {
                        printf("Error writing buffer %d: unsupported opcode/type for DWord atomic write!\n", buf_id.get_data() & 0xFF);
                        exit(EXIT_FAILURE);
                    }
                    break;
                case ATOMIC_OR:
                    v(i) = *((T*)((char*)buff_iter->p_volatile + pos));
                    if (std::is_same_v <T, uint>)
                    {
                        *((T*)((char*)buff_iter->p_volatile + pos)) = (uint)v(i) | (uint)src(i);
                    }
                    else if (std::is_same_v <T, int>)
                    {
                        *((T*)((char*)buff_iter->p_volatile + pos)) = (int)v(i) | (int)src(i);
                    }
                    else
                    {
                        printf("Error writing buffer %d: unsupported opcode/type for DWord atomic write!\n", buf_id.get_data() & 0xFF);
                        exit(EXIT_FAILURE);
                    }
                    break;
                case ATOMIC_XOR:
                    v(i) = *((T*)((char*)buff_iter->p_volatile + pos));
                    if (std::is_same_v <T, uint>)
                    {
                        *((T*)((char*)buff_iter->p_volatile + pos)) = (uint)v(i) ^ (uint)src(i);
                    }
                    else if (std::is_same_v <T, int>)
                    {
                        *((T*)((char*)buff_iter->p_volatile + pos)) = (int)v(i) ^ (int)src(i);
                    }
                    else
                    {
                        printf("Error writing buffer %d: unsupported opcode/type for DWord atomic write!\n", buf_id.get_data() & 0xFF);
                        exit(EXIT_FAILURE);
                    }
                    break;
                case ATOMIC_MINSINT:
                    v(i) = *((T*)((char*)buff_iter->p_volatile + pos));
                    *((T*)((char*)buff_iter->p_volatile + pos)) = (((int) v(i) <= (int) src(i))? v(i): src(i));
                    break;
                case ATOMIC_MAXSINT:
                    v(i) = *((T*)((char*)buff_iter->p_volatile + pos));
                    *((T*)((char*)buff_iter->p_volatile + pos)) = (((int) v(i) >= (int) src(i))? v(i): src(i));
                    break;
                case ATOMIC_FADD:
                    v(i) = *((T*)((char*)buff_iter->p_volatile + pos));
                    if (is_fp_type<T>::value) {
                        *((T*)((char*)buff_iter->p_volatile + pos)) = (float)v(i) + (float)src(i);
                    }
                    break;
                case ATOMIC_FSUB:
                    v(i) = *((T*)((char*)buff_iter->p_volatile + pos));
                    if (is_fp_type<T>::value) {
                        *((T*)((char*)buff_iter->p_volatile + pos)) = (float)v(i) - (float)src(i);
                    }
                    break;
                case ATOMIC_FCMPWR:
                    v(i) = *((T*)((char*)buff_iter->p_volatile + pos));
                    if (is_fp_type<T>::value) {
                        *((T*)((char*)buff_iter->p_volatile + pos)) = ((v(i) == src(i)) ? src(i + 8) : v(i));
                    }
                    break;
                case ATOMIC_FMIN:
                    v(i) = *((T*)((char*)buff_iter->p_volatile + pos));
                    if (is_fp_type<T>::value) {
                        *((T*)((char*)buff_iter->p_volatile + pos)) = (((float)v(i) <= (float)src(i)) ? v(i) : src(i));
                    }
                    break;
                case ATOMIC_FMAX:
                    v(i) = *((T*)((char*)buff_iter->p_volatile + pos));
                    if (is_fp_type<T>::value) {
                        *((T*)((char*)buff_iter->p_volatile + pos)) = (((float)v(i) >= (float)src(i)) ? v(i) : src(i));
                    }
                    break;
                default:
                    printf("Error writing buffer %d: invalid opcode for DWord atomic write!\n", buf_id.get_data()&0xFF);
                    exit(EXIT_FAILURE);
            }
        } else {
            printf("Error writing buffer %d: the buffer type must be GEN4_INPUT_OUTPUT_BUFFER for DWord atomic write!\n", buf_id.get_data()&0xFF);
            exit(EXIT_FAILURE);
        }
    }

    // DeleteCriticalSection(&dataport_cs);

    return true;
}

template <typename T, uint N>
CM_API bool
write(SurfaceIndex & buf_id, CmAtomicOpType op, uint global_offset, vector<uint, 8> &element_offset, vector_ref <T, N> src, int not_used)
{
    std::unique_lock<std::mutex> lk(mutexForWrite);

    static const bool conformable1 = is_dword_type<T>::value;
    uint i;
    uint pos;
    T v;
    cm_list<CmEmulSys::iobuffer>::iterator buff_iter =
                                    CmEmulSys::search_buffer(buf_id.get_data()&0xFF);

    if (not_used != 0) {
        printf("write atomic passed destination vec as int but not NULL %x\n", not_used);
        exit(EXIT_FAILURE);
    }
    if(buff_iter == CmEmulSys::iobuffers.end()) {
        printf("Error reading buffer %d: buffer %d is not registered!\n", buf_id.get_data()&0xFF, buf_id.get_data()&0xFF);
        exit(EXIT_FAILURE);
    }

    int width = buff_iter->width;
    int height = buff_iter->height;

    //assert(height == 1);

    if(buff_iter->bclass != GEN4_INPUT_OUTPUT_BUFFER) {
            printf("Error writing buffer %d: the buffer type must be GEN4_INPUT_OUTPUT_BUFFER for DWord atomic write!\n", buf_id.get_data()&0xFF);
            exit(EXIT_FAILURE);
    }

    if(op > ATOMIC_FMAX) {
        printf("Error writing buffer %d: invalid opcode for DWord atomic write!\n", buf_id.get_data()&0xFF);
        exit(EXIT_FAILURE);
    }

    if(op == ATOMIC_CMPXCHG || op == ATOMIC_FCMPWR) {
        if(N != 16) {
            printf("Error writing buffer %d: src length must be 16 for DWord atomic write when op is ATOMIC_CMPXCHG/ATOMIC_FCMPWR!\n", buf_id.get_data()&0xFF);
            exit(EXIT_FAILURE);
        }
    } else {
        if(N != 8) {
            printf("Error writing buffer %d: src length must be 8 for DWord atomic write when op is not ATOMIC_CMPXCHG/ATOMIC_FCMPWR!\n", buf_id.get_data()&0xFF);
            exit(EXIT_FAILURE);
        }
    }

    // InitializeCriticalSection(&dataport_cs);

    for (i = 0; i < 8; i++) {
        SIMDCF_ELEMENT_SKIP(i);
        pos = (global_offset + element_offset(i))*4;
        if (pos >= width*height) {
            continue;
        }
        if(buff_iter->bclass == GEN4_INPUT_OUTPUT_BUFFER) {
            switch (op) {
                case ATOMIC_ADD:
                    v = *((T*)((char*)buff_iter->p_volatile + pos));
                    *((T*)((char*)buff_iter->p_volatile + pos)) = v + src(i);
                    break;
                case ATOMIC_SUB:
                    v = *((T*)((char*)buff_iter->p_volatile + pos));
                    *((T*)((char*)buff_iter->p_volatile + pos)) = v - src(i);
                    break;
                case ATOMIC_INC:
                    v = *((T*)((char*)buff_iter->p_volatile + pos));
                    *((T*)((char*)buff_iter->p_volatile + pos)) = v + 1;
                    break;
                case ATOMIC_DEC:
                    v = *((T*)((char*)buff_iter->p_volatile + pos));
                    *((T*)((char*)buff_iter->p_volatile + pos)) = v - 1;
                    break;
                case ATOMIC_MIN:
                    v = *((T*)((char*)buff_iter->p_volatile + pos));
                    if(!is_fp_type<T>::value) {
                        *((T*)((char*)buff_iter->p_volatile + pos)) = (((uint) v <= (uint) src(i))? v: src(i));
                    }
                    break;
                case ATOMIC_MAX:
                    v = *((T*)((char*)buff_iter->p_volatile + pos));
                    if(!is_fp_type<T>::value) {
                        *((T*)((char*)buff_iter->p_volatile + pos)) = (((uint) v >= (uint) src(i))? v: src(i));
                    }
                    break;
                case ATOMIC_XCHG:
                    v = *((T*)((char*)buff_iter->p_volatile + pos));
                    *((T*)((char*)buff_iter->p_volatile + pos)) = src(i);
                    break;
                case ATOMIC_CMPXCHG:
                    v = *((T*)((char*)buff_iter->p_volatile + pos));
                    *((T*)((char*)buff_iter->p_volatile + pos)) = ((v == src(i+8))? src(i): v);
                    break;
                case ATOMIC_AND:
                    v = *((T*)((char*)buff_iter->p_volatile + pos));
                    if (std::is_same_v <T, uint>)
                    {
                        *((T*)((char*)buff_iter->p_volatile + pos)) = (uint)v & (uint)src(i);
                    }
                    else if (std::is_same_v <T, int>)
                    {
                        *((T*)((char*)buff_iter->p_volatile + pos)) = (int)v & (int)src(i);
                    }
                    else
                    {
                        printf("Error writing buffer %d: unsupported opcode/type for DWord atomic write!\n", buf_id.get_data() & 0xFF);
                        exit(EXIT_FAILURE);
                    }
                    break;
                case ATOMIC_OR:
                    v = *((T*)((char*)buff_iter->p_volatile + pos));
                    if (std::is_same_v <T, uint>)
                    {
                        *((T*)((char*)buff_iter->p_volatile + pos)) = (uint)v | (uint)src(i);
                    }
                    else if (std::is_same_v <T, int>)
                    {
                        *((T*)((char*)buff_iter->p_volatile + pos)) = (int)v | (int)src(i);
                    }
                    else
                    {
                        printf("Error writing buffer %d: unsupported opcode/type for DWord atomic write!\n", buf_id.get_data() & 0xFF);
                        exit(EXIT_FAILURE);
                    }
                    break;
                case ATOMIC_XOR:
                    v = *((T*)((char*)buff_iter->p_volatile + pos));
                    if (std::is_same_v <T, uint>)
                    {
                        *((T*)((char*)buff_iter->p_volatile + pos)) = (uint)v ^ (uint)src(i);
                    }
                    else if (std::is_same_v <T, int>)
                    {
                        *((T*)((char*)buff_iter->p_volatile + pos)) = (int)v ^ (int)src(i);
                    }
                    else
                    {
                        printf("Error writing buffer %d: unsupported opcode/type for DWord atomic write!\n", buf_id.get_data() & 0xFF);
                        exit(EXIT_FAILURE);
                    }
                    break;
                case ATOMIC_MINSINT:
                    v = *((T*)((char*)buff_iter->p_volatile + pos));
                    *((T*)((char*)buff_iter->p_volatile + pos)) = (((int) v <= (int) src(i))? v: src(i));
                    break;
                case ATOMIC_MAXSINT:
                    v = *((T*)((char*)buff_iter->p_volatile + pos));
                    *((T*)((char*)buff_iter->p_volatile + pos)) = (((int) v >= (int) src(i))? v: src(i));
                    break;
                case ATOMIC_FADD:
                    v = *((T*)((char*)buff_iter->p_volatile + pos));
                    if (is_fp_type<T>::value) {
                        *((T*)((char*)buff_iter->p_volatile + pos)) = (float)v + (float)src(i);
                    }
                    break;
                case ATOMIC_FSUB:
                    v = *((T*)((char*)buff_iter->p_volatile + pos));
                    if (is_fp_type<T>::value) {
                        *((T*)((char*)buff_iter->p_volatile + pos)) = (float)v - (float)src(i);
                    }
                    break;
                case ATOMIC_FCMPWR:
                    v = *((T*)((char*)buff_iter->p_volatile + pos));
                    if (is_fp_type<T>::value) {
                        *((T*)((char*)buff_iter->p_volatile + pos)) = ((v == src(i)) ? src(i + 8) : v);
                    }
                    break;
                case ATOMIC_FMIN:
                    v = *((T*)((char*)buff_iter->p_volatile + pos));
                    if (is_fp_type<T>::value) {
                        *((T*)((char*)buff_iter->p_volatile + pos)) = (((float)v <= (float)src(i)) ? v : src(i));
                    }
                    break;
                case ATOMIC_FMAX:
                    v = *((T*)((char*)buff_iter->p_volatile + pos));
                    if (is_fp_type<T>::value) {
                        *((T*)((char*)buff_iter->p_volatile + pos)) = (((float)v >= (float)src(i)) ? v : src(i));
                    }
                    break;
                default:
                    printf("Error writing buffer %d: invalid opcode for DWord atomic write!\n", buf_id.get_data()&0xFF);
                    exit(EXIT_FAILURE);
            }
        } else {
            printf("Error writing buffer %d: the buffer type must be GEN4_INPUT_OUTPUT_BUFFER for DWord atomic write!\n", buf_id.get_data()&0xFF);
            exit(EXIT_FAILURE);
        }
    }

    // DeleteCriticalSection(&dataport_cs);

    return true;
}

template <typename T, uint N>
CM_API bool
write(SurfaceIndex & buf_id, CmAtomicOpType op, uint global_offset, vector_ref<uint, 8> element_offset, vector <T, N> &src, vector<T, 8> &v)
{
    std::unique_lock<std::mutex> lk(mutexForWrite);

    static const bool conformable1 = is_dword_type<T>::value;
    uint i;
    uint pos;
    cm_list<CmEmulSys::iobuffer>::iterator buff_iter =
                                    CmEmulSys::search_buffer(buf_id.get_data()&0xFF);

    if(buff_iter == CmEmulSys::iobuffers.end()) {
        printf("Error reading buffer %d: buffer %d is not registered!\n", buf_id.get_data()&0xFF, buf_id.get_data()&0xFF);
        exit(EXIT_FAILURE);
    }

    int width = buff_iter->width;
    int height = buff_iter->height;

    //assert(height == 1);

    if(buff_iter->bclass != GEN4_INPUT_OUTPUT_BUFFER) {
            printf("Error writing buffer %d: the buffer type must be GEN4_INPUT_OUTPUT_BUFFER for DWord atomic write!\n", buf_id.get_data()&0xFF);
            exit(EXIT_FAILURE);
    }

    if(op > ATOMIC_FMAX) {
        printf("Error writing buffer %d: invalid opcode for DWord atomic write!\n", buf_id.get_data()&0xFF);
        exit(EXIT_FAILURE);
    }

    if(op == ATOMIC_CMPXCHG || op == ATOMIC_FCMPWR) {
        if(N != 16) {
            printf("Error writing buffer %d: src length must be 16 for DWord atomic write when op is ATOMIC_CMPXCHG/ATOMIC_FCMPWR!\n", buf_id.get_data()&0xFF);
            exit(EXIT_FAILURE);
        }
    } else {
        if(N != 8) {
            printf("Error writing buffer %d: src length must be 8 for DWord atomic write when op is not ATOMIC_CMPXCHG/ATOMIC_FCMPWR!\n", buf_id.get_data()&0xFF);
            exit(EXIT_FAILURE);
        }
    }

    // InitializeCriticalSection(&dataport_cs);

    for (i = 0; i < 8; i++) {
        SIMDCF_ELEMENT_SKIP(i);
        pos = (global_offset + element_offset(i))*4;
        if (pos >= width*height) {
            continue;
        }
        if(buff_iter->bclass == GEN4_INPUT_OUTPUT_BUFFER) {
            switch (op) {
                case ATOMIC_ADD:
                    v(i) = *((T*)((char*)buff_iter->p_volatile + pos));
                    *((T*)((char*)buff_iter->p_volatile + pos)) = v(i) + src(i);
                    break;
                case ATOMIC_SUB:
                    v(i) = *((T*)((char*)buff_iter->p_volatile + pos));
                    *((T*)((char*)buff_iter->p_volatile + pos)) = v(i) - src(i);
                    break;
                case ATOMIC_INC:
                    v(i) = *((T*)((char*)buff_iter->p_volatile + pos));
                    *((T*)((char*)buff_iter->p_volatile + pos)) = v(i) + 1;
                    break;
                case ATOMIC_DEC:
                    v(i) = *((T*)((char*)buff_iter->p_volatile + pos));
                    *((T*)((char*)buff_iter->p_volatile + pos)) = v(i) - 1;
                    break;
                case ATOMIC_MIN:
                    v(i) = *((T*)((char*)buff_iter->p_volatile + pos));
                    if(!is_fp_type<T>::value) {
                        *((T*)((char*)buff_iter->p_volatile + pos)) = (((uint) v(i) <= (uint) src(i))? v(i): src(i));
                    }
                    break;
                case ATOMIC_MAX:
                    v(i) = *((T*)((char*)buff_iter->p_volatile + pos));
                    if(!is_fp_type<T>::value) {
                        *((T*)((char*)buff_iter->p_volatile + pos)) = (((uint) v(i) >= (uint) src(i))? v(i): src(i));
                    }
                    break;
                case ATOMIC_XCHG:
                    v(i) = *((T*)((char*)buff_iter->p_volatile + pos));
                    *((T*)((char*)buff_iter->p_volatile + pos)) = src(i);
                    break;
                case ATOMIC_CMPXCHG:
                    v(i) = *((T*)((char*)buff_iter->p_volatile + pos));
                    *((T*)((char*)buff_iter->p_volatile + pos)) = ((v(i) == src(i+8))? src(i): v(i));
                    break;
                case ATOMIC_AND:
                    v(i) = *((T*)((char*)buff_iter->p_volatile + pos));
                    if (std::is_same_v <T, uint>)
                    {
                        *((T*)((char*)buff_iter->p_volatile + pos)) = (uint)v(i) & (uint)src(i);
                    }
                    else if (std::is_same_v <T, int>)
                    {
                        *((T*)((char*)buff_iter->p_volatile + pos)) = (int)v(i) & (int)src(i);
                    }
                    else
                    {
                        printf("Error writing buffer %d: unsupported opcode/type for DWord atomic write!\n", buf_id.get_data() & 0xFF);
                        exit(EXIT_FAILURE);
                    }
                    break;
                case ATOMIC_OR:
                    v(i) = *((T*)((char*)buff_iter->p_volatile + pos));
                    if (std::is_same_v <T, uint>)
                    {
                        *((T*)((char*)buff_iter->p_volatile + pos)) = (uint)v(i) | (uint)src(i);
                    }
                    else if (std::is_same_v <T, int>)
                    {
                        *((T*)((char*)buff_iter->p_volatile + pos)) = (int)v(i) | (int)src(i);
                    }
                    else
                    {
                        printf("Error writing buffer %d: unsupported opcode/type for DWord atomic write!\n", buf_id.get_data() & 0xFF);
                        exit(EXIT_FAILURE);
                    }
                    break;
                case ATOMIC_XOR:
                    v(i) = *((T*)((char*)buff_iter->p_volatile + pos));
                    if (std::is_same_v <T, uint>)
                    {
                        *((T*)((char*)buff_iter->p_volatile + pos)) = (uint)v(i) ^ (uint)src(i);
                    }
                    else if (std::is_same_v <T, int>)
                    {
                        *((T*)((char*)buff_iter->p_volatile + pos)) = (int)v(i) ^ (int)src(i);
                    }
                    else
                    {
                        printf("Error writing buffer %d: unsupported opcode/type for DWord atomic write!\n", buf_id.get_data() & 0xFF);
                        exit(EXIT_FAILURE);
                    }
                    break;
                case ATOMIC_MINSINT:
                    v(i) = *((T*)((char*)buff_iter->p_volatile + pos));
                    *((T*)((char*)buff_iter->p_volatile + pos)) = (((int) v(i) <= (int) src(i))? v(i): src(i));
                    break;
                case ATOMIC_MAXSINT:
                    v(i) = *((T*)((char*)buff_iter->p_volatile + pos));
                    *((T*)((char*)buff_iter->p_volatile + pos)) = (((int) v(i) >= (int) src(i))? v(i): src(i));
                    break;
                case ATOMIC_FADD:
                    v(i) = *((T*)((char*)buff_iter->p_volatile + pos));
                    if (is_fp_type<T>::value) {
                        *((T*)((char*)buff_iter->p_volatile + pos)) = (float)v(i) + (float)src(i);
                    }
                    break;
                case ATOMIC_FSUB:
                    v(i) = *((T*)((char*)buff_iter->p_volatile + pos));
                    if (is_fp_type<T>::value) {
                        *((T*)((char*)buff_iter->p_volatile + pos)) = (float)v(i) - (float)src(i);
                    }
                    break;
                case ATOMIC_FCMPWR:
                    v(i) = *((T*)((char*)buff_iter->p_volatile + pos));
                    if (is_fp_type<T>::value) {
                        *((T*)((char*)buff_iter->p_volatile + pos)) = ((v(i) == src(i)) ? src(i + 8) : v(i));
                    }
                    break;
                case ATOMIC_FMIN:
                    v(i) = *((T*)((char*)buff_iter->p_volatile + pos));
                    if (is_fp_type<T>::value) {
                        *((T*)((char*)buff_iter->p_volatile + pos)) = (((float)v(i) <= (float)src(i)) ? v(i) : src(i));
                    }
                    break;
                case ATOMIC_FMAX:
                    v(i) = *((T*)((char*)buff_iter->p_volatile + pos));
                    if (is_fp_type<T>::value) {
                        *((T*)((char*)buff_iter->p_volatile + pos)) = (((float)v(i) >= (float)src(i)) ? v(i) : src(i));
                    }
                    break;
                default:
                    printf("Error writing buffer %d: invalid opcode for DWord atomic write!\n", buf_id.get_data()&0xFF);
                    exit(EXIT_FAILURE);
            }
        } else {
            printf("Error writing buffer %d: the buffer type must be GEN4_INPUT_OUTPUT_BUFFER for DWord atomic write!\n", buf_id.get_data()&0xFF);
            exit(EXIT_FAILURE);
        }
    }

    // DeleteCriticalSection(&dataport_cs);

    return true;
}

template <typename T, uint N>
CM_API bool
write(SurfaceIndex & buf_id, CmAtomicOpType op, uint global_offset, vector_ref<uint, 8> element_offset, vector <T, N> &src, vector_ref<T, 8> v)
{
    std::unique_lock<std::mutex> lk(mutexForWrite);

    static const bool conformable1 = is_dword_type<T>::value;
    uint i;
    uint pos;
    cm_list<CmEmulSys::iobuffer>::iterator buff_iter =
                                    CmEmulSys::search_buffer(buf_id.get_data()&0xFF);

    if(buff_iter == CmEmulSys::iobuffers.end()) {
        printf("Error reading buffer %d: buffer %d is not registered!\n", buf_id.get_data()&0xFF, buf_id.get_data()&0xFF);
        exit(EXIT_FAILURE);
    }

    int width = buff_iter->width;
    int height = buff_iter->height;

    //assert(height == 1);

    if(buff_iter->bclass != GEN4_INPUT_OUTPUT_BUFFER) {
            printf("Error writing buffer %d: the buffer type must be GEN4_INPUT_OUTPUT_BUFFER for DWord atomic write!\n", buf_id.get_data()&0xFF);
            exit(EXIT_FAILURE);
    }

    if(op > ATOMIC_FMAX) {
        printf("Error writing buffer %d: invalid opcode for DWord atomic write!\n", buf_id.get_data()&0xFF);
        exit(EXIT_FAILURE);
    }

    if(op == ATOMIC_CMPXCHG || op == ATOMIC_FCMPWR) {
        if(N != 16) {
            printf("Error writing buffer %d: src length must be 16 for DWord atomic write when op is ATOMIC_CMPXCHG/ATOMIC_FCMPWR!\n", buf_id.get_data()&0xFF);
            exit(EXIT_FAILURE);
        }
    } else {
        if(N != 8) {
            printf("Error writing buffer %d: src length must be 8 for DWord atomic write when op is not ATOMIC_CMPXCHG/ATOMIC_FCMPWR!\n", buf_id.get_data()&0xFF);
            exit(EXIT_FAILURE);
        }
    }

    // InitializeCriticalSection(&dataport_cs);

    for (i = 0; i < 8; i++) {
        SIMDCF_ELEMENT_SKIP(i);
        pos = (global_offset + element_offset(i))*4;
        if (pos >= width*height) {
            continue;
        }
        if(buff_iter->bclass == GEN4_INPUT_OUTPUT_BUFFER) {
            switch (op) {
                case ATOMIC_ADD:
                    v(i) = *((T*)((char*)buff_iter->p_volatile + pos));
                    *((T*)((char*)buff_iter->p_volatile + pos)) = v(i) + src(i);
                    break;
                case ATOMIC_SUB:
                    v(i) = *((T*)((char*)buff_iter->p_volatile + pos));
                    *((T*)((char*)buff_iter->p_volatile + pos)) = v(i) - src(i);
                    break;
                case ATOMIC_INC:
                    v(i) = *((T*)((char*)buff_iter->p_volatile + pos));
                    *((T*)((char*)buff_iter->p_volatile + pos)) = v(i) + 1;
                    break;
                case ATOMIC_DEC:
                    v(i) = *((T*)((char*)buff_iter->p_volatile + pos));
                    *((T*)((char*)buff_iter->p_volatile + pos)) = v(i) - 1;
                    break;
                case ATOMIC_MIN:
                    v(i) = *((T*)((char*)buff_iter->p_volatile + pos));
                    if(!is_fp_type<T>::value) {
                        *((T*)((char*)buff_iter->p_volatile + pos)) = (((uint) v(i) <= (uint) src(i))? v(i): src(i));
                    }
                    break;
                case ATOMIC_MAX:
                    v(i) = *((T*)((char*)buff_iter->p_volatile + pos));
                    if(!is_fp_type<T>::value) {
                        *((T*)((char*)buff_iter->p_volatile + pos)) = (((uint) v(i) >= (uint) src(i))? v(i): src(i));
                    }
                    break;
                case ATOMIC_XCHG:
                    v(i) = *((T*)((char*)buff_iter->p_volatile + pos));
                    *((T*)((char*)buff_iter->p_volatile + pos)) = src(i);
                    break;
                case ATOMIC_CMPXCHG:
                    v(i) = *((T*)((char*)buff_iter->p_volatile + pos));
                    *((T*)((char*)buff_iter->p_volatile + pos)) = ((v(i) == src(i+8))? src(i): v(i));
                    break;
                case ATOMIC_AND:
                    v(i) = *((T*)((char*)buff_iter->p_volatile + pos));
                    if (std::is_same_v <T, uint>)
                    {
                        *((T*)((char*)buff_iter->p_volatile + pos)) = (uint)v(i) & (uint)src(i);
                    }
                    else if (std::is_same_v <T, int>)
                    {
                        *((T*)((char*)buff_iter->p_volatile + pos)) = (int)v(i) & (int)src(i);
                    }
                    else
                    {
                        printf("Error writing buffer %d: unsupported opcode/type for DWord atomic write!\n", buf_id.get_data() & 0xFF);
                        exit(EXIT_FAILURE);
                    }
                    break;
                case ATOMIC_OR:
                    v(i) = *((T*)((char*)buff_iter->p_volatile + pos));
                    if (std::is_same_v <T, uint>)
                    {
                        *((T*)((char*)buff_iter->p_volatile + pos)) = (uint)v(i) | (uint)src(i);
                    }
                    else if (std::is_same_v <T, int>)
                    {
                        *((T*)((char*)buff_iter->p_volatile + pos)) = (int)v(i) | (int)src(i);
                    }
                    else
                    {
                        printf("Error writing buffer %d: unsupported opcode/type for DWord atomic write!\n", buf_id.get_data() & 0xFF);
                        exit(EXIT_FAILURE);
                    }
                    break;
                case ATOMIC_XOR:
                    v(i) = *((T*)((char*)buff_iter->p_volatile + pos));
                    if (std::is_same_v <T, uint>)
                    {
                        *((T*)((char*)buff_iter->p_volatile + pos)) = (uint)v(i) ^ (uint)src(i);
                    }
                    else if (std::is_same_v <T, int>)
                    {
                        *((T*)((char*)buff_iter->p_volatile + pos)) = (int)v(i) ^ (int)src(i);
                    }
                    else
                    {
                        printf("Error writing buffer %d: unsupported opcode/type for DWord atomic write!\n", buf_id.get_data() & 0xFF);
                        exit(EXIT_FAILURE);
                    }
                    break;
                case ATOMIC_MINSINT:
                    v(i) = *((T*)((char*)buff_iter->p_volatile + pos));
                    *((T*)((char*)buff_iter->p_volatile + pos)) = (((int) v(i) <= (int) src(i))? v(i): src(i));
                    break;
                case ATOMIC_MAXSINT:
                    v(i) = *((T*)((char*)buff_iter->p_volatile + pos));
                    *((T*)((char*)buff_iter->p_volatile + pos)) = (((int) v(i) >= (int) src(i))? v(i): src(i));
                    break;
                case ATOMIC_FADD:
                    v(i) = *((T*)((char*)buff_iter->p_volatile + pos));
                    if (is_fp_type<T>::value) {
                        *((T*)((char*)buff_iter->p_volatile + pos)) = (float)v(i) + (float)src(i);
                    }
                    break;
                case ATOMIC_FSUB:
                    v(i) = *((T*)((char*)buff_iter->p_volatile + pos));
                    if (is_fp_type<T>::value) {
                        *((T*)((char*)buff_iter->p_volatile + pos)) = (float)v(i) - (float)src(i);
                    }
                    break;
                case ATOMIC_FCMPWR:
                    v(i) = *((T*)((char*)buff_iter->p_volatile + pos));
                    if (is_fp_type<T>::value) {
                        *((T*)((char*)buff_iter->p_volatile + pos)) = ((v(i) == src(i)) ? src(i + 8) : v(i));
                    }
                    break;
                case ATOMIC_FMIN:
                    v(i) = *((T*)((char*)buff_iter->p_volatile + pos));
                    if (is_fp_type<T>::value) {
                        *((T*)((char*)buff_iter->p_volatile + pos)) = (((float)v(i) <= (float)src(i)) ? v(i) : src(i));
                    }
                    break;
                case ATOMIC_FMAX:
                    v(i) = *((T*)((char*)buff_iter->p_volatile + pos));
                    if (is_fp_type<T>::value) {
                        *((T*)((char*)buff_iter->p_volatile + pos)) = (((float)v(i) >= (float)src(i)) ? v(i) : src(i));
                    }
                    break;
                default:
                    printf("Error writing buffer %d: invalid opcode for DWord atomic write!\n", buf_id.get_data()&0xFF);
                    exit(EXIT_FAILURE);
            }
        } else {
            printf("Error writing buffer %d: the buffer type must be GEN4_INPUT_OUTPUT_BUFFER for DWord atomic write!\n", buf_id.get_data()&0xFF);
            exit(EXIT_FAILURE);
        }
    }

    // DeleteCriticalSection(&dataport_cs);

    return true;
}

template <typename T, uint N>
CM_API bool
write(SurfaceIndex & buf_id, CmAtomicOpType op, uint global_offset, vector_ref<uint, 8> element_offset, vector <T, N> &src, int not_used)
{
    std::unique_lock<std::mutex> lk(mutexForWrite);

    static const bool conformable1 = is_dword_type<T>::value;
    uint i;
    uint pos;
    T v;
    cm_list<CmEmulSys::iobuffer>::iterator buff_iter =
                                    CmEmulSys::search_buffer(buf_id.get_data()&0xFF);

    if (not_used != 0) {
        printf("write atomic passed destination vec as int but not NULL %x\n", not_used);
        exit(EXIT_FAILURE);
    }
    if(buff_iter == CmEmulSys::iobuffers.end()) {
        printf("Error reading buffer %d: buffer %d is not registered!\n", buf_id.get_data()&0xFF, buf_id.get_data()&0xFF);
        exit(EXIT_FAILURE);
    }

    int width = buff_iter->width;
    int height = buff_iter->height;

    //assert(height == 1);

    if(buff_iter->bclass != GEN4_INPUT_OUTPUT_BUFFER) {
            printf("Error writing buffer %d: the buffer type must be GEN4_INPUT_OUTPUT_BUFFER for DWord atomic write!\n", buf_id.get_data()&0xFF);
            exit(EXIT_FAILURE);
    }

    if(op > ATOMIC_FMAX) {
        printf("Error writing buffer %d: invalid opcode for DWord atomic write!\n", buf_id.get_data()&0xFF);
        exit(EXIT_FAILURE);
    }

    if(op == ATOMIC_CMPXCHG || op == ATOMIC_FCMPWR) {
        if(N != 16) {
            printf("Error writing buffer %d: src length must be 16 for DWord atomic write when op is ATOMIC_CMPXCHG/ATOMIC_FCMPWR!\n", buf_id.get_data()&0xFF);
            exit(EXIT_FAILURE);
        }
    } else {
        if(N != 8) {
            printf("Error writing buffer %d: src length must be 8 for DWord atomic write when op is not ATOMIC_CMPXCHG/ATOMIC_FCMPWR!\n", buf_id.get_data()&0xFF);
            exit(EXIT_FAILURE);
        }
    }

    // InitializeCriticalSection(&dataport_cs);

    for (i = 0; i < 8; i++) {
        SIMDCF_ELEMENT_SKIP(i);
        pos = (global_offset + element_offset(i))*4;
        if (pos >= width*height) {
            continue;
        }
        if(buff_iter->bclass == GEN4_INPUT_OUTPUT_BUFFER) {
            switch (op) {
                case ATOMIC_ADD:
                    v = *((T*)((char*)buff_iter->p_volatile + pos));
                    *((T*)((char*)buff_iter->p_volatile + pos)) = v + src(i);
                    break;
                case ATOMIC_SUB:
                    v = *((T*)((char*)buff_iter->p_volatile + pos));
                    *((T*)((char*)buff_iter->p_volatile + pos)) = v - src(i);
                    break;
                case ATOMIC_INC:
                    v = *((T*)((char*)buff_iter->p_volatile + pos));
                    *((T*)((char*)buff_iter->p_volatile + pos)) = v + 1;
                    break;
                case ATOMIC_DEC:
                    v = *((T*)((char*)buff_iter->p_volatile + pos));
                    *((T*)((char*)buff_iter->p_volatile + pos)) = v - 1;
                    break;
                case ATOMIC_MIN:
                    v = *((T*)((char*)buff_iter->p_volatile + pos));
                    if(!is_fp_type<T>::value) {
                        *((T*)((char*)buff_iter->p_volatile + pos)) = (((uint) v <= (uint) src(i))? v: src(i));
                    }
                    break;
                case ATOMIC_MAX:
                    v = *((T*)((char*)buff_iter->p_volatile + pos));
                    if(!is_fp_type<T>::value) {
                        *((T*)((char*)buff_iter->p_volatile + pos)) = (((uint) v >= (uint) src(i))? v: src(i));
                    }
                    break;
                case ATOMIC_XCHG:
                    v = *((T*)((char*)buff_iter->p_volatile + pos));
                    *((T*)((char*)buff_iter->p_volatile + pos)) = src(i);
                    break;
                case ATOMIC_CMPXCHG:
                    v = *((T*)((char*)buff_iter->p_volatile + pos));
                    *((T*)((char*)buff_iter->p_volatile + pos)) = ((v == src(i+8))? src(i): v);
                    break;
                case ATOMIC_AND:
                    v = *((T*)((char*)buff_iter->p_volatile + pos));
                    if (std::is_same_v <T, uint>)
                    {
                        *((T*)((char*)buff_iter->p_volatile + pos)) = (uint)v & (uint)src(i);
                    }
                    else if (std::is_same_v <T, int>)
                    {
                        *((T*)((char*)buff_iter->p_volatile + pos)) = (int)v & (int)src(i);
                    }
                    else
                    {
                        printf("Error writing buffer %d: unsupported opcode/type for DWord atomic write!\n", buf_id.get_data() & 0xFF);
                        exit(EXIT_FAILURE);
                    }
                    break;
                case ATOMIC_OR:
                    v = *((T*)((char*)buff_iter->p_volatile + pos));
                    if (std::is_same_v <T, uint>)
                    {
                        *((T*)((char*)buff_iter->p_volatile + pos)) = (uint)v | (uint)src(i);
                    }
                    else if (std::is_same_v <T, int>)
                    {
                        *((T*)((char*)buff_iter->p_volatile + pos)) = (int)v | (int)src(i);
                    }
                    else
                    {
                        printf("Error writing buffer %d: unsupported opcode/type for DWord atomic write!\n", buf_id.get_data() & 0xFF);
                        exit(EXIT_FAILURE);
                    }
                    break;
                case ATOMIC_XOR:
                    v = *((T*)((char*)buff_iter->p_volatile + pos));
                    if (std::is_same_v <T, uint>)
                    {
                        *((T*)((char*)buff_iter->p_volatile + pos)) = (uint)v ^ (uint)src(i);
                    }
                    else if (std::is_same_v <T, int>)
                    {
                        *((T*)((char*)buff_iter->p_volatile + pos)) = (int)v ^ (int)src(i);
                    }
                    else
                    {
                        printf("Error writing buffer %d: unsupported opcode/type for DWord atomic write!\n", buf_id.get_data() & 0xFF);
                        exit(EXIT_FAILURE);
                    }
                    break;
                case ATOMIC_MINSINT:
                    v = *((T*)((char*)buff_iter->p_volatile + pos));
                    *((T*)((char*)buff_iter->p_volatile + pos)) = (((int) v <= (int) src(i))? v: src(i));
                    break;
                case ATOMIC_MAXSINT:
                    v = *((T*)((char*)buff_iter->p_volatile + pos));
                    *((T*)((char*)buff_iter->p_volatile + pos)) = (((int) v >= (int) src(i))? v: src(i));
                    break;
                case ATOMIC_FADD:
                    v = *((T*)((char*)buff_iter->p_volatile + pos));
                    if (is_fp_type<T>::value) {
                        *((T*)((char*)buff_iter->p_volatile + pos)) = (float)v + (float)src(i);
                    }
                    break;
                case ATOMIC_FSUB:
                    v = *((T*)((char*)buff_iter->p_volatile + pos));
                    if (is_fp_type<T>::value) {
                        *((T*)((char*)buff_iter->p_volatile + pos)) = (float)v - (float)src(i);
                    }
                    break;
                case ATOMIC_FCMPWR:
                    v = *((T*)((char*)buff_iter->p_volatile + pos));
                    if (is_fp_type<T>::value) {
                        *((T*)((char*)buff_iter->p_volatile + pos)) = ((v == src(i)) ? src(i + 8) : v);
                    }
                    break;
                case ATOMIC_FMIN:
                    v = *((T*)((char*)buff_iter->p_volatile + pos));
                    if (is_fp_type<T>::value) {
                        *((T*)((char*)buff_iter->p_volatile + pos)) = (((float)v <= (float)src(i)) ? v : src(i));
                    }
                    break;
                case ATOMIC_FMAX:
                    v = *((T*)((char*)buff_iter->p_volatile + pos));
                    if (is_fp_type<T>::value) {
                        *((T*)((char*)buff_iter->p_volatile + pos)) = (((float)v >= (float)src(i)) ? v : src(i));
                    }
                    break;
                default:
                    printf("Error writing buffer %d: invalid opcode for DWord atomic write!\n", buf_id.get_data()&0xFF);
                    exit(EXIT_FAILURE);
            }
        } else {
            printf("Error writing buffer %d: the buffer type must be GEN4_INPUT_OUTPUT_BUFFER for DWord atomic write!\n", buf_id.get_data()&0xFF);
            exit(EXIT_FAILURE);
        }
    }

    // DeleteCriticalSection(&dataport_cs);

    return true;
}

template <typename T, uint N>
CM_API bool
write(SurfaceIndex & buf_id, CmAtomicOpType op, uint global_offset, vector_ref<uint, 8> element_offset, vector_ref <T, N> src, vector<T, 8> &v)
{
    std::unique_lock<std::mutex> lk(mutexForWrite);

    static const bool conformable1 = is_dword_type<T>::value;
    uint i;
    uint pos;
    cm_list<CmEmulSys::iobuffer>::iterator buff_iter =
                                    CmEmulSys::search_buffer(buf_id.get_data()&0xFF);

    if(buff_iter == CmEmulSys::iobuffers.end()) {
        printf("Error reading buffer %d: buffer %d is not registered!\n", buf_id.get_data()&0xFF, buf_id.get_data()&0xFF);
        exit(EXIT_FAILURE);
    }

    int width = buff_iter->width;
    int height = buff_iter->height;

    //assert(height == 1);

    if(buff_iter->bclass != GEN4_INPUT_OUTPUT_BUFFER) {
            printf("Error writing buffer %d: the buffer type must be GEN4_INPUT_OUTPUT_BUFFER for DWord atomic write!\n", buf_id.get_data()&0xFF);
            exit(EXIT_FAILURE);
    }

    if(op > ATOMIC_FMAX) {
        printf("Error writing buffer %d: invalid opcode for DWord atomic write!\n", buf_id.get_data()&0xFF);
        exit(EXIT_FAILURE);
    }

    if(op == ATOMIC_CMPXCHG || op == ATOMIC_FCMPWR) {
        if(N != 16) {
            printf("Error writing buffer %d: src length must be 16 for DWord atomic write when op is ATOMIC_CMPXCHG/ATOMIC_FCMPWR!\n", buf_id.get_data()&0xFF);
            exit(EXIT_FAILURE);
        }
    } else {
        if(N != 8) {
            printf("Error writing buffer %d: src length must be 8 for DWord atomic write when op is not ATOMIC_CMPXCHG/ATOMIC_FCMPWR!\n", buf_id.get_data()&0xFF);
            exit(EXIT_FAILURE);
        }
    }

    // InitializeCriticalSection(&dataport_cs);

    for (i = 0; i < 8; i++) {
        SIMDCF_ELEMENT_SKIP(i);
        pos = (global_offset + element_offset(i))*4;
        if (pos >= width*height) {
            continue;
        }
        if(buff_iter->bclass == GEN4_INPUT_OUTPUT_BUFFER) {
            switch (op) {
                case ATOMIC_ADD:
                    v(i) = *((T*)((char*)buff_iter->p_volatile + pos));
                    *((T*)((char*)buff_iter->p_volatile + pos)) = v(i) + src(i);
                    break;
                case ATOMIC_SUB:
                    v(i) = *((T*)((char*)buff_iter->p_volatile + pos));
                    *((T*)((char*)buff_iter->p_volatile + pos)) = v(i) - src(i);
                    break;
                case ATOMIC_INC:
                    v(i) = *((T*)((char*)buff_iter->p_volatile + pos));
                    *((T*)((char*)buff_iter->p_volatile + pos)) = v(i) + 1;
                    break;
                case ATOMIC_DEC:
                    v(i) = *((T*)((char*)buff_iter->p_volatile + pos));
                    *((T*)((char*)buff_iter->p_volatile + pos)) = v(i) - 1;
                    break;
                case ATOMIC_MIN:
                    v(i) = *((T*)((char*)buff_iter->p_volatile + pos));
                    if(!is_fp_type<T>::value) {
                        *((T*)((char*)buff_iter->p_volatile + pos)) = (((uint) v(i) <= (uint) src(i))? v(i): src(i));
                    }
                    break;
                case ATOMIC_MAX:
                    v(i) = *((T*)((char*)buff_iter->p_volatile + pos));
                    if(!is_fp_type<T>::value) {
                        *((T*)((char*)buff_iter->p_volatile + pos)) = (((uint) v(i) >= (uint) src(i))? v(i): src(i));
                    }
                    break;
                case ATOMIC_XCHG:
                    v(i) = *((T*)((char*)buff_iter->p_volatile + pos));
                    *((T*)((char*)buff_iter->p_volatile + pos)) = src(i);
                    break;
                case ATOMIC_CMPXCHG:
                    v(i) = *((T*)((char*)buff_iter->p_volatile + pos));
                    *((T*)((char*)buff_iter->p_volatile + pos)) = ((v(i) == src(i+8))? src(i): v(i));
                    break;
                case ATOMIC_AND:
                    v(i) = *((T*)((char*)buff_iter->p_volatile + pos));
                    if (std::is_same_v <T, uint>)
                    {
                        *((T*)((char*)buff_iter->p_volatile + pos)) = (uint)v(i) & (uint)src(i);
                    }
                    else if (std::is_same_v <T, int>)
                    {
                        *((T*)((char*)buff_iter->p_volatile + pos)) = (int)v(i) & (int)src(i);
                    }
                    else
                    {
                        printf("Error writing buffer %d: unsupported opcode/type for DWord atomic write!\n", buf_id.get_data() & 0xFF);
                        exit(EXIT_FAILURE);
                    }
                    break;
                case ATOMIC_OR:
                    v(i) = *((T*)((char*)buff_iter->p_volatile + pos));
                    if (std::is_same_v <T, uint>)
                    {
                        *((T*)((char*)buff_iter->p_volatile + pos)) = (uint)v(i) | (uint)src(i);
                    }
                    else if (std::is_same_v <T, int>)
                    {
                        *((T*)((char*)buff_iter->p_volatile + pos)) = (int)v(i) | (int)src(i);
                    }
                    else
                    {
                        printf("Error writing buffer %d: unsupported opcode/type for DWord atomic write!\n", buf_id.get_data() & 0xFF);
                        exit(EXIT_FAILURE);
                    }
                    break;
                case ATOMIC_XOR:
                    v(i) = *((T*)((char*)buff_iter->p_volatile + pos));
                    if (std::is_same_v <T, uint>)
                    {
                        *((T*)((char*)buff_iter->p_volatile + pos)) = (uint)v(i) ^ (uint)src(i);
                    }
                    else if (std::is_same_v <T, int>)
                    {
                        *((T*)((char*)buff_iter->p_volatile + pos)) = (int)v(i) ^ (int)src(i);
                    }
                    else
                    {
                        printf("Error writing buffer %d: unsupported opcode/type for DWord atomic write!\n", buf_id.get_data() & 0xFF);
                        exit(EXIT_FAILURE);
                    }
                    break;
                case ATOMIC_MINSINT:
                    v(i) = *((T*)((char*)buff_iter->p_volatile + pos));
                    *((T*)((char*)buff_iter->p_volatile + pos)) = (((int) v(i) <= (int) src(i))? v(i): src(i));
                    break;
                case ATOMIC_MAXSINT:
                    v(i) = *((T*)((char*)buff_iter->p_volatile + pos));
                    *((T*)((char*)buff_iter->p_volatile + pos)) = (((int) v(i) >= (int) src(i))? v(i): src(i));
                    break;
                case ATOMIC_FADD:
                    v(i) = *((T*)((char*)buff_iter->p_volatile + pos));
                    if (is_fp_type<T>::value) {
                        *((T*)((char*)buff_iter->p_volatile + pos)) = (float)v(i) + (float)src(i);
                    }
                    break;
                case ATOMIC_FSUB:
                    v(i) = *((T*)((char*)buff_iter->p_volatile + pos));
                    if (is_fp_type<T>::value) {
                        *((T*)((char*)buff_iter->p_volatile + pos)) = (float)v(i) - (float)src(i);
                    }
                    break;
                case ATOMIC_FCMPWR:
                    v(i) = *((T*)((char*)buff_iter->p_volatile + pos));
                    if (is_fp_type<T>::value) {
                        *((T*)((char*)buff_iter->p_volatile + pos)) = ((v(i) == src(i)) ? src(i + 8) : v(i));
                    }
                    break;
                case ATOMIC_FMIN:
                    v(i) = *((T*)((char*)buff_iter->p_volatile + pos));
                    if (is_fp_type<T>::value) {
                        *((T*)((char*)buff_iter->p_volatile + pos)) = (((float)v(i) <= (float)src(i)) ? v(i) : src(i));
                    }
                    break;
                case ATOMIC_FMAX:
                    v(i) = *((T*)((char*)buff_iter->p_volatile + pos));
                    if (is_fp_type<T>::value) {
                        *((T*)((char*)buff_iter->p_volatile + pos)) = (((float)v(i) >= (float)src(i)) ? v(i) : src(i));
                    }
                    break;
                default:
                    printf("Error writing buffer %d: invalid opcode for DWord atomic write!\n", buf_id.get_data()&0xFF);
                    exit(EXIT_FAILURE);
            }
        } else {
            printf("Error writing buffer %d: the buffer type must be GEN4_INPUT_OUTPUT_BUFFER for DWord atomic write!\n", buf_id.get_data()&0xFF);
            exit(EXIT_FAILURE);
        }
    }

    // DeleteCriticalSection(&dataport_cs);

    return true;
}

// implementation of write_atomic
CM_API void __this_thread_yield();

template <CmAtomicOpType op, typename T, uint N>
void
__write_atomic_impl(vector<ushort, N> mask, SurfaceIndex &buf_id,
                    uint global_offset, vector<uint, N> element_offset,
                    vector<T, N> src, vector<T, N> src1,
                    vector_ref<T, N> v)
{
    std::unique_lock<std::mutex> lk(mutexForWrite);

    cm_list<CmEmulSys::iobuffer>::iterator buff_iter =
                                    CmEmulSys::search_buffer(buf_id.get_data() & 0xFF);

    if (buff_iter == CmEmulSys::iobuffers.end()) {
        printf("Error reading buffer %d: buffer %d is not registered!\n",
                buf_id.get_data() & 0xFF,
                buf_id.get_data() & 0xFF);
        exit(EXIT_FAILURE);
    }

    int width = buff_iter->width;
    int height = buff_iter->height;

    if (buff_iter->bclass != GEN4_INPUT_OUTPUT_BUFFER) {
        printf("Error writing buffer %d: the buffer type must \
                be GEN4_INPUT_OUTPUT_BUFFER for DWord atomic write!\n",
                buf_id.get_data() & 0xFF);
        exit(EXIT_FAILURE);
    }

    if (op > ATOMIC_FMAX) {
        printf("Error writing buffer %d: invalid opcode for DWord atomic write!\n",
                buf_id.get_data() & 0xFF);
        exit(EXIT_FAILURE);
    }

    for (int i = 0; i < N; i++) {
        SIMDCF_ELEMENT_SKIP(i);
        if (mask(i) == 0) {
            continue;
        }

        int pos = (global_offset + element_offset(i)) * 4;
        if (pos >= width * height) {
            continue;
        }

        switch (op) {
        case ATOMIC_ADD:
            v(i) = *((T*)((char*)buff_iter->p_volatile + pos));
            *((T*)((char*)buff_iter->p_volatile + pos)) = v(i) + src(i);
            break;
        case ATOMIC_SUB:
            v(i) = *((T*)((char*)buff_iter->p_volatile + pos));
            *((T*)((char*)buff_iter->p_volatile + pos)) = v(i) - src(i);
            break;
        case ATOMIC_INC:
            v(i) = *((T*)((char*)buff_iter->p_volatile + pos));
            *((T*)((char*)buff_iter->p_volatile + pos)) = v(i) + 1;
            break;
        case ATOMIC_DEC:
            v(i) = *((T*)((char*)buff_iter->p_volatile + pos));
            *((T*)((char*)buff_iter->p_volatile + pos)) = v(i) - 1;
            break;
        case ATOMIC_MIN:
            v(i) = *((T*)((char*)buff_iter->p_volatile + pos));
            if (!is_fp_type<T>::value) {
                *((T*)((char*)buff_iter->p_volatile + pos)) =
                    (((uint)v(i) <= (uint)src(i)) ? v(i) : src(i));
            }
            break;
        case ATOMIC_MAX:
            v(i) = *((T*)((char*)buff_iter->p_volatile + pos));
            if (!is_fp_type<T>::value) {
                *((T*)((char*)buff_iter->p_volatile + pos)) =
                    (((uint)v(i) >= (uint)src(i)) ? v(i) : src(i));
            }
            break;
        case ATOMIC_XCHG:
            v(i) = *((T*)((char*)buff_iter->p_volatile + pos));
            *((T*)((char*)buff_iter->p_volatile + pos)) = src(i);
            break;
        case ATOMIC_CMPXCHG:
            v(i) = *((T*)((char*)buff_iter->p_volatile + pos));
            *((T*)((char*)buff_iter->p_volatile + pos)) =
                ((v(i) == src1(i)) ? src(i) : v(i));
            break;
        case ATOMIC_AND:
            v(i) = *((T*)((char*)buff_iter->p_volatile + pos));
            if (std::is_same_v <T, uint>)
            {
                *((T*)((char*)buff_iter->p_volatile + pos)) = (uint) v(i) & (uint) src(i);
            }
            else if (std::is_same_v <T, int>)
            {
                *((T*)((char*)buff_iter->p_volatile + pos)) = (int) v(i) & (int) src(i);
            }
            else
            {
                printf("Error writing buffer %d: unsupported opcode/type for DWord atomic write!\n", buf_id.get_data() & 0xFF);
                exit(EXIT_FAILURE);
            }
            break;
        case ATOMIC_OR:
            v(i) = *((T*)((char*)buff_iter->p_volatile + pos));
            if (std::is_same_v <T, uint>)
            {
                *((T*)((char*)buff_iter->p_volatile + pos)) = (uint) v(i) | (uint) src(i);
            }
            else if (std::is_same_v <T, int>)
            {
                *((T*)((char*)buff_iter->p_volatile + pos)) = (int) v(i) | (int) src(i);
            }
            else
            {
                printf("Error writing buffer %d: unsupported opcode/type for DWord atomic write!\n", buf_id.get_data() & 0xFF);
                exit(EXIT_FAILURE);
            }
            break;
        case ATOMIC_XOR:
            v(i) = *((T*)((char*)buff_iter->p_volatile + pos));
            if (std::is_same_v <T, uint>)
            {
                *((T*)((char*)buff_iter->p_volatile + pos)) = (uint) v(i) ^ (uint) src(i);
            }
            else if (std::is_same_v <T, int>)
            {
                *((T*)((char*)buff_iter->p_volatile + pos)) = (int) v(i) ^ (int) src(i);
            }
            else
            {
                printf("Error writing buffer %d: unsupported opcode/type for DWord atomic write!\n", buf_id.get_data() & 0xFF);
                exit(EXIT_FAILURE);
            }
            break;
        case ATOMIC_MINSINT:
            v(i) = *((T*)((char*)buff_iter->p_volatile + pos));
            *((T*)((char*)buff_iter->p_volatile + pos)) = (((int)v(i) <= (int)src(i)) ? v(i) : src(i));
            break;
        case ATOMIC_MAXSINT:
            v(i) = *((T*)((char*)buff_iter->p_volatile + pos));
            *((T*)((char*)buff_iter->p_volatile + pos)) = (((int)v(i) >= (int)src(i)) ? v(i) : src(i));
            break;
        case ATOMIC_FADD:
            v(i) = *((T*)((char*)buff_iter->p_volatile + pos));
            if (is_fp_type<T>::value) {
                *((T*)((char*)buff_iter->p_volatile + pos)) = (float)v(i) + (float)src(i);
            }
            break;
        case ATOMIC_FSUB:
            v(i) = *((T*)((char*)buff_iter->p_volatile + pos));
            if (is_fp_type<T>::value) {
                *((T*)((char*)buff_iter->p_volatile + pos)) = (float)v(i) - (float)src(i);
            }
            break;
        case ATOMIC_FCMPWR:
            v(i) = *((T*)((char*)buff_iter->p_volatile + pos));
            if (is_fp_type<T>::value) {
                *((T*)((char*)buff_iter->p_volatile + pos)) =
                    ((v(i) == src(i)) ? src1(i) : v(i));
            }
            break;
        case ATOMIC_FMIN:
            v(i) = *((T*)((char*)buff_iter->p_volatile + pos));
            if (is_fp_type<T>::value) {
                *((T*)((char*)buff_iter->p_volatile + pos)) =
                    (((float)v(i) <= (float)src(i)) ? v(i) : src(i));
            }
            break;
        case ATOMIC_FMAX:
            v(i) = *((T*)((char*)buff_iter->p_volatile + pos));
            if (is_fp_type<T>::value) {
                *((T*)((char*)buff_iter->p_volatile + pos)) =
                    (((float)v(i) >= (float)src(i)) ? v(i) : src(i));
            }
            break;
        default:
            printf("Error writing buffer %d: invalid opcode for DWord atomic write!\n",
                    buf_id.get_data() & 0xFF);
            exit(EXIT_FAILURE);
        }
    }

    lk.unlock();
    __this_thread_yield();

}

// no return value
template <CmAtomicOpType Op, typename T, int N>
CM_API void
write_atomic(SurfaceIndex index, vector<uint, N> elementOffset,
             vector<T, N> src0, vector<T, N> src1) {
  vector<T, N> dummy;
  vector<ushort, N> mask = 1;
  __write_atomic_impl<Op, T, N>(mask, index, 0, elementOffset, src0, src1, dummy);
}

// mask and no return value
template <CmAtomicOpType Op, typename T, int N>
CM_API void
write_atomic(vector<ushort, N> mask, SurfaceIndex index,
             vector<uint, N> elementOffset, vector<T, N> src0,
             vector<T, N> src1) {
  vector<T, N> dummy;
  __write_atomic_impl<Op, T, N>(mask, index, 0, elementOffset, src0, src1, dummy);
}

// no return value, one source
template <CmAtomicOpType Op, typename T, int N>
CM_API void
write_atomic(SurfaceIndex index, vector<uint, N> elementOffset,
             vector<T, N> src0) {
  vector<T, N> dummy;
  vector<ushort, N> mask = 1;
  __write_atomic_impl<Op, T, N>(mask, index, 0, elementOffset, src0, dummy, dummy);
}

// mask, no return value, one source
template <CmAtomicOpType Op, typename T, int N>
CM_API void
write_atomic(vector<ushort, N> mask, SurfaceIndex index,
             vector<uint, N> elementOffset, vector<T, N> src0) {
  vector<T, N> dummy;
  __write_atomic_impl<Op, T, N>(mask, index, 0, elementOffset, src0, dummy, dummy);
}

// INC/DEC: return value
template <CmAtomicOpType Op, typename T, int N>
CM_API void
write_atomic(SurfaceIndex index, vector<uint, N> elementOffset,
             vector_ref<T, N> ret) {
  vector<uint, N> dummy;
  vector_ref<uint, N> _Ret = ret.template format<uint>();
  vector<ushort, N> mask = 1;
  __write_atomic_impl<Op, T, N>(mask, index, 0, elementOffset, dummy, dummy, _Ret);
}

// INC/DEC: mask and return value
template <CmAtomicOpType Op, typename T, int N>
CM_API void
write_atomic(vector<ushort, N> mask, SurfaceIndex index,
             vector<uint, N> elementOffset, vector_ref<T, N> ret) {
  vector<uint, N> dummy;
  vector_ref<uint, N> _Ret = ret.template format<uint>();
  __write_atomic_impl<Op, T, N>(mask, index, 0, elementOffset, dummy, dummy, _Ret);
}

// INC/DEC: no return value
template <CmAtomicOpType Op, typename T, int N>
CM_API void
write_atomic(SurfaceIndex index, vector<uint, N> elementOffset) {
  vector<T, N> dummy;
  vector<ushort, N> mask = 1;
  __write_atomic_impl<Op, T, N>(mask, index, 0, elementOffset, dummy, dummy, dummy);
}

// INC/DEC: mask and no return value
template <CmAtomicOpType Op, typename T, int N>
CM_API void
write_atomic(vector<ushort, N> mask, SurfaceIndex index,
             vector<uint, N> elementOffset) {
  vector<T, N> dummy;
  __write_atomic_impl<Op, T, N>(mask, index, 0, elementOffset, dummy, dummy, dummy);
}

// return value, one source
template <CmAtomicOpType Op, typename T, int N>
CM_API void
write_atomic(SurfaceIndex index, vector<uint, N> elementOffset,
    vector<T, N> src0, vector_ref<T, N> ret) {
    vector<T, N> dummy;
    vector_ref<T, N> _Ret = ret.template format<T>();
    vector<ushort, N> mask = 1;
    __write_atomic_impl<Op, T, N>(mask, index, 0, elementOffset, src0, dummy, _Ret);
}

// mask and return value, one source
template <CmAtomicOpType Op, typename T, int N>
CM_API void
write_atomic(vector<ushort, N> mask, SurfaceIndex index,
    vector<uint, N> elementOffset, vector<T, N> src0, vector_ref<uint, N> ret) {
    vector<T, N> dummy;
    vector_ref<uint, N> _Ret = ret.template format<uint>();
    __write_atomic_impl<Op, T, N>(mask, index, 0, elementOffset, src0, dummy, _Ret);
}

// CMPXCHG: return value
template <CmAtomicOpType Op, typename T, int N>
CM_API void
write_atomic(SurfaceIndex index, vector<uint, N> elementOffset,
             vector<T, N> src0, vector<T, N> src1, vector_ref<uint, N> ret) {
  vector_ref<uint, N> _Ret = ret.template format<uint>();
  vector<ushort, N> mask = 1;
  __write_atomic_impl<Op, T, N>(mask, index, 0, elementOffset, src0, src1, _Ret);
}

// CMPXCHG: mask and return value
template <CmAtomicOpType Op, typename T, int N>
CM_API void
write_atomic(vector<ushort, N> mask, SurfaceIndex index,
             vector<uint, N> elementOffset, vector<T, N> src0,
             vector<T, N> src1, vector_ref<uint, N> ret) {
  vector_ref<uint, N> _Ret = ret.template format<uint>();
  __write_atomic_impl<Op, T, N>(mask, index, 0, elementOffset, src0, src1, _Ret);
}

template <typename T, uint N>
CM_API bool
write(SurfaceIndex & buf_id, CmAtomicOpType op, uint global_offset, vector_ref<uint, 8> element_offset, vector_ref <T, N> src, vector_ref<T, 8> v)
{
    std::unique_lock<std::mutex> lk(mutexForWrite);

    static const bool conformable1 = is_dword_type<T>::value;
    int i;
    uint pos;
    cm_list<CmEmulSys::iobuffer>::iterator buff_iter =
                                    CmEmulSys::search_buffer(buf_id.get_data()&0xFF);

    if(buff_iter == CmEmulSys::iobuffers.end()) {
        printf("Error reading buffer %d: buffer %d is not registered!\n", buf_id.get_data()&0xFF, buf_id.get_data()&0xFF);
        exit(EXIT_FAILURE);
    }

    int width = buff_iter->width;
    int height = buff_iter->height;

    //assert(height == 1);

    if(buff_iter->bclass != GEN4_INPUT_OUTPUT_BUFFER) {
            printf("Error writing buffer %d: the buffer type must be GEN4_INPUT_OUTPUT_BUFFER for DWord atomic write!\n", buf_id.get_data()&0xFF);
            exit(EXIT_FAILURE);
    }

    if(op > ATOMIC_FMAX) {
        printf("Error writing buffer %d: invalid opcode for DWord atomic write!\n", buf_id.get_data()&0xFF);
        exit(EXIT_FAILURE);
    }

    if(op == ATOMIC_CMPXCHG || op == ATOMIC_FCMPWR) {
        if(N != 16) {
            printf("Error writing buffer %d: src length must be 16 for DWord atomic write when op is ATOMIC_CMPXCHG/ATOMIC_FCMPWR!\n", buf_id.get_data()&0xFF);
            exit(EXIT_FAILURE);
        }
    } else {
        if(N != 8) {
            printf("Error writing buffer %d: src length must be 8 for DWord atomic write when op is not ATOMIC_CMPXCHG/ATOMIC_FCMPWR!\n", buf_id.get_data()&0xFF);
            exit(EXIT_FAILURE);
        }
    }

    // InitializeCriticalSection(&dataport_cs);

    for (i = 0; i < 8; i++) {
        SIMDCF_ELEMENT_SKIP(i);
        pos = (global_offset + element_offset(i))*4;
        if (pos >= width*height) {
            continue;
        }
        if(buff_iter->bclass == GEN4_INPUT_OUTPUT_BUFFER) {
            switch (op) {
                case ATOMIC_ADD:
                    v(i) = *((T*)((char*)buff_iter->p_volatile + pos));
                    *((T*)((char*)buff_iter->p_volatile + pos)) = v(i) + src(i);
                    break;
                case ATOMIC_SUB:
                    v(i) = *((T*)((char*)buff_iter->p_volatile + pos));
                    *((T*)((char*)buff_iter->p_volatile + pos)) = v(i) - src(i);
                    break;
                case ATOMIC_INC:
                    v(i) = *((T*)((char*)buff_iter->p_volatile + pos));
                    *((T*)((char*)buff_iter->p_volatile + pos)) = v(i) + 1;
                    break;
                case ATOMIC_DEC:
                    v(i) = *((T*)((char*)buff_iter->p_volatile + pos));
                    *((T*)((char*)buff_iter->p_volatile + pos)) = v(i) - 1;
                    break;
                case ATOMIC_MIN:
                    v(i) = *((T*)((char*)buff_iter->p_volatile + pos));
                    if(!is_fp_type<T>::value) {
                        *((T*)((char*)buff_iter->p_volatile + pos)) = (((uint) v(i) <= (uint) src(i))? v(i): src(i));
                    }
                    break;
                case ATOMIC_MAX:
                    v(i) = *((T*)((char*)buff_iter->p_volatile + pos));
                    if(!is_fp_type<T>::value) {
                        *((T*)((char*)buff_iter->p_volatile + pos)) = (((uint) v(i) >= (uint) src(i))? v(i): src(i));
                    }
                    break;
                case ATOMIC_XCHG:
                    v(i) = *((T*)((char*)buff_iter->p_volatile + pos));
                    *((T*)((char*)buff_iter->p_volatile + pos)) = src(i);
                    break;
                case ATOMIC_CMPXCHG:
                    v(i) = *((T*)((char*)buff_iter->p_volatile + pos));
                    *((T*)((char*)buff_iter->p_volatile + pos)) = ((v(i) == src(i+8))? src(i): v(i));
                    break;
                case ATOMIC_AND:
                    v(i) = *((T*)((char*)buff_iter->p_volatile + pos));
                    if (std::is_same_v <T, uint>)
                    {
                        *((T*)((char*)buff_iter->p_volatile + pos)) = (uint)v(i) & (uint)src(i);
                    }
                    else if (std::is_same_v <T, int>)
                    {
                        *((T*)((char*)buff_iter->p_volatile + pos)) = (int)v(i) & (int)src(i);
                    }
                    else
                    {
                        printf("Error writing buffer %d: unsupported opcode/type for DWord atomic write!\n", buf_id.get_data() & 0xFF);
                        exit(EXIT_FAILURE);
                    }
                    break;
                case ATOMIC_OR:
                    v(i) = *((T*)((char*)buff_iter->p_volatile + pos));
                    if (std::is_same_v <T, uint>)
                    {
                        *((T*)((char*)buff_iter->p_volatile + pos)) = (uint)v(i) | (uint)src(i);
                    }
                    else if (std::is_same_v <T, int>)
                    {
                        *((T*)((char*)buff_iter->p_volatile + pos)) = (int)v(i) | (int)src(i);
                    }
                    else
                    {
                        printf("Error writing buffer %d: unsupported opcode/type for DWord atomic write!\n", buf_id.get_data() & 0xFF);
                        exit(EXIT_FAILURE);
                    }
                    break;
                case ATOMIC_XOR:
                    v(i) = *((T*)((char*)buff_iter->p_volatile + pos));
                    if (std::is_same_v <T, uint>)
                    {
                        *((T*)((char*)buff_iter->p_volatile + pos)) = (uint)v(i) ^ (uint)src(i);
                    }
                    else if (std::is_same_v <T, int>)
                    {
                        *((T*)((char*)buff_iter->p_volatile + pos)) = (int)v(i) ^ (int)src(i);
                    }
                    else
                    {
                        printf("Error writing buffer %d: unsupported opcode/type for DWord atomic write!\n", buf_id.get_data() & 0xFF);
                        exit(EXIT_FAILURE);
                    }
                    break;
                case ATOMIC_MINSINT:
                    v(i) = *((T*)((char*)buff_iter->p_volatile + pos));
                    *((T*)((char*)buff_iter->p_volatile + pos)) = (((int) v(i) <= (int) src(i))? v(i): src(i));
                    break;
                case ATOMIC_MAXSINT:
                    v(i) = *((T*)((char*)buff_iter->p_volatile + pos));
                    *((T*)((char*)buff_iter->p_volatile + pos)) = (((int) v(i) >= (int) src(i))? v(i): src(i));
                    break;
                case ATOMIC_FADD:
                    v(i) = *((T*)((char*)buff_iter->p_volatile + pos));
                    if (is_fp_type<T>::value) {
                        *((T*)((char*)buff_iter->p_volatile + pos)) = (float)v(i) + (float)src(i);
                    }
                    break;
                case ATOMIC_FSUB:
                    v(i) = *((T*)((char*)buff_iter->p_volatile + pos));
                    if (is_fp_type<T>::value) {
                        *((T*)((char*)buff_iter->p_volatile + pos)) = (float)v(i) - (float)src(i);
                    }
                    break;
                case ATOMIC_FCMPWR:
                    v(i) = *((T*)((char*)buff_iter->p_volatile + pos));
                    if (is_fp_type<T>::value) {
                        *((T*)((char*)buff_iter->p_volatile + pos)) = ((v(i) == src(i)) ? src(i + 8) : v(i));
                    }
                    break;
                case ATOMIC_FMIN:
                    v(i) = *((T*)((char*)buff_iter->p_volatile + pos));
                    if (is_fp_type<T>::value) {
                        *((T*)((char*)buff_iter->p_volatile + pos)) = (((float)v(i) <= (float)src(i)) ? v(i) : src(i));
                    }
                    break;
                case ATOMIC_FMAX:
                    v(i) = *((T*)((char*)buff_iter->p_volatile + pos));
                    if (is_fp_type<T>::value) {
                        *((T*)((char*)buff_iter->p_volatile + pos)) = (((float)v(i) >= (float)src(i)) ? v(i) : src(i));
                    }
                    break;
                default:
                    printf("Error writing buffer %d: invalid opcode for DWord atomic write!\n", buf_id.get_data()&0xFF);
                    exit(EXIT_FAILURE);
            }
        } else {
            printf("Error writing buffer %d: the buffer type must be GEN4_INPUT_OUTPUT_BUFFER for DWord atomic write!\n", buf_id.get_data()&0xFF);
            exit(EXIT_FAILURE);
        }
    }

    // DeleteCriticalSection(&dataport_cs);

    return true;
}

template <typename T, uint N>
CM_API bool
write(SurfaceIndex & buf_id, CmAtomicOpType op, uint global_offset, vector_ref<uint, 8> element_offset, vector_ref <T, N> src, int not_used)
{
    std::unique_lock<std::mutex> lk(mutexForWrite);

    static const bool conformable1 = is_dword_type<T>::value;
    int i;
    uint pos;
    T v;
    cm_list<CmEmulSys::iobuffer>::iterator buff_iter =
                                    CmEmulSys::search_buffer(buf_id.get_data()&0xFF);

    if (not_used != 0) {
        printf("write atomic passed destination vec as int but not NULL %x\n", not_used);
        exit(EXIT_FAILURE);
    }
    if(buff_iter == CmEmulSys::iobuffers.end()) {
        printf("Error reading buffer %d: buffer %d is not registered!\n", buf_id.get_data()&0xFF, buf_id.get_data()&0xFF);
        exit(EXIT_FAILURE);
    }

    int width = buff_iter->width;
    int height = buff_iter->height;

    //assert(height == 1);

    if(buff_iter->bclass != GEN4_INPUT_OUTPUT_BUFFER) {
            printf("Error writing buffer %d: the buffer type must be GEN4_INPUT_OUTPUT_BUFFER for DWord atomic write!\n", buf_id.get_data()&0xFF);
            exit(EXIT_FAILURE);
    }

    if(op > ATOMIC_FMAX) {
        printf("Error writing buffer %d: invalid opcode for DWord atomic write!\n", buf_id.get_data()&0xFF);
        exit(EXIT_FAILURE);
    }

    if(op == ATOMIC_CMPXCHG || op == ATOMIC_FCMPWR) {
        if(N != 16) {
            printf("Error writing buffer %d: src length must be 16 for DWord atomic write when op is ATOMIC_CMPXCHG/ATOMIC_FCMPWR!\n", buf_id.get_data()&0xFF);
            exit(EXIT_FAILURE);
        }
    } else {
        if(N != 8) {
            printf("Error writing buffer %d: src length must be 8 for DWord atomic write when op is not ATOMIC_CMPXCHG/ATOMIC_FCMPWR!\n", buf_id.get_data()&0xFF);
            exit(EXIT_FAILURE);
        }
    }

    // InitializeCriticalSection(&dataport_cs);

    for (i = 0; i < 8; i++) {
        SIMDCF_ELEMENT_SKIP(i);
        pos = (global_offset + element_offset(i))*4;
        if (pos >= width*height) {
            continue;
        }
        if(buff_iter->bclass == GEN4_INPUT_OUTPUT_BUFFER) {
            switch (op) {
                case ATOMIC_ADD:
                    v = *((T*)((char*)buff_iter->p_volatile + pos));
                    *((T*)((char*)buff_iter->p_volatile + pos)) = v + src(i);
                    break;
                case ATOMIC_SUB:
                    v = *((T*)((char*)buff_iter->p_volatile + pos));
                    *((T*)((char*)buff_iter->p_volatile + pos)) = v - src(i);
                    break;
                case ATOMIC_INC:
                    v = *((T*)((char*)buff_iter->p_volatile + pos));
                    *((T*)((char*)buff_iter->p_volatile + pos)) = v + 1;
                    break;
                case ATOMIC_DEC:
                    v = *((T*)((char*)buff_iter->p_volatile + pos));
                    *((T*)((char*)buff_iter->p_volatile + pos)) = v - 1;
                    break;
                case ATOMIC_MIN:
                    v = *((T*)((char*)buff_iter->p_volatile + pos));
                    if(!is_fp_type<T>::value) {
                        *((T*)((char*)buff_iter->p_volatile + pos)) = (((uint) v <= (uint) src(i))? v: src(i));
                    }
                    break;
                case ATOMIC_MAX:
                    v = *((T*)((char*)buff_iter->p_volatile + pos));
                    if(!is_fp_type<T>::value) {
                        *((T*)((char*)buff_iter->p_volatile + pos)) = (((uint) v >= (uint) src(i))? v: src(i));
                    }
                    break;
                case ATOMIC_XCHG:
                    v = *((T*)((char*)buff_iter->p_volatile + pos));
                    *((T*)((char*)buff_iter->p_volatile + pos)) = src(i);
                    break;
                case ATOMIC_CMPXCHG:
                    v = *((T*)((char*)buff_iter->p_volatile + pos));
                    *((T*)((char*)buff_iter->p_volatile + pos)) = ((v == src(i+8))? src(i): v);
                    break;
                case ATOMIC_AND:
                    v = *((T*)((char*)buff_iter->p_volatile + pos));
                    if (std::is_same_v <T, uint>)
                    {
                        *((T*)((char*)buff_iter->p_volatile + pos)) = (uint)v & (uint)src(i);
                    }
                    else if (std::is_same_v <T, int>)
                    {
                        *((T*)((char*)buff_iter->p_volatile + pos)) = (int)v & (int)src(i);
                    }
                    else
                    {
                        printf("Error writing buffer %d: unsupported opcode/type for DWord atomic write!\n", buf_id.get_data() & 0xFF);
                        exit(EXIT_FAILURE);
                    }
                    break;
                case ATOMIC_OR:
                    v = *((T*)((char*)buff_iter->p_volatile + pos));
                    if (std::is_same_v <T, uint>)
                    {
                        *((T*)((char*)buff_iter->p_volatile + pos)) = (uint)v | (uint)src(i);
                    }
                    else if (std::is_same_v <T, int>)
                    {
                        *((T*)((char*)buff_iter->p_volatile + pos)) = (int)v | (int)src(i);
                    }
                    else
                    {
                        printf("Error writing buffer %d: unsupported opcode/type for DWord atomic write!\n", buf_id.get_data() & 0xFF);
                        exit(EXIT_FAILURE);
                    }
                    break;
                case ATOMIC_XOR:
                    v = *((T*)((char*)buff_iter->p_volatile + pos));
                    if (std::is_same_v <T, uint>)
                    {
                        *((T*)((char*)buff_iter->p_volatile + pos)) = (uint)v ^ (uint)src(i);
                    }
                    else if (std::is_same_v <T, int>)
                    {
                        *((T*)((char*)buff_iter->p_volatile + pos)) = (int)v ^ (int)src(i);
                    }
                    else
                    {
                        printf("Error writing buffer %d: unsupported opcode/type for DWord atomic write!\n", buf_id.get_data() & 0xFF);
                        exit(EXIT_FAILURE);
                    }
                    break;
                case ATOMIC_MINSINT:
                    v = *((T*)((char*)buff_iter->p_volatile + pos));
                    *((T*)((char*)buff_iter->p_volatile + pos)) = (((int) v <= (int) src(i))? v: src(i));
                    break;
                case ATOMIC_MAXSINT:
                    v = *((T*)((char*)buff_iter->p_volatile + pos));
                    *((T*)((char*)buff_iter->p_volatile + pos)) = (((int) v >= (int) src(i))? v: src(i));
                    break;
                case ATOMIC_FADD:
                    v = *((T*)((char*)buff_iter->p_volatile + pos));
                    if (is_fp_type<T>::value) {
                        *((T*)((char*)buff_iter->p_volatile + pos)) = (float)v + (float)src(i);
                    }
                    break;
                case ATOMIC_FSUB:
                    v = *((T*)((char*)buff_iter->p_volatile + pos));
                    if (is_fp_type<T>::value) {
                        *((T*)((char*)buff_iter->p_volatile + pos)) = (float)v - (float)src(i);
                    }
                    break;
                case ATOMIC_FCMPWR:
                    v = *((T*)((char*)buff_iter->p_volatile + pos));
                    if (is_fp_type<T>::value) {
                        *((T*)((char*)buff_iter->p_volatile + pos)) = ((v == src(i)) ? src(i + 8) : v);
                    }
                    break;
                case ATOMIC_FMIN:
                    v = *((T*)((char*)buff_iter->p_volatile + pos));
                    if (is_fp_type<T>::value) {
                        *((T*)((char*)buff_iter->p_volatile + pos)) = (((float)v <= (float)src(i)) ? v : src(i));
                    }
                    break;
                case ATOMIC_FMAX:
                    v = *((T*)((char*)buff_iter->p_volatile + pos));
                    if (is_fp_type<T>::value) {
                        *((T*)((char*)buff_iter->p_volatile + pos)) = (((float)v >= (float)src(i)) ? v : src(i));
                    }
                    break;
                default:
                    printf("Error writing buffer %d: invalid opcode for DWord atomic write!\n", buf_id.get_data()&0xFF);
                    exit(EXIT_FAILURE);
            }
        } else {
            printf("Error writing buffer %d: the buffer type must be GEN4_INPUT_OUTPUT_BUFFER for DWord atomic write!\n", buf_id.get_data()&0xFF);
            exit(EXIT_FAILURE);
        }
    }

    // DeleteCriticalSection(&dataport_cs);

    return true;
}

template <typename T>
CM_API bool
write(SurfaceIndex & buf_id, CmAtomicOpType op, uint global_offset, vector<uint, 8> &element_offset, int src, vector<T, 8> &v)
{
    std::unique_lock<std::mutex> lk(mutexForWrite);

    static const bool conformable1 = is_dword_type<T>::value;
    int i;
    uint pos;
    cm_list<CmEmulSys::iobuffer>::iterator buff_iter =
                                    CmEmulSys::search_buffer(buf_id.get_data()&0xFF);

    if(buff_iter == CmEmulSys::iobuffers.end()) {
        printf("Error reading buffer %d: buffer %d is not registered!\n", buf_id.get_data()&0xFF, buf_id.get_data()&0xFF);
        exit(EXIT_FAILURE);
    }

    int width = buff_iter->width;
    int height = buff_iter->height;

    //assert(height == 1);

    if(buff_iter->bclass != GEN4_INPUT_OUTPUT_BUFFER) {
            printf("Error writing buffer %d: the buffer type must be GEN4_INPUT_OUTPUT_BUFFER for DWord atomic write!\n", buf_id.get_data()&0xFF);
            exit(EXIT_FAILURE);
    }

    if((op != ATOMIC_INC) && (op != ATOMIC_DEC)) {
        printf("Error writing buffer %d: NULL src is only allowed for ATOMIC_INC or ATOMIC_DEC!\n", buf_id.get_data()&0xFF);
        exit(EXIT_FAILURE);
    }

    // InitializeCriticalSection(&dataport_cs);

    for (i = 0; i < 8; i++) {
        SIMDCF_ELEMENT_SKIP(i);
        pos = (global_offset + element_offset(i))*4;
        if (pos >= width*height) {
            continue;
        }
        if(buff_iter->bclass == GEN4_INPUT_OUTPUT_BUFFER) {
            switch (op) {
                case ATOMIC_INC:
                    v(i) = *((T*)((char*)buff_iter->p_volatile + pos));
                    *((T*)((char*)buff_iter->p_volatile + pos)) = v(i) + 1;
                    break;
                case ATOMIC_DEC:
                    v(i) = *((T*)((char*)buff_iter->p_volatile + pos));
                    *((T*)((char*)buff_iter->p_volatile + pos)) = v(i) - 1;
                    break;
                default:
                    printf("Error writing buffer %d: invalid opcode for DWord atomic write!\n", buf_id.get_data()&0xFF);
                    exit(EXIT_FAILURE);
            }
        } else {
            printf("Error writing buffer %d: the buffer type must be GEN4_INPUT_OUTPUT_BUFFER for DWord atomic write!\n", buf_id.get_data()&0xFF);
            exit(EXIT_FAILURE);
        }
    }

    // DeleteCriticalSection(&dataport_cs);

    return true;
}

template <typename T>
CM_API bool
write(SurfaceIndex & buf_id, CmAtomicOpType op, uint global_offset, vector<uint, 8> &element_offset, int src, vector_ref<T, 8> v)
{
    std::unique_lock<std::mutex> lk(mutexForWrite);

    static const bool conformable1 = is_dword_type<T>::value;
    int i;
    uint pos;
    cm_list<CmEmulSys::iobuffer>::iterator buff_iter =
                                    CmEmulSys::search_buffer(buf_id.get_data()&0xFF);

    if(buff_iter == CmEmulSys::iobuffers.end()) {
        printf("Error reading buffer %d: buffer %d is not registered!\n", buf_id.get_data()&0xFF, buf_id.get_data()&0xFF);
        exit(EXIT_FAILURE);
    }

    int width = buff_iter->width;
    int height = buff_iter->height;

    //assert(height == 1);

    if(buff_iter->bclass != GEN4_INPUT_OUTPUT_BUFFER) {
            printf("Error writing buffer %d: the buffer type must be GEN4_INPUT_OUTPUT_BUFFER for DWord atomic write!\n", buf_id.get_data()&0xFF);
            exit(EXIT_FAILURE);
    }

    if((op != ATOMIC_INC) && (op != ATOMIC_DEC)) {
        printf("Error writing buffer %d: NULL src is only allowed for ATOMIC_INC or ATOMIC_DEC!\n", buf_id.get_data()&0xFF);
        exit(EXIT_FAILURE);
    }

    // InitializeCriticalSection(&dataport_cs);

    for (i = 0; i < 8; i++) {
        SIMDCF_ELEMENT_SKIP(i);
        pos = (global_offset + element_offset(i))*4;
        if (pos >= width*height) {
            continue;
        }
        if(buff_iter->bclass == GEN4_INPUT_OUTPUT_BUFFER) {
            switch (op) {
                case ATOMIC_INC:
                    v(i) = *((T*)((char*)buff_iter->p_volatile + pos));
                    *((T*)((char*)buff_iter->p_volatile + pos)) = v(i) + 1;
                    break;
                case ATOMIC_DEC:
                    v(i) = *((T*)((char*)buff_iter->p_volatile + pos));
                    *((T*)((char*)buff_iter->p_volatile + pos)) = v(i) - 1;
                    break;
                default:
                    printf("Error writing buffer %d: invalid opcode for DWord atomic write!\n", buf_id.get_data()&0xFF);
                    exit(EXIT_FAILURE);
            }
        } else {
            printf("Error writing buffer %d: the buffer type must be GEN4_INPUT_OUTPUT_BUFFER for DWord atomic write!\n", buf_id.get_data()&0xFF);
            exit(EXIT_FAILURE);
        }
    }

    // DeleteCriticalSection(&dataport_cs);

    return true;
}

template <typename T>
CM_API bool
write(SurfaceIndex & buf_id, CmAtomicOpType op, uint global_offset, vector<uint, 8> &element_offset, int src, int not_used)
{
    std::unique_lock<std::mutex> lk(mutexForWrite);

    static const bool conformable1 = is_dword_type<T>::value;
    int i;
    uint pos;
    T v;
    cm_list<CmEmulSys::iobuffer>::iterator buff_iter =
                                    CmEmulSys::search_buffer(buf_id.get_data()&0xFF);

    if (not_used != 0) {
        printf("write atomic passed destination vec as int but not NULL %x\n", not_used);
        exit(EXIT_FAILURE);
    }
    if(buff_iter == CmEmulSys::iobuffers.end()) {
        printf("Error reading buffer %d: buffer %d is not registered!\n", buf_id.get_data()&0xFF, buf_id.get_data()&0xFF);
        exit(EXIT_FAILURE);
    }

    int width = buff_iter->width;
    int height = buff_iter->height;

    //assert(height == 1);

    if(buff_iter->bclass != GEN4_INPUT_OUTPUT_BUFFER) {
            printf("Error writing buffer %d: the buffer type must be GEN4_INPUT_OUTPUT_BUFFER for DWord atomic write!\n", buf_id.get_data()&0xFF);
            exit(EXIT_FAILURE);
    }

    if((op != ATOMIC_INC) && (op != ATOMIC_DEC)) {
        printf("Error writing buffer %d: NULL src is only allowed for ATOMIC_INC or ATOMIC_DEC!\n", buf_id.get_data()&0xFF);
        exit(EXIT_FAILURE);
    }

    // InitializeCriticalSection(&dataport_cs);

    for (i = 0; i < 8; i++) {
        SIMDCF_ELEMENT_SKIP(i);
        pos = (global_offset + element_offset(i))*4;
        if (pos >= width*height) {
            continue;
        }
        if(buff_iter->bclass == GEN4_INPUT_OUTPUT_BUFFER) {
            switch (op) {
                case ATOMIC_INC:
                    v = *((T*)((char*)buff_iter->p_volatile + pos));
                    *((T*)((char*)buff_iter->p_volatile + pos)) = v + 1;
                    break;
                case ATOMIC_DEC:
                    v = *((T*)((char*)buff_iter->p_volatile + pos));
                    *((T*)((char*)buff_iter->p_volatile + pos)) = v - 1;
                    break;
                default:
                    printf("Error writing buffer %d: invalid opcode for DWord atomic write!\n", buf_id.get_data()&0xFF);
                    exit(EXIT_FAILURE);
            }
        } else {
            printf("Error writing buffer %d: the buffer type must be GEN4_INPUT_OUTPUT_BUFFER for DWord atomic write!\n", buf_id.get_data()&0xFF);
            exit(EXIT_FAILURE);
        }
    }

    // DeleteCriticalSection(&dataport_cs);

    return true;
}

template <typename T>
CM_API bool
write(SurfaceIndex & buf_id, CmAtomicOpType op, uint global_offset, vector_ref<uint, 8> element_offset, int src, vector<T, 8> &v)
{
    std::unique_lock<std::mutex> lk(mutexForWrite);

    static const bool conformable1 = is_dword_type<T>::value;
    int i;
    uint pos;
    cm_list<CmEmulSys::iobuffer>::iterator buff_iter =
                                    CmEmulSys::search_buffer(buf_id.get_data()&0xFF);

    if(buff_iter == CmEmulSys::iobuffers.end()) {
        printf("Error reading buffer %d: buffer %d is not registered!\n", buf_id.get_data()&0xFF, buf_id.get_data()&0xFF);
        exit(EXIT_FAILURE);
    }

    int width = buff_iter->width;
    int height = buff_iter->height;

    //assert(height == 1);

    if(buff_iter->bclass != GEN4_INPUT_OUTPUT_BUFFER) {
            printf("Error writing buffer %d: the buffer type must be GEN4_INPUT_OUTPUT_BUFFER for DWord atomic write!\n", buf_id.get_data()&0xFF);
            exit(EXIT_FAILURE);
    }

    if((op != ATOMIC_INC) && (op != ATOMIC_DEC)) {
        printf("Error writing buffer %d: NULL src is only allowed for ATOMIC_INC or ATOMIC_DEC!\n", buf_id.get_data()&0xFF);
        exit(EXIT_FAILURE);
    }

    // InitializeCriticalSection(&dataport_cs);

    for (i = 0; i < 8; i++) {
        SIMDCF_ELEMENT_SKIP(i);
        pos = (global_offset + element_offset(i))*4;
        if (pos >= width*height) {
            continue;
        }
        if(buff_iter->bclass == GEN4_INPUT_OUTPUT_BUFFER) {
            switch (op) {
                case ATOMIC_INC:
                    v(i) = *((T*)((char*)buff_iter->p_volatile + pos));
                    *((T*)((char*)buff_iter->p_volatile + pos)) = v(i) + 1;
                    break;
                case ATOMIC_DEC:
                    v(i) = *((T*)((char*)buff_iter->p_volatile + pos));
                    *((T*)((char*)buff_iter->p_volatile + pos)) = v(i) - 1;
                    break;
                default:
                    printf("Error writing buffer %d: invalid opcode for DWord atomic write!\n", buf_id.get_data()&0xFF);
                    exit(EXIT_FAILURE);
            }
        } else {
            printf("Error writing buffer %d: the buffer type must be GEN4_INPUT_OUTPUT_BUFFER for DWord atomic write!\n", buf_id.get_data()&0xFF);
            exit(EXIT_FAILURE);
        }
    }

    // DeleteCriticalSection(&dataport_cs);

    return true;
}

template <typename T>
CM_API bool
write(SurfaceIndex & buf_id, CmAtomicOpType op, uint global_offset, vector_ref<uint, 8> element_offset, int src, vector_ref<T, 8> v)
{
    std::unique_lock<std::mutex> lk(mutexForWrite);

    static const bool conformable1 = is_dword_type<T>::value;
    int i;
    uint pos;
    cm_list<CmEmulSys::iobuffer>::iterator buff_iter =
                                    CmEmulSys::search_buffer(buf_id.get_data()&0xFF);

    if(buff_iter == CmEmulSys::iobuffers.end()) {
        printf("Error reading buffer %d: buffer %d is not registered!\n", buf_id.get_data()&0xFF, buf_id.get_data()&0xFF);
        exit(EXIT_FAILURE);
    }

    int width = buff_iter->width;
    int height = buff_iter->height;

    //assert(height == 1);

    if(buff_iter->bclass != GEN4_INPUT_OUTPUT_BUFFER) {
            printf("Error writing buffer %d: the buffer type must be GEN4_INPUT_OUTPUT_BUFFER for DWord atomic write!\n", buf_id.get_data()&0xFF);
            exit(EXIT_FAILURE);
    }

    if((op != ATOMIC_INC) && (op != ATOMIC_DEC)) {
        printf("Error writing buffer %d: NULL src is only allowed for ATOMIC_INC or ATOMIC_DEC!\n", buf_id.get_data()&0xFF);
        exit(EXIT_FAILURE);
    }

    // InitializeCriticalSection(&dataport_cs);

    for (i = 0; i < 8; i++) {
        SIMDCF_ELEMENT_SKIP(i);
        pos = (global_offset + element_offset(i))*4;
        if (pos >= width*height) {
            continue;
        }
        if(buff_iter->bclass == GEN4_INPUT_OUTPUT_BUFFER) {
            switch (op) {
                case ATOMIC_INC:
                    v(i) = *((T*)((char*)buff_iter->p_volatile + pos));
                    *((T*)((char*)buff_iter->p_volatile + pos)) = v(i) + 1;
                    break;
                case ATOMIC_DEC:
                    v(i) = *((T*)((char*)buff_iter->p_volatile + pos));
                    *((T*)((char*)buff_iter->p_volatile + pos)) = v(i) - 1;
                    break;
                default:
                    printf("Error writing buffer %d: invalid opcode for DWord atomic write!\n", buf_id.get_data()&0xFF);
                    exit(EXIT_FAILURE);
            }
        } else {
            printf("Error writing buffer %d: the buffer type must be GEN4_INPUT_OUTPUT_BUFFER for DWord atomic write!\n", buf_id.get_data()&0xFF);
            exit(EXIT_FAILURE);
        }
    }

    // DeleteCriticalSection(&dataport_cs);

    return true;
}

template <typename T>
CM_API bool
write(SurfaceIndex & buf_id, CmAtomicOpType op, uint global_offset, vector_ref<uint, 8> element_offset, int src, int not_used)
{
    std::unique_lock<std::mutex> lk(mutexForWrite);

    static const bool conformable1 = is_dword_type<T>::value;
    int i;
    uint pos;
    cm_list<CmEmulSys::iobuffer>::iterator buff_iter =
                                    CmEmulSys::search_buffer(buf_id.get_data()&0xFF);

    if (not_used != 0) {
        printf("write atomic passed destination vec as int but not NULL %x\n", not_used);
        exit(EXIT_FAILURE);
    }
    if(buff_iter == CmEmulSys::iobuffers.end()) {
        printf("Error reading buffer %d: buffer %d is not registered!\n", buf_id.get_data()&0xFF, buf_id.get_data()&0xFF);
        exit(EXIT_FAILURE);
    }

    int width = buff_iter->width;
    int height = buff_iter->height;

    //assert(height == 1);

    if(buff_iter->bclass != GEN4_INPUT_OUTPUT_BUFFER) {
            printf("Error writing buffer %d: the buffer type must be GEN4_INPUT_OUTPUT_BUFFER for DWord atomic write!\n", buf_id.get_data()&0xFF);
            exit(EXIT_FAILURE);
    }

    if((op != ATOMIC_INC) && (op != ATOMIC_DEC)) {
        printf("Error writing buffer %d: NULL src is only allowed for ATOMIC_INC or ATOMIC_DEC!\n", buf_id.get_data()&0xFF);
        exit(EXIT_FAILURE);
    }

    // InitializeCriticalSection(&dataport_cs);

    T v;
    for (i = 0; i < 8; i++) {
        SIMDCF_ELEMENT_SKIP(i);
        pos = (global_offset + element_offset(i))*4;
        if (pos >= width*height) {
            continue;
        }
        if(buff_iter->bclass == GEN4_INPUT_OUTPUT_BUFFER) {
            switch (op) {
                case ATOMIC_INC:
                    v = *((T*)((char*)buff_iter->p_volatile + pos));
                    *((T*)((char*)buff_iter->p_volatile + pos)) = v + 1;
                    break;
                case ATOMIC_DEC:
                    v = *((T*)((char*)buff_iter->p_volatile + pos));
                    *((T*)((char*)buff_iter->p_volatile + pos)) = v - 1;
                    break;
                default:
                    printf("Error writing buffer %d: invalid opcode for DWord atomic write!\n", buf_id.get_data()&0xFF);
                    exit(EXIT_FAILURE);
            }
        } else {
            printf("Error writing buffer %d: the buffer type must be GEN4_INPUT_OUTPUT_BUFFER for DWord atomic write!\n", buf_id.get_data()&0xFF);
            exit(EXIT_FAILURE);
        }
    }

    // DeleteCriticalSection(&dataport_cs);

    return true;
}

/* This funtion reads Block and does a transpose data from genx dataport */
template <typename T, uint R, uint C>
CM_API bool
read_transpose(SurfaceIndex & buf_id, CM_READ_SIZE region_height, CM_READ_SIZE region_width, int x_pos, int y_pos, matrix<T,R,C> &in)
{
    int i,j;
    int block_width = 0;
    int block_height = 0;
    uint offset;
    cm_list<CmEmulSys::iobuffer>::iterator buff_iter =
                                    CmEmulSys::search_buffer(buf_id.get_data()&0xFF);

    if(buff_iter == CmEmulSys::iobuffers.end()) {
        printf("Error reading buffer %d: buffer %d is not registered!\n", buf_id.get_data()&0xFF, buf_id.get_data()&0xFF);
        exit(EXIT_FAILURE);
    }

    int width = buff_iter->width;
    int height = buff_iter->height;

    {
        if((buff_iter->bclass != GEN4_INPUT_BUFFER) &&
            (buff_iter->bclass != GEN4_INPUT_OUTPUT_BUFFER)) {
                printf("Error reading buffer %d: the registered buffer type is not INPUT_BUFFER!\n", buf_id.get_data()&0xFF);
                exit(EXIT_FAILURE);
        }
    }

    switch( region_height)
    {
    case CM_READ_1:
        {
            block_height = 1;
            break;
        }
    case CM_READ_2:
        {
            block_height = 2;
            break;
        }
    case CM_READ_4:
        {
            block_height = 4;
            break;
        }
    case CM_READ_8:
        {
            block_height = 8;
            break;
        }
    default:
        {
            printf("Error reading buffer %d: Invalid Block Height!\n", buf_id.get_data()&0xFF);
                exit(EXIT_FAILURE);
        }
    }

    switch( region_width)
    {
    case CM_READ_1:
        {
            block_width = 1;
            break;
        }
    case CM_READ_2:
        {
            block_width = 2;
            break;
        }
    case CM_READ_4:
        {
            block_width = 4;
            break;
        }
    case CM_READ_8:
        {
            block_width = 8;
            break;
        }
    default:
        {
            printf("Error reading buffer %d: Invalid Block Width!\n", buf_id.get_data()&0xFF);
                exit(EXIT_FAILURE);
        }
    }

    /*
        Calcultating actual offsets
        offsets are given in number of blocks
    */

    x_pos = x_pos * block_width * 4;
    y_pos = y_pos * block_height;
    if(C != block_height || R != block_width)
    {
        printf("Error reading buffer %d: Block dimensions are not equal to matrix dimensions!\n", buf_id.get_data()&0xFF);
                exit(EXIT_FAILURE);
    }

    int x_pos_a, y_pos_a;  /* Actual positions */
    int sizeofT = sizeof(T); /* Make this into a signed integer */
    uint bpp = CM_genx_bytes_per_pixel(buff_iter->pixelFormat);
    assert(((bpp - 1) & bpp) == 0); // Is power-of-2 number

    if(sizeofT != 4)
    {
        printf("Error reading buffer %d: Invalid matrix format!\n", buf_id.get_data()&0xFF);
                exit(EXIT_FAILURE);
    }
    for (i = 0; i < block_height; i++) {
        for (j = 0; j < block_width; j++) {
            x_pos_a = x_pos + j * sizeof(T);
            {
                y_pos_a = y_pos + i;
            }

            // We should check the boundary condition based on sizeof(T), x_pos_a is 0-based
            // Note: Use a signed variable; otherwise sizeof(T) is unsigned
            if ((x_pos_a + sizeofT) > width) {
                // If we're trying to read outside the boundary, limit the value of x_pos_a
                // Assumption -- We don't this situation:
                //         x_pos_a  width's boundary
                //           |      |
                //           <---type(T)--->
                // At most x_pos_a+sizeof(T) is exactly at the boundary.
                x_pos_a = width;
            }

            if (y_pos_a > height - 1 ||
                x_pos_a < 0 ||
                y_pos_a < 0 ||
                x_pos_a >= width) {
                in(j,i) = 0;
            }
            else {
                offset = y_pos_a * width + x_pos_a;
                {
                    in(j,i) = *((T*)((char*)buff_iter->p + offset));
                }
            }
        }
    }

    return true;
}

/* This funtion reads Block and does a transpose data from genx dataport */
template <typename T, uint R, uint C>
CM_API bool
read_transpose(SurfaceIndex & buf_id, CM_READ_SIZE region_height, CM_READ_SIZE region_width, int x_pos, int y_pos, matrix_ref<T,R,C> in)
{
    int i,j;
    int block_width = 0;
    int block_height = 0;
    uint offset;
    cm_list<CmEmulSys::iobuffer>::iterator buff_iter =
                                    CmEmulSys::search_buffer(buf_id.get_data()&0xFF);

    if(buff_iter == CmEmulSys::iobuffers.end()) {
        printf("Error reading buffer %d: buffer %d is not registered!\n", buf_id.get_data()&0xFF, buf_id.get_data()&0xFF);
        exit(EXIT_FAILURE);
    }

    int width = buff_iter->width;
    int height = buff_iter->height;

    {
        if((buff_iter->bclass != GEN4_INPUT_BUFFER) &&
            (buff_iter->bclass != GEN4_INPUT_OUTPUT_BUFFER)) {
                printf("Error reading buffer %d: the registered buffer type is not INPUT_BUFFER!\n", buf_id.get_data()&0xFF);
                exit(EXIT_FAILURE);
        }
    }

    switch( region_height)
    {
    case CM_READ_1:
        {
            block_height = 1;
            break;
        }
    case CM_READ_2:
        {
            block_height = 2;
            break;
        }
    case CM_READ_4:
        {
            block_height = 4;
            break;
        }
    case CM_READ_8:
        {
            block_height = 8;
            break;
        }
    default:
        {
            printf("Error reading buffer %d: Invalid Block Height!\n", buf_id.get_data()&0xFF);
                exit(EXIT_FAILURE);
        }
    }

    switch( region_width)
    {
    case CM_READ_1:
        {
            block_width = 1;
            break;
        }
    case CM_READ_2:
        {
            block_width = 2;
            break;
        }
    case CM_READ_4:
        {
            block_width = 4;
            break;
        }
    case CM_READ_8:
        {
            block_width = 8;
            break;
        }
    default:
        {
            printf("Error reading buffer %d: Invalid Block Width!\n", buf_id.get_data()&0xFF);
                exit(EXIT_FAILURE);
        }
    }

    /*
        Calcultating actual offsets
        offsets are given in number of blocks
    */

    x_pos = x_pos * block_width * 4;
    y_pos = y_pos * block_height;
    if(C != block_height || R != block_width)
    {
        printf("Error reading buffer %d: Block dimensions are not equal to matrix dimensions!\n", buf_id.get_data()&0xFF);
                exit(EXIT_FAILURE);
    }

    int x_pos_a, y_pos_a;  /* Actual positions */
    int sizeofT = sizeof(T); /* Make this into a signed integer */
    uint bpp = CM_genx_bytes_per_pixel(buff_iter->pixelFormat);
    assert(((bpp - 1) & bpp) == 0); // Is power-of-2 number

    if(sizeofT != 4)
    {
        printf("Error reading buffer %d: Invalid matrix format!\n", buf_id.get_data()&0xFF);
                exit(EXIT_FAILURE);
    }
    for (i = 0; i < block_height; i++) {
        for (j = 0; j < block_width; j++) {
            x_pos_a = x_pos + j * sizeof(T);
            {
                y_pos_a = y_pos + i;
            }

            // We should check the boundary condition based on sizeof(T), x_pos_a is 0-based
            // Note: Use a signed variable; otherwise sizeof(T) is unsigned
            if ((x_pos_a + sizeofT) > width) {
                // If we're trying to read outside the boundary, limit the value of x_pos_a
                // Assumption -- We don't this situation:
                //         x_pos_a  width's boundary
                //           |      |
                //           <---type(T)--->
                // At most x_pos_a+sizeof(T) is exactly at the boundary.
                x_pos_a = width;
            }

            if (y_pos_a > height - 1 ||
                x_pos_a < 0 ||
                y_pos_a < 0 ||
                x_pos_a >= width) {
                in(j,i) = 0;
            }
            else {
                offset = y_pos_a * width + x_pos_a;
                {
                    in(j,i) = *((T*)((char*)buff_iter->p + offset));
                }
            }
        }
    }

    return true;
}

/* Typed surface read */
template <typename RT, uint N1, uint N2>
CM_API bool
read_typed(SurfaceIndex &surfIndex, ChannelMaskType channelMask,
     matrix_ref<RT, N1, N2> &m,
     const vector<uint, N2> &u, const vector<uint, N2> &v = 0, const vector<uint, N2> &r = 0)
{
    static const bool conformable1 = check_true<is_fp_or_dword_type<RT>::value>::value;

    uchar *baseOffset, *byteOffset;
    uchar numColors=0, color[4]={0,0,0,0}, colorNext=0;

    if (channelMask & 0x1) {color[0]=1; numColors++;}
    if ((channelMask >> 1) & 0x1) {color[1]=1; numColors++;}
    if ((channelMask >> 2) & 0x1) {color[2]=1; numColors++;}
    if ((channelMask >> 3) & 0x1) {color[3]=1; numColors++;}

    if (numColors == 0) {
        fprintf(stderr, "read_typed error: At least one"
                "destination channel has to be read.\n");
        exit(EXIT_FAILURE);
    }

    if (N1 < numColors) {
        fprintf(stderr, "read_typed error: destination matrix"
                "does not have enough space to hold data.\n");
        exit(EXIT_FAILURE);
    }

    if (N2 != 8 && N2 != 16) {
        fprintf(stderr, "read_typed error: offset vector size"
                "must be 8 or 16.\n");
        exit(EXIT_FAILURE);
    }

    cm_list<CmEmulSys::iobuffer>::iterator buff_iter =
                                    CmEmulSys::search_buffer(surfIndex.get_data());

    if(buff_iter == CmEmulSys::iobuffers.end()) {
        fprintf(stderr, "Error: cannot read surface %d !\n", surfIndex.get_data());
        exit(EXIT_FAILURE);
    }

    uint width, height, depth, pitch, data_size;
    CmSurfaceFormatID surfFormat;

    surfFormat = buff_iter->pixelFormat;

    if ((surfFormat == R32_SINT) ||
        (surfFormat == R32_UINT) ||
        (surfFormat == R32_FLOAT)) {

        if(channelMask != CM_R_ENABLE) {
            fprintf(stderr, "read_typed error: only CM_R_ENABLE is supported for R32_SINT/R32_UINT/R32_FLOAT surface format.\n");
            exit(EXIT_FAILURE);
        }

        data_size = 4;
    } else if (surfFormat == R8G8B8A8_UINT) {
        data_size = 1;
    } else {
        fprintf(stderr, "read_typed error: only R32_SINT/R32_UINT/R32_FLOAT/R8G8B8A8_UINT surface formats are supported.\n");
        exit(EXIT_FAILURE);
    }

    width = buff_iter->width;
    height = buff_iter->height;
    depth = buff_iter->depth;

    if ((height == 1) && (depth == 1)) {
        baseOffset = (uchar *) buff_iter->p_volatile;

        for (uint channel =0; channel<4; channel++) {
            if (color[channel] == 0) continue;
            for (uint i=0; i<N2; i++) {
                byteOffset = baseOffset +  (data_size * u(i) + data_size * channel);
                if(data_size*(u(i) + channel) >= width) {
                    m(colorNext, i) = 0;
                } else {

                    if(surfFormat == R8G8B8A8_UINT) {
                        SIMDCF_WRAPPER(m(colorNext, i) = *( (unsigned char *)byteOffset ), N2, i);
                    } else {
                        SIMDCF_WRAPPER(m(colorNext, i) = *( (RT *)byteOffset ), N2, i);
                    }
                }
            }
            colorNext++;
        }
    } else if (depth == 1) {
        baseOffset = (uchar *) buff_iter->p_volatile;

        for (uint channel =0; channel<4; channel++) {
            if (color[channel] == 0) continue;
            for (uint i=0; i<N2; i++) {
                byteOffset = baseOffset +  (data_size * u(i) + v(i) * width + data_size * channel);
                if(data_size*(u(i) + channel) >= width ||
                   v(i) >= height) {
                    m(colorNext, i) = 0;
                } else {

                    if(surfFormat == R8G8B8A8_UINT) {
                        SIMDCF_WRAPPER(m(colorNext, i) = *( (unsigned char *)byteOffset ), N2, i);
                    } else {
                        SIMDCF_WRAPPER(m(colorNext, i) = *( (RT *)byteOffset ), N2, i);
                    }
                }
            }
            colorNext++;
        }
    } else {
        baseOffset = (uchar *) buff_iter->p_volatile;

        for (uint channel =0; channel<4; channel++) {
            if (color[channel] == 0) continue;
            for (uint i=0; i<N2; i++) {
                byteOffset = baseOffset +  (data_size * u(i) + v(i) * width + r(i) * width * height + data_size * channel);
                if(data_size*(u(i) + channel) >= width ||
                   v(i) >= height ||
                   r(i) >= depth) {
                    m(colorNext, i) = 0;
                } else {

                    if(surfFormat == R8G8B8A8_UINT) {
                        SIMDCF_WRAPPER(m(colorNext, i) = *( (unsigned char *)byteOffset ), N2, i);
                    } else {
                        SIMDCF_WRAPPER(m(colorNext, i) = *( (RT *)byteOffset ), N2, i);
                    }
                }
            }
            colorNext++;
        }
    }

    return true;
}

template <typename RT, uint N1, uint N2>
CM_API bool
read_typed(SurfaceIndex &surfIndex, ChannelMaskType channelMask,
     matrix<RT, N1, N2> &m,
     const vector<uint, N2> &u, const vector<uint, N2> &v = 0, const vector<uint, N2> &r = 0)
{
    matrix_ref<RT, N1, N2> m_ref = m;
    return read_typed(surfIndex, channelMask, m_ref, u, v, r);
}

/* Typed surface write */
template <typename RT, uint N1, uint N2>
CM_API bool
write_typed(SurfaceIndex &surfIndex, ChannelMaskType channelMask,
     matrix_ref<RT, N1, N2> &m,
     const vector<uint, N2> &u, const vector<uint, N2> &v = 0, const vector<uint, N2> &r = 0)
{
    static const bool conformable1 = check_true<is_fp_or_dword_type<RT>::value>::value;

    uchar *baseOffset, *byteOffset;
    uchar numColors=0, color[4]={0,0,0,0}, colorNext=0;

    if (channelMask & 0x1) {color[0]=1; numColors++;}
    if ((channelMask >> 1) & 0x1) {color[1]=1; numColors++;}
    if ((channelMask >> 2) & 0x1) {color[2]=1; numColors++;}
    if ((channelMask >> 3) & 0x1) {color[3]=1; numColors++;}

    if (numColors == 0) {
        fprintf(stderr, "write_typed error: At least one"
                "destination channel has to be read.\n");
        exit(EXIT_FAILURE);
    }

    if (N1 < numColors) {
        fprintf(stderr, "write_typed error: source matrix"
                "does not have enough space to hold data.\n");
        exit(EXIT_FAILURE);
    }

    if (N2 != 8 && N2 != 16) {
        fprintf(stderr, "write_typed error: offset vector size"
                "must be 8 or 16.\n");
        exit(EXIT_FAILURE);
    }

    cm_list<CmEmulSys::iobuffer>::iterator buff_iter =
                                    CmEmulSys::search_buffer(surfIndex.get_data());

    if(buff_iter == CmEmulSys::iobuffers.end()) {
        fprintf(stderr, "write_typed error: cannot read surface %d !\n", surfIndex.get_data());
        exit(EXIT_FAILURE);
    }

    uint width, height, depth, data_size;
    CmSurfaceFormatID surfFormat;

    surfFormat = buff_iter->pixelFormat;

    if ((surfFormat == R32_SINT) ||
        (surfFormat == R32_UINT) ||
        (surfFormat == R32_FLOAT)) {

        if(channelMask != CM_R_ENABLE) {
            fprintf(stderr, "write_typed error: only CM_R_ENABLE is supported for R32_SINT/R32_UINT/R32_FLOAT surface format.\n");
            exit(EXIT_FAILURE);
        }

        data_size = 4;
    } else if (surfFormat == R8G8B8A8_UINT) {
        data_size = 1;
    } else {
        fprintf(stderr, "write_typed error: only R32_SINT/R32_UINT/R32_FLOAT/R8G8B8A8_UINT surface formats are supported.\n");
        exit(EXIT_FAILURE);
    }

    width = buff_iter->width;
    height = buff_iter->height;
    depth = buff_iter->depth;

    if ((height == 1) && (depth == 1)) {
        baseOffset = (uchar *) buff_iter->p_volatile;

        for (uint channel =0; channel<4; channel++) {
            if (color[channel] == 0) continue;
            for (uint i=0; i<N2; i++) {
                byteOffset = baseOffset +  (data_size * u(i) + data_size * channel);
                if(data_size*(u(i) + channel) >= width) {
                    //break;
                } else {

                    if(surfFormat == R8G8B8A8_UINT) {
                        SIMDCF_WRAPPER(*( (unsigned char *)byteOffset ) = m(colorNext, i), N2, i);
                    } else {
                        SIMDCF_WRAPPER(*( (RT *)byteOffset ) = m(colorNext, i), N2, i);
                    }
                }
            }
            colorNext++;
        }
    } else if (depth == 1) {
        baseOffset = (uchar *) buff_iter->p_volatile;

        for (uint channel =0; channel<4; channel++) {
            if (color[channel] == 0) continue;
            for (uint i=0; i<N2; i++) {
                byteOffset = baseOffset +  (data_size * u(i) + v(i) * width + data_size * channel);
                if(data_size*(u(i) + channel) >= width ||
                   v(i) >= height) {
                    break;
                } else {

                    if(surfFormat == R8G8B8A8_UINT) {
                        SIMDCF_WRAPPER(*( (unsigned char *)byteOffset ) = m(colorNext, i), N2, i);
                    } else {
                        SIMDCF_WRAPPER(*( (RT *)byteOffset ) = m(colorNext, i), N2, i);
                    }
                }
            }
            colorNext++;
        }
    } else {
        baseOffset = (uchar *) buff_iter->p_volatile;

        for (uint channel =0; channel<4; channel++) {
            if (color[channel] == 0) continue;
            for (uint i=0; i<N2; i++) {
                byteOffset = baseOffset +  (data_size * u(i) + v(i) * width + r(i) * width * height + data_size * channel);
                if(data_size*(u(i) + channel) >= width ||
                   v(i) >= height ||
                   r(i) >= depth) {
                    break;
                } else {

                    if(surfFormat == R8G8B8A8_UINT) {
                        SIMDCF_WRAPPER(*( (unsigned char *)byteOffset ) = m(colorNext, i), N2, i);
                    } else {
                        SIMDCF_WRAPPER(*( (RT *)byteOffset ) = m(colorNext, i), N2, i);
                    }
                }
            }
            colorNext++;
        }
    }

    return true;
}

template <typename RT, uint N1, uint N2>
CM_API bool
write_typed(SurfaceIndex &surfIndex, ChannelMaskType channelMask,
     matrix<RT, N1, N2> &m,
     const vector<uint, N2> &u, const vector<uint, N2> &v = 0, const vector<uint, N2> &r = 0)
{
    matrix_ref<RT, N1, N2> m_ref = m;
    return write_typed(surfIndex, channelMask, m_ref, u, v, r);
}

/* Untyped surface read */
template <typename RT, uint N1, uint N2>
CM_API bool
read_untyped(SurfaceIndex &surfIndex, ChannelMaskType channelMask,
     matrix_ref<RT, N1, N2> &m,
     const vector<uint, N2> &u)
{
    static const bool conformable1 = check_true<is_fp_or_dword_type<RT>::value>::value;

    uchar *baseOffset, *byteOffset;
    uchar numColors=0, color[4]={0,0,0,0}, colorNext=0;

    if (channelMask & 0x1) {color[0]=1; numColors++;}
    if ((channelMask >> 1) & 0x1) {color[1]=1; numColors++;}
    if ((channelMask >> 2) & 0x1) {color[2]=1; numColors++;}
    if ((channelMask >> 3) & 0x1) {color[3]=1; numColors++;}

    if (numColors == 0) {
        fprintf(stderr, "read_untyped error: At least one "
                "destination channel has to be read.\n");
        exit(EXIT_FAILURE);
    }

    if (N1 < numColors) {
        fprintf(stderr, "read_untyped error: destination matrix "
                "does not have enough space to hold data.\n");
        exit(EXIT_FAILURE);
    }

    if (N2 != 8 && N2 != 16) {
        fprintf(stderr, "read_untyped error: offset vector size "
                "must be 8 or 16.\n");
        exit(EXIT_FAILURE);
    }

    cm_list<CmEmulSys::iobuffer>::iterator buff_iter =
                                    CmEmulSys::search_buffer(surfIndex.get_data());

    if(buff_iter == CmEmulSys::iobuffers.end()) {
        fprintf(stderr, "read_untyped error: cannot read surface %d !\n", surfIndex.get_data());
        exit(EXIT_FAILURE);
    }

    uint width, height, depth, pitch;
    CmSurfaceFormatID surfFormat;

    width = buff_iter->width;
    height = buff_iter->height;
    depth = buff_iter->depth;

    if (height != 1 || depth != 1) {
        fprintf(stderr, "read_untyped error: invalid input surface type.\n");
        exit(EXIT_FAILURE);
    }

    baseOffset = (uchar *) buff_iter->p_volatile;

    for (uint j=0; j<4; j++) {
        if (color[j] == 0) continue;
        for (uint i=0; i<N2; i++) {
            SIMDCF_ELEMENT_SKIP(i);
            int offset = sizeof(RT) * (u(i) + j);
            byteOffset = baseOffset +  offset;
            if(offset >= width) {
                m(colorNext, i) = 0;
            } else {
                m(colorNext, i) = *( (RT *)byteOffset );
            }
        }
        colorNext++;
    }

    return true;
}

template <typename RT, uint N1, uint N2>
CM_API bool
read_untyped(SurfaceIndex &surfIndex, ChannelMaskType channelMask,
     matrix<RT, N1, N2> &m,
     const vector<uint, N2> &u)
{
    matrix_ref<RT, N1, N2> m_ref = m;
    return read_untyped(surfIndex, channelMask, m_ref, u);
}

/* Untyped surface write */
template <typename RT, uint N1, uint N2>
CM_API bool
write_untyped(SurfaceIndex &surfIndex, ChannelMaskType channelMask,
     matrix_ref<RT, N1, N2> &m,
     const vector<uint, N2> &u)
{
    static const bool conformable1 = check_true<is_fp_or_dword_type<RT>::value>::value;

    uchar *baseOffset, *byteOffset;
    uchar numColors=0, color[4]={0,0,0,0}, colorNext=0;

    if(channelMask != CM_R_ENABLE &&
       channelMask != CM_GR_ENABLE &&
       channelMask != CM_BGR_ENABLE &&
       channelMask != CM_ABGR_ENABLE) {
        fprintf(stderr, "write_untyped error: Invalid channel mask.\n");
        exit(EXIT_FAILURE);
    }

    if (channelMask & 0x1) {color[0]=1; numColors++;}
    if ((channelMask >> 1) & 0x1) {color[1]=1; numColors++;}
    if ((channelMask >> 2) & 0x1) {color[2]=1; numColors++;}
    if ((channelMask >> 3) & 0x1) {color[3]=1; numColors++;}

    if (numColors == 0) {
        fprintf(stderr, "write_untyped error: At least one "
                "destination channel has to be read.\n");
        exit(EXIT_FAILURE);
    }

    if (N1 < numColors) {
        fprintf(stderr, "write_untyped error: source matrix "
                "does not have enough space to hold data.\n");
        exit(EXIT_FAILURE);
    }

    if (N2 != 8 && N2 != 16) {
        fprintf(stderr, "write_untyped error: offset vector size "
                "must be 8 or 16.\n");
        exit(EXIT_FAILURE);
    }

    cm_list<CmEmulSys::iobuffer>::iterator buff_iter =
                                    CmEmulSys::search_buffer(surfIndex.get_data());

    if(buff_iter == CmEmulSys::iobuffers.end()) {
        fprintf(stderr, "write_untyped error: cannot read surface %d !\n", surfIndex.get_data());
        exit(EXIT_FAILURE);
    }

    uint width, height, depth, pitch;
    CmSurfaceFormatID surfFormat;

    width = buff_iter->width;
    height = buff_iter->height;
    depth = buff_iter->depth;

    if (height != 1 || depth != 1) {
        fprintf(stderr, "write_untyped error: invalid input surface type.\n");
        exit(EXIT_FAILURE);
    }

    baseOffset = (uchar *) buff_iter->p_volatile;

    for (uint j=0; j<4; j++) {
        if (color[j] == 0) continue;
        for (uint i=0; i<N2; i++) {
            SIMDCF_ELEMENT_SKIP(i);
            int offset = sizeof(RT) * (u(i) + j);
            byteOffset = baseOffset +  offset;
            if(offset >= width) {
                break;
            } else {
                *( (RT *)byteOffset ) = m(colorNext, i);
            }
        }
        colorNext++;
    }

    return true;
}

template <typename RT, uint N1, uint N2>
CM_API bool
write_untyped(SurfaceIndex &surfIndex, ChannelMaskType channelMask,
     matrix<RT, N1, N2> &m,
     const vector<uint, N2> &u)
{
    matrix_ref<RT, N1, N2> m_ref = m;
    return write_untyped(surfIndex, channelMask, m_ref, u);
}

#endif /* NEW_CM_RT */

// Typed atomic write (2D atomic write)

// Note: u, v, r and LOD must all match or be NULL
// Also, to prevent combinatorial explosion, vector args must be ref-type

// Not all variants of atomic ops require src0 and src1 - in these cases a dummy vector should be
// passed in to the function

// Null return is also supported by the hardware but we always require one to limit the number of variant
// Most of the details for creating the dummy values is handled in the wrapper write_typed_atomic functions

namespace hidden {
    template <bool, class T = void> struct cm_enable_if {};
    template <class T> struct cm_enable_if<true, T> {
        typedef T type;
    };
};

template <typename T, uint N>
CM_API void __write_typed_atomic_impl(SurfaceIndex & buf_id, CmAtomicOpType op,
    vector_ref<T, N> ret, vector<T, N> src0, vector<T, N> src1,
    vector<uint, N> u, vector<uint, N> v, vector<uint, N> r, vector<uint, N> LOD)
{
    fprintf(stderr, "Error: write typed atomic is not supported in emulation mode.\n");
}

template <typename T, uint N>
CM_API void __write_typed_atomic_impl(SurfaceIndex & buf_id, CmAtomicOpType op,
    vector_ref<T, N> ret, vector<T, N> src0,int src1,
    vector<uint, N> u, vector<uint, N> v, vector<uint, N> r, vector<uint, N> LOD)
{
    fprintf(stderr, "Error: write typed atomic is not supported in emulation mode.\n");
}

template <typename T, uint N>
CM_API void __write_typed_atomic_impl(SurfaceIndex & buf_id, CmAtomicOpType op,
    vector_ref<T, N> ret,int src0, int src1,
    vector<uint, N> u, vector<uint, N> v, vector<uint, N> r, vector<uint, N> LOD)
{
    fprintf(stderr, "Error: write typed atomic is not supported in emulation mode.\n");
}

template <typename T, uint N>
CM_API void __write_typed_atomic_impl(SurfaceIndex & buf_id, CmAtomicOpType op,
    vector_ref<T, N> ret, vector<T, N> src0, vector<T, N> src1,
    vector<uint, N> u, vector<uint, N> v, vector<uint, N> r, int LOD)
{
    cm_list<CmEmulSys::iobuffer>::iterator buff_iter =
        CmEmulSys::search_buffer(buf_id.get_data() & 0xFF);

    if (buff_iter == CmEmulSys::iobuffers.end()) {
        printf("Error reading buffer %d: buffer %d is not registered!\n", buf_id.get_data() & 0xFF, buf_id.get_data() & 0xFF);
        exit(EXIT_FAILURE);
    }

    if (buff_iter->bclass != GEN4_INPUT_OUTPUT_BUFFER) {
        printf("Error writing buffer %d: the buffer type must be GEN4_INPUT_OUTPUT_BUFFER for typed atomic write!\n", buf_id.get_data() & 0xFF);
        exit(EXIT_FAILURE);
    }

    if (op > ATOMIC_FMAX) {
        printf("Error writing buffer %d: invalid opcode for typed atomic write!\n", buf_id.get_data() & 0xFF);
        exit(EXIT_FAILURE);
    }
    if (N != 1 && N != 2 && N != 4 && N != 8) {
        printf("Error writing buffer %d: src length must be 1,2,4,8 for typed atomic write!\n", buf_id.get_data() & 0xFF);
        exit(EXIT_FAILURE);
    }

    CmSurfaceFormatID surfFormat;

    surfFormat = buff_iter->pixelFormat;
    int data_size = 4;

    /*if ((surfFormat == R32_SINT) || (surfFormat == R32_UINT)) {
    data_size = 4;
    }
    else {
    fprintf(stderr, "typed atomic write error: only R32_SINT/R32_UINT surface formats are supported.\n");
    exit(EXIT_FAILURE);
    }*/

    int width = buff_iter->width;
    int height = buff_iter->height;
    int depth = buff_iter->depth;
    int pos;

    for (uint i = 0; i < N; i++) {
        pos = data_size * u(i) + v(i) * width + r(i) * height;
        if ((data_size * u(i)) >= width || v(i) >= height || r(i)>=depth) {
            continue;
        }
        else {
            switch (op) {
            case ATOMIC_CMPXCHG:
                ret(i) = *((T*)((char*)buff_iter->p_volatile + pos));
                *((T*)((char*)buff_iter->p_volatile + pos)) = ((ret(i) == src1(i)) ? src0(i) : ret(i));
                break;
            default:
                printf("Error writing buffer %d: invalid opcode for typed atomic write!\n", buf_id.get_data() & 0xFF);
                exit(EXIT_FAILURE);
            }
        }
    }
}

template <typename T, uint N>
CM_API void __write_typed_atomic_impl(SurfaceIndex & buf_id, CmAtomicOpType op,
    vector_ref<T, N> ret, vector<T, N> src0,int src1,
    vector<uint, N> u, vector<uint, N> v, vector<uint, N> r, int LOD)
{
    cm_list<CmEmulSys::iobuffer>::iterator buff_iter =
        CmEmulSys::search_buffer(buf_id.get_data() & 0xFF);

    if (buff_iter == CmEmulSys::iobuffers.end()) {
        printf("Error reading buffer %d: buffer %d is not registered!\n", buf_id.get_data() & 0xFF, buf_id.get_data() & 0xFF);
        exit(EXIT_FAILURE);
    }

    if (buff_iter->bclass != GEN4_INPUT_OUTPUT_BUFFER) {
        printf("Error writing buffer %d: the buffer type must be GEN4_INPUT_OUTPUT_BUFFER for typed atomic write!\n", buf_id.get_data() & 0xFF);
        exit(EXIT_FAILURE);
    }

    if (op > ATOMIC_FMAX) {
        printf("Error writing buffer %d: invalid opcode for typed atomic write!\n", buf_id.get_data() & 0xFF);
        exit(EXIT_FAILURE);
    }
    if (N != 1 && N != 2 && N != 4 && N != 8) {
        printf("Error writing buffer %d: src length must be 1,2,4,8 for typed atomic write!\n", buf_id.get_data() & 0xFF);
        exit(EXIT_FAILURE);
    }

    CmSurfaceFormatID surfFormat;

    surfFormat = buff_iter->pixelFormat;
    int data_size = 4;

    /*if ((surfFormat == R32_SINT) || (surfFormat == R32_UINT)) {
    data_size = 4;
    }
    else {
    fprintf(stderr, "typed atomic write error: only R32_SINT/R32_UINT surface formats are supported.\n");
    exit(EXIT_FAILURE);
    }*/

    int width = buff_iter->width;
    int height = buff_iter->height;
    int depth = buff_iter->depth;
    int pos;

    for (uint i = 0; i < N; i++) {
        pos = data_size * u(i) + v(i) * width + r(i) * height;
        if ((data_size * u(i)) >= width || v(i) >= height || r(i) >= depth) {
            continue;
        }
        else {
            switch (op) {
            case ATOMIC_ADD:
                ret(i) = *((T*)((char*)buff_iter->p_volatile + pos));
                *((T*)((char*)buff_iter->p_volatile + pos)) = ret(i) + src0(i);
                break;
            case ATOMIC_SUB:
                ret(i) = *((T*)((char*)buff_iter->p_volatile + pos));
                *((T*)((char*)buff_iter->p_volatile + pos)) = ret(i) - src0(i);
                break;
            case ATOMIC_MIN:
                ret(i) = *((T*)((char*)buff_iter->p_volatile + pos));
                if (!is_fp_type<T>::value) {
                    *((T*)((char*)buff_iter->p_volatile + pos)) = (((uint)ret(i) <= (uint)src0(i)) ? ret(i) : src0(i));
                }
                break;
            case ATOMIC_MAX:
                ret(i) = *((T*)((char*)buff_iter->p_volatile + pos));
                if (!is_fp_type<T>::value) {
                    *((T*)((char*)buff_iter->p_volatile + pos)) = (((uint)ret(i) >= (uint)src0(i)) ? ret(i) : src0(i));
                }
                break;
            case ATOMIC_XCHG:
                ret(i) = *((T*)((char*)buff_iter->p_volatile + pos));
                *((T*)((char*)buff_iter->p_volatile + pos)) = src0(i);
                break;
            case ATOMIC_AND:
                ret(i) = *((T*)((char*)buff_iter->p_volatile + pos));
                if (std::is_same_v <T, uint>)
                {
                    *((T*)((char*)buff_iter->p_volatile + pos)) = (uint)ret(i)& (uint)src0(i);
                }
                else if (std::is_same_v <T, int>)
                {
                    *((T*)((char*)buff_iter->p_volatile + pos)) = (int)ret(i)& (int)src0(i);
                }
                else
                {
                    printf("Error writing buffer %d: unsupported opcode/type for DWord atomic write!\n", buf_id.get_data() & 0xFF);
                    exit(EXIT_FAILURE);
                }
                break;
            case ATOMIC_OR:
                ret(i) = *((T*)((char*)buff_iter->p_volatile + pos));
                if (std::is_same_v <T, uint>)
                {
                    *((T*)((char*)buff_iter->p_volatile + pos)) = (uint)ret(i) | (uint)src0(i);
                }
                else if (std::is_same_v <T, int>)
                {
                    *((T*)((char*)buff_iter->p_volatile + pos)) = (int)ret(i) | (int)src0(i);
                }
                else
                {
                    printf("Error writing buffer %d: unsupported opcode/type for DWord atomic write!\n", buf_id.get_data() & 0xFF);
                    exit(EXIT_FAILURE);
                }
                break;
            case ATOMIC_XOR:
                ret(i) = *((T*)((char*)buff_iter->p_volatile + pos));
                if (std::is_same_v <T, uint>)
                {
                    *((T*)((char*)buff_iter->p_volatile + pos)) = (uint)ret(i) ^ (uint)src0(i);
                }
                else if (std::is_same_v <T, int>)
                {
                    *((T*)((char*)buff_iter->p_volatile + pos)) = (int)ret(i) ^ (int)src0(i);
                }
                else
                {
                    printf("Error writing buffer %d: unsupported opcode/type for DWord atomic write!\n", buf_id.get_data() & 0xFF);
                    exit(EXIT_FAILURE);
                }
                break;
            case ATOMIC_MINSINT:
                ret(i) = *((T*)((char*)buff_iter->p_volatile + pos));
                *((T*)((char*)buff_iter->p_volatile + pos)) = (((int)ret(i) <= (int)src0(i)) ? ret(i) : src0(i));
                break;
            case ATOMIC_MAXSINT:
                ret(i) = *((T*)((char*)buff_iter->p_volatile + pos));
                *((T*)((char*)buff_iter->p_volatile + pos)) = (((int)ret(i) >= (int)src0(i)) ? ret(i) : src0(i));
                break;
            case ATOMIC_FADD:
                ret(i) = *((T*)((char*)buff_iter->p_volatile + pos));
                if (is_fp_type<T>::value) {
                    *((T*)((char*)buff_iter->p_volatile + pos)) = (float)ret(i) + (float)src0(i);
                }
                break;
            case ATOMIC_FSUB:
                ret(i) = *((T*)((char*)buff_iter->p_volatile + pos));
                if (is_fp_type<T>::value) {
                    *((T*)((char*)buff_iter->p_volatile + pos)) = (float)ret(i) - (float)src0(i);
                }
                break;
            case ATOMIC_FMIN:
                ret(i) = *((T*)((char*)buff_iter->p_volatile + pos));
                if (is_fp_type<T>::value) {
                    *((T*)((char*)buff_iter->p_volatile + pos)) = (((float)ret(i) <= (float)src0(i)) ? ret(i) : src0(i));
                }
                break;
            case ATOMIC_FMAX:
                ret(i) = *((T*)((char*)buff_iter->p_volatile + pos));
                if (is_fp_type<T>::value) {
                    *((T*)((char*)buff_iter->p_volatile + pos)) = (((float)ret(i) >= (float)src0(i)) ? ret(i) : src0(i));
                }
                break;
            default:
                printf("Error writing buffer %d: invalid opcode for typed atomic write!\n", buf_id.get_data() & 0xFF);
                exit(EXIT_FAILURE);
            }
        }
    }
}

template <typename T, uint N>
CM_API void __write_typed_atomic_impl(SurfaceIndex & buf_id, CmAtomicOpType op,
    vector_ref<T, N> ret,int src0,int src1,
    vector<uint, N> u, vector<uint, N> v, vector<uint, N> r, int LOD)
{
    cm_list<CmEmulSys::iobuffer>::iterator buff_iter =
        CmEmulSys::search_buffer(buf_id.get_data() & 0xFF);

    if (buff_iter == CmEmulSys::iobuffers.end()) {
        printf("Error reading buffer %d: buffer %d is not registered!\n", buf_id.get_data() & 0xFF, buf_id.get_data() & 0xFF);
        exit(EXIT_FAILURE);
    }

    if (buff_iter->bclass != GEN4_INPUT_OUTPUT_BUFFER) {
        printf("Error writing buffer %d: the buffer type must be GEN4_INPUT_OUTPUT_BUFFER for typed atomic write!\n", buf_id.get_data() & 0xFF);
        exit(EXIT_FAILURE);
    }

    if (op > ATOMIC_FMAX) {
        printf("Error writing buffer %d: invalid opcode for typed atomic write!\n", buf_id.get_data() & 0xFF);
        exit(EXIT_FAILURE);
    }
    if (N != 1 && N != 2 && N != 4 && N != 8) {
        printf("Error writing buffer %d: src length must be 1,2,4,8 for typed atomic write!\n", buf_id.get_data() & 0xFF);
        exit(EXIT_FAILURE);
    }

    CmSurfaceFormatID surfFormat;

    surfFormat = buff_iter->pixelFormat;
    int data_size = 4;

    /*if ((surfFormat == R32_SINT) || (surfFormat == R32_UINT)) {
    data_size = 4;
    }
    else {
    fprintf(stderr, "typed atomic write error: only R32_SINT/R32_UINT surface formats are supported.\n");
    exit(EXIT_FAILURE);
    }*/

    int width = buff_iter->width;
    int height = buff_iter->height;
    int depth = buff_iter->depth;
    int pos;

    for (uint i = 0; i < N; i++) {
        pos = data_size * u(i) + v(i) * width + r(i) * height;
        if ((data_size * u(i)) >= width || v(i) >= height || r(i) >=depth) {
            continue;
        }
        else {
            switch (op) {
            case ATOMIC_INC:
                ret(i) = *((T*)((char*)buff_iter->p_volatile + pos));
                *((T*)((char*)buff_iter->p_volatile + pos)) = ret(i) + 1;
                break;
            case ATOMIC_DEC:
                ret(i) = *((T*)((char*)buff_iter->p_volatile + pos));
                *((T*)((char*)buff_iter->p_volatile + pos)) = ret(i) - 1;
                break;
            default:
                printf("Error writing buffer %d: invalid opcode for typed atomic write!\n", buf_id.get_data() & 0xFF);
                exit(EXIT_FAILURE);
            }
        }
    }
}

template <typename T, uint N>
CM_API void __write_typed_atomic_impl(SurfaceIndex & buf_id, CmAtomicOpType op,
    vector_ref<T, N> ret, vector<T, N> src0, vector<T, N> src1,
    vector<uint, N> u, vector<uint, N> v, int r, int LOD)
{
    cm_list<CmEmulSys::iobuffer>::iterator buff_iter =
        CmEmulSys::search_buffer(buf_id.get_data() & 0xFF);

    if (buff_iter == CmEmulSys::iobuffers.end()) {
        printf("Error reading buffer %d: buffer %d is not registered!\n", buf_id.get_data() & 0xFF, buf_id.get_data() & 0xFF);
        exit(EXIT_FAILURE);
    }

    if (buff_iter->bclass != GEN4_INPUT_OUTPUT_BUFFER) {
        printf("Error writing buffer %d: the buffer type must be GEN4_INPUT_OUTPUT_BUFFER for typed atomic write!\n", buf_id.get_data() & 0xFF);
        exit(EXIT_FAILURE);
    }

    if (op > ATOMIC_FMAX) {
        printf("Error writing buffer %d: invalid opcode for typed atomic write!\n", buf_id.get_data() & 0xFF);
        exit(EXIT_FAILURE);
    }
    if (N != 1 && N != 2 && N != 4 && N != 8) {
        printf("Error writing buffer %d: src length must be 1,2,4,8 for typed atomic write!\n", buf_id.get_data() & 0xFF);
        exit(EXIT_FAILURE);
    }

    CmSurfaceFormatID surfFormat;

    surfFormat = buff_iter->pixelFormat;
    int data_size = 4;

    /*if ((surfFormat == R32_SINT) || (surfFormat == R32_UINT)) {
    data_size = 4;
    }
    else {
    fprintf(stderr, "typed atomic write error: only R32_SINT/R32_UINT surface formats are supported.\n");
    exit(EXIT_FAILURE);
    }*/

    int width = buff_iter->width;
    int height = buff_iter->height;
    int depth = buff_iter->depth;
    int pos;

    for (uint i = 0; i < N; i++) {
        pos = data_size * u(i) + v(i) * width;
        if ((data_size * u(i)) >= width || v(i) >= height) {
            continue;
        }
        else {
            switch (op) {
            case ATOMIC_CMPXCHG:
                ret(i) = *((T*)((char*)buff_iter->p_volatile + pos));
                *((T*)((char*)buff_iter->p_volatile + pos)) = ((ret(i) == src1(i)) ? src0(i) : ret(i));
                break;
            default:
                printf("Error writing buffer %d: invalid opcode for typed atomic write!\n", buf_id.get_data() & 0xFF);
                exit(EXIT_FAILURE);
            }
        }
    }

}

template <typename T, uint N>
CM_API void __write_typed_atomic_impl(SurfaceIndex & buf_id, CmAtomicOpType op,
    vector_ref<T, N> ret, vector<T, N> src0,int src1,
    vector<uint, N> u, vector<uint, N> v, int r, int LOD)
{
    cm_list<CmEmulSys::iobuffer>::iterator buff_iter =
        CmEmulSys::search_buffer(buf_id.get_data() & 0xFF);

    if (buff_iter == CmEmulSys::iobuffers.end()) {
        printf("Error reading buffer %d: buffer %d is not registered!\n", buf_id.get_data() & 0xFF, buf_id.get_data() & 0xFF);
        exit(EXIT_FAILURE);
    }

    if (buff_iter->bclass != GEN4_INPUT_OUTPUT_BUFFER) {
        printf("Error writing buffer %d: the buffer type must be GEN4_INPUT_OUTPUT_BUFFER for typed atomic write!\n", buf_id.get_data() & 0xFF);
        exit(EXIT_FAILURE);
    }

    if (op > ATOMIC_FMAX) {
        printf("Error writing buffer %d: invalid opcode for typed atomic write!\n", buf_id.get_data() & 0xFF);
        exit(EXIT_FAILURE);
    }
    if (N != 1 && N != 2 && N != 4 && N != 8) {
        printf("Error writing buffer %d: src length must be 1,2,4,8 for typed atomic write!\n", buf_id.get_data() & 0xFF);
        exit(EXIT_FAILURE);
    }

    CmSurfaceFormatID surfFormat;

    surfFormat = buff_iter->pixelFormat;
    int data_size = 4;

    /*if ((surfFormat == R32_SINT) || (surfFormat == R32_UINT)) {
    data_size = 4;
    }
    else {
    fprintf(stderr, "typed atomic write error: only R32_SINT/R32_UINT surface formats are supported.\n");
    exit(EXIT_FAILURE);
    }*/

    int width = buff_iter->width;
    int height = buff_iter->height;
    int depth = buff_iter->depth;
    int pos;

    for (uint i = 0; i < N; i++) {
        pos = data_size * u(i) + v(i) * width;
        if ((data_size * u(i)) >= width || v(i) >= height) {
            continue;
        }
        else {
            switch (op) {
            case ATOMIC_ADD:
               ret(i)= *((T*)((char*)buff_iter->p_volatile + pos));
                *((T*)((char*)buff_iter->p_volatile + pos)) =ret(i)+ src0(i);
                break;
            case ATOMIC_SUB:
               ret(i)= *((T*)((char*)buff_iter->p_volatile + pos));
                *((T*)((char*)buff_iter->p_volatile + pos)) =ret(i)- src0(i);
                break;
            case ATOMIC_MIN:
               ret(i)= *((T*)((char*)buff_iter->p_volatile + pos));
                if (!is_fp_type<T>::value) {
                    *((T*)((char*)buff_iter->p_volatile + pos)) = (((uint)ret(i) <= (uint)src0(i)) ?ret(i): src0(i));
                }
                break;
            case ATOMIC_MAX:
               ret(i)= *((T*)((char*)buff_iter->p_volatile + pos));
                if (!is_fp_type<T>::value) {
                    *((T*)((char*)buff_iter->p_volatile + pos)) = (((uint)ret(i) >= (uint)src0(i)) ?ret(i): src0(i));
                }
                break;
            case ATOMIC_XCHG:
               ret(i)= *((T*)((char*)buff_iter->p_volatile + pos));
                *((T*)((char*)buff_iter->p_volatile + pos)) = src0(i);
                break;
            case ATOMIC_AND:
                ret(i) = *((T*)((char*)buff_iter->p_volatile + pos));
                if (std::is_same_v <T, uint>)
                {
                    *((T*)((char*)buff_iter->p_volatile + pos)) = (uint)ret(i)& (uint)src0(i);
                }
                else if (std::is_same_v <T, int>)
                {
                    *((T*)((char*)buff_iter->p_volatile + pos)) = (int)ret(i)& (int)src0(i);
                }
                else
                {
                    printf("Error writing buffer %d: unsupported opcode/type for DWord atomic write!\n", buf_id.get_data() & 0xFF);
                    exit(EXIT_FAILURE);
                }
                break;
            case ATOMIC_OR:
                ret(i) = *((T*)((char*)buff_iter->p_volatile + pos));
                if (std::is_same_v <T, uint>)
                {
                    *((T*)((char*)buff_iter->p_volatile + pos)) = (uint)ret(i) | (uint)src0(i);
                }
                else if (std::is_same_v <T, int>)
                {
                    *((T*)((char*)buff_iter->p_volatile + pos)) = (int)ret(i) | (int)src0(i);
                }
                else
                {
                    printf("Error writing buffer %d: unsupported opcode/type for DWord atomic write!\n", buf_id.get_data() & 0xFF);
                    exit(EXIT_FAILURE);
                }
                break;
            case ATOMIC_XOR:
                ret(i) = *((T*)((char*)buff_iter->p_volatile + pos));
                if (std::is_same_v <T, uint>)
                {
                    *((T*)((char*)buff_iter->p_volatile + pos)) = (uint)ret(i) ^ (uint)src0(i);
                }
                else if (std::is_same_v <T, int>)
                {
                    *((T*)((char*)buff_iter->p_volatile + pos)) = (int)ret(i) ^ (int)src0(i);
                }
                else
                {
                    printf("Error writing buffer %d: unsupported opcode/type for DWord atomic write!\n", buf_id.get_data() & 0xFF);
                    exit(EXIT_FAILURE);
                }
                break;
            case ATOMIC_MINSINT:
               ret(i)= *((T*)((char*)buff_iter->p_volatile + pos));
                *((T*)((char*)buff_iter->p_volatile + pos)) = (((int)ret(i) <= (int)src0(i)) ?ret(i): src0(i));
                break;
            case ATOMIC_MAXSINT:
               ret(i)= *((T*)((char*)buff_iter->p_volatile + pos));
                *((T*)((char*)buff_iter->p_volatile + pos)) = (((int)ret(i) >= (int)src0(i)) ?ret(i): src0(i));
                break;
            case ATOMIC_FADD:
                ret(i) = *((T*)((char*)buff_iter->p_volatile + pos));
                if (is_fp_type<T>::value) {
                    *((T*)((char*)buff_iter->p_volatile + pos)) = (float)ret(i) + (float)src0(i);
                }
                break;
            case ATOMIC_FSUB:
                ret(i) = *((T*)((char*)buff_iter->p_volatile + pos));
                if (is_fp_type<T>::value) {
                    *((T*)((char*)buff_iter->p_volatile + pos)) = (float)ret(i) - (float)src0(i);
                }
                break;
            case ATOMIC_FMIN:
                ret(i) = *((T*)((char*)buff_iter->p_volatile + pos));
                if (is_fp_type<T>::value) {
                    *((T*)((char*)buff_iter->p_volatile + pos)) = (((float)ret(i) <= (float)src0(i)) ? ret(i) : src0(i));
                }
                break;
            case ATOMIC_FMAX:
                ret(i) = *((T*)((char*)buff_iter->p_volatile + pos));
                if (is_fp_type<T>::value) {
                    *((T*)((char*)buff_iter->p_volatile + pos)) = (((float)ret(i) >= (float)src0(i)) ? ret(i) : src0(i));
                }
                break;
            default:
                printf("Error writing buffer %d: invalid opcode for typed atomic write!\n", buf_id.get_data() & 0xFF);
                exit(EXIT_FAILURE);
            }
        }
    }

}

template <typename T, uint N>
CM_API void __write_typed_atomic_impl(SurfaceIndex & buf_id, CmAtomicOpType op,
    vector_ref<T, N> ret,int src0, int src1,
    vector<uint, N> u, vector<uint, N> v, int r, int LOD)
{
    cm_list<CmEmulSys::iobuffer>::iterator buff_iter =
        CmEmulSys::search_buffer(buf_id.get_data() & 0xFF);

    if (buff_iter == CmEmulSys::iobuffers.end()) {
        printf("Error reading buffer %d: buffer %d is not registered!\n", buf_id.get_data() & 0xFF, buf_id.get_data() & 0xFF);
        exit(EXIT_FAILURE);
    }

    if (buff_iter->bclass != GEN4_INPUT_OUTPUT_BUFFER) {
        printf("Error writing buffer %d: the buffer type must be GEN4_INPUT_OUTPUT_BUFFER for typed atomic write!\n", buf_id.get_data() & 0xFF);
        exit(EXIT_FAILURE);
    }

    if (op > ATOMIC_FMAX) {
        printf("Error writing buffer %d: invalid opcode for typed atomic write!\n", buf_id.get_data() & 0xFF);
        exit(EXIT_FAILURE);
    }
    if (N != 1 && N != 2 && N != 4 && N != 8) {
        printf("Error writing buffer %d: src length must be 1,2,4,8 for typed atomic write!\n", buf_id.get_data() & 0xFF);
        exit(EXIT_FAILURE);
    }

    CmSurfaceFormatID surfFormat;

    surfFormat = buff_iter->pixelFormat;
    int data_size = 4;

    /*if ((surfFormat == R32_SINT) || (surfFormat == R32_UINT)) {
    data_size = 4;
    }
    else {
    fprintf(stderr, "typed atomic write error: only R32_SINT/R32_UINT surface formats are supported.\n");
    exit(EXIT_FAILURE);
    }*/

    int width = buff_iter->width;
    int height = buff_iter->height;
    int depth = buff_iter->depth;
    int pos;

    for (uint i = 0; i < N; i++) {
        pos = data_size * u(i) + v(i) * width;
        if ((data_size * u(i)) >= width || v(i) >= height) {
            continue;
        }
        else {
            switch (op) {
            case ATOMIC_INC:
                ret(i) = *((T*)((char*)buff_iter->p_volatile + pos));
                *((T*)((char*)buff_iter->p_volatile + pos)) = ret(i) + 1;
                break;
            case ATOMIC_DEC:
                ret(i) = *((T*)((char*)buff_iter->p_volatile + pos));
                *((T*)((char*)buff_iter->p_volatile + pos)) = ret(i) - 1;
                break;
            default:
                printf("Error writing buffer %d: invalid opcode for typed atomic write!\n", buf_id.get_data() & 0xFF);
                exit(EXIT_FAILURE);
            }
        }
    }

}

template <typename T, uint N>
CM_API void __write_typed_atomic_impl(SurfaceIndex & buf_id, CmAtomicOpType op,
    vector_ref<T, N> ret, vector<T, N> src0, vector<T, N> src1,
    vector<uint, N> u, int v, int r, int LOD)
{
    cm_list<CmEmulSys::iobuffer>::iterator buff_iter =
        CmEmulSys::search_buffer(buf_id.get_data() & 0xFF);

    if (buff_iter == CmEmulSys::iobuffers.end()) {
        printf("Error reading buffer %d: buffer %d is not registered!\n", buf_id.get_data() & 0xFF, buf_id.get_data() & 0xFF);
        exit(EXIT_FAILURE);
    }

    if (buff_iter->bclass != GEN4_INPUT_OUTPUT_BUFFER) {
        printf("Error writing buffer %d: the buffer type must be GEN4_INPUT_OUTPUT_BUFFER for typed atomic write!\n", buf_id.get_data() & 0xFF);
        exit(EXIT_FAILURE);
    }

    if (op > ATOMIC_FMAX) {
        printf("Error writing buffer %d: invalid opcode for typed atomic write!\n", buf_id.get_data() & 0xFF);
        exit(EXIT_FAILURE);
    }
    if (N != 1 && N != 2 && N != 4 && N != 8) {
        printf("Error writing buffer %d: src length must be 1,2,4,8 for typed atomic write!\n", buf_id.get_data() & 0xFF);
        exit(EXIT_FAILURE);
    }

    CmSurfaceFormatID surfFormat;

    surfFormat = buff_iter->pixelFormat;
    int data_size = 4;

    /*if ((surfFormat == R32_SINT) || (surfFormat == R32_UINT)) {
    data_size = 4;
    }
    else {
    fprintf(stderr, "typed atomic write error: only R32_SINT/R32_UINT surface formats are supported.\n");
    exit(EXIT_FAILURE);
    }*/

    int width = buff_iter->width;
    int height = buff_iter->height;
    int depth = buff_iter->depth;
    int pos;

    for (uint i = 0; i < N; i++) {
        pos = data_size * u(i);
        if ((data_size * u(i)) >= width) {
            continue;
        }
        else {
            switch (op) {
            case ATOMIC_CMPXCHG:
                ret(i) = *((T*)((char*)buff_iter->p_volatile + pos));
                *((T*)((char*)buff_iter->p_volatile + pos)) = ((ret(i) == src1(i)) ? src0(i) : ret(i));
                break;
            default:
                printf("Error writing buffer %d: invalid opcode for typed atomic write!\n", buf_id.get_data() & 0xFF);
                exit(EXIT_FAILURE);
            }
        }
    }
}

template <typename T, uint N>
CM_API void __write_typed_atomic_impl(SurfaceIndex & buf_id, CmAtomicOpType op,
    vector_ref<T, N> ret, vector<T, N> src0,int src1,
    vector<uint, N> u, int v, int r, int LOD)
{
    cm_list<CmEmulSys::iobuffer>::iterator buff_iter =
        CmEmulSys::search_buffer(buf_id.get_data() & 0xFF);

    if (buff_iter == CmEmulSys::iobuffers.end()) {
        printf("Error reading buffer %d: buffer %d is not registered!\n", buf_id.get_data() & 0xFF, buf_id.get_data() & 0xFF);
        exit(EXIT_FAILURE);
    }

    if (buff_iter->bclass != GEN4_INPUT_OUTPUT_BUFFER) {
        printf("Error writing buffer %d: the buffer type must be GEN4_INPUT_OUTPUT_BUFFER for typed atomic write!\n", buf_id.get_data() & 0xFF);
        exit(EXIT_FAILURE);
    }

    if (op > ATOMIC_FMAX) {
        printf("Error writing buffer %d: invalid opcode for typed atomic write!\n", buf_id.get_data() & 0xFF);
        exit(EXIT_FAILURE);
    }
    if (N != 1 && N != 2 && N != 4 && N != 8) {
        printf("Error writing buffer %d: src length must be 1,2,4,8 for typed atomic write!\n", buf_id.get_data() & 0xFF);
        exit(EXIT_FAILURE);
    }

    CmSurfaceFormatID surfFormat;

    surfFormat = buff_iter->pixelFormat;
    int data_size = 4;

    /*if ((surfFormat == R32_SINT) || (surfFormat == R32_UINT)) {
    data_size = 4;
    }
    else {
    fprintf(stderr, "typed atomic write error: only R32_SINT/R32_UINT surface formats are supported.\n");
    exit(EXIT_FAILURE);
    }*/

    int width = buff_iter->width;
    int height = buff_iter->height;
    int depth = buff_iter->depth;
    int pos;

    for (uint i = 0; i < N; i++) {
        pos = data_size * u(i);
        if ((data_size * u(i)) >= width) {
            continue;
        }
        else {
            switch (op) {
            case ATOMIC_ADD:
                ret(i) = *((T*)((char*)buff_iter->p_volatile + pos));
                *((T*)((char*)buff_iter->p_volatile + pos)) = ret(i) + src0(i);
                break;
            case ATOMIC_SUB:
                ret(i) = *((T*)((char*)buff_iter->p_volatile + pos));
                *((T*)((char*)buff_iter->p_volatile + pos)) = ret(i) - src0(i);
                break;
            case ATOMIC_MIN:
                ret(i) = *((T*)((char*)buff_iter->p_volatile + pos));
                if (!is_fp_type<T>::value) {
                    *((T*)((char*)buff_iter->p_volatile + pos)) = (((uint)ret(i) <= (uint)src0(i)) ? ret(i) : src0(i));
                }
                break;
            case ATOMIC_MAX:
                ret(i) = *((T*)((char*)buff_iter->p_volatile + pos));
                if (!is_fp_type<T>::value) {
                    *((T*)((char*)buff_iter->p_volatile + pos)) = (((uint)ret(i) >= (uint)src0(i)) ? ret(i) : src0(i));
                }
                break;
            case ATOMIC_XCHG:
                ret(i) = *((T*)((char*)buff_iter->p_volatile + pos));
                *((T*)((char*)buff_iter->p_volatile + pos)) = src0(i);
                break;
            case ATOMIC_AND:
                ret(i) = *((T*)((char*)buff_iter->p_volatile + pos));
                if (std::is_same_v <T, uint>)
                {
                    *((T*)((char*)buff_iter->p_volatile + pos)) = (uint)ret(i)& (uint)src0(i);
                }
                else if (std::is_same_v <T, int>)
                {
                    *((T*)((char*)buff_iter->p_volatile + pos)) = (int)ret(i)& (int)src0(i);
                }
                else
                {
                    printf("Error writing buffer %d: unsupported opcode/type for DWord atomic write!\n", buf_id.get_data() & 0xFF);
                    exit(EXIT_FAILURE);
                }
                break;
            case ATOMIC_OR:
                ret(i) = *((T*)((char*)buff_iter->p_volatile + pos));
                if (std::is_same_v <T, uint>)
                {
                    *((T*)((char*)buff_iter->p_volatile + pos)) = (uint)ret(i) | (uint)src0(i);
                }
                else if (std::is_same_v <T, int>)
                {
                    *((T*)((char*)buff_iter->p_volatile + pos)) = (int)ret(i) | (int)src0(i);
                }
                else
                {
                    printf("Error writing buffer %d: unsupported opcode/type for DWord atomic write!\n", buf_id.get_data() & 0xFF);
                    exit(EXIT_FAILURE);
                }
                break;
            case ATOMIC_XOR:
                ret(i) = *((T*)((char*)buff_iter->p_volatile + pos));
                if (std::is_same_v <T, uint>)
                {
                    *((T*)((char*)buff_iter->p_volatile + pos)) = (uint)ret(i) ^ (uint)src0(i);
                }
                else if (std::is_same_v <T, int>)
                {
                    *((T*)((char*)buff_iter->p_volatile + pos)) = (int)ret(i) ^ (int)src0(i);
                }
                else
                {
                    printf("Error writing buffer %d: unsupported opcode/type for DWord atomic write!\n", buf_id.get_data() & 0xFF);
                    exit(EXIT_FAILURE);
                }
                break;
            case ATOMIC_MINSINT:
                ret(i) = *((T*)((char*)buff_iter->p_volatile + pos));
                *((T*)((char*)buff_iter->p_volatile + pos)) = (((int)ret(i) <= (int)src0(i)) ? ret(i) : src0(i));
                break;
            case ATOMIC_MAXSINT:
                ret(i) = *((T*)((char*)buff_iter->p_volatile + pos));
                *((T*)((char*)buff_iter->p_volatile + pos)) = (((int)ret(i) >= (int)src0(i)) ? ret(i) : src0(i));
                break;
            case ATOMIC_FADD:
                ret(i) = *((T*)((char*)buff_iter->p_volatile + pos));
                if (is_fp_type<T>::value) {
                    *((T*)((char*)buff_iter->p_volatile + pos)) = (float)ret(i) + (float)src0(i);
                }
                break;
            case ATOMIC_FSUB:
                ret(i) = *((T*)((char*)buff_iter->p_volatile + pos));
                if (is_fp_type<T>::value) {
                    *((T*)((char*)buff_iter->p_volatile + pos)) = (float)ret(i) - (float)src0(i);
                }
                break;
            case ATOMIC_FMIN:
                ret(i) = *((T*)((char*)buff_iter->p_volatile + pos));
                if (is_fp_type<T>::value) {
                    *((T*)((char*)buff_iter->p_volatile + pos)) = (((float)ret(i) <= (float)src0(i)) ? ret(i) : src0(i));
                }
                break;
            case ATOMIC_FMAX:
                ret(i) = *((T*)((char*)buff_iter->p_volatile + pos));
                if (is_fp_type<T>::value) {
                    *((T*)((char*)buff_iter->p_volatile + pos)) = (((float)ret(i) >= (float)src0(i)) ? ret(i) : src0(i));
                }
                break;
            default:
                printf("Error writing buffer %d: invalid opcode for typed atomic write!\n", buf_id.get_data() & 0xFF);
                exit(EXIT_FAILURE);
            }
        }
    }
}

template <typename T, uint N>
CM_API void __write_typed_atomic_impl(SurfaceIndex & buf_id, CmAtomicOpType op,
    vector_ref<T, N> ret,int src0, int src1,
    vector<uint, N> u, int v, int r, int LOD)
{
    cm_list<CmEmulSys::iobuffer>::iterator buff_iter =
        CmEmulSys::search_buffer(buf_id.get_data() & 0xFF);

    if (buff_iter == CmEmulSys::iobuffers.end()) {
        printf("Error reading buffer %d: buffer %d is not registered!\n", buf_id.get_data() & 0xFF, buf_id.get_data() & 0xFF);
        exit(EXIT_FAILURE);
    }

    if (buff_iter->bclass != GEN4_INPUT_OUTPUT_BUFFER) {
        printf("Error writing buffer %d: the buffer type must be GEN4_INPUT_OUTPUT_BUFFER for typed atomic write!\n", buf_id.get_data() & 0xFF);
        exit(EXIT_FAILURE);
    }

    if (op > ATOMIC_FMAX) {
        printf("Error writing buffer %d: invalid opcode for typed atomic write!\n", buf_id.get_data() & 0xFF);
        exit(EXIT_FAILURE);
    }
    if (N != 1 && N != 2 && N != 4 && N != 8) {
        printf("Error writing buffer %d: src length must be 1,2,4,8 for typed atomic write!\n", buf_id.get_data() & 0xFF);
        exit(EXIT_FAILURE);
    }

    CmSurfaceFormatID surfFormat;

    surfFormat = buff_iter->pixelFormat;
    int data_size = 4;

    /*if ((surfFormat == R32_SINT) || (surfFormat == R32_UINT)) {
    data_size = 4;
    }
    else {
    fprintf(stderr, "typed atomic write error: only R32_SINT/R32_UINT surface formats are supported.\n");
    exit(EXIT_FAILURE);
    }*/

    int width = buff_iter->width;
    int height = buff_iter->height;
    int depth = buff_iter->depth;
    int pos;

    for (uint i = 0; i < N; i++) {
        pos = data_size * u(i);
        if ((data_size * u(i)) >= width) {
            continue;
        }
        else {
            switch (op) {
            case ATOMIC_INC:
                ret(i) = *((T*)((char*)buff_iter->p_volatile + pos));
                *((T*)((char*)buff_iter->p_volatile + pos)) = ret(i) + 1;
                break;
            case ATOMIC_DEC:
                ret(i) = *((T*)((char*)buff_iter->p_volatile + pos));
                *((T*)((char*)buff_iter->p_volatile + pos)) = ret(i) - 1;
                break;
            default:
                printf("Error writing buffer %d: invalid opcode for typed atomic write!\n", buf_id.get_data() & 0xFF);
                exit(EXIT_FAILURE);
            }
        }
    }
}

template<CmAtomicOpType Op, typename T, uint N>
_GENX_ inline typename hidden::cm_enable_if<(Op == ATOMIC_INC || Op == ATOMIC_DEC), void>::type
write_typed_atomic(SurfaceIndex surfIndex, vector_ref<T, N> ret, vector<uint, N> u) {
    __write_typed_atomic_impl<T>(surfIndex, Op, ret, NULL,NULL,u,NULL,NULL,NULL);
}

template<CmAtomicOpType Op, typename T, uint N>
_GENX_ inline typename hidden::cm_enable_if<(Op == ATOMIC_INC || Op == ATOMIC_DEC), void>::type
write_typed_atomic(SurfaceIndex surfIndex, vector_ref<T, N> ret, vector<uint, N> u, vector<uint, N> v) {
    __write_typed_atomic_impl<T>(surfIndex, Op, ret,NULL,NULL, u, v,NULL,NULL);
}

template<CmAtomicOpType Op, typename T, uint N>
_GENX_ inline typename hidden::cm_enable_if<(Op == ATOMIC_INC || Op == ATOMIC_DEC), void>::type
write_typed_atomic(SurfaceIndex surfIndex, vector_ref<T, N> ret, vector<uint, N> u, vector<uint, N> v,
    vector<uint, N> r) {
    __write_typed_atomic_impl<T>(surfIndex, Op, ret,NULL,NULL, u, v, r,NULL);
}

template<CmAtomicOpType Op, typename T, uint N>
_GENX_ inline typename hidden::cm_enable_if<(Op == ATOMIC_INC || Op == ATOMIC_DEC), void>::type
write_typed_atomic(SurfaceIndex surfIndex, vector_ref<T, N> ret, vector<uint, N> u, vector<uint, N> v,
    vector<uint, N> r, vector<uint, N> LOD) {
    __write_typed_atomic_impl<T>(surfIndex, Op, ret,NULL,NULL, u, v, r, LOD);
}

template<CmAtomicOpType Op, typename T, uint N>
_GENX_ inline typename hidden::cm_enable_if<(Op != ATOMIC_CMPXCHG && Op != ATOMIC_FCMPWR && Op != ATOMIC_INC && Op != ATOMIC_DEC), void>::type
write_typed_atomic(SurfaceIndex surfIndex, vector_ref<T, N> ret, vector<T, N> src0,
    vector<uint, N> u) {
    __write_typed_atomic_impl<T>(surfIndex, Op, ret, src0 ,NULL, u,NULL,NULL,NULL);
}

template<CmAtomicOpType Op, typename T, uint N>
_GENX_ inline typename hidden::cm_enable_if<(Op != ATOMIC_CMPXCHG && Op != ATOMIC_FCMPWR && Op != ATOMIC_INC && Op != ATOMIC_DEC), void>::type
write_typed_atomic(SurfaceIndex surfIndex, vector_ref<T, N> ret, vector<T, N> src0,
    vector<uint, N> u, vector<uint, N> v) {
    __write_typed_atomic_impl<T>(surfIndex, Op, ret, src0 , NULL,u, v,NULL,NULL);
}

template<CmAtomicOpType Op, typename T, uint N>
_GENX_ inline typename hidden::cm_enable_if<(Op != ATOMIC_CMPXCHG && Op != ATOMIC_INC && Op != ATOMIC_DEC), void>::type
write_typed_atomic(SurfaceIndex surfIndex, vector_ref<T, N> ret, vector<T, N> src0,
    vector<uint, N> u, vector<uint, N> v, vector<uint, N> r) {
    __write_typed_atomic_impl<T>(surfIndex, Op, ret, src0 ,NULL, u, v, r,NULL);
}

template<CmAtomicOpType Op, typename T, uint N>
_GENX_ inline typename hidden::cm_enable_if<(Op != ATOMIC_CMPXCHG && Op != ATOMIC_FCMPWR && Op == ATOMIC_FCMPWR && Op != ATOMIC_INC && Op != ATOMIC_DEC), void>::type
write_typed_atomic(SurfaceIndex surfIndex, vector_ref<T, N> ret, vector<T, N> src0,
    vector<uint, N> u, vector<uint, N> v, vector<uint, N> r, vector<uint, N> LOD) {
    __write_typed_atomic_impl<T>(surfIndex, Op, ret, src0 ,NULL, u, v, r, LOD);
}

template<CmAtomicOpType Op, typename T, uint N>
_GENX_ inline typename hidden::cm_enable_if<(Op == ATOMIC_CMPXCHG || Op == ATOMIC_FCMPWR), void>::type
write_typed_atomic(SurfaceIndex surfIndex, vector_ref<T, N> ret, vector<T, N> src0, vector<T, N> src1,
    vector<uint, N> u) {
    __write_typed_atomic_impl<T>(surfIndex, Op, ret, src0, src1, u,NULL,NULL,NULL);
}

template<CmAtomicOpType Op, typename T, uint N>
_GENX_ inline typename hidden::cm_enable_if<(Op == ATOMIC_CMPXCHG || Op == ATOMIC_FCMPWR), void>::type
write_typed_atomic(SurfaceIndex surfIndex, vector_ref<T, N> ret, vector<T, N> src0, vector<T, N> src1,
    vector<uint, N> u, vector<uint, N> v) {
    __write_typed_atomic_impl<T>(surfIndex, Op, ret, src0, src1, u, v,NULL,NULL);
}

template<CmAtomicOpType Op, typename T, uint N>
_GENX_ inline typename hidden::cm_enable_if<(Op == ATOMIC_CMPXCHG || Op == ATOMIC_FCMPWR), void>::type
write_typed_atomic(SurfaceIndex surfIndex, vector_ref<T, N> ret, vector<T, N> src0, vector<T, N> src1,
    vector<uint, N> u, vector<uint, N> v, vector<uint, N> r) {
    __write_typed_atomic_impl<T>(surfIndex, Op, ret, src0, src1, u, v, r, NULL);
}

template<CmAtomicOpType Op, typename T, uint N>
_GENX_ inline typename hidden::cm_enable_if<(Op == ATOMIC_CMPXCHG || Op == ATOMIC_FCMPWR), void>::type
write_typed_atomic(SurfaceIndex surfIndex, vector_ref<T, N> ret, vector<T, N> src0, vector<T, N> src1,
    vector<uint, N> u, vector<uint, N> v, vector<uint, N> r, vector<uint, N> LOD) {
    __write_typed_atomic_impl<T>(surfIndex, Op, ret, src0, src1, u, v, r, LOD);
}

// write_typed_atomic done

#endif /* GENX_DATAPORT_H */

