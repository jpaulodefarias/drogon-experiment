#pragma once

#include "domain/UserRepository.hpp"

#include <functional>
#include <string>
#include <variant>

namespace example {
enum class DeleteUserError { InvalidId, NotFound, Unavailable };
using DeleteUserResult = std::variant<std::monostate, DeleteUserError>;

class DeleteUserUseCase {
  public:
    explicit DeleteUserUseCase(UserRepository &repository);
    void execute(std::string id, std::function<void(DeleteUserResult)> done) const;

  private:
    UserRepository &repository_;
};
} // namespace example
