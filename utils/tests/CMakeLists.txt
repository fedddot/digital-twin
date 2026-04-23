add_executable(
    utils_tests
    ${CMAKE_CURRENT_LIST_DIR}/src/ut_uart_connection.cpp
)

target_link_libraries(
    utils_tests
    PRIVATE
    uart_connection
    gtest
    gtest_main
)

add_test(NAME utils_tests COMMAND utils_tests)
