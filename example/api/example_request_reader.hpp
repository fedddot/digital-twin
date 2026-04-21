#ifndef EXAMPLE_REQUEST_READER_HPP
#define EXAMPLE_REQUEST_READER_HPP

#include "example.pb.h"
#include "example_request.hpp"
#include "nanoipc_reader.hpp"
#include "nanopb_parser.hpp"

namespace api {

    /// @brief Reader specialised for ExampleRequest messages.
    class ExampleRequestReader : public nanoipc::NanoIpcReader<ExampleRequest> {
    public:
        /// Construct a reader that reads encoded frames from `read_buffer`.
        explicit ExampleRequestReader(nanoipc::ReadBuffer* read_buffer)
        : nanoipc::NanoIpcReader<ExampleRequest>(
            nanoipc::NanoPbParser<ExampleRequest, example_api_ExampleRequest>(pb_to_request, example_api_ExampleRequest_fields),
            read_buffer
        ) {}
    private:
        static ExampleRequest pb_to_request(const example_api_ExampleRequest& pb) {
            return ExampleRequest{ .value = pb.value, .action = static_cast<Action>(pb.action) };
        }
    };

} // namespace api

#endif // EXAMPLE_REQUEST_READER_HPP
