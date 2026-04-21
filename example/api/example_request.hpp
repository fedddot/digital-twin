#ifndef EXAMPLE_REQUEST_HPP
#define EXAMPLE_REQUEST_HPP

#include <cstdint>

enum class Action : int32_t { ADD = 0, SUBTRACT = 1 };

struct ExampleRequest {
    int32_t value;
    Action  action;
};

#endif // EXAMPLE_REQUEST_HPP
