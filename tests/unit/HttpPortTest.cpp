#include "config/HttpPort.hpp"

#include <gtest/gtest.h>

#include <stdexcept>

namespace example::config {
TEST(HttpPortTest, AcceptsValidPortsAtBounds) {
    EXPECT_EQ(parseHttpPort("1"), 1);
    EXPECT_EQ(parseHttpPort("65535"), 65535);
}

TEST(HttpPortTest, RejectsEmptyMalformedAndOutOfRangeValues) {
    for (const auto value : {"", "abc", "8080extra", "0", "65536", "-1"}) {
        EXPECT_THROW(parseHttpPort(value), std::invalid_argument) << value;
    }
}
} // namespace example::config
