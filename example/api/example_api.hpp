#ifndef EXAMPLE_API_HPP
#define EXAMPLE_API_HPP

#include <cstdint>

namespace api {
    enum class Action : int32_t { ADD = 0, SUBTRACT = 1 };

    struct ExampleRequest {
        int32_t value;
        Action  action;
    };

    struct ExampleResponse {
        int32_t result;
    };
} // namespace api

#endif // EXAMPLE_API_HPP
