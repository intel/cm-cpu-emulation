if(NOT DEFINED MDF_RELEASE_PATH)
    message(FATAL_ERROR
        "Path to a valid MDF release should be defined"
        " in the MDF_RELEASE_PATH variable. Use cmake -DMDF_RELEASE_PATH=Path")
endif()

# paths to external components
set(CMRT_INCLUDE_DIR "${MDF_RELEASE_PATH}/runtime/include")
set(CMC_INCLUDE_DIR "${MDF_RELEASE_PATH}/compiler/include_icl")

set(CMRT_LIB64_DIR "${MDF_RELEASE_PATH}/runtime/lib/x64")
set(CMC_LIB64_DIR "${MDF_RELEASE_PATH}/compiler/lib/x64")
set(CMRT_LIB32_DIR "${MDF_RELEASE_PATH}/runtime/lib/x86")
set(CMC_LIB32_DIR "${MDF_RELEASE_PATH}/compiler/lib/x86")
if(IS64)
    set(CMRT_LIB_DIR ${CMRT_LIB64_DIR})
    set(CMC_LIB_DIR ${CMC_LIB64_DIR})
else()
    set(CMRT_LIB_DIR ${CMRT_LIB32_DIR})
    set(CMC_LIB_DIR ${CMC_LIB32_DIR})
endif()

# imported library definitions
set(LIB_CM_64 libcm64)
add_library(${LIB_CM_64} SHARED IMPORTED GLOBAL)
    set_target_properties(${LIB_CM_64} PROPERTIES
        IMPORTED_LOCATION ${CMC_LIB64_DIR}/libcm.so
    )

target_include_directories(${LIB_CM_64} INTERFACE
    ${CMC_INCLUDE_DIR}
)
target_compile_definitions(${LIB_CM_64} INTERFACE
    CMRT_EMU=1
    gen12
)

set(LIB_CM_32 libcm32)
add_library(${LIB_CM_32} SHARED IMPORTED GLOBAL)
    set_target_properties(${LIB_CM_32} PROPERTIES
        IMPORTED_LOCATION ${CMC_LIB32_DIR}/libcm.so
    )

target_include_directories(${LIB_CM_32} INTERFACE
    ${CMC_INCLUDE_DIR}
)
target_compile_definitions(${LIB_CM_32} INTERFACE
    CMRT_EMU=1
    gen12
)


set(LIB_CMEMU_64 igfx11cmrt64_emu)
add_library(${LIB_CMEMU_64} SHARED IMPORTED GLOBAL)
    set_target_properties(${LIB_CMEMU_64} PROPERTIES
        IMPORTED_LOCATION ${CMRT_LIB64_DIR}/libigfxcmrt_emu.so
    )
target_include_directories(${LIB_CMEMU_64} INTERFACE
    ${CMRT_INCLUDE_DIR}
)
target_link_libraries(${LIB_CMEMU_64} INTERFACE
    ${LIB_CM_64}
)
target_compile_definitions(${LIB_CMEMU_64} INTERFACE
    CMRT_EMU=1
    gen12
)

set(LIB_CMEMU_32 igfxcmrt_emu)
add_library(${LIB_CMEMU_32} SHARED IMPORTED GLOBAL)
    set_target_properties(${LIB_CMEMU_32} PROPERTIES
        IMPORTED_LOCATION ${CMRT_LIB32_DIR}/libigfxcmrt_emu.so
     )
target_include_directories(${LIB_CMEMU_32} INTERFACE
    ${CMRT_INCLUDE_DIR}
)
if (UNIX)
    target_include_directories(${LIB_CMEMU_32} INTERFACE
        ${LIBVA_INCLUDE_DIRS}
    )
endif()
target_link_libraries(${LIB_CMEMU_32} INTERFACE
    ${LIB_CM_32}
)
target_compile_definitions(${LIB_CMEMU_32} INTERFACE
    CMRT_EMU=1
    gen12
)

