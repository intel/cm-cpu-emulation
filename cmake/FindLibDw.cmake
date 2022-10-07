find_path(LibDw_INCLUDE_DIR elfutils/libdw.h
  HINTS ${LibDw_PREFIX}/include
)

find_library(LibDw_LIBRARY 
  NAMES dw libdw
  HINTS ${LibDw_PREFIX}/lib
)

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(LibDw DEFAULT_MSG
  LibDw_LIBRARY
  LibDw_INCLUDE_DIR
)

mark_as_advanced(
  LibDw_INCLUDE_DIR
  LibDw_LIBRARY
)

if(LibDw_FOUND AND NOT TARGET LibDw::LibDw)
  add_library(LibDw::LibDw INTERFACE IMPORTED)
  set_target_properties(LibDw::LibDw PROPERTIES
    INTERFACE_LINK_LIBRARIES "${LibDw_LIBRARIES}"
    INTERFACE_INCLUDE_DIRECTORIES "${LibDw_INCLUDE_DIRS}")
endif()
