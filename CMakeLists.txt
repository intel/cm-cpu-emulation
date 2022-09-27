
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

# Install License and Notices
if(WIN32)
    install(FILES
        license/NOTICES.txt
        license/windows/binary/C_for_Metal_Binary_Intel_Redist_License_Agreement_04.06.22_Final.md
        DESTINATION ${CMAKE_INSTALL_PREFIX}/lib
    )
else()
    install(FILES
        license/NOTICES.txt
        license/linux/LICENSE.md
        DESTINATION ${CMAKE_INSTALL_PREFIX}/lib/cmemu
    )
endif()

include(cmake/package.cmake)
include(cmake/changelog.cmake)

