#ifndef	NANOIPC_WRITER_HPP
#define	NANOIPC_WRITER_HPP

#include <cstddef>
#include <cstdint>
#include <functional>
#include <stdexcept>
#include <vector>

#include "nanoipc_utils.hpp"

namespace nanoipc {
	template <typename Message>
	class NanoIpcWriter {
	public:
		using MessageSerializer = std::function<std::vector<std::uint8_t>(const Message& message)>;
		using RawDataWriter = std::function<void(const std::uint8_t *raw_data, const std::size_t raw_data_size)>;

		NanoIpcWriter(
			const MessageSerializer& message_serializer,
			const RawDataWriter& raw_data_writer
		): m_message_serializer(message_serializer), m_raw_data_writer(raw_data_writer) {
			if (!m_message_serializer || !m_raw_data_writer) {
				throw std::invalid_argument("invalid arguments in NanoIpcWriter ctor");
			}
		}
		NanoIpcWriter(const NanoIpcWriter&) = default;
		NanoIpcWriter& operator=(const NanoIpcWriter&) = default;
		virtual ~NanoIpcWriter() noexcept = default;

		void write(const Message& message) {
			const auto serial_message = m_message_serializer(message);
			const auto encoded_message = encode_frame(serial_message.data(), serial_message.size());
			m_raw_data_writer(encoded_message.data(), encoded_message.size());
		}

	private:
		MessageSerializer m_message_serializer;
		RawDataWriter m_raw_data_writer;
	};
}

#endif // NANOIPC_WRITER_HPP
