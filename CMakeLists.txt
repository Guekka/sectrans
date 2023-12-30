# basic cmakelists

cmake_minimum_required(VERSION 3.18)

project(cybersec_project)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_subdirectory(external/macrohard)
add_subdirectory(external/nlohmann)
add_subdirectory(external/libhydrogen)
add_subdirectory(src)
add_subdirectory(tests)
