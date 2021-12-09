
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
  cm_surface_emumode.cpp
  cm_surface_manager_emumode.cpp
  cm_task_emumode.cpp
  cm_thread_space_emumode.cpp)


if (WIN32)
  set(LIBCMRT_OS_HEADERS_PATH ${CMAKE_CURRENT_SOURCE_DIR}/windows)
else()
  set(LIBCMRT_OS_HEADERS_PATH ${CMAKE_CURRENT_SOURCE_DIR}/linux)
endif()

# Library name
if (WIN32)
  if (CMAKE_SIZEOF_VOID_P EQUAL 8)
    set(_name igfxcmrt64_emu)
    set(_name11 igfx11cmrt64_emu)
  else()
    set(_name igfxcmrt32_emu)
    set(_name11 igfx11cmrt32_emu)
  endif()
else()
  set(_name igfxcmrt_emu)
endif()

add_library(igfxcmrt SHARED ${CMRT_SOURCE})
set_target_properties(igfxcmrt PROPERTIES
  CXX_STANDARD ${_cxx}
  CXX_STANDARD_REQUIRED ON
  CXX_VISIBILITY_PRESET default
  OUTPUT_NAME ${_name}
  PUBLIC_HEADER "${COMMON_HEADERS}")

if (WIN32)
  target_compile_definitions(igfxcmrt PUBLIC CM_DX9)
endif()

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
## target_link_libraries(igfxcmrt PRIVATE emu_common)

install(TARGETS igfxcmrt
  ARCHIVE DESTINATION lib
  LIBRARY DESTINATION lib
  RUNTIME DESTINATION bin
  PUBLIC_HEADER DESTINATION include/igfxcmrt_emu)

if (WIN32) # DX11
  add_library(igfx11cmrt SHARED ${CMRT_SOURCE})
  set_target_properties(igfx11cmrt PROPERTIES
    CXX_STANDARD ${_cxx}
    CXX_STANDARD_REQUIRED ON
    CXX_VISIBILITY_PRESET default
    OUTPUT_NAME ${_name11})

  target_compile_definitions(igfx11cmrt PRIVATE CM_RT_EXPORTS)
  target_compile_definitions(igfx11cmrt PUBLIC CMRT CM_DX11)

  target_include_directories(igfx11cmrt 
    PRIVATE
      $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
      $<BUILD_INTERFACE:${LIBCMRT_OS_HEADERS_PATH}>
    PUBLIC
        $<BUILD_INTERFACE:${COMMON_HEADERS_PATH}>
        $<BUILD_INTERFACE:${COMMON_OS_HEADERS_PATH}>
        $<INSTALL_INTERFACE:include/igfxcmrt_emu>)

  target_link_libraries(igfx11cmrt PUBLIC libcm)
  ## target_link_libraries(igfx11cmrt PRIVATE emu_common)

  install(TARGETS igfx11cmrt
    ARCHIVE DESTINATION lib
    LIBRARY DESTINATION lib
    RUNTIME DESTINATION bin)
endif()
