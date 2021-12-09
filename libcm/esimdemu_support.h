/*========================== begin_copyright_notice ============================

Copyright (C) 2020 Intel Corporation

SPDX-License-Identifier: MIT

============================= end_copyright_notice ===========================*/

#ifndef _ESIMDEMU_RUNTIME_H_INCLUDED_
#define _ESIMDEMU_RUNTIME_H_INCLUDED_

#include <vector>

#ifdef _WIN32
#define ESIMD_API __declspec(dllexport)
#else // _WIN32
#define ESIMD_API
#endif // _WIN32

using fptrVoid = void(*)();

/// Imported from rt.h : Begin

/// Imported from rt.h : End

class EsimdemuKernel
{
 private:
  const std::vector<uint32_t> &m_groupDim;
  const std::vector<uint32_t> &m_localDim;
  uint32_t m_parallel;
  fptrVoid m_entryPoint;

 public:
  ESIMD_API
  EsimdemuKernel(fptrVoid entryPoint,
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
void split_barrier(uint32_t mask);

ESIMD_API
void aux_barrier();

ESIMD_API
void init_slm(size_t sz);

ESIMD_API
char *get_slm_base(void);

ESIMD_API
void fence(void);

} // namespace cm_support

#endif // _ESIMDEMU_RUNTIME_H_INCLUDED_
