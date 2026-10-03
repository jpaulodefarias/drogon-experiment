#pragma once

#include "domain/UserRepository.hpp"

#include <functional>
#include <variant>

namespace example {
enum class DeleteAllUsersError { Unavailable };
using DeleteAllUsersResult = std::variant<std::monostate, DeleteAllUsersError>;

class DeleteAllUsersUseCase {
  public:
    explicit DeleteAllUsersUseCase(UserRepository &repository);
    void execute(std::function<void(DeleteAllUsersResult)> done) const;

  private:
    UserRepository &repository_;
};
} // namespace example
