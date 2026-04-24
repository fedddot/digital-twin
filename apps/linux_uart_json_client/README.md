# linux_uart_json_client

A command-line application that communicates with an embedded service over UART by sending and receiving JSON messages encoded with COBS framing.

It opens the specified serial port, sends a JSON request (from a file or a built-in default), and prints the JSON response received from the device.

## Building

The app is part of the top-level nanoipc CMake project and is only compiled when the `NANOIPC_LINUX_FEATURES` option is enabled (it is **on** by default).

```bash
# From the repository root
cmake -S . -B build -DNANOIPC_LINUX_FEATURES=ON -DNANOIPC_TESTS=OFF
cmake --build build --target linux_uart_json_client
```

The resulting binary is placed in `build/apps/linux_uart_json_client/`.

## Running

```
linux_uart_json_client [OPTIONS]
```

| Option | Default | Description |
|---|---|---|
| `--port` | `/dev/ttyACM0` | Serial device path (e.g. `/dev/ttyUSB0`) |
| `--baud` | `9600` | Baud rate (e.g. `115200`, `230400`) |
| `--parity` | `NONE` | Parity: `NONE`, `EVEN`, `ODD`, `MARK`, `SPACE` |
| `--data-bits` | `8` | Data bits: `5`, `6`, `7`, `8`, `16` |
| `--stop-bits` | `1` | Stop bits: `1`, `1.5`, `2` |
| `--request-file` | *(none)* | Path to a JSON file to send. Omit to send a built-in test request. |

### Example

```bash
# Send a request from a file at 115200 baud
./linux_uart_json_client \
    --port /dev/ttyUSB0 \
    --baud 115200 \
    --request-file request.json
```

**Expected output:**

```
Sending request: {
   "type" : "ping"
}
Received response: {
   "status" : "ok",
   "type" : "pong"
}
```

If `--request-file` is omitted the application sends a default test message:

```json
{
   "payload" : "Hello, world!",
   "type" : "test_request"
}
```
