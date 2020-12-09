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

#include <cm_kernel_base.h>

#include "rt.h"
#include "esimdcpu.hpp"

ESIMD_API
ESimdCPUKernel::ESimdCPUKernel(const fptrVoid entryPoint,
                               const std::vector<uint32_t> &spaceDim)
  : m_entryPoint(entryPoint), m_spaceDim(spaceDim)
{
  m_parallel = (uint32_t)std::thread::hardware_concurrency();
  if (m_parallel == 0)
  {
    m_parallel = 1;
  }
}

ESIMD_API
void ESimdCPUKernel::launchMT(const uint32_t argSize,
                             const void* argPtr)
{
  assert(m_spaceDim.size() != 0);

  cmrt::CmEmuMt_Kernel
  {
    m_singleGrpDim, /// groupSpaceWidth, groupSpaceHeight, groupSpaceDepth
    m_spaceDim, /// threadSpaceWidth, threadSpaceHeight, threadSpaceDepth
    1, // resident group
    m_parallel,
    CmEmu_KernelLauncher
    {
      m_entryPoint,
      {{argPtr, argSize}}
    }
  }.run();
}

ESIMD_API
void esimdcpu_mt_barrier()
{

    cmrt::simple_group_barrier_signal();
    cmrt::simple_group_barrier_wait();
}

ESIMD_API
void esimdcpu_split_barrier(int flag)
{

    flag ? cmrt::simple_group_barrier_signal() :
           cmrt::simple_group_barrier_wait();
}

ESIMD_API
void esimdcpu_set_slm_size(size_t sz)
{
  cmrt::set_slm_size((unsigned int)sz);
  cmrt::alloc_slm((unsigned int)sz);
}

ESIMD_API
size_t esimdcpu_get_slm_size()
{
  return cmrt::get_slm_size();
}

ESIMD_API
char* esimdcpu_get_slm()
{
  return cmrt::get_slm();
}

ESIMD_API
int32_t esimdcpu_thread_local_idx()
{
  return cmrt::thread_local_idx();
}
