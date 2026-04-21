#include <string>
#include <vector>

#include "gtest/gtest.h"

#include "nanoipc_writer.hpp"
#include "nanoipc_utils.hpp"
#include "nanoipc_read_buffer.hpp"

using namespace nanoipc;

using Message = std::string;
using TestNanoIpcWriter = NanoIpcWriter<Message>;

static std::vector<std::uint8_t> serialize_message(const Message& message) {
	return std::vector<std::uint8_t>(message.begin(), message.end());
}

TEST(ut_nanoipc_writer, sanity) {
	// GIVEN
	const auto test_message = Message("hello world");
	std::vector<std::uint8_t> captured_data;
	auto raw_data_writer = [&captured_data](const std::uint8_t *raw_data, const std::size_t raw_data_size) {
		captured_data.assign(raw_data, raw_data + raw_data_size);
	};

	// WHEN
	TestNanoIpcWriter writer(serialize_message, raw_data_writer);
	writer.write(test_message);

	// THEN: decode what was written and verify it matches the original message
	const auto decoded = decode_frame(captured_data.data(), captured_data.size());
	const auto result = Message(reinterpret_cast<const char*>(decoded.data()), decoded.size());
	ASSERT_EQ(result, test_message);
}

TEST(ut_nanoipc_writer, invalid_args) {
	// GIVEN
	auto raw_data_writer = [](const std::uint8_t *, const std::size_t) {};

	// THEN
	ASSERT_THROW(TestNanoIpcWriter(nullptr, raw_data_writer), std::invalid_argument);
	ASSERT_THROW(TestNanoIpcWriter(serialize_message, nullptr), std::invalid_argument);
}
