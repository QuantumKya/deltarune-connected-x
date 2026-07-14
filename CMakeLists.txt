cmake_minimum_required(VERSION 3.26)
set(CMAKE_C_COMPILER "gcc")

project(deltarune_connected LANGUAGES C)
find_package(raylib CONFIG REQUIRED)
add_executable(deltarune_connected main.c)
target_link_libraries(deltarune_connected PRIVATE raylib)

target_sources(deltarune_connected
  PRIVATE
    main.c
    player.h
)

find_package(raylib CONFIG REQUIRED)
add_executable(deltabuilder builder.c)
target_link_libraries(deltabuilder PRIVATE raylib)

target_sources(deltabuilder
  PRIVATE
    builder.c
    builder_parts.h
    building.h
    dep/tinyfiledialogs.h
    dep/tinyfiledialogs.c
)