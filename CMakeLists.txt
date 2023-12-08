# basic cmakelists

cmake_minimum_required(VERSION 3.18)

project(cybersec_project)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

include(cmake/cpp_warnings.cmake)

add_subdirectory(external/macrohard)
add_subdirectory(src)
add_subdirectory(tests)