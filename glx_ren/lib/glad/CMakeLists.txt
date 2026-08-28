cmake_minimum_required(VERSION 3.20)

add_library(glx_glad STATIC
    src/glad.c
)

target_include_directories(glx_glad PUBLIC include/)