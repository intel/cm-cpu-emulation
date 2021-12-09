/*========================== begin_copyright_notice ============================

Copyright (C) 2019 Intel Corporation

SPDX-License-Identifier: MIT

============================= end_copyright_notice ===========================*/

#pragma once
#ifndef CM_MT_RT_INCLUDED
#define CM_MT_RT_INCLUDED

#include <cassert>
#include <cstdint>
#include <stdexcept>

#include <memory>

#include <mutex>
#include <thread>
#include <future>
#include <atomic>
#include <condition_variable>

#include <list>
#include <array>
#include <vector>
#include <iostream>
#include <string>

#include "cm_vm.h"
#include "emu_kernel_arg.h"
#include "emu_kernel_support.h"

namespace cmrt
{

class CmEmu_KernelLauncher
{
public:
    friend class CmEmuMt_Thread;
    using VoidFuncPtr = void(*)();
    static constexpr size_t kThreadIdUnset = -1;

private:

    const GfxEmu::KernelSupport::ProgramModule& m_programModule;
    VoidFuncPtr m_kernel_func_ptr = nullptr;
    std::string m_kernelName;
    std::vector<GfxEmu::KernelArg> m_args;

    size_t m_thread_linear_id;

public:
    CM_API CmEmu_KernelLauncher(
        VoidFuncPtr,
        const std::vector<GfxEmu::KernelArg>&,
        size_t threadId = kThreadIdUnset);

    CM_API CmEmu_KernelLauncher(
        const char *kernelName,
        const GfxEmu::KernelSupport::ProgramModule& programModule,
        const std::vector<GfxEmu::KernelArg>&,
        size_t threadId = kThreadIdUnset,
        VoidFuncPtr = nullptr);

    CM_API ~CmEmu_KernelLauncher();

protected:
    CM_API void launch();
};

using CmEmuThreadBroadcastEl = uint32_t;

constexpr size_t
    kGrfSize = 32,
    kCmEmuKnlTimeout = 40000
;

extern thread_local class CmEmuMt_Thread* g_resident_thread; // For helper funcs.

class CmEmuMt_ThreadBell
{
private:
    std::mutex              m_mutex;
    std::condition_variable m_condition;
    bool                    m_ringing{false};

public:
    void wait_for_ring(); // Will suspend thread while waiting.
    void ring();
};

class CmEmuMt_Kernel;

class CmEmuMt_GroupBarrier
{
private:
    CmEmuMt_Kernel *      m_kernel;
    std::atomic<uint32_t> m_counter{0};
    std::atomic<uint32_t> m_global_sense{0};
    std::vector<uint32_t> m_local_sense;

public:
    CmEmuMt_GroupBarrier(CmEmuMt_Kernel *kernel);
    void         signal(CmEmuMt_Thread *thread);
    void         wait(CmEmuMt_Thread *thread);
    CmEmuMt_Kernel* kernel() const { return m_kernel; }
};

struct CmEmuMt_SLM
{
    constexpr static size_t
        kChunkSize = 4ll << 10,
        kDefaultMaxSize = 64ll << 10
    ;
    std::vector<char> m_memory;
    std::mutex        m_mutex;

    unsigned int m_buffer_offset = 0;
    unsigned int m_basic_offset  = 0;
    size_t       m_size          = 0;

    char *data()
    {
        return m_memory.data();
    }

    void set_size(unsigned int size);

    size_t get_size() {return m_size;}

    unsigned int alloc(unsigned int bufferSize)
    {
        std::lock_guard<std::mutex> lk(m_mutex);

        if (m_memory.empty())
        {
            throw std::runtime_error("SLM not initialized");
        }

        if (bufferSize > m_memory.size())
        {
            throw std::runtime_error("SLM allocate size larger than initial size " +
                std::to_string(m_memory.size()));
        }

        // Note: don't support multiple allocation
        if (m_buffer_offset == 0)
        {
            m_basic_offset = m_buffer_offset;
            m_buffer_offset += bufferSize;
        }
        else
        {
            if (m_buffer_offset != bufferSize)
            {
                throw std::runtime_error("SLM already allocated with size " +
                    std::to_string(bufferSize));
            }
        }

        return m_basic_offset;
    }
};

struct CmEmuMt_GroupState
{
    CmEmuMt_SLM slm;
    std::unique_ptr<CmEmuMt_GroupBarrier> simple_barrier, aux_barrier;

