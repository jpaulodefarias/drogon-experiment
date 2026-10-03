#pragma once

#include "domain/UserRepository.hpp"

#include <functional>
#include <variant>
#include <vector>

namespace example {
enum class ListUsersError { Unavailable };
using ListUsersResult = std::variant<std::vector<User>, ListUsersError>;

class ListUsersUseCase {
  public:
    explicit ListUsersUseCase(UserRepository &repository);
    void execute(std::function<void(ListUsersResult)> done) const;

  private:
    UserRepository &repository_;
};
} // namespace example
