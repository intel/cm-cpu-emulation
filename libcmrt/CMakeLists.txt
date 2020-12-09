
cmake_minimum_required(VERSION 3.10)

# Workaround for std::byte vs WinSDK byte conflict
add_definitions(-D_HAS_STD_BYTE=0)
set(_cxx 17)

set(CMRT_SOURCE
  cm_array.cpp
  cm_buffer_emumode.cpp
  cm_device_emumode.cpp
  cm_device_export_emumode.cpp
  cm_event_emumode.cpp
  cm_group_space_emumode.cpp
  cm_kernel_emumode.cpp
  cm_memory_object_control.cpp
  cm_program_emumode.cpp
  cm_queue_emumode.cpp
  cm_rt_helpers.cpp
  cm_statistics.cpp
  cm_surface_2d_emumode.cpp
  cm_surface_3d_emumode.cpp
  cm_surface_alias_table.cpp
  cm_surface_emumode.cpp
  cm_surface_manager_emumode.cpp
  cm_task_emumode.cpp
  cm_thread_space_emumode.cpp)


  set(LIBCMRT_OS_HEADERS_PATH ${CMAKE_CURRENT_SOURCE_DIR}/linux)

# Library name
  set(_name igfxcmrt_emu)

add_library(igfxcmrt SHARED ${CMRT_SOURCE})
set_target_properties(igfxcmrt PROPERTIES
  CXX_STANDARD ${_cxx}
  CXX_STANDARD_REQUIRED ON
  CXX_VISIBILITY_PRESET hidden
  OUTPUT_NAME ${_name}
  PUBLIC_HEADER "${COMMON_HEADERS}")


target_compile_definitions(igfxcmrt PRIVATE CM_RT_EXPORTS)
target_compile_definitions(igfxcmrt PUBLIC CMRT)

target_include_directories(igfxcmrt 
    PRIVATE
      $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
      $<BUILD_INTERFACE:${LIBCMRT_OS_HEADERS_PATH}>
    PUBLIC
      $<BUILD_INTERFACE:${COMMON_HEADERS_PATH}>
      $<BUILD_INTERFACE:${COMMON_OS_HEADERS_PATH}>
      $<INSTALL_INTERFACE:include/igfxcmrt_emu>)

target_link_libraries(igfxcmrt PUBLIC libcm)

install(TARGETS igfxcmrt
  ARCHIVE DESTINATION lib
  LIBRARY DESTINATION lib
  RUNTIME DESTINATION bin
  PUBLIC_HEADER DESTINATION include/igfxcmrt_emu)

