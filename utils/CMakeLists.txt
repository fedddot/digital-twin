FetchContent_Declare(
    serialib
    GIT_REPOSITORY https://github.com/imabot2/serialib.git
    GIT_TAG        master
)
FetchContent_MakeAvailable(serialib)

# Create an interface library for serialib since it doesn't have CMakeLists.txt
add_library(serialib_interface INTERFACE)
target_include_directories(serialib_interface INTERFACE ${serialib_SOURCE_DIR}/lib)
target_sources(serialib_interface INTERFACE ${serialib_SOURCE_DIR}/lib/serialib.cpp)
target_compile_features(serialib_interface INTERFACE cxx_std_17)

add_library(uart_connection)
target_sources(
    uart_connection
    PRIVATE
    ${CMAKE_CURRENT_LIST_DIR}/src/uart_connection.cpp
)
target_include_directories(
    uart_connection
    PUBLIC
    ${CMAKE_CURRENT_LIST_DIR}/include
)
target_link_libraries(
    uart_connection
    PUBLIC
    serialib_interface
)
target_compile_features(uart_connection PRIVATE cxx_std_17)

if(BUILD_TESTS)
  add_subdirectory(tests)
endif()
