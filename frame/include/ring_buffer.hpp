#ifndef RING_BUFFER_HPP
#define RING_BUFFER_HPP

#include <array>
#include <cstddef>
#include <cstdint>
#include <stdexcept>

#include "read_buffer.hpp"

namespace nanoipc {
    /// @brief Fixed-capacity ring buffer implementing the ReadBuffer interface.
    ///
    /// RingBuffer provides a compact, header-only circular buffer that
    /// implements nanoipc::ReadBuffer so it can be used with NanoIpcReader. The
    /// buffer supports writing bytes with push_back() and reading them via
    /// pop_front(), size(), and get().
    ///
    /// Template parameter N specifies the capacity (maximum number of bytes the
    /// buffer can hold).
    ///
    /// Notes:
    /// - This implementation does not perform any synchronization. If used from
    ///   multiple threads or from interrupt context, the caller must provide
    ///   appropriate synchronization or use an ISR-safe variant.
    /// - All methods throw exceptions on error conditions (full buffer, empty buffer, out of range).
    template <std::size_t N>
    class RingBuffer : public ReadBuffer {
    public:
        /// @brief Construct an empty ring buffer.
        RingBuffer() : m_head(0), m_tail(0), m_size(0) {

        }
        RingBuffer(const RingBuffer&) = default;
        RingBuffer& operator=(const RingBuffer&) = default;

        /// @brief Append a byte to the back of the buffer.
        /// @param v The byte to append.
        /// @throws std::overflow_error if the buffer is full.
        void push_back(std::uint8_t v) {
            if (m_size >= N) {
                throw std::overflow_error("RingBuffer: buffer is full");
            }
            m_data[m_tail] = v;
            m_tail = (m_tail + 1) % N;
            ++m_size;
        }

        /// @brief Remove and return the oldest byte from the buffer.
        /// @return The oldest byte in the buffer.
        /// @throws std::out_of_range if the buffer is empty.
        std::uint8_t pop_front() override {
            if (m_size == 0) {
                throw std::out_of_range("RingBuffer: buffer is empty");
            }
            const std::uint8_t v = m_data[m_head];
            m_head = (m_head + 1) % N;
            --m_size;
            return v;
        }

        /// @brief Number of bytes currently in the buffer.
        std::size_t size() const override {
            return m_size;
        }

        /// @brief Peek at a byte without removing it.
        /// @param index Zero-based offset from the front of the buffer.
        /// @throws std::out_of_range if index is >= size().
        std::uint8_t get(const std::size_t index) const override {
            if (index >= m_size) {
                throw std::out_of_range("RingBuffer: index out of range");
            }
            return m_data[(m_head + index) % N];
        }
    private:
        std::array<std::uint8_t, N> m_data;
        std::size_t m_head;
        std::size_t m_tail;
        std::size_t m_size;
    };
}

#endif // RING_BUFFER_HPP
