
include(FindPackageHandleStandardArgs)

find_path(LibFFI_INCLUDE_DIR
  NAMES ffi.h
  PATHS "${LibFFI_INSTALL_PATH}"
  PATH_SUFFIXES include include/ffi)

if (CMAKE_SIZEOF_VOID_P EQUAL 8)
  set(_lib lib lib64 lib/x86_64-linux-gnu lib/x64 Release)
else()
  set(_lib lib lib32 lib/i386-linux-gnu lib/x86 Release)
endif()

find_library(LibFFI_LIBRARY
  NAMES ffi libffi
  PATHS "${LibFFI_INSTALL_PATH}"
  PATH_SUFFIXES ${_lib})

find_package_handle_standard_args(LibFFI
  REQUIRED_VARS LibFFI_LIBRARY LibFFI_INCLUDE_DIR
  HANDLE_COMPONENTS)
mark_as_advanced(LibFFI_LIBRARY LibFFI_INCLUDE_DIR)

if(LibFFI_FOUND)
  add_definitions(-DLIBFFI_FOUND)
  list(APPEND LibFFI_LIBRARIES ${LibFFI_LIBRARY} ${CMAKE_DL_LIBS})
  list(APPEND LibFFI_INCLUDE_DIRS ${LibFFI_INCLUDE_DIR})
endif()

if(LibFFI_FOUND AND NOT TARGET LibFFI::LibFFI)
  add_library(LibFFI::LibFFI INTERFACE IMPORTED)
  set_target_properties(LibFFI::LibFFI PROPERTIES
    INTERFACE_LINK_LIBRARIES "${LibFFI_LIBRARIES}")
  set_target_properties(LibFFI::LibFFI PROPERTIES
    INTERFACE_INCLUDE_DIRECTORIES "${LibFFI_INCLUDE_DIRS}")
endif()

