
cmake_minimum_required(VERSION 3.10)

set(COMMON_SOURCE
  shim.cpp
  os_utils.cpp
  kernel_utils.cpp)

set(COMMON_PRIVATE_HEADERS
  kernel_utils.h
  os_utils.h
  shim.h)

set(COMMON_HEADERS
  os_defines.h
  shim_support.h)

add_library(shim_common STATIC
  ${COMMON_SOURCE}
  ${COMMON_HEADERS}
  ${COMMON_PRIVATE_HEADERS})
set_target_properties(shim_common PROPERTIES
  CXX_STANDARD 17
  POSITION_INDEPENDENT_CODE ON)
target_include_directories(shim_common PUBLIC
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
  $<INSTALL_INTERFACE:include/shim>)
target_link_libraries(shim_common PUBLIC igfxcmrt)

install(FILES ${COMMON_HEADERS} DESTINATION include/shim)

add_subdirectory(ocl)
add_subdirectory(l0)
