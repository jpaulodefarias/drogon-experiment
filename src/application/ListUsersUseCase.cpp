#include "application/ListUsersUseCase.hpp"

namespace example {
ListUsersUseCase::ListUsersUseCase(UserRepository &repository) : repository_(repository) {}

void ListUsersUseCase::execute(std::function<void(ListUsersResult)> done) const {
    repository_.list([done](std::vector<User> users) { done(std::move(users)); },
                     [done](RepositoryError) { done(ListUsersError::Unavailable); });
}
} // namespace example
