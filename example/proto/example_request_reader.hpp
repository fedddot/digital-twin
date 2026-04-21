#ifndef EXAMPLE_REQUEST_READER_HPP
#define EXAMPLE_REQUEST_READER_HPP

#include "example.pb.h"
#include "example_request.hpp"

static inline ExampleRequest pb_to_request(const example_api_ExampleRequest& pb) {
    return ExampleRequest{ .value = pb.value, .action = static_cast<Action>(pb.action) };
}

#endif // EXAMPLE_REQUEST_READER_HPP
