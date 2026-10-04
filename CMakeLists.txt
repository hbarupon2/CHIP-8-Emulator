cmake_minimum_required(VERSION 3.16)
project(CHIP8 C)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

find_package(SDL2 REQUIRED)

add_executable(CHIP8
        src/main.c
        src/fontset.h
)
target_compile_options(CHIP8 PRIVATE
    -Wall
    -Wextra
)
target_link_libraries(CHIP8 PRIVATE
    SDL2::SDL2
)

if (CMAKE_BUILD_TYPE STREQUAL "Debug")
    target_compile_options(CHIP8 PRIVATE -g -fsanitize=address,undefined)
    target_link_options(CHIP8 PRIVATE -fsanitize=address,undefined)
endif()
