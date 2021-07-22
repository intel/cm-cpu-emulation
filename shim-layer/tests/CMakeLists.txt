cmake_minimum_required(VERSION 3.15)

include(functions.cmake)

message(STATUS "Building tests.")

set(TESTS_DIR
    ${CMAKE_CURRENT_LIST_DIR}
)


    set(TESTS
        #xehpsdvbgemm
        vadd_hello_ocl
        vadd_hello_l0
    )

foreach(TEST ${TESTS})
    if (NOT IS_DIRECTORY ${TESTS_DIR}/${TEST})
        message(WARNING "Wrong test directory name in the test list: ${TEST}") 
        list(REMOVE_ITEM TESTS ${TEST})
    endif()
endforeach()

message(STATUS "The following tests have been found:")
foreach(TEST ${TESTS})
    if (${TEST} MATCHES "_l0")
        set(TYPE "L0")
    else()
        set(TYPE "OCL")
    endif()
    message(STATUS "${TEST}: ${TYPE}")

    file(GLOB ALL_SOURCES "${TESTS_DIR}/${TEST}/*.cpp")
    set(KERNEL_CODE ${ALL_SOURCES})
    list(FILTER KERNEL_CODE INCLUDE REGEX "_genx.cpp")
    set(HOST_CODE ${ALL_SOURCES})
    list(FILTER HOST_CODE EXCLUDE REGEX "_genx.cpp")

    set(KERNEL_TARGET_NAME "${TEST}.kernel")
    set(HOST_TARGET_NAME "${TEST}.host")
    add_kernel_target("${KERNEL_TARGET_NAME}" ${KERNEL_CODE})
    if (TYPE STREQUAL "L0")
        add_l0_host_target(${HOST_TARGET_NAME} ${KERNEL_TARGET_NAME} ${HOST_CODE})
    else()
        add_host_target(${HOST_TARGET_NAME} ${KERNEL_TARGET_NAME} ${HOST_CODE})
    endif()
    add_dependencies(${HOST_TARGET_NAME} ${KERNEL_TARGET_NAME})

    #install(TARGETS ${HOST_TARGET_NAME} DESTINATION tests)
    #install(TARGETS ${KERNEL_TARGET_NAME} DESTINATION tests)
endforeach()
