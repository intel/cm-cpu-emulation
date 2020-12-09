
cmake_minimum_required(VERSION 3.10)

find_package(LevelZero REQUIRED)

set(SHIM_SOURCES l0_mapping.cpp)
    
add_library(shim_l0 SHARED ${SHIM_SOURCES})
set_target_properties(shim_l0 PROPERTIES CXX_STANDARD 17)

target_include_directories(shim_l0 SYSTEM PRIVATE ${LevelZero_INCLUDE_DIRS})
target_link_libraries(shim_l0 PUBLIC shim_common)

if (UNIX)
  target_link_libraries(shim_l0 PRIVATE dl)
    endif()
if (MSVC)
  target_link_options(shim_l0 PRIVATE /DEF:${CMAKE_CURRENT_SOURCE_DIR}/shim.def)
endif()

install(TARGETS shim_l0
  ARCHIVE DESTINATION lib
  LIBRARY DESTINATION lib
  RUNTIME DESTINATION bin)
