#ifndef EXAMPLE_RESPONSE_WRITER_HPP
#define EXAMPLE_RESPONSE_WRITER_HPP

#include "example.pb.h"
#include "example_api.hpp"

static inline example_api_ExampleResponse response_to_pb(const ExampleResponse& resp) {
    example_api_ExampleResponse pb = example_api_ExampleResponse_init_default;
    pb.result = resp.result;
    return pb;
}

#endif // EXAMPLE_RESPONSE_WRITER_HPP
