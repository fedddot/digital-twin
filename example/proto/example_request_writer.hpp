#ifndef EXAMPLE_REQUEST_WRITER_HPP
#define EXAMPLE_REQUEST_WRITER_HPP

#include "example.pb.h"
#include "example_request.hpp"

static inline example_api_ExampleRequest request_to_pb(const ExampleRequest& req) {
    example_api_ExampleRequest pb = example_api_ExampleRequest_init_default;
    pb.value  = req.value;
    pb.action = static_cast<example_api_Action>(req.action);
    return pb;
}

#endif // EXAMPLE_REQUEST_WRITER_HPP
