#ifndef	NANOIPC_READER_HPP
#define	NANOIPC_READER_HPP

#include <cstddef>
#include <cstdint>
#include <functional>
#include <optional>
#include <stdexcept>
#include <vector>

#include "nanoipc_read_buffer.hpp"
#include "nanoipc_utils.hpp"

namespace nanoipc {
	template <typename Message>
	class NanoIpcReader {
	public:
		using MessageParser = std::function<Message(const std::uint8_t *raw_data, const std::size_t raw_data_size)>;

		NanoIpcReader(
			const MessageParser& message_parser,
			ReadBuffer *read_buffer
		): m_message_parser(message_parser), m_read_buffer(read_buffer) {
			if (!m_message_parser || !m_read_buffer) {
				throw std::invalid_argument("invalid arguments in NanoIpcReader ctor");
			}
		}
		NanoIpcReader(const NanoIpcReader&) = default;
		NanoIpcReader& operator=(const NanoIpcReader&) = default;
		virtual ~NanoIpcReader() noexcept = default;

		std::optional<Message> read() {
			const auto encoded_frame = read_encoded_frame(m_read_buffer);
			if (!encoded_frame.has_value()) {
				return std::nullopt;
			}
			const auto decoded_frame = decode_frame(encoded_frame->data(), encoded_frame->size());
			return m_message_parser(decoded_frame.data(), decoded_frame.size());
		}

	private:
		MessageParser m_message_parser;
		ReadBuffer *m_read_buffer;
	};
}

#endif // NANOIPC_READER_HPP