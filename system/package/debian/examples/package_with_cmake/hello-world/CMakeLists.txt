project(greet-the-world)

cmake_minimum_required(VERSION 2.6)

set(EXECUTABLE_OUTPUT_PATH ${greet-the-world_SOURCE_DIR})

add_executable(greet-the-world main.cpp)

install(
    TARGETS
        greet-the-world
    RUNTIME DESTINATION
        ${CMAKE_INSTALL_PREFIX}/bin
)
