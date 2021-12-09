
cmake_minimum_required(VERSION 3.10)

find_package(OpenCLHeaders REQUIRED)

set(SHIM_SOURCES
  context.cpp
  device.cpp
  event.cpp
  extensions.cpp
  kernel.cpp
  memory.cpp
  platform.cpp
  program.cpp
  queue.cpp
  runtime.cpp)

add_library(shim SHARED ${SHIM_SOURCES})
set_target_properties(shim PROPERTIES CXX_STANDARD 17)

target_include_directories(shim PRIVATE ${CMAKE_SOURCE_DIR}/common
                                        ${CMAKE_SOURCE_DIR}/libcmrt)

if (MSVC)
  target_include_directories(shim PRIVATE ${CMAKE_SOURCE_DIR}/libcmrt/windows)
endif()

if (UNIX)
  target_include_directories(shim PRIVATE ${CMAKE_SOURCE_DIR}/libcmrt/linux)
endif()

target_include_directories(shim SYSTEM PUBLIC ${OpenCLHeaders_INCLUDE_DIRS})
target_link_libraries(shim PUBLIC shim_common)

if (UNIX)
  target_link_libraries(shim PRIVATE dl)
endif()
if (MSVC)
  target_link_options(shim PRIVATE /DEF:${CMAKE_CURRENT_SOURCE_DIR}/shim.def)
endif()

install(TARGETS shim
  ARCHIVE DESTINATION lib
  LIBRARY DESTINATION lib
  RUNTIME DESTINATION bin)
