/*========================== begin_copyright_notice ============================

Copyright (C) 2019 Intel Corporation

SPDX-License-Identifier: MIT

============================= end_copyright_notice ===========================*/

#include <chrono>

#include <cm_priv_def.h>
#include <cm_kernel_base.h>

#include "rt.h"
#include "emu_log.h"
#include "emu_kernel_support.h"
#include "emu_utils.h"

#include "emu_cfg.h"

using namespace GfxEmu::Log::Flags;

#ifdef LIBFFI_FOUND
#define FFI_BUILDING
#include <ffi.h>
#endif

namespace cmrt
{

size_t thread_linear_id ();

CmEmu_KernelLauncher::CmEmu_KernelLauncher(
    VoidFuncPtr p,
    const std::vector<GfxEmu::KernelArg>& argsVecRef,
    size_t threadId
) : CmEmu_KernelLauncher(
    nullptr,
    GfxEmu::KernelSupport::setupProgram(0),
    argsVecRef,
    threadId,
    p
) {}

CmEmu_KernelLauncher::CmEmu_KernelLauncher(
    const char *kernelName,
    const GfxEmu::KernelSupport::ProgramModule& programModule,
    const std::vector<GfxEmu::KernelArg>& argsVecRef,
    size_t threadId,
    VoidFuncPtr p
) :
    m_programModule(programModule),
    m_kernel_func_ptr(p),
    m_thread_linear_id(threadId)
{
    if(kernelName)
        m_kernelName = kernelName;

    for (const auto& a: argsVecRef) {
        if (!a.isSet()) break;
        m_args.push_back(a);
    }

#ifdef GFX_EMU_KERNEL_SUPPORT_ENABLED
    const auto& paramsDesc =
        GfxEmu::KernelSupport::getKernelDesc(
                        m_kernelName,
                        m_programModule,
                        m_kernel_func_ptr).params;

    if (paramsDesc.size () != m_args.size ()) {
        if (paramsDesc.size ())
            GFX_EMU_FAIL_WITH_MESSAGE(
                "kernel %s arguments count (%u) doesn't match "
                "with parameters count from kernel debug data (%u)\n",
                m_kernelName.c_str(),
                m_args.size (),
                paramsDesc.size ()
            );
    } else {
        int i = 0;
        for(const auto& a: paramsDesc)
            m_args.at(i++).annotate(a);
    }
#endif
}

CmEmu_KernelLauncher::~CmEmu_KernelLauncher()
{
}

void CmEmuMt_Thread::execute() {
    GFX_EMU_MESSAGE_SCOPE_PREFIX(
        std::string{""} +
            "<gid:" + std::to_string(m_group_idx) + ",lid:" + std::to_string(m_local_idx) + "> ");
    m_kernel_launcher.launch();
}

void CmEmu_KernelLauncher::launch()
{
#ifdef LIBFFI_FOUND

    if(m_thread_linear_id == kThreadIdUnset) {
         m_thread_linear_id = cmrt::thread_linear_id ();
    }

    class FfiArgData
    {

    private:
        std::vector<ffi_type*> classMemberDescPtrs;
        ffi_type classTypeDesc;
        void* argPtr__ {nullptr};

    public:
        ffi_type* typeDescPtr;
        void* argPtr {nullptr};

        FfiArgData () = default;
        void create (
                const bool isPointer,
                const bool isFloat,
                bool isClass,
                size_t size,
                void* argPtr_
        ) {
            isClass |= size > sizeof(uint64_t); // Fallback in case no kernel arguments metadata.

            if(isClass) {
#if defined(_WIN32)
                classTypeDesc.size = classTypeDesc.alignment = 0;
                classTypeDesc.type = FFI_TYPE_STRUCT;
                while (size--) classMemberDescPtrs.push_back(&ffi_type_uchar);
                classMemberDescPtrs.push_back(nullptr);

                classTypeDesc.elements =
                    (ffi_type**)classMemberDescPtrs.data();

                typeDescPtr = &classTypeDesc;
                argPtr = argPtr_;
#else
                argPtr__ = argPtr_;
                argPtr = &argPtr__;
                typeDescPtr = &ffi_type_pointer;
#endif
            } else {
                argPtr = argPtr_;
                typeDescPtr =
                    isPointer ? &ffi_type_pointer :
                    isFloat ? (
                        size == sizeof(double) ?      &ffi_type_double :
                        size == sizeof(long double) ? &ffi_type_longdouble :
                                                      &ffi_type_float ) :
                    size == sizeof(uint8_t) ?  &ffi_type_uint8 :
                    size == sizeof(uint16_t) ? &ffi_type_uint16 :
                    size == sizeof(uint32_t) ? &ffi_type_uint32 :
                    size == sizeof(uint64_t) ? &ffi_type_uint64 :
                                               &ffi_type_pointer
                ;
            }
        }
    };

    std::vector<FfiArgData> ffiArgsData {m_args.size()}; // NB: will contain self-referrenced data, no relocations please.

    std::vector<ffi_type*> argsLibFfiT;
    std::vector<void*> argsLibFfiV;
    int argIdx = 0;
    for (const auto& arg: m_args) {

        auto& ffiArgData = ffiArgsData[argIdx++];

        GFX_EMU_MESSAGE(fKernelLaunch | fDetail,
                "arg %s of type %s info: isPtr: %u, isFloat: %u, isClass: %u, size: %u\n",
            arg.name.c_str (),
            arg.typeName.c_str (),
            arg.isPointer,
            arg.isFloat,
            arg.isClass,
            arg.getUnitSize ());

        ffiArgData.create(
            arg.isPointer,
            arg.isFloat,
            arg.isClass,
            arg.getUnitSize (),
            arg.getBufferPtr(arg.isPerThread () ? m_thread_linear_id : 0)
        );

        argsLibFfiT.emplace_back(ffiArgData.typeDescPtr);
        argsLibFfiV.emplace_back(ffiArgData.argPtr);
    }

    GFX_EMU_DEBUG_MESSAGE(fKernelLaunch | fInfo, "calling kernel %s at %p\n",
        m_kernelName.c_str (), m_kernel_func_ptr);

    ffi_cif cif;
    if(ffi_prep_cif(&cif, FFI_DEFAULT_ABI, argsLibFfiT.size (), &ffi_type_void, argsLibFfiT.data ())
        == FFI_OK) {
        ffi_call(
            &cif,
            m_kernel_func_ptr,
            nullptr,
            argsLibFfiV.data ());
    }
    else
        GFX_EMU_FAIL_WITH_MESSAGE(fKernelLaunch,
            "ffi_prep_cif returned not FFI_OK. Unable to prepare data for and call kernel at %p\n",
                m_kernel_func_ptr);

#endif // LIBFFI_FOUND
}

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
CmEmuMt_Thread::CmEmuMt_Thread(
                   CmEmu_KernelLauncher launcher,
                   CmEmuMt_Kernel* kernel)
    : m_resources(nullptr),
      m_extra_resources(nullptr),
      m_kernel_launcher(launcher),
      m_kernel(kernel)
{
    wrapper_debug();
}

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
    state(CmEmuMt_Thread::State::UNSPAWNED);
}

