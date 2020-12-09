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

#include <chrono>

#include <cm_priv_def.h>
#include <cm_kernel_base.h>
//#include <cm_debug.h>

#include "rt.h"

#define FFI_BUILDING
#include <ffi.h>

CmEmu_KernelLauncher::CmEmu_KernelLauncher(
    VoidFuncPtr p,
    const std::vector<CmEmuArg>& argsVecRef,
    size_t threadId) :
        m_kernel_func_ptr((void (*)())p),
        m_argsVecRef(argsVecRef),
        m_thread_linear_id(threadId)
{
}

CmEmu_KernelLauncher::~CmEmu_KernelLauncher()
{
}

namespace cmrt
{
size_t thread_linear_id ();
}

void CmEmu_KernelLauncher::launch()
{
    if(m_thread_linear_id == kThreadIdUnset)
    {
         m_thread_linear_id = cmrt::thread_linear_id ();
    }

    std::vector<CmEmu_KernelLauncher::ArgInfo> argsVect;

    for (auto& arg: m_argsVecRef) {
        if (!arg.isSet ()) break;
        argsVect.emplace_back(arg.getBufferPtr(arg.isPerThread () ? m_thread_linear_id : 0), arg.getUnitSize ());
    }

    std::vector<ffi_type*> argsLibFfiT;
    std::vector<void*>     argsLibFfiV;

    using FfiStructData = std::pair<std::vector<ffi_type*>, ffi_type>;

    for (const auto& a: argsVect)
    {
        const bool isStruct = a.size > sizeof(long long);

        argsLibFfiT.emplace_back(
            a.size == sizeof(char)   ? &ffi_type_uchar :
            a.size == sizeof(short)  ? &ffi_type_ushort :
            a.size == sizeof(int)    ? &ffi_type_uint :
            a.size == sizeof(long)   ? &ffi_type_ulong :
            a.size == sizeof(double) ? &ffi_type_double :
            &ffi_type_pointer
        );

        argsLibFfiV.emplace_back(
            isStruct ? (void*)&a.pValue : a.pValue
        );
    }

    ffi_cif cif;
    if(ffi_prep_cif(&cif, FFI_DEFAULT_ABI, argsLibFfiT.size (), &ffi_type_void, argsLibFfiT.data ())
        == FFI_OK)
    {
        ffi_call(
            &cif,
            m_kernel_func_ptr,
            nullptr,
            argsLibFfiV.data ());
    }
}

