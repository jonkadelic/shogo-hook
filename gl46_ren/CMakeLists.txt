cmake_minimum_required(VERSION 3.20)

project(gl46_ren LANGUAGES C)

include(FetchContent)

# SDL3 builds its own tests/examples by default; disable to keep this fast.
set(SDL_TEST_LIBRARY OFF CACHE BOOL "" FORCE)
set(SDL_TESTS OFF CACHE BOOL "" FORCE)
set(SDL_SHARED OFF CACHE BOOL "" FORCE)
set(SDL_STATIC ON CACHE BOOL "" FORCE)
set(SDL_JOYSTICK OFF CACHE BOOL "" FORCE)
set(SDL_HAPTIC OFF CACHE BOOL "" FORCE)

FetchContent_Declare(
    SDL3
    GIT_REPOSITORY https://github.com/libsdl-org/SDL.git
    GIT_TAG release-3.4.14
    GIT_SHALLOW TRUE
)
FetchContent_Declare(
    glm
    GIT_REPOSITORY https://github.com/g-truc/glm.git
    GIT_TAG 1.0.3
)
FetchContent_MakeAvailable(SDL3 glm)

file(GLOB_RECURSE GL46_SRCS CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/src/*.c")

add_library(gl46_ren SHARED
    src/res.rc
    ${GL46_SRCS}
)

set_target_properties(gl46_ren PROPERTIES
    CMAKE_C_STANDARD_REQUIRED ON
    CMAKE_C_EXTENSIONS ON
)

add_subdirectory(lib)

target_include_directories(gl46_ren PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src
)

target_link_libraries(gl46_ren PRIVATE
    SDL3::SDL3-static
    lt_common
    gl46_glad
    hmm
)

if (MINGW)
    target_link_options(gl46_ren PRIVATE
        -static
    )
endif()

# Output file is literally "gl46.ren" (no .dll extension), still a PE DLL.
set_target_properties(gl46_ren PROPERTIES
    PREFIX ""
    OUTPUT_NAME "gl46"
    SUFFIX ".ren"
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/gl46_ren"
    LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/gl46_ren"
)
