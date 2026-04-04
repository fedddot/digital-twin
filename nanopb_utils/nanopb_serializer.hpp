#ifndef	NANOPB_SERIALIZER_HPP
#define	NANOPB_SERIALIZER_HPP

#include <cstdint>
#include <functional>
#include <stdexcept>
#include <vector>

#include "pb.h"
#include "pb_encode.h"

namespace nanoipc {
	template <typename Message, typename PbMessage, std::size_t MAX_MSG_SIZE = 0xFF>
	class NanoPbSerializer {
	public:
		using MessageToPbTransformer = std::function<PbMessage(const Message& message)>;

		NanoPbSerializer(
			const MessageToPbTransformer& message_transformer,
			const pb_msgdesc_t *nanopb_message_fields
		): m_message_transformer(message_transformer), m_nanopb_message_fields(nanopb_message_fields) {
			if (!m_message_transformer || !m_nanopb_message_fields) {
				throw std::invalid_argument("invalid arguments in NanoPbSerializer ctor");
			}
		}
		NanoPbSerializer(const NanoPbSerializer&) = default;
		NanoPbSerializer& operator=(const NanoPbSerializer&) = default;
		virtual ~NanoPbSerializer() noexcept = default;

		std::vector<std::uint8_t> operator()(const Message& message) const {
			const auto pb_message = m_message_transformer(message);
			pb_byte_t buff[MAX_MSG_SIZE] = { 0 };
			pb_ostream_t ostream = pb_ostream_from_buffer(
                buff,
                MAX_MSG_SIZE
            );
            if (!pb_encode(&ostream, m_nanopb_message_fields, &pb_message)) {
                throw std::runtime_error("failed to encode api message into protocol buffer: " + std::string(PB_GET_ERROR(&ostream)));
            }
			return std::vector<std::uint8_t>(buff, buff + ostream.bytes_written);
		}
	private:
		MessageToPbTransformer m_message_transformer;
		const pb_msgdesc_t *m_nanopb_message_fields;
	};
}

#endif // NANOPB_SERIALIZER_HPP
