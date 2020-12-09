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

#ifndef GUARD_common_cm_kernel_base_h
#define GUARD_common_cm_kernel_base_h

#include <cstddef>
#include <cstring>
#include <cassert>
#include <stdexcept>
#include <memory>

#include "cm_include.h"
//#include "cm_debug.h"
//#include "cm_mem.h"

class SurfaceIndex;
class CmThreadSpace;
class CmThreadGroupSpace;

struct CmEmuArg {
public:
  enum class SurfaceKind {
    DATA_PORT_SURF
  };

private:
  std::shared_ptr<void> m_bufPtr {nullptr};
  uint32_t m_unitCount {0};
  uint16_t m_unitSize {0};
  uint16_t m_unitAlignedSize {0};

private:
  size_t roudUpToMaxLign (size_t size) {
    return size % alignof(std::max_align_t) + size;
  }

  bool allocateBuffer (size_t size, size_t count) {
    if (m_bufPtr) {
      if (m_unitSize != size || m_unitCount != count) {
        //CmErrorMessage ("Argment buffer of %u elements of size %u already allocated."
        //  "Requested to allocate again with count %u of elements of size %u.",
        //    unitSize, unitCount, size, count);
        return false;
      }

      return true;
    }

    assert (size && "Requested to allocate buffer of zero size.");
    assert (count && "Requested to allocate buffer of zero count.");

    m_unitCount = count;
    m_unitSize = size;
    m_unitAlignedSize = roudUpToMaxLign (size);
    m_bufPtr.reset (std::calloc(count, m_unitAlignedSize), std::free);
    return true;
  }

public:
  CmEmuArg () noexcept = default;
  CmEmuArg (const void* fromPtr, size_t size) {
    if(!setValueFrom (fromPtr, size))
        throw std::runtime_error("CmEmuArg: failed to set value on construction.");
  }
  CmEmuArg (const void* fromPtr, size_t size, size_t tid, size_t count) {
    if(!setValueFrom (fromPtr, size, tid, count))
        throw std::runtime_error("CmEmuArg: failed to set per-thread value on construction.");
  }

  bool isSet () const { return m_unitCount; }
  bool isPerThread () const { return m_unitCount > 1; }

  size_t getUnitCount () const { return m_unitCount; }
  size_t getUnitSize () const { return m_unitSize; }
  void reset () {
    m_bufPtr.reset ();
    m_unitCount = 0;
    m_unitSize = 0;
    m_unitAlignedSize = 0;
  }

  // Letting raw in-buffer pointers out for the low-level stuff of libffi.
  void* getBufferPtr (size_t tid = 0) const {
    if (!m_bufPtr)
        return m_bufPtr.get ();

    assert (m_unitCount && tid < m_unitCount &&
        "Thread ID must be < than unitCount configured for the argument.");

    return
        static_cast<uint8_t*> (m_bufPtr.get ()) +
            tid * m_unitAlignedSize;
  }

  bool setValueFrom (
    const void* fromPtr,
    size_t size)
  {
    if (allocateBuffer (size, 1)) {
      std::memcpy(getBufferPtr (), fromPtr, size);
      return true;
    }

    return false;
  }

  bool setValueFrom (
    const void* fromPtr,
    size_t size,
    size_t tid,
    size_t count)
  {
    if (allocateBuffer (size, count)) {
      std::memcpy(getBufferPtr (tid), fromPtr, size);
      return true;
    }

    return false;
  }
};

typedef struct {
  unsigned short name_index;
  unsigned char size;
  unsigned char* values;
  char *name;
} attribute_info_t;

typedef struct {
  unsigned short name_index;
  unsigned char bit_properties;
  unsigned short num_elements;
  unsigned short alias_index;
  unsigned short alias_offset;
  unsigned char attribute_count;
  attribute_info_t* attributes;
} gen_var_info_t;

typedef struct {
  unsigned short name_index;
  unsigned short num_elements;
  unsigned char attribute_count;
  attribute_info_t* attributes;
} spec_var_info_t;

typedef struct {
  unsigned short name_index;
  unsigned char kind;
  unsigned char attribute_count;
  attribute_info_t* attributes;
} label_info_t;

#define CM_MAX_KERNEL_NAME_SIZE_IN_BYTE 256
struct CM_KERNEL_INFO {
  char kernelName[ CM_MAX_KERNEL_NAME_SIZE_IN_BYTE ];
  uint32_t inputCountOffset;

  //Just a copy for original binary pointer and size (GTPin using only)
  void* pOrigBinary;
  uint32_t uiOrigBinarySize;

  unsigned short globalStringCount;
  const char** globalStrings;
  char kernelASMName[CM_MAX_KERNEL_NAME_SIZE_IN_BYTE + 1];        //The name of the Gen assembly file for this kernel (no extension)
  uint8_t kernelSLMSize;     //Size of the SLM used by each thread group
  uint32_t variable_count;
  gen_var_info_t *variables;
  uint32_t address_count;
  spec_var_info_t *address;
  uint32_t predicte_count;
  spec_var_info_t *predictes;
  uint32_t label_count;
  label_info_t *label;
  uint32_t surface_count;
  spec_var_info_t *surface;
  uint32_t kernelInfoRefCount;    //reference counter for kernel info to reuse kernel info and jitbinary
  uint32_t per_thread_input_size;
};

typedef struct _CM_KERNEL_M_INFO {
  char kernelASMName[ CM_MAX_KERNEL_NAME_SIZE_IN_BYTE+4 ];
  uint32_t id;
  uint32_t length;
} CM_KERNEL_M_INFO;

struct CM_HAL_MAX_VALUES;

typedef struct _CM_SURFACE_BINDING_INDEX {
  int16_t surface_id;
  int16_t binding_id;
  int16_t binding_entries;
  int16_t binding_insert;
} CM_SURFACE_BINDING_INDEX;

#define CM_BINDING_TABLE_SIZE 256

#define CM_MAX_GLOBAL_SURFACE_NUMBER 4
#define CM_MAX_SURFACES_PER_KERNEL 255

#define CM_MAX_SAMPLER_TABLE_SIZE 64
#define CM_MAX_SAMPLERS_PER_KERNEL 16

#include "type_kernel_base.h"

#endif  // GUARD_common_cm_kernel_base_h
