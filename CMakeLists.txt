cmake_minimum_required(VERSION 3.16)

project(mcu_server C CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

option(BUILD_TESTS "Build the project's test suite" ON)
if(BUILD_TESTS)
    enable_testing()
    add_subdirectory(googletest EXCLUDE_FROM_ALL)
endif()

add_subdirectory(provider)
add_subdirectory(host)
add_subdirectory(service)
add_subdirectory(ipc)