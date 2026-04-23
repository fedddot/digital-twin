cmake_minimum_required(VERSION 3.16)
project(example_client C CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

include(FetchContent)
FetchContent_Declare(serialib
  GIT_REPOSITORY https://github.com/imabot2/serialib.git
  GIT_TAG        master
  GIT_SHALLOW    TRUE
)
FetchContent_MakeAvailable(serialib)
add_library(serialib_lib STATIC ${serialib_SOURCE_DIR}/lib/serialib.cpp)
target_include_directories(serialib_lib PUBLIC ${serialib_SOURCE_DIR}/lib)

add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/../api ${CMAKE_CURRENT_BINARY_DIR}/api_build)

add_executable(example_client
  ${CMAKE_CURRENT_LIST_DIR}/src/example_client.cpp
)
target_include_directories(example_client PRIVATE
  ${CMAKE_CURRENT_LIST_DIR}/src
)
target_link_libraries(example_client PRIVATE
  nanoipc_ring_buffer
  example_request_writer
  example_response_reader
  serialib_lib
)
