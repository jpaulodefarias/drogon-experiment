#include "application/DeleteUserUseCase.hpp"
#include "application/Uuid.hpp"

#include <utility>

namespace example {

DeleteUserUseCase::DeleteUserUseCase(UserRepository &repository) : repository_(repository) {}

void DeleteUserUseCase::execute(std::string id, std::function<void(DeleteUserResult)> done) const {
    if (!detail::validUuid(id)) {
        done(DeleteUserError::InvalidId);
        return;
    }

    repository_.deleteById(
        std::move(id),
        [done](bool deleted) {
            done(deleted ? DeleteUserResult{std::monostate{}}
                         : DeleteUserResult{DeleteUserError::NotFound});
        },
        [done](RepositoryError) { done(DeleteUserError::Unavailable); });
}
} // namespace example
