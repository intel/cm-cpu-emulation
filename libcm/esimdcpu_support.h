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

#ifndef _ESIMDCPU_RUNTIME_H_INCLUDED_
#define _ESIMDCPU_RUNTIME_H_INCLUDED_

#include <vector>

#define ESIMD_API

using fptrVoid = void(*)();

/// Imported from rt.h : Begin

/// Imported from rt.h : End

class ESimdCPUKernel
{
 private:
  const std::vector<uint32_t> &m_groupDim;
  const std::vector<uint32_t> &m_localDim;
  uint32_t m_parallel;
  fptrVoid m_entryPoint;

 public:
  ESIMD_API
  ESimdCPUKernel(fptrVoid entryPoint,
                 const std::vector<uint32_t> &groupDim,
                 const std::vector<uint32_t> &localDim);

  ESIMD_API
  void launchMT(const uint32_t argSize,
               const void* rawArg);

};

namespace cm_support {

ESIMD_API
size_t get_thread_idx(uint32_t dim);

ESIMD_API
size_t get_group_idx(uint32_t dim);

ESIMD_API
size_t get_thread_count(uint32_t dim);

ESIMD_API
size_t get_group_count(uint32_t dim);

ESIMD_API
char *get_surface_base_addr(int index);

ESIMD_API
int32_t get_thread_local_idx();

ESIMD_API
void barrier();

ESIMD_API
void split_barrier(uint mask);

ESIMD_API
void aux_barrier();

ESIMD_API
void init_slm(size_t sz);

ESIMD_API
char *get_slm_base(void);

ESIMD_API
void fence(void);

} // namespace cm_support

#endif // _ESIMDCPU_RUNTIME_H_INCLUDED_

