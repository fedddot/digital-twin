cmake_minimum_required(VERSION 3.16)
project(example_server C CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# ── Path to the nanoipc repository root ───────────────────────────────────────
get_filename_component(NANOIPC_ROOT "${CMAKE_CURRENT_LIST_DIR}/../.." ABSOLUTE)

# ── Pull in the root nanoipc project (provides nanoipc_reader, nanoipc_writer,
#    nanopb_serializer, nanopb_parser and all their transitive deps) ────────────
set(BUILD_TESTS OFF CACHE BOOL "" FORCE)
add_subdirectory("${NANOIPC_ROOT}" nanoipc)

# ── Protobuf code generation ──────────────────────────────────────────────────
find_package(Python REQUIRED)

set(PROTO_SRC_DIR   "${CMAKE_CURRENT_LIST_DIR}/../proto")
set(PROTO_GEN_DIR   "${CMAKE_CURRENT_BINARY_DIR}/generated")
file(MAKE_DIRECTORY "${PROTO_GEN_DIR}")

add_custom_command(
  OUTPUT
    "${PROTO_GEN_DIR}/example.pb.c"
    "${PROTO_GEN_DIR}/example.pb.h"
  DEPENDS
    "${PROTO_SRC_DIR}/example.proto"
  COMMAND
    "${Python_EXECUTABLE}"
    "${nanopb_SOURCE_DIR}/generator/nanopb_generator.py"
    -I "${PROTO_SRC_DIR}"
    -D "${PROTO_GEN_DIR}"
    example.proto
  COMMENT "Generating nanopb stubs from example.proto"
)

add_custom_target(generate_example_api
  DEPENDS
    "${PROTO_GEN_DIR}/example.pb.c"
    "${PROTO_GEN_DIR}/example.pb.h"
)

add_library(example_api INTERFACE)
add_dependencies(example_api generate_example_api)
target_sources(example_api INTERFACE "${PROTO_GEN_DIR}/example.pb.c")
target_include_directories(example_api INTERFACE "${PROTO_GEN_DIR}")
target_link_libraries(example_api INTERFACE nanopb_common)

# ── Server executable ─────────────────────────────────────────────────────────
add_executable(example_server
  ${CMAKE_CURRENT_LIST_DIR}/src/example_server.cpp
)

target_include_directories(example_server PRIVATE
  ${CMAKE_CURRENT_LIST_DIR}/src
)

target_link_libraries(example_server PRIVATE
  nanoipc_reader
  nanoipc_writer
  nanopb_serializer
  nanopb_parser
  example_api
)

# ── Generate .hex when cross-compiling for AVR ────────────────────────────────
if(CMAKE_SYSTEM_PROCESSOR STREQUAL "avr")
  include(${CMAKE_CURRENT_LIST_DIR}/avr-toolchain.cmake OPTIONAL)
  avr_generate_hex(example_server)
endif()
