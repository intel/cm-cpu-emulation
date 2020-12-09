
include(FindPackageHandleStandardArgs)

if (DEFINED LevelZero_INSTALL_PATH)
  find_path(LevelZero_INCLUDE_DIR
    NAMES level_zero/ze_api.h
    PATHS "${LevelZero_INSTALL_PATH}"
    PATH_SUFFIXES include
    NO_DEFAULT_PATH)

  find_library(LevelZero_LIBRARY
    NAMES ze_loader ze_loader32 ze_loader64
    PATHS "${LevelZero_INSTALL_PATH}"
    PATH_SUFFIXES lib lib64 lib/x86_64-linux-gnu lib/x86 lib/x86
    NO_DEFAULT_PATH)
else()
  find_path(LevelZero_INCLUDE_DIR
    NAMES level_zero/ze_api.h
    PATH_SUFFIXES include)

  find_library(LevelZero_LIBRARY
    NAMES ze_loader ze_loader32 ze_loader64
    PATH_SUFFIXES lib lib64 lib/x86_64-linux-gnu lib/x86 lib/x86)
endif()

find_package_handle_standard_args(LevelZero
  REQUIRED_VARS
    LevelZero_LIBRARY
    LevelZero_INCLUDE_DIR
  HANDLE_COMPONENTS)
mark_as_advanced(LevelZero_LIBRARY LevelZero_INCLUDE_DIR)

if(LevelZero_FOUND)
    list(APPEND LevelZero_LIBRARIES ${LevelZero_LIBRARY} ${CMAKE_DL_LIBS})
    list(APPEND LevelZero_INCLUDE_DIRS ${LevelZero_INCLUDE_DIR})
    if(OpenCL_FOUND)
        list(APPEND LevelZero_INCLUDE_DIRS ${OpenCL_INCLUDE_DIRS})
    endif()
endif()

if(LevelZero_FOUND AND NOT TARGET LevelZero::LevelZero)
    add_library(LevelZero::LevelZero INTERFACE IMPORTED)
    set_target_properties(LevelZero::LevelZero
      PROPERTIES INTERFACE_LINK_LIBRARIES "${LevelZero_LIBRARIES}")
    set_target_properties(LevelZero::LevelZero
      PROPERTIES INTERFACE_INCLUDE_DIRECTORIES "${LevelZero_INCLUDE_DIRS}")
endif()
