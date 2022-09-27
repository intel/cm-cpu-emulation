
####################################################


####################################################


####################################################

list(APPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_LIST_DIR}/../cmake)

####################################################

set(COMMON_HEADERS_PATH ${CMAKE_CURRENT_LIST_DIR}/../common)
set(COMMON_SRC_PATH ${CMAKE_CURRENT_LIST_DIR}/../common)

####################################################

if (WIN32)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /MP")
endif()

####################################################

if (WIN32)
  set(COMMON_OS_HEADERS_PATH ${COMMON_HEADERS_PATH}/windows)
  set(COMMON_OS_SRC_PATH ${COMMON_SRC_PATH}/windows)
else()
  set(COMMON_OS_HEADERS_PATH ${COMMON_HEADERS_PATH}/linux)
  set(COMMON_OS_SRC_PATH ${COMMON_SRC_PATH}/linux)
endif()

# These headers are the accumulated set of those
# interused between libcm, libcmrt and shim-layer.
set(COMMON_HEADERS
  ${COMMON_HEADERS_PATH}/va_stub.h

  ${COMMON_HEADERS_PATH}/kernel_utils.h
  ${COMMON_HEADERS_PATH}/os_utils.h
  
  ${COMMON_HEADERS_PATH}/emu_dbgsymb_types.h
  ${COMMON_OS_HEADERS_PATH}/emu_dbgsymb_os.h
  ${COMMON_HEADERS_PATH}/emu_kernel_support.h
  ${COMMON_HEADERS_PATH}/emu_kernel_support_types.h

  ${COMMON_HEADERS_PATH}/emu_log_flags.h
  ${COMMON_HEADERS_PATH}/emu_log.h

  ${COMMON_HEADERS_PATH}/emu_platform.h


  ${COMMON_HEADERS_PATH}/emu_cfg.h
  ${COMMON_HEADERS_PATH}/emu_cfg_params.h
  ${COMMON_HEADERS_PATH}/emu_cfg_platform.h

  ${COMMON_HEADERS_PATH}/emu_api_export.h

  ${COMMON_HEADERS_PATH}/cm_version_defs.h
  ${COMMON_HEADERS_PATH}/cm_index_types.h

  ${COMMON_HEADERS_PATH}/emu_utils.h
  ${COMMON_HEADERS_PATH}/cm_rt.h

  ${COMMON_HEADERS_PATH}/emu_kernel_arg.h
  ${COMMON_HEADERS_PATH}/type_frame.h
  ${COMMON_HEADERS_PATH}/type_gpu_platform.h
  ${COMMON_HEADERS_PATH}/type_large_integer.h
  ${COMMON_HEADERS_PATH}/type_memory_type.h
  ${COMMON_HEADERS_PATH}/type_queue_priority.h
  ${COMMON_HEADERS_PATH}/type_queue_create_option.h
  ${COMMON_HEADERS_PATH}/type_task_config.h
  ${COMMON_HEADERS_PATH}/type_return_code.h
  ${COMMON_HEADERS_PATH}/type_buffer_base.h
  ${COMMON_HEADERS_PATH}/type_buffer_state_param.h
  ${COMMON_HEADERS_PATH}/type_buffer_stateless_base.h
  ${COMMON_HEADERS_PATH}/type_buffer_svm_base.h
  ${COMMON_HEADERS_PATH}/type_buffer_up_base.h
  ${COMMON_HEADERS_PATH}/type_cond_end_operator_code.h
  ${COMMON_HEADERS_PATH}/type_cond_end_param.h
  ${COMMON_HEADERS_PATH}/type_event_base.h
  ${COMMON_HEADERS_PATH}/type_exec_config.h
  ${COMMON_HEADERS_PATH}/type_kernel_base.h
  ${COMMON_HEADERS_PATH}/type_kernel_exec_mode.h
  ${COMMON_HEADERS_PATH}/type_kernel_sync_config.h
  ${COMMON_HEADERS_PATH}/type_power_option.h
  ${COMMON_HEADERS_PATH}/type_queue_base.h
  ${COMMON_HEADERS_PATH}/type_queue_type.h
  ${COMMON_HEADERS_PATH}/type_surface_2d_state_param.h
  ${COMMON_HEADERS_PATH}/type_surface_2d_stateless_base.h
  ${COMMON_HEADERS_PATH}/type_surface_2d_up_base.h
  ${COMMON_HEADERS_PATH}/type_surface_3d_base.h
  ${COMMON_HEADERS_PATH}/type_surface_details.h
  ${COMMON_HEADERS_PATH}/type_task_base.h
  ${COMMON_HEADERS_PATH}/type_thread_space_base.h
  ${COMMON_HEADERS_PATH}/type_dll_file_version.h
  ${COMMON_HEADERS_PATH}/type_26zi_dispatch_pattern.h
  ${COMMON_HEADERS_PATH}/type_coord.h
  ${COMMON_HEADERS_PATH}/type_dependency.h
  ${COMMON_HEADERS_PATH}/type_dependency_pattern.h
  ${COMMON_HEADERS_PATH}/type_device_cap_name.h
  ${COMMON_HEADERS_PATH}/type_event_profiling_info.h
  ${COMMON_HEADERS_PATH}/type_fastcopy_option.h
  ${COMMON_HEADERS_PATH}/type_l3_config_register_values.h
  ${COMMON_HEADERS_PATH}/type_l3_suggest_config.h
  ${COMMON_HEADERS_PATH}/type_message_sequence.h
  ${COMMON_HEADERS_PATH}/type_min_max_filter_ctrl.h
  ${COMMON_HEADERS_PATH}/type_mw_group_select.h
  ${COMMON_HEADERS_PATH}/type_pixel_type.h
  ${COMMON_HEADERS_PATH}/type_platform_info.h
  ${COMMON_HEADERS_PATH}/type_queue_sseu_usage_hint_type.h
  ${COMMON_HEADERS_PATH}/type_surface_address_control_mode.h
  ${COMMON_HEADERS_PATH}/type_status.h
  ${COMMON_HEADERS_PATH}/type_thread_param.h
  ${COMMON_HEADERS_PATH}/type_walking_pattern.h
  ${COMMON_HEADERS_PATH}/type_walking_patterns.h
  ${COMMON_HEADERS_PATH}/type_device_base.h
  ${COMMON_HEADERS_PATH}/type_surface_2d_base.h

  ${COMMON_HEADERS_PATH}/cm_version_defs.h

  ${COMMON_OS_HEADERS_PATH}/cm_include.h
  ${COMMON_OS_HEADERS_PATH}/cm_rt_def_os.h
  ${COMMON_OS_HEADERS_PATH}/cm_rt_api_os.h)

####################################################

include(${CMAKE_CURRENT_LIST_DIR}/debug_symbols_access.cmake)

####################################################



