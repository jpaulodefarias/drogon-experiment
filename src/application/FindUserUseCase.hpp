#pragma once

#include "domain/UserRepository.hpp"

#include <functional>
#include <string>
#include <variant>

namespace example {
enum class FindUserError { InvalidId, NotFound, Unavailable };
using FindUserResult = std::variant<User, FindUserError>;

class FindUserUseCase {
  public:
    explicit FindUserUseCase(UserRepository &repository);
    void execute(std::string id, std::function<void(FindUserResult)> done) const;

  private:
    UserRepository &repository_;
};
} // namespace example
