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
	/// @brief Generic reader that extracts messages of type `Message` from a `ReadBuffer`.
	///
	/// `NanoIpcReader` uses a `MessageParser` to convert decoded frame payloads
	/// into `Message` instances. It reads encoded frames from the provided
	/// `ReadBuffer`, decodes them using `decode_frame`, and then invokes the
	/// parser to produce the `Message` value.
	///
	/// Template parameter `Message` is the user-defined type returned by the
	/// `MessageParser`.
	template <typename Message>
	class NanoIpcReader {
	public:
		/// @brief Function type that parses a raw payload into `Message`.
		///
		/// The parser receives a pointer to the decoded payload and its size and
		/// must return a `Message` instance representing the payload contents.
		using MessageParser = std::function<Message(const std::uint8_t *raw_data, const std::size_t raw_data_size)>;

		/// @brief Construct a `NanoIpcReader`.
		///
		/// @param message_parser Callable that converts decoded payload bytes into `Message`.
		/// @param read_buffer Pointer to the `ReadBuffer` to read encoded frames from. Must not be `nullptr`.
		/// @throws std::invalid_argument if `message_parser` is empty or `read_buffer` is `nullptr`.
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

		/// @brief Attempt to read and parse the next `Message`.
		///
		/// This method tries to read a complete encoded frame from the associated
		/// `ReadBuffer`. If no complete frame is available, the function returns
		/// `std::nullopt` and leaves the `ReadBuffer` unmodified. When a frame is
		/// available it is decoded and passed to the `MessageParser` to produce
		/// the returned `Message`.
		///
		/// @return std::optional<Message> The parsed `Message` when available,
		///         otherwise `std::nullopt`.
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