std::atomic<uint32_t> g_stat_current_os_threads = 0, g_stat_max_os_threads = 0;

CmEmuMt_Thread::~CmEmuMt_Thread() {}

//-----------------------------------------------------------------------------
void CmEmuMt_Thread::suspend() {
    kernel()->suspend_thread(this);
}

void CmEmuMt_Thread::resume() {
    GFX_EMU_DEBUG_MESSAGE(fSched | fDetail, "resuming thread with local idx %u\n", local_idx());

        if(m_state.load() == State::UNSPAWNED) {
            m_os_thread_ptr.reset(new std::thread(&CmEmuMt_Thread::wrapper, this));
            m_os_thread_ptr->detach ();

            g_stat_current_os_threads.fetch_add(1);
            GfxEmu::Utils::atomicUpdateMax(g_stat_max_os_threads, g_stat_current_os_threads.load ());
            GFX_EMU_DEBUG_MESSAGE(fSched | fDetail, "OS threads stat: current: %u, max: %u\n",
                g_stat_current_os_threads.load(),
                g_stat_max_os_threads.load());

            while(!suspended());
        }

    kernel()->resume_thread(this);
}

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
    GFX_EMU_MESSAGE(fSched | fDetail,
        "completing thread with local idx %u\n", local_idx());
    m_state.store(CmEmuMt_Thread::State::COMPLETED);
    g_stat_current_os_threads.fetch_sub(1, std::memory_order_relaxed);
    kernel()->complete_thread(this);
}

//-----------------------------------------------------------------------------
bool CmEmuMt_Thread::completed() {
    return m_state.load() == State::COMPLETED;
}

//-----------------------------------------------------------------------------
void CmEmuMt_Thread::wrapper_debug() {
    g_resident_thread = this;
    for (m_group_idx = 0; m_group_idx < kernel()->group_count(); m_group_idx++) {
        for (m_local_idx = 0; m_local_idx < kernel()->group_size(); m_local_idx++) {
          execute();
        }
    }
}

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

      if(GfxEmu::Cfg::ParallelThreads ().isNotDefault ()) {
        m_parallel_threads_limit = GfxEmu::Cfg::ParallelThreads ().getInt ();
      }

      if(GfxEmu::Cfg::ResidentGroups ().isNotDefault ()) {
        m_resident_groups_limit = GfxEmu::Cfg::ResidentGroups ().getInt ();
      }

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

      m_resident_groups_limit = std::min(m_group_count, m_resident_groups_limit);
}

thread_local void* CmEmuMt_Kernel::m_sched_ctx = nullptr;

CmEmuMt_Kernel::~CmEmuMt_Kernel(){
}

//-----------------------------------------------------------------------------
void CmEmuMt_Kernel::suspend_thread(CmEmuMt_Thread* thread) {
        if (thread->suspended ()) {
            GFX_EMU_ERROR_MESSAGE("trying to suspend an already suspended thread.\n");
            exit(EXIT_FAILURE);
        }

    if(!thread->suspended () && !thread->unspawned())
        m_running_threads_count.fetch_sub(1);

    thread->state(CmEmuMt_Thread::State::SUSPENDED);
    thread->bell()->wait_for_ring();
}

