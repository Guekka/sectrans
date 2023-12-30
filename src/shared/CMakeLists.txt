set(ROOT_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../..)
set(SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR})
set(INCLUDE_DIR ${ROOT_DIR}/include)

set(SOURCES
        ${INCLUDE_DIR}/shared/json.hpp
        ${INCLUDE_DIR}/shared/message_types.hpp
)

add_library(shared STATIC ${SOURCES})

set_target_properties(shared PROPERTIES LINKER_LANGUAGE CXX)

target_link_libraries(shared
        PUBLIC
        nlohmann_json)


target_include_directories(shared
        SYSTEM PUBLIC ${INCLUDE_DIR}
        PRIVATE ${INCLUDE_DIR}
)
