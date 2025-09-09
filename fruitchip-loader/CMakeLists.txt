cmake_minimum_required(VERSION 3.13)

project(fruitchip-loader)

set(CMAKE_EXPORT_COMPILE_COMMANDS 1)

set(EE_STAGE_1_ADDR 0x001001B0)
set(EE_STAGE_2_ADDR 0x000E0000)
set(EE_STAGE_3_ADDR 0x00090000)

add_subdirectory(ee-stage1)
add_subdirectory(ee-stage2)
