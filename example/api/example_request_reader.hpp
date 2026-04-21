#ifndef EXAMPLE_REQUEST_READER_HPP
#define EXAMPLE_REQUEST_READER_HPP

#include "example.pb.h"
#include "example_request.hpp"
#include "../..//nanoipc_reader.hpp"
#include "../../nanopb_utils/nanopb_parser.hpp"

namespace api {

static inline ExampleRequest pb_to_request(const example_api_ExampleRequest& pb) {
    return ExampleRequest{ .value = pb.value, .action = static_cast<Action>(pb.action) };
}

/// @brief Reader specialised for ExampleRequest messages.
class ExampleRequestReader : public nanoipc::NanoIpcReader<ExampleRequest> {
public:
    /// Construct a reader that reads encoded frames from `read_buffer`.
    explicit ExampleRequestReader(nanoipc::ReadBuffer* read_buffer)
    : nanoipc::NanoIpcReader<ExampleRequest>(
        nanoipc::NanoPbParser<ExampleRequest, example_api_ExampleRequest>(pb_to_request, example_api_ExampleRequest_fields),
        read_buffer
    ) {}
};

} // namespace api

#endif // EXAMPLE_REQUEST_READER_HPP
