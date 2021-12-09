/*========================== begin_copyright_notice ============================

Copyright (C) 2017 Intel Corporation

SPDX-License-Identifier: MIT

============================= end_copyright_notice ===========================*/

#ifndef GUARD_common_type_task_base_h
#define GUARD_common_type_task_base_h
//! \brief      CmTask Class to manage task parameters.
//! \details    CmTask contains one or multiple CmKernels, optional
//!             synchronization point between two consecutive kernels,
//!             and some execution parameters. CmTask is the unit to
//!             enqueue.
//!             If there is no synchronization point. kernels will run
//!             concurrently. If there is a synchronization point, kernels
//!             after the synchronization point will not start until kernels
//!             before the synchronization point finishes.
class CmTask
{
public:
    //!
    //! \brief      Add a CmKernel pointer to CmTask.
    //! \details    Same kernel can appear in the task multiple times as long
    //!             as all the value of its arguments are the same for multiple
    //!             copies of the kernel.
    //! \param      [in] pKernel
    //!             A pointer to CmKernel object.
    //! \retval     CM_SUCCESS if pKernel is added.
    //! \retval     CM_EXCEED_MAX_KERNEL_PER_ENQUEUE trying to add more kernels
    //!             than CAP_KERNEL_COUNT_PER_TASK.
    //! \retval     CM_INVALID_ARG_VALUE if pKernel is NULL.
    //!
    CM_RT_API virtual int32_t AddKernel(CmKernel *pKernel) = 0;

    //! \brief      Resets a CmTask object
    //! \details    All contents contained in CmTask get reset. Application need
    //!             add kernel, optional synchronization points, etc. again to
    //!             the CmTask. This function is to reuse CmTask for diffrent
    //!             contents so CmTask creation/destroy overhead can be avoided.

    //! \returns    CM_SUCCESS.
    //!
    CM_RT_API virtual int32_t Reset(void) = 0;

    //!
    //! \brief      Inserts a synchronization point among kernels.
    //! \details    Kernels after the synchronization point will not start
    //!             execution untill kernels before the synchronization
    //!             point finishes execution. A CmTask can have multiple
    //!             synchronization points.
    //! \returns    CM_SUCCESS.
    //!
    CM_RT_API virtual int32_t AddSync(void) = 0;

    //!
    //! \brief      Set a per-task based power option to current task.
    //! \details    The power option includes the hardware configuration of
    //!             slice, subslice and EU number. The setting takes effect
    //!             only for current task; the value needs to be set again if
    //!             user wants it to take effect for the next task.
    //! \param      [in] pCmPowerOption
    //!             A pointer to CM_POWER_OPTION struct
    //! \retval     CM_SUCCESS.
    //! \retval     CM_EXCEED_MAX_POWER_OPTION_FOR_ PLATFORM if the any of the
    //!             settings exceeds the limit of current platform
    //!             configuration.
    //!
    CM_RT_API virtual int32_t SetPowerOption( PCM_POWER_OPTION pCmPowerOption ) = 0;

    //!
    //! \brief      This API is used for the conditional end feature.
    //! \details    It adds a conditional batch buffer end command between two
    //!             kernels in a task. The pConditionalSurface + offset is the
    //!             address storing its data against the dword value provided in
    //!             pCondParam. If the data at the compare memory address is
    //!             greater than the dword set in pCondParam, the execution of
    //!             the command buffer will continue. If not, the remaining
    //!             kernels in the task will be skipped. The user can call this
    //!             API multiple times to insert multiple conditional ends
    //!             between different kernels. When opMask in pCondParam is 1,
    //!             the actual comparison value is the result of bitwise-and of
    //!             the value in memory and mask. For Gen12, the comparing
    //!             operation type and end level can be configured as well.
    //! \param      [in] pConditionalSurface
    //!             Pointer to the surface used to store comparison
    //!             dword by kernel.
    //! \param      [in] offset
    //!             The offset pointered by pSurface where stores comparison
    //!             dword value, and mask if opMask in pCondParam is set to 1.
    //! \param      [in] pCondParam
    //!             Pointer to the parameters of conditional batch buffer end
    //!             command, for Gen9/10/11, only opValue and opMask fields are
    //!             used, for Gen12+, all other fields can apply.
    //! \retval     CM_SUCCESS if successfully add a conditional batch buffer
    //!             end.
    //! \retval     CM_FAILURE otherwise.
    //!
    CM_RT_API virtual int32_t AddConditionalEnd(SurfaceIndex* pConditionalSurface, uint32_t offset, CM_CONDITIONAL_END_PARAM *pCondParam) = 0;

    //!
    //! \brief      Expose bitfield for task related property.
    //! \details    Currently this function can be used to expose the
    //!             bitfield for fused EU dispatch of Media_VFE_STATE and the
    //!             bitfield for turbo boost.
    //! \param      [out] taskConfig
    //!             specify which bitfield will be exposed.
    //! \returns    CM_SUCCESS.
    //!
    CM_RT_API virtual int32_t SetProperty(const CM_TASK_CONFIG &taskConfig) = 0;

    //! \brief      Add a CmKernel pointer with execution config to CmTask.
    //! \details    Same kernel can appear in the task multiple times as long
    //!             as all the value of its arguments are the same for multiple
    //!             copies of the kernel.
    //! \param      [in] pKernel
    //!             A pointer to CmKernel object.
    //! \retval     CM_SUCCESS if pKernel is added.
    //! \retval     CM_EXCEED_MAX_KERNEL_PER_ENQUEUE trying to add more kernels
    //!             than CAP_KERNEL_COUNT_PER_TASK.
    //! \retval     CM_INVALID_ARG_VALUE if pKernel is NULL.
    //!

    CM_RT_API virtual int32_t AddKernelWithConfig( CmKernel *pKernel, const CM_EXECUTION_CONFIG *config ) = 0;

    //! \brief      Get CM_TASK_CONFIG.
    //! \details    Currently this function can be used to get the
    //!             bitfield for turbo boost.
    //! \param      [out] taskConfig
    //!             A pointer to Cm_TASK_CONFIG object.
    //! \retval     CM_SUCCESS if CM_TASK_CONFIG info can be successfully retrieved.
    //! \retval     CM_FAILURE if the pointer is invalid or info cannot be retrieved.
    //!
    CM_RT_API virtual int32_t GetProperty(CM_TASK_CONFIG &taskConfig) = 0;

    //! \brief      Set a per-kernel based power option to current kernel with dataCacheFlush flag or etc.
    //! \details    this function can be inserted after kernel added so that pipe_control will be inserted after the
    //!             walker with dataCacheFlush flag.
    //! \param      [in] taskConfig
    //!             A pointer to CM_KERNEL_SYNC_CONFIG object.
    //! \retval     CM_SUCCESS if CM_KERNEL_SYNC_CONFIG info can be successfully stored in CM_TASK.
    //! \retval     CM_FAILURE if the pointer is invalid.
    //!
    CM_RT_API virtual int32_t AddSyncEx(const CM_KERNEL_SYNC_CONFIG *config) = 0;

    virtual ~CmTask () = default;
};
#endif // GUARD_common_type_task_base_h
