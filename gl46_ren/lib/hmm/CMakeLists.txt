cmake_minimum_required(VERSION 3.20)

project(hmm LANGUAGES C)

add_library(hmm INTERFACE)

target_include_directories(hmm INTERFACE include/)