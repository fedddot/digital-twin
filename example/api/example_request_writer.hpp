#ifndef EXAMPLE_REQUEST_WRITER_HPP
#define EXAMPLE_REQUEST_WRITER_HPP

#include "example.pb.h"
#include "example_request.hpp"
#include "../../nanopb_utils/nanopb_serializer.hpp"
#include "../../nanoipc_writer.hpp"

namespace api {

static inline example_api_ExampleRequest request_to_pb(const ExampleRequest& req) {
    example_api_ExampleRequest pb = example_api_ExampleRequest_init_default;
    pb.value  = req.value;
    pb.action = static_cast<example_api_Action>(req.action);
    return pb;
}

/// @brief Writer specialised for ExampleRequest messages.
class ExampleRequestWriter : public nanoipc::NanoIpcWriter<ExampleRequest> {
public:
    using RawDataWriter = typename nanoipc::NanoIpcWriter<ExampleRequest>::RawDataWriter;

    /// Construct a writer that emits encoded frames via `raw_writer`.
    explicit ExampleRequestWriter(const RawDataWriter& raw_writer)
    : nanoipc::NanoIpcWriter<ExampleRequest>(
        nanoipc::NanoPbSerializer<ExampleRequest, example_api_ExampleRequest>(request_to_pb, example_api_ExampleRequest_fields),
        raw_writer
    ) {}
};

} // namespace api

#endif // EXAMPLE_REQUEST_WRITER_HPP
