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

#ifndef _ESIMDCPU_DATAPORT_H_
#define _ESIMDCPU_DATAPORT_H_

#define ESIMD_API

#include <mutex>
#include <vector>

#include "cm_common_macros.h"

#include "cm_list.h"
#include "dataport_common.h"

static std::mutex mutexForWrite;

namespace cm_support {
#include "cm_common_macros.h"

#include "cm_index_types.h"

template<typename T>
ESIMD_API
bool vector_read(SurfaceIndex & idx,
                          int offset,
                          std::vector<T> & in)
{
  int i;
  uint pos;
  int N = in.size();
  cm_list<::CmEmulSys::iobuffer>::iterator buff_iter =
      ::CmEmulSys::search_buffer(idx.get_data()&0xFF);

  if(buff_iter == ::CmEmulSys::iobuffers.end()) {
    printf("Error reading buffer %d: buffer %d is not registered!\n",
           idx.get_data()&0xFF, idx.get_data()&0xFF);
    exit(EXIT_FAILURE);
  }

  std::unique_lock<std::mutex> lock(mutexForWrite);

  assert(buff_iter->height == 1);

  int width = buff_iter->width;
  char * buff = (char*) buff_iter->p_volatile;
  int sizeofT = sizeof(T); /* Make this into a signed integer */

  for (i = 0; i < N; i++) {
    pos = offset + i * sizeofT;
    if (pos >= width)
    {
      in[i] = 0;
    }
    else
    {
      in[i] = *((T*)(buff + pos));
    }
  }
  return true;
}

template<typename T>
ESIMD_API
bool vector_write(SurfaceIndex & idx,
                           int offset,
                           const std::vector<T> & out)
{
  int i;
  uint pos;
  int N = out.size();
  cm_list<::CmEmulSys::iobuffer>::iterator buff_iter =
      ::CmEmulSys::search_buffer(idx.get_data()&0xFF);

  if(buff_iter == ::CmEmulSys::iobuffers.end()) {
    printf("Error reading buffer %d: buffer %d is not registered!\n",
           idx.get_data()&0xFF, idx.get_data()&0xFF);
    exit(EXIT_FAILURE);
  }

  std::unique_lock<std::mutex> lock(mutexForWrite);

  assert(buff_iter->height == 1);

  int width = buff_iter->width;
  char * buff = (char*) buff_iter->p_volatile;
  int sizeofT = sizeof(T); /* Make this into a signed integer */
  for (i = 0; i < N; i++) {
    pos = offset + i * sizeofT;
    if (pos >= width)
    {
      printf("Warning writing buffer %d: there is unexpected out-of-bound access for Oword block write!\n",
             idx.get_data() & 0xFF);
      break;
    }
    else
    {

      *((T*)( (char*)buff_iter->p_volatile + pos )) = out[i];
    }
  }
  return true;
}

template<typename T>
ESIMD_API
bool matrix_read(SurfaceIndex & idx,
                          int x_pos,
                          int y_pos,
                          std::vector<std::vector<T>> & in)
{
  int R = in.size();
  int C = in[0].size();
  cm_list<::CmEmulSys::iobuffer>::iterator buff_iter =
      ::CmEmulSys::search_buffer(idx.get_data()&0xFF);

  if(buff_iter == ::CmEmulSys::iobuffers.end()) {
    printf("Error reading buffer %d: buffer %d is not registered!\n",
           idx.get_data()&0xFF, idx.get_data()&0xFF);
    exit(EXIT_FAILURE);
  }
  std::unique_lock<std::mutex> lock(mutexForWrite);

  int width = buff_iter->width;
  int height = buff_iter->height;

  if((buff_iter->bclass != GEN4_INPUT_BUFFER) &&
     (buff_iter->bclass != GEN4_INPUT_OUTPUT_BUFFER)) {
    printf("Error reading buffer %d: the registered buffer type is not INPUT_BUFFER!\n", idx.get_data()&0xFF);
    exit(EXIT_FAILURE);
  }
  int x_pos_a, y_pos_a;  /* Actual positions */
  int sizeofT = sizeof(T); /* Make this into a signed integer */
  uint bpp = CM_genx_bytes_per_pixel(buff_iter->pixelFormat);
  assert(((bpp - 1) & bpp) == 0); // Is power-of-2 number

  int offset;

  for (int i = 0; i < R; i++) {
    for (int j = 0; j < C; j++) {
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
      if((buff_iter->pixelFormat == ::YCRCB_NORMAL ||
          buff_iter->pixelFormat == ::YCRCB_SWAPY) &&
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
          printf("Invalid matrix width [%d]for Packed format!\n", idx.get_data()&0xFF);
          exit(EXIT_FAILURE);
        }
        offset = y_pos_a * width;
        if(buff_iter->pixelFormat == ::YCRCB_NORMAL)
        {
          /*
          ((unsigned char*)in.get_addr(i*C+j))[0] = *((unsigned char*)((char*)buff_iter->p + offset + 0));
          ((unsigned char*)in.get_addr(i*C+j))[1] = *((unsigned char*)((char*)buff_iter->p + offset + 1));
          ((unsigned char*)in.get_addr(i*C+j))[2] = *((unsigned char*)((char*)buff_iter->p + offset + 0));
          ((unsigned char*)in.get_addr(i*C+j))[3] = *((unsigned char*)((char*)buff_iter->p + offset + 3));
          */
          char *pTempBase = (char*)buff_iter->p + offset;

          unsigned char tmp00 = *((unsigned char*)(pTempBase + 0));
          unsigned char tmp01 = *((unsigned char*)(pTempBase + 1));
          unsigned char tmp02 = *((unsigned char*)(pTempBase + 0));
          unsigned char tmp03 = *((unsigned char*)(pTempBase + 3));

          in[i][j] = (((T)tmp00)<<24) | (((T)tmp01) << 16) | (((T)tmp02)<< 8) | ((T)tmp03);
        }else if(buff_iter->pixelFormat == ::YCRCB_SWAPY)
        {
          /*
          ((unsigned char*)in.get_addr(i*C+j))[0] = *((unsigned char*)((char*)buff_iter->p + offset + 0));
          ((unsigned char*)in.get_addr(i*C+j))[1] = *((unsigned char*)((char*)buff_iter->p + offset + 1));
          ((unsigned char*)in.get_addr(i*C+j))[2] = *((unsigned char*)((char*)buff_iter->p + offset + 2));
          ((unsigned char*)in.get_addr(i*C+j))[3] = *((unsigned char*)((char*)buff_iter->p + offset + 1));
          */
          char *pTempBase = (char*)buff_iter->p + offset;

          unsigned char tmp00 = *((unsigned char*)(pTempBase + 0));
          unsigned char tmp01 = *((unsigned char*)(pTempBase + 1));
          unsigned char tmp02 = *((unsigned char*)(pTempBase + 2));
          unsigned char tmp03 = *((unsigned char*)(pTempBase + 1));

          in[i][j] = (((T)tmp00)<<24) | (((T)tmp01) << 16) | (((T)tmp02)<< 8) | ((T)tmp03);
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

        if((buff_iter->pixelFormat == ::YCRCB_NORMAL ||
            buff_iter->pixelFormat == ::YCRCB_SWAPY))
        {
          // If we're trying to read outside the left boundary, increase x_pos_a
          //sizeofT
          /*
            case 1 matrix 1 byte per element
            case 2 matrix 2 byte per element
            case 3 matrix 4 byte per element
          */
          if((j + (4/sizeofT)) > C) {
            printf("Invalid matrix width [%d] for Packed format!\n", idx.get_data()&0xFF);
            exit(EXIT_FAILURE);
          }
          //setting offset to width - 4 for row we are processing
          offset = y_pos_a * width + width - 4;
          if(buff_iter->pixelFormat == ::YCRCB_NORMAL)
          {
            /*
            ((unsigned char*)in.get_addr(i*C+j))[0] = *((unsigned char*)((char*)buff_iter->p + offset + 2));
            ((unsigned char*)in.get_addr(i*C+j))[1] = *((unsigned char*)((char*)buff_iter->p + offset + 1));
            ((unsigned char*)in.get_addr(i*C+j))[2] = *((unsigned char*)((char*)buff_iter->p + offset + 2));
            ((unsigned char*)in.get_addr(i*C+j))[3] = *((unsigned char*)((char*)buff_iter->p + offset + 3));
            */
            char *pTempBase = (char*)buff_iter->p + offset;

            unsigned char tmp00 = *((unsigned char*)(pTempBase + 2));
            unsigned char tmp01 = *((unsigned char*)(pTempBase + 1));
            unsigned char tmp02 = *((unsigned char*)(pTempBase + 2));
            unsigned char tmp03 = *((unsigned char*)(pTempBase + 3));

            in[i][j] = (((T)tmp00)<<24) | (((T)tmp01) << 16) | (((T)tmp02)<< 8) | ((T)tmp03);
          }else if(buff_iter->pixelFormat == ::YCRCB_SWAPY)
          {
            /*
            ((unsigned char*)in.get_addr(i*C+j))[0] = *((unsigned char*)((char*)buff_iter->p + offset + 0));
            ((unsigned char*)in.get_addr(i*C+j))[1] = *((unsigned char*)((char*)buff_iter->p + offset + 3));
            ((unsigned char*)in.get_addr(i*C+j))[2] = *((unsigned char*)((char*)buff_iter->p + offset + 2));
            ((unsigned char*)in.get_addr(i*C+j))[3] = *((unsigned char*)((char*)buff_iter->p + offset + 3));
            */
            char *pTempBase = (char*)buff_iter->p + offset;

            unsigned char tmp00 = *((unsigned char*)(pTempBase + 0));
            unsigned char tmp01 = *((unsigned char*)(pTempBase + 3));
            unsigned char tmp02 = *((unsigned char*)(pTempBase + 2));
            unsigned char tmp03 = *((unsigned char*)(pTempBase + 3));

            in[i][j] = (((T)tmp00)<<24) | (((T)tmp01) << 16) | (((T)tmp02)<< 8) | ((T)tmp03);
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
                in[i][j] = *((T*)((char*)buff_iter->p + offset + bpp_count));
              }
              j--;
              break;
            }
            else
            {
              // ((unsigned char*)in.get_addr(i*C+j))[byte_count] = *((unsigned char*)((char*)buff_iter->p + offset));
              unsigned char *pTempBase = in[i].data() + j * sizeof(T);
              pTempBase[byte_count] = *((unsigned char*)((char*)buff_iter->p + offset));
            }

            x_pos_a = x_pos_a + 1;
          }
          x_pos_a = width;
        }
      }
      else {
        offset = y_pos_a * width + x_pos_a;
        {
          in[i][j] = *((T*)((char*)buff_iter->p + offset));
        }
      }
    }
  }
  return true;
}

