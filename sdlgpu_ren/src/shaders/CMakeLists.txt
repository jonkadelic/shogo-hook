find_program(SLANG_COMPILER NAMES slangc REQUIRED)
find_program(SPIRV_VAL NAMES spirv-val OPTIONAL)

file(GLOB_RECURSE SHADER_FILES CONFIGURE_DEPENDS
    "${CMAKE_CURRENT_SOURCE_DIR}/*.slang"
)

set(ALL_SPV_OUTPUTS "")
set(C_FILE "${CMAKE_CURRENT_BINARY_DIR}/compiled_shaders.c")
set(H_FILE "${CMAKE_CURRENT_BINARY_DIR}/compiled_shaders.h")

set(C_CONTENT "#include <stdint.h>\n#include <stddef.h>\n#include <stdalign.h>\n\n")
set(H_CONTENT "#pragma once\n\n#include <stdint.h>\n#include <stddef.h>\n\n")

foreach(SHADER_FILE ${SHADER_FILES})
    file(RELATIVE_PATH REL_PATH "${CMAKE_CURRENT_SOURCE_DIR}" "${SHADER_FILE}")
    set(SPV_FILE "${CMAKE_CURRENT_BINARY_DIR}/${REL_PATH}.spv")

    get_filename_component(SPV_DIR "${SPV_FILE}" DIRECTORY)
    file(MAKE_DIRECTORY "${SPV_DIR}")

    add_custom_command(
        OUTPUT "${SPV_FILE}"
        COMMAND "${SLANG_COMPILER}" "${SHADER_FILE}" -entry vertex_main -stage vertex -entry fragment_main -stage fragment -target spirv -o "${SPV_FILE}"
        DEPENDS "${SHADER_FILE}"
        COMMENT "Compiling Slang shader ${REL_PATH} to SPIR-V"
        VERBATIM
    )
    if(${SPIRV_VAL})
        add_custom_command(
            COMMAND "${SPIRV_VAL}" "${SPV_FILE}"
            DEPENDS "${SPV_FILE}"
            COMMENT "Validating compiled SPIR-V shader"
            VERBATIM
        )
    endif()

    list(APPEND ALL_SPV_OUTPUTS "${SPV_FILE}")

    string(REGEX REPLACE "[^a-zA-Z0-9]" "_" SYM_NAME "${REL_PATH}")

    string(APPEND C_CONTENT "/* Generated from ${REL_PATH} */\n")
    string(APPEND C_CONTENT "alignas(4) const uint8_t ${SYM_NAME}_spv[] = {\n")
    string(APPEND C_CONTENT "#embed \"${SPV_FILE}\"\n")
    string(APPEND C_CONTENT "};\n")
    string(APPEND C_CONTENT "const size_t ${SYM_NAME}_spv_len = sizeof(${SYM_NAME}_spv);\n\n")

    string(APPEND H_CONTENT "extern const uint8_t ${SYM_NAME}_spv[];\n")
    string(APPEND H_CONTENT "extern const size_t ${SYM_NAME}_spv_len;\n\n")
endforeach()

file(WRITE "${C_FILE}" "${C_CONTENT}")
file(WRITE "${H_FILE}" "${H_CONTENT}")

if(ALL_SPV_OUTPUTS)
    add_custom_target(sdlgpu_shaders_compile DEPENDS ${ALL_SPV_OUTPUTS})
    set_source_files_properties("${C_FILE}" PROPERTIES
        OBJECT_DEPENDS "${ALL_SPV_OUTPUTS}"
        GENERATED TRUE
    )
else()
    set_source_files_properties("${C_FILE}" PROPERTIES
        GENERATED TRUE
    )
endif()

set(SHADER_GENERATED_SOURCES "${C_FILE}" PARENT_SCOPE)

if(TARGET sdlgpu_ren)
    target_sources(sdlgpu_ren PRIVATE "${C_FILE}")
    target_include_directories(sdlgpu_ren PRIVATE "${CMAKE_CURRENT_BINARY_DIR}")
    if(ALL_SPV_OUTPUTS)
        add_dependencies(sdlgpu_ren sdlgpu_shaders_compile)
    endif()
endif()
