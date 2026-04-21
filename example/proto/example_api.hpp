#ifndef EXAMPLE_API_HPP
#define EXAMPLE_API_HPP

#include <cstdint>

namespace api {

    /// Strongly-typed action enum matching the protobuf definition.
    enum class Action : int32_t { ADD = 0, SUBTRACT = 1 };

    /// Request type for the example API.
    struct ExampleRequest {
        int32_t value;
        Action  action;
    };

    /// Response type for the example API.
    struct ExampleResponse {
        int32_t result;
    };

} // namespace api

#endif // EXAMPLE_API_HPP
