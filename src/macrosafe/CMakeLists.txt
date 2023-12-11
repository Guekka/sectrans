set(ROOT_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../..)
set(SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR})
set(INCLUDE_DIR ${ROOT_DIR}/include)

set(SOURCES
        ${INCLUDE_DIR}/macrosafe/client.hpp
        ${INCLUDE_DIR}/macrosafe/common.hpp
        ${INCLUDE_DIR}/macrosafe/server.hpp
        ${INCLUDE_DIR}/macrosafe/message.hpp

        ${INCLUDE_DIR}/macrosafe/utils/metaprogramming.hpp
        ${INCLUDE_DIR}/macrosafe/utils/threading.hpp

        ${SOURCE_DIR}/client.cpp
        ${SOURCE_DIR}/server.cpp
        message.cpp
)

add_library(macrosafe STATIC ${SOURCES})

target_link_libraries(macrosafe PRIVATE macrohard::client macrohard::server)
target_include_directories(macrosafe
        SYSTEM PUBLIC ${INCLUDE_DIR}
        PRIVATE ${INCLUDE_DIR}
)
