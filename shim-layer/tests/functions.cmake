macro(ADD_KERNEL_TARGET TARGET_NAME SOURCES)
    add_library(${TARGET_NAME} MODULE ${SOURCES})
    target_include_directories(${TARGET_NAME} PRIVATE
        $<TARGET_PROPERTY:shim,INTERFACE_INCLUDE_DIRECTORIES>
    )
    if (IS64)
        target_link_libraries(${TARGET_NAME} PRIVATE
            ${LIB_CM_64}
        )
    else()
        target_link_libraries(${TARGET_NAME} PRIVATE
            ${LIB_CM_32}
        )
    endif()
endmacro()

macro(ADD_HOST_TARGET TARGET_NAME KERNEL_TARGET_NAME SOURCES)
    add_executable(${TARGET_NAME} ${SOURCES})
    target_include_directories(${TARGET_NAME} PRIVATE
        ${OpenCL_INCLUDE_DIRS}
    )
    target_compile_definitions(${TARGET_NAME} PRIVATE
        BINNAME=\"$<TARGET_FILE_NAME:${KERNEL_TARGET_NAME}>\"
    )
    if(MSVC)
        target_compile_definitions(${TARGET_NAME} PRIVATE
            _CRT_SECURE_NO_WARNINGS
        )
    endif()
    target_link_libraries(${TARGET_NAME} PRIVATE
        shim
    )  
endmacro()

macro(ADD_L0_HOST_TARGET TARGET_NAME KERNEL_TARGET_NAME SOURCES)
    add_executable(${TARGET_NAME} ${SOURCES})
    target_include_directories(${TARGET_NAME} PRIVATE
        ${L0_INCLUDE_DIRS}
    )
    target_compile_definitions(${TARGET_NAME} PRIVATE
        BINNAME=\"$<TARGET_FILE_NAME:${KERNEL_TARGET_NAME}>\"
    )
    if(MSVC)
        target_compile_definitions(${TARGET_NAME} PRIVATE
            _CRT_SECURE_NO_WARNINGS
        )
    endif()
    target_link_libraries(${TARGET_NAME} PRIVATE
        shim_l0
    )  
endmacro()
