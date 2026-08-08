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
set(EXCLUDED_FORMAT_FILE
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

macro(load_dir_recurse id)
    file(GLOB DEP_SCRIPTS "${CMAKE_SOURCE_DIR}/cmake/${id}/*.cmake")

    foreach(script ${DEP_SCRIPTS})
        if(ALL_PLATFORM_ACTS)
            get_filename_component(script_name "${script}" NAME)

            if(NOT script_name IN_LIST ALL_PLATFORM_SAFE_SCRIPTS)
                message(STATUS "Skipping ${script} (not all platform)")
                continue()
            endif()
        endif()

        message(STATUS "Loading ${script}")
        include(${script})
    endforeach()
endmacro()

macro(add_format_excluded path)
    list(APPEND EXCLUDED_FORMAT_FILE "${path}")
endmacro()

load_dir_recurse(deps)

include("${CMAKE_SOURCE_DIR}/cmake/GSCObf.cmake")
include("${CMAKE_SOURCE_DIR}/cmake/format.cmake")