template<typename T>
ESIMD_API
bool matrix_write(SurfaceIndex & idx,
                           int x_pos,
                           int y_pos,
                           const std::vector<std::vector<T>> & out)
{
  int R = out.size();
  int C = out[0].size();
  // temp
  cm_list<::CmEmulSys::iobuffer>::iterator buff_iter =
      ::CmEmulSys::search_buffer(idx.get_data()&0xFF);

  if(buff_iter == ::CmEmulSys::iobuffers.end()) {
    printf("Error reading buffer %d: buffer %d is not registered!\n",
           idx.get_data()&0xFF, idx.get_data()&0xFF);
    exit(EXIT_FAILURE);
  }

  std::unique_lock<std::mutex> lock(mutexForWrite);

  int width = buff_iter->width;
  int height = buff_iter->height;

  if((x_pos % 4) != 0) {
    printf("Error writing buffer %d: X-coordinate must be 4-byte aligned!\n", idx.get_data()&0xFF);
    exit(EXIT_FAILURE);
  }
  int sizeofT = sizeof(T); /* Make this into a signed integer */
  if(((C * sizeofT) % 4) != 0) {
    printf("Error writing buffer %d: input matrix width must be 4-byte aligned!\n", idx.get_data()&0xFF);
    exit(EXIT_FAILURE);
  }

  if((buff_iter->bclass != GEN4_OUTPUT_BUFFER) &&
     (buff_iter->bclass != GEN4_INPUT_OUTPUT_BUFFER)) {
    printf("Error writing buffer %d: incorrect buffer type!\n", idx.get_data()&0xFF);
  }

  uint x_pos_a, y_pos_a;  /* Actual positions */
  uint bpp = CM_genx_bytes_per_pixel(buff_iter->pixelFormat);
  assert(((bpp - 1) & bpp) == 0); // Is power-of-2 number

  int offset;

  for (int i = 0; i < R; i++) {
    for (int j = 0; j < C; j++) {
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
        *((T*)( (char*)buff_iter->p_volatile + offset )) = out[i][j];
      } else {
        *((T*)( (char*)buff_iter->p + offset )) = out[i][j];
      }
    }
  }

  return true;
}

ESIMD_API
void fence()
{
  ::cm_fence();
}

ESIMD_API
void fence(unsigned char bit_mask)
{
  ::cm_fence(bit_mask);
}

} // namespace cm_support

#endif // _ESIMDCPU_DATAPORT_H_
