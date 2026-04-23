#include "frame_reader.hpp"
#include "ring_buffer.hpp"
#include "gtest/gtest.h"

using namespace nanoipc;

TEST(ut_frame_reader, ctor) {
    // WHEN
    RingBuffer<10> ring_buffer;

    // THEN
    ASSERT_NO_THROW(FrameReader reader(&ring_buffer));
}
