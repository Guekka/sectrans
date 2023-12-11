set(INCLUDE_DIR "${CMAKE_CURRENT_SOURCE_DIR}")
set(SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}")

set(SOURCE_FILES
        "${SOURCE_DIR}/doctest.h"
        "${SOURCE_DIR}/send_and_receive.cpp"
        "${SOURCE_DIR}/main.cpp"
)

add_executable(tests ${SOURCE_FILES})

target_compile_options(
        tests PRIVATE "$<$<CXX_COMPILER_ID:AppleClang,Clang,GNU>:-fvisibility=hidden>"
        "$<$<CXX_COMPILER_ID:MSVC>:/utf-8>")

target_include_directories(tests PRIVATE "${INCLUDE_DIR}" "${SOURCE_DIR}")

target_link_libraries(tests PRIVATE macrosafe)

target_compile_options(tests
        PUBLIC "$<$<CXX_COMPILER_ID:MSVC>:/Zc:__cplusplus>")
