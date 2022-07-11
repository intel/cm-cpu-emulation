
project(libcm)

include(${CMAKE_CURRENT_SOURCE_DIR}/../cmake/common.cmake)

cmake_minimum_required(VERSION 3.10)


find_package(LibFFI REQUIRED)



set(LIBCM_SOURCE
  # NB: this shall go to a separate emu_common dll.
  ${COMMON_SRC_PATH}/emu_cfg.cpp
  ${COMMON_SRC_PATH}/emu_cfg_platform.cpp
  ${COMMON_SRC_PATH}/emu_utils.cpp
  ${COMMON_SRC_PATH}/emu_log.cpp
  ${COMMON_SRC_PATH}/emu_kernel_support.cpp
  ${COMMON_SRC_PATH}/emu_kernel_support_types.cpp

  ${COMMON_SRC_PATH}/os_utils.cpp
  ${COMMON_SRC_PATH}/kernel_utils.cpp

  cm_internal.cpp
  cm_intrin.cpp
  esimdemu_support.cpp
  genx_dataport_emu.cpp
  genx_threading.cpp
  rt.cpp)

set(LIBCM_HEADERS
  ${COMMON_HEADERS}
  cm.h
  cm_color.h
  libcm_common.h
  libcm_def.h
  cm_has_instr.h
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
  esimdemu_support.h
  genx_dataport.h
  genx_simdcontrolflow.h
  genx_simdcontrolflow_emu.h
  genx_threading.h
  half_type.h
  rt.h)


add_library(libcm SHARED ${LIBCM_SOURCE})
set_target_properties(libcm PROPERTIES
  CXX_STANDARD 17
  LINK_OPTIONS "$<$<CXX_COMPILER_ID:MSVC>:/SAFESEH:NO>"
  CXX_STANDARD_REQUIRED YES
  PUBLIC_HEADER "${LIBCM_HEADERS}"
  PREFIX "")

#set(CMAKE_EXE_LINKER_FLAGS_DEBUG  "${CMAKE_EXE_LINKER_FLAGS_DEBUG} /SAFESEH:NO")
#set(CMAKE_SHARED_LINKER_FLAGS_DEBUG  "${CMAKE_EXE_LINKER_FLAGS_DEBUG} /SAFESEH:NO")

target_compile_definitions(libcm PUBLIC CMRT_EMU)
target_compile_definitions(libcm PRIVATE NEW_LIBCM_RT LIBCM_TEST_EXPORTS)

target_include_directories(libcm BEFORE PUBLIC
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
  $<INSTALL_INTERFACE:include/libcm>)
target_include_directories(libcm BEFORE PRIVATE private)
target_include_directories(libcm PRIVATE
  ${COMMON_HEADERS_PATH}
  ${COMMON_OS_HEADERS_PATH})
  

if(UNIX AND 
    CMAKE_COMPILER_IS_GNUCC AND
        "${CMAKE_CXX_COMPILER_VERSION}" VERSION_LESS "9.0.0")
    target_link_libraries(libcm PRIVATE
            stdc++fs 
        )
endif()


if(NOT WIN32)
  target_link_libraries(libcm PRIVATE dl pthread)
endif()


target_link_libraries(libcm PRIVATE LibFFI::LibFFI)



if(DBGSYMB_PROVIDER_LIB_FOUND)
  target_link_libraries(libcm PRIVATE DbgSymb::Provider)
endif ()

install(TARGETS libcm
  ARCHIVE DESTINATION lib
  LIBRARY DESTINATION lib
  RUNTIME DESTINATION bin
  PUBLIC_HEADER DESTINATION include/libcm/cm
)
#  PRIVATE_HEADER DESTINATION include/libcm/)

#install(FILES ${COMMON_HEADERS} DESTINATION include/libcm)
