#include <cstddef>
#include <cstring>
#include <optional>
#include <string>

#include "gtest/gtest.h"

#include "cobs_frame_reader.hpp"
#include "cobs_frame_writer.hpp"
#include "pb_encode.h"
#include "pb_message_reader.hpp"
#include "pb_message_writer.hpp"

#include "ring_buffer.hpp"
#include "test.pb.h"

using namespace nanoipc;

static bool decode_string(pb_istream_t *stream, const pb_field_t *field, void **arg);
static bool encode_string(pb_ostream_t *stream, const pb_field_t *field, void * const *arg);
enum: std::size_t { MAX_BUFFER_SIZE = 256 };

TEST(ut_pb_message, pb_message_reader_ctor_sanity) {
    // WHEN
    RingBuffer<MAX_BUFFER_SIZE> ring_buffer;
    CobsFrameReader frame_reader(&ring_buffer);

    // THEN
    ASSERT_NO_THROW(PbMessageReader<test_api_TestMessage> reader(&frame_reader, &test_api_TestMessage_msg));
}

TEST(ut_pb_message, pb_message_writer_ctor_sanity) {
    // WHEN
    const auto dummy_raw_data_writer = [](const std::uint8_t *raw_data, const std::size_t raw_data_size) {

    };
    const FrameWriter frame_writer(dummy_raw_data_writer);

    // THEN
    ASSERT_NO_THROW(PbMessageWriter<test_api_TestMessage> writer(&frame_writer, &test_api_TestMessage_msg));
}

TEST(ut_pb_message, pb_message_writing_reading_sanity) {
    // GIVEN
    const auto test_string = std::string("test string");
    test_api_TestMessage pb_message {
        .int_value = 123,
        .string_value = {
            .funcs = { .encode = encode_string },
            .arg = const_cast<std::string *>(&test_string),
        },
    };

    // WHEN
    RingBuffer<MAX_BUFFER_SIZE> ring_buffer;
    const auto raw_data_writer = [&ring_buffer](const std::uint8_t *raw_data, const std::size_t raw_data_size) {
        for (std::size_t i = 0; i < raw_data_size; ++i) {
            ring_buffer.push_back(raw_data[i]);
        }
    };
    CobsFrameReader frame_reader(&ring_buffer);
    FrameWriter frame_writer(raw_data_writer);
    std::string string_buffer;
    PbMessageReader<test_api_TestMessage> pb_message_reader(
        &frame_reader,
        &test_api_TestMessage_msg,
        [&string_buffer](test_api_TestMessage *pb_message) {
            pb_message->string_value.funcs.decode = decode_string;
            pb_message->string_value.arg = &string_buffer;
        }
    );
    PbMessageWriter<test_api_TestMessage> pb_message_writer(&frame_writer, &test_api_TestMessage_msg);

    std::optional<test_api_TestMessage> read_pb_message;


    // THEN
    // before writing, there should be no pb_message data to read
    ASSERT_NO_THROW(read_pb_message = pb_message_reader.read());
    ASSERT_FALSE(read_pb_message.has_value());

    // after writing, the read pb_message data should be the same as the written pb_message data
    ASSERT_NO_THROW(pb_message_writer.write(pb_message));
    ASSERT_NO_THROW(read_pb_message = pb_message_reader.read());
    ASSERT_TRUE(read_pb_message.has_value());
    ASSERT_EQ(read_pb_message.value().int_value, pb_message.int_value);
    ASSERT_EQ(string_buffer, test_string);
}

bool encode_string(pb_ostream_t *stream, const pb_field_t *field, void * const *arg) {
	if (!arg || !*arg) {
		throw std::runtime_error("encode_string called with null arg");
	}
	const auto str = static_cast<const std::string *>(*arg);
	if (!pb_encode_tag_for_field(stream, field)) {
		return false;
	}
	return pb_encode_string(stream, (const pb_byte_t *)(str->c_str()), str->size());
}

bool decode_string(pb_istream_t *stream, const pb_field_t *field, void **arg) {
	if (!arg) {
		throw std::runtime_error("encode_string called with null arg");
	}
	std::string *dst = *(std::string **)(arg);
    if (!dst) {
		throw std::runtime_error("destination buffer is not set");
	}
	pb_byte_t buff[MAX_BUFFER_SIZE] = {'\0'};
	if (!pb_read(stream, buff, stream->bytes_left)) {
		return false;
	}
	*dst = std::string((const char *)buff);
    return true;
}