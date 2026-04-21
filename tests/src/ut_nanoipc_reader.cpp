#include <climits>
#include <stdexcept>
#include <string>
#include <vector>

#include "gtest/gtest.h"

#include "nanoipc_reader.hpp"
#include "nanoipc_utils.hpp"

using namespace nanoipc;

using Message = std::string;

class VectorBuffer : public ReadBuffer {
public:
	VectorBuffer() = default;
	std::uint8_t pop_front() override {
		if (m_data.empty()) {
			throw std::runtime_error("pop_front called on empty buffer");
		}
		const auto front = m_data.front();
		m_data.erase(m_data.begin());
		return front;
	}
	std::size_t size() const override {
		return m_data.size();
	}
	std::uint8_t get(const std::size_t index) const override {
		if (index >= m_data.size()) {
			throw std::out_of_range("index out of range in get");
		}
		return m_data[index];
	}
	void push_back(const std::uint8_t byte) {
		m_data.push_back(byte);
	}
private:
	std::vector<std::uint8_t> m_data;
};

static Message parse_message(const std::uint8_t *raw_data, const std::size_t raw_data_size) {
	return Message(reinterpret_cast<const char*>(raw_data), raw_data_size);
}

TEST(ut_nanoipc_reader, sanity) {
	// GIVEN
	const auto expected_message = Message("hello world");

	VectorBuffer read_buffer;
	const auto encoded_message = encode_frame(
		reinterpret_cast<const std::uint8_t*>(expected_message.c_str()),
		expected_message.size()
	);
	for (const auto byte : encoded_message) {
		read_buffer.push_back(byte);
	}

	// WHEN
	NanoIpcReader<Message> reader(parse_message, &read_buffer);
	const auto result = reader.read();

	// THEN
	ASSERT_TRUE(result.has_value());
	ASSERT_EQ(result.value(), expected_message);
}

TEST(ut_nanoipc_reader, returns_nullopt_when_buffer_empty) {
	// GIVEN
	VectorBuffer empty_buffer;

	// WHEN
	NanoIpcReader<Message> reader(parse_message, &empty_buffer);
	const auto result = reader.read();

	// THEN
	ASSERT_FALSE(result.has_value());
}
