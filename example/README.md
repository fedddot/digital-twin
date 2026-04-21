# NanoIPC Example

This directory contains two standalone CMake projects that demonstrate end-to-end
protobuf messaging over UART using the NanoIPC library:

| Project | Target platform | Role |
|---|---|---|
| `example_server` | STM32F103C8T6 (Blue Pill) | Receives `ExampleRequest`, processes it, sends `ExampleResponse` |
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

Runs on the [Blue Pill](https://stm32-base.org/boards/STM32F103C8T6-Blue-Pill.html)
development board (STM32F103C8T6, Cortex-M3, 72 MHz, 64 KiB flash, 20 KiB RAM).
UART communication is on **USART1** (PA9 TX / PA10 RX, 9600 baud 8N1).

### Prerequisites

| Tool | Notes |
|---|---|
| `arm-none-eabi-gcc` / `g++` | ARM cross-compiler (`sudo apt install gcc-arm-none-eabi binutils-arm-none-eabi`) |
| `st-flash` or `OpenOCD` | Flash uploader |
| CMake ≥ 3.16 | |
| Python ≥ 3.8 | Required by the nanopb protobuf generator |

### Build

A CMake toolchain file for `arm-none-eabi-g++` is provided at
`example_server/stm32-bluepill-toolchain.cmake`. It targets the STM32F103C8T6
and automatically generates `.hex` and `.bin` files after every build.

```bash
cd example/example_server
cmake -B build -DCMAKE_TOOLCHAIN_FILE=stm32-bluepill-toolchain.cmake \
               -DCMAKE_BUILD_TYPE=MinSizeRel
cmake --build build
```

### Flash to Blue Pill

**st-link (ST-LINK/V2 dongle):**
```bash
st-flash write build/example_server.bin 0x08000000
```

**OpenOCD:**
```bash
openocd -f interface/stlink.cfg \
        -f target/stm32f1x.cfg \
        -c "program build/example_server.elf verify reset exit"
```

### Pin-out

| Blue Pill pin | Signal | Connect to |
|---|---|---|
| PA9  | TX | USB-UART adapter RX |
| PA10 | RX | USB-UART adapter TX |
| GND  | GND | USB-UART adapter GND |

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
cmake -B build
cmake --build build
```

### Run

```bash
./build/example_client --port /dev/ttyUSB0 --baud 9600 --value 42 --action add
```

| Option | Description |
|---|---|
| `--port`   | Serial port connected to the Blue Pill |
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
│   ├── CMakeLists.txt                 # Standalone CMake project (Blue Pill target)
│   ├── stm32-bluepill-toolchain.cmake # ARM cross-compiler toolchain
│   ├── STM32F103C8Tx_FLASH.ld         # Linker script
│   └── src/
│       ├── startup_stm32f103.c        # Vector table, Reset_Handler, SystemInit
│       ├── uart_read_buffer.hpp       # ISR-safe circular ReadBuffer
│       └── example_server.cpp         # Main application
├── example_client/
│   ├── CMakeLists.txt         # Standalone CMake project (PC target)
│   └── src/
│       └── example_client.cpp     # Main application
└── README.md                  # This file
```

