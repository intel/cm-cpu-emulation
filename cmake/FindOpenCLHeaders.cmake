
include(FindPackageHandleStandardArgs)

find_path(
  OpenCLHeaders_INCLUDE_DIR
  NAMES CL/cl_icd.h
  PATHS ${OPENCL_HEADERS_PATH}
  PATH_SUFFIXES "" "include")

find_package_handle_standard_args(
  OpenCLHeaders
  REQUIRED_VARS OpenCLHeaders_INCLUDE_DIR
  HANDLE_COMPONENTS)
mark_as_advanced(OpenCLHeaders_INCLUDE_DIR)

if(OpenCLHeaders_FOUND)
  list(APPEND OpenCLHeaders_INCLUDE_DIRS ${OpenCLHeaders_INCLUDE_DIR})
endif()
