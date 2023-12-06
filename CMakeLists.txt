# basic cmakelists

cmake_minimum_required(VERSION 3.18)

project(cybersec_project)

set(CMAKE_CXX_STANDARD 17)

add_subdirectory(external/macrohard)
add_subdirectory(src)