
cmake_minimum_required(VERSION 3.10)

project(cm-emu)

include(cmake/common.cmake)

add_subdirectory(libcm)

  add_subdirectory(libcmrt)
  if (DEFINED __SYCL_EXPLICIT_SIMD_PLUGIN__)
    message("ESIMD_EMU build : Building CM_EMU without shim-layer")
  else()
    add_subdirectory(shim-layer)
  endif()


install(FILES
    README.md
    README_CONFIG.md
    DESTINATION share/doc
)

include(cmake/package.cmake)
include(cmake/changelog.cmake)

