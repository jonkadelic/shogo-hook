cmake_minimum_required(VERSION 3.22)

set(CMAKE_C_STANDARD 23)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS ON)

add_subdirectory(hook_dll)
add_subdirectory(launcher)
# add_subdirectory(sdlgpu_ren)
# add_subdirectory(glx_ren)
add_subdirectory(gl46_ren)
add_subdirectory(lt_common)
