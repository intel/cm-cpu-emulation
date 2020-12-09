
cmake_minimum_required(VERSION 3.10)

project(cm-emu)

list(APPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake)


set(COMMON_HEADERS_PATH ${CMAKE_CURRENT_SOURCE_DIR}/common)


  set(COMMON_OS_HEADERS_PATH ${CMAKE_CURRENT_SOURCE_DIR}/common/linux)

include(cmake/common_headers.cmake)


add_subdirectory(libcm)
add_subdirectory(libcmrt)
add_subdirectory(shim-layer)

include(cmake/package.cmake)

