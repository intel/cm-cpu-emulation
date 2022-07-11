/*========================== begin_copyright_notice ============================

Copyright (C) 2020 Intel Corporation

SPDX-License-Identifier: MIT

============================= end_copyright_notice ===========================*/

#include <cm_kernel_base.h>

#include "cm_intrin.h"
#include "libcm_def.h"
#include "cm_list.h"
#include "emu_log.h"
#include "cm_common_macros.h"
#include "genx_dataport.h"

#include "rt.h"
#include "esimdemu_support.h"
#include "emu_cfg.h"

/// Imported from rt.h : Begin

constexpr size_t
    kMaxNamedBarriersCount = 32,
    kMaxThreadsPerGroup = 128,
    kDpaswMaxSystolicDepth = 8,

    kGrfSize = 32,
    kCmEmuKnlTimeout = 40000,

    kCmEmuGrfSizeInXThreadBufEls = kGrfSize / sizeof(CmEmuThreadBroadcastEl),
    kCmEmuXThreadBroadcastPerThreadBufSize =
        (kDpaswMaxSystolicDepth/2) * kCmEmuGrfSizeInXThreadBufEls,
    kCmEmuXThreadBroadcastBufSize =
        kMaxThreadsPerGroup * kCmEmuXThreadBroadcastPerThreadBufSize
;

/// Imported from rt.h : End
std::vector<CmEmuThreadBroadcastEl> esimd_xthread_broadcast_buffer;

ESIMD_API
EsimdemuKernel::EsimdemuKernel(const fptrVoid entryPoint,
                               const uint32_t *groupDim,
                               const uint32_t *localDim)
  : m_entryPoint(entryPoint) ,
    m_groupDim({groupDim[0], groupDim[1], groupDim[2]}),
    m_localDim({localDim[0], localDim[1], localDim[2]})
{
  m_parallel = (uint32_t)std::thread::hardware_concurrency();
  if (m_parallel == 0)
  {
    m_parallel = 1;
  }
  esimd_xthread_broadcast_buffer.resize(kCmEmuXThreadBroadcastBufSize,0);
}

ESIMD_API
void EsimdemuKernel::launchMT(const uint32_t argSize,
                              const void* argPtr)
{
  assert(m_groupDim.size() != 0);
  assert(m_localDim.size() != 0);

  uint32_t residentGrp = GfxEmu::Cfg::ResidentGroups ().getInt();

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

  if (getenv("ESIMDEMU_DEBUG")){
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
char *get_xthread_broadcast_buf()
{
  return reinterpret_cast<char*>(::esimd_xthread_broadcast_buffer.data());
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
void split_barrier(uint32_t mask)
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
