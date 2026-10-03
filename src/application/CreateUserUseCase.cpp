#include "application/CreateUserUseCase.hpp"

namespace example {
CreateUserUseCase::CreateUserUseCase(UserRepository &repository) : repository_(repository) {}

void CreateUserUseCase::execute(CreateUserCommand command,
                                std::function<void(CreateUserResult)> done) const {
    const auto at = command.email.find('@');
    if (command.name.empty() || command.name.size() > 200 || command.email.empty() ||
        command.email.size() > 320 || at == std::string::npos || at == 0 ||
        at + 1 >= command.email.size()) {
        done(CreateUserError::InvalidInput);
        return;
    }

    repository_.save(
        User{"", std::move(command.name), std::move(command.email)},
        [done](User user) { done(std::move(user)); },
        [done](RepositoryError error) {
            done(error == RepositoryError::Conflict ? CreateUserError::Conflict
                                                    : CreateUserError::Unavailable);
        });
}
} // namespace example
