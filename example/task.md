Under dir example, create an example project consisting of:
- example_server - a service residing on an arduino nano
- example_client - a client running on a PC that can send requests to the example_server
# example_server
Example server is based on example.proto. It compiles the proto and initializes:
- nanoipc_reader.hpp
- nanoipc_writer.hpp

in a loop it queries the nanoipc reader (initialized with nanopb_utils/nanopb_parser.hpp) for incoming ExampleRequest messages, processes them, and responds with ExampleResponse messages by writing the message with nanoipc_writer (initialized with nanopb_utils/nanopb_serializer.hpp). The server application should init a UART Rx interrupt to update the ReadBuffer each time a new byte is received.
The server is implemented in example_server/example_server as a separate CMake project (which is not a part of the root CMakeLists.txt).
# example_client
Example client is a simple command-line application that can send ExampleRequest messages by writing to the nanoipc_writer (initialized with nanopb_utils/nanopb_serializer.hpp) and reads responses from the nanoipc_reader (initialized with nanopb_utils/nanopb_parser.hpp). The client writes the request and reads the response with UART communication.
The client is implemented in example_client/example_client as a separate CMake project (which is not a part of the root CMakeLists.txt).

# Other
Add a README.md file in the example directory that explains how to build and run both the example_server and example_client, including any necessary setup for the Arduino Nano and the PC.