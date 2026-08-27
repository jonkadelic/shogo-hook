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

add_library(gl3_ren SHARED
    src/main.c
    src/blit.c
    src/renderer.c
    src/res.rc
    src/surfaces.c
    src/object.c
)

add_subdirectory(lib/glad)

target_include_directories(gl3_ren PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src
)

target_link_libraries(gl3_ren PRIVATE SDL3::SDL3-static glad lt_common)
target_link_options(gl3_ren PRIVATE -static-libgcc)

# Output file is literally "gl3.ren" (no .dll extension), still a PE DLL.
set_target_properties(gl3_ren PROPERTIES
    PREFIX ""
    OUTPUT_NAME "gl3"
    SUFFIX ".ren"
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/gl3_ren"
    LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/gl3_ren"
)

add_custom_command(TARGET gl3_ren POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E chdir ${CMAKE_SOURCE_DIR} ${CMAKE_SOURCE_DIR}/copy.sh
    COMMENT "Copying gl3.ren to target directory"
)
