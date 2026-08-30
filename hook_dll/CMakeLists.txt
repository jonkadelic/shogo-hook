add_subdirectory(lib/minhook-1.3.4 ${CMAKE_CURRENT_BINARY_DIR}/minhook)

add_library(shogo_hook SHARED
    src/main.c
    src/hook.c
)

target_include_directories(shogo_hook PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src
)

target_link_libraries(shogo_hook PRIVATE
    minhook
    winmm
    gdi32
)

set_target_properties(shogo_hook PROPERTIES
    PREFIX ""
    OUTPUT_NAME "shogo_hook"
    LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/hook_dll"
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/hook_dll"
)
