#pragma once

#include <charconv>
#include <stdexcept>
#include <string_view>

namespace example::config {
inline int parseHttpPort(std::string_view value) {
    int port = 0;
    const auto [end, error] = std::from_chars(value.data(), value.data() + value.size(), port);
    if (error != std::errc{} || end != value.data() + value.size() || port < 1 || port > 65535) {
        throw std::invalid_argument("HTTP_PORT must be an integer between 1 and 65535");
    }
    return port;
}
} // namespace example::config