//-----------------------------------------------------------------------------
void CmEmuMt_Kernel::resume_thread(CmEmuMt_Thread * thread) {
    if (thread->suspended () || thread->unspawned()) {
        thread->state(CmEmuMt_Thread::State::RUNNING);
        m_running_threads_count.fetch_add(1);
        thread->bell()->ring();
    } else {
        GFX_EMU_ERROR_MESSAGE("*** Error: trying to resume not a suspended thread.\n");
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
bool CmEmuMt_Kernel::run_debug()
{
    GFX_EMU_MESSAGE(fSched, "--==--==--==--==--==--==--==--==--==--==--==--==--==--==--==--\n");
    GFX_EMU_MESSAGE(fSched, "RUNNING IN SINGLE-THREAD NON-FIBERS WORK-ITEMS SCHEDULING MODE.\n");
    GFX_EMU_MESSAGE(fSched, "NB: Kernels with synchronization will not work in this mode.\n");
    GFX_EMU_MESSAGE(fSched, "NB: Use only for debugging purposes on simple kernels!\n");
    GFX_EMU_MESSAGE(fSched, "NB: Alternative modes for debugging kernels with synchronization are:\n");
    GFX_EMU_MESSAGE(fSched, "NB: 1) CM_RT_PARALLEL_THREADS=1\n");
    GFX_EMU_MESSAGE(fSched, "NB: See README_CONFIG.md for details.\n");
    GFX_EMU_MESSAGE(fSched, "--==--==--==--==--==--==--==--==--==--==--==--==--==--==--==--\n");
    CmEmuMt_Thread { m_kernel_launcher, this };
    return true;
}

bool CmEmuMt_Kernel::run(double timeout)
{

    //-----------------------------------------------------------------
    static auto once = 0;
    if(!once++) {
        GFX_EMU_MESSAGE(fSched, "work-items scheduling mode: multi-thread work-items.\n");
        GfxEmu::Log::adviceToEnable(fSched, "for more info on scheduling.\n");
    }
    GFX_EMU_MESSAGE(fSched, "work-groups: %d\n", m_group_count);
    GFX_EMU_MESSAGE(fSched, "resident work-groups: %d\n", m_resident_groups_limit);
    GFX_EMU_MESSAGE(fSched, "work-items per work-group: %d\n", m_group_size);

    std::list<CmEmuMt_Thread> threadsList;

    for (uint32_t group_idx = 0;
        group_idx < m_resident_groups_limit;
        ++group_idx)
    {
        auto groupState1Ptr = std::make_shared<CmEmuMt_GroupState>(this);
        auto groupState2Ptr = std::make_shared<CmEmuMt_GroupState>(this);

        for (uint32_t local_idx = 0; local_idx < m_group_size; ++local_idx) {
            //m_running_threads_count++;
            threadsList.emplace_back(
                    local_idx,
                    group_idx,
                    groupState1Ptr,
                    groupState2Ptr,
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
            GFX_EMU_ERROR_MESSAGE("*** Error: timeout while running a kernel!\n");
            return true;
        }
        return false;
    };

    auto curThreadIt = threadsList.begin();
    while (threadsList.size ())
    {
        if (isTimeout ()) return false;

        if (curThreadIt->suspended() || curThreadIt->unspawned()) {
            if(m_running_threads_count.load() < m_parallel_threads_limit)
                curThreadIt->resume ();
        }

        if(m_parallel_threads_limit > 1 ||
           m_running_threads_count.load() == 0 // Guarantee sequentiality of threads spawning in parallelism = 1 mode.
        )
        {
            curThreadIt = curThreadIt->completed () ?
                threadsList.erase(curThreadIt) :
                std::next(curThreadIt);
        }

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
    g_resident_thread->simple_barrier()->signal(g_resident_thread);
}

void simple_group_barrier_wait()
{
    g_resident_thread->simple_barrier()->wait(g_resident_thread);
}

void aux_barrier_signal() { g_resident_thread->aux_barrier()->signal(g_resident_thread);}
void aux_barrier_wait() { g_resident_thread->aux_barrier()->wait(g_resident_thread);}

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
        GFX_EMU_FAIL_WITH_MESSAGE("SLM size must not be 0.\n");
    }

    const auto slm_max_size =
        CmEmuMt_SLM::kDefaultMaxSize
    ;

    size = ((int)ceil((double)size / CmEmuMt_SLM::kChunkSize)) * CmEmuMt_SLM::kChunkSize;

    if (size > slm_max_size) {
        GFX_EMU_FAIL_WITH_MESSAGE("Error in SLM Emulation:  Max SLM size = %dK!\n", slm_max_size/1024);
    }

    if (m_memory.empty()) {
        m_memory.resize(size);
    } else {
        if (m_memory.size() != size) {
            GFX_EMU_FAIL_WITH_MESSAGE("Requesting SLM size of %u while SLM size already set to %u\n",
                size, m_memory.size());
        }
    }

    m_size = size;
}

} // namespace cmrt
