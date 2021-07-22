
cmake_minimum_required(VERSION 3.10)

set(SHIM_COMMON_SOURCE
  shim.cpp
)

set(SHIM_COMMON_PRIVATE_HEADERS
  shim.h)

set(SHIM_COMMON_HEADERS
  ${COMMON_HEADERS_PATH}/os_utils.h
  ${COMMON_HEADERS_PATH}/kernel_utils.h
  ${COMMON_HEADERS_PATH}/shim_os_defines.h
  ${COMMON_HEADERS_PATH}/shim_support.h)

add_library(shim_common STATIC
  ${SHIM_COMMON_SOURCE}
  ${SHIM_COMMON_HEADERS}
  ${SHIM_COMMON_PRIVATE_HEADERS})
set_target_properties(shim_common PROPERTIES
  CXX_STANDARD 17
  POSITION_INDEPENDENT_CODE ON)
target_include_directories(shim_common PUBLIC
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
  $<INSTALL_INTERFACE:include/shim>)
target_link_libraries(shim_common PUBLIC igfxcmrt)

install(FILES ${SHIM_COMMON_HEADERS} DESTINATION include/shim)

add_subdirectory(ocl)
add_subdirectory(l0)
