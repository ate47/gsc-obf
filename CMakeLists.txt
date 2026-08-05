cmake_minimum_required(VERSION 3.20)
project(GSCObf LANGUAGES CXX C)
set_property(GLOBAL PROPERTY USE_FOLDERS ON)

# Read version information from the version file
file(STRINGS "${CMAKE_SOURCE_DIR}/release/version" _VER_LINES)
list(GET _VER_LINES 0 VERSION)
list(GET _VER_LINES 1 VERSION_ID)

configure_file(
    "${CMAKE_SOURCE_DIR}/cmake/version.hpp.in"
    "${CMAKE_SOURCE_DIR}/src/gsc-obf/version.hpp"
)


macro(append_common_defs target)
    set(target_type "${ARGV1}")

    if ("${target_type}" STREQUAL "")
        set(target_type "CXX")
    endif()

    target_compile_definitions(${target} PRIVATE
        $<$<CONFIG:Debug>:DEBUG>
        $<$<CONFIG:Release>:NDEBUG>
    )
    set_target_properties(${target} PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/build/bin"
        ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/build/bin"
        LIBRARY_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/build/bin"
        OBJECT_OUTPUT_DIRECTORY  "${CMAKE_SOURCE_DIR}/build/obj"
    )
    if ("${target_type}" STREQUAL "CXX")
        set_target_properties(${target} PROPERTIES
            CXX_STANDARD 20
            CXX_STANDARD_REQUIRED ON
        )
        target_link_options(${target} PRIVATE /DEBUG)
    elseif("${target_type}" STREQUAL "C")
        set_target_properties(${target} PROPERTIES 
            LINKER_LANGUAGE C
        )
        target_compile_options(${target} PRIVATE /Zi)
        target_link_options(${target} PRIVATE /DEBUG)
    endif()

endmacro()

include("${CMAKE_SOURCE_DIR}/cmake/GSCObf.cmake")
include("${CMAKE_SOURCE_DIR}/cmake/format.cmake")