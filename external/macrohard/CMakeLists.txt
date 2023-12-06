set(TYPES client server)
foreach (type IN LISTS TYPES)
    add_library(
            macrohard_${type}
            SHARED
            IMPORTED GLOBAL
    )

    add_library(
            macrohard::${type}
            ALIAS
            macrohard_${type})

    set_target_properties(
            macrohard_${type}
            PROPERTIES IMPORTED_LOCATION
            ${CMAKE_CURRENT_SOURCE_DIR}/${type}/lib${type}.so
    )

    message(STATUS "macrohard_${type} imported from ${CMAKE_CURRENT_SOURCE_DIR}/${type}/lib${type}.so")

    target_include_directories(
            macrohard_${type}
            INTERFACE
            ${CMAKE_CURRENT_SOURCE_DIR}/${type}
    )
endforeach ()

