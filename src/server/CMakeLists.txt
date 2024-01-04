set(ROOT_DIR ${CMAKE_CURRENT_SOURCE_DIR}/..)
set(SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR})

set(SOURCES
        ${SOURCE_DIR}/auth.hpp
        ${SOURCE_DIR}/auth.cpp

        ${SOURCE_DIR}/main.cpp
)

add_executable(${PROJECT_NAME}_server ${SOURCES})
target_link_libraries(${PROJECT_NAME}_server PRIVATE macrosafe shared)
