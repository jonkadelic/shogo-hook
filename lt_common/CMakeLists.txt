cmake_minimum_required(VERSION 3.20)

project(lt_common LANGUAGES C)

add_library(lt_common INTERFACE)

target_include_directories(lt_common INTERFACE include/)