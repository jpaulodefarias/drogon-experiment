#pragma once

#include <cctype>
#include <string>

namespace example::detail {
inline bool validUuid(const std::string &id) {
    if (id.size() != 36) {
        return false;
    }
    for (std::size_t index = 0; index < id.size(); ++index) {
        if (index == 8 || index == 13 || index == 18 || index == 23) {
            if (id[index] != '-') {
                return false;
            }
        } else if (std::isxdigit(static_cast<unsigned char>(id[index])) == 0) {
            return false;
        }
    }
    return true;
}
} // namespace example::detail
