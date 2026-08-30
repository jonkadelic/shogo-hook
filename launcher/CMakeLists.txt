add_executable(shogo_hook_launcher
    src/main.c
)

target_include_directories(shogo_hook_launcher PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src
)

target_link_libraries(shogo_hook_launcher PRIVATE
    kernel32
)

set_target_properties(shogo_hook_launcher PROPERTIES
    OUTPUT_NAME "shogo_hook_launcher"
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/launcher"
)
