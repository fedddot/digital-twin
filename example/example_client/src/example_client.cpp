/// @file example_client.cpp
/// @brief NanoIPC example client – runs on a PC (Linux / macOS).
///
/// Opens a serial port, sends an ExampleRequest to the server, and prints
/// the ExampleResponse result to stdout.
///
/// Usage:
///   example_client --port <dev> --baud <rate> --value <n> --action <add|subtract>
///
/// Example:
///   example_client --port /dev/ttyUSB0 --baud 9600 --value 5 --action add

#include <algorithm>
#include <chrono>
#include <cstdint>
#include <cstdlib>
#include <iostream>
#include <stdexcept>
#include <string>
#include <thread>

// Serial port
#include "serialib.h"

// NanoIPC
#include "nanoipc_ring_buffer.hpp"

// API
#include "example_api.hpp"
#include "example_request_writer.hpp"
#include "example_response_reader.hpp"

// =============================================================================
// CLI parsing
// =============================================================================

struct Config {
    std::string port   = "/dev/ttyUSB0";
    uint32_t    baud   = 9600;
    int32_t     value  = 0;
    api::Action action = api::Action::ADD;
};

static void usage(const char* prog) {
    std::cerr << "Usage: " << prog
              << " --port <dev> --baud <rate> --value <n> --action <add|subtract>\n";
}

static Config parse_args(int argc, char* argv[]) {
    Config cfg;
    for (int i = 1; i < argc; ++i) {
        const std::string arg = argv[i];
        if ((arg == "--port") && i + 1 < argc) {
            cfg.port = argv[++i];
        } else if ((arg == "--baud") && i + 1 < argc) {
            cfg.baud = static_cast<uint32_t>(std::stoul(argv[++i]));
        } else if ((arg == "--value") && i + 1 < argc) {
            cfg.value = static_cast<int32_t>(std::stol(argv[++i]));
        } else if ((arg == "--action") && i + 1 < argc) {
            const std::string act = argv[++i];
            if (act == "add") {
                cfg.action = api::Action::ADD;
            } else if (act == "subtract") {
                cfg.action = api::Action::SUBTRACT;
            } else {
                throw std::runtime_error("unknown action '" + act + "' (use add or subtract)");
            }
        } else {
            throw std::runtime_error("unknown argument: " + arg);
        }
    }
    return cfg;
}

// =============================================================================
// Main
// =============================================================================

int main(int argc, char* argv[]) {
    Config cfg;
    try {
        cfg = parse_args(argc, argv);
    } catch (const std::exception& e) {
        std::cerr << "error: " << e.what() << "\n";
        usage(argv[0]);
        return EXIT_FAILURE;
    }

    serialib serial;
    try {
        if (serial.openDevice(cfg.port.c_str(), cfg.baud) != 1) {
            throw std::runtime_error("cannot open port " + cfg.port);
        }

        nanoipc::NanoipcRingBuffer<256> read_buffer;

        api::ExampleRequestWriter writer(
            [&serial](const std::uint8_t* data, const std::size_t size) {
                serial.writeBytes(data, static_cast<unsigned int>(size));
            }
        );

        api::ExampleResponseReader reader(&read_buffer);

        // Send the request
        writer.write(api::ExampleRequest{ cfg.value, cfg.action });

        // Poll for a response (timeout: 5 s)
        const auto deadline = std::chrono::steady_clock::now() + std::chrono::seconds(5);
        while (std::chrono::steady_clock::now() < deadline) {
            // Drain any available bytes from the serial port into the ring buffer
            const int avail = serial.available();
            if (avail > 0) {
                uint8_t tmp[64];
                const int n = serial.readBytes(tmp, std::min(avail, 64), 10);
                for (int i = 0; i < n; ++i) {
                    read_buffer.push_back(tmp[i]);
                }
            }

            const auto response = reader.read();
            if (response.has_value()) {
                std::cout << response->result << "\n";
                serial.closeDevice();
                return EXIT_SUCCESS;
            }
            std::this_thread::sleep_for(std::chrono::milliseconds(1));
        }

        std::cerr << "error: timed out waiting for response\n";
        serial.closeDevice();
        return EXIT_FAILURE;

    } catch (const std::exception& e) {
        std::cerr << "error: " << e.what() << "\n";
        serial.closeDevice();
        return EXIT_FAILURE;
    }
}
