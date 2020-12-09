
cmake_minimum_required(VERSION 3.10)

find_package(LibFFI REQUIRED)

  if (DEFINED LIBVA_INSTALL_PATH)
    set(LIBVA_INCLUDE_DIRS ${LIBVA_INSTALL_PATH}/include)
  else()
    include(FindPkgConfig)
    pkg_check_modules(LIBVA REQUIRED libva>=1.6.0)
  endif()

set(LIBCM_SOURCE
  cm_internal.cpp
  cm_intrin.cpp
  esimdcpu_runtime.cpp
  genx_dataport_emu.cpp
  genx_threading.cpp
  rt.cpp)

set(LIBCM_HEADERS
  ${COMMON_HEADERS}
  cm.h
  cm_color.h
  libcm_common.h
  libcm_def.h
  cm_common_macros.h
  cm_internal.h
  cm_internal_emu.h
  cm_intrin.h
  cm_list.h
  cm_mask.h
  cm_printf_base.h
  cm_printf_device.h
  cm_slm_user.h
  cm_traits.h
  cm_vm.h
  cmtl/cmtl.h
  esimdcpu_runtime.h
  esimdcpu_dataport.h
  dataport_common.h
  genx_dataport.h
  genx_simdcontrolflow.h
  genx_simdcontrolflow_emu.h
  genx_threading.h
  half_type.h
  rt.h)


add_library(libcm SHARED ${LIBCM_SOURCE})
set_target_properties(libcm PROPERTIES
  CXX_STANDARD 17
  CXX_STANDARD_REQUIRED YES
  PUBLIC_HEADER "${LIBCM_HEADERS}"
  PREFIX "")

target_compile_definitions(libcm PUBLIC CMRT_EMU)
target_compile_definitions(libcm PRIVATE NEW_LIBCM_RT LIBCM_TEST_EXPORTS)

target_include_directories(libcm BEFORE PUBLIC
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
  $<INSTALL_INTERFACE:include/libcm>)
target_include_directories(libcm BEFORE PRIVATE private)
target_include_directories(libcm PRIVATE
  ${COMMON_HEADERS_PATH}
  ${COMMON_OS_HEADERS_PATH})

target_include_directories(libcm SYSTEM PRIVATE ${LibFFI_INCLUDE_DIRS})


  target_include_directories(libcm SYSTEM PUBLIC ${LIBVA_INCLUDE_DIRS})
  target_link_libraries(libcm PRIVATE dl pthread)

target_link_libraries(libcm PRIVATE LibFFI::LibFFI)

install(TARGETS libcm
  ARCHIVE DESTINATION lib
  LIBRARY DESTINATION lib
  RUNTIME DESTINATION bin
  PUBLIC_HEADER DESTINATION include/libcm/cm
)
#  PRIVATE_HEADER DESTINATION include/libcm/)

#install(FILES ${COMMON_HEADERS} DESTINATION include/libcm)
