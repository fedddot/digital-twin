#include <cstddef>
#include <cstdint>
#include <optional>
#include <stdexcept>
#include <string>
#include <vector>

#include "gtest/gtest.h"
#include "gmock/gmock.h"

#include "nanopb_message_reader.hpp"
#include "data_reader.hpp"
#include "pb.h"
#include "pb_encode.h"
#include "test.pb.h"

using namespace ipc;

using ApiMessage = std::string;

class MockPackageReader : public DataReader<std::optional<std::vector<std::uint8_t>>> {
public:
    MOCK_METHOD(std::optional<std::vector<std::uint8_t>>, read, (), (override));
};

static std::vector<std::uint8_t> serialize_api_message(const ApiMessage& msg);
static bool decode_string(pb_istream_t *stream, const pb_field_t *field, void **arg);

TEST(ut_nanopb_message_reader, read_sanity) {
	// GIVEN
	const auto test_api_message = ApiMessage("test_msg");
	const auto test_serialized_message = serialize_api_message(test_api_message);
	auto package_reader = ::testing::NiceMock<MockPackageReader>();
    EXPECT_CALL(package_reader, read())
		.WillOnce(::testing::Return(test_serialized_message));
	const auto message_parser = [](const test_api_TestRequest& data) -> ApiMessage {
		return ApiMessage(*(const std::string *)data.request.arg);
	};
	const auto init_pb_msg = []() -> test_api_TestRequest {
		char buff[256] = {'\0'};
		test_api_TestRequest pb_message = test_api_TestRequest_init_default;
		pb_message.request.funcs.decode = decode_string;
		pb_message.request.arg = new std::string();
		return pb_message;
	};
	const auto deinit_pb_msg = [](test_api_TestRequest *pb_message) {
		delete (std::string *)((pb_message->request).arg);
	};

	// WHEN
	NanopbMessageReader<ApiMessage, test_api_TestRequest> instance(
		&package_reader,
		message_parser,
		init_pb_msg,
		deinit_pb_msg,
		test_api_TestRequest_fields
	);
	auto result = std::optional<ApiMessage>();

	// THEN
	ASSERT_NO_THROW(result = instance.read());
	ASSERT_TRUE(result.has_value());
	ASSERT_EQ(*result, test_api_message);
}

static bool encode_string(pb_ostream_t *stream, const pb_field_t *field, void * const *arg);
inline std::vector<std::uint8_t> serialize_api_message(const ApiMessage& msg) {
	test_api_TestRequest api_request(test_api_TestRequest_init_default);
	api_request.request.funcs.encode = encode_string;
	api_request.request.arg = const_cast<char *>(msg.c_str());
	enum: std::size_t { BUFF_SIZE = 256UL };
	pb_byte_t buff[BUFF_SIZE] = { '\0' };
	pb_ostream_t ostream = pb_ostream_from_buffer(buff, BUFF_SIZE);
	if (!pb_encode(&ostream, test_api_TestRequest_fields, &api_request)) {
		throw std::runtime_error("failed to encode api message");
	}
	return std::vector<std::uint8_t>(static_cast<const std::uint8_t *>(buff), static_cast<const std::uint8_t *>(buff + ostream.bytes_written));
}

inline bool encode_string(pb_ostream_t *stream, const pb_field_t *field, void * const *arg) {
	if (!arg || !*arg) {
		throw std::runtime_error("encode_string called with null arg");
	}
	const auto str = static_cast<const char *>(*arg);
	if (!pb_encode_tag_for_field(stream, field)) {
		return false;
	}
	return pb_encode_string(stream, (const pb_byte_t *)(str), std::strlen(str));
}

inline bool decode_string(pb_istream_t *stream, const pb_field_t *field, void **arg) {
	if (!arg) {
		throw std::runtime_error("encode_string called with null arg");
	}
	std::string *dst = *(std::string **)(arg);
    if (!dst) {
		throw std::runtime_error("destination buffer is not set");
	}
	pb_byte_t buff[256UL] = {'\0'};
	if (!pb_read(stream, buff, stream->bytes_left)) {
		return false;
	}
	*dst = std::string((const char *)buff);
    return true;
}