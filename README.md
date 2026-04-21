# nanoipc

A minimal C++17 IPC library for embedded systems using [nanopb](https://github.com/nanopb/nanopb) (Protocol Buffers) and [nanocobs](https://github.com/charlesnicholson/nanocobs) (COBS framing) over a byte-stream transport such as UART.

## Repository layout

```
nanoipc/
├── nanoipc_read_buffer.hpp   # ReadBuffer interface
├── nanoipc_reader.hpp        # NanoIpcReader – deserialises incoming messages
├── nanoipc_writer.hpp        # NanoIpcWriter – serialises outgoing messages
├── nanoipc_utils/            # COBS encode/decode helpers
├── nanopb_utils/             # nanopb parser and serialiser adapters
├── tests/                    # Google Test suite (host build)
└── example/
    ├── proto/                # example.proto definition
    └── example_server/       # BluePill firmware (see below)
```

## Example server (STM32F103C8T6 – Blue Pill)

The example server runs on the [Blue Pill](https://stm32-base.org/boards/STM32F103C8T6-Blue-Pill.html) development board (STM32F103C8T6, Cortex-M3, 64 KiB flash, 20 KiB RAM).

It receives `ExampleRequest` protobuf messages over **USART1** (PA9 TX / PA10 RX at 9600 baud), processes them, and responds with `ExampleResponse` messages over the same port.

The system clock is configured to **72 MHz** via the on-board 8 MHz crystal (HSE) and the internal PLL (×9) in `startup_stm32f103.c`.

### Prerequisites

| Tool | Install (Debian / Ubuntu) |
|------|--------------------------|
| ARM GCC cross-compiler | `sudo apt install gcc-arm-none-eabi binutils-arm-none-eabi` |
| CMake ≥ 3.16 | `sudo apt install cmake` |
| Python 3 (nanopb generator) | `sudo apt install python3` |
| Git | `sudo apt install git` |

A flasher such as [st-flash](https://github.com/stlink-org/stlink) or [OpenOCD](https://openocd.org/) is needed to write the firmware to the board.

### Build

```bash
cd example/example_server

cmake -B build \
      -DCMAKE_TOOLCHAIN_FILE=stm32-bluepill-toolchain.cmake \
      -DCMAKE_BUILD_TYPE=MinSizeRel

cmake --build build
```

The build produces three artefacts inside `build/`:

| File | Description |
|------|-------------|
| `example_server.elf` | ELF image (use with a debugger / OpenOCD) |
| `example_server.hex` | Intel HEX – suitable for `st-flash` / `stm32flash` |
| `example_server.bin` | Raw binary – suitable for `st-flash write` |
| `example_server.map` | Linker map file |

### Flash

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

### USART1 pin-out

| Blue Pill pin | Signal | Connect to |
|---------------|--------|------------|
| PA9  | TX | USB-UART adapter RX |
| PA10 | RX | USB-UART adapter TX |
| GND  | GND | USB-UART adapter GND |

Settings: **9600 baud, 8N1**.

## Running the tests (host)

```bash
cmake -B build-tests
cmake --build build-tests
ctest --test-dir build-tests
```

