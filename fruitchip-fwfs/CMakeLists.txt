cmake_minimum_required(VERSION 3.13)

project(fruitchip-fwfs)

set(IOP_BIN fruitchip-fwfs.irx)

add_executable(${IOP_BIN}
    src/main.c
    src/fwfs.c
    ${CMAKE_CURRENT_BINARY_DIR}/imports.c
    ${CMAKE_CURRENT_BINARY_DIR}/exports.c
)

build_iop_imports(
    ${CMAKE_CURRENT_BINARY_DIR}/imports.c
    ${CMAKE_CURRENT_SOURCE_DIR}/imports.lst
)

build_iop_exports(
    ${CMAKE_CURRENT_BINARY_DIR}/exports.c
    ${CMAKE_CURRENT_SOURCE_DIR}/exports.tab
)

target_include_directories(${IOP_BIN}
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/include
        ${CMAKE_CURRENT_SOURCE_DIR}/../fruitchip-include
)

target_compile_options(${IOP_BIN}
    PUBLIC
        -Wall
        -fno-builtin
        -O3
)

target_link_options(${IOP_BIN}
    PUBLIC
        -Wl,--gc-sections
        -s  # strip
)
