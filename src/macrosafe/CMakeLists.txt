set(ROOT_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../..)
set(SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR})
set(INCLUDE_DIR ${ROOT_DIR}/include)

set(SOURCES
        ${INCLUDE_DIR}/macrosafe/detail/client.hpp
        ${INCLUDE_DIR}/macrosafe/detail/common.hpp
        ${INCLUDE_DIR}/macrosafe/detail/server.hpp
        ${INCLUDE_DIR}/macrosafe/detail/message.hpp

        ${INCLUDE_DIR}/macrosafe/channel.hpp
        ${INCLUDE_DIR}/macrosafe/encrypted_channel.hpp

        ${INCLUDE_DIR}/macrosafe/utils/metaprogramming.hpp
        ${INCLUDE_DIR}/macrosafe/utils/serde.hpp
        ${INCLUDE_DIR}/macrosafe/utils/threading.hpp

        ${SOURCE_DIR}/client.cpp
        ${SOURCE_DIR}/server.cpp
        ${SOURCE_DIR}/message.cpp
        ${SOURCE_DIR}/channel.cpp
        ${SOURCE_DIR}/encrypted_channel.cpp
)

add_library(macrosafe STATIC ${SOURCES})

target_link_libraries(macrosafe PRIVATE macrohard::client macrohard::server)
target_include_directories(macrosafe
        SYSTEM PUBLIC ${INCLUDE_DIR}
        PRIVATE ${INCLUDE_DIR}
)

target_link_libraries(macrosafe PUBLIC hydrogen::hydrogen)
