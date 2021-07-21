
cmake_minimum_required(VERSION 3.10)

project(cm-emu)

list(APPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake)


set(COMMON_HEADERS_PATH ${CMAKE_CURRENT_SOURCE_DIR}/common)
set(COMMON_SRC_PATH ${CMAKE_CURRENT_SOURCE_DIR}/common)


  set(COMMON_OS_HEADERS_PATH ${CMAKE_CURRENT_SOURCE_DIR}/common/linux)
  set(COMMON_OS_SRC_PATH ${CMAKE_CURRENT_SOURCE_DIR}/common/linux)


include(cmake/common.cmake)

add_subdirectory(libcm)
add_subdirectory(libcmrt)
if (DEFINED __SYCL_EXPLICIT_SIMD_PLUGIN__)
  message("ESIMD_CPU build : Building CM_EMU without shim-layer")
else()
  add_subdirectory(shim-layer)
endif()

include(cmake/package.cmake)

