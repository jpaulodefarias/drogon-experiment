#pragma once

#include "domain/UserRepository.hpp"

#include <functional>
#include <string>
#include <variant>

namespace example {
struct CreateUserCommand {
    std::string name;
    std::string email;
};

enum class CreateUserError { InvalidInput, Conflict, Unavailable };
using CreateUserResult = std::variant<User, CreateUserError>;

class CreateUserUseCase {
  public:
    explicit CreateUserUseCase(UserRepository &repository);
    void execute(CreateUserCommand command, std::function<void(CreateUserResult)> done) const;

  private:
    UserRepository &repository_;
};
} // namespace example
