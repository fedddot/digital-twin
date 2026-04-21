#ifndef EXAMPLE_RESPONSE_READER_HPP
#define EXAMPLE_RESPONSE_READER_HPP

#include "example.pb.h"
#include "example_api.hpp"

static inline ExampleResponse pb_to_response(const example_api_ExampleResponse& pb) {
    return ExampleResponse{ .result = pb.result };
}

#endif // EXAMPLE_RESPONSE_READER_HPP
