set(ROOT_DIR ${CMAKE_CURRENT_SOURCE_DIR}/..)
set(SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR})
set(INCLUDE_DIR ${ROOT_DIR}/include)

set(SOURCES
        ${SOURCE_DIR}/main.cpp
)

add_executable(${PROJECT_NAME}_server ${SOURCES})
target_link_libraries(${PROJECT_NAME}_server PRIVATE macrohard::server)
target_compile_options(${PROJECT_NAME}_server PRIVATE ${PROJECT_WARNING_FLAGS})