
cmake_minimum_required(VERSION 3.10)

find_package(LevelZero REQUIRED)

set(ZE_SHIM_SOURCES
  cmdlist.cpp
  context.cpp
  ddi.cpp
  device.cpp
  driver.cpp
  event.cpp
  fence.cpp
  image.cpp
  memory.cpp
  module.cpp
  queue.cpp
  sampler.cpp)

add_library(ze_shim SHARED ${ZE_SHIM_SOURCES})
set_target_properties(ze_shim PROPERTIES
  CXX_STANDARD 17
  CXX_STANDARD_REQUIRED ON
  OUTPUT_NAME shim_l0
  VERSION 1.0 # LevelZero API version
  SOVERSION 1)

target_include_directories(ze_shim PRIVATE ${LevelZero_INCLUDE_DIRS}
                                           ${CMAKE_SOURCE_DIR}/common
                                           ${CMAKE_SOURCE_DIR}/libcmrt)

target_include_directories(ze_shim SYSTEM PUBLIC ${OpenCLHeaders_INCLUDE_DIRS})
target_link_libraries(ze_shim PUBLIC shim_common)

if (WIN32)
  target_compile_definitions(ze_shim PRIVATE NOMINMAX)
  target_include_directories(ze_shim PRIVATE ${CMAKE_SOURCE_DIR}/libcmrt/windows)
  target_link_options(ze_shim PRIVATE /DEF:${CMAKE_CURRENT_SOURCE_DIR}/ze_shim.def)
else()
  target_include_directories(ze_shim PRIVATE ${CMAKE_SOURCE_DIR}/libcmrt/linux)
  target_link_libraries(ze_shim PRIVATE dl)
endif()

install(TARGETS ze_shim
  ARCHIVE DESTINATION lib
  LIBRARY DESTINATION lib
  RUNTIME DESTINATION bin)