namespace cmrt
{

//=============================================================================
void CmEmuMt_ThreadBell::wait_for_ring() {
     std::unique_lock<std::mutex> lk(m_mutex);
     m_condition.wait(lk, [this] {
          if (this->m_ringing == true) {
               this->m_ringing = false;
               return true;
          } else {
               return false;
          }
     });
}

//-----------------------------------------------------------------------------
void CmEmuMt_ThreadBell::ring() {
    std::lock_guard<std::mutex> lk(m_mutex);
    m_ringing = true;
    m_condition.notify_all();
}

//=============================================================================
CmEmuMt_Thread::CmEmuMt_Thread(uint32_t local_idx, uint32_t group_idx,
                   std::shared_ptr<CmEmuMt_GroupState> resources,
                   std::shared_ptr<CmEmuMt_GroupState> extra_resources,
                   CmEmu_KernelLauncher launcher,
                   CmEmuMt_Kernel* kernel)
    : m_local_idx(local_idx),
      m_group_idx(group_idx),
      m_resources(resources),
      m_extra_resources(extra_resources),
      m_kernel_launcher(launcher),
      m_kernel(kernel)
{
    //m_fcompleted = m_pcompleted.get_future();
    m_os_thread = std::thread(&CmEmuMt_Thread::wrapper, this);
}

CmEmuMt_Thread::~CmEmuMt_Thread() { m_os_thread.join(); }

//-----------------------------------------------------------------------------
void CmEmuMt_Thread::suspend() { kernel()->suspend_thread(this); }
void CmEmuMt_Thread::resume() { kernel()->resume_thread(this); }

//-----------------------------------------------------------------------------
bool CmEmuMt_Thread::next_group() {
    m_group_idx += kernel()->resident_groups_limit();
    if (m_group_idx < kernel()->group_count()) {
        std::swap(m_resources, m_extra_resources);
        return true;
    }
    return false;
}

//-----------------------------------------------------------------------------
void CmEmuMt_Thread::complete() {
    m_state.store(CmEmuMt_Thread::State::COMPLETED);
    kernel()->complete_thread(this);
}

//-----------------------------------------------------------------------------
bool CmEmuMt_Thread::completed() {
    return m_state.load() == State::COMPLETED;
}

//-----------------------------------------------------------------------------
void CmEmuMt_Thread::wrapper() {
    g_resident_thread = this;
    suspend();
    do {
        execute();
        aux_barrier()->signal(this);
        aux_barrier()->wait(this);
        fflush(stderr);
    } while (next_group());
    complete();
}

//=============================================================================

CmEmuMt_Kernel::CmEmuMt_Kernel(
    std::vector<uint32_t> grid_dims,
    std::vector<uint32_t> group_dims,
    uint32_t resident_groups_limit,
    uint32_t parallel_threads_limit,
    CmEmu_KernelLauncher launcher)
    : m_kernel_launcher(launcher),
      m_grid_dims(grid_dims),
      m_group_dims(group_dims),
      m_resident_groups_limit(resident_groups_limit),
      m_parallel_threads_limit(parallel_threads_limit) {

      auto get_strides =
          [](std::vector<uint32_t> dims) {
              auto n = dims.size();
              std::vector<uint32_t> strides(n, 1);
              for (size_t i = 1; i < n; ++i) {
                  strides[n-1-i] = strides[n-i]*dims[n-i];
              }
              return strides;
          };

      m_grid_strides = get_strides(m_grid_dims);
      m_group_strides = get_strides(m_group_dims);
      m_group_count = m_grid_strides[0] * m_grid_dims[0];
      m_group_size = m_group_strides[0] * m_group_dims[0];

      m_resident_groups_limit = std::min(m_group_count, resident_groups_limit);
}

CmEmuMt_Kernel::~CmEmuMt_Kernel(){
}

//-----------------------------------------------------------------------------
void CmEmuMt_Kernel::suspend_thread(CmEmuMt_Thread* thread) {
    if (!thread->suspended ()) {
        thread->state(CmEmuMt_Thread::State::SUSPENDED);
        m_running_threads_count.fetch_sub(1);
        thread->bell()->wait_for_ring();
    } else {
        std::cerr << "*** Error: trying to suspend an already suspended thread." << std::endl;
        exit(EXIT_FAILURE);
    };
}

//-----------------------------------------------------------------------------
void CmEmuMt_Kernel::resume_thread(CmEmuMt_Thread * thread) {
    if (thread->suspended ())
    {
        thread->state(CmEmuMt_Thread::State::RUNNING);
        m_running_threads_count.fetch_add(1);
        thread->bell()->ring();
    } else {
        std::cerr << "*** Error: trying to resume not a suspended thread." << std::endl;
        exit(EXIT_FAILURE);
    };
}

//-----------------------------------------------------------------------------
void CmEmuMt_Kernel::complete_thread(CmEmuMt_Thread *) {
    m_running_threads_count.fetch_sub(1);
}

//-----------------------------------------------------------------------------
uint32_t CmEmuMt_Kernel::thread_idx(uint32_t idx, uint32_t dim) {
    assert(dim < m_group_dims.size());
    return (idx / m_group_strides[dim]) % m_group_dims[dim];
}

uint32_t CmEmuMt_Kernel::group_idx(uint32_t idx, uint32_t dim) {
    assert(dim < m_grid_dims.size());
    return (idx / m_grid_strides[dim]) % m_grid_dims[dim];
}

uint32_t CmEmuMt_Kernel::thread_count(uint32_t dim) {
    assert(dim < m_group_dims.size());
    return m_group_dims[dim];
}

uint32_t CmEmuMt_Kernel::group_count(uint32_t dim) {
    assert(dim < m_grid_dims.size());
    return m_grid_dims[dim];
}

//-----------------------------------------------------------------------------
bool CmEmuMt_Kernel::run(double timeout)
{
    std::list<CmEmuMt_Thread> threadsList;

    for (uint32_t group_idx = 0; group_idx < m_resident_groups_limit; ++group_idx) {
        auto resources = std::make_shared<CmEmuMt_GroupState>(this);
        auto extra_resources = std::make_shared<CmEmuMt_GroupState>(this);

        for (uint32_t local_idx = 0; local_idx < m_group_size; ++local_idx) {
            m_running_threads_count++;
            threadsList.emplace_back(
                    local_idx,
                    group_idx,
                    resources, extra_resources,
                    m_kernel_launcher,
                    this);
        }
    }

    if (!timeout)
        timeout = kCmEmuKnlTimeout;

    const auto startTime = std::chrono::system_clock::now();
    auto isTimeout = [startTime,timeout] {
        static int chkTimeoutI = 0;
        if(!(chkTimeoutI++ % 100) &&
            std::chrono::duration<double> {std::chrono::system_clock::now() - startTime}.count () > timeout)
        {
            std::cerr << "*** Error: timeout while running a kernel!" << std::endl;
            return true;
        }
        return false;
    };

    auto curThreadIt = threadsList.begin();
    while (threadsList.size ())
    {
        if (isTimeout ()) return false;

        if (curThreadIt->suspended()) {
            if(m_running_threads_count.load() < m_parallel_threads_limit)
                curThreadIt->resume ();
        }

        curThreadIt = curThreadIt->completed () ?
            threadsList.erase(curThreadIt) :
            std::next(curThreadIt);

        if (curThreadIt == threadsList.end())
            curThreadIt = threadsList.begin();
    }

    return true;
}

//=============================================================================
CmEmuMt_GroupBarrier::CmEmuMt_GroupBarrier(CmEmuMt_Kernel *kernel) : m_kernel(kernel) {
    m_local_sense.resize(m_kernel->group_size(), 0);
}

//-----------------------------------------------------------------------------
void CmEmuMt_GroupBarrier::signal(CmEmuMt_Thread *thread) {
    // we use split-version of sensing barrier
    m_local_sense[thread->local_idx()] = ~m_local_sense[thread->local_idx()];
    while (m_counter.load() > m_kernel->group_size() - 1)
        ;
    if (m_counter.fetch_add(1) == m_kernel->group_size() - 1) {
        m_counter.store(0);
        m_global_sense.store(m_local_sense[thread->local_idx()]);
    }
}

//-----------------------------------------------------------------------------
void CmEmuMt_GroupBarrier::wait(CmEmuMt_Thread *thread) {
    do {
        thread->suspend();
    } while (m_global_sense.load() != m_local_sense[thread->local_idx()]);
}

//=============================================================================

thread_local CmEmuMt_Thread *g_resident_thread{nullptr};

int32_t thread_global_idx() {
    return g_resident_thread->local_idx() +
         g_resident_thread->group_idx() *
             group_size();
}

int32_t thread_local_idx() { return g_resident_thread->local_idx(); }
int32_t thread_block_idx() { return g_resident_thread->group_idx(); }

int32_t thread_idx(uint32_t dim) {
    auto local_idx = g_resident_thread->local_idx();
    return g_resident_thread->kernel()->thread_idx(local_idx, dim);
}

int32_t group_idx(uint32_t dim) {
    auto group_idx = g_resident_thread->group_idx();
    return g_resident_thread->kernel()->group_idx(group_idx, dim);
}

int32_t thread_count(uint32_t dim) {
    return g_resident_thread->kernel()->thread_count(dim);
}

int32_t group_count(uint32_t dim) {
    return g_resident_thread->kernel()->group_count(dim);
}

int32_t group_size () {
    return g_resident_thread->kernel()->group_size();
}

void simple_group_barrier_signal()
{
    return g_resident_thread->simple_barrier()->signal(g_resident_thread);
}

void simple_group_barrier_wait()
{
    g_resident_thread->simple_barrier()->wait(g_resident_thread);
}

void aux_barrier_signal() { g_resident_thread->aux_barrier()->signal(g_resident_thread);}
void aux_barrier_wait() { g_resident_thread->aux_barrier()->wait(g_resident_thread);}

void suspend() {}

size_t thread_linear_id ()
{
    return thread_idx(0) + thread_count(0) * (thread_idx(1) + thread_count(1) * (thread_idx(2)));
}

CmEmuMt_Thread * get_thread ()
{
    return g_resident_thread;
}

char *get_slm(){
    return g_resident_thread->slm();
}

void set_slm_size(unsigned int size){
    g_resident_thread->set_slm_size(size);
}

size_t get_slm_size()
{
    return g_resident_thread->get_slm_size();
}

unsigned int alloc_slm(unsigned int bufferSize){
    return g_resident_thread->alloc_slm(bufferSize);
}

void this_thread_yield(){
    g_resident_thread->suspend();
}

//=============================================================================

void CmEmuMt_SLM::set_size (unsigned int size)
{
    std::lock_guard<std::mutex> lk(m_mutex);

    if (!size) {
        fprintf(stderr, "Error in SLM Emulation:  Specify SLM size!\n");
        exit(EXIT_FAILURE);
    }

    int slm_max_size = CmEmuMt_SLM::kDefaultMaxSize;

    size = ((int)ceil((double)size / CmEmuMt_SLM::kChunkSize)) * CmEmuMt_SLM::kChunkSize;

    if (size > slm_max_size) {
        fprintf(stderr, "Error in SLM Emulation:  Max SLM size = %dK!\n", slm_max_size/1024);
        exit(EXIT_FAILURE);
    }

    if (m_memory.empty())
    {
        m_memory.resize(size);
    }
    else
    {
        if (m_memory.size() != size)
        {
            throw std::runtime_error("SLM size already set to " + std::to_string(m_memory.size()));
        }
    }

    m_size = size;
}

} // namespace cmrt
