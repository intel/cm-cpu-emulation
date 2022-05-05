/*========================== begin_copyright_notice ============================

Copyright (C) 2017 Intel Corporation

SPDX-License-Identifier: MIT

============================= end_copyright_notice ===========================*/

#ifndef GUARD_common_cm_kernel_base_h
#define GUARD_common_cm_kernel_base_h

#include <cstddef>
#include <cstring>
#include <stdexcept>
#include <memory>
#include <string>

#include "cm_include.h"
#include "emu_kernel_arg.h"

class SurfaceIndex;
class CmThreadSpace;
class CmThreadGroupSpace;

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
  uint8_t kernelNBarrierCnt;
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
