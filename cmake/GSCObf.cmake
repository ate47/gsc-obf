# GSCObf
file(GLOB_RECURSE GOBF_SOURCES
    "${CMAKE_SOURCE_DIR}/src/gsc-obf/*.cpp"
    "${CMAKE_SOURCE_DIR}/src/gsc-obf/*.h"
    "${CMAKE_SOURCE_DIR}/src/gsc-obf/*.hpp"
)

add_executable(GSCObf ${GOBF_SOURCES})
source_group(
    TREE "${CMAKE_SOURCE_DIR}/src/gsc-obf"
    PREFIX gsc-obf
    FILES ${GOBF_SOURCES}
)
set_target_properties(GSCObf PROPERTIES
    OUTPUT_NAME "gsc-obf"
    FOLDER "Executables"
)

append_common_defs(GSCObf)

target_include_directories(GSCObf PRIVATE 
    "${CMAKE_SOURCE_DIR}/src/gsc-obf"
    "${CMAKE_SOURCE_DIR}/deps/crc_cpp/include/"
)