    CmEmuMt_GroupState(CmEmuMt_Kernel *kernel)
    {
        simple_barrier = std::make_unique<CmEmuMt_GroupBarrier>(kernel);
        aux_barrier = std::make_unique<CmEmuMt_GroupBarrier>(kernel);
    }
};

class CmEmuMt_Thread
{
public:
    enum class State
    {
        UNSPAWNED,
        RUNNING,
        SUSPENDED,
        COMPLETED
    };

private:
    uint32_t m_local_idx, m_group_idx; // local thread index, and group_dims index
    // we need extra resources to avoid races when we start executing the next
    // thread group_dims in the same thread. so we ping-pong between the two
    std::shared_ptr<CmEmuMt_GroupState> m_resources, m_extra_resources;
    CmEmu_KernelLauncher   m_kernel_launcher;
    CmEmuMt_Kernel*        m_kernel;
    CmEmuMt_ThreadBell     m_bell;
    std::unique_ptr<std::thread>  m_os_thread_ptr;
    std::atomic<State>     m_state {State::RUNNING};

public:
    CmEmuMt_Thread(
        CmEmu_KernelLauncher launcher,
        CmEmuMt_Kernel* kernel);
    CmEmuMt_Thread(
        uint32_t local_idx,
        uint32_t group_idx,
        std::shared_ptr<CmEmuMt_GroupState> resources,
        std::shared_ptr<CmEmuMt_GroupState> extra_resources,
        CmEmu_KernelLauncher launcher,
        CmEmuMt_Kernel* kernel);
    ~CmEmuMt_Thread();

    void        wrapper();
    void        wrapper_debug();
    void        suspend();
    void        resume();
    void        complete();

    bool                unspawned() const { return m_state.load() == State::UNSPAWNED; }
    bool                suspended() const { return m_state.load() == State::SUSPENDED; }
    bool                running() const { return m_state.load() == State::RUNNING; }
    bool                completed();
    void                state(State state) { m_state.store(state); }
    CmEmuMt_ThreadBell *bell() { return &m_bell; }
    uint32_t            local_idx() const { return m_local_idx; }
    uint32_t            group_idx() const { return m_group_idx; }
    bool                next_group();
    void                execute();

    CmEmuMt_Kernel* kernel() const { return m_kernel; }
    CmEmuMt_GroupBarrier* simple_barrier() const { return m_resources->simple_barrier.get(); }
    CmEmuMt_GroupBarrier* aux_barrier() const { return m_resources->aux_barrier.get(); }
    std::shared_ptr<CmEmuMt_GroupState> resources() const { return m_resources; }

    char *                slm() const { return m_resources->slm.data(); }
    void                  set_slm_size(unsigned int size) { m_resources->slm.set_size(size); }
    size_t                get_slm_size() { return m_resources->slm.get_size();}
    unsigned int          alloc_slm(unsigned int bufferSize) { return m_resources->slm.alloc(bufferSize); }

};

class CmEmuMt_Kernel
{
public:
    static thread_local void* m_sched_ctx;
private:
    CmEmu_KernelLauncher m_kernel_launcher;

    // kernel has total number of groups (or "blocks") to be executed,
    //            resident number of groups, that are alive at the same time,
    //            size of a group and the number of parallel workers.
    // we spawn m_group_size*m_number_resident_groups of threads
    // so some OSes may have limits
    std::vector<uint32_t> m_grid_dims, m_grid_strides;
    std::vector<uint32_t> m_group_dims, m_group_strides;
    uint32_t              m_group_count{0},
                          m_resident_groups_limit{0},
                          m_group_size{0},
                          m_parallel_threads_limit{0};

    // keep track of currently running number of threads, must be < m_parallel_threads_limit
    std::atomic<uint32_t> m_running_threads_count{0};

public:
    CM_API CmEmuMt_Kernel(
        std::vector<uint32_t> grid_dims,
        std::vector<uint32_t> group_dims,
        uint32_t resident_groups_limit,
        uint32_t parallel_threads_limit,
        CmEmu_KernelLauncher launcher);

    CM_API ~CmEmuMt_Kernel();
    CM_API bool run(double timeout = 0);
    CM_API bool run_debug();
    void     suspend_thread(CmEmuMt_Thread *);
    void     resume_thread(CmEmuMt_Thread *);
    void     complete_thread(CmEmuMt_Thread *);
    uint32_t group_size() const { return m_group_size; }
    uint32_t group_count() const { return m_group_count; }
    uint32_t resident_groups_limit() const { return m_resident_groups_limit; }

    uint32_t thread_idx(uint32_t idx, uint32_t dim);
    uint32_t group_idx(uint32_t idx, uint32_t dim);

    uint32_t thread_count(uint32_t idx);
    uint32_t group_count(uint32_t idx);
};

/* helper function that can be called by the client kernel */

int32_t thread_global_idx();
int32_t thread_local_idx();
int32_t thread_block_idx();
int32_t thread_idx(uint32_t);
int32_t thread_count(uint32_t);
int32_t group_idx(uint32_t);
int32_t group_count(uint32_t);
int32_t group_size();

void    simple_group_barrier_signal();
void    simple_group_barrier_wait();

void    aux_barrier_signal();
void    aux_barrier_wait();

CmEmuMt_Thread * get_thread ();

char *       get_slm();
void         set_slm_size(unsigned int size);
size_t       get_slm_size();
unsigned int alloc_slm(unsigned int bufferSize);

CM_API void this_thread_yield();

}  // namespace cmrt

#endif // CM_MT_RT_INCLUDED
