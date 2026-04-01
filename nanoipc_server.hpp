#ifndef	NANOIPC_HPP
#define	NANOIPC_HPP

#include <cstddef>
#include <cstdint>
#include <functional>
#include <optional>
#include <stdexcept>
#include <vector>

#include "nanoipc_read_buffer.hpp"
#include "cobs.h"

namespace nanoipc {
	template <typename IpcRequest, typename IpcResponse>
	class NanoIpcServer {
	public:
		using RequestHandler = std::function<IpcResponse(const IpcRequest&)>;
		using RequestParser = std::function<IpcRequest(const std::uint8_t *raw_data, const std::size_t raw_data_size)>;
		using ResponseSerializer = std::function<std::size_t(const IpcResponse& response, std::uint8_t *dest_buff, const std::size_t dest_buff_capacity)>;
		using SerialDataWriter = std::function<void(const std::uint8_t *raw_data, const std::size_t raw_data_size)>;

		NanoIpcServer(
			const RequestHandler& request_handler,
			const RequestParser& request_parser,
			const ResponseSerializer& response_serializer,
			const SerialDataWriter& serial_data_writer,
			ReadBuffer *read_buffer
		): m_request_handler(request_handler), m_request_parser(request_parser), m_response_serializer(response_serializer), m_serial_data_writer(serial_data_writer), m_read_buffer(read_buffer) {
			if (!m_request_handler || !m_request_parser || !m_response_serializer || !m_serial_data_writer || !m_read_buffer) {
				throw std::invalid_argument("invalid arguments in NanoIpcServer ctor");
			}
		}
		NanoIpcServer(const NanoIpcServer&) = default;
		NanoIpcServer& operator=(const NanoIpcServer&) = default;
		virtual ~NanoIpcServer() noexcept = default;
		void process() {
			const auto frame_raw_data = read_frame(m_read_buffer);
			if (!frame_raw_data.has_value()) {
				return;
			}
			throw std::runtime_error("not implemented");
		}
	private:
		RequestHandler m_request_handler;
		RequestParser m_request_parser;
		ResponseSerializer m_response_serializer;
		SerialDataWriter m_serial_data_writer;
		ReadBuffer *m_read_buffer;
		
		static std::optional<std::size_t> find_delimiter(const ReadBuffer& read_buffer) {
			for (auto i = std::size_t(0); i < read_buffer.size(); ++i) {
				if (read_buffer.get(i) == COBS_FRAME_DELIMITER) {
					return i;
				}
			}
			return std::nullopt;
		}

		static std::optional<std::vector<std::uint8_t>> read_encoded_frame(ReadBuffer *read_buffer) {
			const auto delimiter_index = find_delimiter(*read_buffer);
			if (!delimiter_index.has_value()) {
				return std::nullopt;
			}
			std::vector<std::uint8_t> encoded_frame_data;
			encoded_frame_data.reserve(delimiter_index.value() + 1);
			for (auto i = std::size_t(0); i <= delimiter_index.value(); ++i) {
				encoded_frame_data.push_back(read_buffer->pop_front());
			}
			return std::make_optional(encoded_frame_data);
		}

		static std::optional<std::vector<std::uint8_t>> read_frame(ReadBuffer *read_buffer) {
			const auto encoded_frame_data = read_encoded_frame(read_buffer);
			if (!encoded_frame_data.has_value()) {
				return std::nullopt;
			}
			std::vector<std::uint8_t> decoded_frame_data;
			std::size_t decoded_frame_data_size = 0;
			decoded_frame_data.reserve(encoded_frame_data->size());
			const auto decode_result = cobs_decode(encoded_frame_data->data(), encoded_frame_data->size(), decoded_frame_data.data(), decoded_frame_data.capacity(), &decoded_frame_data_size);
			if (decode_result != COBS_RET_SUCCESS) {
				throw std::runtime_error("failed to decode COBS frame");
			}
			decoded_frame_data.resize(decoded_frame_data_size);
			return std::make_optional(decoded_frame_data);
		}
	};
}

#endif // NANOIPC_HPP