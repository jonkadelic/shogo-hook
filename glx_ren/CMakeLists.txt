include(FetchContent)

# SDL3 builds its own tests/examples by default; disable to keep this fast.
set(SDL_TEST_LIBRARY OFF CACHE BOOL "" FORCE)
set(SDL_TESTS OFF CACHE BOOL "" FORCE)
set(SDL_SHARED OFF CACHE BOOL "" FORCE)
set(SDL_STATIC ON CACHE BOOL "" FORCE)

FetchContent_Declare(
    SDL3
    GIT_REPOSITORY https://github.com/libsdl-org/SDL.git
    GIT_TAG release-3.4.14
    GIT_SHALLOW TRUE
)
FetchContent_MakeAvailable(SDL3)

add_library(glx_ren SHARED
    src/blit.c
    src/main.c
    src/object.c
    src/renderer.c
    src/res.rc
    src/surfaces.c
)

add_subdirectory(lib/glad)

target_include_directories(glx_ren PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src
)

target_link_libraries(glx_ren PRIVATE SDL3::SDL3-static glx_glad lt_common)
target_link_options(glx_ren PRIVATE -static-libgcc)

# Output file is literally "gl3.ren" (no .dll extension), still a PE DLL.
set_target_properties(glx_ren PROPERTIES
    PREFIX ""
    OUTPUT_NAME "glx"
    SUFFIX ".ren"
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/glx_ren"
    LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/glx_ren"
)

add_custom_command(TARGET glx_ren POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E chdir ${CMAKE_SOURCE_DIR} ${CMAKE_SOURCE_DIR}/copy.sh
    COMMENT "Copying glx.ren to target directory"
)
