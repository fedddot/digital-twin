#pragma once

#include <cstddef>
#include <cstdint>

#include "nanoipc_read_buffer.hpp"

namespace example {

/// @brief Fixed-capacity circular buffer that implements `nanoipc::ReadBuffer`.
///
/// `UartReadBuffer` is designed to be written from an interrupt service routine
/// (e.g. the UART Rx ISR on an AVR MCU) via `push_back()`, and read from the
/// main application loop via `pop_front()` / `size()` / `get()`.
///
/// @tparam CAPACITY  Maximum number of bytes the buffer can hold at one time.
template <std::size_t CAPACITY = 256>
class UartReadBuffer : public nanoipc::ReadBuffer {
public:
    UartReadBuffer() : m_head(0), m_tail(0), m_size(0) {}

    // ----- ReadBuffer interface -----

    /// @brief Remove and return the oldest byte in the buffer.
    /// @note Must only be called from the main application context (not ISR).
    std::uint8_t pop_front() override {
        if (m_size == 0) {
            return 0;
        }
        const std::uint8_t byte = m_data[m_head];
        m_head = (m_head + 1) % CAPACITY;
        --m_size;
        return byte;
    }

    /// @brief Number of bytes currently in the buffer.
    std::size_t size() const override {
        return m_size;
    }

    /// @brief Peek at a byte without removing it.
    /// @param index  Zero-based offset from the front of the buffer.
    std::uint8_t get(const std::size_t index) const override {
        return m_data[(m_head + index) % CAPACITY];
    }

    // ----- ISR-callable writer -----

    /// @brief Append a byte at the back of the buffer.
    ///
    /// This method is intended to be called from the UART Rx ISR.
    /// If the buffer is full the incoming byte is silently dropped.
    void push_back(const std::uint8_t byte) {
        if (m_size >= CAPACITY) {
            return; // overflow – drop byte
        }
        m_data[m_tail] = byte;
        m_tail = (m_tail + 1) % CAPACITY;
        ++m_size;
    }

private:
    volatile std::uint8_t  m_data[CAPACITY];
    volatile std::size_t   m_head;
    volatile std::size_t   m_tail;
    volatile std::size_t   m_size;
};

} // namespace example
