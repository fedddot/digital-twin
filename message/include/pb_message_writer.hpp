#ifndef	PB_MESSAGE_WRITER_HPP
#define	PB_MESSAGE_WRITER_HPP

#include <cstddef>
#include <cstdint>
#include <stdexcept>
#include <vector>

#include "pb.h"
#include "pb_encode.h"
#include "writer.hpp"

namespace nanoipc {
    template <typename Tpb_msg, std::size_t MAX_MSG_SIZE = 0xFF>
	class PbMessageWriter: public Writer<Tpb_msg> {
	public:
		PbMessageWriter(
            const Writer<std::vector<std::uint8_t>> *frame_writer,
            const pb_msgdesc_t *nanopb_message_fields
        ): m_frame_writer(frame_writer), m_nanopb_message_fields(nanopb_message_fields) {
			if (!m_frame_writer || !m_nanopb_message_fields) {
				throw std::invalid_argument("invalid arguments in PbMessageWriter ctor");
			}
		}
		PbMessageWriter(const PbMessageWriter&) = default;
		PbMessageWriter& operator=(const PbMessageWriter&) = default;

		void write(const Tpb_msg& data) const override {
			pb_byte_t buff[MAX_MSG_SIZE] = { 0 };
			pb_ostream_t ostream = pb_ostream_from_buffer(buff, MAX_MSG_SIZE);
			if (!pb_encode(&ostream, m_nanopb_message_fields, &data)) {
				throw std::runtime_error("failed to encode api message into protocol buffer: " + std::string(PB_GET_ERROR(&ostream)));
			}
			m_frame_writer->write(std::vector<std::uint8_t>(buff, buff + ostream.bytes_written));
		}

	private:
        const Writer<std::vector<std::uint8_t>> *m_frame_writer;
        const pb_msgdesc_t *m_nanopb_message_fields;
	};
}

#endif // PB_MESSAGE_WRITER_HPP
