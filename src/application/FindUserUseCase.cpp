#include "application/FindUserUseCase.hpp"
#include "application/Uuid.hpp"

namespace example {

FindUserUseCase::FindUserUseCase(UserRepository &repository) : repository_(repository) {}

void FindUserUseCase::execute(std::string id, std::function<void(FindUserResult)> done) const {
    if (!detail::validUuid(id)) {
        done(FindUserError::InvalidId);
        return;
    }
    repository_.findById(
        std::move(id),
        [done](std::optional<User> user) {
            if (user) {
                done(std::move(*user));
            } else {
                done(FindUserError::NotFound);
            }
        },
        [done](RepositoryError) { done(FindUserError::Unavailable); });
}
} // namespace example
