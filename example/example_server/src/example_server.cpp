/// @file example_server.cpp
/// @brief NanoIPC example server – runs on an Arduino Nano (ATmega328P).
///
/// The server listens for ExampleRequest protobuf messages arriving over UART,
/// processes each request, and responds with an ExampleResponse message.
///
/// Platform-specific UART code is compiled only when targeting AVR
/// (__AVR__ is defined by avr-g++).  When building for a host PC (e.g. for
/// unit testing / smoke-testing the logic) the UART callbacks are replaced by
/// thin stdin/stdout stubs so that the exact same business logic can be
/// exercised without hardware.

// ─── Standard headers ────────────────────────────────────────────────────────
#include <cstddef>
#include <cstdint>
#include <stdexcept>

// ─── NanoIPC ─────────────────────────────────────────────────────────────────
#include "nanoipc_reader.hpp"
#include "nanoipc_writer.hpp"
#include "nanopb_parser.hpp"
#include "nanopb_serializer.hpp"

// ─── Generated protobuf stubs (produced by nanopb_generator.py) ──────────────
#include "example.pb.h"

// ─── Local helpers ───────────────────────────────────────────────────────────
#include "uart_read_buffer.hpp"

// ─── Platform-specific includes ──────────────────────────────────────────────
#ifdef __AVR__
#  include <avr/io.h>
#  include <avr/interrupt.h>
#endif

// =============================================================================
// Domain types
// =============================================================================

/// Strongly-typed action enum mirroring the protobuf definition.
enum class Action : int32_t { ADD = 0, SUBTRACT = 1 };

/// C++ request type used throughout the application.
struct ExampleRequest {
    int32_t value;
    Action  action;
};

/// C++ response type used throughout the application.
struct ExampleResponse {
    int32_t result;
};

// =============================================================================
// Globals
// =============================================================================

/// Buffer filled byte-by-byte from the UART Rx ISR.
static example::UartReadBuffer<256> g_read_buffer;

// =============================================================================
// Platform-specific UART implementation
// =============================================================================

#ifdef __AVR__

/// @brief Initialise USART0 on the ATmega328P.
/// @param baud Desired baud rate (e.g. 9600).
static void uart_init(const uint32_t baud) {
    const uint16_t ubrr = static_cast<uint16_t>(F_CPU / (16UL * baud) - 1);
    UBRR0H = static_cast<uint8_t>(ubrr >> 8);
    UBRR0L = static_cast<uint8_t>(ubrr);
    // Enable receiver, transmitter, and Rx-complete interrupt.
    UCSR0B = (1 << RXEN0) | (1 << TXEN0) | (1 << RXCIE0);
    // 8-bit data, 1 stop bit, no parity.
    UCSR0C = (1 << UCSZ01) | (1 << UCSZ00);
    sei(); // enable global interrupts
}

/// @brief Transmit a single byte over USART0, blocking until the Tx buffer is
/// ready.
static void uart_write_byte(const uint8_t byte) {
    while (!(UCSR0A & (1 << UDRE0))) {
        // spin-wait
    }
    UDR0 = byte;
}

/// @brief USART0 Rx-complete ISR – push the received byte into the read buffer.
ISR(USART_RX_vect) {
    g_read_buffer.push_back(UDR0);
}

#else // ── Host (PC) stubs ────────────────────────────────────────────────────

#include <cstdio>

static void uart_init(const uint32_t /*baud*/) {
    // Nothing to initialise on the host.
}

/// On the host, output bytes are written to stdout so the example_client can
/// read them from the same serial device / pipe.
static void uart_write_byte(const uint8_t byte) {
    std::putchar(static_cast<int>(byte));
    std::fflush(stdout);
}

#endif // __AVR__

// =============================================================================
// Protobuf transformers
// =============================================================================

/// Convert a decoded `example_api_ExampleRequest` nanopb struct into the
/// application-level `ExampleRequest` type.
static ExampleRequest pb_to_request(const example_api_ExampleRequest& pb) {
    return ExampleRequest{
        .value  = pb.value,
        .action = static_cast<Action>(pb.action)
    };
}

/// Convert an application-level `ExampleResponse` into the nanopb struct used
/// for encoding.
static example_api_ExampleResponse response_to_pb(const ExampleResponse& resp) {
    example_api_ExampleResponse pb = example_api_ExampleResponse_init_default;
    pb.result = resp.result;
    return pb;
}

// =============================================================================
// Business logic
// =============================================================================

/// Process a single request and return the corresponding response.
static ExampleResponse process_request(const ExampleRequest& req) {
    switch (req.action) {
        case Action::ADD:
            return ExampleResponse{ .result = req.value + 1 };
        case Action::SUBTRACT:
            return ExampleResponse{ .result = req.value - 1 };
        default:
            return ExampleResponse{ .result = req.value };
    }
}

// =============================================================================
// Application entry points (Arduino sketch-style)
// =============================================================================

static nanoipc::NanoIpcReader<ExampleRequest>  *g_reader  = nullptr;
static nanoipc::NanoIpcWriter<ExampleResponse> *g_writer  = nullptr;

/// Static storage for reader/writer instances (avoids heap allocation).
static uint8_t g_reader_storage[sizeof(nanoipc::NanoIpcReader<ExampleRequest>)];
static uint8_t g_writer_storage[sizeof(nanoipc::NanoIpcWriter<ExampleResponse>)];

/// Initialise UART and construct the reader/writer.
void setup() {
    uart_init(9600);

    // --- Reader: parses incoming ExampleRequest messages ---
    nanoipc::NanoPbParser<ExampleRequest, example_api_ExampleRequest> parser(
        pb_to_request,
        example_api_ExampleRequest_fields
    );

    g_reader = new (g_reader_storage) nanoipc::NanoIpcReader<ExampleRequest>(
        parser,
        &g_read_buffer
    );

    // --- Writer: serialises outgoing ExampleResponse messages ---
    nanoipc::NanoPbSerializer<ExampleResponse, example_api_ExampleResponse> serializer(
        response_to_pb,
        example_api_ExampleResponse_fields
    );

    auto raw_writer = [](const uint8_t* data, const std::size_t size) {
        for (std::size_t i = 0; i < size; ++i) {
            uart_write_byte(data[i]);
        }
    };

    g_writer = new (g_writer_storage) nanoipc::NanoIpcWriter<ExampleResponse>(
        serializer,
        raw_writer
    );
}

/// Main loop – poll the reader; when a complete request arrives, process it
/// and send the response.
void loop() {
    if (!g_reader || !g_writer) {
        return;
    }

    const auto request = g_reader->read();
    if (!request.has_value()) {
        return; // no complete message yet
    }

    const auto response = process_request(request.value());
    g_writer->write(response);
}

// =============================================================================
// Host entry point
// =============================================================================

#ifndef __AVR__
int main() {
    setup();
    while (true) {
        loop();
    }
    return 0;
}
#endif // !__AVR__
