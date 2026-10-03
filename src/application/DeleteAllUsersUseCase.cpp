#include "application/DeleteAllUsersUseCase.hpp"

namespace example {
DeleteAllUsersUseCase::DeleteAllUsersUseCase(UserRepository &repository)
    : repository_(repository) {}

void DeleteAllUsersUseCase::execute(std::function<void(DeleteAllUsersResult)> done) const {
    repository_.deleteAll([done] { done(std::monostate{}); },
                          [done](RepositoryError) { done(DeleteAllUsersError::Unavailable); });
}
} // namespace example
