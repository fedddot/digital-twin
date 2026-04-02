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
		using RequestParser = std::function<Message(const std::uint8_t *raw_data, const std::size_t raw_data_size)>;

		NanoIpcReader(
			const RequestHandler& request_handler,
			const RequestParser& request_parser,
			const ResponseSerializer& response_serializer,
			const SerialDataWriter& serial_data_writer,
			ReadBuffer *read_buffer
		): m_request_handler(request_handler), m_request_parser(request_parser), m_response_serializer(response_serializer), m_serial_data_writer(serial_data_writer), m_read_buffer(read_buffer) {
			if (!m_request_handler || !m_request_parser || !m_response_serializer || !m_serial_data_writer || !m_read_buffer) {
				throw std::invalid_argument("invalid arguments in NanoIpcReader ctor");
			}
		}
		NanoIpcReader(const NanoIpcReader&) = default;
		NanoIpcReader& operator=(const NanoIpcReader&) = default;
		virtual ~NanoIpcReader() noexcept = default;
		void process() {
			const auto encoded_frame = read_encoded_frame(m_read_buffer);
			if (!encoded_frame.has_value()) {
				return;
			}
			const auto decoded_frame = decode_frame(encoded_frame->data(), encoded_frame->size());
			const auto request = m_request_parser(decoded_frame.data(), decoded_frame.size());
			const auto response = m_request_handler(request);
			const auto serial_response = m_response_serializer(response);
			const auto encoded_response = encode_frame(serial_response.data(), serial_response.size());
			m_serial_data_writer(encoded_response.data(), encoded_response.size());
		}
	private:
		RequestHandler m_request_handler;
		RequestParser m_request_parser;
		ResponseSerializer m_response_serializer;
		SerialDataWriter m_serial_data_writer;
		ReadBuffer *m_read_buffer;
	};
}

#endif // NANOIPC_READER_HPP