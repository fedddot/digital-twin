# NanoIPC Example

This directory contains two standalone CMake projects that demonstrate end-to-end
protobuf messaging over UART using the NanoIPC library:

| Project | Target platform | Role |
|---|---|---|
| `example_server` | Arduino Nano (ATmega328P) | Receives `ExampleRequest`, processes it, sends `ExampleResponse` |
| `example_client` | PC (Linux / macOS / Windows) | Sends `ExampleRequest` over a serial port, prints the `ExampleResponse` |

Both projects are independent of the root `CMakeLists.txt` and fetch their own
external dependencies.

---

## Proto definition

`proto/example.proto` defines the message types shared by both sides:

```proto
message ExampleRequest  { int32 value = 1; Action action = 2; }
message ExampleResponse { int32 result = 1; }
enum   Action           { ADD = 0; SUBTRACT = 1; }
```

---

## example_server

### Prerequisites (cross-compilation for Arduino Nano)

| Tool | Notes |
|---|---|
| `avr-g++` / `avr-gcc` | AVR cross-compiler (`sudo apt install gcc-avr binutils-avr`) |
| `avrdude` | Flash uploader (`sudo apt install avrdude`) |
| CMake ≥ 3.16 | |
| Python ≥ 3.8 | Required by the nanopb protobuf generator |

> **Host build (for testing without hardware)**  
> The server can also be compiled for the host machine (Linux/macOS).  
> Simply omit the cross-compiler toolchain file in the steps below.

### Build (host – smoke-test)

```bash
cd example/example_server
mkdir -p build && cd build
cmake ..
cmake --build . -j
```

### Build (Arduino Nano – cross-compile)

A ready-to-use CMake toolchain file for `avr-g++` is provided at
`example_server/avr-toolchain.cmake`. It targets the ATmega328P at 16 MHz and
automatically generates a `.hex` file suitable for `avrdude` after every build.

```bash
cd example/example_server
mkdir -p build-avr && cd build-avr
cmake .. -DCMAKE_TOOLCHAIN_FILE=../avr-toolchain.cmake \
         -DF_CPU=16000000UL \
         -DCMAKE_BUILD_TYPE=MinSizeRel
cmake --build . -j
```

### Flash to Arduino Nano

```bash
avrdude -c arduino -p atmega328p \
        -P /dev/ttyUSB0 -b 115200 \
        -U flash:w:build-avr/example_server.hex
```

Replace `/dev/ttyUSB0` with the actual serial port of your Arduino Nano.

---

## example_client

### Prerequisites

| Tool | Notes |
|---|---|
| `g++` / `clang++` | C++17-capable compiler |
| CMake ≥ 3.16 | |
| Python ≥ 3.8 | Required by the nanopb protobuf generator |

### Build

```bash
cd example/example_client
mkdir -p build && cd build
cmake ..
cmake --build . -j
```

### Run

```bash
./example_client --port /dev/ttyUSB0 --baud 9600 --value 42 --action add
```

| Option | Description |
|---|---|
| `--port`   | Serial port connected to the Arduino Nano |
| `--baud`   | Baud rate (must match the server, default 9600) |
| `--value`  | Integer value sent in the `ExampleRequest` |
| `--action` | `add` or `subtract` |

The client prints the `result` field of the received `ExampleResponse`.

---

## Communication protocol

Both sides use the same framing stack:

```
ExampleRequest / ExampleResponse
        │ nanopb (protobuf encode/decode)
        ▼
   raw protobuf bytes
        │ COBS encode/decode  (nanoipc_utils)
        ▼
  COBS-framed bytes  ──── UART 9600 8N1 ────►
```

Frames are delimited by the COBS zero byte (`0x00`).

---

## Directory layout

```
example/
├── proto/
│   └── example.proto          # Shared protobuf definition
├── example_server/
│   ├── CMakeLists.txt         # Standalone CMake project (Arduino Nano target)
│   └── src/
│       ├── uart_read_buffer.hpp   # ISR-safe circular ReadBuffer
│       └── example_server.cpp     # Main application
├── example_client/
│   ├── CMakeLists.txt         # Standalone CMake project (PC target)
│   └── src/
│       └── example_client.cpp     # Main application
└── README.md                  # This file
```
