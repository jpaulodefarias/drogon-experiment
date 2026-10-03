#include "application/ListUsersUseCase.hpp"

#include <gmock/gmock.h>
#include <gtest/gtest.h>

namespace example {
namespace {
class MockUserRepository final : public UserRepository {
  public:
    MOCK_METHOD(void, save, (User, OnSaved, OnError), (override));
    MOCK_METHOD(void, findById, (std::string, OnFound, OnError), (override));
    MOCK_METHOD(void, list, (OnListed, OnError), (override));
    MOCK_METHOD(void, deleteById, (std::string, OnDeleted, OnError), (override));
    MOCK_METHOD(void, deleteAll, (OnDeletedAll, OnError), (override));
};

TEST(ListUsersUseCaseTest, ReturnsUsersFromRepository) {
    MockUserRepository repository;
    EXPECT_CALL(repository, list(testing::_, testing::_))
        .WillOnce([](UserRepository::OnListed listed, UserRepository::OnError) {
            listed({User{"id-1", "John", "john@example.com"},
                    User{"id-2", "Jane", "jane@example.com"}});
        });

    ListUsersUseCase useCase{repository};
    useCase.execute([](ListUsersResult result) {
        ASSERT_TRUE(std::holds_alternative<std::vector<User>>(result));
        const auto &users = std::get<std::vector<User>>(result);
        ASSERT_EQ(users.size(), 2U);
        EXPECT_EQ(users[0].id, "id-1");
        EXPECT_EQ(users[1].email, "jane@example.com");
    });
}

TEST(ListUsersUseCaseTest, RepositoryFailureReturnsUnavailable) {
    MockUserRepository repository;
    EXPECT_CALL(repository, list(testing::_, testing::_))
        .WillOnce([](UserRepository::OnListed, UserRepository::OnError error) {
            error(RepositoryError::Unavailable);
        });

    ListUsersUseCase useCase{repository};
    useCase.execute([](ListUsersResult result) {
        ASSERT_TRUE(std::holds_alternative<ListUsersError>(result));
        EXPECT_EQ(std::get<ListUsersError>(result), ListUsersError::Unavailable);
    });
}
} // namespace
} // namespace example
