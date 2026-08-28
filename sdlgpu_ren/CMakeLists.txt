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

file(GLOB_RECURSE SDLGPU_SRCS src/*.c)

add_library(sdlgpu_ren SHARED
    src/res.rc
    ${SDLGPU_SRCS}
)

add_subdirectory(src/shaders)

target_include_directories(sdlgpu_ren PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src
)

target_link_libraries(sdlgpu_ren PRIVATE
    SDL3::SDL3-static
    lt_common
)
target_link_options(sdlgpu_ren PRIVATE -static-libgcc)

# Output file is literally "sdlgpu.ren" (no .dll extension), still a PE DLL.
set_target_properties(sdlgpu_ren PROPERTIES
    PREFIX ""
    OUTPUT_NAME "sdlgpu"
    SUFFIX ".ren"
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/sdlgpu_ren"
    LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/sdlgpu_ren"
)

add_custom_command(TARGET sdlgpu_ren POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E chdir ${CMAKE_SOURCE_DIR} ${CMAKE_SOURCE_DIR}/copy.sh
    COMMENT "Copying sdlgpu.ren to target directory"
)
