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

#include <cm_kernel_base.h>

#include "cm_intrin.h"
#include "libcm_def.h"
#include "cm_list.h"
#include "emu_log.h"
#include "cm_common_macros.h"
#include "dataport_common.h"

#include "rt.h"
#include "esimdcpu_support.h"
#include "emu_cfg.h"

/// Imported from rt.h : Begin

constexpr size_t
    kGrfSize = 32,
    kCmEmuKnlTimeout = 40000
;

/// Imported from rt.h : End

ESIMD_API
ESimdCPUKernel::ESimdCPUKernel(const fptrVoid entryPoint,
                               const std::vector<uint32_t> &groupDim,
                               const std::vector<uint32_t> &localDim)
  : m_entryPoint(entryPoint), m_groupDim(groupDim), m_localDim(localDim)
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
  assert(m_groupDim.size() != 0);
  assert(m_localDim.size() != 0);

  uint32_t residentGrp = GfxEmu::Cfg ().ResidentGroups.getInt();

  cmrt::CmEmuMt_Kernel kernel(
    m_groupDim, /// groupSpaceWidth, groupSpaceHeight, groupSpaceDepth
    m_localDim, /// threadSpaceWidth, threadSpaceHeight, threadSpaceDepth
    residentGrp, // resident group
    m_parallel,
    cmrt::CmEmu_KernelLauncher
    {
      m_entryPoint,
      {{argPtr, argSize}}
    }
  );

  if (getenv("ESIMDCPU_DEBUG")){
    kernel.run_debug();
  }
  else {
    kernel.run();
  }
}

namespace cm_support {

ESIMD_API
size_t get_thread_idx(uint32_t dim)
{
  return (size_t)cmrt::thread_idx(dim);
}

ESIMD_API
size_t get_group_idx(uint32_t dim)
{
  return (size_t)cmrt::group_idx(dim);
}

ESIMD_API
size_t get_thread_count(uint32_t dim)
{
  return (size_t)cmrt::thread_count(dim);
}

ESIMD_API
size_t get_group_count(uint32_t dim)
{
  return (size_t)cmrt::group_count(dim);
}

ESIMD_API
char *get_surface_base_addr(int index)
{
  cm_list<CmEmulSys::iobuffer>::iterator buff_iter =
    CmEmulSys::search_buffer(index & 0xFF);
  if(buff_iter == CmEmulSys::iobuffers.end()) {
    GFX_EMU_FAIL_WITH_MESSAGE("reading buffer %d: buffer %d is not registered!\n",
                              index & 0xFF, index & 0xFF);
    exit(EXIT_FAILURE);
  }

  return (char*) buff_iter->p_volatile;
}

ESIMD_API
int32_t get_thread_local_idx()
{
  return cmrt::thread_local_idx();
}

ESIMD_API
void barrier()
{
  cm_barrier();
}

ESIMD_API
void split_barrier(uint mask)
{
  cm_sbarrier(mask);
}

ESIMD_API
void aux_barrier()
{
  __cm_emu_aux_barrier();
}

ESIMD_API
void init_slm(size_t sz)
{
  cm_slm_init((unsigned int)sz);
}

ESIMD_API
char *get_slm_base(void)
{
  return __cm_emu_get_slm();
}

ESIMD_API
void fence(void)
{
  cm_fence();
}

} // namespace cm_support

